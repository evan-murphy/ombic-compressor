#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "Components/OmbicLookAndFeel.h"
#include "Components/CompressorSection.h"
#include "Components/SaturatorSection.h"
#include "Components/OutputSection.h"
#include "Components/SidechainFilterSection.h"
#include "Components/MainViewAsTubeComponent.h"

/** v2 editor: main view is the tube (saturation-driven glow + filament); Neon section is knobs only. All v1 features preserved. */
class OmbicCompressorEditorV2 : public juce::AudioProcessorEditor,
                                private juce::Timer
{
public:
    explicit OmbicCompressorEditorV2(OmbicCompressorProcessor&);
    ~OmbicCompressorEditorV2() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void updateModeVisibility();

    static constexpr int kBaseWidth = 960;
    static constexpr int kBaseHeight = 540;
    static constexpr int kHeaderH = 38;
    static constexpr int kFooterH = 26;
    static constexpr int kMainViewH = 120;

    OmbicCompressorProcessor& processorRef;
    bool lastCurveDataState_ = false;
    OmbicLookAndFeel ombicLf;
    juce::Image logoWatermark_;

    juce::TextButton optoPill_;
    juce::TextButton fetPill_;
    juce::TextButton pwmPill_;
    juce::TextButton vcaPill_;

    SidechainFilterSection sidechainFilterSection;
    CompressorSection compressorSection;
    SaturatorSection saturatorSection;
    OutputSection outputSection;
    MainViewAsTubeComponent mainViewAsTube_;

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
    std::unique_ptr<SliderAttachment> speedAttachment;
    std::unique_ptr<SliderAttachment> makeupAttachment;
    std::unique_ptr<SliderAttachment> ironAttachment;
    std::unique_ptr<ButtonAttachment> autoGainAttachment;
    std::unique_ptr<SliderAttachment> neonDriveAttachment;
    std::unique_ptr<SliderAttachment> neonIntensityAttachment;
    std::unique_ptr<SliderAttachment> neonToneAttachment;
    std::unique_ptr<SliderAttachment> neonMixAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OmbicCompressorEditorV2)
};
