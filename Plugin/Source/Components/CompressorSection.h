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
    juce::Slider& getSpeedSlider() { return speedSlider; }
    juce::Component* getGainReductionMeter() { return &grMeter; }

    /** mode: 0=Opto, 1=FET, 2=PWM. Controls which controls are visible (Compress/Limit vs Ratio/Attack/Release vs Ratio/Speed). */
    void setModeControlsVisible(int mode);
    /** Legacy: true = FET (1), false = Opto (0). */
    void setModeControlsVisible(bool fetishParamsVisible) { setModeControlsVisible(fetishParamsVisible ? 1 : 0); }

    /** When false, hide GR meter and readout (e.g. v2 uses main view + output for GR). */
    void setShowGrMeter(bool show);
    /** Call from editor timer to refresh GR readout from processor. */
    void updateGrReadout();
    /** Call from editor timer to sync Compress/Limit toggle state from param. */
    void updateCompressLimitButtonStates();
    /** True if user is dragging any control in this section. */
    bool isInteracting() const;
    void setHighlight(bool on);

private:
    bool highlighted_ = false;
    bool showGrMeter_ = true;
    OmbicCompressorProcessor& proc;
    OmbicLookAndFeel ombicLf;

    juce::ComboBox modeCombo;
    juce::ComboBox compressLimitCombo;
    juce::ToggleButton compressLimitToggle_;
    juce::Slider thresholdSlider;
    juce::Slider ratioSlider;
    juce::Slider attackSlider;
    juce::Slider releaseSlider;
    juce::Slider speedSlider;
    juce::Label modeLabel;
    juce::Label compressLimitLabel;
    juce::Label thresholdLabel;
    juce::Label ratioLabel;
    juce::Label attackLabel;
    juce::Label releaseLabel;
    juce::Label speedLabel;
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
