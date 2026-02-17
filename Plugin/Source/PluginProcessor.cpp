#include "PluginProcessor.h"
#include "PluginEditor.h"
#if OMBIC_USE_V2_EDITOR
#include "PluginEditorV2.h"
#endif
#include "Emulation/DataLoader.h"
#include "Emulation/MVPChain.h"
#include "Emulation/PwmChain.h"
#include "Emulation/IronTransformer.h"
#if JUCE_MAC
#include <dlfcn.h>
#endif

//==============================================================================
namespace
{
/** On macOS, resolve plugin bundle's Contents/Resources/CurveData (bundled at build time). */
juce::File getBundledCurveDataRoot()
{
#if JUCE_MAC
    Dl_info info{};
    // Use a symbol from this TU so dladdr returns our plugin module path when loaded in host
    if (dladdr((void*)&getBundledCurveDataRoot, &info) != 0 && info.dli_fname != nullptr)
    {
        juce::File moduleFile(juce::String::fromUTF8(info.dli_fname));
        juce::File exe = moduleFile.getLinkedTarget();
        if (!exe.exists())
            exe = moduleFile;
        juce::File macosDir = exe.getParentDirectory();
        if (macosDir.getFileName() != "MacOS")
            return {};
        juce::File contents = macosDir.getParentDirectory();
        juce::File bundleRoot = contents.getParentDirectory();
        juce::File curveData = bundleRoot.getChildFile("Contents/Resources/CurveData");
        if (curveData.getChildFile("fetish_v2/compression_curve.csv").existsAsFile()
            || curveData.getChildFile("dbcomp_vca/compression_curve.csv").existsAsFile())
            return curveData;
    }
#endif
    return {};
}
}

