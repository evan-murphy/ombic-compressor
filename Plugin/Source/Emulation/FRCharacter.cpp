#include "FRCharacter.h"
#include <algorithm>
#include <cmath>
#include <map>

namespace emulation {

static float interp1d(const std::vector<float>& x, const std::vector<float>& y, float xq)
{
    if (x.empty() || x.size() != y.size()) return 1.0f;
    if (xq <= x.front()) return y.front();
    if (xq >= x.back()) return y.back();
    size_t i = 0;
    while (i + 1 < x.size() && x[i + 1] <= xq) ++i;
    if (i + 1 >= x.size()) return y.back();
    float t = (xq - x[i]) / (x[i + 1] - x[i]);
    return y[i] + t * (y[i + 1] - y[i]);
}

FRCharacter::FRCharacter(const std::vector<FRRow>& frRows, double sampleRate,
                         std::optional<float> driveLevelDb, int irLength)
{
    irLength = std::max(1, irLength);
    int order = (int)std::round(std::log2(irLength));
    int nFft = 1 << order;
    ir_.resize((size_t)nFft, 0.0f);
    ir_[0] = 1.0f;
    if (frRows.empty()) return;

    std::vector<FRRow> rows;
    if (driveLevelDb.has_value())
    {
        for (const auto& r : frRows)
            if (r.driveLevelDb.has_value() && std::abs(*r.driveLevelDb - *driveLevelDb) < 0.01f)
                rows.push_back(r);
    }
    if (rows.empty()) rows = frRows;
    if (rows.empty()) return;

    std::map<float, std::vector<float>> byFreq;
    for (const auto& r : rows)
    {
        if (r.frequencyHz.has_value() && r.magnitudeDb.has_value())
            byFreq[*r.frequencyHz].push_back(*r.magnitudeDb);
    }
    std::vector<float> freqs, magDb;
    for (const auto& p : byFreq)
    {
        freqs.push_back(p.first);
        float mean = 0;
        for (float v : p.second) mean += v;
        magDb.push_back(mean / (float)p.second.size());
    }
    if (freqs.empty()) return;
    std::vector<size_t> idx(freqs.size());
    for (size_t i = 0; i < idx.size(); ++i) idx[i] = i;
    std::sort(idx.begin(), idx.end(), [&](size_t a, size_t b) { return freqs[a] < freqs[b]; });
    float meanDb = 0;
    for (size_t i : idx) meanDb += magDb[i];
    meanDb /= (float)idx.size();
    std::vector<float> magLinear(idx.size());
    for (size_t i = 0; i < idx.size(); ++i)
        magLinear[i] = std::pow(10.0f, (magDb[idx[i]] - meanDb) / 20.0f);
    std::vector<float> sortedFreqs;
    for (size_t i : idx) sortedFreqs.push_back(freqs[i]);

    int nBins = nFft / 2 + 1;
    std::vector<float> binFreqs((size_t)nBins);
    for (int i = 0; i < nBins; ++i)
        binFreqs[(size_t)i] = (float)(i * sampleRate / nFft);
    std::vector<float> magAtBins((size_t)nBins);
    for (int i = 0; i < nBins; ++i)
        magAtBins[(size_t)i] = interp1d(sortedFreqs, magLinear, binFreqs[(size_t)i]);

    // JUCE real-only FFT buffer: [Re(0), Re(Nyquist), Re(1), Im(1), Re(2), Im(2), ...]
    std::vector<float> fftBuffer((size_t)nFft);
    fftBuffer[0] = magAtBins[0];
    fftBuffer[1] = (nBins > 1) ? magAtBins[(size_t)(nBins - 1)] : magAtBins[0];
    for (int i = 1; i < nBins - 1; ++i)
    {
        fftBuffer[2 * i] = magAtBins[(size_t)i];
        fftBuffer[2 * i + 1] = 0.0f;
    }
    juce::dsp::FFT fft((size_t)order);
    fft.performRealOnlyInverseTransform(fftBuffer.data());
    for (int i = 0; i < nFft; ++i)
        ir_[(size_t)i] = fftBuffer[(size_t)i];
    // Roll to center for linear phase
    int half = nFft / 2;
    std::vector<float> shifted((size_t)nFft);
    for (int i = 0; i < nFft; ++i)
        shifted[(size_t)((i + half) % nFft)] = ir_[(size_t)i];
    ir_ = shifted;
    float sum = 0;
    for (float v : ir_) sum += std::abs(v);
    if (sum > 1e-12f)
        for (float& v : ir_) v /= sum;
    valid_ = true;
}

void FRCharacter::process(juce::AudioBuffer<float>& buffer)
{
    if (!valid_ || ir_.empty()) return;
    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();
    int N = (int)ir_.size();
    int half = N / 2;
    for (int ch = 0; ch < numChannels; ++ch)
    {
        auto* ptr = buffer.getWritePointer(ch);
        for (int i = 0; i < numSamples; ++i)
        {
            float y = 0;
            for (int k = 0; k < N; ++k)
            {
                int idx = i - half + k;
                if (idx >= 0 && idx < numSamples)
                    y += ir_[(size_t)k] * ptr[idx];
            }
            ptr[i] = y;
        }
    }
}

} // namespace emulation
