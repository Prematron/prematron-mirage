#include "PluginEditor.h"
#include "BinaryData.h"

using namespace juce;

namespace
{
    const Colour violet  { 0xffd98cff };
    const Colour pink    { 0xffff7ad9 };
    const Colour ice     { 0xff7fd0ff };
    const Colour teal    { 0xff4fe3d6 };
    const Colour textDim { 0xffa398d4 };
    const Colour textLbl { 0xffbdb2ea };
    const Colour textVal { 0xfff0ebff };

    Font bold (float h, float kerning = 0.0f)
    {
        return Font (FontOptions (h, Font::bold).withKerningFactor (kerning));
    }
}

//==============================================================================
mirage::MirageLookAndFeel::MirageLookAndFeel()
{
    setColour (ResizableWindow::backgroundColourId, Colour (0xff0d0919));
}

void mirage::MirageLookAndFeel::drawRotarySlider (Graphics& g, int x, int y, int w, int h, float pos,
                                                  float startAngle, float endAngle, Slider& slider)
{
    const auto bounds = Rectangle<float> ((float) x, (float) y, (float) w, (float) h).reduced (2.0f);
    const float d = jmin (bounds.getWidth(), bounds.getHeight());
    const auto c = bounds.getCentre();
    const float r = d * 0.5f;
    const Colour accent ((uint32) (int) slider.getProperties()["accent"]);
    const float track = jmax (3.0f, d * 0.085f);
    const float ringR = r - track * 0.5f;
    const float angle = startAngle + pos * (endAngle - startAngle);
    const bool active = slider.isEnabled();

    Path trackPath;
    trackPath.addCentredArc (c.x, c.y, ringR, ringR, 0.0f, startAngle, endAngle, true);
    g.setColour (Colour (0xff2b2444));
    g.strokePath (trackPath, PathStrokeType (track, PathStrokeType::curved, PathStrokeType::rounded));

    if (pos > 0.0f)
    {
        Path value;
        value.addCentredArc (c.x, c.y, ringR, ringR, 0.0f, startAngle, angle, true);
        g.setColour (accent.withAlpha (active ? 0.22f : 0.0f));
        g.strokePath (value, PathStrokeType (track * 2.4f, PathStrokeType::curved, PathStrokeType::rounded));
        g.setColour (accent);
        g.strokePath (value, PathStrokeType (track, PathStrokeType::curved, PathStrokeType::rounded));
    }

    // Cap: soft drop shadow, domed gradient, rim light
    const float capR = r - track - d * 0.07f;
    Path cap;
    cap.addEllipse (c.x - capR, c.y - capR, capR * 2.0f, capR * 2.0f);
    DropShadow (Colours::black.withAlpha (0.65f), roundToInt (d * 0.15f), { 0, roundToInt (d * 0.07f) }).drawForPath (g, cap);

    ColourGradient dome (Colour (0xff62568c), c.x - capR * 0.3f, c.y - capR * 0.5f,
                         Colour (0xff17122b), c.x + capR * 0.6f, c.y + capR * 0.9f, true);
    dome.addColour (0.55, Colour (0xff2a2245));
    g.setGradientFill (dome);
    g.fillPath (cap);
    g.setColour (Colours::white.withAlpha (0.14f));
    g.drawEllipse (c.x - capR + 0.5f, c.y - capR + 0.5f, capR * 2.0f - 1.0f, capR * 2.0f - 1.0f, 1.0f);

    // Indicator
    Path ind;
    ind.addRoundedRectangle (-d * 0.027f, -capR + d * 0.05f, d * 0.054f, capR * 0.4f, d * 0.027f);
    ind.applyTransform (AffineTransform::rotation (angle).translated (c.x, c.y));
    g.setColour (accent.withAlpha (0.45f));
    g.strokePath (ind, PathStrokeType (d * 0.035f));
    g.setColour (Colours::white);
    g.fillPath (ind);
}

