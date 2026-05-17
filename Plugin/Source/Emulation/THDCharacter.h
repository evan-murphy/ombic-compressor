#pragma once

#include "DataLoader.h"
#include <JuceHeader.h>
#include <vector>

namespace emulation {

/** Tanh saturation tuned from thd_vs_level.json; drive from THD% at reference level. */
class THDCharacter
{
public:
    THDCharacter(const std::vector<THDRow>& thdRows,
                 float referenceLevelDb = -4.0f,
                 float mix = 1.0f);

    void process(juce::AudioBuffer<float>& buffer);

private:
    float drive_ = 1.0f;
    float mix_ = 1.0f;
};

} // namespace emulation
