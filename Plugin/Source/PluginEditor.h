#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "Components/OmbicLookAndFeel.h"
#include "Components/CompressorSection.h"
#include "Components/SaturatorSection.h"
#include "Components/OutputSection.h"
#include "Components/SidechainFilterSection.h"
#include "Components/MeterStrip.h"

//==============================================================================
class OmbicCompressorEditor : public juce::AudioProcessorEditor,
                              private juce::Timer
{
public:
    explicit OmbicCompressorEditor(OmbicCompressorProcessor&);
    ~OmbicCompressorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void updateModeVisibility();
    void applyColumnLayout(int scFilterW, int compW, int neonW, int outW);

    OmbicCompressorProcessor& processorRef;
    bool lastCurveDataState_ = false;
    OmbicLookAndFeel ombicLf;
    juce::Image logoWatermark_;

    // §3: animate column widths over 300ms when switching Opto/FET
    float animScFilterW_ = 0, animCompW_ = 0, animNeonW_ = 0, animOutW_ = 0;
    int contentX_ = 0, contentY_ = 0, contentW_ = 0, contentH_ = 0;
    const float animRate_ = 0.15f;  // ~300ms at 25 Hz

    juce::TextButton optoPill_;
    juce::TextButton fetPill_;

    SidechainFilterSection sidechainFilterSection;
    CompressorSection compressorSection;
    SaturatorSection saturatorSection;
    OutputSection outputSection;
    MeterStrip meterStrip;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboBoxAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

    std::unique_ptr<SliderAttachment> scFrequencyAttachment;
    std::unique_ptr<ButtonAttachment> scListenAttachment;
    std::unique_ptr<ComboBoxAttachment> modeAttachment;
    std::unique_ptr<ComboBoxAttachment> compressLimitAttachment;
    std::unique_ptr<SliderAttachment> thresholdAttachment;
    std::unique_ptr<SliderAttachment> ratioAttachment;
    std::unique_ptr<SliderAttachment> attackAttachment;
    std::unique_ptr<SliderAttachment> releaseAttachment;
    std::unique_ptr<SliderAttachment> makeupAttachment;
    std::unique_ptr<SliderAttachment> neonDriveAttachment;
    std::unique_ptr<SliderAttachment> neonIntensityAttachment;
    std::unique_ptr<SliderAttachment> neonToneAttachment;
    std::unique_ptr<SliderAttachment> neonMixAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OmbicCompressorEditor)
};
