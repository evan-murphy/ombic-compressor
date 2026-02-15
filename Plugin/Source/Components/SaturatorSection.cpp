#include "SaturatorSection.h"
#include "../PluginProcessor.h"
#include "OmbicAssets.h"

SaturatorSection::ScopeComponent::ScopeComponent(juce::Slider& drive, juce::Slider& intensity)
    : driveSlider_(&drive), intensitySlider_(&intensity) {}

void SaturatorSection::ScopeComponent::paint(juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat();
    // Spec §7: background #080a12, border 1px pluginBorder, 10px radius
    g.setColour(juce::Colour(0xFF080a12));
    g.fillRoundedRectangle(b, 10.0f);
    g.setColour(OmbicLookAndFeel::pluginBorder());
    g.drawRoundedRectangle(b.reduced(0.5f), 10.0f, 1.0f);

    float drive = driveSlider_ && intensitySlider_ ? static_cast<float>(driveSlider_->getValue() * 0.5 + intensitySlider_->getValue() * 0.25) : 0.4f;
    float intensity = driveSlider_ && intensitySlider_ ? static_cast<float>(intensitySlider_->getValue()) : 0.45f;
    drive = juce::jlimit(0.0f, 1.0f, drive);

    // §7 Box glow: subtle inner glow on scope
    float boxGlowAlpha = (drive * 0.12f + intensity * 0.06f) * 0.5f;
    g.setColour(juce::Colour(0xFFe85590).withAlpha(boxGlowAlpha));
    g.fillRoundedRectangle(b.reduced(3.0f), 7.0f);

    auto plotArea = b.reduced(8.0f, 6.0f);
    // Center line at 50% height
    g.setColour(juce::Colour(0x0affffff));
    g.drawHorizontalLine(static_cast<int>(plotArea.getCentreY()), plotArea.getX(), plotArea.getRight());

    float phase = static_cast<float>(juce::Time::getMillisecondCounter() % 100000) * 0.0001f;
    const int n = 200;
    juce::Path inputPath;
    juce::Path outputPath;
    for (int i = 0; i <= n; ++i)
    {
        float t = static_cast<float>(i) / static_cast<float>(n);
        float x = plotArea.getX() + t * plotArea.getWidth();
        float yMid = plotArea.getCentreY();
        float amp = plotArea.getHeight() * 0.4f;
        float sine = std::sin(phase * 40.0f + t * 6.0f * juce::MathConstants<float>::pi);
        float inY = yMid - sine * amp;
        float satGain = 1.0f + drive * 3.0f * t;
        float saturated = std::tanh(sine * satGain);
        float outY = yMid - saturated * amp;
        if (i == 0) { inputPath.startNewSubPath(x, inY); outputPath.startNewSubPath(x, outY); }
        else { inputPath.lineTo(x, inY); outputPath.lineTo(x, outY); }
    }
    // Input: pluginMuted 60%, 1.5px
    g.setColour(OmbicLookAndFeel::pluginMuted().withAlpha(0.6f));
    g.strokePath(inputPath, juce::PathStrokeType(1.5f));
    // Glow layer: same path 10px stroke
    g.setColour(OmbicLookAndFeel::ombicPink().withAlpha(drive * 0.25f));
    g.strokePath(outputPath, juce::PathStrokeType(10.0f));
    g.setColour(OmbicLookAndFeel::ombicRed().withAlpha(drive * 0.2f));
    g.strokePath(outputPath, juce::PathStrokeType(6.0f));
    // Saturated waveform: gradient pluginMuted → #e85590 → ombicRed (single stroke blend)
    juce::Colour satCol = OmbicLookAndFeel::pluginMuted().interpolatedWith(juce::Colour(0xFFe85590), 0.5f).interpolatedWith(OmbicLookAndFeel::ombicRed(), 0.3f);
    g.setColour(satCol.withAlpha(0.6f + 0.4f * drive));
    g.strokePath(outputPath, juce::PathStrokeType(2.0f));
}

SaturatorSection::SaturatorSection(OmbicCompressorProcessor& processor)
    : proc(processor)
    , scopeComponent_(driveSlider, intensitySlider)
{
    setLookAndFeel(&ombicLf);
    addAndMakeVisible(scopeComponent_);

    const juce::BorderSize<int> labelPadding(4, 0, 0, 0);
    const juce::Colour textCol = OmbicLookAndFeel::pluginText();
    const juce::Colour labelCol = OmbicLookAndFeel::pluginMuted();
    const juce::Font labelFont = OmbicLookAndFeel::getOmbicFontForPainting(9.0f, true);
    auto percentFromValue = [](double v) { return juce::String(static_cast<int>(v * 100.0 + 0.5)) + "%"; };
    auto valueFromPercent = [](const juce::String& t) {
        return t.trim().trimCharactersAtEnd("%").getFloatValue() / 100.0;
    };
    driveSlider.setName("saturation");
    driveSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
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

    setTooltip("Random gain wobble (like a starved bulb flickering), not a fixed waveshape.");
}

