#include "SaturatorSection.h"
#include "../PluginProcessor.h"
#include <vector>

SaturatorSection::ScopeComponent::ScopeComponent(OmbicCompressorProcessor& processor, juce::Slider& drive, juce::Slider& intensity,
                                                   juce::Slider& tone, juce::Slider& mix)
    : proc_(&processor), driveSlider_(&drive), intensitySlider_(&intensity), toneSlider_(&tone), mixSlider_(&mix) {}

void SaturatorSection::ScopeComponent::paint(juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat();
    // NEON_BULB_GUI_SPEC: background #080a12, border 1px pluginBorder, 10px radius
    g.setColour(juce::Colour(0xFF080a12));
    g.fillRoundedRectangle(b, 10.0f);
    g.setColour(OmbicLookAndFeel::pluginBorder());
    g.drawRoundedRectangle(b.reduced(0.5f), 10.0f, 1.0f);

    auto plotArea = b.reduced(8.0f, 6.0f);

    // When SC Listen is on, show real sidechain signal (teal); otherwise tube + filament (Option 5)
    std::vector<float> sidechainSamples;
    if (proc_ && proc_->isScListenActive() && proc_->getScopeSidechainSamples(sidechainSamples) && sidechainSamples.size() > 1)
    {
        g.setColour(OmbicLookAndFeel::ombicTeal().withAlpha(0.04f));
        g.fillRoundedRectangle(b.reduced(3.0f), 7.0f);

        juce::Path path;
        const size_t n = sidechainSamples.size();
        const float w = plotArea.getWidth();
        const float yMid = plotArea.getCentreY();
        const float amp = plotArea.getHeight() * 0.4f;
        float peak = 1e-6f;
        for (size_t i = 0; i < n; ++i)
            peak = juce::jmax(peak, std::abs(sidechainSamples[i]));
        const float scale = (peak > 1e-6f) ? (amp / peak) : amp;
        for (size_t i = 0; i < n; ++i)
        {
            float t = static_cast<float>(i) / static_cast<float>(n - 1);
            float x = plotArea.getX() + t * w;
            float y = yMid - sidechainSamples[i] * scale;
            if (i == 0)
                path.startNewSubPath(x, y);
            else
                path.lineTo(x, y);
        }
        g.setColour(OmbicLookAndFeel::ombicTeal().withAlpha(0.2f));
        g.strokePath(path, juce::PathStrokeType(8.0f));
        g.setColour(OmbicLookAndFeel::ombicTeal().withAlpha(0.85f));
        g.strokePath(path, juce::PathStrokeType(2.0f));
        return;
    }

    // --- Option 5 Tube + filament (NEON_BULB_GUI_SPEC) ---
    const float drive = driveSlider_ ? static_cast<float>(driveSlider_->getValue()) : 0.45f;
    const float intensity = intensitySlider_ ? static_cast<float>(intensitySlider_->getValue()) : 0.5f;
    const float mix = mixSlider_ ? static_cast<float>(mixSlider_->getValue()) : 1.0f;

    // Less compact: use more of the area (smaller padding, taller tube, more vertical range)
    const float tubeH = 42.0f;
    const float tubeRadius = tubeH * 0.5f;
    const float tubeY = plotArea.getCentreY() - tubeRadius;
    const float tubeX = plotArea.getX() + 4.0f;
    const float tubeW = plotArea.getWidth() - 8.0f;
    const float yMid = plotArea.getCentreY();
    const float amp = 18.0f;

    // Tube outline: horizontal pill, stroke only (pluginBorder, ~0.6 alpha)
    g.setColour(OmbicLookAndFeel::pluginBorder().withAlpha(0.6f));
    g.drawRoundedRectangle(tubeX, tubeY, tubeW, tubeH, tubeRadius, 1.5f);

    juce::Path tubeClipPath;
    tubeClipPath.addRoundedRectangle(tubeX, tubeY, tubeW, tubeH, tubeRadius);

    // Neon pink for filament (never use teal/blue here — this is the saturation display)
    const juce::Colour neonPink(0xFFe85590);

    // Below this peak we treat as silence and show a calm idle (avoids amplifying noise into crazy display)
    const float kSilenceThreshold = 1e-4f;

    juce::Path filamentPath;
    std::vector<float> waveformSamples;
    bool hasRealSignal = false;
    float peak = 0.0f;
    if (proc_ && proc_->getScopeWaveformSamples(waveformSamples) && waveformSamples.size() > 1)
    {
        const size_t n = waveformSamples.size();
        for (size_t i = 0; i < n; ++i)
            peak = juce::jmax(peak, std::abs(waveformSamples[i]));
        hasRealSignal = (peak >= kSilenceThreshold);
    }

    if (hasRealSignal)
    {
        // Real waveform: normalize by peak, apply display drive, optional light smoothing to reduce noise jitter
        const size_t n = waveformSamples.size();
        const float displayGain = 1.0f + drive * 4.0f;
        const float displayNorm = std::tanh(displayGain);
        const int smoothHalf = 1;  // 3-point running average to reduce noise jitter
        for (size_t i = 0; i < n; ++i)
        {
            float s = waveformSamples[i] / (peak + 1e-9f);
            if (smoothHalf > 0 && n > static_cast<size_t>(smoothHalf * 2))
            {
                float sum = 0.0f;
                int count = 0;
                for (int d = -smoothHalf; d <= smoothHalf; ++d)
                {
                    int j = static_cast<int>(i) + d;
                    if (j >= 0 && j < static_cast<int>(n))
                    {
                        sum += waveformSamples[static_cast<size_t>(j)] / (peak + 1e-9f);
                        ++count;
                    }
                }
                s = count > 0 ? sum / static_cast<float>(count) : s;
            }
            float displayed = std::tanh(s * displayGain) / displayNorm;
            float t = static_cast<float>(i) / static_cast<float>(n - 1);
            float x = tubeX + t * tubeW;
            float yPos = yMid - displayed * amp;
            if (i == 0)
                filamentPath.startNewSubPath(x, yPos);
            else
                filamentPath.lineTo(x, yPos);
        }
    }
    else
    {
        // No signal or silence: calm idle — flat line with a very gentle wave so it doesn't look dead
        const float phase = static_cast<float>(juce::Time::getMillisecondCounter() % 100000) * 0.00008f;
        const int nPts = 80;
        const float idleAmp = 0.08f;  // subtle movement only
        for (int i = 0; i <= nPts; ++i)
        {
            const float xNorm = static_cast<float>(i) / static_cast<float>(nPts);
            const float x = tubeX + xNorm * tubeW;
            const float y = std::sin(juce::MathConstants<float>::pi * xNorm + phase) * idleAmp;
            const float yPos = yMid - y * amp;
            if (i == 0)
                filamentPath.startNewSubPath(x, yPos);
            else
                filamentPath.lineTo(x, yPos);
        }
    }

    // Mix = dry/wet visibility: 0 = off, 1 = full. Intensity = glow strength + stroke thickness (spec formulas).
    const float lineOpacity = mix;
    const float glowOpacity = mix * (0.05f + 0.95f * intensity);
    const float glowStrokeWidth = 1.0f + 38.0f * intensity;
    const float lineStrokeWidth = 0.4f + 4.0f * intensity;

    // Confine waveform to the tube so it never spills outside the pill
    g.saveState();
    g.reduceClipRegion(tubeClipPath);

    if (glowOpacity > 0.001f)
    {
        g.setColour(neonPink.withAlpha(glowOpacity));
        g.strokePath(filamentPath, juce::PathStrokeType(glowStrokeWidth));
    }
    if (lineOpacity > 0.001f)
    {
        g.setColour(neonPink.withAlpha(lineOpacity));
        g.strokePath(filamentPath, juce::PathStrokeType(lineStrokeWidth));
    }

    g.restoreState();
}

