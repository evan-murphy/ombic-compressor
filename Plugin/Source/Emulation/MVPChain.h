#pragma once

#include "DataLoader.h"
#include "MeasuredCompressor.h"
#include "FRCharacter.h"
#include "THDCharacter.h"
#include "NeonTapeSaturation.h"
#include <JuceHeader.h>
#include <memory>
#include <optional>

namespace emulation {

/** Single entry point: FET or Opto mode, optional FR/THD character, optional neon before/after. Matches docs/mvp_usage.md. */
class MVPChain
{
public:
    enum class Mode { FET, Opto };

    MVPChain(Mode mode, double sampleRate,
             const juce::File& fetishDataDir,
             const juce::File& lalaDataDir,
             bool characterFr = false,
             bool characterThd = false,
             std::optional<float> characterFrDriveDb = {},
             float characterThdMix = 1.0f,
             bool neonEnable = false,
             bool neonBeforeCompressor = false,
             float neonDepth = 0.02f,
             float neonModulationBandwidthHz = 1000.0f,
             float neonBurstiness = 0.0f,
             float neonGMin = 0.92f,
             float neonDryWet = 1.0f,
             bool neonSaturationAfter = false);

    /** Process buffer. FET: threshold (dB), ratio, attack_param, release_param. Opto: threshold (0–100). optoLimitMode: when Opto, true = Limit (more HF in sidechain). */
    void process(juce::AudioBuffer<float>& buffer,
                 float threshold,
                 std::optional<float> ratio,
                 std::optional<float> attackParam,
                 std::optional<float> releaseParam,
                 int blockSize = 512,
                 std::optional<bool> optoLimitMode = std::nullopt);

    void setNeonParams(float depth, float modulationBandwidthHz, float toneFilterCutoffHz, float burstiness, float gMin, float dryWet, float intensity, bool saturationAfter);
    void setNeonEnabled(bool enabled) { neonEnabled_ = enabled; }
    void setNeonBeforeCompressor(bool before) { neonBeforeCompressor_ = before; }

    MeasuredCompressor* getCompressor() { return compressor_.get(); }
    float getLastGainReductionDb() const { return lastGrDb_; }

private:
    Mode mode_;
    double sampleRate_;
    std::unique_ptr<MeasuredCompressor> compressor_;
    std::unique_ptr<FRCharacter> frCharacter_;
    std::unique_ptr<THDCharacter> thdCharacter_;
    std::unique_ptr<NeonTapeSaturation> neon_;
    bool neonBeforeCompressor_ = false;
    bool neonEnabled_ = false;
    mutable float lastGrDb_ = 0.0f;
};

} // namespace emulation
