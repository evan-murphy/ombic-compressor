#include "MainVuComponent.h"
#include "../PluginProcessor.h"

namespace
{
    const int kToggleH = 36;
    const int kDisplayPad = 14;
    // Compact readout row to match HTML: small font, minimal height — curve is the focal point
    const int kReadoutH = 16;
    const int kReadoutRowPadding = 6;
    const int kReadoutLabelW = 52;
    const int kReadoutGap = 24;
}

MainVuComponent::MainVuComponent(OmbicCompressorProcessor& processor)
    : proc_(processor)
    , transferCurve_(processor)
{
    setLookAndFeel(&ombicLf_);

    fancyButton_.setButtonText("Fancy");
    fancyButton_.setName("main_vu_fancy");
    fancyButton_.setClickingTogglesState(false);
    fancyButton_.onClick = [this]() {
        if (auto* p = proc_.getValueTreeState().getParameter(OmbicCompressorProcessor::paramMainVuDisplay))
            p->setValueNotifyingHost(p->convertTo0to1(0));
    };
    addAndMakeVisible(fancyButton_);

    simpleButton_.setButtonText("Simple");
    simpleButton_.setName("main_vu_simple");
    simpleButton_.setClickingTogglesState(false);
    simpleButton_.onClick = [this]() {
        if (auto* p = proc_.getValueTreeState().getParameter(OmbicCompressorProcessor::paramMainVuDisplay))
            p->setValueNotifyingHost(p->convertTo0to1(1));
    };
    addAndMakeVisible(simpleButton_);

    addAndMakeVisible(transferCurve_);
    inReadout_.setJustificationType(juce::Justification::centred);
    inReadout_.setFont(OmbicLookAndFeel::getOmbicFontForPainting(9.5f, true));
    addAndMakeVisible(inReadout_);
    grReadout_.setJustificationType(juce::Justification::centred);
    grReadout_.setFont(OmbicLookAndFeel::getOmbicFontForPainting(9.5f, true));
    addAndMakeVisible(grReadout_);
    outReadout_.setJustificationType(juce::Justification::centred);
    outReadout_.setFont(OmbicLookAndFeel::getOmbicFontForPainting(9.5f, true));
    addAndMakeVisible(outReadout_);
    inReadout_.setTooltip("Input peak level (fast response).");
    grReadout_.setTooltip("Gain reduction: how much the compressor is reducing level. Updates quickly with the signal.");
    outReadout_.setTooltip("Output peak level (fast response).");

    startTimerHz(kMeterHz);
    updateFromParameter();
}

bool MainVuComponent::isFancy() const
{
    auto* p = proc_.getValueTreeState().getParameter(OmbicCompressorProcessor::paramMainVuDisplay);
    if (!p) return true;
    return static_cast<int>(p->getValue() * 1.99f) == 0;
}

void MainVuComponent::updateFromParameter()
{
    bool fancy = isFancy();
    fancyButton_.setToggleState(fancy, juce::dontSendNotification);
    simpleButton_.setToggleState(!fancy, juce::dontSendNotification);
    transferCurve_.setVisible(fancy);
    inReadout_.setVisible(fancy);
    grReadout_.setVisible(fancy);
    outReadout_.setVisible(fancy);
}

void MainVuComponent::timerCallback()
{
    float inPeak = proc_.inputPeakDb.load();
    float outPeak = proc_.outputPeakDb.load();
    float grDb = proc_.gainReductionDb.load();

    if (inPeak > peakInDb_) peakInDb_ += kPeakAttackCoeff * (inPeak - peakInDb_);
    else peakInDb_ += kPeakReleaseCoeff * (inPeak - peakInDb_);
    if (outPeak > peakOutDb_) peakOutDb_ += kPeakAttackCoeff * (outPeak - peakOutDb_);
    else peakOutDb_ += kPeakReleaseCoeff * (outPeak - peakOutDb_);
    if (grDb > smoothedGrDb_) smoothedGrDb_ += kGrAttackCoeff * (grDb - smoothedGrDb_);
    else smoothedGrDb_ += kGrReleaseCoeff * (grDb - smoothedGrDb_);

    inReadout_.setText((peakInDb_ <= -60.0f ? juce::String("-60") : juce::String(static_cast<int>(peakInDb_))) + " dB", juce::dontSendNotification);
    inReadout_.setColour(juce::Label::textColourId, OmbicLookAndFeel::ombicBlue());
    grReadout_.setText(juce::String(smoothedGrDb_, 1) + " dB", juce::dontSendNotification);
    float absGr = std::abs(smoothedGrDb_);
    if (absGr < 3.0f)
        grReadout_.setColour(juce::Label::textColourId, OmbicLookAndFeel::ombicTeal());
    else if (absGr < 6.0f)
        grReadout_.setColour(juce::Label::textColourId, OmbicLookAndFeel::ombicYellow());
    else
        grReadout_.setColour(juce::Label::textColourId, OmbicLookAndFeel::ombicRed());
    outReadout_.setText((peakOutDb_ <= -60.0f ? juce::String("-60") : juce::String(static_cast<int>(peakOutDb_))) + " dB", juce::dontSendNotification);
    outReadout_.setColour(juce::Label::textColourId, OmbicLookAndFeel::ombicTeal());

    updateFromParameter();
    repaint();
}

