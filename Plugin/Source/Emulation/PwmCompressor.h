#pragma once

#include <JuceHeader.h>
#include <optional>

namespace emulation {

/** PWM (feedback-topology) compressor: clean gain, detector reads output. No curve data. */
class PwmCompressor
{
public:
    PwmCompressor() = default;

    void prepare(double sampleRate);
    /** Process buffer. thresholdPercent 0–100, ratio 1.5–8, attackMs/releaseMs from Speed mapping.
     *  externalDetector: when non-null, use for level detection; when null, use internal 150 Hz HPF on output. */
    void process(juce::AudioBuffer<float>& buffer,
                 float thresholdPercent,
                 float ratio,
                 float attackMs,
                 float releaseMs,
                 const juce::AudioBuffer<float>* externalDetector);

    float getLastGainReductionDb() const { return lastGrDb_; }

private:
    float gainComputerDb(float levelDb, float thresholdDb, float ratio) const;
    void updateEnvelope(float detectorLevel, int numSamples);
    float speedToCoeff(float timeMs, bool isAttack) const;

    double sampleRate_ = 48000.0;
    float envelope_ = 0.0f;
    float attackCoeff_ = 0.0f;
    float releaseCoeff_ = 0.0f;
    float thresholdDb_ = -30.0f;
    mutable float lastGrDb_ = 0.0f;

    // Program-dependent release: extend release when GR is deep and sustained
    int samplesInGr_ = 0;
    float currentGrDb_ = 0.0f;
    static constexpr float kProgramReleaseK = 0.3f;

    // Internal 150 Hz HPF (Butterworth) when no external sidechain
    juce::dsp::IIR::Filter<float> internalHpf_;
    juce::dsp::IIR::Coefficients<float>::Ptr internalHpfCoeffs_;
    float internalHpfState_[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

    juce::AudioBuffer<float> detectorBuffer_;
};

} // namespace emulation
