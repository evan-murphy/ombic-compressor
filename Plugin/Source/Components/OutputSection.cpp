#include "OutputSection.h"
#include "../PluginProcessor.h"

OutputSection::OutputSection(OmbicCompressorProcessor& processor)
    : proc(processor)
{
    setLookAndFeel(&ombicLf);

    outputSlider.setSliderStyle(juce::Slider::LinearVertical);
    outputSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 56, 20);
    addAndMakeVisible(outputSlider);
    outputLabel.setText("Output", juce::dontSendNotification);
    addAndMakeVisible(outputLabel);
}

OutputSection::~OutputSection()
{
    setLookAndFeel(nullptr);
}

void OutputSection::mouseEnter(const juce::MouseEvent&)
{
    hovered_ = true;
    repaint();
}

void OutputSection::mouseExit(const juce::MouseEvent&)
{
    hovered_ = false;
    repaint();
}

void OutputSection::paint(juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat();
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
    g.setColour(OmbicLookAndFeel::ombicPurple());
    g.fillRoundedRectangle(headerRect, 20.0f);
    g.setColour(juce::Colours::white);
    g.setFont(OmbicLookAndFeel::getOmbicFontForPainting(20.0f, true));
    g.drawText("Output", 12, 2, 200, 24, juce::Justification::left);
}

void OutputSection::resized()
{
    auto r = getLocalBounds().reduced(12);
    r.removeFromTop(28);
    r.removeFromTop(8);

    const int labelH = 18;
    const int sliderW = 44;
    const int sliderH = 100;

    outputLabel.setBounds(r.getX(), r.getY(), sliderW, labelH);
    outputSlider.setBounds(r.getX(), r.getY() + labelH, sliderW, sliderH);
}
