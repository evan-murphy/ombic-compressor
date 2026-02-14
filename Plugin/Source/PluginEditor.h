#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "Components/OmbicLookAndFeel.h"
#include "Components/CompressorSection.h"
#include "Components/SaturatorSection.h"
#include "Components/OutputSection.h"
#include "Components/MeterStrip.h"
#include "Components/TransferCurveComponent.h"

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

    OmbicCompressorProcessor& processorRef;
    bool lastCurveDataState_ = false;
    OmbicLookAndFeel ombicLf;
    juce::Image logoImage_;

    CompressorSection compressorSection;
    SaturatorSection saturatorSection;
    OutputSection outputSection;
    TransferCurveComponent transferCurve;
    MeterStrip meterStrip;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboBoxAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

    std::unique_ptr<ComboBoxAttachment> modeAttachment;
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
