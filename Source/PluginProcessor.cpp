#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
    constexpr float twoPi = 6.28318530718f;
    constexpr int controlInterval = 16;                       // filter coefficients refresh every N samples
    const int allpassLengths[2][4] { { 221, 389, 577, 877 },  // samples at 48 kHz, ~5-18 ms
                                     { 241, 421, 613, 929 } };// right channel offset for stereo spread
}

MirageProcessor::MirageProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "MIRAGE_STATE", mirage::createLayout())
{
}

void MirageProcessor::prepareToPlay (double newSampleRate, int)
{
    sampleRate = newSampleRate;

    const int capacity = (int) (8.0 * sampleRate) + 64;
    for (auto& d : delay) d.prepare (capacity);
    maxDelaySamples = (float) std::min ((double) delay[0].capacity() - 8.0, 8.0 * sampleRate);

    const double scale = sampleRate / 48000.0;
    float total = 0.0f;
    for (int c = 0; c < 2; ++c)
    {
        for (int k = 0; k < numAllpass; ++k)
        {
            allpass[c][k].prepare ((int) std::round (allpassLengths[c][k] * scale));
            if (c == 0) total += (float) allpass[c][k].length();
        }
        shifter[c].prepare (sampleRate);
        dcBlock[c].prepare (sampleRate);
        highpass[c].reset();
        lowpass[c].reset();
        prevWet[c] = 0.0f;
    }
    diffusionDelaySamples = total;

    auto get = [this] (const char* id) { return apvts.getRawParameterValue (id)->load(); };

    for (auto& s : delaySm) { s.reset (sampleRate, 0.08); s.setCurrentAndTargetValue (sampleRate * 0.375); }
    auto init = [this] (juce::SmoothedValue<float>& s, float v, double secs) { s.reset (sampleRate, secs); s.setCurrentAndTargetValue (v); };
    init (feedbackSm, get (mirage::id::feedback), 0.03);
    init (freezeSm,   get (mirage::id::freeze),   0.05);
    init (modDepthSm, get (mirage::id::modDepth), 0.05);
    init (xModSm,     get (mirage::id::xMod),     0.05);
    init (diffuseSm,  get (mirage::id::diffuse),  0.05);
    init (shimmerSm,  get (mirage::id::shimmer),  0.05);
    init (widthSm,    get (mirage::id::width),    0.03);
    init (mixSm,      get (mirage::id::mix),      0.03);
    init (resonanceSm,get (mirage::id::resonance),0.03);
    lowCutSm.reset  (sampleRate, 0.03); lowCutSm.setCurrentAndTargetValue  (get (mirage::id::lowCut));
    highCutSm.reset (sampleRate, 0.03); highCutSm.setCurrentAndTargetValue (get (mirage::id::highCut));
    updateFilters (get (mirage::id::lowCut), get (mirage::id::highCut), get (mirage::id::resonance));
}

bool MirageProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return layouts.getMainInputChannelSet()  == juce::AudioChannelSet::stereo()
        && layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

void MirageProcessor::updateFilters (float lowCutHz, float highCutHz, float resonance)
{
    const float nyq = (float) (0.45 * sampleRate);
    const float gHp = std::tan (3.14159265f * juce::jlimit (10.0f, nyq, lowCutHz)  / (float) sampleRate);
    const float gLp = std::tan (3.14159265f * juce::jlimit (50.0f, nyq, highCutHz) / (float) sampleRate);
    const float kLp = 1.414f * std::pow (0.0141f, resonance);   // 1.414 (flat) -> ~0.02 (self-oscillation)
    for (int c = 0; c < 2; ++c)
    {
        highpass[c].set (gHp, 1.414f);
        lowpass[c].set (gLp, kLp);
    }
}

void MirageProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const int numSamples = buffer.getNumSamples();
    if (buffer.getNumChannels() < 2) return;

    auto get = [this] (const char* id) { return apvts.getRawParameterValue (id)->load(); };

    // Tempo from the host (falls back to 120 BPM)
    double bpm = 120.0;
    if (auto* ph = getPlayHead())
        if (auto pos = ph->getPosition())
            if (auto b = pos->getBpm())
                if (*b > 20.0) bpm = *b;

    const bool dual     = get (mirage::id::mode) < 0.5f;
    const bool sync     = get (mirage::id::sync) > 0.5f;
    const bool pingPong = get (mirage::id::routing) > 0.5f;
    const int  xMode    = (int) get (mirage::id::xModMode);       // 0 time, 1 feedback, 2 both
    const bool xTime    = xMode == 0 || xMode == 2;
    const bool xFeed    = xMode == 1 || xMode == 2;
    const float rate    = get (mirage::id::modRate);

    auto timeSamples = [&] (int division, float freeMs)
    {
        const double s = sync ? mirage::divisionBeats[juce::jlimit (0, 12, division)] * 60.0 / bpm * sampleRate
                              : (double) freeMs * 0.001 * sampleRate;
        return juce::jlimit (4.0f, maxDelaySamples, (float) s);
    };
    const float tL = timeSamples ((int) get (mirage::id::timeL), get (mirage::id::freeL));
    const float tR = dual ? timeSamples ((int) get (mirage::id::timeR), get (mirage::id::freeR)) : tL;
    delaySm[0].setTargetValue (tL);
    delaySm[1].setTargetValue (tR);

    feedbackSm.setTargetValue (get (mirage::id::feedback));
    freezeSm.setTargetValue   (get (mirage::id::freeze) > 0.5f ? 1.0f : 0.0f);
    modDepthSm.setTargetValue (get (mirage::id::modDepth));
    xModSm.setTargetValue     (get (mirage::id::xMod));
    diffuseSm.setTargetValue  (get (mirage::id::diffuse));
    shimmerSm.setTargetValue  (get (mirage::id::shimmer));
    widthSm.setTargetValue    (get (mirage::id::width));
    mixSm.setTargetValue      (get (mirage::id::mix));
    resonanceSm.setTargetValue(get (mirage::id::resonance));
    lowCutSm.setTargetValue   (get (mirage::id::lowCut));
    highCutSm.setTargetValue  (get (mirage::id::highCut));

    const float lfoInc  = rate / (float) sampleRate;
    const float modUnit = 0.005f * (float) sampleRate;            // 5 ms of wobble at full depth
    const float xUnit   = 0.010f * (float) sampleRate;            // 10 ms of cross-mod at full amount

    float* data[2] { buffer.getWritePointer (0), buffer.getWritePointer (1) };

    for (int i = 0; i < numSamples; ++i)
    {
        const float freezeV = freezeSm.getNextValue();
        const float fbBase  = feedbackSm.getNextValue();
        const float depth   = modDepthSm.getNextValue();
        const float xm      = xModSm.getNextValue();
        const float diff    = diffuseSm.getNextValue();
        const float shim    = shimmerSm.getNextValue();
        const float width   = widthSm.getNextValue();
        const float mix     = mixSm.getNextValue();
        const float res     = resonanceSm.getNextValue();
        const float lowCut  = lowCutSm.getNextValue();
        const float highCut = highCutSm.getNextValue();

        if ((i % controlInterval) == 0)
            updateFilters (lowCut, highCut, res * (1.0f - freezeV));   // freeze drops the resonant peak so the loop holds steady

        const float fb     = fbBase + (1.0f - fbBase) * freezeV;  // frozen: loop holds at unity gain
        const float inGain = 1.0f - freezeV;                       // frozen: no new audio enters the loop
        const float apG    = 0.5f + 0.25f * diff;
        const float blend  = std::min (1.0f, diff * 1.5f);         // diffusion amount
        const float comp   = diffusionDelaySamples * blend;        // keeps repeats near their grid position

        // Organic wobble: two sines, right channel a quarter-cycle behind
        auto wobble = [&] (float offset)
        {
            return 0.7f * std::sin (twoPi * (lfoPhase + offset)) + 0.3f * std::sin (twoPi * (lfoPhase2 + offset));
        };
        const float wob[2] { wobble (0.0f), wobble (0.25f) };
        lfoPhase  += lfoInc;         if (lfoPhase  >= 1.0f) lfoPhase  -= 1.0f;
        lfoPhase2 += lfoInc * 0.37f; if (lfoPhase2 >= 1.0f) lfoPhase2 -= 1.0f;

        float wet[2];
        float fbOut[2];
        for (int c = 0; c < 2; ++c)
        {
            const int o = 1 - c;
            float d = delaySm[c].getNextValue() - comp + wob[c] * depth * modUnit;
            if (xTime) d += xm * prevWet[o] * xUnit;

            float y = delay[c].read (juce::jlimit (4.0f, maxDelaySamples, d));

            y = highpass[c].highpass (y);                          // low cut
            y = lowpass[c].lowpass (y);                            // high cut + resonance
            y += shim * 0.8f * shifter[c].process (y);             // shimmer: octave-up feeds back on itself
            y += blend * (allpass[c][3].process (allpass[c][2].process (allpass[c][1].process (allpass[c][0].process (y, apG), apG), apG), apG) - y);
            y = mirage::softClip (y);                              // feedback can never hard clip
            y = dcBlock[c].process (y);

            wet[c] = y;
            float g = fb;
            if (xFeed) g *= 1.0f + (1.0f - freezeV) * xm * prevWet[o] * 1.5f;
            fbOut[c] = y * juce::jlimit (0.0f, 1.3f, g);
        }
        prevWet[0] = wet[0];
        prevWet[1] = wet[1];

        const float inL = data[0][i], inR = data[1][i];
        if (pingPong)
        {
            // mono in -> left line; left feeds right; right feeds back into left
            delay[0].write (0.5f * (inL + inR) * inGain + fbOut[1]);
            delay[1].write (fbOut[0]);
        }
        else
        {
            delay[0].write (inL * inGain + fbOut[0]);
            delay[1].write (inR * inGain + fbOut[1]);
        }

        // Stereo width on the wet signal (mid/side), then equal-power dry/wet
        const float mid  = 0.5f * (wet[0] + wet[1]);
        const float side = 0.5f * (wet[0] - wet[1]) * width;
        const float dryG = std::cos (mix * 1.57079633f);
        const float wetG = std::sin (mix * 1.57079633f);
        data[0][i] = inL * dryG + (mid + side) * wetG;
        data[1][i] = inR * dryG + (mid - side) * wetG;
    }
}

juce::AudioProcessorEditor* MirageProcessor::createEditor()
{
    return new MirageEditor (*this);
}

void MirageProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary (*xml, destData);
}

void MirageProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MirageProcessor();
}
