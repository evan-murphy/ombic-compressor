#pragma once

#include <JuceHeader.h>
#include "OmbicLookAndFeel.h"

class OmbicCompressorProcessor;

/** Compressor controls + mode selector + GR meter. */
class CompressorSection : public juce::Component
{
public:
    explicit CompressorSection(OmbicCompressorProcessor& processor);
    ~CompressorSection() override;
    void resized() override;
    void paint(juce::Graphics& g) override;

    juce::ComboBox& getModeCombo() { return modeCombo; }
    juce::Slider& getThresholdSlider() { return thresholdSlider; }
    juce::Slider& getRatioSlider() { return ratioSlider; }
    juce::Slider& getAttackSlider() { return attackSlider; }
    juce::Slider& getReleaseSlider() { return releaseSlider; }
    juce::Component* getGainReductionMeter() { return &grMeter; }

    void setModeControlsVisible(bool fetishParamsVisible);
    /** Call from editor timer to refresh GR readout from processor. */
    void updateGrReadout();

    void mouseEnter(const juce::MouseEvent&) override;
    void mouseExit(const juce::MouseEvent&) override;

private:
    bool hovered_ = false;
    OmbicCompressorProcessor& proc;
    OmbicLookAndFeel ombicLf;

    juce::ComboBox modeCombo;
    juce::Slider thresholdSlider;
    juce::Slider ratioSlider;
    juce::Slider attackSlider;
    juce::Slider releaseSlider;
    juce::Label modeLabel;
    juce::Label thresholdLabel;
    juce::Label ratioLabel;
    juce::Label attackLabel;
    juce::Label releaseLabel;
    juce::Label grReadoutLabel;
    float smoothedGrDb_ = 0.0f; // VU-style ballistics for readout

    class GainReductionMeterComponent : public juce::Component
    {
    public:
        GainReductionMeterComponent(OmbicCompressorProcessor& p);
        void paint(juce::Graphics& g) override;
    private:
        OmbicCompressorProcessor& processor;
        float smoothedGrDb_ = 0.0f; // VU-style ballistics
    } grMeter;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CompressorSection)
};
