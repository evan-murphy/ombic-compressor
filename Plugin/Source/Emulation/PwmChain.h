#pragma once

#include "PwmCompressor.h"
#include "NeonTapeSaturation.h"
#include <JuceHeader.h>
#include <memory>

namespace emulation {

/** PWM mode chain: Neon (optional) + PwmCompressor. No curve data. */
class PwmChain
{
public:
    explicit PwmChain(double sampleRate,
                      bool neonEnable = true,
                      bool neonBeforeCompressor = true,
                      float neonDepth = 0.02f,
                      float neonModulationBandwidthHz = 1000.0f,
                      float neonBurstiness = 0.0f,
                      float neonGMin = 0.92f,
                      float neonDryWet = 1.0f,
                      bool neonSaturationAfter = false);

    void prepare(double sampleRate);

    /** thresholdPercent 0–100, ratio 1.5–8, attackMs/releaseMs from Speed mapping.
     *  externalDetectorBuffer: when non-null (SC active), use for detector; else internal 150 Hz HPF. */
    void process(juce::AudioBuffer<float>& buffer,
                 float thresholdPercent,
                 float ratio,
                 float attackMs,
                 float releaseMs,
                 const juce::AudioBuffer<float>* externalDetectorBuffer);

    void setNeonParams(float depth, float modulationBandwidthHz, float toneFilterCutoffHz,
                      float burstiness, float gMin, float dryWet, float intensity, bool saturationAfter);
    void setNeonEnabled(bool enabled) { neonEnabled_ = enabled; }
    void setNeonBeforeCompressor(bool before) { neonBeforeCompressor_ = before; }

    float getLastGainReductionDb() const { return pwm_->getLastGainReductionDb(); }

private:
    double sampleRate_;
    std::unique_ptr<NeonTapeSaturation> neon_;
    std::unique_ptr<PwmCompressor> pwm_;
    bool neonBeforeCompressor_ = true;
    bool neonEnabled_ = true;
};

} // namespace emulation
