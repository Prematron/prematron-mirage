#pragma once
#include "PluginProcessor.h"

namespace mirage
{
    // Tactile knobs, glowing value arcs, raised buttons. Everything is drawn as vectors so it stays
    // sharp at any window size (Retina and 4K included).
    class MirageLookAndFeel : public juce::LookAndFeel_V4
    {
    public:
        MirageLookAndFeel();
        void drawRotarySlider (juce::Graphics&, int x, int y, int w, int h, float pos,
                               float startAngle, float endAngle, juce::Slider&) override;
        void drawButtonBackground (juce::Graphics&, juce::Button&, const juce::Colour&, bool, bool) override;
        void drawButtonText (juce::Graphics&, juce::TextButton&, bool, bool) override;
    };

    // Toggle button bound to a bool (or two-choice) parameter. `invert` flips the meaning (used for Dual/Single).
    class ParamButton : public juce::TextButton
    {
    public:
        ParamButton (juce::AudioProcessorValueTreeState&, const char* paramId, const juce::String& text, bool invert = false);
    private:
        bool invert;
        juce::ParameterAttachment attachment;
    };

    // Row of exclusive buttons bound to a choice parameter.
    class SegmentedChoice : public juce::Component
    {
    public:
        SegmentedChoice (juce::AudioProcessorValueTreeState&, const char* paramId, const juce::StringArray& labels);
        void paint (juce::Graphics&) override;
        void resized() override;
    private:
        void select (int index);
        juce::OwnedArray<juce::TextButton> buttons;
        juce::ParameterAttachment attachment;
    };

    struct KnobCell
    {
        juce::Slider slider;
        juce::Label name, value;
        std::unique_ptr<juce::SliderParameterAttachment> attachment;
        void attach (juce::AudioProcessorValueTreeState&, const char* paramId);
        void setActive (bool);
    };
}

class MirageEditor : public juce::AudioProcessorEditor
{
public:
    explicit MirageEditor (MirageProcessor&);
    ~MirageEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void setupKnob (mirage::KnobCell&, const juce::String& title, juce::Colour accent);
    void attachTimeKnobs (bool synced);
    void layoutKnobs (juce::Rectangle<float> area, std::initializer_list<std::pair<mirage::KnobCell*, float>> cells);
    float scale() const { return (float) getWidth() / 1200.0f; }

    MirageProcessor& processor;
    mirage::MirageLookAndFeel laf;
    juce::ComponentBoundsConstrainer constrainer;
    juce::Image logo;

    mirage::KnobCell timeL, timeR, feedback, lowCut, highCut, resonance,
                     modRate, modDepth, xMod, diffuse, shimmer, width, mix;

    mirage::ParamButton dualButton, syncButton, pingPongButton, freezeButton;
    mirage::SegmentedChoice xModTarget;

    juce::ParameterAttachment syncWatcher, modeWatcher;

    juce::Rectangle<float> panels[6];
    juce::Rectangle<float> delayTray;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MirageEditor)
};
