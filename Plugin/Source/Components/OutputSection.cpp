#include "OutputSection.h"
#include "../PluginProcessor.h"

static float levelToNorm(float db)
{
    return juce::jlimit(0.0f, 1.0f, (db + 60.0f) / 60.0f);
}

OutputSection::LevelMeterComponent::LevelMeterComponent(OmbicCompressorProcessor& p, bool isInput)
    : proc_(&p), isInput_(isInput) {}

void OutputSection::LevelMeterComponent::paint(juce::Graphics& g)
{
    float rawDb = isInput_ ? proc_->inputPeakDb.load() : proc_->outputPeakDb.load();
    if (rawDb > peakDb_)
        peakDb_ += kPeakAttackCoeff * (rawDb - peakDb_);
    else
        peakDb_ += kPeakReleaseCoeff * (rawDb - peakDb_);
    if (peakDb_ >= peakHoldDb_) { peakHoldDb_ = peakDb_; peakHoldTicks_ = kPeakHoldTicks; }
    else { if (peakHoldTicks_ > 0) --peakHoldTicks_; if (peakHoldTicks_ <= 0) peakHoldDb_ += kPeakReleaseCoeff * (peakDb_ - peakHoldDb_); }
    float norm = levelToNorm(peakDb_);
    auto b = getLocalBounds().toFloat();
    g.setColour(OmbicLookAndFeel::pluginBg());
    g.fillRoundedRectangle(b, 3.0f);
    g.setColour(OmbicLookAndFeel::pluginBorder());
    g.drawRoundedRectangle(b.reduced(0.5f), 3.0f, 1.0f);
    if (norm > 0.002f)
    {
        g.setColour(isInput_ ? OmbicLookAndFeel::ombicBlue() : OmbicLookAndFeel::ombicTeal());
        float h = b.getHeight() * norm;
        g.fillRoundedRectangle(b.getX() + 1, b.getBottom() - h, b.getWidth() - 2, h, 2.0f);
    }
    float holdNorm = levelToNorm(peakHoldDb_);
    if (holdNorm > 0.002f && holdNorm > norm + 0.02f)
    {
        g.setColour((isInput_ ? OmbicLookAndFeel::ombicBlue() : OmbicLookAndFeel::ombicTeal()).withAlpha(0.6f));
        float holdY = b.getBottom() - b.getHeight() * holdNorm;
        g.fillRect(b.getX() + 1, holdY - 0.5f, b.getWidth() - 2, 1.0f);
    }
}

