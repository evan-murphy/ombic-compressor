#include "MainViewAsTubeComponent.h"
#include "../PluginProcessor.h"

namespace
{
    const int kReadoutH = 16;
    const int kReadoutRowPadding = 6;
    const int kReadoutLabelW = 52;
    const int kReadoutGap = 24;
}

MainViewAsTubeComponent::MainViewAsTubeComponent(OmbicCompressorProcessor& processor)
    : proc_(processor)
    , transferCurve_(processor)
{
    setLookAndFeel(&ombicLf_);
    addAndMakeVisible(transferCurve_);
    inReadout_.setJustificationType(juce::Justification::centred);
    inReadout_.setFont(OmbicLookAndFeel::getOmbicFontForPainting(9.5f, true));
    addAndMakeVisible(inReadout_);
    grReadout_.setJustificationType(juce::Justification::centred);
    grReadout_.setFont(OmbicLookAndFeel::getOmbicFontForPainting(16.0f, true));  // §6 GR readout 16px
    addAndMakeVisible(grReadout_);
    outReadout_.setJustificationType(juce::Justification::centred);
    outReadout_.setFont(OmbicLookAndFeel::getOmbicFontForPainting(9.5f, true));
    addAndMakeVisible(outReadout_);
    startTimerHz(25);
}

void MainViewAsTubeComponent::timerCallback()
{
    float inDb = proc_.inputLevelDb.load();
    float grDb = proc_.gainReductionDb.load();
    float outDb = proc_.outputLevelDb.load();
    smoothedInDb_  += ballisticsCoeff_ * (inDb  - smoothedInDb_);
    smoothedGrDb_   += ballisticsCoeff_ * (grDb  - smoothedGrDb_);
    smoothedOutDb_  += ballisticsCoeff_ * (outDb - smoothedOutDb_);

    inReadout_.setText((smoothedInDb_ <= -60.0f ? juce::String("-60") : juce::String(static_cast<int>(smoothedInDb_))) + " dB", juce::dontSendNotification);
    inReadout_.setColour(juce::Label::textColourId, OmbicLookAndFeel::ombicBlue());
    grReadout_.setText(juce::String(smoothedGrDb_, 1) + " dB", juce::dontSendNotification);
    float absGr = std::abs(smoothedGrDb_);
    if (absGr < 3.0f)
        grReadout_.setColour(juce::Label::textColourId, OmbicLookAndFeel::ombicTeal());
    else if (absGr < 6.0f)
        grReadout_.setColour(juce::Label::textColourId, OmbicLookAndFeel::ombicYellow());
    else
        grReadout_.setColour(juce::Label::textColourId, OmbicLookAndFeel::ombicRed());
    outReadout_.setText((smoothedOutDb_ <= -60.0f ? juce::String("-60") : juce::String(static_cast<int>(smoothedOutDb_))) + " dB", juce::dontSendNotification);
    outReadout_.setColour(juce::Label::textColourId, OmbicLookAndFeel::ombicTeal());

    filamentPhase_ += 0.06f;
    repaint();
}

void MainViewAsTubeComponent::paintTubeGlow(juce::Graphics& g)
{
    auto* driveP = proc_.getValueTreeState().getRawParameterValue(OmbicCompressorProcessor::paramNeonDrive);
    auto* intensityP = proc_.getValueTreeState().getRawParameterValue(OmbicCompressorProcessor::paramNeonIntensity);
    auto* mixP = proc_.getValueTreeState().getRawParameterValue(OmbicCompressorProcessor::paramNeonMix);
    float drive = driveP ? driveP->load() : 0.45f;
    float intensity = intensityP ? intensityP->load() : 0.5f;
    float mix = mixP ? mixP->load() : 1.0f;

    float glowStrength = (drive * 0.5f + intensity * 0.6f + mix * 0.5f);
    float pinkAlpha = juce::jmin(1.0f, glowStrength * 0.85f);
    float redAlpha = juce::jmin(0.6f, glowStrength * 0.5f);

    auto b = getLocalBounds().toFloat();
    g.setColour(juce::Colour(0xFF080a12));
    g.fillRoundedRectangle(b, 12.0f);

    juce::Colour pink(0xFFe85590);
    juce::Colour red(0xFFff001f);
    g.setColour(pink.withAlpha(pinkAlpha * 0.5f));
    g.fillRoundedRectangle(b.reduced(2.0f), 10.0f);
    g.setColour(red.withAlpha(redAlpha * 0.25f));
    g.fillRoundedRectangle(b.reduced(4.0f), 8.0f);
}

