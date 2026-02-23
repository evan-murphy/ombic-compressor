#pragma once

#include <JuceHeader.h>
#include "OmbicLookAndFeel.h"
#include "TransferCurveComponent.h"

class OmbicCompressorProcessor;

/** v2: Main view *is* the tube. Draws saturation-driven background glow, transfer curve, In/GR/Out readouts, and filament (waveform). */
class MainViewAsTubeComponent : public juce::Component,
                                private juce::Timer
{
public:
    explicit MainViewAsTubeComponent(OmbicCompressorProcessor& processor);
    void resized() override;
    void paint(juce::Graphics& g) override;
    void paintOverChildren(juce::Graphics& g) override;

private:
    void timerCallback() override;
    void paintTubeGlow(juce::Graphics& g);
    void paintFilament(juce::Graphics& g);

    OmbicCompressorProcessor& proc_;
    OmbicLookAndFeel ombicLf_;
    TransferCurveComponent transferCurve_;
    juce::Label inReadout_;
    juce::Label grReadout_;
    juce::Label outReadout_;

    float peakInDb_  = -60.0f;
    float peakOutDb_  = -60.0f;
    float smoothedGrDb_ = 0.0f;
    static constexpr float kPeakAttackCoeff = 0.99f;
    static constexpr float kPeakReleaseCoeff = 0.054f;
    static constexpr float kGrAttackCoeff = 0.77f;
    static constexpr float kGrReleaseCoeff = 0.071f;
    float filamentPhase_ = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainViewAsTubeComponent)
};