OutputSection::OutputSection(OmbicCompressorProcessor& processor)
    : proc(processor)
    , inMeter_(processor, true)
    , outMeter_(processor, false)
{
    setLookAndFeel(&ombicLf);

    const juce::BorderSize<int> labelPadding(4, 0, 0, 0);
    const juce::Colour textCol = OmbicLookAndFeel::pluginText();
    const juce::Colour labelCol = OmbicLookAndFeel::pluginMuted();
    const juce::Font labelFont = OmbicLookAndFeel::getOmbicFontForPainting(9.0f, true);
    addAndMakeVisible(inMeter_);
    addAndMakeVisible(outMeter_);
    inLabel_.setTooltip("Input peak level. Thin line = 2 s peak hold.");
    outLabel_.setTooltip("Output peak level. Thin line = 2 s peak hold.");
    inLabel_.setText("IN", juce::dontSendNotification);
    outLabel_.setText("OUT", juce::dontSendNotification);
    inLabel_.setFont(OmbicLookAndFeel::getOmbicFontForPainting(8.0f, true));
    outLabel_.setFont(OmbicLookAndFeel::getOmbicFontForPainting(8.0f, true));
    inLabel_.setColour(juce::Label::textColourId, labelCol);
    outLabel_.setColour(juce::Label::textColourId, labelCol);
    addAndMakeVisible(inLabel_);
    addAndMakeVisible(outLabel_);
    ironSlider.setName("iron");
    ironSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    ironSlider.setRotaryParameters(juce::Slider::RotaryParameters{ -2.356f, 2.356f, true });
    ironSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xffe5a800)); // ombicYellow
    ironSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 40, 16);
    ironSlider.setColour(juce::Slider::textBoxTextColourId, textCol);
    ironSlider.setVelocityBasedMode(false);
    addAndMakeVisible(ironSlider);
    ironLabel.setText("IRON", juce::dontSendNotification);
    ironLabel.setBorderSize(labelPadding);
    ironLabel.setColour(juce::Label::textColourId, labelCol);
    ironLabel.setFont(labelFont);
    addAndMakeVisible(ironLabel);
    ironSubLabel_.setText("Output saturation", juce::dontSendNotification);
    ironSubLabel_.setColour(juce::Label::textColourId, labelCol.withAlpha(0.7f));
    ironSubLabel_.setFont(OmbicLookAndFeel::getOmbicFontForPainting(8.0f, false));
    addAndMakeVisible(ironSubLabel_);
    autoGainButton.setName("autoGain");
    autoGainButton.setButtonText("Auto Gain");
    autoGainButton.setClickingTogglesState(true);
    addAndMakeVisible(autoGainButton);
    grLabel_.setText("GR", juce::dontSendNotification);
    grLabel_.setFont(OmbicLookAndFeel::getOmbicFontForPainting(8.0f, true));
    grLabel_.setColour(juce::Label::textColourId, labelCol);
    grLabel_.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(grLabel_);
    outputSlider.setName("output");
    outputSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    outputSlider.setRotaryParameters(juce::Slider::RotaryParameters{ -2.356f, 2.356f, true });
    outputSlider.setColour(juce::Slider::rotarySliderFillColourId, OmbicLookAndFeel::ombicPurple());
    outputSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 52, 18);
    outputSlider.setColour(juce::Slider::textBoxTextColourId, textCol);
    outputSlider.setVelocityBasedMode(false);
    addAndMakeVisible(outputSlider);
    outputLabel.setText("OUTPUT", juce::dontSendNotification);
    outputLabel.setBorderSize(labelPadding);
    outputLabel.setColour(juce::Label::textColourId, labelCol);
    outputLabel.setFont(labelFont);
    addAndMakeVisible(outputLabel);
    grReadoutLabel_.setText("0.0 dB", juce::dontSendNotification);
    grReadoutLabel_.setJustificationType(juce::Justification::centred);
    grReadoutLabel_.setFont(OmbicLookAndFeel::getOmbicFontForPainting(16.0f, true));
    grReadoutLabel_.setColour(juce::Label::textColourId, OmbicLookAndFeel::ombicTeal());
    grReadoutLabel_.setTooltip("Gain reduction: how much the compressor is reducing. Fast response.");
    addAndMakeVisible(grReadoutLabel_);
}

OutputSection::~OutputSection()
{
    setLookAndFeel(nullptr);
}

bool OutputSection::isInteracting() const
{
    return outputSlider.isMouseButtonDown() || ironSlider.isMouseButtonDown();
}

void OutputSection::setHighlight(bool on)
{
    if (highlighted_ != on) { highlighted_ = on; repaint(); }
}

void OutputSection::updateGrReadout()
{
    float grDb = proc.gainReductionDb.load();
    if (grDb > smoothedGrDb_)
        smoothedGrDb_ += kGrAttackCoeff * (grDb - smoothedGrDb_);
    else
        smoothedGrDb_ += kGrReleaseCoeff * (grDb - smoothedGrDb_);
    if (smoothedGrDb_ > grHoldDb_) { grHoldDb_ = smoothedGrDb_; grHoldTicks_ = kGrHoldTicks; }
    else { if (grHoldTicks_ > 0) --grHoldTicks_; if (grHoldTicks_ <= 0) grHoldDb_ += kGrReleaseCoeff * (smoothedGrDb_ - grHoldDb_); }
    grReadoutLabel_.setText(juce::String(smoothedGrDb_, 1) + " dB", juce::dontSendNotification);
    float absGr = std::abs(smoothedGrDb_);
    if (absGr < 3.0f)
        grReadoutLabel_.setColour(juce::Label::textColourId, OmbicLookAndFeel::ombicTeal());
    else if (absGr < 6.0f)
        grReadoutLabel_.setColour(juce::Label::textColourId, OmbicLookAndFeel::ombicYellow());
    else
        grReadoutLabel_.setColour(juce::Label::textColourId, OmbicLookAndFeel::ombicRed());
}

