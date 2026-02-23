#pragma once

#include <JuceHeader.h>
#include "OmbicLookAndFeel.h"
#include "TransferCurveComponent.h"

class OmbicCompressorProcessor;

/** Main VU: focal display with Fancy (transfer curve + In/GR/Out readouts) or Simple (arc meter).
 *  Toggle via "Fancy" / "Simple" buttons; state stored in paramMainVuDisplay (0 = Fancy, 1 = Simple).
 */
class MainVuComponent : public juce::Component,
                        private juce::Timer
{
public:
    explicit MainVuComponent(OmbicCompressorProcessor& processor);
    void resized() override;
    void paint(juce::Graphics& g) override;

    /** Call from editor or timer to sync toggle and visibility from APVTS. */
    void updateFromParameter();

    juce::TextButton& getFancyButton() { return fancyButton_; }
    juce::TextButton& getSimpleButton() { return simpleButton_; }

private:
    void timerCallback() override;

    OmbicCompressorProcessor& proc_;
    OmbicLookAndFeel ombicLf_;

    juce::TextButton fancyButton_;
    juce::TextButton simpleButton_;
    TransferCurveComponent transferCurve_;
    juce::Label inReadout_;
    juce::Label grReadout_;
    juce::Label outReadout_;

    float peakInDb_ = -60.0f;
    float peakOutDb_ = -60.0f;
    float smoothedGrDb_ = 0.0f;
    static constexpr int kMeterHz = 45;
    static constexpr float kPeakAttackCoeff = 0.99f;
    static constexpr float kPeakReleaseCoeff = 0.054f;
    static constexpr float kGrAttackCoeff = 0.77f;
    static constexpr float kGrReleaseCoeff = 0.071f;

    bool isFancy() const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainVuComponent)
};
