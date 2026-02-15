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
    float db = isInput_ ? proc_->inputLevelDb.load() : proc_->outputLevelDb.load();
    smoothedDb_ += 0.15f * (db - smoothedDb_);
    float norm = levelToNorm(smoothedDb_);
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
    inLabel_.setText("IN", juce::dontSendNotification);
    outLabel_.setText("OUT", juce::dontSendNotification);
    inLabel_.setFont(OmbicLookAndFeel::getOmbicFontForPainting(8.0f, true));
    outLabel_.setFont(OmbicLookAndFeel::getOmbicFontForPainting(8.0f, true));
    inLabel_.setColour(juce::Label::textColourId, labelCol);
    outLabel_.setColour(juce::Label::textColourId, labelCol);
    addAndMakeVisible(inLabel_);
    addAndMakeVisible(outLabel_);
    grLabel_.setText("GR", juce::dontSendNotification);
    grLabel_.setFont(OmbicLookAndFeel::getOmbicFontForPainting(8.0f, true));
    grLabel_.setColour(juce::Label::textColourId, labelCol);
    grLabel_.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(grLabel_);
    outputSlider.setName("output");
    outputSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
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
    addAndMakeVisible(grReadoutLabel_);
}

OutputSection::~OutputSection()
{
    setLookAndFeel(nullptr);
}

bool OutputSection::isInteracting() const
{
    return outputSlider.isMouseButtonDown();
}

void OutputSection::setHighlight(bool on)
{
    if (highlighted_ != on) { highlighted_ = on; repaint(); }
}

void OutputSection::updateGrReadout()
{
    float grDb = proc.gainReductionDb.load();
    smoothedGrDb_ += 0.12f * (grDb - smoothedGrDb_);
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
    auto headerRect = b.removeFromTop(36.0f);
    g.setColour(OmbicLookAndFeel::ombicPurple());
    g.fillRoundedRectangle(headerRect.withBottom(headerRect.getY() + 36.0f), 16.0f);
    g.setColour(juce::Colours::white);
    g.setFont(OmbicLookAndFeel::getOmbicFontForPainting(13.0f, true));
    g.drawText("OUTPUT", static_cast<int>(headerRect.getX()) + 14, static_cast<int>(headerRect.getY()) + 8, 200, 20, juce::Justification::left);
}

void OutputSection::resized()
{
    auto r = getLocalBounds();
    r.removeFromTop(36);
    r.reduce(14, 14);
    const int meterW = 6;
    const int meterH = 80;
    const int knobSize = 56;
    const int gap = 10;
    const int labelH = 18;
    const int meterLabelH = 12;
    const int minRowW = meterW + gap + knobSize + gap + meterW;
    int rowW = juce::jmin(r.getWidth(), minRowW);
    int startX = r.getX() + (r.getWidth() - rowW) / 2;
    int x = startX;
    inMeter_.setBounds(x, r.getY() + labelH, meterW, meterH);
    inLabel_.setBounds(x - 2, r.getY() + labelH + meterH + 2, meterW + 4, meterLabelH);
    x += meterW + gap;
    outputLabel.setBounds(x, r.getY(), knobSize, labelH);
    outputSlider.setBounds(x, r.getY() + labelH, knobSize, knobSize);
    x += knobSize + gap;
    outMeter_.setBounds(x, r.getY() + labelH, meterW, meterH);
    outLabel_.setBounds(x - 2, r.getY() + labelH + meterH + 2, meterW + 4, meterLabelH);
    const int grValueH = 22;
    grLabel_.setBounds(r.getX(), r.getY() + labelH + meterH + 4, r.getWidth(), meterLabelH);
    grReadoutLabel_.setBounds(r.getX(), r.getY() + labelH + meterH + 4 + meterLabelH, r.getWidth(), grValueH);
}
