#pragma once

#include <JuceHeader.h>
#include <cmath>

namespace emulation {

/** Neon-style tape saturation: stochastic gain modulation → multiply → optional tanh. Matches Python NeonTapeSaturation. */
class NeonTapeSaturation
{
public:
    explicit NeonTapeSaturation(double sampleRate);

    /** depth: 0..1 — modulation amount and saturation drive (audible tanh scales with this).
     *  Modulation range is exaggerated (effective max gain = 1 + depth * modulationScale_) so the neon wobble is clearly audible. */
    void setDepth(float depth);
    void setModulationBandwidthHz(float hz);
    void setBurstiness(float b);
    void setGMin(float g);
    void setDryWet(float dw);
    /** 0 = normal range, 1 = can overblow (scales saturation gain). */
    void setSaturationIntensity(float intensity);
    void setSaturationAfter(bool after);
    /** Wet-path lowpass cutoff (Hz). Lower = darker, higher = brighter. Makes Tone slider clearly audible. */
    void setToneFilterCutoffHz(float hz);

    void process(juce::AudioBuffer<float>& buffer);

private:
    float nextNoise();
    float nextBurst();
    float gainFromC(float c) const;
    float advanceModulation();

    double sampleRate_ = 48000.0;
    float depth_ = 0.02f;
    float modulationBandwidthHz_ = 1000.0f;
    float burstiness_ = 0.0f;
    float gMin_ = 0.92f;
    float dryWet_ = 1.0f;
    float intensity_ = 0.0f;
    bool saturationAfter_ = false;

    float lpAlpha_ = 0.0f;
    float hpAlpha_ = 0.0f;
    float smoothBeta_ = 0.0f;

    float lpState_ = 0.0f;
    float hpState_ = 0.0f;
    float hpXPrev_ = 0.0f;
    float smoothState_ = 1.0f;
    float toneFilterCutoffHz_ = 8000.0f;
    float toneFilterAlpha_ = 0.0f;
    float toneFilterState_[2] = { 0.0f, 0.0f }; // per-channel one-pole state
    float runningMean_ = 0.0f;
    float runningVar_ = 1.0f;
    float pinkState_[4] = { 0, 0, 0, 0 };
    float burstPhaseSamples_ = 0.0f;
    float burstEnvelope_ = 0.0f;
    float nextEventSamples_ = 0.0f;

    juce::Random rng_;
    static constexpr float meanAlpha_ = 0.9999f;
    /** Exaggeration: scale modulation so gain can deviate further (more audible neon wobble). 2.f = at full depth, gain can reach 3x. */
    static constexpr float modulationScale_ = 2.0f;
};

} // namespace emulation
