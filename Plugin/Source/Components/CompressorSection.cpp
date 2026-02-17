#include "CompressorSection.h"
#include "../PluginProcessor.h"

//==============================================================================
CompressorSection::GainReductionMeterComponent::GainReductionMeterComponent(OmbicCompressorProcessor& p)
    : processor(p) {}

void CompressorSection::GainReductionMeterComponent::paint(juce::Graphics& g)
{
    float grDb = processor.gainReductionDb.load();
    smoothedGrDb_ += 0.12f * (grDb - smoothedGrDb_); // ~300 ms ballistics
    const float grFullScaleDb = 40.0f;  // 40 dB = full scale so meter doesn't peg with heavy compression
    float norm = juce::jlimit(0.0f, 1.0f, smoothedGrDb_ / grFullScaleDb);
    auto b = getLocalBounds().toFloat();
    g.setColour(OmbicLookAndFeel::line());
    g.fillRoundedRectangle(b, 4.0f);
    g.setColour(OmbicLookAndFeel::ombicRed());
    g.fillRoundedRectangle(b.removeFromBottom(b.getHeight() * norm), 4.0f);
    g.setColour(OmbicLookAndFeel::ink());
    g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(0.5f), 4.0f, 2.0f);
}

//==============================================================================
CompressorSection::CompressorSection(OmbicCompressorProcessor& processor)
    : proc(processor)
    , grMeter(processor)
{
    setLookAndFeel(&ombicLf);

    modeCombo.addItem("Opto", 1);
    modeCombo.addItem("FET", 2);
    modeCombo.addItem("PWM", 3);
    modeCombo.setSelectedId(1); // will be overwritten by attachment
    addAndMakeVisible(modeCombo);
    modeCombo.setVisible(false); // editor uses pills instead
    modeLabel.setText("Mode", juce::dontSendNotification);
    modeLabel.attachToComponent(&modeCombo, true);
    modeLabel.setVisible(false);

    compressLimitCombo.addItem("Compress", 1);
    compressLimitCombo.addItem("Limit", 2);
    compressLimitCombo.setSelectedId(1);
    compressLimitCombo.setTooltip("Opto only: Compress = normal. Limit = more high-frequency in sidechain (tighter).");
    addChildComponent(compressLimitCombo); // kept for attachment; UI uses toggle
    compressLimitCombo.setVisible(false);
    compressLimitLabel.setVisible(false);
    compressLimitLabel.setText("COMPRESS / LIMIT", juce::dontSendNotification);
    compressLimitLabel.attachToComponent(&compressLimitCombo, true);
    addChildComponent(compressLimitLabel);

    compressLimitToggle_.setName("compressLimitSwitch");
    compressLimitToggle_.setClickingTogglesState(true);
    compressLimitToggle_.setTooltip("Opto only: Compress = normal. Limit = more high-frequency in sidechain (tighter).");
    compressLimitToggle_.onClick = [this]() {
        compressLimitCombo.setSelectedId(compressLimitToggle_.getToggleState() ? 2 : 1, juce::sendNotificationSync);
    };
    addAndMakeVisible(compressLimitToggle_);

    const juce::BorderSize<int> labelPadding(4, 0, 0, 0);
    const juce::Colour textCol = OmbicLookAndFeel::pluginText();
    const juce::Colour labelCol = OmbicLookAndFeel::pluginMuted();
    const juce::Font labelFont = OmbicLookAndFeel::getOmbicFontForPainting(9.0f, true);
    modeLabel.setBorderSize(labelPadding);
    modeLabel.setColour(juce::Label::textColourId, labelCol);
    modeLabel.setFont(labelFont);
    compressLimitLabel.setBorderSize(labelPadding);
    compressLimitLabel.setColour(juce::Label::textColourId, labelCol);
    compressLimitLabel.setFont(labelFont);
    thresholdSlider.setName("compressor");
    thresholdSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    thresholdSlider.setRotaryParameters(juce::Slider::RotaryParameters{ -2.356f, 2.356f, true });
    thresholdSlider.setColour(juce::Slider::rotarySliderFillColourId, OmbicLookAndFeel::ombicBlue());
    thresholdSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 52, 18);
    thresholdSlider.setColour(juce::Slider::textBoxTextColourId, textCol);
    thresholdSlider.setVelocityBasedMode(false);
    addAndMakeVisible(thresholdSlider);
    thresholdLabel.setText("THRESHOLD", juce::dontSendNotification);
    thresholdLabel.setBorderSize(labelPadding);
    thresholdLabel.setColour(juce::Label::textColourId, labelCol);
    thresholdLabel.setFont(labelFont);
    addAndMakeVisible(thresholdLabel);

    ratioSlider.setName("compressor");
    ratioSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    ratioSlider.setRotaryParameters(juce::Slider::RotaryParameters{ -2.356f, 2.356f, true });
    ratioSlider.setColour(juce::Slider::rotarySliderFillColourId, OmbicLookAndFeel::ombicBlue());
    ratioSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 52, 18);
    ratioSlider.setColour(juce::Slider::textBoxTextColourId, textCol);
    ratioSlider.setVelocityBasedMode(false);
    addAndMakeVisible(ratioSlider);
    ratioLabel.setText("RATIO", juce::dontSendNotification);
    ratioLabel.setBorderSize(labelPadding);
    ratioLabel.setColour(juce::Label::textColourId, labelCol);
    ratioLabel.setFont(labelFont);
    addAndMakeVisible(ratioLabel);

    attackSlider.setName("compressor");
    attackSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    attackSlider.setRotaryParameters(juce::Slider::RotaryParameters{ -2.356f, 2.356f, true });
    attackSlider.setColour(juce::Slider::rotarySliderFillColourId, OmbicLookAndFeel::ombicBlue());
    attackSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 52, 18);
    attackSlider.setColour(juce::Slider::textBoxTextColourId, textCol);
    attackSlider.setVelocityBasedMode(false);
    addAndMakeVisible(attackSlider);
    attackLabel.setText("ATTACK", juce::dontSendNotification);
    attackLabel.setBorderSize(labelPadding);
    attackLabel.setColour(juce::Label::textColourId, labelCol);
    attackLabel.setFont(labelFont);
    addAndMakeVisible(attackLabel);

    releaseSlider.setName("compressor");
    releaseSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    releaseSlider.setRotaryParameters(juce::Slider::RotaryParameters{ -2.356f, 2.356f, true });
    releaseSlider.setColour(juce::Slider::rotarySliderFillColourId, OmbicLookAndFeel::ombicBlue());
    releaseSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 52, 18);
    releaseSlider.setColour(juce::Slider::textBoxTextColourId, textCol);
    releaseSlider.setVelocityBasedMode(false);
    addAndMakeVisible(releaseSlider);
    releaseLabel.setText("RELEASE", juce::dontSendNotification);
    releaseLabel.setBorderSize(labelPadding);
    releaseLabel.setColour(juce::Label::textColourId, labelCol);
    releaseLabel.setFont(labelFont);
    addAndMakeVisible(releaseLabel);

    speedSlider.setName("pwm_speed");
    speedSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    speedSlider.setRotaryParameters(juce::Slider::RotaryParameters{ -2.356f, 2.356f, true });
    speedSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xff00a67e)); // ombicTeal
    speedSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 52, 18);
    speedSlider.setColour(juce::Slider::textBoxTextColourId, textCol);
    speedSlider.setVelocityBasedMode(false);
    addAndMakeVisible(speedSlider);
    speedLabel.setText("SPEED", juce::dontSendNotification);
    speedLabel.setBorderSize(labelPadding);
    speedLabel.setColour(juce::Label::textColourId, labelCol);
    speedLabel.setFont(labelFont);
    addAndMakeVisible(speedLabel);

    addAndMakeVisible(grMeter);
    grReadoutLabel.setText("0.0 dB", juce::dontSendNotification);
    grReadoutLabel.setJustificationType(juce::Justification::centred);
    grReadoutLabel.setFont(OmbicLookAndFeel::getOmbicFontForPainting(16.0f, true));  // §6 GR readout 16px
    addAndMakeVisible(grReadoutLabel);
}

