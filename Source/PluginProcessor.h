#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include "Parameters.h"
#include "DSP/DelayLine.h"
#include "DSP/SVF.h"
#include "DSP/Diffusion.h"
#include "DSP/Utility.h"

class MirageProcessor : public juce::AudioProcessor
{
public:
    MirageProcessor();
    ~MirageProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    using juce::AudioProcessor::processBlock;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Mirage"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 20.0; } // feedback, freeze and shimmer tails

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    juce::AudioProcessorValueTreeState apvts;

private:
    static constexpr int numAllpass = 4;

    double sampleRate = 44100.0;
    float maxDelaySamples = 0.0f;
    float diffusionDelaySamples = 0.0f;   // total allpass chain length, used to keep echo timing honest

    mirage::DelayLine delay[2];
    mirage::SVF highpass[2], lowpass[2];
    mirage::Allpass allpass[2][numAllpass];
    mirage::OctaveShifter shifter[2];
    mirage::DcBlocker dcBlock[2];

    juce::SmoothedValue<float> delaySm[2];
    juce::SmoothedValue<float> feedbackSm, freezeSm, modDepthSm, xModSm, diffuseSm, shimmerSm, widthSm, mixSm, resonanceSm;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Multiplicative> lowCutSm, highCutSm;

    float lfoPhase = 0.0f, lfoPhase2 = 0.0f;
    float prevWet[2] { 0.0f, 0.0f };

    void updateFilters (float lowCutHz, float highCutHz, float resonance);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MirageProcessor)
};
