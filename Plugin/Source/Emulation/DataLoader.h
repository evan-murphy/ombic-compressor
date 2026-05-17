#pragma once

#include <JuceHeader.h>
#include <vector>
#include <optional>

namespace emulation {

struct CompressionRow {
    float inputDb = 0;
    float outputDb = 0;
    float gainReductionDb = 0;
    std::optional<float> threshold;
    std::optional<float> ratio;
    std::optional<float> attackMs;
    std::optional<float> releaseMs;
};

struct TimingRow {
    std::optional<float> attackParam;
    std::optional<float> releaseParam;
    std::optional<float> threshold;
    std::optional<float> ratio;
    std::optional<float> attackTimeMs;
    std::optional<float> releaseTimeMs;
};

struct FRRow {
    std::optional<float> frequencyHz;
    std::optional<float> magnitudeDb;
    std::optional<float> driveLevelDb;
};

struct THDRow {
    std::optional<float> levelDb;
    std::optional<float> thdPercent;
};

struct AnalyzerOutput {
    std::vector<CompressionRow> compressionRows;
    std::vector<TimingRow> timingRows;
    std::vector<FRRow> frRows;
    std::vector<THDRow> thdRows;
};

/** Load all analyzer outputs from a directory (compression_curve.csv, timing.csv, frequency_response.csv, thd_vs_level.json). */
AnalyzerOutput loadAnalyzerOutput(const juce::File& outputDir);

} // namespace emulation
