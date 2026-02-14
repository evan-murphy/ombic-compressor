#pragma once

#include <JuceHeader.h>
#include "OmbicLookAndFeel.h"

class OmbicCompressorProcessor;

/** Output-stage gain (makeup). Comes after saturator and compressor in the signal chain. */
class OutputSection : public juce::Component
{
public:
    explicit OutputSection(OmbicCompressorProcessor& processor);
    ~OutputSection() override;
    void resized() override;
    void paint(juce::Graphics& g) override;

    void mouseEnter(const juce::MouseEvent&) override;
    void mouseExit(const juce::MouseEvent&) override;

    juce::Slider& getOutputSlider() { return outputSlider; }

private:
    bool hovered_ = false;
    [[maybe_unused]] OmbicCompressorProcessor& proc;
    OmbicLookAndFeel ombicLf;

    juce::Slider outputSlider;
    juce::Label outputLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OutputSection)
};