void mirage::MirageLookAndFeel::drawButtonBackground (Graphics& g, Button& b, const Colour&, bool highlighted, bool down)
{
    auto r = b.getLocalBounds().toFloat().reduced (1.0f);
    const bool on = b.getToggleState();
    const bool big = b.getProperties().contains ("big");
    const bool seg = b.getProperties().contains ("seg");
    const float rad = big ? r.getHeight() * 0.25f : r.getHeight() * 0.28f;

    if (big)
    {
        Path p;
        p.addRoundedRectangle (r, rad);
        if (on)
        {
            DropShadow (ice.withAlpha (0.55f), roundToInt (r.getHeight() * 0.45f), {}).drawForPath (g, p);
            g.setGradientFill (ColourGradient (Colour (0xff9bf0ff), 0, r.getY(), Colour (0xff3b7bff), 0, r.getBottom(), false));
        }
        else
        {
            DropShadow (Colours::black.withAlpha (0.55f), roundToInt (r.getHeight() * 0.22f), { 0, roundToInt (r.getHeight() * 0.1f) }).drawForPath (g, p);
            g.setGradientFill (ColourGradient (Colour (0xff3a2f5e), 0, r.getY(), Colour (0xff1c1535), 0, r.getBottom(), false));
        }
        g.fillPath (p);
        g.setColour (Colours::white.withAlpha (on ? 0.35f : 0.18f));
        g.drawRoundedRectangle (r, rad, 1.0f);
        if (down) { g.setColour (Colours::black.withAlpha (0.2f)); g.fillRoundedRectangle (r, rad); }
        return;
    }

    if (on)
    {
        g.setGradientFill (ColourGradient (Colour (0xffc26bff), r.getX(), r.getY(), Colour (0xff4fd8e0), r.getRight(), r.getBottom(), false));
        g.fillRoundedRectangle (r, rad);
        g.setColour (Colours::white.withAlpha (0.35f));
        g.drawLine (r.getX() + rad, r.getY() + 1.0f, r.getRight() - rad, r.getY() + 1.0f);
    }
    else if (! seg)
    {
        g.setColour (Colour (0xff1a1330));
        g.fillRoundedRectangle (r, rad);
    }
    if (highlighted && ! on) { g.setColour (Colours::white.withAlpha (0.07f)); g.fillRoundedRectangle (r, rad); }
    if (down) { g.setColour (Colours::black.withAlpha (0.18f)); g.fillRoundedRectangle (r, rad); }
}

void mirage::MirageLookAndFeel::drawButtonText (Graphics& g, TextButton& b, bool, bool)
{
    const bool big = b.getProperties().contains ("big");
    const bool on = b.getToggleState();
    g.setFont (bold ((float) b.getHeight() * (big ? 0.28f : 0.34f), big ? 0.2f : 0.06f));
    g.setColour (on ? Colour (big ? 0xff08162b : 0xff12091f) : Colour (big ? 0xfff0ebff : 0xffcfc6f5));
    g.drawText (b.getButtonText().toUpperCase(), b.getLocalBounds(), Justification::centred, false);
}

//==============================================================================
mirage::ParamButton::ParamButton (AudioProcessorValueTreeState& s, const char* id, const String& label, bool inv)
    : invert (inv),
      attachment (*s.getParameter (id), [this] (float v) { setToggleState ((v > 0.5f) != invert, dontSendNotification); })
{
    setButtonText (label);
    setClickingTogglesState (true);
    onClick = [this] { attachment.setValueAsCompleteGesture ((getToggleState() != invert) ? 1.0f : 0.0f); };
    attachment.sendInitialUpdate();
}

mirage::SegmentedChoice::SegmentedChoice (AudioProcessorValueTreeState& s, const char* id, const StringArray& labels)
    : attachment (*s.getParameter (id), [this] (float v) { select ((int) std::round (v)); })
{
    for (int i = 0; i < labels.size(); ++i)
    {
        auto* b = buttons.add (new TextButton (labels[i]));
        b->getProperties().set ("seg", true);
        b->onClick = [this, i] { attachment.setValueAsCompleteGesture ((float) i); };
        addAndMakeVisible (b);
    }
    attachment.sendInitialUpdate();
}

void mirage::SegmentedChoice::select (int index)
{
    for (int i = 0; i < buttons.size(); ++i)
        buttons[i]->setToggleState (i == index, dontSendNotification);
}

void mirage::SegmentedChoice::paint (Graphics& g)
{
    auto r = getLocalBounds().toFloat();
    g.setColour (Colour (0xff120d24));
    g.fillRoundedRectangle (r, r.getHeight() * 0.28f);
    g.setColour (Colours::black.withAlpha (0.5f));
    g.drawRoundedRectangle (r.reduced (0.5f), r.getHeight() * 0.28f, 1.0f);
}

