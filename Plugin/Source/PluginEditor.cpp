#include "PluginEditor.h"
#include "OmbicAssets.h"

//==============================================================================
OmbicCompressorEditor::OmbicCompressorEditor(OmbicCompressorProcessor& p)
    : AudioProcessorEditor(&p)
    , processorRef(p)
    , compressorSection(p)
    , saturatorSection(p)
    , outputSection(p)
    , meterStrip(p)
{
    setLookAndFeel(&ombicLf);
    const int specW = 900;
    const int specH = 480;
    setSize(specW, specH);
    setResizable(true, true);
    setResizeLimits(specW, specH, -1, -1);

    addAndMakeVisible(optoPill_);
    addAndMakeVisible(fetPill_);
    optoPill_.setName("optoPill");
    fetPill_.setName("fetPill");
    optoPill_.setClickingTogglesState(false);
    fetPill_.setClickingTogglesState(false);
    optoPill_.setButtonText("OPTO");
    fetPill_.setButtonText("FET");
    optoPill_.onClick = [this]() { compressorSection.getModeCombo().setSelectedId(1, juce::sendNotificationSync); };
    fetPill_.onClick  = [this]() { compressorSection.getModeCombo().setSelectedId(2, juce::sendNotificationSync); };

    addAndMakeVisible(compressorSection);
    addAndMakeVisible(saturatorSection);
    addAndMakeVisible(outputSection);
    addAndMakeVisible(meterStrip);

    auto& apvts = processorRef.getValueTreeState();

    modeAttachment = std::make_unique<ComboBoxAttachment>(
        apvts, OmbicCompressorProcessor::paramCompressorMode, compressorSection.getModeCombo());
    compressLimitAttachment = std::make_unique<ComboBoxAttachment>(
        apvts, OmbicCompressorProcessor::paramOptoCompressLimit, compressorSection.getCompressLimitCombo());
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
        logoWatermark_ = juce::ImageCache::getFromMemory(logoData, logoSize);
}

OmbicCompressorEditor::~OmbicCompressorEditor()
{
    setLookAndFeel(nullptr);
}

void OmbicCompressorEditor::timerCallback()
{
    updateModeVisibility();
    // §3: animate column widths toward target (300ms ease)
    if (contentW_ > 0)
    {
        int modeIndex = compressorSection.getModeCombo().getSelectedId() == 2 ? 1 : 0;
        float compFr = modeIndex == 0 ? 0.85f : 1.5f;
        float neonFr = modeIndex == 0 ? 1.6f : 1.3f;
        float outFr = modeIndex == 0 ? 0.7f : 0.6f;
        float total = compFr + neonFr + outFr;
        const int minOutW = 100;
        int available = contentW_;
        int tComp = juce::jmax(320, static_cast<int>(available * (compFr / total)));
        int tNeon = static_cast<int>(available * (neonFr / total));
        int tOut = available - tComp - tNeon;
        if (tOut < minOutW) { tOut = minOutW; tNeon = available - tComp - tOut; }
        animCompW_ += (static_cast<float>(tComp) - animCompW_) * animRate_;
        animNeonW_ += (static_cast<float>(tNeon) - animNeonW_) * animRate_;
        animOutW_ += (static_cast<float>(tOut) - animOutW_) * animRate_;
        applyColumnLayout(static_cast<int>(animCompW_ + 0.5f), static_cast<int>(animNeonW_ + 0.5f), static_cast<int>(animOutW_ + 0.5f));
    }

    bool curveLoaded = processorRef.hasCurveDataLoaded();
    if (curveLoaded != lastCurveDataState_)
    {
        lastCurveDataState_ = curveLoaded;
        repaint();
    }
    compressorSection.updateGrReadout();
    compressorSection.setHighlight(compressorSection.isInteracting());
    saturatorSection.setHighlight(saturatorSection.isInteracting());
    outputSection.setHighlight(outputSection.isInteracting());
    outputSection.updateGrReadout();
    outputSection.repaint();  // IN/OUT meters
    saturatorSection.repaint();  // neon capsule glow + scope
    meterStrip.repaint();
    int modeId = compressorSection.getModeCombo().getSelectedId();
    optoPill_.setToggleState(modeId == 1, juce::dontSendNotification);
    fetPill_.setToggleState(modeId == 2, juce::dontSendNotification);
    if (compressorSection.getGainReductionMeter())
        compressorSection.getGainReductionMeter()->repaint();
}

void OmbicCompressorEditor::updateModeVisibility()
{
    auto* choice = processorRef.getValueTreeState().getParameter(OmbicCompressorProcessor::paramCompressorMode);
    if (!choice) return;
    float modeVal = choice->getValue();
    // Choice 0 = Opto (index 0), 1 = FET (index 1)
    int modeIndex = static_cast<int>(modeVal * 1.99f);
    compressorSection.setModeControlsVisible(modeIndex == 1);
}

namespace
{
    const int kHeaderH = 38;
    const int kFooterH = 28;
}

