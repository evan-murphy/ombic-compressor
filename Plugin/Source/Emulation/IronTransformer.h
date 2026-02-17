#pragma once

#include <JuceHeader.h>
#include <array>

namespace emulation {

/** Transformer saturation: asymmetric waveshaper + LF/HF shaping. Mode-aware coefficients (Opto/FET/PWM). */
class IronTransformer
{
public:
    IronTransformer() = default;

    void prepare(double sampleRate);
    /** Process buffer. mode: 0=Opto, 1=FET, 2=PWM. ironAmount: 0–1 (0=bypass). */
    void process(juce::AudioBuffer<float>& buffer, int mode, float ironAmount);

private:
    void updateCoeffs(int mode, float ironAmount);
    float waveshape(float x) const;

    double sampleRate_ = 48000.0;
    float drive_ = 0.0f;
    float asymmetry_ = 0.0f;
    float wet_ = 0.0f;

    // Pre-emphasis (LF shelf boost), de-emphasis (cut to restore flat), HF shelf. Per-channel state.
    std::array<juce::dsp::IIR::Filter<float>, 2> lfPre_;
    std::array<juce::dsp::IIR::Filter<float>, 2> lfPost_;
    std::array<juce::dsp::IIR::Filter<float>, 2> hfShelf_;
    juce::dsp::IIR::Coefficients<float>::Ptr lfPreCoeffs_;
    juce::dsp::IIR::Coefficients<float>::Ptr lfPostCoeffs_;
    juce::dsp::IIR::Coefficients<float>::Ptr hfShelfCoeffs_;

    // Mode-aware coefficient set (LF gain dB, HF freq Hz, asymmetry scale)
    float lfGainDb_ = 2.0f;
    float hfFreqHz_ = 10000.0f;
    float asymmetryScale_ = 1.0f;

    static constexpr float kLfShelfFreqHz = 200.0f;
    static constexpr float kMaxDrive = 2.5f;
};

} // namespace emulation