void OutputSection::paint(juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat();
    g.setColour(OmbicLookAndFeel::pluginSurface());
    g.fillRoundedRectangle(b, 16.0f);
    g.setColour(OmbicLookAndFeel::pluginBorder());
    g.drawRoundedRectangle(b, 16.0f, 2.0f);
    const float headerH = getHeight() < 110 ? 22.0f : 36.0f;  // §6 header 36px
    auto headerRect = b.removeFromTop(headerH);
    g.setColour(OmbicLookAndFeel::ombicPurple());
    g.fillRoundedRectangle(headerRect.withBottom(headerRect.getY() + headerH), 16.0f);
    g.setColour(juce::Colours::white);
    g.setFont(OmbicLookAndFeel::getOmbicFontForPainting(13.0f, true));  // §7 Module headers 13px
    g.drawText("OUTPUT", static_cast<int>(headerRect.getX()) + 12, static_cast<int>((headerH - 13.0f) * 0.5f), 200, 14, juce::Justification::left);
    // Divider line above Iron area (drawn in resized region; ironAreaY_ set in resized)
    if (ironAreaY_ > 0)
    {
        g.setColour(OmbicLookAndFeel::pluginBorder().withAlpha(0.6f));
        g.drawHorizontalLine(ironAreaY_, b.getX() + 8.0f, b.getRight() - 8.0f);
    }
}

void OutputSection::resized()
{
    auto r = getLocalBounds();
    const bool compact = (r.getHeight() < 110);
    const int headerH = compact ? 22 : 36;  // §6 Module card header 36px
    r.removeFromTop(headerH);
    r.reduce(compact ? 8 : 14, compact ? 8 : 14);  // §6 body padding 14px
    const int meterW = 6;
    const int meterH = compact ? 40 : 80;
    const int ironKnobSize = compact ? 40 : 48;
    const int outputKnobSize = compact ? 44 : 56;
    const int gap = compact ? 6 : 10;
    const int labelH = compact ? 10 : 18;
    const int meterLabelH = compact ? 8 : 12;

    // Main output area: IN | Output | OUT (no Iron here)
    const int mainRowW = meterW + gap + outputKnobSize + gap + meterW;
    int startX = r.getX() + (r.getWidth() - mainRowW) / 2;
    int x = startX;
    inMeter_.setBounds(x, r.getY() + labelH, meterW, meterH);
    inLabel_.setBounds(x - 2, r.getY() + labelH + meterH + 2, meterW + 4, meterLabelH);
    x += meterW + gap;
    outputLabel.setBounds(x, r.getY(), outputKnobSize, labelH);
    outputSlider.setBounds(x, r.getY() + labelH, outputKnobSize, outputKnobSize);
    x += outputKnobSize + gap;
    outMeter_.setBounds(x, r.getY() + labelH, meterW, meterH);
    outLabel_.setBounds(x - 2, r.getY() + labelH + meterH + 2, meterW + 4, meterLabelH);

    const int grValueH = compact ? 14 : 22;
    const int toggleH = compact ? 18 : 22;
    const int mainBlockBottom = r.getY() + labelH + meterH + 4 + toggleH + meterLabelH + grValueH;
    autoGainButton.setBounds(r.getX(), r.getY() + labelH + meterH + 4, 80, toggleH);
    grLabel_.setBounds(r.getX(), r.getY() + labelH + meterH + 4 + toggleH, r.getWidth(), meterLabelH);
    grReadoutLabel_.setBounds(r.getX(), r.getY() + labelH + meterH + 4 + toggleH + meterLabelH, r.getWidth(), grValueH);

    // Iron as its own area below main output
    const int ironGap = compact ? 6 : 10;
    ironAreaY_ = mainBlockBottom + ironGap;
    auto ironRow = r.withTop(ironAreaY_).withHeight(labelH + ironKnobSize + 4 + meterLabelH);
    int ix = ironRow.getX() + (ironRow.getWidth() - (ironKnobSize + 20)) / 2;
    ironLabel.setBounds(ix, ironRow.getY(), ironKnobSize + 20, labelH);
    ironSubLabel_.setBounds(ix, ironRow.getY() + labelH + ironKnobSize + 2, ironKnobSize + 20, meterLabelH);
    ironSlider.setBounds(ix, ironRow.getY() + labelH, ironKnobSize, ironKnobSize);
}