//==============================================================================
const char* OmbicCompressorProcessor::paramCompressorMode    = "compressor_mode";
const char* OmbicCompressorProcessor::paramThreshold         = "threshold";
const char* OmbicCompressorProcessor::paramRatio             = "ratio";
const char* OmbicCompressorProcessor::paramAttack            = "attack";
const char* OmbicCompressorProcessor::paramRelease           = "release";
const char* OmbicCompressorProcessor::paramMakeupGainDb      = "makeup_gain_db";
const char* OmbicCompressorProcessor::paramNeonEnable        = "neon_enable";
const char* OmbicCompressorProcessor::paramNeonDrive         = "neon_drive";
const char* OmbicCompressorProcessor::paramNeonTone          = "neon_tone";
const char* OmbicCompressorProcessor::paramNeonMix           = "neon_mix";
const char* OmbicCompressorProcessor::paramNeonIntensity      = "neon_intensity";
const char* OmbicCompressorProcessor::paramNeonBurstiness    = "neon_burstiness";
const char* OmbicCompressorProcessor::paramNeonGMin          = "neon_g_min";
const char* OmbicCompressorProcessor::paramNeonSaturationAfter = "neon_saturation_after";
const char* OmbicCompressorProcessor::paramOptoCompressLimit   = "opto_compress_limit";
const char* OmbicCompressorProcessor::paramScFrequency        = "sc_frequency";
const char* OmbicCompressorProcessor::paramScListen             = "sc_listen";
const char* OmbicCompressorProcessor::paramMainVuDisplay        = "main_vu_display";
const char* OmbicCompressorProcessor::paramPwmSpeed           = "pwm_speed";
const char* OmbicCompressorProcessor::paramIron                = "iron";
const char* OmbicCompressorProcessor::paramAutoGain            = "auto_gain";

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout OmbicCompressorProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{ paramCompressorMode, 1 },
        "Compressor Mode",
        juce::StringArray{ "Opto", "FET", "PWM", "VCA" },
        0));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ paramThreshold, 1 },
        "Threshold",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f, 1.0f),
        50.0f,
        juce::AudioParameterFloatAttributes().withLabel("%")));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ paramRatio, 1 },
        "Ratio",
        juce::NormalisableRange<float>(1.0f, 20.0f, 0.1f, 0.5f),
        4.0f,
        juce::AudioParameterFloatAttributes().withLabel(":1")));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ paramAttack, 1 },
        "Attack",
        juce::NormalisableRange<float>(20.0f, 800.0f, 1.0f, 0.5f),
        410.0f,
        juce::AudioParameterFloatAttributes().withLabel("")));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ paramRelease, 1 },
        "Release",
        juce::NormalisableRange<float>(50.0f, 1100.0f, 1.0f, 0.4f),
        200.0f,
        juce::AudioParameterFloatAttributes().withLabel("ms")));

    // Output (makeup/trim) capped at +12 dB max for safe listening; -24 dB for trim.
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ paramMakeupGainDb, 1 },
        "Output",
        juce::NormalisableRange<float>(-24.0f, 12.0f, 0.1f, 1.0f),
        0.0f,
        juce::AudioParameterFloatAttributes().withLabel("dB")));

    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{ paramNeonEnable, 1 },
        "Neon Saturator On",
        false));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ paramNeonDrive, 1 },
        "Drive",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f, 1.0f),
        0.4f,
        juce::AudioParameterFloatAttributes().withLabel("")));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ paramNeonTone, 1 },
        "Tone",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f, 1.0f),
        0.5f,
        juce::AudioParameterFloatAttributes().withLabel("")));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ paramNeonMix, 1 },
        "Mix",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f, 1.0f),
        1.0f,
        juce::AudioParameterFloatAttributes().withLabel("%")));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ paramNeonIntensity, 1 },
        "Intensity",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f, 1.0f),
        0.0f,
        juce::AudioParameterFloatAttributes().withLabel("%")));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ paramNeonBurstiness, 1 },
        "Burstiness",
        juce::NormalisableRange<float>(0.0f, 10.0f, 0.1f, 0.5f),
        0.0f,
        juce::AudioParameterFloatAttributes().withLabel("")));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ paramNeonGMin, 1 },
        "G Min",
        juce::NormalisableRange<float>(0.85f, 1.0f, 0.01f, 1.0f),
        0.92f,
        juce::AudioParameterFloatAttributes().withLabel("")));

    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{ paramNeonSaturationAfter, 1 },
        "Soft Sat After",
        false));

    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{ paramOptoCompressLimit, 1 },
        "Compress / Limit",
        juce::StringArray{ "Compress", "Limit" },
        0));

    // Sidechain filter: 20–500 Hz, default 20 = OFF (bypass). Skew 0.35 for log-ish (more resolution 20–200).
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ paramScFrequency, 1 },
        "SC Frequency",
        juce::NormalisableRange<float>(20.0f, 500.0f, 1.0f, 0.35f),
        20.0f,
        juce::AudioParameterFloatAttributes().withLabel("Hz")));

    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{ paramScListen, 1 },
        "SC Listen",
        false));

    // Main VU display: 0 = Fancy (transfer curve + readouts), 1 = Simple (arc meter)
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{ paramMainVuDisplay, 1 },
        "Main VU Display",
        juce::StringArray{ "Fancy", "Simple" },
        0));

    // PWM: single Speed control 0–100 (replaces Attack/Release in PWM mode)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ paramPwmSpeed, 1 },
        "Speed",
        juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f, 1.0f),
        40.0f,
        juce::AudioParameterFloatAttributes().withLabel("%")));

    // Iron: transformer saturation 0–100%, default 0 (bypass)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ paramIron, 1 },
        "Iron",
        juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f, 1.0f),
        0.0f,
        juce::AudioParameterFloatAttributes().withLabel("%")));

    // Auto Gain: parameter-based auto-makeup (additive to manual Output)
    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{ paramAutoGain, 1 },
        "Auto Gain",
        false));

    return layout;
}

//==============================================================================
OmbicCompressorProcessor::OmbicCompressorProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput("Input", juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
#endif
    apvts(*this, nullptr, "Parameters", createParameterLayout())
{
    inputRms.reset(0, 10);
    outputRms.reset(0, 10);
}

OmbicCompressorProcessor::~OmbicCompressorProcessor() = default;

//==============================================================================
void OmbicCompressorProcessor::updateSidechainFilterCoeffs(float frequencyHz)
{
    if (sampleRateHz <= 0 || frequencyHz <= kScFilterOffHz)
        return;
    sidechainHpfCoeffs_ = juce::dsp::IIR::Coefficients<float>::makeHighPass(
        sampleRateHz, frequencyHz, 0.7071f);  // Butterworth
    if (sidechainHpfCoeffs_)
        sidechainHpf_.coefficients = sidechainHpfCoeffs_;
}

void OmbicCompressorProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    sampleRateHz = sampleRate;
    pwmChain_.reset();
    iron_.reset();
    inputRms.reset(sampleRate, 0.05);
    outputRms.reset(sampleRate, 0.05);
    smoothedScFrequency_.reset(sampleRate, 0.015);  // 15 ms ramp
    smoothedScFrequency_.setCurrentAndTargetValue(kScFilterOffHz);
    updateSidechainFilterCoeffs(100.0f);  // initial coeffs for when filter is used
    sidechainMonoBuffer_.setSize(1, juce::jmax(512, samplesPerBlock));
    sidechainStereoForListen_.setSize(2, juce::jmax(512, samplesPerBlock));
}