CompressorSection::~CompressorSection()
{
    setLookAndFeel(nullptr);
}

void CompressorSection::updateGrReadout()
{
    float grDb = proc.gainReductionDb.load();
    smoothedGrDb_ += 0.12f * (grDb - smoothedGrDb_); // ~300 ms ballistics
    grReadoutLabel.setText(juce::String(smoothedGrDb_, 1) + " dB", juce::dontSendNotification);
    // Spec §8: GR color-coded — teal <3 dB, yellow 3–6 dB, red >6 dB
    float absGr = std::abs(smoothedGrDb_);
    if (absGr < 3.0f)
        grReadoutLabel.setColour(juce::Label::textColourId, OmbicLookAndFeel::ombicTeal());
    else if (absGr < 6.0f)
        grReadoutLabel.setColour(juce::Label::textColourId, OmbicLookAndFeel::ombicYellow());
    else
        grReadoutLabel.setColour(juce::Label::textColourId, OmbicLookAndFeel::ombicRed());
}

void CompressorSection::setModeControlsVisible(int mode)
{
    const bool isOpto = (mode == 0);
    const bool isFet = (mode == 1);
    const bool isPwm = (mode == 2);
    ratioSlider.setVisible(isFet || isPwm);
    ratioLabel.setVisible(isFet || isPwm);
    attackSlider.setVisible(isFet);
    attackLabel.setVisible(isFet);
    releaseSlider.setVisible(isFet);
    releaseLabel.setVisible(isFet);
    speedSlider.setVisible(isPwm);
    speedLabel.setVisible(isPwm);
    compressLimitToggle_.setVisible(isOpto);
}

