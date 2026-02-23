#pragma once

#include <JuceHeader.h>
#include "OmbicLookAndFeel.h"

class OmbicCompressorProcessor;

/** Neon bulb saturator controls: drive, intensity, tone, mix. */
class SaturatorSection : public juce::Component
{
public:
    explicit SaturatorSection(OmbicCompressorProcessor& processor);
    ~SaturatorSection() override;
    void resized() override;
    void paint(juce::Graphics& g) override;

    void mouseEnter(const juce::MouseEvent&) override;
    void mouseExit(const juce::MouseEvent&) override;

    juce::Slider& getDriveSlider() { return driveSlider; }
    juce::Slider& getIntensitySlider() { return intensitySlider; }
    juce::Slider& getToneSlider() { return toneSlider; }
    juce::Slider& getMixSlider() { return mixSlider; }
    juce::Slider& getBurstinessSlider() { return burstinessSlider; }
    juce::Slider& getGMinSlider() { return gMinSlider; }
    juce::ToggleButton& getSoftSatAfterButton() { return softSatAfterButton; }
    juce::ToggleButton& getNeonEnableButton() { return neonEnableButton; }

    /** Re-apply 0–100% display for Drive/Intensity/Tone/Mix and G Min. Call after SliderAttachments are created. */
    void applyPercentDisplay();

    /** When false (v2 layout), scope is hidden and section shows knobs only. Default true. */
    void setScopeVisible(bool visible);
    bool isScopeVisible() const { return scopeVisible_; }

    bool isInteracting() const;
    void setHighlight(bool on);

private:
    class ScopeComponent : public juce::Component
    {
    public:
        ScopeComponent(OmbicCompressorProcessor& processor, juce::Slider& drive, juce::Slider& intensity,
                      juce::Slider& tone, juce::Slider& mix);
        void paint(juce::Graphics& g) override;
    private:
        OmbicCompressorProcessor* proc_ = nullptr;
        juce::Slider* driveSlider_ = nullptr;
        juce::Slider* intensitySlider_ = nullptr;
        [[maybe_unused]] juce::Slider* toneSlider_ = nullptr;
        juce::Slider* mixSlider_ = nullptr;
    };

    bool hovered_ = false;
    bool highlighted_ = false;
    bool scopeVisible_ = true;
    [[maybe_unused]] OmbicCompressorProcessor& proc;
    OmbicLookAndFeel ombicLf;

    juce::Slider driveSlider;
    juce::Slider intensitySlider;
    juce::Slider toneSlider;
    juce::Slider mixSlider;
    juce::Slider burstinessSlider;
    juce::Slider gMinSlider;
    juce::ToggleButton softSatAfterButton;
    juce::ToggleButton neonEnableButton;
    ScopeComponent scopeComponent_;  // when SC Listen on, shows sidechain (teal); else synthetic waveform
    juce::Label driveLabel;
    juce::Label intensityLabel;
    juce::Label toneLabel;
    juce::Label mixLabel;
    juce::Label burstinessLabel;
    juce::Label gMinLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SaturatorSection)
};
