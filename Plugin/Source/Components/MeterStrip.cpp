#include "MeterStrip.h"
#include "../PluginProcessor.h"

static float levelToNorm(float db)
{
    return juce::jlimit(0.0f, 1.0f, (db + 60.0f) / 60.0f);
}

MeterStrip::MeterStrip(OmbicCompressorProcessor& processor)
    : proc(processor)
{
    startTimerHz(25);
}

void MeterStrip::timerCallback()
{
    float inDb = proc.inputLevelDb.load();
    float grDb = proc.gainReductionDb.load();
    float outDb = proc.outputLevelDb.load();
    smoothedInDb_ += ballisticsCoeff_ * (inDb - smoothedInDb_);
    smoothedGrDb_ += ballisticsCoeff_ * (grDb - smoothedGrDb_);
    smoothedOutDb_ += ballisticsCoeff_ * (outDb - smoothedOutDb_);
    repaint();
}

void MeterStrip::resized() {}

void MeterStrip::paint(juce::Graphics& g)
{
    float inNorm = levelToNorm(smoothedInDb_);
    float grNorm = juce::jlimit(0.0f, 1.0f, smoothedGrDb_ / 20.0f);
    float outNorm = levelToNorm(smoothedOutDb_);

    auto b = getLocalBounds().toFloat();
    const float shadowOff = 8.0f;
    g.setColour(OmbicLookAndFeel::ink());
    g.fillRoundedRectangle(b.translated(shadowOff, shadowOff), 20.0f);
    g.setColour(OmbicLookAndFeel::bg());
    g.fillRoundedRectangle(b, 20.0f);
    g.setColour(OmbicLookAndFeel::ink());
    g.drawRoundedRectangle(b, 20.0f, 3.0f);
    b = b.reduced(12.0f);

    const int meterW = 28;
    const int gap = 10;
    const float boxH = b.getHeight() - 36.0f;
    int x = static_cast<int>(b.getX()) + 24;

    // Shared dB scale for level meters (In/Out): 0, -20, -40, -60 dB
    g.setFont(OmbicLookAndFeel::getOmbicFontForPainting(9.0f, false));
    g.setColour(OmbicLookAndFeel::ink());
    int scaleX = static_cast<int>(b.getX());
    auto dbToY = [&](float db) {
        return static_cast<int>(boxH * (1.0f - (db + 60.0f) / 60.0f));
    };
    g.drawText("0 dB", scaleX, dbToY(0) - 6, 22, 12, juce::Justification::right);
    g.drawText("-20", scaleX, dbToY(-20.0f) - 6, 22, 12, juce::Justification::right);
    g.drawText("-40", scaleX, dbToY(-40.0f) - 6, 22, 12, juce::Justification::right);
    g.drawText("-60", scaleX, dbToY(-60.0f) - 6, 22, 12, juce::Justification::right);

    auto drawLevelMeter = [&](float norm, const juce::Colour& fillColour,
                              const juce::String& title, float currentDb) {
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
        juce::String dbStr = (currentDb <= -60.0f) ? "-60" : juce::String(static_cast<int>(currentDb));
        g.drawText(dbStr + " dB", x - 2, static_cast<int>(box.getBottom()) + 14, meterW + 4, 12, juce::Justification::centred);
        x += meterW + gap;
    };

    drawLevelMeter(inNorm, OmbicLookAndFeel::ombicBlue(), "In", smoothedInDb_);

    // GR meter: 0 to -20 dB (bar = gain reduction in dB), with numeric readout
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
    g.setFont(OmbicLookAndFeel::getOmbicFontForPainting(9.0f, false));
    g.setColour(OmbicLookAndFeel::ink());
    g.drawText("GR", x - 2, static_cast<int>(grBox.getBottom()) + 2, meterW + 4, 12, juce::Justification::centred);
    juce::String grDbStr = juce::String(smoothedGrDb_, 1) + " dB";
    g.drawText(grDbStr, x - 2, static_cast<int>(grBox.getBottom()) + 14, meterW + 4, 12, juce::Justification::centred);
    x += meterW + gap;

    drawLevelMeter(outNorm, OmbicLookAndFeel::ombicTeal(), "Out", smoothedOutDb_);
}
