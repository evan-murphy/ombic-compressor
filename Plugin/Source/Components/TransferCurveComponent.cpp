#include "TransferCurveComponent.h"
#include "../PluginProcessor.h"

TransferCurveComponent::TransferCurveComponent(OmbicCompressorProcessor& processor)
    : proc(processor) {}

void TransferCurveComponent::resized() {}

void TransferCurveComponent::mouseEnter(const juce::MouseEvent&)
{
    hovered_ = true;
    repaint();
}

void TransferCurveComponent::mouseExit(const juce::MouseEvent&)
{
    hovered_ = false;
    repaint();
}

void TransferCurveComponent::paint(juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat();
    const float shadowOff = hovered_ ? 6.0f : 8.0f;
    g.setColour(OmbicLookAndFeel::ink());
    g.fillRoundedRectangle(b.translated(shadowOff, shadowOff), 12.0f);
    g.setColour(OmbicLookAndFeel::bg());
    g.fillRoundedRectangle(b, 12.0f);
    g.setColour(OmbicLookAndFeel::ink());
    g.drawRoundedRectangle(b, 12.0f, 2.0f);
    if (hovered_)
    {
        g.setColour(OmbicLookAndFeel::ombicBlue().withAlpha(0.8f));
        g.drawRoundedRectangle(b.reduced(2.0f), 10.0f, 2.0f);
    }

    auto plotArea = b.reduced(28.0f, 12.0f); // margins for labels
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
        mode = static_cast<int>(r->load() + 0.5f);

    // FET: threshold 0..100 -> dB -60..0; LALA: same 0..100 for display, but gentler curve
    float threshDb = -60.0f + (thresholdRaw / 100.0f) * 60.0f;
    threshDb = juce::jlimit(dbMin, dbMax, threshDb);
    float displayRatio = (mode == 1) ? juce::jmax(1.0f, ratio) : 2.5f; // LALA (opto) gentler

    // 1:1 reference (dashed)
    g.setColour(OmbicLookAndFeel::line());
    float dashLen = 4.0f;
    g.drawDashedLine(juce::Line<float>(dbToX(dbMin), dbToY(dbMin), dbToX(dbMax), dbToY(dbMax)), &dashLen, 1, 1.0f);

    // Transfer curve: below threshold out = in, above out = thresh + (in - thresh) / ratio
    juce::Path curve;
    const int steps = 64;
    bool started = false;
    for (int i = 0; i <= steps; ++i)
    {
        float inDb = dbMin + (dbMax - dbMin) * static_cast<float>(i) / static_cast<float>(steps);
        float outDb = inDb <= threshDb ? inDb : threshDb + (inDb - threshDb) / displayRatio;
        outDb = juce::jlimit(dbMin, dbMax, outDb);
        float x = dbToX(inDb);
        float y = dbToY(outDb);
        if (!started) { curve.startNewSubPath(x, y); started = true; }
        else curve.lineTo(x, y);
    }
    g.setColour(OmbicLookAndFeel::ombicBlue());
    g.strokePath(curve, juce::PathStrokeType(2.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

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

    // Axis labels
    g.setFont(OmbicLookAndFeel::getOmbicFontForPainting(10.0f, false));
    g.setColour(OmbicLookAndFeel::ink());
    g.drawText("In (dB)", static_cast<int>(plotArea.getCentreX() - 24), static_cast<int>(b.getBottom()) - 14, 48, 12, juce::Justification::centred);
    g.drawText("Out", 4, static_cast<int>(plotArea.getY() + plotArea.getHeight() * 0.5f - 6), 24, 12, juce::Justification::left);
}
