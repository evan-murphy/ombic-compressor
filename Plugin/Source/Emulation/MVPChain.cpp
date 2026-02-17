#include "MVPChain.h"
#include "DataLoader.h"
#include <cmath>

namespace emulation {

MVPChain::MVPChain(Mode mode, double sampleRate,
                   const juce::File& fetishDataDir,
                   const juce::File& lalaDataDir,
                   const juce::File& vcaDataDir,
                   bool characterFr,
                   bool characterThd,
                   std::optional<float> characterFrDriveDb,
                   float characterThdMix,
                   bool neonEnable,
                   bool neonBeforeCompressor,
                   float neonDepth,
                   float neonModulationBandwidthHz,
                   float neonBurstiness,
                   float neonGMin,
                   float neonDryWet,
                   bool neonSaturationAfter)
    : mode_(mode)
    , sampleRate_(sampleRate)
    , neonBeforeCompressor_(neonBeforeCompressor)
{
    juce::File dataDir = (mode == Mode::VCA) ? vcaDataDir : ((mode == Mode::FET) ? fetishDataDir : lalaDataDir);
    AnalyzerOutput data = loadAnalyzerOutput(dataDir);
    compressor_ = std::make_unique<MeasuredCompressor>(data);

    if (characterFr && !data.frRows.empty())
        frCharacter_ = std::make_unique<FRCharacter>(data.frRows, sampleRate, characterFrDriveDb);
    if (characterThd && !data.thdRows.empty())
        thdCharacter_ = std::make_unique<THDCharacter>(data.thdRows, -4.0f, characterThdMix);
    neonEnabled_ = neonEnable;
    neonBeforeCompressor_ = neonBeforeCompressor;
    if (neonEnable)
    {
        neon_ = std::make_unique<NeonTapeSaturation>(sampleRate);
        neon_->setDepth(neonDepth);
        neon_->setModulationBandwidthHz(neonModulationBandwidthHz);
        neon_->setToneFilterCutoffHz(400.0f + (neonModulationBandwidthHz - 200.0f) / 4800.0f * 11600.0f); // 0..1 tone -> 400 Hz..12 kHz
        neon_->setBurstiness(neonBurstiness);
        neon_->setGMin(neonGMin);
        neon_->setDryWet(neonDryWet);
        neon_->setSaturationAfter(neonSaturationAfter);
    }
}

void MVPChain::process(juce::AudioBuffer<float>& buffer,
                       float threshold,
                       std::optional<float> ratio,
                       std::optional<float> attackParam,
                       std::optional<float> releaseParam,
                       int blockSize,
                       std::optional<bool> optoLimitMode,
                       const juce::AudioBuffer<float>* externalDetectorBuffer)
{
    if (mode_ == Mode::Opto)
    {
        ratio = std::nullopt;
        attackParam = std::nullopt;
        releaseParam = std::nullopt;
    }
    else if (mode_ == Mode::VCA)
    {
        // VCA curve has no timing (program-dependent in reference); pass nullopt so MeasuredCompressor uses 0,0 in cache lookup
        attackParam = std::nullopt;
        releaseParam = std::nullopt;
    }

    if (neon_ && neonEnabled_ && neonBeforeCompressor_)
        neon_->process(buffer);

    if (compressor_)
    {
        if (mode_ == Mode::Opto && externalDetectorBuffer == nullptr)
            compressor_->setSidechainOptoOptions(true, optoLimitMode.value_or(false), sampleRate_);
        else if (mode_ == Mode::Opto && externalDetectorBuffer != nullptr)
            compressor_->setSidechainOptoOptions(false, false, sampleRate_);  // external detector: no internal Opto LPF/shelf
        compressor_->process(buffer, sampleRate_, threshold, ratio, attackParam, releaseParam, blockSize, externalDetectorBuffer);
        lastGrDb_ = compressor_->getLastGainReductionDb();
        // Opto curve is gentle; apply extra gain reduction so it can sound more aggressive
        if (mode_ == Mode::Opto)
        {
            float extraGrDb = lastGrDb_ * 1.0f; // same again = 2x total GR
            float extraGain = std::pow(10.0f, -extraGrDb / 20.0f);
            buffer.applyGain(extraGain);
            lastGrDb_ += extraGrDb;
        }
    }

    if (frCharacter_)
        frCharacter_->process(buffer);
    if (thdCharacter_)
        thdCharacter_->process(buffer);
    if (neon_ && neonEnabled_ && !neonBeforeCompressor_)
        neon_->process(buffer);
}

void MVPChain::setNeonParams(float depth, float modulationBandwidthHz, float toneFilterCutoffHz, float burstiness, float gMin, float dryWet, float intensity, bool saturationAfter)
{
    if (neon_)
    {
        neon_->setDepth(depth);
        neon_->setModulationBandwidthHz(modulationBandwidthHz);
        neon_->setToneFilterCutoffHz(toneFilterCutoffHz);
        neon_->setBurstiness(burstiness);
        neon_->setGMin(gMin);
        neon_->setDryWet(dryWet);
        neon_->setSaturationIntensity(intensity);
        neon_->setSaturationAfter(saturationAfter);
    }
}

} // namespace emulation
