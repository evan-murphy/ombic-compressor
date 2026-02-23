#include "MeterStrip.h"
#include "../PluginProcessor.h"

static float levelToNorm(float db)
{
    return juce::jlimit(0.0f, 1.0f, (db + 60.0f) / 60.0f);
}

static void updatePeakBallistics(float rawDb, float& peakDb, float attackCoeff, float releaseCoeff)
{
    if (rawDb > peakDb)
        peakDb += attackCoeff * (rawDb - peakDb);
    else
        peakDb += releaseCoeff * (rawDb - peakDb);
}

MeterStrip::MeterStrip(OmbicCompressorProcessor& processor)
    : proc(processor)
{
    peakButton_.setButtonText("Peak");
    peakButton_.setClickingTogglesState(false);
    peakButton_.onClick = [this]() { showPeak_ = true; peakButton_.setToggleState(true, juce::dontSendNotification); vuButton_.setToggleState(false, juce::dontSendNotification); };
    vuButton_.setButtonText("VU");
    vuButton_.setClickingTogglesState(false);
    vuButton_.onClick = [this]() { showPeak_ = false; vuButton_.setToggleState(true, juce::dontSendNotification); peakButton_.setToggleState(false, juce::dontSendNotification); };
    peakButton_.setToggleState(true, juce::dontSendNotification);
    peakButton_.setTooltip("Show peak level (fast response, L/R stereo, 2 s hold).");
    vuButton_.setTooltip("Show average level (VU-style, ~300 ms).");
    addAndMakeVisible(peakButton_);
    addAndMakeVisible(vuButton_);
    startTimerHz(kMeterHz);
}

void MeterStrip::timerCallback()
{
    float inPeakRaw = proc.inputPeakDb.load();
    float outPeakRaw = proc.outputPeakDb.load();
    float inL = proc.inputPeakDbL.load();
    float inR = proc.inputPeakDbR.load();
    float outL = proc.outputPeakDbL.load();
    float outR = proc.outputPeakDbR.load();
    float inRmsRaw = proc.inputLevelDb.load();
    float outRmsRaw = proc.outputLevelDb.load();
    float grDb = proc.gainReductionDb.load();

    updatePeakBallistics(inPeakRaw, peakInDb_, kPeakAttackCoeff, kPeakReleaseCoeff);
    updatePeakBallistics(outPeakRaw, peakOutDb_, kPeakAttackCoeff, kPeakReleaseCoeff);
    updatePeakBallistics(inL, peakInL_, kPeakAttackCoeff, kPeakReleaseCoeff);
    updatePeakBallistics(inR, peakInR_, kPeakAttackCoeff, kPeakReleaseCoeff);
    updatePeakBallistics(outL, peakOutL_, kPeakAttackCoeff, kPeakReleaseCoeff);
    updatePeakBallistics(outR, peakOutR_, kPeakAttackCoeff, kPeakReleaseCoeff);
    avgInDb_ += kVuCoeff * (inRmsRaw - avgInDb_);
    avgOutDb_ += kVuCoeff * (outRmsRaw - avgOutDb_);

    if (peakInDb_ >= peakHoldInDb_) { peakHoldInDb_ = peakInDb_; peakHoldInTicks_ = kPeakHoldTicks; }
    else { if (peakHoldInTicks_ > 0) --peakHoldInTicks_; if (peakHoldInTicks_ <= 0) peakHoldInDb_ += kPeakReleaseCoeff * (peakInDb_ - peakHoldInDb_); }
    if (peakOutDb_ >= peakHoldOutDb_) { peakHoldOutDb_ = peakOutDb_; peakHoldOutTicks_ = kPeakHoldTicks; }
    else { if (peakHoldOutTicks_ > 0) --peakHoldOutTicks_; if (peakHoldOutTicks_ <= 0) peakHoldOutDb_ += kPeakReleaseCoeff * (peakOutDb_ - peakHoldOutDb_); }

    if (grDb > smoothedGrDb_)
        smoothedGrDb_ += kGrAttackCoeff * (grDb - smoothedGrDb_);
    else
        smoothedGrDb_ += kGrReleaseCoeff * (grDb - smoothedGrDb_);
    if (smoothedGrDb_ > grHoldDb_) { grHoldDb_ = smoothedGrDb_; grHoldTicks_ = kGrHoldTicks; }
    else { if (grHoldTicks_ > 0) --grHoldTicks_; if (grHoldTicks_ <= 0) grHoldDb_ += kGrReleaseCoeff * (smoothedGrDb_ - grHoldDb_); }  // decay hold toward current

    repaint();
}

