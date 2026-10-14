#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

namespace mirage::id
{
    // Delay
    inline constexpr auto mode      = "mode";        // Dual / Single
    inline constexpr auto sync      = "sync";        // tempo sync on/off
    inline constexpr auto timeL     = "timeL";       // synced division, left
    inline constexpr auto timeR     = "timeR";       // synced division, right
    inline constexpr auto freeL     = "freeL";       // free time ms, left
    inline constexpr auto freeR     = "freeR";       // free time ms, right
    inline constexpr auto routing   = "routing";     // Normal / Ping-Pong
    // Feedback
    inline constexpr auto feedback  = "feedback";
    inline constexpr auto freeze    = "freeze";
    // Filter
    inline constexpr auto lowCut    = "lowCut";
    inline constexpr auto highCut   = "highCut";
    inline constexpr auto resonance = "resonance";
    // Modulation
    inline constexpr auto modRate   = "modRate";
    inline constexpr auto modDepth  = "modDepth";
    inline constexpr auto xMod      = "xMod";
    inline constexpr auto xModMode  = "xModMode";    // Time / Feedback / Both
    // Diffuse
    inline constexpr auto diffuse   = "diffuse";
    inline constexpr auto shimmer   = "shimmer";
    // Output
    inline constexpr auto width     = "width";
    inline constexpr auto mix       = "mix";
}

namespace mirage
{
    inline const juce::StringArray divisions {
        "1 bar", "1/2", "1/2 D", "1/2 T", "1/4", "1/4 D", "1/4 T",
        "1/8", "1/8 D", "1/8 T", "1/16", "1/16 D", "1/16 T" };

    // Length of each division in quarter notes (beats), same order as above.
    inline constexpr double divisionBeats[] {
        4.0, 2.0, 3.0, 4.0 / 3.0, 1.0, 1.5, 2.0 / 3.0,
        0.5, 0.75, 1.0 / 3.0, 0.25, 0.375, 1.0 / 6.0 };

    inline juce::AudioProcessorValueTreeState::ParameterLayout createLayout()
    {
        using juce::AudioParameterBool;
        using juce::AudioParameterChoice;
        using juce::AudioParameterFloat;
        using juce::NormalisableRange;
        using juce::ParameterID;

        constexpr int v = 1; // parameter version hint for hosts

        auto pct = [] (float value, int) { return juce::String (juce::roundToInt (value * 100.0f)) + " %"; };
        auto hz  = [] (float value, int) { return value >= 1000.0f ? juce::String (value / 1000.0f, 1) + " kHz"
                                                                  : juce::String (juce::roundToInt (value)) + " Hz"; };
        auto ms  = [] (float value, int) { return juce::String (juce::roundToInt (value)) + " ms"; };

        juce::AudioProcessorValueTreeState::ParameterLayout layout;

        // Delay
        layout.add (std::make_unique<AudioParameterChoice> (ParameterID { id::mode, v }, "Mode",
                                                            juce::StringArray { "Dual", "Single" }, 0));
        layout.add (std::make_unique<AudioParameterBool>   (ParameterID { id::sync, v }, "Sync", true));
        layout.add (std::make_unique<AudioParameterChoice> (ParameterID { id::timeL, v }, "Time L", divisions, 7));
        layout.add (std::make_unique<AudioParameterChoice> (ParameterID { id::timeR, v }, "Time R", divisions, 5));
        layout.add (std::make_unique<AudioParameterFloat>  (ParameterID { id::freeL, v }, "Free Time L",
                                                            NormalisableRange<float> (1.0f, 2000.0f, 0.1f, 0.35f), 375.0f,
                                                            juce::AudioParameterFloatAttributes().withStringFromValueFunction (ms)));
        layout.add (std::make_unique<AudioParameterFloat>  (ParameterID { id::freeR, v }, "Free Time R",
                                                            NormalisableRange<float> (1.0f, 2000.0f, 0.1f, 0.35f), 500.0f,
                                                            juce::AudioParameterFloatAttributes().withStringFromValueFunction (ms)));
        layout.add (std::make_unique<AudioParameterChoice> (ParameterID { id::routing, v }, "Routing",
                                                            juce::StringArray { "Normal", "Ping-Pong" }, 0));
        // Feedback
        layout.add (std::make_unique<AudioParameterFloat>  (ParameterID { id::feedback, v }, "Feedback",
                                                            NormalisableRange<float> (0.0f, 1.1f, 0.001f), 0.5f,
                                                            juce::AudioParameterFloatAttributes().withStringFromValueFunction (pct)));
        layout.add (std::make_unique<AudioParameterBool>   (ParameterID { id::freeze, v }, "Freeze", false));
        // Filter
        layout.add (std::make_unique<AudioParameterFloat>  (ParameterID { id::lowCut, v }, "Low Cut",
                                                            NormalisableRange<float> (20.0f, 2000.0f, 0.0f, 0.4f), 20.0f,
                                                            juce::AudioParameterFloatAttributes().withStringFromValueFunction (hz)));
        layout.add (std::make_unique<AudioParameterFloat>  (ParameterID { id::highCut, v }, "High Cut",
                                                            NormalisableRange<float> (500.0f, 20000.0f, 0.0f, 0.4f), 20000.0f,
                                                            juce::AudioParameterFloatAttributes().withStringFromValueFunction (hz)));
        layout.add (std::make_unique<AudioParameterFloat>  (ParameterID { id::resonance, v }, "Resonance",
                                                            NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.2f,
                                                            juce::AudioParameterFloatAttributes().withStringFromValueFunction (pct)));
        // Modulation
        layout.add (std::make_unique<AudioParameterFloat>  (ParameterID { id::modRate, v }, "Mod Rate",
                                                            NormalisableRange<float> (0.05f, 10.0f, 0.0f, 0.4f), 0.8f,
                                                            juce::AudioParameterFloatAttributes().withLabel ("Hz")));
        layout.add (std::make_unique<AudioParameterFloat>  (ParameterID { id::modDepth, v }, "Mod Depth",
                                                            NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.3f,
                                                            juce::AudioParameterFloatAttributes().withStringFromValueFunction (pct)));
        layout.add (std::make_unique<AudioParameterFloat>  (ParameterID { id::xMod, v }, "Cross-Mod",
                                                            NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.0f,
                                                            juce::AudioParameterFloatAttributes().withStringFromValueFunction (pct)));
        layout.add (std::make_unique<AudioParameterChoice> (ParameterID { id::xModMode, v }, "Cross-Mod Target",
                                                            juce::StringArray { "Time", "Feedback", "Both" }, 2));
        // Diffuse
        layout.add (std::make_unique<AudioParameterFloat>  (ParameterID { id::diffuse, v }, "Diffuse",
                                                            NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.0f,
                                                            juce::AudioParameterFloatAttributes().withStringFromValueFunction (pct)));
        layout.add (std::make_unique<AudioParameterFloat>  (ParameterID { id::shimmer, v }, "Shimmer",
                                                            NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.0f,
                                                            juce::AudioParameterFloatAttributes().withStringFromValueFunction (pct)));
        // Output
        layout.add (std::make_unique<AudioParameterFloat>  (ParameterID { id::width, v }, "Width",
                                                            NormalisableRange<float> (0.0f, 2.0f, 0.001f), 1.0f,
                                                            juce::AudioParameterFloatAttributes().withStringFromValueFunction (pct)));
        layout.add (std::make_unique<AudioParameterFloat>  (ParameterID { id::mix, v }, "Mix",
                                                            NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.35f,
                                                            juce::AudioParameterFloatAttributes().withStringFromValueFunction (pct)));
        return layout;
    }
}
