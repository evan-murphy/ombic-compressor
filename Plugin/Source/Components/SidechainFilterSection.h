#pragma once

#include <JuceHeader.h>
#include "OmbicLookAndFeel.h"

class OmbicCompressorProcessor;

/** Sidechain filter module: HPF for detector, frequency response display, Listen button. */
class SidechainFilterSection : public juce::Component,
                               private juce::Timer
{
public:
    explicit SidechainFilterSection(OmbicCompressorProcessor& processor);
    ~SidechainFilterSection() override;
    void resized() override;
    void paint(juce::Graphics& g) override;

    juce::Slider& getFrequencySlider() { return frequencySlider_; }
    juce::ToggleButton& getListenButton() { return listenButton_; }

private:
    void timerCallback() override;

    class FrequencyResponseDisplay : public juce::Component
    {
    public:
        FrequencyResponseDisplay(OmbicCompressorProcessor& p);
        void paint(juce::Graphics& g) override;
        void setFrequencyHz(float hz) { frequencyHz_ = hz; }
    private:
        float frequencyHz_ = 20.0f;
    };

    OmbicCompressorProcessor& proc_;
    OmbicLookAndFeel ombicLf_;
    FrequencyResponseDisplay freqResponseDisplay_;
    juce::Slider frequencySlider_;
    juce::Label frequencyLabel_;
    juce::Label frequencyValueLabel_;
    juce::ToggleButton listenButton_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SidechainFilterSection)
};
