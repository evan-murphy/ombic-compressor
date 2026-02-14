#pragma once

#include "DataLoader.h"
#include <JuceHeader.h>
#include <map>
#include <vector>
#include <optional>

namespace emulation {

/** Compressor from analyzer data: interpolate gain_reduction_db from compression CSV; optional one-pole envelope from timing CSV. */
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

    /** Process buffer. When attack_param/release_param are set and timing data exists, uses one-pole envelope. */
    void process(juce::AudioBuffer<float>& buffer, double sampleRate,
                 float threshold, std::optional<float> ratio,
                 std::optional<float> attackParam, std::optional<float> releaseParam,
                 int blockSize = 512);

    /** Last gain reduction (dB) applied in process() — for metering. */
    float getLastGainReductionDb() const { return lastGrDb_; }

private:
    void buildCurveCache();
    std::vector<std::tuple<float, float, float, float>> nearestKeys(float threshold, std::optional<float> ratio,
                                                                     std::optional<float> attackMs, std::optional<float> releaseMs) const;

    AnalyzerOutput data_;
    using CurveKey = std::tuple<float, float, float, float>;
    std::map<CurveKey, std::pair<std::vector<float>, std::vector<float>>> curveCache_;
    float envelopeGrDb_ = 0.0f;
    float lastGrDb_ = 0.0f;
};

} // namespace emulation
