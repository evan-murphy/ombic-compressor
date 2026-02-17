#include "PluginEditorV2.h"
#include "OmbicAssets.h"

OmbicCompressorEditorV2::OmbicCompressorEditorV2(OmbicCompressorProcessor& p)
    : AudioProcessorEditor(&p)
    , processorRef(p)
    , sidechainFilterSection(p)
    , compressorSection(p)
    , saturatorSection(p)
    , outputSection(p)
    , mainViewAsTube_(p)
{
    setLookAndFeel(&ombicLf);
    setSize(kBaseWidth, kBaseHeight);
    setResizable(true, true);
    setResizeLimits(kBaseWidth, kBaseHeight, 1600, 900);

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

    addAndMakeVisible(sidechainFilterSection);
    addAndMakeVisible(compressorSection);
    addAndMakeVisible(saturatorSection);
    addAndMakeVisible(outputSection);
    addAndMakeVisible(mainViewAsTube_);

    saturatorSection.setScopeVisible(false);
    compressorSection.setShowGrMeter(false);  // GR only in main view + output

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

    saturatorSection.applyPercentDisplay();

    updateModeVisibility();
    startTimerHz(25);

    int logoSize = 0;
    const char* logoData = OmbicAssets::getNamedResource("Ombic_Alpha_png", logoSize);
    if (logoData != nullptr && logoSize > 0)
        logoWatermark_ = juce::ImageCache::getFromMemory(logoData, logoSize);
}

OmbicCompressorEditorV2::~OmbicCompressorEditorV2()
{
    setLookAndFeel(nullptr);
}

void OmbicCompressorEditorV2::timerCallback()
{
    updateModeVisibility();
    compressorSection.updateGrReadout();
    compressorSection.updateCompressLimitButtonStates();
    outputSection.updateGrReadout();
    outputSection.repaint();
    saturatorSection.repaint();
    mainViewAsTube_.repaint();
    int modeId = compressorSection.getModeCombo().getSelectedId();
    optoPill_.setToggleState(modeId == 1, juce::dontSendNotification);
    fetPill_.setToggleState(modeId == 2, juce::dontSendNotification);

    bool curveLoaded = processorRef.hasCurveDataLoaded();
    if (curveLoaded != lastCurveDataState_)
    {
        lastCurveDataState_ = curveLoaded;
        repaint();
    }
}

void OmbicCompressorEditorV2::updateModeVisibility()
{
    auto* choice = processorRef.getValueTreeState().getParameter(OmbicCompressorProcessor::paramCompressorMode);
    if (!choice) return;
    int modeIndex = static_cast<int>(choice->getValue() * 1.99f);
    compressorSection.setModeControlsVisible(modeIndex == 1);
}

void OmbicCompressorEditorV2::paint(juce::Graphics& g)
{
    auto full = getLocalBounds();
    const int w = full.getWidth();
    const int h = full.getHeight();

    g.fillAll(OmbicLookAndFeel::pluginBg());

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

    g.setColour(OmbicLookAndFeel::pluginShadow());
    g.fillRoundedRectangle(full.toFloat().translated(8.0f, 8.0f), 20.0f);

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
        g.drawText("\u25CF SC LISTEN", w - 200, 0, 90, kHeaderH, juce::Justification::right);
    if (processorRef.hasCurveDataLoaded())
        g.drawText("\u25CF Curve OK", w - 100, 0, 90, kHeaderH, juce::Justification::right);
    else
        g.drawText("No curve data", w - 100, 0, 90, kHeaderH, juce::Justification::right);

    auto footerRect = getLocalBounds().withTop(h - kFooterH);
    g.setColour(OmbicLookAndFeel::pluginBorder());
    g.drawHorizontalLine(footerRect.getY(), 0.0f, static_cast<float>(w));
    g.setColour(OmbicLookAndFeel::pluginMuted());
    g.setFont(OmbicLookAndFeel::getOmbicFontForPainting(9.0f, true));
    g.drawText("OMBIC SOUND", 20, footerRect.getY(), 100, footerRect.getHeight(), juce::Justification::left);
    g.drawText("v2.0.0", footerRect.getCentreX() - 30, footerRect.getY(), 60, footerRect.getHeight(), juce::Justification::centred);
    g.drawText("STEREO", footerRect.getRight() - 55, footerRect.getY(), 50, footerRect.getHeight(), juce::Justification::right);
}

