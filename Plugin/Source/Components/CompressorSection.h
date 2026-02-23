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
    juce::ComboBox& getFetCharacterCombo() { return fetCharacterCombo; }
    juce::Slider& getThresholdSlider() { return thresholdSlider; }
    juce::Slider& getRatioSlider() { return ratioSlider; }
    juce::Slider& getAttackSlider() { return attackSlider; }
    juce::Slider& getReleaseSlider() { return releaseSlider; }
    juce::Slider& getSpeedSlider() { return speedSlider; }
    juce::Component* getGainReductionMeter() { return &grMeter; }

    /** mode: 0=Opto, 1=FET, 2=PWM, 3=VCA. Controls which controls are visible (Compress/Limit vs Ratio/Attack/Release vs Ratio/Speed vs Threshold+Ratio only for VCA). */
    void setModeControlsVisible(int mode);
    /** Legacy: true = FET (1), false = Opto (0). */
    void setModeControlsVisible(bool fetishParamsVisible) { setModeControlsVisible(fetishParamsVisible ? 1 : 0); }

    /** When false, hide GR meter and readout (e.g. v2 uses main view + output for GR). */
    void setShowGrMeter(bool show);
    /** Call from editor timer to refresh GR readout from processor. */
    void updateGrReadout();
    /** Call from editor timer to sync Compress/Limit toggle state from param. */
    void updateCompressLimitButtonStates();
    /** Call from editor timer to sync FET character pill states from param (when in FET mode). */
    void updateFetCharacterPillStates();
    /** True if user is dragging any control in this section. */
    bool isInteracting() const;
    void setHighlight(bool on);

private:
    void setFetCharacterFromPill(int index);
    bool highlighted_ = false;
    bool showGrMeter_ = true;
    OmbicCompressorProcessor& proc;
    OmbicLookAndFeel ombicLf;

    juce::ComboBox modeCombo;
    juce::ComboBox compressLimitCombo;
    juce::ComboBox fetCharacterCombo;
    juce::Label fetCharacterLabel;
    juce::TextButton fetCharacterPillOff_;
    juce::TextButton fetCharacterPillRevA_;
    juce::TextButton fetCharacterPillLN_;
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
    float smoothedGrDb_ = 0.0f;
    float grHoldDb_ = 0.0f;
    int grHoldTicks_ = 0;
    static constexpr float kGrAttackCoeff = 0.77f;
    static constexpr float kGrReleaseCoeff = 0.071f;
    static constexpr int kGrHoldTicks = 68;

    class GainReductionMeterComponent : public juce::Component
    {
    public:
        GainReductionMeterComponent(OmbicCompressorProcessor& p);
        void paint(juce::Graphics& g) override;
    private:
        OmbicCompressorProcessor& processor;
        float smoothedGrDb_ = 0.0f;
        float grHoldDb_ = 0.0f;
        int grHoldTicks_ = 0;
        static constexpr float kGrAttackCoeff = 0.77f;
        static constexpr float kGrReleaseCoeff = 0.071f;
        static constexpr int kGrHoldTicks = 68;
    } grMeter;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CompressorSection)
};