void OmbicCompressorEditor::paint(juce::Graphics& g)
{
    auto full = getLocalBounds();
    const int w = full.getWidth();
    const int h = full.getHeight();

    // Spec §1: background #0f1220
    g.fillAll(OmbicLookAndFeel::pluginBg());

    // §1 Logo watermark: centered in plugin body, ~260px tall, 6% opacity, behind all modules
    if (logoWatermark_.isValid())
    {
        auto bodyArea = full.withTop(kHeaderH).withTrimmedBottom(kFooterH);
        const int logoH = 260;
        int logoW = (logoWatermark_.getWidth() * logoH) / juce::jmax(1, logoWatermark_.getHeight());
        logoW = juce::jmin(logoW, bodyArea.getWidth() - 40);
        int actualH = (logoWatermark_.getHeight() * logoW) / juce::jmax(1, logoWatermark_.getWidth());
        actualH = juce::jmin(actualH, logoH);
        auto logoRect = bodyArea.withSizeKeepingCentre(logoW, actualH);
        g.setOpacity(0.06f);
        g.drawImageWithin(logoWatermark_, logoRect.getX(), logoRect.getY(), logoRect.getWidth(), logoRect.getHeight(),
                          juce::RectanglePlacement::centred, false);
        g.setOpacity(1.0f);
    }

    // Spec §12: hard offset shadow 8px 8px, zero blur
    g.setColour(OmbicLookAndFeel::pluginShadow());
    g.fillRoundedRectangle(full.toFloat().translated(8.0f, 8.0f), 20.0f);

    // §9 Header: pluginSurface, quiet — "Ombic" pluginMuted, "Compressor" pluginText 90%, "● Curve OK" ombicTeal
    auto headerRect = full.removeFromTop(kHeaderH);
    g.setColour(OmbicLookAndFeel::pluginSurface());
    g.fillRect(headerRect);
    g.setColour(OmbicLookAndFeel::pluginBorder());
    g.drawHorizontalLine(kHeaderH - 1, 0.0f, static_cast<float>(w));
    g.setColour(OmbicLookAndFeel::pluginMuted());
    g.setFont(OmbicLookAndFeel::getOmbicFontForPainting(14.0f, true));
    g.drawText("Ombic", 12, 0, 60, kHeaderH, juce::Justification::left);
    g.setColour(OmbicLookAndFeel::pluginText().withAlpha(0.9f));
    g.drawText("Compressor", 68, 0, 180, kHeaderH, juce::Justification::left);
    g.setFont(OmbicLookAndFeel::getOmbicFontForPainting(10.0f, true));
    g.setColour(OmbicLookAndFeel::ombicTeal());
    if (processorRef.hasCurveDataLoaded())
        g.drawText("\u25CF Curve OK", w - 100, 0, 90, kHeaderH, juce::Justification::right);
    else
        g.drawText("No curve data", w - 100, 0, 90, kHeaderH, juce::Justification::right);

    // Spec §10: Footer — border top, 8px vertical padding, OMBIC SOUND | V1.0.0 | STEREO
    auto footerRect = full.withTop(h - kFooterH);
    g.setColour(OmbicLookAndFeel::pluginBorder());
    g.drawHorizontalLine(footerRect.getY(), 0.0f, static_cast<float>(w));
    g.setColour(OmbicLookAndFeel::pluginMuted());
    g.setFont(OmbicLookAndFeel::getOmbicFontForPainting(9.0f, true));
    g.drawText("OMBIC SOUND", 20, footerRect.getY(), 100, footerRect.getHeight(), juce::Justification::left);
    g.drawText("v1.0.0", footerRect.getCentreX() - 30, footerRect.getY(), 60, footerRect.getHeight(), juce::Justification::centred);
    g.drawText("STEREO", footerRect.getRight() - 55, footerRect.getY(), 50, footerRect.getHeight(), juce::Justification::right);
}

void OmbicCompressorEditor::applyColumnLayout(int compW, int neonW, int outW)
{
    const int gridGap = 12;
    int x = contentX_;
    compressorSection.setBounds(x, contentY_, compW, contentH_);
    x += compW + gridGap;
    saturatorSection.setBounds(x, contentY_, neonW, contentH_);
    x += neonW + gridGap;
    outputSection.setBounds(x, contentY_, outW, contentH_);
    meterStrip.setBounds(0, 0, 0, 0);
}

void OmbicCompressorEditor::resized()
{
    auto r = getLocalBounds();
    const int gridGap = 12;
    const int gridPadH = 12;
    const int gridPadBottom = 16;
    const int footerH = kFooterH;
    const int shadowOff = 8;

    r.removeFromTop(kHeaderH);
    auto pillsRow = r.removeFromTop(36).reduced(gridPadH, 0);
    pillsRow.removeFromLeft(gridPadH);
    const int pillW = 80;
    optoPill_.setBounds(pillsRow.getX(), pillsRow.getY(), pillW, 26);
    fetPill_.setBounds(pillsRow.getX() + pillW + 8, pillsRow.getY(), pillW, 26);

    r.removeFromTop(gridGap);
    auto content = r.withTrimmedBottom(footerH).reduced(gridPadH, 0);
    content.removeFromLeft(shadowOff);
    content.removeFromBottom(shadowOff + gridPadBottom);
    content.removeFromRight(shadowOff);

    contentX_ = content.getX();
    contentY_ = content.getY();
    contentH_ = content.getHeight();
    int available = content.getWidth() - 2 * gridGap;
    contentW_ = available;

    int modeIndex = compressorSection.getModeCombo().getSelectedId() == 2 ? 1 : 0;
    float compFr = modeIndex == 0 ? 0.85f : 1.5f;
    float neonFr = modeIndex == 0 ? 1.6f : 1.3f;
    float outFr = modeIndex == 0 ? 0.7f : 0.6f;
    float total = compFr + neonFr + outFr;
    const int minOutW = 100;
    int compW = juce::jmax(320, static_cast<int>(available * (compFr / total)));
    int neonW = static_cast<int>(available * (neonFr / total));
    int outW = available - compW - neonW;
    if (outW < minOutW) { outW = minOutW; neonW = available - compW - outW; }

    animCompW_ = static_cast<float>(compW);
    animNeonW_ = static_cast<float>(neonW);
    animOutW_ = static_cast<float>(outW);
    applyColumnLayout(compW, neonW, outW);
}
