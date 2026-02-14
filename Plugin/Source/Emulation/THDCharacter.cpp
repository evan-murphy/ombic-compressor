#include "THDCharacter.h"
#include <cmath>

namespace emulation {

THDCharacter::THDCharacter(const std::vector<THDRow>& thdRows,
                           float referenceLevelDb, float mix)
{
    mix_ = juce::jlimit(0.0f, 1.0f, mix);
    float thdPct = 0.0f;
    for (const auto& r : thdRows)
    {
        if (r.levelDb.has_value() && std::abs(*r.levelDb - referenceLevelDb) < 0.1f)
        {
            if (r.thdPercent.has_value()) thdPct = *r.thdPercent;
            break;
        }
    }
    if (thdPct <= 0.0f)
    {
        for (const auto& r : thdRows)
            if (r.thdPercent.has_value() && *r.thdPercent > thdPct)
                thdPct = *r.thdPercent;
    }
    float k = 2.0f;
    drive_ = 1.0f + k * (thdPct / 100.0f);
    drive_ = juce::jlimit(1.0f, 4.0f, drive_);
}

void THDCharacter::process(juce::AudioBuffer<float>& buffer)
{
    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();
    float scale = 1.0f / (std::tanh(drive_) + 1e-12f);
    for (int ch = 0; ch < numChannels; ++ch)
    {
        auto* ptr = buffer.getWritePointer(ch);
        for (int i = 0; i < numSamples; ++i)
        {
            float x = ptr[i];
            float sat = std::tanh(x * drive_) * scale;
            ptr[i] = mix_ * sat + (1.0f - mix_) * x;
        }
    }
}

} // namespace emulation