void mirage::SegmentedChoice::resized()
{
    auto r = getLocalBounds().reduced (jmax (2, getHeight() / 11));
    const int gap = jmax (2, getHeight() / 11);
    const int n = buttons.size();
    const int w = (r.getWidth() - gap * (n - 1)) / jmax (1, n);
    for (int i = 0; i < n; ++i)
        buttons[i]->setBounds (r.getX() + i * (w + gap), r.getY(), w, r.getHeight());
}

void mirage::KnobCell::attach (AudioProcessorValueTreeState& s, const char* id)
{
    attachment.reset();
    auto* p = s.getParameter (id);
    attachment = std::make_unique<SliderParameterAttachment> (*p, slider);
    slider.setDoubleClickReturnValue (true, p->convertFrom0to1 (p->getDefaultValue()));
    value.setText (slider.getTextFromValue (slider.getValue()), dontSendNotification);
}

void mirage::KnobCell::setActive (bool on)
{
    slider.setEnabled (on);
    const float a = on ? 1.0f : 0.35f;
    slider.setAlpha (a); name.setAlpha (a); value.setAlpha (a);
}

//==============================================================================
MirageEditor::MirageEditor (MirageProcessor& p)
    : AudioProcessorEditor (&p),
      processor (p),
      dualButton (p.apvts, mirage::id::mode, "Dual", true),
      syncButton (p.apvts, mirage::id::sync, "Sync"),
      pingPongButton (p.apvts, mirage::id::routing, "Ping-Pong"),
      freezeButton (p.apvts, mirage::id::freeze, "Freeze"),
      xModTarget (p.apvts, mirage::id::xModMode, { "Time", "Fdbk", "Both" }),
      syncWatcher (*p.apvts.getParameter (mirage::id::sync), [this] (float v) { attachTimeKnobs (v > 0.5f); }),
      modeWatcher (*p.apvts.getParameter (mirage::id::mode), [this] (float v) { timeR.setActive (v < 0.5f); })
{
    setLookAndFeel (&laf);
    logo = ImageCache::getFromMemory (BinaryData::Prematron_Logo_png, BinaryData::Prematron_Logo_pngSize);

    setupKnob (timeL,    "Time L",   violet);
    setupKnob (timeR,    "Time R",   violet);
    setupKnob (feedback, "Feedback", pink);
    setupKnob (lowCut,   "Low Cut",  ice);
    setupKnob (highCut,  "High Cut", ice);
    setupKnob (resonance,"Reso",     ice);
    setupKnob (modRate,  "Rate",     violet);
    setupKnob (modDepth, "Depth",    violet);
    setupKnob (xMod,     "X-Mod",    violet);
    setupKnob (diffuse,  "Diffuse",  teal);
    setupKnob (shimmer,  "Shimmer",  teal);
    setupKnob (width,    "Width",    teal);
    setupKnob (mix,      "Mix",      teal);

    auto& s = p.apvts;
    feedback.attach  (s, mirage::id::feedback);
    lowCut.attach    (s, mirage::id::lowCut);
    highCut.attach   (s, mirage::id::highCut);
    resonance.attach (s, mirage::id::resonance);
    modRate.attach   (s, mirage::id::modRate);
    modDepth.attach  (s, mirage::id::modDepth);
    xMod.attach      (s, mirage::id::xMod);
    diffuse.attach   (s, mirage::id::diffuse);
    shimmer.attach   (s, mirage::id::shimmer);
    width.attach     (s, mirage::id::width);
    mix.attach       (s, mirage::id::mix);

    freezeButton.getProperties().set ("big", true);
    for (auto* b : { &dualButton, &syncButton, &pingPongButton, &freezeButton })
        addAndMakeVisible (b);
    addAndMakeVisible (xModTarget);

    syncWatcher.sendInitialUpdate();   // attaches the time knobs (division or free ms)
    modeWatcher.sendInitialUpdate();

    constrainer.setFixedAspectRatio (1200.0 / 640.0);
    constrainer.setSizeLimits (900, 480, 2400, 1280);
    setConstrainer (&constrainer);
    setResizable (true, true);
    setSize (1200, 640);
}

MirageEditor::~MirageEditor()
{
    setLookAndFeel (nullptr);
}

