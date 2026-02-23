#include "TransferCurveComponent.h"
#include "../PluginProcessor.h"

TransferCurveComponent::TransferCurveComponent(OmbicCompressorProcessor& processor)
    : proc(processor) {}

void TransferCurveComponent::resized() {}

void TransferCurveComponent::paint(juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat();
    // Spec §6: background pluginBg, border 1px pluginBorder, 10px radius
    g.setColour(OmbicLookAndFeel::pluginBg());
    g.fillRoundedRectangle(b, 10.0f);
    g.setColour(OmbicLookAndFeel::pluginBorder());
    g.drawRoundedRectangle(b.reduced(0.5f), 10.0f, 1.0f);

    if (!proc.hasCurveDataLoaded())
    {
        g.setColour(OmbicLookAndFeel::pluginMuted());
        g.setFont(OmbicLookAndFeel::getOmbicFontForPainting(11.0f, true));
        g.drawText("Compression curve data not found", b.toNearestInt(), juce::Justification::centred);
        return;
    }

    auto plotArea = b.reduced(20.0f, 10.0f);
    const float dbMin = -50.0f;
    const float dbMax = 0.0f;

    auto dbToX = [&](float db) {
        return plotArea.getX() + plotArea.getWidth() * (db - dbMin) / (dbMax - dbMin);
    };
    auto dbToY = [&](float db) {
        return plotArea.getBottom() - plotArea.getHeight() * (db - dbMin) / (dbMax - dbMin);
    };

    auto& apvts = proc.getValueTreeState();
    float thresholdRaw = 50.0f;
    float ratio = 4.0f;
    int mode = 0;
    if (auto* r = apvts.getRawParameterValue(OmbicCompressorProcessor::paramThreshold))
        thresholdRaw = r->load();
    if (auto* r = apvts.getRawParameterValue(OmbicCompressorProcessor::paramRatio))
        ratio = r->load();
    if (auto* r = apvts.getRawParameterValue(OmbicCompressorProcessor::paramCompressorMode))
        mode = juce::jlimit(0, 3, static_cast<int>(r->load() * 3.0f + 0.5f));

    // FET/VCA: threshold 0..100 -> dB -60..0 (VCA internal -1..3 mapped same for display); Opto: gentler; PWM: soft knee
    float threshDb = -60.0f + (thresholdRaw / 100.0f) * 60.0f;
    threshDb = juce::jlimit(dbMin, dbMax, threshDb);
    float displayRatio = (mode == 1 || mode == 3) ? juce::jmax(1.0f, ratio) : (mode == 2) ? juce::jlimit(1.5f, 8.0f, ratio) : 2.5f;
    const bool softKnee = (mode == 2);
    const float kneeDb = softKnee ? 3.0f : 0.0f;

    // Grid 6×6, 4% white; unity line dashed 8% white
    g.setColour(juce::Colour(0x0affffff));
    for (int i = 1; i < 6; ++i)
    {
        float f = static_cast<float>(i) / 6.0f;
        g.drawVerticalLine(static_cast<int>(plotArea.getX() + plotArea.getWidth() * f), plotArea.getY(), plotArea.getBottom());
        g.drawHorizontalLine(static_cast<int>(plotArea.getY() + plotArea.getHeight() * (1.0f - f)), plotArea.getX(), plotArea.getRight());
    }
    g.setColour(juce::Colour(0x14ffffff));
    float dashLen = 3.0f;
    g.drawDashedLine(juce::Line<float>(dbToX(dbMin), dbToY(dbMin), dbToX(dbMax), dbToY(dbMax)), &dashLen, 1, 1.0f);

    // Transfer curve: below threshold out = in; above (with optional soft knee) out = thresh + (in - thresh) / ratio
    juce::Path curve;
    const int steps = 64;
    bool started = false;
    for (int i = 0; i <= steps; ++i)
    {
        float inDb = dbMin + (dbMax - dbMin) * static_cast<float>(i) / static_cast<float>(steps);
        float outDb;
        if (inDb <= threshDb - kneeDb)
            outDb = inDb;
        else if (inDb >= threshDb + kneeDb)
            outDb = threshDb + (inDb - threshDb) / displayRatio;
        else if (kneeDb > 0.0f)
        {
            float t = (inDb - (threshDb - kneeDb)) / (2.0f * kneeDb);
            t = t * t * (3.0f - 2.0f * t);
            float linearOut = inDb;
            float compressedOut = threshDb + (inDb - threshDb) / displayRatio;
            outDb = linearOut + t * (compressedOut - linearOut);
        }
        else
            outDb = threshDb + (inDb - threshDb) / displayRatio;
        outDb = juce::jlimit(dbMin, dbMax, outDb);
        float x = dbToX(inDb);
        float y = dbToY(outDb);
        if (!started) { curve.startNewSubPath(x, y); started = true; }
        else curve.lineTo(x, y);
    }
    juce::Colour curveCol = (mode == 2) ? OmbicLookAndFeel::ombicTeal() : OmbicLookAndFeel::ombicBlue();
    g.setColour(curveCol);
    g.strokePath(curve, juce::PathStrokeType(2.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // Current input/output point (pink accent + red core per OMBIC fun colors)
    float inDb = proc.inputLevelDb.load();
    float outDb = proc.outputLevelDb.load();
    inDb = juce::jlimit(dbMin, dbMax, inDb);
    outDb = juce::jlimit(dbMin, dbMax, outDb);
    float px = dbToX(inDb);
    float py = dbToY(outDb);
    g.setColour(OmbicLookAndFeel::ombicPink().withAlpha(0.7f));
    g.fillEllipse(px - 5, py - 5, 10, 10);
    g.setColour(OmbicLookAndFeel::ombicRed());
    g.fillEllipse(px - 4, py - 4, 8, 8);
    g.setColour(OmbicLookAndFeel::ink());
    g.drawEllipse(px - 4, py - 4, 8, 8, 1.0f);

    // Axis labels: "OUT" top-left, "IN" bottom-right, 8px, 25% white
    g.setFont(OmbicLookAndFeel::getOmbicFontForPainting(8.0f, true));
    g.setColour(juce::Colour(0x40ffffff));
    g.drawText("OUT", static_cast<int>(plotArea.getX()), static_cast<int>(plotArea.getY() - 2), 24, 10, juce::Justification::left);
    g.drawText("IN", static_cast<int>(plotArea.getRight() - 18), static_cast<int>(plotArea.getBottom() - 10), 18, 10, juce::Justification::right);
}