void MainVuComponent::resized()
{
    auto r = getLocalBounds();
    if (r.isEmpty()) return;

    auto header = r.removeFromTop(kToggleH);
    const int btnW = 78;
    const int gap = 12;
    fancyButton_.setBounds(header.removeFromLeft(btnW).reduced(2));
    header.removeFromLeft(gap);
    simpleButton_.setBounds(header.removeFromLeft(btnW).reduced(2));

    r.reduce(kDisplayPad, kDisplayPad);
    if (r.getHeight() <= 0) return;

    const int readoutRowTotalH = kReadoutH + kReadoutRowPadding;
    const int curveH = juce::jmin(118, r.getHeight() - readoutRowTotalH);
    transferCurve_.setBounds(r.getX(), r.getY(), r.getWidth(), curveH);
    auto readoutRow = r.removeFromBottom(readoutRowTotalH);

    // Match HTML: readouts centered with fixed width + gap (not 1/3 width each — curve is focal point)
    const int totalReadoutW = 3 * kReadoutLabelW + 2 * kReadoutGap;
    int rx = readoutRow.getX() + (readoutRow.getWidth() - totalReadoutW) / 2;
    const int ry = readoutRow.getY() + (readoutRow.getHeight() - kReadoutH) / 2;
    inReadout_.setBounds(rx, ry, kReadoutLabelW, kReadoutH);
    rx += kReadoutLabelW + kReadoutGap;
    grReadout_.setBounds(rx, ry, kReadoutLabelW, kReadoutH);
    rx += kReadoutLabelW + kReadoutGap;
    outReadout_.setBounds(rx, ry, kReadoutLabelW, kReadoutH);
}

void MainVuComponent::paint(juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat();
    g.setColour(OmbicLookAndFeel::pluginSurface());
    g.fillRoundedRectangle(b, 12.0f);
    g.setColour(OmbicLookAndFeel::pluginBorder());
    g.drawRoundedRectangle(b.reduced(0.5f), 12.0f, 1.0f);

    auto header = getLocalBounds().removeFromTop(kToggleH);
    g.setColour(OmbicLookAndFeel::pluginRaised());
    g.fillRoundedRectangle(header.toFloat().reduced(1.0f), 8.0f);

    if (!isFancy())
    {
        auto displayArea = getLocalBounds();
        displayArea.removeFromTop(kToggleH + kDisplayPad);
        displayArea.removeFromBottom(kDisplayPad);
        displayArea.reduce(kDisplayPad, 0);
        if (displayArea.getHeight() > 0 && displayArea.getWidth() > 0)
        {
            // Same arc design as before, rotated 90° so arc is vertical (bottom = 0, top = full, like output section)
            auto arcR = displayArea.toFloat();
            const float cx = arcR.getCentreX();
            const float cy = arcR.getCentreY();
            const float radius = juce::jmin(arcR.getHeight(), arcR.getWidth() * 2.0f) * 0.45f;
            const float arcX = cx + radius * 0.5f;  // arc on the right half so it runs vertically
            const float arcW = 8.0f;
            const float startAngle = juce::MathConstants<float>::halfPi;   // bottom
            const float endAngle = juce::MathConstants<float>::halfPi * 3.0f; // top
            const float sweep = juce::MathConstants<float>::pi;

            juce::Path arcTrack;
            arcTrack.addArc(arcX - radius, cy - radius, radius * 2.0f, radius * 2.0f, startAngle, endAngle, true);
            g.setColour(OmbicLookAndFeel::pluginBorder());
            g.strokePath(arcTrack, juce::PathStrokeType(arcW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

            auto dbToNorm = [](float db) { return juce::jlimit(0.0f, 1.0f, (db + 60.0f) / 60.0f); };
            float inNorm = dbToNorm(peakInDb_);
            float outNorm = dbToNorm(peakOutDb_);

            juce::Path inArc;
            inArc.addArc(arcX - radius, cy - radius, radius * 2.0f, radius * 2.0f, startAngle, startAngle + sweep * inNorm, true);
            g.setColour(OmbicLookAndFeel::ombicBlue());
            g.strokePath(inArc, juce::PathStrokeType(arcW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

            juce::Path outArc;
            outArc.addArc(arcX - radius, cy - radius, radius * 2.0f, radius * 2.0f, startAngle, startAngle + sweep * outNorm, true);
            g.setColour(OmbicLookAndFeel::ombicTeal());
            g.strokePath(outArc, juce::PathStrokeType(arcW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

            g.setFont(OmbicLookAndFeel::getOmbicFontForPainting(8.0f, true));
            g.setColour(OmbicLookAndFeel::pluginMuted());
            g.drawText("In / Out", static_cast<int>(cx - 40), static_cast<int>(cy + radius * 0.5f + 4), 80, 12, juce::Justification::centred);
        }
    }
}