void CompressorSection::setShowGrMeter(bool show)
{
    showGrMeter_ = show;
    grMeter.setVisible(show);
    grReadoutLabel.setVisible(show);
}

void CompressorSection::updateCompressLimitButtonStates()
{
    compressLimitToggle_.setToggleState(compressLimitCombo.getSelectedId() == 2, juce::dontSendNotification);
}

bool CompressorSection::isInteracting() const
{
    return modeCombo.isMouseButtonDown() || compressLimitCombo.isMouseButtonDown()
        || compressLimitToggle_.isMouseButtonDown()
        || thresholdSlider.isMouseButtonDown() || ratioSlider.isMouseButtonDown()
        || attackSlider.isMouseButtonDown() || releaseSlider.isMouseButtonDown()
        || speedSlider.isMouseButtonDown();
}

void CompressorSection::setHighlight(bool on)
{
    if (highlighted_ != on) { highlighted_ = on; repaint(); }
}

void CompressorSection::paint(juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat();
    // Spec §4: module card — pluginSurface, 2px pluginBorder, 16px radius
    g.setColour(OmbicLookAndFeel::pluginSurface());
    g.fillRoundedRectangle(b, 16.0f);
    g.setColour(OmbicLookAndFeel::pluginBorder());
    g.drawRoundedRectangle(b, 16.0f, 2.0f);
    const float headerH = getHeight() < 110 ? 22.0f : 36.0f;  // §6 header 36px
    auto headerRect = b.removeFromTop(headerH);
    g.setColour(OmbicLookAndFeel::ombicBlue());
    g.fillRoundedRectangle(headerRect.withBottom(headerRect.getY() + headerH), 16.0f);
    g.setColour(juce::Colours::white);
    g.setFont(OmbicLookAndFeel::getOmbicFontForPainting(13.0f, true));  // §7 Module headers 13px
    g.drawText("COMPRESSOR", static_cast<int>(headerRect.getX()) + 12, static_cast<int>(headerRect.getY() + (headerH - 13.0f) * 0.5f), 200, 14, juce::Justification::left);
}

