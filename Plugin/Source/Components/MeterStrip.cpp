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

void MeterStrip::mouseEnter(const juce::MouseEvent&)
{
    hovered_ = true;
    repaint();
}

void MeterStrip::mouseExit(const juce::MouseEvent&)
{
    hovered_ = false;
    repaint();
}

void MeterStrip::paint(juce::Graphics& g)
{
    float inNorm = levelToNorm(smoothedInDb_);
    float grNorm = juce::jlimit(0.0f, 1.0f, smoothedGrDb_ / 20.0f);
    float outNorm = levelToNorm(smoothedOutDb_);

    auto b = getLocalBounds().toFloat();
    const float shadowOff = hovered_ ? 6.0f : 8.0f;
    g.setColour(OmbicLookAndFeel::ink());
    g.fillRoundedRectangle(b.translated(shadowOff, shadowOff), 20.0f);
    g.setColour(OmbicLookAndFeel::bg());
    g.fillRoundedRectangle(b, 20.0f);
    g.setColour(OmbicLookAndFeel::ink());
    g.drawRoundedRectangle(b, 20.0f, 3.0f);
    if (hovered_)
    {
        g.setColour(OmbicLookAndFeel::ombicBlue().withAlpha(0.8f));
        g.drawRoundedRectangle(b.reduced(2.0f), 18.0f, 2.0f);
    }
    b = b.reduced(12.0f);

    const int meterW = 24;
    const int gap = 12;
    int x = static_cast<int>(b.getX()) + 4;

    auto drawVerticalMeter = [&](float norm, const juce::Colour& fillColour, const juce::String& title) {
        auto box = b.withX(static_cast<float>(x)).withWidth(static_cast<float>(meterW)).reduced(1);
        g.setColour(OmbicLookAndFeel::ink());
        g.drawRoundedRectangle(box, 4.0f, 2.0f);
        g.setColour(OmbicLookAndFeel::line());
        g.fillRoundedRectangle(box.reduced(1), 3.0f);
        float barH = box.getHeight() * norm;
        if (barH > 0.5f)
        {
            g.setColour(fillColour);
            g.fillRoundedRectangle(box.getX() + 2, box.getBottom() - barH, box.getWidth() - 4, barH, 3.0f);
        }
        g.setFont(OmbicLookAndFeel::getOmbicFontForPainting(9.0f, false));
        g.setColour(OmbicLookAndFeel::ink());
        g.drawText(title, x - 4, static_cast<int>(box.getBottom()) + 2, meterW + 8, 14, juce::Justification::centred);
        x += meterW + gap;
    };

    g.setFont(OmbicLookAndFeel::getOmbicFontForPainting(9.0f, false));
    g.setColour(OmbicLookAndFeel::ink());
    float boxH = b.getHeight() - 18.0f;
    int scaleX = static_cast<int>(b.getX()) - 16;
    g.drawText("0", scaleX, 0, 14, 12, juce::Justification::right);
    g.drawText("-20", scaleX, static_cast<int>(boxH * 0.33f) - 6, 14, 12, juce::Justification::right);
    g.drawText("-40", scaleX, static_cast<int>(boxH * 0.66f) - 6, 14, 12, juce::Justification::right);

    drawVerticalMeter(inNorm, OmbicLookAndFeel::ombicBlue(), "In");
    drawVerticalMeter(grNorm, OmbicLookAndFeel::ombicRed(), "GR");
    drawVerticalMeter(outNorm, OmbicLookAndFeel::ombicTeal(), "Out");
}