SaturatorSection::SaturatorSection(OmbicCompressorProcessor& processor)
    : proc(processor)
    , scopeComponent_(processor, driveSlider, intensitySlider, toneSlider, mixSlider)
{
    setLookAndFeel(&ombicLf);
    addAndMakeVisible(scopeComponent_);

    const juce::BorderSize<int> labelPadding(4, 0, 0, 0);
    const juce::Colour textCol = OmbicLookAndFeel::pluginText();
    const juce::Colour labelCol = OmbicLookAndFeel::pluginMuted();
    const juce::Font labelFont = OmbicLookAndFeel::getOmbicFontForPainting(9.0f, true);
    // UX: show 0–100% for 0–1 parameters so users see "50%" not "0.5" (human-readable, not binary)
    auto percentFromValue = [](double v) { return juce::String(static_cast<int>(v * 100.0 + 0.5)) + "%"; };
    auto valueFromPercent = [](const juce::String& t) {
        return t.trim().trimCharactersAtEnd("%").getFloatValue() / 100.0;
    };
    driveSlider.setName("saturation");
    driveSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    driveSlider.setRotaryParameters(juce::Slider::RotaryParameters{ -2.356f, 2.356f, true });
    driveSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xFFe85590));
    driveSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 52, 18);
    driveSlider.setColour(juce::Slider::textBoxTextColourId, textCol);
    driveSlider.setVelocityBasedMode(false);
    driveSlider.textFromValueFunction = [percentFromValue](double v) { return percentFromValue(v); };
    driveSlider.valueFromTextFunction = [valueFromPercent](const juce::String& t) { return valueFromPercent(t); };
    driveSlider.setNumDecimalPlacesToDisplay(0);
    addAndMakeVisible(driveSlider);
    driveLabel.setText("DRIVE", juce::dontSendNotification);
    driveLabel.setBorderSize(labelPadding);
    driveLabel.setColour(juce::Label::textColourId, labelCol);
    driveLabel.setFont(labelFont);
    addAndMakeVisible(driveLabel);

    intensitySlider.setName("saturation");
    intensitySlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    intensitySlider.setRotaryParameters(juce::Slider::RotaryParameters{ -2.356f, 2.356f, true });
    intensitySlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xFFe85590));
    intensitySlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 52, 18);
    intensitySlider.setColour(juce::Slider::textBoxTextColourId, textCol);
    intensitySlider.setVelocityBasedMode(false);
    intensitySlider.textFromValueFunction = [percentFromValue](double v) { return percentFromValue(v); };
    intensitySlider.valueFromTextFunction = [valueFromPercent](const juce::String& t) { return valueFromPercent(t); };
    intensitySlider.setNumDecimalPlacesToDisplay(0);
    addAndMakeVisible(intensitySlider);
    intensityLabel.setText("INTENSITY", juce::dontSendNotification);
    intensityLabel.setBorderSize(labelPadding);
    intensityLabel.setColour(juce::Label::textColourId, labelCol);
    intensityLabel.setFont(labelFont);
    addAndMakeVisible(intensityLabel);

    toneSlider.setName("saturation");
    toneSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    toneSlider.setRotaryParameters(juce::Slider::RotaryParameters{ -2.356f, 2.356f, true });
    toneSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xFFe85590));
    toneSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 52, 18);
    toneSlider.setColour(juce::Slider::textBoxTextColourId, textCol);
    toneSlider.setVelocityBasedMode(false);
    toneSlider.textFromValueFunction = [percentFromValue](double v) { return percentFromValue(v); };
    toneSlider.valueFromTextFunction = [valueFromPercent](const juce::String& t) { return valueFromPercent(t); };
    toneSlider.setNumDecimalPlacesToDisplay(0);
    addAndMakeVisible(toneSlider);
    toneLabel.setText("TONE", juce::dontSendNotification);
    toneLabel.setBorderSize(labelPadding);
    toneLabel.setColour(juce::Label::textColourId, labelCol);
    toneLabel.setFont(labelFont);
    addAndMakeVisible(toneLabel);

    mixSlider.setName("saturation");
    mixSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    mixSlider.setRotaryParameters(juce::Slider::RotaryParameters{ -2.356f, 2.356f, true });
    mixSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xFFe85590));
    mixSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 52, 18);
    mixSlider.setColour(juce::Slider::textBoxTextColourId, textCol);
    mixSlider.setVelocityBasedMode(false);
    mixSlider.textFromValueFunction = [percentFromValue](double v) { return percentFromValue(v); };
    mixSlider.valueFromTextFunction = [valueFromPercent](const juce::String& t) { return valueFromPercent(t); };
    mixSlider.setNumDecimalPlacesToDisplay(0);
    addAndMakeVisible(mixSlider);
    mixLabel.setText("MIX", juce::dontSendNotification);
    mixLabel.setBorderSize(labelPadding);
    mixLabel.setColour(juce::Label::textColourId, labelCol);
    mixLabel.setFont(labelFont);
    addAndMakeVisible(mixLabel);

    // MVP: Burstiness 0–10 (integer display)
    burstinessSlider.setName("neon_burstiness");
    burstinessSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    burstinessSlider.setRotaryParameters(juce::Slider::RotaryParameters{ -2.356f, 2.356f, true });
    burstinessSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xFFe85590));
    burstinessSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 44, 16);
    burstinessSlider.setColour(juce::Slider::textBoxTextColourId, textCol);
    burstinessSlider.setVelocityBasedMode(false);
    addAndMakeVisible(burstinessSlider);
    burstinessLabel.setText("BURST", juce::dontSendNotification);
    burstinessLabel.setBorderSize(labelPadding);
    burstinessLabel.setColour(juce::Label::textColourId, labelCol);
    burstinessLabel.setFont(labelFont);
    addAndMakeVisible(burstinessLabel);

    // MVP: G Min 0.85–1.0 (show as 85–100%)
    gMinSlider.setName("neon_g_min");
    gMinSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    gMinSlider.setRotaryParameters(juce::Slider::RotaryParameters{ -2.356f, 2.356f, true });
    gMinSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xFFe85590));
    gMinSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 44, 16);
    gMinSlider.setColour(juce::Slider::textBoxTextColourId, textCol);
    gMinSlider.setVelocityBasedMode(false);
    auto gMinFromValue = [](double v) { return juce::String(static_cast<int>(v * 100.0 + 0.5)) + "%"; };
    auto gMinValueFrom = [](const juce::String& t) { return t.trim().trimCharactersAtEnd("%").getFloatValue() / 100.0; };
    gMinSlider.textFromValueFunction = [gMinFromValue](double v) { return gMinFromValue(v); };
    gMinSlider.valueFromTextFunction = [gMinValueFrom](const juce::String& t) { return gMinValueFrom(t); };
    gMinSlider.setNumDecimalPlacesToDisplay(0);
    addAndMakeVisible(gMinSlider);
    gMinLabel.setText("G MIN", juce::dontSendNotification);
    gMinLabel.setBorderSize(labelPadding);
    gMinLabel.setColour(juce::Label::textColourId, labelCol);
    gMinLabel.setFont(labelFont);
    addAndMakeVisible(gMinLabel);

    // MVP: Soft Sat After toggle
    softSatAfterButton.setName("neon_saturation_after");
    softSatAfterButton.setButtonText("Soft Sat After");
    softSatAfterButton.setColour(juce::ToggleButton::tickColourId, juce::Colour(0xFFe85590));
    addAndMakeVisible(softSatAfterButton);

    // MVP: Neon On toggle (bypass when off)
    neonEnableButton.setName("neon_enable");
    neonEnableButton.setButtonText("On");
    neonEnableButton.setColour(juce::ToggleButton::tickColourId, juce::Colour(0xFFe85590));
    addAndMakeVisible(neonEnableButton);
}

