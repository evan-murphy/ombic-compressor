#pragma once

#include <JuceHeader.h>
#include <memory>
#include <vector>

namespace emulation { class MVPChain; class NeonTapeSaturation; class PwmChain; class IronTransformer; }

//==============================================================================
class OmbicCompressorProcessor : public juce::AudioProcessor
{
public:
    OmbicCompressorProcessor();
    ~OmbicCompressorProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Ombic Compressor"; }

    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState& getValueTreeState() { return apvts; }
    const juce::AudioProcessorValueTreeState& getValueTreeState() const { return apvts; }

    // Meter values (written on audio thread, read on message thread)
    std::atomic<float> inputLevelDb{ -60.0f };
    std::atomic<float> gainReductionDb{ 0.0f };
    std::atomic<float> outputLevelDb{ -60.0f };

    /** True after ensureChains() has successfully loaded at least one curve set (FET or Opto). */
    bool hasCurveDataLoaded() const { return curveDataLoaded_.load(); }

    static const char* paramCompressorMode;
    static const char* paramThreshold;
    static const char* paramRatio;
    static const char* paramAttack;
    static const char* paramRelease;
    static const char* paramMakeupGainDb;
    static const char* paramNeonEnable;
    static const char* paramNeonDrive;
    static const char* paramNeonTone;
    static const char* paramNeonMix;
    static const char* paramNeonIntensity;
    static const char* paramNeonBurstiness;
    static const char* paramNeonGMin;
    static const char* paramNeonSaturationAfter;
    static const char* paramOptoCompressLimit;
    static const char* paramScFrequency;
    static const char* paramScListen;
    static const char* paramMainVuDisplay;
    static const char* paramPwmSpeed;
    static const char* paramIron;
    static const char* paramAutoGain;

    /** True when SC Listen is active (for header indicator). */
    bool isScListenActive() const;

    /** Copy of latest sidechain buffer for scope when Listen is on. Returns true if out was filled (Listen active and we have samples). Call from message thread only. */
    bool getScopeSidechainSamples(std::vector<float>& out) const;

    /** Copy of latest main output (mono) for Neon scope when Listen is off. Returns true if out was filled. Call from message thread only. */
    bool getScopeWaveformSamples(std::vector<float>& out) const;

private:
    std::atomic<bool> curveDataLoaded_{ false };

    juce::AudioProcessorValueTreeState apvts;
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    double sampleRateHz = 48000.0;
    juce::LinearSmoothedValue<float> inputRms;
    juce::LinearSmoothedValue<float> outputRms;

    juce::File dataRoot_;
    std::unique_ptr<emulation::MVPChain> fetChain_;
    std::unique_ptr<emulation::MVPChain> optoChain_;
    std::unique_ptr<emulation::MVPChain> vcaChain_;
    std::unique_ptr<emulation::PwmChain> pwmChain_;
    std::unique_ptr<emulation::IronTransformer> iron_;
    std::unique_ptr<emulation::NeonTapeSaturation> standaloneNeon_;
    void ensureChains();
    void ensurePwmChain();
    /** Parameter-based estimate of makeup gain (dB) for Auto Gain. Uses nominal threshold/ratio/speed. */
    float estimateMakeupDb(int mode, float thresholdRaw, float ratio, float attackParam, float releaseParam, float speedParam) const;

    // Sidechain filter module: HPF on mono sum for detector; true bypass at 20 Hz
    static constexpr float kScFilterOffHz = 20.0f;
    juce::dsp::IIR::Filter<float> sidechainHpf_;
    juce::dsp::IIR::Coefficients<float>::Ptr sidechainHpfCoeffs_;
    juce::SmoothedValue<float> smoothedScFrequency_;
    juce::AudioBuffer<float> sidechainMonoBuffer_;
    juce::AudioBuffer<float> sidechainStereoForListen_;
    void updateSidechainFilterCoeffs(float frequencyHz);

    // Scope: when Listen is on, copy latest sidechain block for Neon scope (audio thread writes, message thread reads)
    mutable juce::CriticalSection scopeSidechainLock_;
    std::vector<float> scopeSidechainBuffer_;
    // Scope: when Listen is off, copy latest main output (mono) so Neon tube can show real waveform
    mutable juce::CriticalSection scopeWaveformLock_;
    std::vector<float> scopeWaveformBuffer_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OmbicCompressorProcessor)
};
