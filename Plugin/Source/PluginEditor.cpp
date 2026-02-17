#include "PluginEditor.h"
#include "OmbicAssets.h"

//==============================================================================
OmbicCompressorEditor::OmbicCompressorEditor(OmbicCompressorProcessor& p)
    : AudioProcessorEditor(&p)
    , processorRef(p)
    , sidechainFilterSection(p)
    , compressorSection(p)
    , saturatorSection(p)
    , outputSection(p)
    , meterStrip(p)
    , mainVu_(p)
{
    setLookAndFeel(&ombicLf);
    const int specW = 900;
    const int specH = 480;
    setSize(specW, specH);
    setResizable(true, true);
    setResizeLimits(specW, specH, -1, -1);

    addAndMakeVisible(optoPill_);
    addAndMakeVisible(fetPill_);
    addAndMakeVisible(pwmPill_);
    addAndMakeVisible(vcaPill_);
    optoPill_.setName("optoPill");
    fetPill_.setName("fetPill");
    pwmPill_.setName("pwmPill");
    vcaPill_.setName("vcaPill");
    optoPill_.setClickingTogglesState(false);
    fetPill_.setClickingTogglesState(false);
    pwmPill_.setClickingTogglesState(false);
    vcaPill_.setClickingTogglesState(false);
    optoPill_.setButtonText("OPTO");
    fetPill_.setButtonText("FET");
    pwmPill_.setButtonText("PWM");
    vcaPill_.setButtonText("VCA");
    optoPill_.onClick = [this]() { compressorSection.getModeCombo().setSelectedId(1, juce::sendNotificationSync); };
    fetPill_.onClick  = [this]() { compressorSection.getModeCombo().setSelectedId(2, juce::sendNotificationSync); };
    pwmPill_.onClick  = [this]() { compressorSection.getModeCombo().setSelectedId(3, juce::sendNotificationSync); };
    vcaPill_.onClick  = [this]() { compressorSection.getModeCombo().setSelectedId(4, juce::sendNotificationSync); };

    addAndMakeVisible(sidechainFilterSection);
    addAndMakeVisible(compressorSection);
    addAndMakeVisible(saturatorSection);
    addAndMakeVisible(outputSection);
    addAndMakeVisible(meterStrip);
    addAndMakeVisible(mainVu_);

    auto& apvts = processorRef.getValueTreeState();

    scFrequencyAttachment = std::make_unique<SliderAttachment>(
        apvts, OmbicCompressorProcessor::paramScFrequency, sidechainFilterSection.getFrequencySlider());
    scListenAttachment = std::make_unique<ButtonAttachment>(
        apvts, OmbicCompressorProcessor::paramScListen, sidechainFilterSection.getListenButton());
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
    speedAttachment = std::make_unique<SliderAttachment>(
        apvts, OmbicCompressorProcessor::paramPwmSpeed, compressorSection.getSpeedSlider());
    makeupAttachment = std::make_unique<SliderAttachment>(
        apvts, OmbicCompressorProcessor::paramMakeupGainDb, outputSection.getOutputSlider());
    ironAttachment = std::make_unique<SliderAttachment>(
        apvts, OmbicCompressorProcessor::paramIron, outputSection.getIronSlider());
    autoGainAttachment = std::make_unique<ButtonAttachment>(
        apvts, OmbicCompressorProcessor::paramAutoGain, outputSection.getAutoGainButton());

    neonDriveAttachment = std::make_unique<SliderAttachment>(
        apvts, OmbicCompressorProcessor::paramNeonDrive, saturatorSection.getDriveSlider());
    neonIntensityAttachment = std::make_unique<SliderAttachment>(
        apvts, OmbicCompressorProcessor::paramNeonIntensity, saturatorSection.getIntensitySlider());
    neonToneAttachment = std::make_unique<SliderAttachment>(
        apvts, OmbicCompressorProcessor::paramNeonTone, saturatorSection.getToneSlider());
    neonMixAttachment = std::make_unique<SliderAttachment>(
        apvts, OmbicCompressorProcessor::paramNeonMix, saturatorSection.getMixSlider());

    // Ensure 0–100% display for Neon knobs (params are 0–1; show "50%" not "0.5")
    saturatorSection.applyPercentDisplay();

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
        const int gridGap = 12;
        const int minScFilterW = 130;
        const int minOutW = 110;
        int available = contentW_ - 3 * gridGap;
        float scFr = 0.55f;
        int modeId = compressorSection.getModeCombo().getSelectedId();
        int modeIndex = (modeId == 2) ? 1 : (modeId == 3) ? 2 : (modeId == 4) ? 3 : 0;
        float compFr = (modeIndex == 0) ? 0.85f : (modeIndex == 2) ? 1.2f : (modeIndex == 3) ? 1.5f : 1.5f;  // Opto / PWM / FET / VCA
        float neonFr = modeIndex == 0 ? 2.2f : 1.8f;   // neon bulb gets more width for bigger knobs
        float outFr = modeIndex == 0 ? 0.7f : 0.6f;
        float total = scFr + compFr + neonFr + outFr;
        int tScFilter = juce::jmax(minScFilterW, static_cast<int>(available * (scFr / total)));
        int tComp = juce::jmax(340, static_cast<int>(available * (compFr / total)));
        int tNeon = static_cast<int>(available * (neonFr / total));
        int tOut = available - tScFilter - tComp - tNeon;
        if (tOut < minOutW) { tOut = minOutW; tNeon = available - tScFilter - tComp - tOut; }
        animScFilterW_ += (static_cast<float>(tScFilter) - animScFilterW_) * animRate_;
        animCompW_ += (static_cast<float>(tComp) - animCompW_) * animRate_;
        animNeonW_ += (static_cast<float>(tNeon) - animNeonW_) * animRate_;
        animOutW_ += (static_cast<float>(tOut) - animOutW_) * animRate_;
        applyColumnLayout(static_cast<int>(animScFilterW_ + 0.5f), static_cast<int>(animCompW_ + 0.5f), static_cast<int>(animNeonW_ + 0.5f), static_cast<int>(animOutW_ + 0.5f), mainVuH_);
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
    pwmPill_.setToggleState(modeId == 3, juce::dontSendNotification);
    vcaPill_.setToggleState(modeId == 4, juce::dontSendNotification);
    if (compressorSection.getGainReductionMeter())
        compressorSection.getGainReductionMeter()->repaint();
}