void SaturatorSection::applyPercentDisplay()
{
    auto percentFromValue = [](double v) { return juce::String(static_cast<int>(v * 100.0 + 0.5)) + "%"; };
    auto valueFromPercent = [](const juce::String& t) {
        return t.trim().trimCharactersAtEnd("%").getFloatValue() / 100.0;
    };
    for (juce::Slider* sl : { &driveSlider, &intensitySlider, &toneSlider, &mixSlider })
    {
        sl->textFromValueFunction = [percentFromValue](double v) { return percentFromValue(v); };
        sl->valueFromTextFunction = [valueFromPercent](const juce::String& t) { return valueFromPercent(t); };
        sl->setNumDecimalPlacesToDisplay(0);
    }
    auto gMinFromValue = [](double v) { return juce::String(static_cast<int>(v * 100.0 + 0.5)) + "%"; };
    auto gMinValueFrom = [](const juce::String& t) { return t.trim().trimCharactersAtEnd("%").getFloatValue() / 100.0; };
    gMinSlider.textFromValueFunction = [gMinFromValue](double v) { return gMinFromValue(v); };
    gMinSlider.valueFromTextFunction = [gMinValueFrom](const juce::String& t) { return gMinValueFrom(t); };
    gMinSlider.setNumDecimalPlacesToDisplay(0);
}

