#include "PwmCompressor.h"
#include <cmath>

namespace emulation {

namespace {
constexpr float kPwmInternalHpfHz = 150.0f;
constexpr float kSoftKneeDb = 3.0f;
} // namespace

void PwmCompressor::prepare(double sampleRate)
{
    sampleRate_ = sampleRate;
    envelope_ = 0.0f;
    samplesInGr_ = 0;
    currentGrDb_ = 0.0f;
    lastGrDb_ = 0.0f;
    internalHpfCoeffs_ = juce::dsp::IIR::Coefficients<float>::makeHighPass(
        sampleRate, kPwmInternalHpfHz, 0.7071f);
    if (internalHpfCoeffs_)
        internalHpf_.coefficients = internalHpfCoeffs_;
    for (int i = 0; i < 4; ++i)
        internalHpfState_[i] = 0.0f;
}

float PwmCompressor::speedToCoeff(float timeMs, bool isAttack) const
{
    if (sampleRate_ <= 0) return 0.0f;
    float tauSamples = static_cast<float>(timeMs * 0.001 * sampleRate_);
    if (tauSamples < 1.0f) tauSamples = 1.0f;
    float coeff = 1.0f - std::exp(-1.0f / tauSamples);
    return juce::jlimit(0.0f, 1.0f, coeff);
}

float PwmCompressor::gainComputerDb(float levelDb, float thresholdDb, float ratio) const
{
    float over = levelDb - thresholdDb;
    if (over <= -kSoftKneeDb) return 0.0f;
    if (over >= kSoftKneeDb)
        return over * (1.0f - 1.0f / ratio);
    float t = (over + kSoftKneeDb) / (2.0f * kSoftKneeDb);
    t = t * t * (3.0f - 2.0f * t);
    return t * over * (1.0f - 1.0f / ratio);
}

void PwmCompressor::updateEnvelope(float detectorLevel, int numSamples)
{
    float target = detectorLevel;
    for (int i = 0; i < numSamples; ++i)
    {
        float diff = target - envelope_;
        float coeff = diff >= 0.0f ? attackCoeff_ : releaseCoeff_;
        envelope_ += coeff * diff;
    }
}

void PwmCompressor::process(juce::AudioBuffer<float>& buffer,
                            float thresholdPercent,
                            float ratio,
                            float attackMs,
                            float releaseMs,
                            const juce::AudioBuffer<float>* externalDetector)
{
    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();
    if (numChannels == 0 || numSamples == 0) return;

    thresholdDb_ = -60.0f + (thresholdPercent / 100.0f) * 60.0f;
    attackCoeff_ = speedToCoeff(attackMs, true);
    releaseCoeff_ = speedToCoeff(releaseMs, false);

    if (detectorBuffer_.getNumSamples() < numSamples)
        detectorBuffer_.setSize(1, numSamples);

    const bool useExternal = (externalDetector != nullptr && externalDetector->getNumSamples() >= numSamples);
    const float* extMono = useExternal ? externalDetector->getReadPointer(0) : nullptr;

    float* L = buffer.getWritePointer(0);
    float* R = (numChannels > 1) ? buffer.getWritePointer(1) : nullptr;

    for (int i = 0; i < numSamples; ++i)
    {
        float inL = L[i];
        float inR = R ? R[i] : inL;
        float inMono = 0.5f * (inL + inR);

        float levelLinear = envelope_;
        if (levelLinear < 1e-6f) levelLinear = 1e-6f;
        float levelDb = 20.0f * std::log10(levelLinear);

        float grDb = gainComputerDb(levelDb, thresholdDb_, ratio);
        currentGrDb_ = grDb;
        if (grDb > 0.1f) samplesInGr_++; else samplesInGr_ = 0;

        float holdSec = static_cast<float>(samplesInGr_) / static_cast<float>(sampleRate_);
        float programFactor = 1.0f + kProgramReleaseK * grDb * holdSec;
        float effectiveReleaseMs = releaseMs * juce::jlimit(1.0f, 4.0f, programFactor);
        releaseCoeff_ = speedToCoeff(effectiveReleaseMs, false);

        float gain = std::pow(10.0f, -grDb / 20.0f);
        float outL = inL * gain;
        float outR = inR * gain;
        float outMono = 0.5f * (outL + outR);

        float detectorLevel;
        if (useExternal)
            detectorLevel = std::abs(extMono[i]);
        else
        {
            if (internalHpf_.coefficients != nullptr)
                detectorLevel = std::abs(internalHpf_.processSample(outMono));
            else
                detectorLevel = std::abs(outMono);
        }

        float diff = detectorLevel - envelope_;
        float coeff = diff >= 0.0f ? attackCoeff_ : releaseCoeff_;
        envelope_ += coeff * diff;

        L[i] = outL;
        if (R) R[i] = outR;
    }

    float levelLinear = envelope_;
    if (levelLinear < 1e-6f) levelLinear = 1e-6f;
    float levelDb = 20.0f * std::log10(levelLinear);
    lastGrDb_ = -gainComputerDb(levelDb, thresholdDb_, ratio);
}

} // namespace emulation