void OmbicCompressorProcessor::releaseResources()
{
    fetChain_.reset();
    optoChain_.reset();
    pwmChain_.reset();
    iron_.reset();
    standaloneNeon_.reset();
}

bool OmbicCompressorProcessor::isScListenActive() const
{
    auto* p = apvts.getParameter(paramScListen);
    return p && p->getValue() > 0.5f;
}

bool OmbicCompressorProcessor::getScopeSidechainSamples(std::vector<float>& out) const
{
    const juce::ScopedLock sl(scopeSidechainLock_);
    if (scopeSidechainBuffer_.empty())
        return false;
    out = scopeSidechainBuffer_;
    return true;
}

bool OmbicCompressorProcessor::getScopeWaveformSamples(std::vector<float>& out) const
{
    const juce::ScopedLock sl(scopeWaveformLock_);
    if (scopeWaveformBuffer_.empty())
        return false;
    out = scopeWaveformBuffer_;
    return true;
}

juce::AudioProcessorEditor* OmbicCompressorProcessor::createEditor()
{
#if OMBIC_USE_V2_EDITOR
    return new OmbicCompressorEditorV2(*this);
#else
    return new OmbicCompressorEditor(*this);
#endif
}

void OmbicCompressorProcessor::ensureChains()
{
    juce::File fetishDir;
    juce::File lalaDir;
    juce::File vcaDir;

    // 1) Bundled data (normal for shipped plugins: curve data lives inside the .vst3)
    juce::File curveDataRoot = getBundledCurveDataRoot();
    if (curveDataRoot.exists())
    {
        fetishDir = curveDataRoot.getChildFile("fetish_v2");
        lalaDir = curveDataRoot.getChildFile("lala_v2");
        vcaDir = curveDataRoot.getChildFile("dbcomp_vca");
    }

    // 2) Project / env path (for development: output/fetish_v2, output/lala_v2, output/dbcomp_vca)
    if (!fetishDir.getChildFile("compression_curve.csv").existsAsFile())
    {
        juce::File root = dataRoot_;
        if (!root.exists() || !root.getChildFile("output/fetish_v2/compression_curve.csv").existsAsFile())
        {
            juce::String dataPath = juce::SystemStats::getEnvironmentVariable("OMBIC_COMPRESSOR_DATA_PATH", {});
            if (dataPath.isNotEmpty())
                root = dataPath.startsWithChar('/') ? juce::File(dataPath) : juce::File::getCurrentWorkingDirectory().getChildFile(dataPath);
            if (!root.exists() || !root.getChildFile("output/fetish_v2/compression_curve.csv").existsAsFile())
                root = juce::File::getCurrentWorkingDirectory();
            if (!root.getChildFile("output/fetish_v2/compression_curve.csv").existsAsFile())
                root = juce::File::getSpecialLocation(juce::File::currentApplicationFile).getParentDirectory().getParentDirectory();
            if (!root.getChildFile("output/fetish_v2/compression_curve.csv").existsAsFile())
                root = root.getParentDirectory();
            dataRoot_ = root;
        }
        fetishDir = root.getChildFile("output/fetish_v2");
        lalaDir = root.getChildFile("output/lala_v2");
        vcaDir = root.getChildFile("output/dbcomp_vca");
    }
    bool fetOk = fetishDir.getChildFile("compression_curve.csv").existsAsFile();
    bool lalaOk = lalaDir.getChildFile("compression_curve.csv").existsAsFile();
    bool vcaOk = vcaDir.getChildFile("compression_curve.csv").existsAsFile();
    std::optional<float> noFrDrive;
    if (fetOk && !fetChain_)
        fetChain_ = std::make_unique<emulation::MVPChain>(
            emulation::MVPChain::Mode::FET, sampleRateHz,
            fetishDir, lalaDir, vcaDir, false, false, noFrDrive, 1.0f,
            true, false, 0.02f, 1000.0f, 0.0f, 0.92f, 1.0f, false);
    if (lalaOk && !optoChain_)
        optoChain_ = std::make_unique<emulation::MVPChain>(
            emulation::MVPChain::Mode::Opto, sampleRateHz,
            fetishDir, lalaDir, vcaDir, false, false, noFrDrive, 1.0f,
            true, false, 0.02f, 1000.0f, 0.0f, 0.92f, 1.0f, false);
    if (vcaOk && !vcaChain_)
        vcaChain_ = std::make_unique<emulation::MVPChain>(
            emulation::MVPChain::Mode::VCA, sampleRateHz,
            fetishDir, lalaDir, vcaDir, false, false, noFrDrive, 1.0f,
            true, false, 0.02f, 1000.0f, 0.0f, 0.92f, 1.0f, false);
    if (fetChain_ || optoChain_ || vcaChain_)
        curveDataLoaded_.store(true);
}

