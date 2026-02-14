#pragma once

#include <JuceHeader.h>
#include "OmbicLookAndFeel.h"

class OmbicCompressorProcessor;

/** Transfer curve: input level (x) vs output level (y). 1:1 below threshold, flattening above by ratio. */
class TransferCurveComponent : public juce::Component
{
public:
    explicit TransferCurveComponent(OmbicCompressorProcessor& processor);
    void paint(juce::Graphics& g) override;
    void resized() override;

    void mouseEnter(const juce::MouseEvent&) override;
    void mouseExit(const juce::MouseEvent&) override;

private:
    bool hovered_ = false;
    OmbicCompressorProcessor& proc;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TransferCurveComponent)
};
