#pragma once

#include "DataLoader.h"
#include <JuceHeader.h>
#include <vector>
#include <optional>

namespace emulation {

/** Apply magnitude curve from frequency_response.csv as linear-phase FIR EQ. */
class FRCharacter
{
public:
    FRCharacter(const std::vector<FRRow>& frRows, double sampleRate,
                std::optional<float> driveLevelDb = {},
                int irLength = 256);

    void process(juce::AudioBuffer<float>& buffer);

private:
    std::vector<float> ir_;
    bool valid_ = false;
};

} // namespace emulation
