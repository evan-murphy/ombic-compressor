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

    bool isInteracting() const;
    void setHighlight(bool on);

private:
    class ScopeComponent : public juce::Component
    {
    public:
        ScopeComponent(juce::Slider& drive, juce::Slider& intensity);
        void paint(juce::Graphics& g) override;
    private:
        juce::Slider* driveSlider_ = nullptr;
        juce::Slider* intensitySlider_ = nullptr;
    };

    void ensureLogoLoaded();

    bool hovered_ = false;
    bool highlighted_ = false;
    [[maybe_unused]] OmbicCompressorProcessor& proc;
    OmbicLookAndFeel ombicLf;
    juce::Image logoImage_;

    juce::Slider driveSlider;
    juce::Slider intensitySlider;
    juce::Slider toneSlider;
    juce::Slider mixSlider;
    ScopeComponent scopeComponent_;
    juce::Label driveLabel;
    juce::Label intensityLabel;
    juce::Label toneLabel;
    juce::Label mixLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SaturatorSection)
};