void OmbicCompressorEditor::updateModeVisibility()
{
    auto* choice = processorRef.getValueTreeState().getParameter(OmbicCompressorProcessor::paramCompressorMode);
    if (!choice) return;
    int modeIndex = juce::jlimit(0, 3, static_cast<int>(choice->getValue() * 3.0f + 0.5f));
    compressorSection.setModeControlsVisible(modeIndex);
}

namespace
{
    const int kHeaderH = 38;
    const int kFooterH = 26;
    // Controls-first: strip gets enough height so sections are NOT compact (>= 110px) and knobs are usable
    const int kControlsStripH = 180;
    const int kMainVuMinH = 80;
    inline int getMainVuHeight(int contentAreaHeight)
    {
        return juce::jmax(kMainVuMinH, contentAreaHeight - kControlsStripH);
    }
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

    // §9 Header: ombicRed bg, "OMBIC COMPRESSOR" 18px weight 900 white, "● Curve OK" 10px weight 900
    auto headerRect = full.removeFromTop(kHeaderH);
    g.setColour(OmbicLookAndFeel::ombicRed());
    g.fillRect(headerRect);
    g.setColour(OmbicLookAndFeel::pluginBorder());
    g.drawHorizontalLine(kHeaderH - 1, 0.0f, static_cast<float>(w));
    g.setColour(juce::Colours::white);
    g.setFont(OmbicLookAndFeel::getOmbicFontForPainting(18.0f, true));
    g.drawText("OMBIC COMPRESSOR", 12, 0, 220, kHeaderH, juce::Justification::left);
    g.setFont(OmbicLookAndFeel::getOmbicFontForPainting(10.0f, true));
    if (processorRef.isScListenActive())
    {
        g.setColour(juce::Colours::white);
        g.drawText("\u25CF SC LISTEN", w - 200, 0, 90, kHeaderH, juce::Justification::right);
    }
    g.setColour(juce::Colours::white);
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

void OmbicCompressorEditor::applyColumnLayout(int scFilterW, int compW, int neonW, int outW, int mainVuH)
{
    const int gridGap = 12;
    int x = contentX_;
    sidechainFilterSection.setBounds(x, contentY_, scFilterW, contentH_);
    x += scFilterW + gridGap;
    compressorSection.setBounds(x, contentY_, compW, contentH_);
    x += compW + gridGap;
    saturatorSection.setBounds(x, contentY_, neonW, contentH_);
    x += neonW + gridGap;
    outputSection.setBounds(x, contentY_, outW, contentH_);
    meterStrip.setBounds(0, 0, 0, 0);
    mainVu_.setBounds(mainVuX_, mainVuY_, mainVuW_, mainVuH);
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
    auto pillsRow = r.removeFromTop(32).reduced(gridPadH, 0);
    pillsRow.removeFromLeft(gridPadH);
    const int pillW = 84;
    const int pillGap = 12;
    optoPill_.setBounds(pillsRow.getX(), pillsRow.getY(), pillW, 26);
    fetPill_.setBounds(pillsRow.getX() + pillW + pillGap, pillsRow.getY(), pillW, 26);
    pwmPill_.setBounds(pillsRow.getX() + 2 * (pillW + pillGap), pillsRow.getY(), pillW, 26);
    vcaPill_.setBounds(pillsRow.getX() + 3 * (pillW + pillGap), pillsRow.getY(), pillW, 26);

    r.removeFromTop(gridGap);
    auto content = r.withTrimmedBottom(footerH).reduced(gridPadH, 0);
    content.removeFromLeft(shadowOff);
    content.removeFromBottom(shadowOff + gridPadBottom);
    content.removeFromRight(shadowOff);

    const int contentTotalH = content.getHeight();
    const int mainVuH = getMainVuHeight(contentTotalH);
    mainVuX_ = content.getX();
    mainVuY_ = content.getY();
    mainVuW_ = content.getWidth();
    contentX_ = content.getX();
    contentY_ = content.getY() + mainVuH;
    contentH_ = contentTotalH - mainVuH;
    mainVuH_ = mainVuH;
    const int numGaps = 3;
    int available = content.getWidth() - numGaps * gridGap;
    contentW_ = content.getWidth();

    const float scFr = 0.55f;
    int modeIndex = compressorSection.getModeCombo().getSelectedId() == 2 ? 1 : 0;
    float compFr = modeIndex == 0 ? 0.85f : 1.5f;
    float neonFr = modeIndex == 0 ? 2.2f : 1.8f;   // neon bulb gets more width for bigger knobs
    float outFr = modeIndex == 0 ? 0.7f : 0.6f;
    float total = scFr + compFr + neonFr + outFr;
    const int minScFilterW = 130;
    const int minOutW = 110;
    int scFilterW = juce::jmax(minScFilterW, static_cast<int>(available * (scFr / total)));
    int compW = juce::jmax(340, static_cast<int>(available * (compFr / total)));
    int neonW = static_cast<int>(available * (neonFr / total));
    int outW = available - scFilterW - compW - neonW;
    if (outW < minOutW) { outW = minOutW; neonW = available - scFilterW - compW - outW; }

    animScFilterW_ = static_cast<float>(scFilterW);
    animCompW_ = static_cast<float>(compW);
    animNeonW_ = static_cast<float>(neonW);
    animOutW_ = static_cast<float>(outW);
    applyColumnLayout(scFilterW, compW, neonW, outW, mainVuH);
}
