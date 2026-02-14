#include "SaturatorSection.h"
#include "../PluginProcessor.h"

SaturatorSection::SaturatorSection(OmbicCompressorProcessor& processor)
    : proc(processor)
{
    setLookAndFeel(&ombicLf);

    driveSlider.setSliderStyle(juce::Slider::LinearVertical);
    driveSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 56, 20);
    addAndMakeVisible(driveSlider);
    driveLabel.setText("Drive", juce::dontSendNotification);
    addAndMakeVisible(driveLabel);

    intensitySlider.setSliderStyle(juce::Slider::LinearVertical);
    intensitySlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 56, 20);
    addAndMakeVisible(intensitySlider);
    intensityLabel.setText("Intensity", juce::dontSendNotification);
    addAndMakeVisible(intensityLabel);

    toneSlider.setSliderStyle(juce::Slider::LinearVertical);
    toneSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 56, 20);
    addAndMakeVisible(toneSlider);
    toneLabel.setText("Tone", juce::dontSendNotification);
    addAndMakeVisible(toneLabel);

    mixSlider.setSliderStyle(juce::Slider::LinearVertical);
    mixSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 56, 20);
    addAndMakeVisible(mixSlider);
    mixLabel.setText("Mix", juce::dontSendNotification);
    addAndMakeVisible(mixLabel);
}

SaturatorSection::~SaturatorSection()
{
    setLookAndFeel(nullptr);
}

void SaturatorSection::mouseEnter(const juce::MouseEvent&)
{
    hovered_ = true;
    repaint();
}

void SaturatorSection::mouseExit(const juce::MouseEvent&)
{
    hovered_ = false;
    repaint();
}

void SaturatorSection::paint(juce::Graphics& g)
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
    g.setColour(OmbicLookAndFeel::ombicPink());
    g.fillRoundedRectangle(b.removeFromTop(28.0f), 20.0f);
    g.setColour(OmbicLookAndFeel::ink());
    g.setFont(OmbicLookAndFeel::getOmbicFontForPainting(20.0f, true));
    g.drawText("Neon Saturation", 12, 2, 200, 24, juce::Justification::left);
}

void SaturatorSection::resized()
{
    auto r = getLocalBounds().reduced(12);
    r.removeFromTop(28);
    r.removeFromTop(8);

    const int labelH = 18;
    const int sliderW = 44;
    const int gap = 8;

    int x = r.getX();
    const int sliderH = 80;
    auto place = [&](juce::Slider& sl, juce::Label& lb) {
        lb.setBounds(x, r.getY(), sliderW + gap, labelH);
        sl.setBounds(x, r.getY() + labelH, sliderW, sliderH);
        x += sliderW + gap;
    };
    place(driveSlider, driveLabel);
    place(intensitySlider, intensityLabel);
    place(toneSlider, toneLabel);
    place(mixSlider, mixLabel);
}
