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

    /** Call from editor timer to refresh GR readout. */
    void updateGrReadout();

private:
    bool highlighted_ = false;
    OmbicCompressorProcessor& proc;
    OmbicLookAndFeel ombicLf;

    /** Spec §8: 6px × 80px level meter, bottom-up fill. */
    class LevelMeterComponent : public juce::Component
    {
    public:
        LevelMeterComponent(OmbicCompressorProcessor& p, bool isInput);
        void paint(juce::Graphics& g) override;
    private:
        OmbicCompressorProcessor* proc_;
        bool isInput_;
        float smoothedDb_ = -60.0f;
    };

    LevelMeterComponent inMeter_;
    LevelMeterComponent outMeter_;
    juce::Label inLabel_;
    juce::Label outLabel_;
    juce::Slider outputSlider;
    juce::Label outputLabel;
    juce::Label grLabel_;       // "GR" 8px above value
    juce::Label grReadoutLabel_;
    float smoothedGrDb_ = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OutputSection)
};
