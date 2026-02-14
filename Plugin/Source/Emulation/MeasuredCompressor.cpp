#include "MeasuredCompressor.h"
#include <algorithm>
#include <cmath>

namespace emulation {

static float interp1d(const std::vector<float>& x, const std::vector<float>& y, float xq)
{
    if (x.empty() || x.size() != y.size()) return 0.0f;
    if (xq <= x.front()) return y.front();
    if (xq >= x.back()) return y.back();
    size_t i = 0;
    while (i + 1 < x.size() && x[i + 1] <= xq) ++i;
    if (i + 1 >= x.size()) return y.back();
    float t = (xq - x[i]) / (x[i + 1] - x[i]);
    return y[i] + t * (y[i + 1] - y[i]);
}

MeasuredCompressor::MeasuredCompressor(const AnalyzerOutput& data) : data_(data)
{
    buildCurveCache();
}

void MeasuredCompressor::buildCurveCache()
{
    curveCache_.clear();
    if (data_.compressionRows.empty()) return;

    std::map<CurveKey, std::map<float, std::vector<float>>> groups;
    for (const auto& row : data_.compressionRows)
    {
        float t = row.threshold.value_or(0.0f);
        float r = row.ratio.value_or(0.0f);
        float a = row.attackMs.value_or(0.0f);
        float rel = row.releaseMs.value_or(0.0f);
        CurveKey key(t, r, a, rel);
        groups[key][row.inputDb].push_back(row.gainReductionDb);
    }
    for (auto& [key, byInput] : groups)
    {
        std::vector<float> inputDb, grDb;
        for (auto& [inDb, grs] : byInput)
        {
            inputDb.push_back(inDb);
            float mean = 0;
            for (float g : grs) mean += g;
            mean /= (float)grs.size();
            grDb.push_back(mean);
        }
        // Sort by input_db
        std::vector<size_t> idx(inputDb.size());
        for (size_t i = 0; i < idx.size(); ++i) idx[i] = i;
        std::sort(idx.begin(), idx.end(), [&](size_t a, size_t b) { return inputDb[a] < inputDb[b]; });
        std::vector<float> sortedIn, sortedGr;
        for (size_t i : idx) { sortedIn.push_back(inputDb[i]); sortedGr.push_back(grDb[i]); }
        curveCache_[key] = { sortedIn, sortedGr };
    }
}

std::vector<std::tuple<float, float, float, float>> MeasuredCompressor::nearestKeys(
    float threshold, std::optional<float> ratio, std::optional<float> attackMs, std::optional<float> releaseMs) const
{
    if (curveCache_.empty()) return {};
    float q0 = threshold, q1 = ratio.value_or(0.0f), q2 = attackMs.value_or(0.0f), q3 = releaseMs.value_or(0.0f);
    std::vector<CurveKey> keys;
    keys.reserve(curveCache_.size());
    for (auto it = curveCache_.begin(); it != curveCache_.end(); ++it)
        keys.push_back(it->first);
    std::sort(keys.begin(), keys.end(), [&](const CurveKey& a, const CurveKey& b) {
        float da = (std::get<0>(a) - q0) * (std::get<0>(a) - q0) + (std::get<1>(a) - q1) * (std::get<1>(a) - q1)
                 + (std::get<2>(a) - q2) * (std::get<2>(a) - q2) + (std::get<3>(a) - q3) * (std::get<3>(a) - q3);
        float db = (std::get<0>(b) - q0) * (std::get<0>(b) - q0) + (std::get<1>(b) - q1) * (std::get<1>(b) - q1)
                 + (std::get<2>(b) - q2) * (std::get<2>(b) - q2) + (std::get<3>(b) - q3) * (std::get<3>(b) - q3);
        return da < db;
    });
    return keys;
}

float MeasuredCompressor::gainReductionDb(float threshold, float inputDb,
                                          std::optional<float> ratio,
                                          std::optional<float> attackMs,
                                          std::optional<float> releaseMs) const
{
    auto keys = nearestKeys(threshold, ratio, attackMs, releaseMs);
    if (keys.empty()) return 0.0f;
    auto it = curveCache_.find(keys[0]);
    const auto& [x0, y0] = it->second;
    float gr0 = interp1d(x0, y0, inputDb);
    if (keys.size() == 1) return gr0;
    auto it1 = curveCache_.find(keys[1]);
    const auto& [x1, y1] = it1->second;
    float gr1 = interp1d(x1, y1, inputDb);
    float t0 = std::get<0>(keys[0]), t1 = std::get<0>(keys[1]);
    if (std::abs(t1 - t0) < 1e-9f) return gr0;
    float w = (threshold - t0) / (t1 - t0);
    w = juce::jlimit(0.0f, 1.0f, w);
    return (1.0f - w) * gr0 + w * gr1;
}

std::pair<std::optional<float>, std::optional<float>> MeasuredCompressor::getAttackReleaseMs(float attackParam, float releaseParam) const
{
    if (data_.timingRows.empty()) return { {}, {} };
    const TimingRow* best = nullptr;
    float bestDist = 1e30f;
    for (const auto& row : data_.timingRows)
    {
        if (!row.attackParam.has_value() || !row.releaseParam.has_value()) continue;
        float d = (*row.attackParam - attackParam) * (*row.attackParam - attackParam)
                + (*row.releaseParam - releaseParam) * (*row.releaseParam - releaseParam);
        if (d < bestDist) { bestDist = d; best = &row; }
    }
    if (!best) return { {}, {} };
    return { best->attackTimeMs, best->releaseTimeMs };
}

void MeasuredCompressor::process(juce::AudioBuffer<float>& buffer, double sampleRate,
                                 float threshold, std::optional<float> ratio,
                                 std::optional<float> attackParam, std::optional<float> releaseParam,
                                 int blockSize)
{
    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();
    if (numChannels == 0 || numSamples == 0) return;

    // Enable envelope whenever we have attack/release params (FET mode). Timing CSV may have
    // attack_time_ms but often has empty release_time_ms (analyzer didn't measure it); use param
    // values as fallback so the sliders always do something.
    bool useEnvelope = false;
    float attackTimeMs = 10.0f, releaseTimeMs = 100.0f;
    if (attackParam.has_value() && releaseParam.has_value())
    {
        auto [atMs, reMs] = getAttackReleaseMs(*attackParam, *releaseParam);
        attackTimeMs = atMs.has_value() ? std::max(0.1f, *atMs) : (*attackParam / 1000.0f); // attack param in µs
        releaseTimeMs = reMs.has_value() ? std::max(0.1f, *reMs) : *releaseParam;          // release param in ms
        useEnvelope = true;
    }
    float coeffAttack = 1.0f, coeffRelease = 1.0f;
    if (useEnvelope)
    {
        float tauAttackSamp = (attackTimeMs / 1000.0f) * (float)sampleRate;
        float tauReleaseSamp = (releaseTimeMs / 1000.0f) * (float)sampleRate;
        coeffAttack = 1.0f - std::exp(-(float)blockSize / tauAttackSamp);
        coeffRelease = 1.0f - std::exp(-(float)blockSize / tauReleaseSamp);
    }

    int nBlocks = (numSamples + blockSize - 1) / blockSize;
    for (int b = 0; b < nBlocks; ++b)
    {
        int start = b * blockSize;
        int end = std::min(start + blockSize, numSamples);
        int len = end - start;
        float sumSq = 0;
        for (int ch = 0; ch < numChannels; ++ch)
            for (int i = start; i < end; ++i)
                sumSq += buffer.getSample(ch, i) * buffer.getSample(ch, i);
        float rms = std::sqrt(sumSq / (numChannels * len));
        float inputDb = rms <= 1e-10f ? -100.0f : 20.0f * std::log10(rms);
        float targetGrDb = gainReductionDb(threshold, inputDb, ratio, {}, {});

        float grDb;
        if (useEnvelope)
        {
            float coeff = (targetGrDb > envelopeGrDb_) ? coeffAttack : coeffRelease;
            envelopeGrDb_ += (targetGrDb - envelopeGrDb_) * coeff;
            grDb = envelopeGrDb_;
        }
        else
            grDb = targetGrDb;
        lastGrDb_ = grDb;

        float gain = std::pow(10.0f, -grDb / 20.0f);
        for (int ch = 0; ch < numChannels; ++ch)
            buffer.applyGain(ch, start, len, gain);
    }
}

} // namespace emulation