void OmbicCompressorEditorV2::resized()
{
    auto r = getLocalBounds();
    const int gridGap = 12;
    const int gridPadH = 12;
    const int gridPadBottom = 16;
    const int shadowOff = 8;
    const int pillsRowH = 32;

    r.removeFromTop(kHeaderH);
    auto pillsRow = r.removeFromTop(pillsRowH).reduced(gridPadH, 0);
    pillsRow.removeFromLeft(gridPadH);
    const int pillW = 84;
    const int pillGap = 12;
    optoPill_.setBounds(pillsRow.getX(), pillsRow.getY(), pillW, 26);
    fetPill_.setBounds(pillsRow.getX() + pillW + pillGap, pillsRow.getY(), pillW, 26);

    r.removeFromTop(gridGap);
    auto content = r.withTrimmedBottom(kFooterH).reduced(gridPadH, 0);
    content.removeFromLeft(shadowOff);
    content.removeFromBottom(shadowOff + gridPadBottom);
    content.removeFromRight(shadowOff);

    // §6: Main view height 120px default; when resizable proportional, min 100px
    const int contentAreaTotal = content.getHeight();
    const int defaultContentH = 382; // at 960×540 the content area height
    const int maxMainViewH = juce::jmax(100, contentAreaTotal - 80);  // leave ≥80px for control row
    const int mainViewH = juce::jlimit(100, maxMainViewH, contentAreaTotal * 120 / defaultContentH);
    auto mainViewRect = content.removeFromTop(mainViewH);
    mainViewAsTube_.setBounds(mainViewRect);

    int modeIndex = compressorSection.getModeCombo().getSelectedId() == 2 ? 1 : 0;

    // §4: Layout mechanism FlexBox or Grid from getLocalBounds()
    juce::Grid grid;
    grid.templateRows = { juce::Grid::TrackInfo(juce::Grid::Fr(1)) };
    const float scFr = 0.55f;
    float compFr = modeIndex == 0 ? 0.85f : 1.5f;
    float neonFr = modeIndex == 0 ? 2.2f : 1.8f;
    float outFr = modeIndex == 0 ? 0.7f : 0.6f;
    float total = scFr + compFr + neonFr + outFr;
    const int minScFilterW = 130;
    const int minCompW = 340;  // FET: 4 knobs need 4*(60+16)=304; body = width-28 so need 332+ → 340
    const int minOutW = 110;
    int available = content.getWidth() - 3 * gridGap;
    int scFilterW = juce::jmax(minScFilterW, static_cast<int>(available * (scFr / total)));
    int compW = juce::jmax(minCompW, static_cast<int>(available * (compFr / total)));
    int neonW = static_cast<int>(available * (neonFr / total));
    int outW = available - scFilterW - compW - neonW;
    if (outW < minOutW) { outW = minOutW; neonW = available - scFilterW - compW - outW; }

    grid.templateColumns = {
        juce::Grid::TrackInfo(juce::Grid::Px(scFilterW)),
        juce::Grid::TrackInfo(juce::Grid::Px(compW)),
        juce::Grid::TrackInfo(juce::Grid::Px(neonW)),
        juce::Grid::TrackInfo(juce::Grid::Px(outW))
    };
    grid.columnGap = juce::Grid::Px(gridGap);
    grid.items = {
        juce::GridItem(sidechainFilterSection).withArea(1, 1),
        juce::GridItem(compressorSection).withArea(1, 2),
        juce::GridItem(saturatorSection).withArea(1, 3),
        juce::GridItem(outputSection).withArea(1, 4)
    };
    grid.performLayout(content);
}
