#include "SidechainFilterSection.h"
#include "../PluginProcessor.h"
#include <cmath>

//==============================================================================
SidechainFilterSection::FrequencyResponseDisplay::FrequencyResponseDisplay(OmbicCompressorProcessor&)
{
}

void SidechainFilterSection::FrequencyResponseDisplay::paint(juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat();
    g.setColour(OmbicLookAndFeel::pluginBg());
    g.fillRoundedRectangle(b, 10.0f);
    g.setColour(OmbicLookAndFeel::pluginBorder());
    g.drawRoundedRectangle(b, 10.0f, 1.0f);

    const float freqMin = 20.0f;
    const float freqMax = 20000.0f;
    const float dbMin = -24.0f;
    const float dbMax = 0.0f;
    const bool off = frequencyHz_ <= 20.0f;

    auto inner = b.reduced(4.0f);

    juce::Path path;
    bool started = false;

    auto logX = [freqMin, freqMax, &inner](float freq) {
        if (freq <= 0) return inner.getX();
        float t = (std::log10(freq) - std::log10(freqMin)) / (std::log10(freqMax) - std::log10(freqMin));
        return inner.getX() + t * inner.getWidth();
    };
    auto dbToY = [dbMin, dbMax, &inner](float db) {
        float t = (db - dbMax) / (dbMin - dbMax);
        return inner.getY() + t * inner.getHeight();
    };

    if (off)
    {
        g.setColour(OmbicLookAndFeel::pluginMuted().withAlpha(0.4f));
        float y = dbToY(0.0f);
        g.drawHorizontalLine(juce::roundToInt(y), inner.getX(), inner.getRight());
        return;
    }

    auto magnitudeDb = [this, dbMin](float freq) -> float {
        if (freq <= 0) return dbMin;
        float u2 = (freq / frequencyHz_) * (freq / frequencyHz_);
        float magSq = (u2 * u2) / (1.0f + u2 * u2);
        float mag = std::sqrt(magSq);
        return 20.0f * std::log10(mag + 1e-10f);
    };

    const int steps = 80;
    for (int i = 0; i <= steps; ++i)
    {
        float t = static_cast<float>(i) / static_cast<float>(steps);
        float freq = freqMin * std::pow(freqMax / freqMin, t);
        float db = juce::jlimit(dbMin, dbMax, magnitudeDb(freq));
        float x = logX(freq);
        float y = dbToY(db);
        if (!started) { path.startNewSubPath(x, y); started = true; }
        else path.lineTo(x, y);
    }

    path.lineTo(logX(freqMax), inner.getBottom());
    path.lineTo(logX(freqMin), inner.getBottom());
    path.closeSubPath();
    g.setColour(OmbicLookAndFeel::ombicTeal().withAlpha(0.05f));
    g.fillPath(path);

    path.clear();
    started = false;
    for (int i = 0; i <= steps; ++i)
    {
        float t = static_cast<float>(i) / static_cast<float>(steps);
        float freq = freqMin * std::pow(freqMax / freqMin, t);
        float db = juce::jlimit(dbMin, dbMax, magnitudeDb(freq));
        float x = logX(freq);
        float y = dbToY(db);
        if (!started) { path.startNewSubPath(x, y); started = true; }
        else path.lineTo(x, y);
    }
    g.setColour(OmbicLookAndFeel::ombicTeal().withAlpha(0.15f));
    g.strokePath(path, juce::PathStrokeType(6.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    g.setColour(OmbicLookAndFeel::ombicTeal());
    g.strokePath(path, juce::PathStrokeType(1.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
}

//==============================================================================
SidechainFilterSection::SidechainFilterSection(OmbicCompressorProcessor& processor)
    : proc_(processor)
    , freqResponseDisplay_(processor)
{
    setLookAndFeel(&ombicLf_);
    addAndMakeVisible(freqResponseDisplay_);

    frequencySlider_.setName("sc_freq");
    frequencySlider_.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    frequencySlider_.setRotaryParameters(juce::Slider::RotaryParameters{ -2.356f, 2.356f, true });
    frequencySlider_.setColour(juce::Slider::rotarySliderFillColourId, OmbicLookAndFeel::ombicTeal());
    frequencySlider_.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    frequencySlider_.setRange(20.0, 500.0, 1.0);
    frequencySlider_.setValue(20.0);
    addAndMakeVisible(frequencySlider_);

    frequencyLabel_.setText("SC FREQ", juce::dontSendNotification);
    frequencyLabel_.setColour(juce::Label::textColourId, OmbicLookAndFeel::pluginMuted());
    frequencyLabel_.setFont(OmbicLookAndFeel::getOmbicFontForPainting(9.0f, true));
    addAndMakeVisible(frequencyLabel_);

    frequencyValueLabel_.setText("OFF", juce::dontSendNotification);
    frequencyValueLabel_.setColour(juce::Label::textColourId, OmbicLookAndFeel::pluginText());
    frequencyValueLabel_.setFont(OmbicLookAndFeel::getOmbicFontForPainting(14.0f, true));
    frequencyValueLabel_.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(frequencyValueLabel_);

    listenButton_.setButtonText("LISTEN");
    listenButton_.setName("sc_listen");
    listenButton_.setClickingTogglesState(true);
    addAndMakeVisible(listenButton_);

    startTimerHz(25);
}

SidechainFilterSection::~SidechainFilterSection()
{
    setLookAndFeel(nullptr);
}

void SidechainFilterSection::paint(juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat();
    g.setColour(OmbicLookAndFeel::pluginSurface());
    g.fillRoundedRectangle(b, 16.0f);
    g.setColour(OmbicLookAndFeel::pluginBorder());
    g.drawRoundedRectangle(b, 16.0f, 2.0f);
    const float headerH = getHeight() < 110 ? 22.0f : 36.0f;  // §6 Module card header 36px
    auto headerRect = b.removeFromTop(headerH);
    g.setColour(OmbicLookAndFeel::ombicTeal());
    g.fillRoundedRectangle(headerRect.withBottom(headerRect.getY() + headerH), 16.0f);
    g.setColour(juce::Colours::white);
    g.setFont(OmbicLookAndFeel::getOmbicFontForPainting(13.0f, true));  // §7 Module headers 13px
    g.drawText("SC FILTER", static_cast<int>(headerRect.getX()) + 12, static_cast<int>((headerH - 13.0f) * 0.5f), 120, 14, juce::Justification::left);
}

void SidechainFilterSection::resized()
{
    auto r = getLocalBounds();
    const bool compact = (r.getHeight() < 110);
    const int headerH = compact ? 22 : 36;  // §6 header 36px
    r.removeFromTop(headerH);
    const int bodyPad = compact ? 8 : 14;   // §6 body padding 14px
    r.reduce(bodyPad, 0);
    r.removeFromBottom(bodyPad);

    const int respH = compact ? 0 : 52;
    const int marginBottom = compact ? 0 : 18;
    if (respH > 0)
    {
        freqResponseDisplay_.setBounds(r.getX(), r.getY(), r.getWidth(), respH);
        r.removeFromTop(respH + marginBottom);
    }
    else
    {
        freqResponseDisplay_.setBounds(0, 0, 0, 0);
    }

    const int knobSize = compact ? 50 : 68;   // SC freq knob: visible, easy to grab
    const int listenW = compact ? 28 : 32;
    const int listenH = compact ? 20 : 24;
    const int labelH = compact ? 10 : 14;
    int x = r.getX();
    frequencyLabel_.setBounds(x, r.getY(), knobSize + 20, labelH);
    frequencySlider_.setBounds(x, r.getY() + labelH, knobSize, knobSize);
    frequencyValueLabel_.setBounds(x, r.getY() + labelH + knobSize, knobSize, compact ? 12 : 18);
    int listenX = x + knobSize + 8;
    int listenY = r.getY() + labelH + (knobSize - listenH) / 2;
    listenButton_.setBounds(listenX, listenY, listenW, listenH);
}

void SidechainFilterSection::timerCallback()
{
    if (auto* r = proc_.getValueTreeState().getRawParameterValue(OmbicCompressorProcessor::paramScFrequency))
    {
        float hz = r->load();
        freqResponseDisplay_.setFrequencyHz(hz);
        if (hz <= 20.5f)
            frequencyValueLabel_.setText("OFF", juce::dontSendNotification);
        else
            frequencyValueLabel_.setText(juce::String(juce::roundToInt(hz)) + " Hz", juce::dontSendNotification);
    }
    freqResponseDisplay_.repaint();
}