void MeterStrip::resized()
{
    auto r = getLocalBounds();
    if (r.getHeight() < 50) return;
    auto bottom = r.removeFromBottom(20);
    const int btnW = 36;
    const int gap = 4;
    peakButton_.setBounds(bottom.removeFromLeft(btnW).reduced(gap, 2));
    bottom.removeFromLeft(gap);
    vuButton_.setBounds(bottom.removeFromLeft(btnW).reduced(gap, 2));
}

void MeterStrip::paint(juce::Graphics& g)
{
    float inDisplayDb = showPeak_ ? peakInDb_ : avgInDb_;
    float outDisplayDb = showPeak_ ? peakOutDb_ : avgOutDb_;
    float inNorm = levelToNorm(inDisplayDb);
    float outNorm = levelToNorm(outDisplayDb);
    const float grFullScaleDb = 40.0f;
    float grNorm = juce::jlimit(0.0f, 1.0f, smoothedGrDb_ / grFullScaleDb);

    auto b = getLocalBounds().toFloat();
    const float shadowOff = 8.0f;
    g.setColour(OmbicLookAndFeel::ink());
    g.fillRoundedRectangle(b.translated(shadowOff, shadowOff), 20.0f);
    g.setColour(OmbicLookAndFeel::bg());
    g.fillRoundedRectangle(b, 20.0f);
    g.setColour(OmbicLookAndFeel::ink());
    g.drawRoundedRectangle(b, 20.0f, 3.0f);
    b = b.reduced(12.0f);
    if (peakButton_.isVisible() && peakButton_.getHeight() > 0)
        b = b.withTrimmedBottom(static_cast<float>(peakButton_.getHeight() + 4));

    const int meterW = 28;
    const int gap = 10;
    const float boxH = b.getHeight() - 36.0f;
    int x = static_cast<int>(b.getX()) + 24;

    // Scale: 0, -18 (reference), -20, -40, -60 dB
    g.setFont(OmbicLookAndFeel::getOmbicFontForPainting(9.0f, false));
    g.setColour(OmbicLookAndFeel::ink());
    int scaleX = static_cast<int>(b.getX());
    auto dbToY = [&](float db) {
        return static_cast<int>(boxH * (1.0f - (db + 60.0f) / 60.0f));
    };
    g.drawText("0", scaleX, dbToY(0) - 6, 22, 12, juce::Justification::right);
    g.drawText("-18", scaleX, dbToY(-18.0f) - 6, 22, 12, juce::Justification::right);
    g.drawText("-20", scaleX, dbToY(-20.0f) - 6, 22, 12, juce::Justification::right);
    g.drawText("-40", scaleX, dbToY(-40.0f) - 6, 22, 12, juce::Justification::right);
    g.drawText("-60", scaleX, dbToY(-60.0f) - 6, 22, 12, juce::Justification::right);

    auto drawLevelMeterMono = [&](float norm, const juce::Colour& fillColour,
                                  const juce::String& title, float displayDb) {
        auto box = b.withX(static_cast<float>(x)).withWidth(static_cast<float>(meterW))
                        .withHeight(boxH).reduced(1);
        g.setColour(OmbicLookAndFeel::ink());
        g.drawRoundedRectangle(box, 4.0f, 2.0f);
        g.setColour(OmbicLookAndFeel::line());
        g.fillRoundedRectangle(box.reduced(1), 3.0f);
        if (norm > 0.005f)
        {
            g.setColour(fillColour);
            float barH = box.getHeight() * norm;
            g.fillRoundedRectangle(box.getX() + 2, box.getBottom() - barH, box.getWidth() - 4, barH, 3.0f);
        }
        g.setFont(OmbicLookAndFeel::getOmbicFontForPainting(9.0f, false));
        g.setColour(OmbicLookAndFeel::ink());
        g.drawText(title, x - 2, static_cast<int>(box.getBottom()) + 2, meterW + 4, 12, juce::Justification::centred);
        juce::String dbStr = (displayDb <= -60.0f) ? "-60" : juce::String(static_cast<int>(displayDb));
        g.drawText(dbStr + " dB", x - 2, static_cast<int>(box.getBottom()) + 14, meterW + 4, 12, juce::Justification::centred);
        x += meterW + gap;
    };
    auto drawLevelMeterStereo = [&](float normL, float normR, float holdDb, const juce::Colour& fillColour,
                                    const juce::String& title, float displayDb) {
        auto box = b.withX(static_cast<float>(x)).withWidth(static_cast<float>(meterW))
                        .withHeight(boxH).reduced(1);
        g.setColour(OmbicLookAndFeel::ink());
        g.drawRoundedRectangle(box, 4.0f, 2.0f);
        g.setColour(OmbicLookAndFeel::line());
        g.fillRoundedRectangle(box.reduced(1), 3.0f);
        const float halfW = (box.getWidth() - 4) * 0.5f;
        if (normL > 0.005f)
        {
            g.setColour(fillColour);
            float barH = box.getHeight() * normL;
            g.fillRoundedRectangle(box.getX() + 2, box.getBottom() - barH, halfW, barH, 2.0f);
        }
        if (normR > 0.005f)
        {
            g.setColour(fillColour.withAlpha(0.85f));
            float barH = box.getHeight() * normR;
            g.fillRoundedRectangle(box.getX() + 2 + halfW, box.getBottom() - barH, halfW, barH, 2.0f);
        }
        float holdNorm = levelToNorm(holdDb);
        float maxNorm = juce::jmax(normL, normR);
        if (holdNorm > 0.005f && holdNorm > maxNorm + 0.01f)
        {
            g.setColour(fillColour.withAlpha(0.6f));
            float holdY = box.getBottom() - box.getHeight() * holdNorm;
            g.fillRect(box.getX() + 2, holdY - 1, box.getWidth() - 4, 2.0f);
        }
        g.setFont(OmbicLookAndFeel::getOmbicFontForPainting(9.0f, false));
        g.setColour(OmbicLookAndFeel::ink());
        g.drawText(title, x - 2, static_cast<int>(box.getBottom()) + 2, meterW + 4, 12, juce::Justification::centred);
        juce::String dbStr = (displayDb <= -60.0f) ? "-60" : juce::String(static_cast<int>(displayDb));
        g.drawText(dbStr + " dB", x - 2, static_cast<int>(box.getBottom()) + 14, meterW + 4, 12, juce::Justification::centred);
        x += meterW + gap;
    };

    if (showPeak_)
        drawLevelMeterStereo(levelToNorm(peakInL_), levelToNorm(peakInR_), peakHoldInDb_, OmbicLookAndFeel::ombicBlue(), "In", inDisplayDb);
    else
        drawLevelMeterMono(inNorm, OmbicLookAndFeel::ombicBlue(), "In", inDisplayDb);

    auto grBox = b.withX(static_cast<float>(x)).withWidth(static_cast<float>(meterW)).withHeight(boxH).reduced(1);
    g.setColour(OmbicLookAndFeel::ink());
    g.drawRoundedRectangle(grBox, 4.0f, 2.0f);
    g.setColour(OmbicLookAndFeel::line());
    g.fillRoundedRectangle(grBox.reduced(1), 3.0f);
    if (grNorm > 0.005f)
    {
        g.setColour(OmbicLookAndFeel::ombicRed());
        float barH = grBox.getHeight() * grNorm;
        g.fillRoundedRectangle(grBox.getX() + 2, grBox.getBottom() - barH, grBox.getWidth() - 4, barH, 3.0f);
    }
    float grHoldNorm = juce::jlimit(0.0f, 1.0f, grHoldDb_ / grFullScaleDb);
    if (grHoldNorm > 0.005f && grHoldNorm > grNorm + 0.01f)
    {
        g.setColour(OmbicLookAndFeel::ombicRed().withAlpha(0.6f));
        float holdY = grBox.getBottom() - grBox.getHeight() * grHoldNorm;
        g.fillRect(grBox.getX() + 2, holdY - 1, grBox.getWidth() - 4, 2.0f);
    }
    g.setFont(OmbicLookAndFeel::getOmbicFontForPainting(9.0f, false));
    g.setColour(OmbicLookAndFeel::ink());
    g.drawText("GR", x - 2, static_cast<int>(grBox.getBottom()) + 2, meterW + 4, 12, juce::Justification::centred);
    g.drawText(juce::String(smoothedGrDb_, 1) + " dB", x - 2, static_cast<int>(grBox.getBottom()) + 14, meterW + 4, 12, juce::Justification::centred);
    x += meterW + gap;

    if (showPeak_)
        drawLevelMeterStereo(levelToNorm(peakOutL_), levelToNorm(peakOutR_), peakHoldOutDb_, OmbicLookAndFeel::ombicTeal(), "Out", outDisplayDb);
    else
        drawLevelMeterMono(outNorm, OmbicLookAndFeel::ombicTeal(), "Out", outDisplayDb);
}