void MirageEditor::setupKnob (mirage::KnobCell& k, const String& title, Colour accent)
{
    k.slider.setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
    k.slider.setTextBoxStyle (Slider::NoTextBox, false, 0, 0);
    k.slider.setRotaryParameters (MathConstants<float>::pi * 1.25f, MathConstants<float>::pi * 2.75f, true);
    k.slider.setMouseDragSensitivity (240);
    k.slider.getProperties().set ("accent", (int) accent.getARGB());
    k.slider.onValueChange = [&k] { k.value.setText (k.slider.getTextFromValue (k.slider.getValue()), dontSendNotification); };

    k.name.setText (title.toUpperCase(), dontSendNotification);
    k.name.setJustificationType (Justification::centred);
    k.name.setColour (Label::textColourId, textLbl);
    k.name.setInterceptsMouseClicks (false, false);
    k.value.setJustificationType (Justification::centred);
    k.value.setColour (Label::textColourId, textVal);
    k.value.setInterceptsMouseClicks (false, false);

    addAndMakeVisible (k.slider);
    addAndMakeVisible (k.name);
    addAndMakeVisible (k.value);
}

void MirageEditor::attachTimeKnobs (bool synced)
{
    timeL.attach (processor.apvts, synced ? mirage::id::timeL : mirage::id::freeL);
    timeR.attach (processor.apvts, synced ? mirage::id::timeR : mirage::id::freeR);
}

void MirageEditor::layoutKnobs (Rectangle<float> area, std::initializer_list<std::pair<mirage::KnobCell*, float>> cells)
{
    const float s = scale();
    const int n = (int) cells.size();
    const float cellW = area.getWidth() / (float) n;
    int i = 0;
    for (auto& [cell, size] : cells)
    {
        const float ks = size * s;
        const float stack = ks + 37.0f * s;
        const float cx = area.getX() + cellW * ((float) i + 0.5f);
        const float top = area.getCentreY() - stack * 0.5f;
        cell->slider.setBounds (Rectangle<float> (cx - ks * 0.5f, top, ks, ks).toNearestInt());
        cell->name.setFont (bold (12.0f * s, 0.12f));
        cell->value.setFont (Font (FontOptions (13.0f * s, Font::plain)));
        cell->name.setBounds (Rectangle<float> (cx - cellW * 0.5f, top + ks + 6.0f * s, cellW, 14.0f * s).toNearestInt());
        cell->value.setBounds (Rectangle<float> (cx - cellW * 0.5f, top + ks + 22.0f * s, cellW, 15.0f * s).toNearestInt());
        ++i;
    }
}

void MirageEditor::resized()
{
    const float s = scale();
    const float widths[3] { 406.0f, 353.0f, 353.0f };
    const float xs[3] { 30.0f, 450.0f, 817.0f };
    const float ys[2] { 136.0f, 388.0f };
    for (int i = 0; i < 6; ++i)
        panels[i] = Rectangle<float> (xs[i % 3] * s, ys[i / 3] * s, widths[i % 3] * s, 238.0f * s);

    auto content = [s] (Rectangle<float> p) { return p.withTrimmedTop (34.0f * s).withTrimmedBottom (16.0f * s).reduced (18.0f * s, 0.0f); };
    const float trayH = 44.0f * s;

    // Delay
    {
        auto c = content (panels[0]);
        delayTray = c.removeFromBottom (trayH);
        c.removeFromBottom (12.0f * s);
        layoutKnobs (c, { { &timeL, 68.0f }, { &timeR, 68.0f } });
        auto t = delayTray.reduced (4.0f * s);
        const float gap = 4.0f * s;
        const float w = (t.getWidth() - gap * 2.0f) / 3.0f;
        dualButton.setBounds     (Rectangle<float> (t.getX(), t.getY(), w, t.getHeight()).toNearestInt());
        syncButton.setBounds     (Rectangle<float> (t.getX() + w + gap, t.getY(), w, t.getHeight()).toNearestInt());
        pingPongButton.setBounds (Rectangle<float> (t.getX() + (w + gap) * 2.0f, t.getY(), w, t.getHeight()).toNearestInt());
    }
    // Feedback
    {
        auto c = content (panels[1]);
        auto left = c.removeFromLeft (c.getWidth() * 0.5f);
        layoutKnobs (left, { { &feedback, 104.0f } });
        freezeButton.setBounds (Rectangle<float> (0.0f, 0.0f, 112.0f * s, 56.0f * s).withCentre (c.getCentre()).toNearestInt());
    }
    // Filter
    layoutKnobs (content (panels[2]), { { &lowCut, 68.0f }, { &highCut, 68.0f }, { &resonance, 68.0f } });
    // Modulation
    {
        auto c = content (panels[3]);
        xModTarget.setBounds (c.removeFromBottom (trayH).toNearestInt());
        c.removeFromBottom (12.0f * s);
        layoutKnobs (c, { { &modRate, 68.0f }, { &modDepth, 68.0f }, { &xMod, 68.0f } });
    }
    // Diffuse
    layoutKnobs (content (panels[4]), { { &diffuse, 68.0f }, { &shimmer, 68.0f } });
    // Output
    layoutKnobs (content (panels[5]), { { &width, 68.0f }, { &mix, 92.0f } });
}

