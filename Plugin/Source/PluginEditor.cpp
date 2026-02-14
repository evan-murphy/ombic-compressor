#include "PluginEditor.h"
#include "OmbicAssets.h"

//==============================================================================
OmbicCompressorEditor::OmbicCompressorEditor(OmbicCompressorProcessor& p)
    : AudioProcessorEditor(&p)
    , processorRef(p)
    , compressorSection(p)
    , saturatorSection(p)
    , outputSection(p)
    , transferCurve(p)
    , meterStrip(p)
{
    setLookAndFeel(&ombicLf);
    setSize(720, 500);
    setResizable(true, true);

    addAndMakeVisible(compressorSection);
    addAndMakeVisible(saturatorSection);
    addAndMakeVisible(outputSection);
    addAndMakeVisible(transferCurve);
    addAndMakeVisible(meterStrip);

    auto& apvts = processorRef.getValueTreeState();

    modeAttachment = std::make_unique<ComboBoxAttachment>(
        apvts, OmbicCompressorProcessor::paramCompressorMode, compressorSection.getModeCombo());
    thresholdAttachment = std::make_unique<SliderAttachment>(
        apvts, OmbicCompressorProcessor::paramThreshold, compressorSection.getThresholdSlider());
    ratioAttachment = std::make_unique<SliderAttachment>(
        apvts, OmbicCompressorProcessor::paramRatio, compressorSection.getRatioSlider());
    attackAttachment = std::make_unique<SliderAttachment>(
        apvts, OmbicCompressorProcessor::paramAttack, compressorSection.getAttackSlider());
    releaseAttachment = std::make_unique<SliderAttachment>(
        apvts, OmbicCompressorProcessor::paramRelease, compressorSection.getReleaseSlider());
    makeupAttachment = std::make_unique<SliderAttachment>(
        apvts, OmbicCompressorProcessor::paramMakeupGainDb, outputSection.getOutputSlider());

    neonDriveAttachment = std::make_unique<SliderAttachment>(
        apvts, OmbicCompressorProcessor::paramNeonDrive, saturatorSection.getDriveSlider());
    neonIntensityAttachment = std::make_unique<SliderAttachment>(
        apvts, OmbicCompressorProcessor::paramNeonIntensity, saturatorSection.getIntensitySlider());
    neonToneAttachment = std::make_unique<SliderAttachment>(
        apvts, OmbicCompressorProcessor::paramNeonTone, saturatorSection.getToneSlider());
    neonMixAttachment = std::make_unique<SliderAttachment>(
        apvts, OmbicCompressorProcessor::paramNeonMix, saturatorSection.getMixSlider());

    updateModeVisibility();
    startTimerHz(25);

    int logoSize = 0;
    const char* logoData = OmbicAssets::getNamedResource("Ombic_Alpha_png", logoSize);
    if (logoData != nullptr && logoSize > 0)
        logoImage_ = juce::ImageCache::getFromMemory(logoData, logoSize);
}

OmbicCompressorEditor::~OmbicCompressorEditor()
{
    setLookAndFeel(nullptr);
}

void OmbicCompressorEditor::timerCallback()
{
    updateModeVisibility();
    bool curveLoaded = processorRef.hasCurveDataLoaded();
    if (curveLoaded != lastCurveDataState_)
    {
        lastCurveDataState_ = curveLoaded;
        repaint();
    }
    compressorSection.updateGrReadout();
    transferCurve.repaint();
    meterStrip.repaint();
    if (compressorSection.getGainReductionMeter())
        compressorSection.getGainReductionMeter()->repaint();
}

void OmbicCompressorEditor::updateModeVisibility()
{
    auto* choice = processorRef.getValueTreeState().getParameter(OmbicCompressorProcessor::paramCompressorMode);
    if (!choice) return;
    float modeVal = choice->getValue();
    // Choice 0 = LALA (index 0), 1 = FETish (index 1)
    int modeIndex = static_cast<int>(modeVal * 1.99f);
    compressorSection.setModeControlsVisible(modeIndex == 1);
}

void OmbicCompressorEditor::paint(juce::Graphics& g)
{
    g.fillAll(OmbicLookAndFeel::line()); // light grey so white cards stand out (GUI spec 1.3)

    auto b = getLocalBounds();
    auto headerRect = b.removeFromTop(40);
    g.setColour(OmbicLookAndFeel::ombicRed()); // #ff001f header
    g.fillRect(headerRect);
    g.setColour(juce::Colours::white);

    const int logoH = 28;
    const int logoLeft = 8;
    if (logoImage_.isValid())
    {
        auto logoArea = headerRect.withLeft(logoLeft).withWidth(logoH * 2).withHeight(logoH).reduced(2);
        g.drawImageWithin(logoImage_, logoArea.getX(), logoArea.getY(), logoArea.getWidth(), logoArea.getHeight(),
                          juce::RectanglePlacement::centred, false);
        g.setFont(OmbicLookAndFeel::getOmbicFontForPainting(24.0f, true));
        g.drawText("Compressor", logoLeft + logoH * 2 + 8, 6, 220, 28, juce::Justification::left);
    }
    else
    {
        g.setFont(OmbicLookAndFeel::getOmbicFontForPainting(24.0f, true));
        g.drawText("OMBIC Compressor", 12, 6, 300, 28, juce::Justification::left);
    }
    g.setFont(OmbicLookAndFeel::getOmbicFontForPainting(12.0f, true));
    if (processorRef.hasCurveDataLoaded())
        g.drawText("Curve data: OK", headerRect.getWidth() - 140, 10, 128, 20, juce::Justification::right);
    else
    {
        g.setColour(juce::Colours::white.withAlpha(0.9f));
        g.drawText("No curve data", headerRect.getWidth() - 140, 10, 128, 20, juce::Justification::right);
    }
}

void OmbicCompressorEditor::resized()
{
    auto r = getLocalBounds();
    r.removeFromTop(40);
    const int pad = 12;
    const int shadowPad = 10; // space for card shadow (8px) so it isn't clipped
    r.reduce(pad, pad);
    r.removeFromRight(shadowPad);
    r.removeFromBottom(shadowPad);

    transferCurve.setBounds(r.removeFromTop(100));
    r.removeFromTop(pad);

    auto content = r;
    int stripW = 180;
    int meterStripH = 120;
    meterStrip.setBounds(content.getRight() - stripW - pad, content.getY(), stripW, meterStripH);
    content.removeFromRight(stripW + pad * 2);

    int outputSectionW = 72;
    outputSection.setBounds(content.getRight() - outputSectionW - pad, content.getY(), outputSectionW, content.getHeight());
    content.removeFromRight(outputSectionW + pad);

    int compW = content.getWidth() * 55 / 100;
    compressorSection.setBounds(content.removeFromLeft(compW));
    content.removeFromLeft(pad);
    saturatorSection.setBounds(content);
}
