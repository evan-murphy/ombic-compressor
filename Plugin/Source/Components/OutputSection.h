#pragma once

#include <JuceHeader.h>
#include "OmbicLookAndFeel.h"

class OmbicCompressorProcessor;

/** Output-stage gain (makeup). Spec §8: IN meter | Output knob | OUT meter, GR readout below. */
class OutputSection : public juce::Component
{
public:
    explicit OutputSection(OmbicCompressorProcessor& processor);
    ~OutputSection() override;
    void resized() override;
    void paint(juce::Graphics& g) override;

    bool isInteracting() const;
    void setHighlight(bool on);

    juce::Slider& getOutputSlider() { return outputSlider; }
    juce::Slider& getIronSlider() { return ironSlider; }
    juce::ToggleButton& getAutoGainButton() { return autoGainButton; }

    /** Call from editor timer to refresh GR readout. */
    void updateGrReadout();

private:
    bool highlighted_ = false;
    int ironAreaY_ = 0;  // Y position of Iron block (for divider line in paint)
    OmbicCompressorProcessor& proc;
    OmbicLookAndFeel ombicLf;

    /** Spec §8: 6px × 80px level meter, peak ballistics + peak hold. */
    class LevelMeterComponent : public juce::Component
    {
    public:
        LevelMeterComponent(OmbicCompressorProcessor& p, bool isInput);
        void paint(juce::Graphics& g) override;
    private:
        OmbicCompressorProcessor* proc_;
        bool isInput_;
        float peakDb_ = -60.0f;
        float peakHoldDb_ = -60.0f;
        int peakHoldTicks_ = 0;
        static constexpr float kPeakAttackCoeff = 0.99f;
        static constexpr float kPeakReleaseCoeff = 0.054f;
        static constexpr int kPeakHoldTicks = 90;  // ~2 s at 45 Hz
    };

    LevelMeterComponent inMeter_;
    LevelMeterComponent outMeter_;
    juce::Label inLabel_;
    juce::Label outLabel_;
    juce::Slider ironSlider;
    juce::Label ironLabel;
    juce::Label ironSubLabel_;  // "Output saturation" — not a compressor mode
    juce::Slider outputSlider;
    juce::Label outputLabel;
    juce::ToggleButton autoGainButton;
    juce::Label grLabel_;       // "GR" 8px above value
    juce::Label grReadoutLabel_;
    float smoothedGrDb_ = 0.0f;
    float grHoldDb_ = 0.0f;
    int grHoldTicks_ = 0;
    static constexpr float kGrAttackCoeff = 0.77f;
    static constexpr float kGrReleaseCoeff = 0.071f;
    static constexpr int kGrHoldTicks = 68;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OutputSection)
};