void CompressorSection::resized()
{
    auto r = getLocalBounds();
    const bool compact = (r.getHeight() < 110);
    const int headerH = compact ? 22 : 36;  // §6 Module card header 36px
    r.removeFromTop(headerH);
    const int bodyPad = compact ? 8 : 14;  // §6 body padding 14px
    r.reduce(bodyPad, 0);
    r.removeFromBottom(bodyPad);

    const int labelH = compact ? 12 : 18;
    const int gap = compact ? 10 : 16;  // Spec §6: 16px gap between FET knobs
    modeCombo.setBounds(0, 0, 1, 1);
    int x = r.getX();
    bool pwmVisible = speedSlider.isVisible();
    bool fetVisible = ratioSlider.isVisible() && !pwmVisible;
    bool optoVisible = !fetVisible && !pwmVisible;
    if (optoVisible)
    {
        const int switchW = compact ? 100 : 120;
        const int switchH = compact ? 22 : 26;
        compressLimitToggle_.setBounds(x, r.getY() + labelH, switchW, switchH);
        x += switchW + gap;
    }
    auto placeKnob = [&](juce::Slider& sl, juce::Label& lb, int size) {
        lb.setBounds(x, r.getY(), size + gap, labelH);
        sl.setBounds(x, r.getY() + labelH, size, size);
        x += size + gap;
    };
    const int knobSizeOpto = compact ? 52 : 80;   // §6 Opto (single) 80px diameter
    int knobSizeFet = compact ? 46 : 60;         // §6 FET (each) 60px diameter
    const int numKnobs = pwmVisible ? 3 : (fetVisible ? 4 : 1);
    if (fetVisible || pwmVisible)
    {
        int availableW = r.getWidth();
        int requiredForKnobs = numKnobs * (knobSizeFet + gap);
        if (showGrMeter_)
        {
            const int grBlockW = gap + (compact ? 20 : 24) + 8;
            requiredForKnobs += grBlockW;
        }
        if (availableW < requiredForKnobs && knobSizeFet > 40)
            knobSizeFet = juce::jmax(40, (availableW - (showGrMeter_ ? (gap + (compact ? 20 : 24) + 8) : 0) - numKnobs * gap) / numKnobs);
    }
    if (pwmVisible)
    {
        placeKnob(thresholdSlider, thresholdLabel, knobSizeFet);
        placeKnob(ratioSlider, ratioLabel, knobSizeFet);
        placeKnob(speedSlider, speedLabel, knobSizeFet);
    }
    else if (fetVisible)
    {
        placeKnob(thresholdSlider, thresholdLabel, knobSizeFet);
        placeKnob(ratioSlider, ratioLabel, knobSizeFet);
        placeKnob(attackSlider, attackLabel, knobSizeFet);
        placeKnob(releaseSlider, releaseLabel, knobSizeFet);
    }
    else
    {
        int optoAreaW = r.getWidth() - (x - r.getX());
        int threshX = x + (optoAreaW - knobSizeOpto) / 2;
        thresholdLabel.setBounds(threshX, r.getY(), knobSizeOpto + gap, labelH);
        thresholdSlider.setBounds(threshX, r.getY() + labelH, knobSizeOpto, knobSizeOpto);
        x = threshX + knobSizeOpto + gap;
    }

    int knobH = (fetVisible || pwmVisible) ? knobSizeFet : knobSizeOpto;
    if (showGrMeter_)
    {
        const int grMeterW = compact ? 20 : 24;
        const int grReadoutH = compact ? 14 : 18;
        grMeter.setBounds(x + gap, r.getY() + labelH, grMeterW, juce::jmax(20, knobH - grReadoutH));
        grReadoutLabel.setBounds(x + gap, r.getY() + labelH + juce::jmax(20, knobH - grReadoutH), grMeterW + 8, grReadoutH);
    }

    if (optoVisible)
        thresholdSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, compact ? 44 : 56, compact ? 16 : 24);
    else
        thresholdSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, compact ? 40 : 52, compact ? 14 : 18);
    float valueFontSize = (fetVisible || pwmVisible) ? (compact ? 12.0f : 14.0f) : (compact ? 14.0f : 20.0f);
    for (int i = 0; i < thresholdSlider.getNumChildComponents(); ++i)
    {
        if (auto* lb = dynamic_cast<juce::Label*>(thresholdSlider.getChildComponent(i)))
        {
            lb->setFont(OmbicLookAndFeel::getOmbicFontForPainting(valueFontSize, true));
            break;
        }
    }
}
