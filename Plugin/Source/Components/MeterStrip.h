#pragma once

#include <JuceHeader.h>
#include "OmbicLookAndFeel.h"
#include <vector>

class OmbicCompressorProcessor;

/** Horizontal strip: input level, gain reduction, output level. Dual ballistics (peak + VU average), peak hold, GR fast attack. */
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
    juce::TextButton peakButton_;
    juce::TextButton vuButton_;
    bool showPeak_ = true;  // true = peak, false = VU (average)

    // Level: peak (fast attack / slow release) + average (VU ~300 ms)
    float peakInDb_ = -60.0f;
    float peakOutDb_ = -60.0f;
    float peakInL_ = -60.0f, peakInR_ = -60.0f;
    float peakOutL_ = -60.0f, peakOutR_ = -60.0f;
    float avgInDb_ = -60.0f;
    float avgOutDb_ = -60.0f;
    // Peak hold: 2 s at 60 Hz
    float peakHoldInDb_ = -60.0f;
    float peakHoldOutDb_ = -60.0f;
    int peakHoldInTicks_ = 0;
    int peakHoldOutTicks_ = 0;
    // GR: fast attack (~15 ms), slow release (~300 ms), 1.5 s hold
    float smoothedGrDb_ = 0.0f;
    float grHoldDb_ = 0.0f;
    int grHoldTicks_ = 0;
    // Scrolling GR history: ring buffer, ~500 ms at 60 Hz
    static constexpr size_t kGrHistorySize = 30;
    std::vector<float> grHistory_;
    size_t grHistoryWrite_ = 0;
    bool grHistoryFilled_ = false;

    static constexpr int kMeterHz = 60;
    static constexpr float kPeakAttackCoeff = 0.99f;   // ~5 ms
    static constexpr float kPeakReleaseCoeff = 0.054f;  // ~400 ms
    static constexpr float kVuCoeff = 0.071f;          // ~300 ms VU
    static constexpr float kGrAttackCoeff = 0.77f;     // ~15 ms
    static constexpr float kGrReleaseCoeff = 0.071f;   // ~300 ms
    static constexpr int kPeakHoldTicks = 120;         // 2 s at 60 Hz
    static constexpr int kGrHoldTicks = 90;            // ~1.5 s at 60 Hz

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MeterStrip)
};