SaturatorSection::~SaturatorSection()
{
    setLookAndFeel(nullptr);
}

bool SaturatorSection::isInteracting() const
{
    return driveSlider.isMouseButtonDown() || intensitySlider.isMouseButtonDown()
        || toneSlider.isMouseButtonDown() || mixSlider.isMouseButtonDown();
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

void SaturatorSection::ensureLogoLoaded()
{
    if (logoImage_.isValid())
        return;
    int logoSize = 0;
    const char* logoData = OmbicAssets::getNamedResource("Ombic_Alpha_png", logoSize);
    if (logoData != nullptr && logoSize > 0)
        logoImage_ = juce::ImageCache::getFromMemory(logoData, logoSize);
}

void SaturatorSection::paint(juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat();
    g.setColour(OmbicLookAndFeel::pluginSurface());
    g.fillRoundedRectangle(b, 16.0f);
    g.setColour(OmbicLookAndFeel::pluginBorder());
    g.drawRoundedRectangle(b, 16.0f, 2.0f);
    auto headerRect = b.removeFromTop(36.0f);
    g.setGradientFill(juce::ColourGradient(
        juce::Colour(0xFFcc3a6e), headerRect.getX(), headerRect.getY(),
        juce::Colour(0xFFe85590), headerRect.getRight(), headerRect.getY(), false));
    g.fillRoundedRectangle(headerRect.withBottom(headerRect.getY() + 36.0f), 16.0f);

    // Neon bulb motif: capsule sized to fit Ombic logo clearly; glow scales with Drive
    const float capW = 20.0f;
    const float capH = 26.0f;
    const float capX = 6.0f;
    const float capY = 1.0f;
    const float capR = capW * 0.5f;
    const float glowPad = 5.0f;
    float drive = static_cast<float>(driveSlider.getValue());  // 0..1, drives glow strength
    drive = juce::jlimit(0.0f, 1.0f, drive);

    juce::Path glowPath;
    glowPath.addRoundedRectangle(capX - glowPad, capY - glowPad,
                                  capW + glowPad * 2.0f, capH + glowPad * 2.0f, capR + 2.0f);
    float glowAlpha = 0.2f + 0.4f * drive;  // more drive → stronger halo
    g.setColour(OmbicLookAndFeel::ombicPink().withAlpha(glowAlpha));
    g.fillPath(glowPath);

    juce::Path capsulePath;
    capsulePath.addRoundedRectangle(capX, capY, capW, capH, capR);
    float fillAlpha = 0.35f + 0.45f * drive;  // more drive → brighter tube
    g.setColour(OmbicLookAndFeel::ombicPink().withAlpha(fillAlpha));
    g.fillPath(capsulePath);
    g.setColour(OmbicLookAndFeel::ink());
    g.strokePath(capsulePath, juce::PathStrokeType(1.2f));

    // Ombic logo inside the capsule (centred, enough room to be recognizable)
    ensureLogoLoaded();
    if (logoImage_.isValid())
    {
        const float pad = 2.0f;
        auto logoArea = juce::Rectangle<float>(capX + pad, capY + pad, capW - pad * 2.0f, capH - pad * 2.0f);
        g.drawImageWithin(logoImage_,
                          static_cast<int>(logoArea.getX()), static_cast<int>(logoArea.getY()),
                          static_cast<int>(logoArea.getWidth()), static_cast<int>(logoArea.getHeight()),
                          juce::RectanglePlacement::centred, false);
    }

    g.setColour(juce::Colours::white);
    g.setFont(OmbicLookAndFeel::getOmbicFontForPainting(13.0f, true));
    g.drawText("NEON BULB SATURATION", static_cast<int>(capX + capW + 8), 8, 200, 20, juce::Justification::left);
}

void SaturatorSection::resized()
{
    auto r = getLocalBounds();
    r.removeFromTop(36);
    r.reduce(14, 14);
    const int scopeH = 110;  // Spec §7
    const int marginBelowScope = 14;
    scopeComponent_.setBounds(r.getX(), r.getY(), r.getWidth(), scopeH);
    r.removeFromTop(scopeH + marginBelowScope);
    const int labelH = 18;
    const int knobSize = 48;
    const int gap = 16;
    const int columnW = knobSize + gap;

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
}
