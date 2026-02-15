#include "CompressorSection.h"
#include "../PluginProcessor.h"

//==============================================================================
CompressorSection::GainReductionMeterComponent::GainReductionMeterComponent(OmbicCompressorProcessor& p)
    : processor(p) {}

void CompressorSection::GainReductionMeterComponent::paint(juce::Graphics& g)
{
    float grDb = processor.gainReductionDb.load();
    smoothedGrDb_ += 0.12f * (grDb - smoothedGrDb_); // ~300 ms ballistics
    float norm = juce::jlimit(0.0f, 1.0f, smoothedGrDb_ / 20.0f);
    auto b = getLocalBounds().toFloat();
    g.setColour(OmbicLookAndFeel::line());
    g.fillRoundedRectangle(b, 4.0f);
    g.setColour(OmbicLookAndFeel::ombicRed());
    g.fillRoundedRectangle(b.removeFromBottom(b.getHeight() * norm), 4.0f);
    g.setColour(OmbicLookAndFeel::ink());
    g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(0.5f), 4.0f, 2.0f);
}

//==============================================================================
CompressorSection::CompressorSection(OmbicCompressorProcessor& processor)
    : proc(processor)
    , grMeter(processor)
    , transferCurve_(processor)
{
    setLookAndFeel(&ombicLf);

    modeCombo.addItem("Opto", 1);
    modeCombo.addItem("FET", 2);
    modeCombo.setSelectedId(1); // will be overwritten by attachment
    addAndMakeVisible(modeCombo);
    modeCombo.setVisible(false); // editor uses pills instead
    modeLabel.setText("Mode", juce::dontSendNotification);
    modeLabel.attachToComponent(&modeCombo, true);
    modeLabel.setVisible(false);
    addAndMakeVisible(transferCurve_);

    compressLimitCombo.addItem("Compress", 1);
    compressLimitCombo.addItem("Limit", 2);
    compressLimitCombo.setSelectedId(1);
    compressLimitCombo.setTooltip("Opto only: Compress = normal. Limit = more high-frequency in sidechain (tighter).");
    addAndMakeVisible(compressLimitCombo);
    compressLimitLabel.setText("COMPRESS / LIMIT", juce::dontSendNotification);
    compressLimitLabel.attachToComponent(&compressLimitCombo, true);
    addAndMakeVisible(compressLimitLabel);

    const juce::BorderSize<int> labelPadding(4, 0, 0, 0);
    const juce::Colour textCol = OmbicLookAndFeel::pluginText();
    const juce::Colour labelCol = OmbicLookAndFeel::pluginMuted();
    const juce::Font labelFont = OmbicLookAndFeel::getOmbicFontForPainting(9.0f, true);
    modeLabel.setBorderSize(labelPadding);
    modeLabel.setColour(juce::Label::textColourId, labelCol);
    modeLabel.setFont(labelFont);
    compressLimitLabel.setBorderSize(labelPadding);
    compressLimitLabel.setColour(juce::Label::textColourId, labelCol);
    compressLimitLabel.setFont(labelFont);
    thresholdSlider.setName("compressor");
    thresholdSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    thresholdSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 52, 18);
    thresholdSlider.setColour(juce::Slider::textBoxTextColourId, textCol);
    thresholdSlider.setVelocityBasedMode(false);
    addAndMakeVisible(thresholdSlider);
    thresholdLabel.setText("THRESHOLD", juce::dontSendNotification);
    thresholdLabel.setBorderSize(labelPadding);
    thresholdLabel.setColour(juce::Label::textColourId, labelCol);
    thresholdLabel.setFont(labelFont);
    addAndMakeVisible(thresholdLabel);

    ratioSlider.setName("compressor");
    ratioSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    ratioSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 52, 18);
    ratioSlider.setColour(juce::Slider::textBoxTextColourId, textCol);
    ratioSlider.setVelocityBasedMode(false);
    addAndMakeVisible(ratioSlider);
    ratioLabel.setText("RATIO", juce::dontSendNotification);
    ratioLabel.setBorderSize(labelPadding);
    ratioLabel.setColour(juce::Label::textColourId, labelCol);
    ratioLabel.setFont(labelFont);
    addAndMakeVisible(ratioLabel);

    attackSlider.setName("compressor");
    attackSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    attackSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 52, 18);
    attackSlider.setColour(juce::Slider::textBoxTextColourId, textCol);
    attackSlider.setVelocityBasedMode(false);
    addAndMakeVisible(attackSlider);
    attackLabel.setText("ATTACK", juce::dontSendNotification);
    attackLabel.setBorderSize(labelPadding);
    attackLabel.setColour(juce::Label::textColourId, labelCol);
    attackLabel.setFont(labelFont);
    addAndMakeVisible(attackLabel);

    releaseSlider.setName("compressor");
    releaseSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    releaseSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 52, 18);
    releaseSlider.setColour(juce::Slider::textBoxTextColourId, textCol);
    releaseSlider.setVelocityBasedMode(false);
    addAndMakeVisible(releaseSlider);
    releaseLabel.setText("RELEASE", juce::dontSendNotification);
    releaseLabel.setBorderSize(labelPadding);
    releaseLabel.setColour(juce::Label::textColourId, labelCol);
    releaseLabel.setFont(labelFont);
    addAndMakeVisible(releaseLabel);

    addAndMakeVisible(grMeter);
    grReadoutLabel.setText("0.0 dB", juce::dontSendNotification);
    grReadoutLabel.setJustificationType(juce::Justification::centred);
    grReadoutLabel.setFont(OmbicLookAndFeel::getOmbicFontForPainting(14.0f, true));
    addAndMakeVisible(grReadoutLabel);
}