void MainViewAsTubeComponent::paintFilament(juce::Graphics& g)
{
    auto* driveP = proc_.getValueTreeState().getRawParameterValue(OmbicCompressorProcessor::paramNeonDrive);
    auto* intensityP = proc_.getValueTreeState().getRawParameterValue(OmbicCompressorProcessor::paramNeonIntensity);
    auto* toneP = proc_.getValueTreeState().getRawParameterValue(OmbicCompressorProcessor::paramNeonTone);
    auto* mixP = proc_.getValueTreeState().getRawParameterValue(OmbicCompressorProcessor::paramNeonMix);
    float drive = driveP ? driveP->load() : 0.45f;
    float intensity = intensityP ? intensityP->load() : 0.5f;
    float tone = toneP ? toneP->load() : 0.5f;
    float mix = mixP ? mixP->load() : 1.0f;

    auto plotArea = getLocalBounds().toFloat().reduced(8.0f, 6.0f);
    const float tubeH = 42.0f;
    const float tubeRadius = tubeH * 0.5f;
    const float tubeY = plotArea.getCentreY() - tubeRadius;
    const float tubeX = plotArea.getX() + 4.0f;
    const float tubeW = plotArea.getWidth() - 8.0f;
    const float yMid = plotArea.getCentreY();
    const float amp = 14.0f;  // §6 Filament amplitude ±14px

    juce::Path filamentPath;
    std::vector<float> waveformSamples;
    bool hasRealSignal = false;
    float peak = 0.0f;
    if (proc_.getScopeWaveformSamples(waveformSamples) && waveformSamples.size() > 1)
    {
        for (size_t i = 0; i < waveformSamples.size(); ++i)
            peak = juce::jmax(peak, std::abs(waveformSamples[i]));
        hasRealSignal = (peak >= 1e-4f);
    }

    const juce::Colour neonPink(0xFFe85590);

    if (hasRealSignal)
    {
        const size_t n = waveformSamples.size();
        const float displayGain = 1.0f + drive * 4.0f;
        const float displayNorm = std::tanh(displayGain);
        for (size_t i = 0; i < n; ++i)
        {
            float s = waveformSamples[i] / (peak + 1e-9f);
            float displayed = std::tanh(s * displayGain) / displayNorm;
            float t = static_cast<float>(i) / static_cast<float>(n - 1);
            float x = tubeX + t * tubeW;
            float yPos = yMid - displayed * amp;
            if (i == 0)
                filamentPath.startNewSubPath(x, yPos);
            else
                filamentPath.lineTo(x, yPos);
        }
    }
    else
    {
        const int nPts = 80;
        for (int i = 0; i <= nPts; ++i)
        {
            float xNorm = static_cast<float>(i) / static_cast<float>(nPts);
            float low = std::sin(2.0f * juce::MathConstants<float>::pi * xNorm + filamentPhase_ * 0.7f) * 0.6f
                     + std::sin(4.0f * juce::MathConstants<float>::pi * xNorm + filamentPhase_ * 2.0f) * 0.3f;
            float high = std::sin(8.0f * juce::MathConstants<float>::pi * xNorm + filamentPhase_ * 1.3f) * 0.5f
                      + std::sin(16.0f * juce::MathConstants<float>::pi * xNorm + filamentPhase_ * 0.9f) * 0.35f;
            float y = low * (1.0f - tone) + high * tone;
            float gain = 1.0f + drive * 5.0f;
            y = std::tanh(y * gain) / std::tanh(gain);
            float x = tubeX + xNorm * tubeW;
            float yPos = yMid - y * amp * 0.42f;
            if (i == 0)
                filamentPath.startNewSubPath(x, yPos);
            else
                filamentPath.lineTo(x, yPos);
        }
    }

    float lineOpacity = mix * (0.7f + intensity * 0.3f);
    float glowOpacity = mix * (0.2f + intensity * 0.6f);
    float glowStroke = 16.0f + 36.0f * intensity;
    float lineStroke = 0.4f + 4.0f * intensity;

    juce::Path tubeClip;
    tubeClip.addRoundedRectangle(tubeX, tubeY, tubeW, tubeH, tubeRadius);
    g.saveState();
    g.reduceClipRegion(tubeClip);

    if (glowOpacity > 0.001f)
    {
        g.setColour(neonPink.withAlpha(glowOpacity));
        g.strokePath(filamentPath, juce::PathStrokeType(glowStroke));
    }
    if (lineOpacity > 0.001f)
    {
        g.setColour(neonPink.withAlpha(lineOpacity));
        g.strokePath(filamentPath, juce::PathStrokeType(lineStroke));
    }
    g.restoreState();
}

void MainViewAsTubeComponent::paint(juce::Graphics& g)
{
    paintTubeGlow(g);

    auto b = getLocalBounds().toFloat();
    g.setColour(OmbicLookAndFeel::pluginBorder().withAlpha(0.5f));
    g.drawRoundedRectangle(b.reduced(0.5f), 12.0f, 1.0f);
}

void MainViewAsTubeComponent::paintOverChildren(juce::Graphics& g)
{
    paintFilament(g);
}

void MainViewAsTubeComponent::resized()
{
    auto r = getLocalBounds();
    if (r.isEmpty()) return;

    const int readoutRowTotalH = kReadoutH + kReadoutRowPadding;
    const int curveH = juce::jmin(90, r.getHeight() - readoutRowTotalH);  // §6 Transfer curve ~90px of main view
    transferCurve_.setBounds(r.getX(), r.getY(), r.getWidth(), curveH);
    auto readoutRow = r.removeFromBottom(readoutRowTotalH);

    const int totalReadoutW = 3 * kReadoutLabelW + 2 * kReadoutGap;
    int rx = readoutRow.getX() + (readoutRow.getWidth() - totalReadoutW) / 2;
    const int ry = readoutRow.getY() + (readoutRow.getHeight() - kReadoutH) / 2;
    inReadout_.setBounds(rx, ry, kReadoutLabelW, kReadoutH);
    rx += kReadoutLabelW + kReadoutGap;
    grReadout_.setBounds(rx, ry, kReadoutLabelW, kReadoutH);
    rx += kReadoutLabelW + kReadoutGap;
    outReadout_.setBounds(rx, ry, kReadoutLabelW, kReadoutH);
}
