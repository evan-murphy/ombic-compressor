#include "IronTransformer.h"
#include <cmath>

namespace emulation {

namespace {
// Mode-aware presets: LF shelf gain (dB), HF shelf freq (Hz), asymmetry scale (1 = nominal)
void getModeCoeffs(int mode, float ironAmount, float& lfGainDb, float& hfFreqHz, float& asymmetryScale)
{
    const float amt = juce::jlimit(0.0f, 1.0f, ironAmount);
    switch (mode)
    {
    case 0: // Opto: more HF rolloff, warmer 2nd
        lfGainDb = 2.0f * amt;
        hfFreqHz = 8000.0f + (1.0f - amt) * 2000.0f;
        asymmetryScale = 0.9f;
        break;
    case 1: // FET: tighter LF, more 3rd
        lfGainDb = 1.2f * amt;
        hfFreqHz = 10000.0f + (1.0f - amt) * 2000.0f;
        asymmetryScale = 1.2f;
        break;
    case 2: // PWM: max LF thickening, pronounced HF rolloff
        lfGainDb = 3.0f * amt;
        hfFreqHz = 7000.0f + (1.0f - amt) * 3000.0f;
        asymmetryScale = 1.0f;
        break;
    default:
        lfGainDb = 2.0f * amt;
        hfFreqHz = 10000.0f;
        asymmetryScale = 1.0f;
        break;
    }
}
} // namespace

void IronTransformer::prepare(double sampleRate)
{
    sampleRate_ = sampleRate;
    lfGainDb_ = 2.0f;
    hfFreqHz_ = 10000.0f;
    asymmetryScale_ = 1.0f;
    drive_ = 0.0f;
    asymmetry_ = 0.0f;
    wet_ = 0.0f;
    lfPreCoeffs_ = juce::dsp::IIR::Coefficients<float>::makeLowShelf(
        sampleRate, kLfShelfFreqHz, 0.707f, 1.0f);
    lfPostCoeffs_ = juce::dsp::IIR::Coefficients<float>::makeLowShelf(
        sampleRate, kLfShelfFreqHz, 0.707f, 1.0f);
    hfShelfCoeffs_ = juce::dsp::IIR::Coefficients<float>::makeHighShelf(
        sampleRate, 10000.0f, 0.707f, 1.0f);
    for (int ch = 0; ch < 2; ++ch)
    {
        if (lfPreCoeffs_) lfPre_[ch].coefficients = lfPreCoeffs_;
        if (lfPostCoeffs_) lfPost_[ch].coefficients = lfPostCoeffs_;
        if (hfShelfCoeffs_) hfShelf_[ch].coefficients = hfShelfCoeffs_;
    }
}

void IronTransformer::updateCoeffs(int mode, float ironAmount)
{
    getModeCoeffs(mode, ironAmount, lfGainDb_, hfFreqHz_, asymmetryScale_);
    float amt = juce::jlimit(0.0f, 1.0f, ironAmount);
    drive_ = amt * kMaxDrive;
    asymmetry_ = 0.15f * amt * asymmetryScale_;
    wet_ = amt;

    if (sampleRate_ > 0)
    {
        float lfGainLinear = std::pow(10.0f, lfGainDb_ / 20.0f);
        lfPreCoeffs_ = juce::dsp::IIR::Coefficients<float>::makeLowShelf(
            sampleRate_, kLfShelfFreqHz, 0.707f, lfGainLinear);
        lfPostCoeffs_ = juce::dsp::IIR::Coefficients<float>::makeLowShelf(
            sampleRate_, kLfShelfFreqHz, 0.707f, 1.0f / lfGainLinear);
        float hfGain = 0.5f;
        hfShelfCoeffs_ = juce::dsp::IIR::Coefficients<float>::makeHighShelf(
            sampleRate_, hfFreqHz_, 0.707f, hfGain);
        for (int ch = 0; ch < 2; ++ch)
        {
            if (lfPreCoeffs_) lfPre_[ch].coefficients = lfPreCoeffs_;
            if (lfPostCoeffs_) lfPost_[ch].coefficients = lfPostCoeffs_;
            if (hfShelfCoeffs_) hfShelf_[ch].coefficients = hfShelfCoeffs_;
        }
    }
}

float IronTransformer::waveshape(float x) const
{
    float xDriven = x * (1.0f + drive_);
    float xBiased = xDriven + asymmetry_;
    float y = std::tanh(xBiased) - std::tanh(asymmetry_);
    return y;
}

void IronTransformer::process(juce::AudioBuffer<float>& buffer, int mode, float ironAmount)
{
    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();
    if (numChannels == 0 || numSamples == 0) return;

    ironAmount = juce::jlimit(0.0f, 1.0f, ironAmount);
    if (ironAmount < 0.001f)
        return;

    updateCoeffs(mode, ironAmount);

    int chMax = juce::jmin(numChannels, 2);
    for (int ch = 0; ch < chMax; ++ch)
    {
        float* ptr = buffer.getWritePointer(ch);
        auto& lfPre = lfPre_[ch];
        auto& lfPost = lfPost_[ch];
        auto& hf = hfShelf_[ch];
        for (int i = 0; i < numSamples; ++i)
        {
            float x = ptr[i];
            float xPre = lfPre.processSample(x);
            float y = waveshape(xPre);
            float yDe = lfPost.processSample(y);
            float yHf = hf.processSample(yDe);
            ptr[i] = x + wet_ * (yHf - x);
        }
    }
}

} // namespace emulation