SaturatorSection::~SaturatorSection()
{
    setLookAndFeel(nullptr);
}

bool SaturatorSection::isInteracting() const
{
    return driveSlider.isMouseButtonDown() || intensitySlider.isMouseButtonDown()
        || toneSlider.isMouseButtonDown() || mixSlider.isMouseButtonDown()
        || burstinessSlider.isMouseButtonDown() || gMinSlider.isMouseButtonDown();
}

void SaturatorSection::setHighlight(bool on)
{
    if (highlighted_ != on) { highlighted_ = on; repaint(); }
}

void SaturatorSection::mouseEnter(const juce::MouseEvent&)
{
    if (!hovered_) { hovered_ = true; repaint(); }
}

void SaturatorSection::mouseExit(const juce::MouseEvent&)
{
    if (hovered_) { hovered_ = false; repaint(); }
}

void SaturatorSection::paint(juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat();
    g.setColour(OmbicLookAndFeel::pluginSurface());
    g.fillRoundedRectangle(b, 16.0f);
    g.setColour(OmbicLookAndFeel::pluginBorder());
    g.drawRoundedRectangle(b, 16.0f, 2.0f);
    const float headerH = getHeight() < 110 ? 22.0f : 36.0f;  // §6 header 36px
    auto headerRect = b.removeFromTop(headerH);
    g.setGradientFill(juce::ColourGradient(
        juce::Colour(0xFFcc3a6e), headerRect.getX(), headerRect.getY(),
        juce::Colour(0xFFe85590), headerRect.getRight(), headerRect.getY(), false));
    g.fillRoundedRectangle(headerRect.withBottom(headerRect.getY() + headerH), 16.0f);

    g.setColour(juce::Colours::white);
    g.setFont(OmbicLookAndFeel::getOmbicFontForPainting(13.0f, true));  // §7 Module headers 13px
    // §5 Neon section: "NEON" or "NEON SATURATION" when knobs only (v2)
    g.drawText(scopeVisible_ ? "NEON BULB SATURATION" : "NEON", static_cast<int>(headerRect.getX()) + 12, static_cast<int>((headerH - 13.0f) * 0.5f), 220, 14, juce::Justification::left);
}

