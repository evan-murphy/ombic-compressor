#include "PwmChain.h"

namespace emulation {

PwmChain::PwmChain(double sampleRate,
                   bool neonEnable,
                   bool neonBeforeCompressor,
                   float neonDepth,
                   float neonModulationBandwidthHz,
                   float neonBurstiness,
                   float neonGMin,
                   float neonDryWet,
                   bool neonSaturationAfter)
    : sampleRate_(sampleRate)
    , neonBeforeCompressor_(neonBeforeCompressor)
    , neonEnabled_(neonEnable)
{
    pwm_ = std::make_unique<PwmCompressor>();
    pwm_->prepare(sampleRate);

    if (neonEnable)
    {
        neon_ = std::make_unique<NeonTapeSaturation>(sampleRate);
        neon_->setDepth(neonDepth);
        neon_->setModulationBandwidthHz(neonModulationBandwidthHz);
        neon_->setToneFilterCutoffHz(400.0f + (neonModulationBandwidthHz - 200.0f) / 4800.0f * 11600.0f);
        neon_->setBurstiness(neonBurstiness);
        neon_->setGMin(neonGMin);
        neon_->setDryWet(neonDryWet);
        neon_->setSaturationAfter(neonSaturationAfter);
    }
}

void PwmChain::prepare(double sampleRate)
{
    sampleRate_ = sampleRate;
    pwm_->prepare(sampleRate);
}

void PwmChain::process(juce::AudioBuffer<float>& buffer,
                       float thresholdPercent,
                       float ratio,
                       float attackMs,
                       float releaseMs,
                       const juce::AudioBuffer<float>* externalDetectorBuffer)
{
    if (neon_ && neonEnabled_ && neonBeforeCompressor_)
        neon_->process(buffer);

    pwm_->process(buffer, thresholdPercent, ratio, attackMs, releaseMs, externalDetectorBuffer);
}

void PwmChain::setNeonParams(float depth, float modulationBandwidthHz, float toneFilterCutoffHz,
                            float burstiness, float gMin, float dryWet, float intensity, bool saturationAfter)
{
    if (neon_)
    {
        neon_->setDepth(depth);
        neon_->setModulationBandwidthHz(modulationBandwidthHz);
        neon_->setToneFilterCutoffHz(toneFilterCutoffHz);
        neon_->setBurstiness(burstiness);
        neon_->setGMin(gMin);
        neon_->setDryWet(dryWet);
        neon_->setSaturationIntensity(intensity);
        neon_->setSaturationAfter(saturationAfter);
    }
}

} // namespace emulation