CompressorSection::~CompressorSection()
{
    setLookAndFeel(nullptr);
}

void CompressorSection::updateGrReadout()
{
    float grDb = proc.gainReductionDb.load();
    smoothedGrDb_ += 0.12f * (grDb - smoothedGrDb_); // ~300 ms ballistics
    grReadoutLabel.setText(juce::String(smoothedGrDb_, 1) + " dB", juce::dontSendNotification);
    // Spec §8: GR color-coded — teal <3 dB, yellow 3–6 dB, red >6 dB
    float absGr = std::abs(smoothedGrDb_);
    if (absGr < 3.0f)
        grReadoutLabel.setColour(juce::Label::textColourId, OmbicLookAndFeel::ombicTeal());
    else if (absGr < 6.0f)
        grReadoutLabel.setColour(juce::Label::textColourId, OmbicLookAndFeel::ombicYellow());
    else
        grReadoutLabel.setColour(juce::Label::textColourId, OmbicLookAndFeel::ombicRed());
}

void CompressorSection::setModeControlsVisible(bool fetishParamsVisible)
{
    ratioSlider.setVisible(fetishParamsVisible);
    ratioLabel.setVisible(fetishParamsVisible);
    attackSlider.setVisible(fetishParamsVisible);
    attackLabel.setVisible(fetishParamsVisible);
    releaseSlider.setVisible(fetishParamsVisible);
    releaseLabel.setVisible(fetishParamsVisible);
    compressLimitCombo.setVisible(!fetishParamsVisible);
    compressLimitLabel.setVisible(!fetishParamsVisible);
}

bool CompressorSection::isInteracting() const
{
    return modeCombo.isMouseButtonDown() || compressLimitCombo.isMouseButtonDown() || thresholdSlider.isMouseButtonDown()
        || ratioSlider.isMouseButtonDown() || attackSlider.isMouseButtonDown() || releaseSlider.isMouseButtonDown();
}

void CompressorSection::setHighlight(bool on)
{
    if (highlighted_ != on) { highlighted_ = on; repaint(); }
}