void MirageEditor::paint (Graphics& g)
{
    const float s = scale();
    const float w = (float) getWidth(), h = (float) getHeight();

    g.fillAll (Colour (0xff0d0919));
    g.setGradientFill (ColourGradient (Colour (0x3db53cff), 170.0f * s, 0.0f, Colours::transparentBlack, 170.0f * s + 640.0f * s, 0.0f, true));
    g.fillRect (getLocalBounds());
    g.setGradientFill (ColourGradient (Colour (0x2b28c8d2), 1100.0f * s, h, Colours::transparentBlack, 1100.0f * s - 680.0f * s, h, true));
    g.fillRect (getLocalBounds());

    // Logo, top left: the P mark with PREMATRON directly beneath it
    if (logo.isValid())
    {
        g.setImageResamplingQuality (Graphics::highResamplingQuality);
        const float lh = 96.0f * s;
        g.drawImage (logo, Rectangle<float> (30.0f * s, 20.0f * s, lh * (float) logo.getWidth() / (float) logo.getHeight(), lh),
                     RectanglePlacement::xLeft | RectanglePlacement::yTop);
    }

    // Title: bold, wide-spaced, gradient filled, soft glow
    {
        GlyphArrangement ga;
        ga.addLineOfText (bold (62.0f * s, 0.16f), "MIRAGE", 0.0f, 0.0f);
        Path title;
        ga.createPath (title);
        const auto tb = title.getBounds();
        title.applyTransform (AffineTransform::translation (w * 0.5f - tb.getCentreX(), 62.0f * s - tb.getCentreY()));
        const auto b = title.getBounds();
        DropShadow (Colour (0x66b57bff), roundToInt (22.0f * s), {}).drawForPath (g, title);
        ColourGradient grad (Colour (0xffff7ad9), b.getX(), 0.0f, Colour (0xff4fe3d6), b.getRight(), 0.0f, false);
        grad.addColour (0.5, Colour (0xffb57bff));
        g.setGradientFill (grad);
        g.fillPath (title);

        g.setColour (textDim);
        g.setFont (bold (11.0f * s, 0.5f));
        g.drawText ("PSYCHEDELIC DELAY", Rectangle<float> (0.0f, 94.0f * s, w, 16.0f * s).toNearestInt(), Justification::centred, false);
    }

    g.setColour (Colour (0xff8f84c2));
    g.setFont (bold (11.0f * s, 0.2f));
    g.drawFittedText ("VST3\nAU\nCLAP", Rectangle<float> (w - 30.0f * s - 120.0f * s, 34.0f * s, 120.0f * s, 56.0f * s).toNearestInt(),
                      Justification::centredRight, 3);

    // Panels
    const char* titles[6] { "DELAY", "FEEDBACK", "FILTER", "MODULATION", "DIFFUSE", "OUTPUT" };
    for (int i = 0; i < 6; ++i)
    {
        const auto r = panels[i];
        const float rad = 18.0f * s;
        Path p;
        p.addRoundedRectangle (r, rad);
        DropShadow (Colours::black.withAlpha (0.35f), roundToInt (24.0f * s), { 0, roundToInt (10.0f * s) }).drawForPath (g, p);
        g.setGradientFill (ColourGradient (Colours::white.withAlpha (0.07f), r.getX(), r.getY(), Colours::white.withAlpha (0.02f), r.getRight(), r.getBottom(), false));
        g.fillPath (p);
        g.setColour (Colours::white.withAlpha (0.09f));
        g.drawRoundedRectangle (r.reduced (0.5f), rad, 1.0f);
        g.setColour (textDim);
        g.setFont (bold (12.0f * s, 0.28f));
        g.drawText (titles[i], Rectangle<float> (r.getX() + 18.0f * s, r.getY() + 12.0f * s, r.getWidth() - 36.0f * s, 16.0f * s).toNearestInt(),
                    Justification::centredLeft, false);
    }

    g.setColour (Colour (0xff120d24));
    g.fillRoundedRectangle (delayTray, delayTray.getHeight() * 0.28f);
}
