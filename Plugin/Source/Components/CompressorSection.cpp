#include "CompressorSection.h"
#include "../PluginProcessor.h"

//==============================================================================
CompressorSection::GainReductionMeterComponent::GainReductionMeterComponent(OmbicCompressorProcessor& p)
    : processor(p) {}

void CompressorSection::GainReductionMeterComponent::paint(juce::Graphics& g)
{
    float grDb = processor.gainReductionDb.load();
    smoothedGrDb_ += 0.12f * (grDb - smoothedGrDb_); // ~300 ms ballistics
    float norm = juce::jlimit(0.0f, 1.0f, smoothedGrDb_ / 20.0f);
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

    modeCombo.addItem("LALA", 1);
    modeCombo.addItem("FETish", 2);
    modeCombo.setSelectedId(1); // will be overwritten by attachment
    addAndMakeVisible(modeCombo);
    modeLabel.setText("Mode", juce::dontSendNotification);
    modeLabel.attachToComponent(&modeCombo, true);
    addAndMakeVisible(modeLabel);

    thresholdSlider.setSliderStyle(juce::Slider::LinearVertical);
    thresholdSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 56, 20);
    addAndMakeVisible(thresholdSlider);
    thresholdLabel.setText("Threshold", juce::dontSendNotification);
    addAndMakeVisible(thresholdLabel);

    ratioSlider.setSliderStyle(juce::Slider::LinearVertical);
    ratioSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 56, 20);
    addAndMakeVisible(ratioSlider);
    ratioLabel.setText("Ratio", juce::dontSendNotification);
    addAndMakeVisible(ratioLabel);

    attackSlider.setSliderStyle(juce::Slider::LinearVertical);
    attackSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 56, 20);
    addAndMakeVisible(attackSlider);
    attackLabel.setText("Attack", juce::dontSendNotification);
    addAndMakeVisible(attackLabel);

    releaseSlider.setSliderStyle(juce::Slider::LinearVertical);
    releaseSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 56, 20);
    addAndMakeVisible(releaseSlider);
    releaseLabel.setText("Release", juce::dontSendNotification);
    addAndMakeVisible(releaseLabel);

    addAndMakeVisible(grMeter);
    grReadoutLabel.setText("0.0 dB", juce::dontSendNotification);
    grReadoutLabel.setJustificationType(juce::Justification::centred);
    grReadoutLabel.setFont(OmbicLookAndFeel::getOmbicFontForPainting(14.0f, true));
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
}

void CompressorSection::setModeControlsVisible(bool fetishParamsVisible)
{
    ratioSlider.setVisible(fetishParamsVisible);
    ratioLabel.setVisible(fetishParamsVisible);
    attackSlider.setVisible(fetishParamsVisible);
    attackLabel.setVisible(fetishParamsVisible);
    releaseSlider.setVisible(fetishParamsVisible);
    releaseLabel.setVisible(fetishParamsVisible);
}

void CompressorSection::mouseEnter(const juce::MouseEvent&)
{
    hovered_ = true;
    repaint();
}

void CompressorSection::mouseExit(const juce::MouseEvent&)
{
    hovered_ = false;
    repaint();
}

void CompressorSection::paint(juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat();
    // OMBIC card: shadow 8px 8px 0 #0f1220 (6px when hovered), then white fill, 3px border, 20px radius
    const float shadowOff = hovered_ ? 6.0f : 8.0f;
    g.setColour(OmbicLookAndFeel::ink());
    g.fillRoundedRectangle(b.translated(shadowOff, shadowOff), 20.0f);
    g.setColour(OmbicLookAndFeel::bg());
    g.fillRoundedRectangle(b, 20.0f);
    g.setColour(OmbicLookAndFeel::ink());
    g.drawRoundedRectangle(b, 20.0f, 3.0f);
    if (hovered_)
    {
        g.setColour(OmbicLookAndFeel::ombicBlue().withAlpha(0.8f));
        g.drawRoundedRectangle(b.reduced(2.0f), 18.0f, 2.0f);
    }
    auto headerRect = b.removeFromTop(28.0f);
    g.setColour(OmbicLookAndFeel::ombicBlue());
    g.fillRoundedRectangle(headerRect, 20.0f);
    g.setColour(juce::Colours::white);
    g.setFont(OmbicLookAndFeel::getOmbicFontForPainting(20.0f, true));
    g.drawText("Compressor", 12, 2, 200, 24, juce::Justification::left);
}

void CompressorSection::resized()
{
    auto r = getLocalBounds().reduced(12);
    r.removeFromTop(28);
    r.removeFromTop(8);

    const int labelH = 18;
    const int sliderW = 44;
    const int gap = 8;

    modeLabel.setBounds(r.getX(), r.getY(), 120, labelH);
    modeCombo.setBounds(r.getX(), r.getY() + labelH, 120, 28);
    r.removeFromTop(labelH + 28 + 12);

    int x = r.getX();
    const int sliderH = 100;
    auto placeSlider = [&](juce::Slider& sl, juce::Label& lb) {
        lb.setBounds(x, r.getY(), sliderW + gap, labelH);
        sl.setBounds(x, r.getY() + labelH, sliderW, sliderH);
        x += sliderW + gap;
    };
    placeSlider(thresholdSlider, thresholdLabel);
    placeSlider(ratioSlider, ratioLabel);
    placeSlider(attackSlider, attackLabel);
    placeSlider(releaseSlider, releaseLabel);

    const int grMeterW = 24;
    const int grReadoutH = 20;
    grMeter.setBounds(x + gap, r.getY() + labelH, grMeterW, sliderH - grReadoutH);
    grReadoutLabel.setBounds(x + gap, r.getY() + labelH + sliderH - grReadoutH, grMeterW + 8, grReadoutH);
}
