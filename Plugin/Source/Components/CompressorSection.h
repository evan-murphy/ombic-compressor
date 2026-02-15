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
    juce::ComboBox& getCompressLimitCombo() { return compressLimitCombo; }
    juce::Slider& getThresholdSlider() { return thresholdSlider; }
    juce::Slider& getRatioSlider() { return ratioSlider; }
    juce::Slider& getAttackSlider() { return attackSlider; }
    juce::Slider& getReleaseSlider() { return releaseSlider; }
    juce::Component* getGainReductionMeter() { return &grMeter; }

    /** fetishParamsVisible: show ratio/attack/release (FET). When false (Opto), show Compress/Limit instead. */
    void setModeControlsVisible(bool fetishParamsVisible);
    /** Call from editor timer to refresh GR readout from processor. */
    void updateGrReadout();
    /** True if user is dragging any control in this section. */
    bool isInteracting() const;
    void setHighlight(bool on);

private:
    bool highlighted_ = false;
    OmbicCompressorProcessor& proc;
    OmbicLookAndFeel ombicLf;

    juce::ComboBox modeCombo;
    juce::ComboBox compressLimitCombo;
    juce::Slider thresholdSlider;
    juce::Slider ratioSlider;
    juce::Slider attackSlider;
    juce::Slider releaseSlider;
    juce::Label modeLabel;
    juce::Label compressLimitLabel;
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
