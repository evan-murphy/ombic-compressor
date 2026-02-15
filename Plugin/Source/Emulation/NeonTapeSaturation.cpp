#include "NeonTapeSaturation.h"

namespace emulation {

static float onePoleCoeffFromHz(float fcHz, float sampleRate)
{
    if (fcHz <= 0 || sampleRate <= 0) return 0.0f;
    return std::exp(-2.0f * juce::MathConstants<float>::twoPi * fcHz / (float)sampleRate);
}

static float onePoleHighpassCoeffFromHz(float fcHz, float sampleRate)
{
    if (fcHz <= 0 || sampleRate <= 0) return 0.0f;
    float w = 2.0f * juce::MathConstants<float>::pi * fcHz / (float)sampleRate;
    float cosW = std::cos(w);
    float alpha = 2.0f - cosW - std::sqrt((2.0f - cosW) * (2.0f - cosW) - 1.0f);
    return juce::jlimit(0.0f, 1.0f, alpha);
}

NeonTapeSaturation::NeonTapeSaturation(double sampleRate) : sampleRate_(sampleRate)
{
    lpAlpha_ = onePoleCoeffFromHz(modulationBandwidthHz_, (float)sampleRate_);
    hpAlpha_ = onePoleHighpassCoeffFromHz(100.0f, (float)sampleRate_);
    // Faster smoothing (~250 Hz) so gain follows noise more — more audible "neon flicker"
    smoothBeta_ = onePoleCoeffFromHz(250.0f, (float)sampleRate_);
    toneFilterAlpha_ = onePoleCoeffFromHz(toneFilterCutoffHz_, (float)sampleRate_);
}

void NeonTapeSaturation::setDepth(float depth)
{
    depth_ = juce::jlimit(0.0f, 1.0f, depth);
}

void NeonTapeSaturation::setModulationBandwidthHz(float hz)
{
    modulationBandwidthHz_ = juce::jlimit(200.0f, 5000.0f, hz);
    lpAlpha_ = onePoleCoeffFromHz(modulationBandwidthHz_, (float)sampleRate_);
}

void NeonTapeSaturation::setBurstiness(float b)
{
    burstiness_ = juce::jlimit(0.0f, 10.0f, b);
}

void NeonTapeSaturation::setGMin(float g)
{
    gMin_ = juce::jlimit(0.85f, 1.0f, g);
}

void NeonTapeSaturation::setDryWet(float dw)
{
    dryWet_ = juce::jlimit(0.0f, 1.0f, dw);
}

void NeonTapeSaturation::setSaturationIntensity(float intensity)
{
    intensity_ = juce::jlimit(0.0f, 1.0f, intensity);
}

void NeonTapeSaturation::setSaturationAfter(bool after)
{
    saturationAfter_ = after;
}

void NeonTapeSaturation::setToneFilterCutoffHz(float hz)
{
    toneFilterCutoffHz_ = juce::jlimit(80.0f, 20000.0f, hz);
    toneFilterAlpha_ = onePoleCoeffFromHz(toneFilterCutoffHz_, (float)sampleRate_);
}

float NeonTapeSaturation::nextNoise()
{
    float u1 = rng_.nextFloat() + 1e-9f;
    float u2 = rng_.nextFloat();
    float w = std::sqrt(-2.0f * std::log(u1)) * std::cos(juce::MathConstants<float>::twoPi * u2);
    pinkState_[0] = 0.998f * pinkState_[0] + 0.5f * w;
    pinkState_[1] = 0.95f * pinkState_[1] + 0.4f * w;
    pinkState_[2] = 0.8f * pinkState_[2] + 0.3f * w;
    pinkState_[3] = 0.5f * pinkState_[3] + 0.2f * w;
    return 0.4f * (pinkState_[0] + pinkState_[1] + pinkState_[2] + pinkState_[3]);
}

float NeonTapeSaturation::nextBurst()
{
    if (burstiness_ <= 0) return 0.0f;
    float sr = (float)sampleRate_;
    if (nextEventSamples_ <= 0)
    {
        float rate = (burstiness_ > 0) ? (sr / burstiness_) : 1e9f;
        nextEventSamples_ = (rate > 0) ? (float)(-std::log(rng_.nextFloat() + 1e-9f) * rate) : 1e9f;
        float durMs = 1.0f + 19.0f * rng_.nextFloat();
        burstPhaseSamples_ = durMs * 0.001f * sr;
        burstEnvelope_ = 1.0f;
    }
    nextEventSamples_ -= 1.0f;
    if (burstPhaseSamples_ > 0)
    {
        float decay = std::exp(-3.0f / std::max(1.0f, burstPhaseSamples_));
        burstEnvelope_ *= decay;
        burstPhaseSamples_ -= 1.0f;
        // Exaggerated burst level (2x) so neon "discharge" events are clearly audible when burstiness > 0
        return 2.0f * burstEnvelope_ * (2.0f * rng_.nextFloat() - 1.0f);
    }
    return 0.0f;
}

float NeonTapeSaturation::gainFromC(float c) const
{
    float g = 1.0f + c;
    float maxGain = 1.0f + depth_ * modulationScale_;
    return juce::jlimit(gMin_, maxGain, g);
}

float NeonTapeSaturation::advanceModulation()
{
    float noiseRaw = nextNoise() + nextBurst();
    lpState_ = lpAlpha_ * lpState_ + (1.0f - lpAlpha_) * noiseRaw;
    float noiseLp = lpState_;
    float hpIn = noiseLp;
    float hpOut = hpAlpha_ * (hpState_ + hpIn - hpXPrev_);
    hpState_ = hpOut;
    hpXPrev_ = hpIn;
    noiseLp = hpOut;
    runningMean_ = meanAlpha_ * runningMean_ + (1.0f - meanAlpha_) * noiseLp;
    runningVar_ = meanAlpha_ * runningVar_ + (1.0f - meanAlpha_) * (noiseLp - runningMean_) * (noiseLp - runningMean_);
    float sigma = std::sqrt(runningVar_) + 1e-12f;
    float c = depth_ * modulationScale_ * (noiseLp - runningMean_) / sigma;
    float gMod = gainFromC(c);
    smoothState_ = smoothBeta_ * smoothState_ + (1.0f - smoothBeta_) * gMod;
    return smoothState_;
}

void NeonTapeSaturation::process(juce::AudioBuffer<float>& buffer)
{
    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();
    // Intensity 0 = normal (1 + depth*8), Intensity 1 = overblown (1 + depth*32)
    const float intensityMult = 1.0f + intensity_ * 3.0f;
    const float satInputGain = 1.0f + depth_ * 8.0f * intensityMult;
    for (int i = 0; i < numSamples; ++i)
    {
        float gMod = advanceModulation();
        for (int ch = 0; ch < numChannels; ++ch)
        {
            float x = buffer.getSample(ch, i);
            float yMod = gMod * x;
            yMod = std::tanh(satInputGain * yMod);
            // Wet-path tone filter: low cutoff = dark, high = bright (makes Tone slider clearly audible)
            const int chIdx = ch < 2 ? ch : 1;
            toneFilterState_[chIdx] = toneFilterAlpha_ * toneFilterState_[chIdx] + (1.0f - toneFilterAlpha_) * yMod;
            yMod = toneFilterState_[chIdx];
            buffer.setSample(ch, i, (1.0f - dryWet_) * x + dryWet_ * yMod);
        }
    }
}

} // namespace emulation
