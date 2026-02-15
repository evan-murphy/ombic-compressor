#pragma once

#include <JuceHeader.h>
#include "OmbicLookAndFeel.h"

class OmbicCompressorProcessor;

/** Horizontal strip: input level, gain reduction, output level. VU-style ballistics (~300 ms). */
class MeterStrip : public juce::Component,
                   private juce::Timer
{
public:
    explicit MeterStrip(OmbicCompressorProcessor& processor);
    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    void timerCallback() override;

    OmbicCompressorProcessor& proc;
    float smoothedInDb_ = -60.0f;
    float smoothedGrDb_ = 0.0f;
    float smoothedOutDb_ = -60.0f;
    static constexpr float ballisticsCoeff_ = 0.12f; // ~300 ms at 25 Hz

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MeterStrip)
};