void SaturatorSection::setScopeVisible(bool visible)
{
    if (scopeVisible_ == visible) return;
    scopeVisible_ = visible;
    scopeComponent_.setVisible(visible);
    resized();
}

void SaturatorSection::resized()
{
    auto r = getLocalBounds();
    const bool compact = (r.getHeight() < 110);
    const int headerH = compact ? 22 : 36;  // §6 Module card header 36px
    r.removeFromTop(headerH);
    r.reduce(compact ? 8 : 14, compact ? 8 : 14);
    const int labelH = compact ? 6 : 14;
    // §6 Neon knobs 48px, gap 14px; when scope visible use larger for v1
    const int knobSize = compact ? 36 : (scopeVisible_ ? 72 : 48);
    const int gap = compact ? 4 : (scopeVisible_ ? 14 : 14);
    const int columnW = knobSize + gap;

    if (scopeVisible_)
    {
        const int scopeH = compact ? 22 : 90;
        const int marginBelowScope = compact ? 2 : 12;
        scopeComponent_.setBounds(r.getX(), r.getY(), r.getWidth(), scopeH);
        r.removeFromTop(scopeH + marginBelowScope);
    }

    int x = r.getX();
    auto place = [&](juce::Slider& sl, juce::Label& lb) {
        lb.setBounds(x, r.getY(), columnW, labelH);
        sl.setBounds(x, r.getY() + labelH, knobSize, knobSize);
        x += columnW;
    };
    place(driveSlider, driveLabel);
    place(intensitySlider, intensityLabel);
    place(toneSlider, toneLabel);
    place(mixSlider, mixLabel);

    // MVP: second row — Burstiness, G Min, then toggles (smaller knobs when space is tight)
    const int row2Y = r.getY() + labelH + knobSize + (compact ? 6 : 12);
    const int knob2Size = compact ? 28 : (scopeVisible_ ? 48 : 40);
    const int column2W = knob2Size + (compact ? 4 : 8);
    int x2 = r.getX();
    burstinessLabel.setBounds(x2, row2Y, column2W, labelH);
    burstinessSlider.setBounds(x2, row2Y + labelH, knob2Size, knob2Size);
    x2 += column2W;
    gMinLabel.setBounds(x2, row2Y, column2W, labelH);
    gMinSlider.setBounds(x2, row2Y + labelH, knob2Size, knob2Size);
    x2 += column2W;
    const int toggleH = compact ? 18 : 22;
    const int toggleW = scopeVisible_ ? 100 : 72;
    softSatAfterButton.setBounds(x2, row2Y, toggleW, toggleH);
    x2 += toggleW + (compact ? 4 : 8);
    neonEnableButton.setBounds(x2, row2Y, 36, toggleH);
}