void OmbicCompressorProcessor::ensurePwmChain()
{
    if (pwmChain_) return;
    pwmChain_ = std::make_unique<emulation::PwmChain>(
        sampleRateHz, true, true, 0.02f, 1000.0f, 0.0f, 0.92f, 1.0f, false);
}

float OmbicCompressorProcessor::estimateMakeupDb(int mode, float thresholdRaw, float ratio,
                                                 float attackParam, float releaseParam, float speedParam) const
{
    float thresholdDb = -60.0f + (thresholdRaw / 100.0f) * 60.0f;
    float peakDb = -6.0f;
    if (peakDb <= thresholdDb) return 0.0f;
    float over = peakDb - thresholdDb;
    float grDb = over * (1.0f - 1.0f / juce::jmax(1.5f, ratio));
    return juce::jlimit(0.0f, 12.0f, grDb);
}

void OmbicCompressorProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();
    if (numChannels == 0 || numSamples == 0)
        return;

    // Input level (RMS) before processing
    float sumSq = 0.0f;
    for (int ch = 0; ch < numChannels; ++ch)
        for (int i = 0; i < numSamples; ++i)
            sumSq += buffer.getSample(ch, i) * buffer.getSample(ch, i);
    float rms = std::sqrt(sumSq / (numChannels * numSamples));
    float inDb = rms > 1e-6f ? 20.0f * std::log10(rms) : -60.0f;
    inputLevelDb.store(juce::jlimit(-60.0f, 0.0f, inDb));

    ensureChains();

    // Sidechain filter: mono sum of input, optional HPF (bypass at 20 Hz)
    const float scFreqParam = apvts.getRawParameterValue(paramScFrequency)->load();
    const bool scListen = apvts.getRawParameterValue(paramScListen)->load() > 0.5f;
    smoothedScFrequency_.setTargetValue(scFreqParam);
    if (sidechainMonoBuffer_.getNumSamples() < numSamples)
    {
        sidechainMonoBuffer_.setSize(1, numSamples);
        sidechainStereoForListen_.setSize(2, numSamples);
    }
    float* mono = sidechainMonoBuffer_.getWritePointer(0);
    for (int i = 0; i < numSamples; ++i)
    {
        float sum = 0.0f;
        for (int ch = 0; ch < numChannels; ++ch)
            sum += buffer.getSample(ch, i);
        mono[i] = sum / static_cast<float>(numChannels);
    }
    float currentScFreq = smoothedScFrequency_.getNextValue();
    if (currentScFreq > kScFilterOffHz)
    {
        updateSidechainFilterCoeffs(currentScFreq);
        for (int i = 0; i < numSamples; ++i)
            mono[i] = sidechainHpf_.processSample(mono[i]);
    }
    for (int ch = 0; ch < 2; ++ch)
        sidechainStereoForListen_.copyFrom(ch, 0, sidechainMonoBuffer_, 0, 0, numSamples);

    const bool neonOn = true; /* Neon always in path; use Mix for dry/wet (0 = dry). */
    const float modeVal = apvts.getRawParameterValue(paramCompressorMode)->load();
    const int mode = juce::jlimit(0, 3, static_cast<int>(modeVal + 0.5f));
    const float thresholdRaw = apvts.getRawParameterValue(paramThreshold)->load();
    const float ratio = apvts.getRawParameterValue(paramRatio)->load();
    const float attackParam = apvts.getRawParameterValue(paramAttack)->load();
    const float releaseParam = apvts.getRawParameterValue(paramRelease)->load();
    const float speedParam = apvts.getRawParameterValue(paramPwmSpeed)->load();
    float makeupDb = apvts.getRawParameterValue(paramMakeupGainDb)->load();
    makeupDb = juce::jlimit(-24.0f, 12.0f, makeupDb);  // Safe listening: cap boost
    const float ironAmount = apvts.getRawParameterValue(paramIron)->load() / 100.0f;
    const bool autoGain = apvts.getRawParameterValue(paramAutoGain)->load() > 0.5f;
    const float neonDrive = apvts.getRawParameterValue(paramNeonDrive)->load();
    const float neonTone = apvts.getRawParameterValue(paramNeonTone)->load();
    const float neonMix = apvts.getRawParameterValue(paramNeonMix)->load();
    const float neonIntensity = apvts.getRawParameterValue(paramNeonIntensity)->load();
    const float neonBurstiness = apvts.getRawParameterValue(paramNeonBurstiness)->load();
    const float neonGMin = apvts.getRawParameterValue(paramNeonGMin)->load();
    const bool neonSatAfter = apvts.getRawParameterValue(paramNeonSaturationAfter)->load() > 0.5f;
    // Opto-only: GUI "Compress / Limit" dropdown → 0 = Compress, 1 = Limit (more HF in sidechain)
    const int optoCompressLimitChoice = static_cast<int>(apvts.getRawParameterValue(paramOptoCompressLimit)->load() + 0.5f);

    float threshold = thresholdRaw;
    std::optional<float> ratioOpt, attackOpt, releaseOpt;
    if (mode == 1) // FET: threshold in dB (-60..0), ratio, attack_param, release_param
    {
        threshold = -60.0f + (thresholdRaw / 100.0f) * 60.0f;
        ratioOpt = ratio;
        attackOpt = attackParam;
        releaseOpt = releaseParam;
    }
    else if (mode == 3) // VCA: threshold in reference plugin units (-1..3), ratio; no attack/release (program-dependent)
    {
        threshold = -1.0f + (thresholdRaw / 100.0f) * 4.0f;
        ratioOpt = ratio;
    }
    // Opto: threshold stays 0..100. PWM: handled below.

    if (mode == 2) // PWM
    {
        ensurePwmChain();
        float speedNorm = juce::jlimit(0.0f, 100.0f, speedParam) / 100.0f;
        float attackMs = 80.0f * std::pow(0.0125f, speedNorm);
        float releaseMs = 800.0f * std::pow(0.0375f, speedNorm);
        attackMs = juce::jlimit(0.5f, 80.0f, attackMs);
        releaseMs = juce::jlimit(30.0f, 800.0f, releaseMs);
        float pwmRatio = juce::jlimit(1.5f, 8.0f, ratio);
        pwmChain_->setNeonEnabled(neonOn);
        pwmChain_->setNeonBeforeCompressor(true);
        pwmChain_->setNeonParams(
            neonDrive * 1.0f,
            200.0f + neonTone * 4800.0f,
            400.0f + neonTone * 11600.0f,
            neonBurstiness,
            neonGMin,
            neonMix,
            neonIntensity,
            neonSatAfter);
        const juce::AudioBuffer<float>* detectorBuffer = (currentScFreq > kScFilterOffHz) ? &sidechainMonoBuffer_ : nullptr;
        pwmChain_->process(buffer, thresholdRaw, pwmRatio, attackMs, releaseMs, detectorBuffer);
        gainReductionDb.store(pwmChain_->getLastGainReductionDb());
    }
    else
    {
        emulation::MVPChain* chain = (mode == 3) ? vcaChain_.get() : ((mode == 1) ? fetChain_.get() : optoChain_.get());
        if (chain)
        {
            chain->setNeonEnabled(neonOn);
            chain->setNeonBeforeCompressor(true);  // fixed: saturator always before compressor
            chain->setNeonParams(
                neonDrive * 1.0f,
                200.0f + neonTone * 4800.0f,
                400.0f + neonTone * 11600.0f,  // wet-path tone: 400 Hz (dark) .. 12 kHz (bright)
                neonBurstiness,
                neonGMin,
                neonMix,
                neonIntensity,
                neonSatAfter);
            std::optional<bool> optoLimitMode = (mode == 0) ? std::optional<bool>(optoCompressLimitChoice == 1) : std::nullopt;  // Limit when dropdown = "Limit"
            const juce::AudioBuffer<float>* detectorBuffer = (currentScFreq > kScFilterOffHz) ? &sidechainMonoBuffer_ : nullptr;
            chain->process(buffer, threshold, ratioOpt, attackOpt, releaseOpt, 512, optoLimitMode, detectorBuffer);
            gainReductionDb.store(chain->getLastGainReductionDb());
        }
        else
        {
            gainReductionDb.store(0.0f);
            if (neonOn)
            {
                if (!standaloneNeon_)
                    standaloneNeon_ = std::make_unique<emulation::NeonTapeSaturation>(sampleRateHz);
                standaloneNeon_->setDepth(neonDrive * 1.0f);
                standaloneNeon_->setModulationBandwidthHz(200.0f + neonTone * 4800.0f);
                standaloneNeon_->setToneFilterCutoffHz(400.0f + neonTone * 11600.0f);
                standaloneNeon_->setBurstiness(neonBurstiness);
                standaloneNeon_->setGMin(neonGMin);
                standaloneNeon_->setDryWet(neonMix);
                standaloneNeon_->setSaturationIntensity(neonIntensity);
                standaloneNeon_->setSaturationAfter(neonSatAfter);
                standaloneNeon_->process(buffer);
            }
            else
                standaloneNeon_.reset();
        }
    }

    // Output: Listen replaces with sidechain at unity; otherwise apply makeup.
    if (scListen)
    {
        for (int ch = 0; ch < numChannels && ch < 2; ++ch)
            buffer.copyFrom(ch, 0, sidechainStereoForListen_, ch, 0, numSamples);
        // Copy sidechain for Neon scope (thread-safe for UI)
        {
            const juce::ScopedLock sl(scopeSidechainLock_);
            scopeSidechainBuffer_.resize(static_cast<size_t>(numSamples));
            const float* readPtr = sidechainMonoBuffer_.getReadPointer(0);
            for (int i = 0; i < numSamples; ++i)
                scopeSidechainBuffer_[static_cast<size_t>(i)] = readPtr[i];
        }
        { const juce::ScopedLock wfSl(scopeWaveformLock_); scopeWaveformBuffer_.clear(); }
    }
    else
    {
        {
            const juce::ScopedLock sl(scopeSidechainLock_);
            scopeSidechainBuffer_.clear();
        }
        if (ironAmount > 0.001f)
        {
            if (!iron_)
            {
                iron_ = std::make_unique<emulation::IronTransformer>();
                iron_->prepare(sampleRateHz);
            }
            iron_->process(buffer, mode, ironAmount);
        }
        float makeupTotal = makeupDb;
        if (autoGain)
            makeupTotal += estimateMakeupDb(mode, thresholdRaw, ratio, attackParam, releaseParam, speedParam);
        makeupTotal = juce::jlimit(-24.0f, 24.0f, makeupTotal);
        float makeupGain = std::pow(10.0f, makeupTotal / 20.0f);
        buffer.applyGain(makeupGain);
        // Copy main output (mono) for Neon tube scope so it can follow the waveform
        const juce::ScopedLock wfSl(scopeWaveformLock_);
        scopeWaveformBuffer_.resize(static_cast<size_t>(numSamples));
        if (numChannels >= 1)
        {
            const float* L = buffer.getReadPointer(0);
            if (numChannels >= 2)
            {
                const float* R = buffer.getReadPointer(1);
                for (int i = 0; i < numSamples; ++i)
                    scopeWaveformBuffer_[static_cast<size_t>(i)] = 0.5f * (L[i] + R[i]);
            }
            else
            {
                for (int i = 0; i < numSamples; ++i)
                    scopeWaveformBuffer_[static_cast<size_t>(i)] = L[i];
            }
        }
    }

    sumSq = 0.0f;
    for (int ch = 0; ch < numChannels; ++ch)
        for (int i = 0; i < numSamples; ++i)
            sumSq += buffer.getSample(ch, i) * buffer.getSample(ch, i);
    rms = std::sqrt(sumSq / (numChannels * numSamples));
    float outDb = rms > 1e-6f ? 20.0f * std::log10(rms) : -60.0f;
    outputLevelDb.store(juce::jlimit(-60.0f, 0.0f, outDb));
}

//==============================================================================
void OmbicCompressorProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    juce::MemoryOutputStream stream(destData, true);
    apvts.state.writeToStream(stream);
}

void OmbicCompressorProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    auto tree = juce::ValueTree::readFromData(data, size_t(sizeInBytes));
    if (tree.isValid())
    {
        apvts.replaceState(tree);
        // Safety: never restore with SC Listen on — override to off
        if (auto* p = apvts.getParameter(paramScListen))
            p->setValueNotifyingHost(p->convertTo0to1(false));
    }
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new OmbicCompressorProcessor();
}
