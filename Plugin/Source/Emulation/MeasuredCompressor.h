#pragma once

#include "DataLoader.h"
#include <JuceHeader.h>
#include <map>
#include <vector>
#include <optional>

namespace emulation {

/** Compressor from analyzer data: interpolate gain_reduction_db from compression CSV; optional one-pole envelope from timing CSV.
 *  Opto mode: fixed program-dependent envelope (attack ~10 ms, dual release); optional sidechain LPF (rolloff) and HF shelf (Limit). */
class MeasuredCompressor
{
public:
    explicit MeasuredCompressor(const AnalyzerOutput& data);

    /** Interpolate gain reduction (dB) from measured curve. Opto: pass only threshold (e.g. 25,50,75). FET: threshold + ratio (+ optional attack_ms, release_ms). */
    float gainReductionDb(float threshold, float inputDb,
                         std::optional<float> ratio = {},
                         std::optional<float> attackMs = {},
                         std::optional<float> releaseMs = {}) const;

    /** Interpolate (attack_time_ms, release_time_ms) from timing table. Returns (nullopt, nullopt) if no data. */
    std::pair<std::optional<float>, std::optional<float>> getAttackReleaseMs(float attackParam, float releaseParam) const;

    /** Opto sidechain: rolloff = LPF so bass drives compression more; limit = HF shelf so Limit mode has more HF sensitivity. Call when in Opto mode (and on sample rate change). */
    void setSidechainOptoOptions(bool rolloff, bool limit, double sampleRate);

    /** Process buffer. When attack_param/release_param are set and timing data exists, uses one-pole envelope. When both nullopt (Opto), uses fixed program-dependent envelope.
     *  If externalDetectorBuffer is non-null, level is taken from that buffer (e.g. SC-filtered mono); gain is still applied to buffer. When set, internal Opto LPF/shelf are not applied.
     *  fetCharacter: only used when ratio/attack/release are set (FET mode). 0 = Off (no scale), 1 = Rev A (more GR in knee), 2 = LN (less GR). */
    void process(juce::AudioBuffer<float>& buffer, double sampleRate,
                 float threshold, std::optional<float> ratio,
                 std::optional<float> attackParam, std::optional<float> releaseParam,
                 int blockSize = 512,
                 const juce::AudioBuffer<float>* externalDetectorBuffer = nullptr,
                 std::optional<int> fetCharacter = std::nullopt);

    /** Last gain reduction (dB) applied in process() — for metering. */
    float getLastGainReductionDb() const { return lastGrDb_; }

private:
    void buildCurveCache();
    void ensureSidechainFilters(int numChannels, double sampleRate);
    std::vector<std::tuple<float, float, float, float>> nearestKeys(float threshold, std::optional<float> ratio,
                                                                     std::optional<float> attackMs, std::optional<float> releaseMs) const;

    AnalyzerOutput data_;
    using CurveKey = std::tuple<float, float, float, float>;
    std::map<CurveKey, std::pair<std::vector<float>, std::vector<float>>> curveCache_;
    float envelopeGrDb_ = 0.0f;
    float lastGrDb_ = 0.0f;

    bool sidechainRolloff_ = false;
    bool sidechainLimit_ = false;
    double sidechainSampleRate_ = 48000.0;
    static constexpr int kMaxSidechainChannels = 2;
    std::vector<juce::dsp::IIR::Filter<float>> sidechainLpf_;
    std::vector<juce::dsp::IIR::Filter<float>> sidechainShelf_;
    juce::dsp::IIR::Coefficients<float>::Ptr lpfCoeffs_;
    juce::dsp::IIR::Coefficients<float>::Ptr shelfCoeffs_;
    juce::AudioBuffer<float> sidechainBuffer_;
};

} // namespace emulation