void CompressorSection::paint(juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat();
    // Spec §4: module card — pluginSurface, 2px pluginBorder, 16px radius
    g.setColour(OmbicLookAndFeel::pluginSurface());
    g.fillRoundedRectangle(b, 16.0f);
    g.setColour(OmbicLookAndFeel::pluginBorder());
    g.drawRoundedRectangle(b, 16.0f, 2.0f);
    auto headerRect = b.removeFromTop(36.0f);
    g.setColour(OmbicLookAndFeel::ombicBlue());
    g.fillRoundedRectangle(headerRect.withBottom(headerRect.getY() + 36.0f), 16.0f);
    g.setColour(juce::Colours::white);
    g.setFont(OmbicLookAndFeel::getOmbicFontForPainting(13.0f, true));
    g.drawText("COMPRESSOR", static_cast<int>(headerRect.getX()) + 14, static_cast<int>(headerRect.getY()) + 8, 200, 20, juce::Justification::left);
}

void CompressorSection::resized()
{
    auto r = getLocalBounds();
    r.removeFromTop(36); // header
    const int bodyPad = 14;
    r.reduce(bodyPad, 0);
    r.removeFromBottom(bodyPad);

    // Spec §6: transfer curve full card width × 90px, margin bottom 14px
    transferCurve_.setBounds(r.getX(), r.getY(), r.getWidth(), 90);
    r.removeFromTop(90 + 14);

    const int labelH = 18;
    const int gap = 16;  // Spec §6: 16px gap between knob groups
    modeCombo.setBounds(0, 0, 1, 1);
    int x = r.getX();
    bool fetVisible = ratioSlider.isVisible();
    if (!fetVisible)
    {
        compressLimitLabel.setBounds(x, r.getY(), 120, labelH);
        compressLimitCombo.setBounds(x, r.getY() + labelH, 120, 28);
        x += 120 + gap;
    }
    auto placeKnob = [&](juce::Slider& sl, juce::Label& lb, int size) {
        lb.setBounds(x, r.getY(), size + gap, labelH);
        sl.setBounds(x, r.getY() + labelH, size, size);
        x += size + gap;
    };
    const int knobSizeOpto = 80;
    const int knobSizeFet = 60;
    if (fetVisible)
    {
        placeKnob(thresholdSlider, thresholdLabel, knobSizeFet);
        placeKnob(ratioSlider, ratioLabel, knobSizeFet);
        placeKnob(attackSlider, attackLabel, knobSizeFet);
        placeKnob(releaseSlider, releaseLabel, knobSizeFet);
    }
    else
    {
        // Spec §6 Opto: single 80px threshold centered in remaining space
        int optoAreaW = r.getWidth() - (x - r.getX());
        int threshX = x + (optoAreaW - knobSizeOpto) / 2;
        thresholdLabel.setBounds(threshX, r.getY(), knobSizeOpto + gap, labelH);
        thresholdSlider.setBounds(threshX, r.getY() + labelH, knobSizeOpto, knobSizeOpto);
        x = threshX + knobSizeOpto + gap;
    }

    const int grMeterW = 24;
    const int grReadoutH = 20;
    int knobH = fetVisible ? knobSizeFet : knobSizeOpto;
    grMeter.setBounds(x + gap, r.getY() + labelH, grMeterW, knobH - grReadoutH);
    grReadoutLabel.setBounds(x + gap, r.getY() + labelH + knobH - grReadoutH, grMeterW + 8, grReadoutH);

    // Spec §5: Opto — large param value (20px); FET — 14px
    if (!fetVisible)
        thresholdSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 56, 24);
    else
        thresholdSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 52, 18);
    float valueFontSize = fetVisible ? 14.0f : 20.0f;
    for (int i = 0; i < thresholdSlider.getNumChildComponents(); ++i)
    {
        if (auto* lb = dynamic_cast<juce::Label*>(thresholdSlider.getChildComponent(i)))
        {
            lb->setFont(OmbicLookAndFeel::getOmbicFontForPainting(valueFontSize, true));
            break;
        }
    }
}
