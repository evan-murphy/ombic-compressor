#include "CompressorSection.h"
#include "../PluginProcessor.h"

//==============================================================================
CompressorSection::GainReductionMeterComponent::GainReductionMeterComponent(OmbicCompressorProcessor& p)
    : processor(p) {}

void CompressorSection::GainReductionMeterComponent::paint(juce::Graphics& g)
{
    float grDb = processor.gainReductionDb.load();
    if (grDb > smoothedGrDb_)
        smoothedGrDb_ += kGrAttackCoeff * (grDb - smoothedGrDb_);
    else
        smoothedGrDb_ += kGrReleaseCoeff * (grDb - smoothedGrDb_);
    if (smoothedGrDb_ > grHoldDb_) { grHoldDb_ = smoothedGrDb_; grHoldTicks_ = kGrHoldTicks; }
    else { if (grHoldTicks_ > 0) --grHoldTicks_; if (grHoldTicks_ <= 0) grHoldDb_ += kGrReleaseCoeff * (smoothedGrDb_ - grHoldDb_); }
    const float grFullScaleDb = 40.0f;
    float norm = juce::jlimit(0.0f, 1.0f, smoothedGrDb_ / grFullScaleDb);
    auto fullBounds = getLocalBounds().toFloat();
    auto b = fullBounds;
    g.setColour(OmbicLookAndFeel::line());
    g.fillRoundedRectangle(b, 4.0f);
    g.setColour(OmbicLookAndFeel::ombicRed());
    g.fillRoundedRectangle(b.removeFromBottom(b.getHeight() * norm), 4.0f);
    float holdNorm = juce::jlimit(0.0f, 1.0f, grHoldDb_ / grFullScaleDb);
    if (holdNorm > 0.01f && holdNorm > norm + 0.02f)
    {
        g.setColour(OmbicLookAndFeel::ombicRed().withAlpha(0.6f));
        float holdY = fullBounds.getBottom() - fullBounds.getHeight() * holdNorm;
        g.fillRect(fullBounds.getX() + 1, holdY - 0.5f, fullBounds.getWidth() - 2, 1.0f);
    }
    g.setColour(OmbicLookAndFeel::ink());
    g.drawRoundedRectangle(fullBounds.reduced(0.5f), 4.0f, 2.0f);
}

//==============================================================================
CompressorSection::CompressorSection(OmbicCompressorProcessor& processor)
    : proc(processor)
    , grMeter(processor)
{
    setLookAndFeel(&ombicLf);

    modeCombo.addItem("Opto", 1);
    modeCombo.addItem("FET", 2);
    modeCombo.addItem("PWM", 3);
    modeCombo.addItem("VCA", 4);
    modeCombo.setSelectedId(1); // will be overwritten by attachment
    addAndMakeVisible(modeCombo);
    modeCombo.setVisible(false); // editor uses pills instead
    modeLabel.setText("Mode", juce::dontSendNotification);
    modeLabel.attachToComponent(&modeCombo, true);
    modeLabel.setVisible(false);

    compressLimitCombo.addItem("Compress", 1);
    compressLimitCombo.addItem("Limit", 2);
    compressLimitCombo.setSelectedId(1);
    compressLimitCombo.setTooltip("Opto only: Compress = normal. Limit = more high-frequency in sidechain (tighter).");
    addChildComponent(compressLimitCombo); // kept for attachment; UI uses toggle
    compressLimitCombo.setVisible(false);
    compressLimitLabel.setVisible(false);
    compressLimitLabel.setText("COMPRESS / LIMIT", juce::dontSendNotification);
    compressLimitLabel.attachToComponent(&compressLimitCombo, true);
    addChildComponent(compressLimitLabel);

    fetCharacterCombo.addItem("Off", 1);
    fetCharacterCombo.addItem("Rev A", 2);
    fetCharacterCombo.addItem("LN", 3);
    fetCharacterCombo.setSelectedId(1);
    fetCharacterCombo.setTooltip("FET only: character of gain reduction. Off = default; Rev A = more grab; LN = gentler.");
    addChildComponent(fetCharacterCombo);
    fetCharacterCombo.setVisible(false);
    fetCharacterLabel.setText("CHARACTER", juce::dontSendNotification);
    fetCharacterLabel.attachToComponent(&fetCharacterCombo, true);
    addChildComponent(fetCharacterLabel);
    fetCharacterLabel.setVisible(false);

    fetCharacterPillOff_.setButtonText("Bypass");
    fetCharacterPillOff_.setClickingTogglesState(false);
    fetCharacterPillOff_.setTooltip("FET only: default character.");
    fetCharacterPillOff_.onClick = [this]() { setFetCharacterFromPill(0); };
    addChildComponent(fetCharacterPillOff_);
    fetCharacterPillOff_.setVisible(false);
    fetCharacterPillRevA_.setButtonText("Rev A");
    fetCharacterPillRevA_.setClickingTogglesState(false);
    fetCharacterPillRevA_.setTooltip("FET only: more grab in knee.");
    fetCharacterPillRevA_.onClick = [this]() { setFetCharacterFromPill(1); };
    addChildComponent(fetCharacterPillRevA_);
    fetCharacterPillRevA_.setVisible(false);
    fetCharacterPillLN_.setButtonText("LN");
    fetCharacterPillLN_.setClickingTogglesState(false);
    fetCharacterPillLN_.setTooltip("FET only: gentler character.");
    fetCharacterPillLN_.onClick = [this]() { setFetCharacterFromPill(2); };
    addChildComponent(fetCharacterPillLN_);
    fetCharacterPillLN_.setVisible(false);

    compressLimitToggle_.setName("compressLimitSwitch");
    compressLimitToggle_.setClickingTogglesState(true);
    compressLimitToggle_.setTooltip("Opto only: Compress = normal. Limit = more high-frequency in sidechain (tighter).");
    compressLimitToggle_.onClick = [this]() {
        compressLimitCombo.setSelectedId(compressLimitToggle_.getToggleState() ? 2 : 1, juce::sendNotificationSync);
    };
    addAndMakeVisible(compressLimitToggle_);

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
    thresholdSlider.setRotaryParameters(juce::Slider::RotaryParameters{ -2.356f, 2.356f, true });
    thresholdSlider.setColour(juce::Slider::rotarySliderFillColourId, OmbicLookAndFeel::ombicBlue());
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
    ratioSlider.setRotaryParameters(juce::Slider::RotaryParameters{ -2.356f, 2.356f, true });
    ratioSlider.setColour(juce::Slider::rotarySliderFillColourId, OmbicLookAndFeel::ombicBlue());
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
    attackSlider.setRotaryParameters(juce::Slider::RotaryParameters{ -2.356f, 2.356f, true });
    attackSlider.setColour(juce::Slider::rotarySliderFillColourId, OmbicLookAndFeel::ombicBlue());
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
    releaseSlider.setRotaryParameters(juce::Slider::RotaryParameters{ -2.356f, 2.356f, true });
    releaseSlider.setColour(juce::Slider::rotarySliderFillColourId, OmbicLookAndFeel::ombicBlue());
    releaseSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 52, 18);
    releaseSlider.setColour(juce::Slider::textBoxTextColourId, textCol);
    releaseSlider.setVelocityBasedMode(false);
    addAndMakeVisible(releaseSlider);
    releaseLabel.setText("RELEASE", juce::dontSendNotification);
    releaseLabel.setBorderSize(labelPadding);
    releaseLabel.setColour(juce::Label::textColourId, labelCol);
    releaseLabel.setFont(labelFont);
    addAndMakeVisible(releaseLabel);

    fetCharacterLabel.setBorderSize(labelPadding);
    fetCharacterLabel.setColour(juce::Label::textColourId, labelCol);
    fetCharacterLabel.setFont(labelFont);

    speedSlider.setName("pwm_speed");
    speedSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    speedSlider.setRotaryParameters(juce::Slider::RotaryParameters{ -2.356f, 2.356f, true });
    speedSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xff00a67e)); // ombicTeal
    speedSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 52, 18);
    speedSlider.setColour(juce::Slider::textBoxTextColourId, textCol);
    speedSlider.setVelocityBasedMode(false);
    addAndMakeVisible(speedSlider);
    speedLabel.setText("SPEED", juce::dontSendNotification);
    speedLabel.setBorderSize(labelPadding);
    speedLabel.setColour(juce::Label::textColourId, labelCol);
    speedLabel.setFont(labelFont);
    addAndMakeVisible(speedLabel);

    addAndMakeVisible(grMeter);
    grReadoutLabel.setText("0.0 dB", juce::dontSendNotification);
    grReadoutLabel.setTooltip("Gain reduction in dB (bar above). Colour: teal <3 dB, yellow 3–6 dB, red >6 dB. Fast response and hold.");
    grReadoutLabel.setJustificationType(juce::Justification::centred);
    grReadoutLabel.setFont(OmbicLookAndFeel::getOmbicFontForPainting(16.0f, true));  // §6 GR readout 16px
    addAndMakeVisible(grReadoutLabel);
}

CompressorSection::~CompressorSection()
{
    setLookAndFeel(nullptr);
}

void CompressorSection::updateGrReadout()
{
    float grDb = proc.gainReductionDb.load();
    if (grDb > smoothedGrDb_)
        smoothedGrDb_ += kGrAttackCoeff * (grDb - smoothedGrDb_);
    else
        smoothedGrDb_ += kGrReleaseCoeff * (grDb - smoothedGrDb_);
    if (smoothedGrDb_ > grHoldDb_) { grHoldDb_ = smoothedGrDb_; grHoldTicks_ = kGrHoldTicks; }
    else { if (grHoldTicks_ > 0) --grHoldTicks_; if (grHoldTicks_ <= 0) grHoldDb_ += kGrReleaseCoeff * (smoothedGrDb_ - grHoldDb_); }
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

void CompressorSection::setModeControlsVisible(int mode)
{
    const bool isOpto = (mode == 0);
    const bool isFet = (mode == 1);
    const bool isPwm = (mode == 2);
    const bool isVca = (mode == 3);
    ratioSlider.setVisible(isFet || isPwm || isVca);
    ratioLabel.setVisible(isFet || isPwm || isVca);
    attackSlider.setVisible(isFet);
    attackLabel.setVisible(isFet);
    releaseSlider.setVisible(isFet);
    releaseLabel.setVisible(isFet);
    speedSlider.setVisible(isPwm);
    speedLabel.setVisible(isPwm);
    compressLimitToggle_.setVisible(isOpto);
    fetCharacterLabel.setVisible(isFet);
    fetCharacterCombo.setVisible(false);  // kept for attachment; UI uses pills in FET mode
    fetCharacterPillOff_.setVisible(isFet);
    fetCharacterPillRevA_.setVisible(isFet);
    fetCharacterPillLN_.setVisible(isFet);
    if (isFet)
        updateFetCharacterPillStates();
}

void CompressorSection::setShowGrMeter(bool show)
{
    showGrMeter_ = show;
    grMeter.setVisible(show);
    grReadoutLabel.setVisible(show);
}

void CompressorSection::updateCompressLimitButtonStates()
{
    compressLimitToggle_.setToggleState(compressLimitCombo.getSelectedId() == 2, juce::dontSendNotification);
}

void CompressorSection::setFetCharacterFromPill(int index)
{
    index = juce::jlimit(0, 2, index);
    if (auto* p = proc.getValueTreeState().getParameter(OmbicCompressorProcessor::paramFetCharacter))
        p->setValueNotifyingHost(static_cast<float>(index) / 2.0f);  // 3-choice: 0, 0.5, 1
    fetCharacterCombo.setSelectedId(index + 1, juce::dontSendNotification);
    updateFetCharacterPillStates();
}

void CompressorSection::updateFetCharacterPillStates()
{
    int index = 0;
    if (auto* raw = proc.getValueTreeState().getRawParameterValue(OmbicCompressorProcessor::paramFetCharacter))
        index = juce::jlimit(0, 2, static_cast<int>(raw->load() * 2.0f + 0.5f));
    fetCharacterPillOff_.setToggleState(index == 0, juce::dontSendNotification);
    fetCharacterPillRevA_.setToggleState(index == 1, juce::dontSendNotification);
    fetCharacterPillLN_.setToggleState(index == 2, juce::dontSendNotification);
}

bool CompressorSection::isInteracting() const
{
    return modeCombo.isMouseButtonDown() || compressLimitCombo.isMouseButtonDown()
        || compressLimitToggle_.isMouseButtonDown()
        || fetCharacterCombo.isMouseButtonDown()
        || fetCharacterPillOff_.isMouseButtonDown() || fetCharacterPillRevA_.isMouseButtonDown() || fetCharacterPillLN_.isMouseButtonDown()
        || thresholdSlider.isMouseButtonDown() || ratioSlider.isMouseButtonDown()
        || attackSlider.isMouseButtonDown() || releaseSlider.isMouseButtonDown()
        || speedSlider.isMouseButtonDown();
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
    const float headerH = getHeight() < 110 ? 22.0f : 36.0f;  // §6 header 36px
    auto headerRect = b.removeFromTop(headerH);
    g.setColour(OmbicLookAndFeel::ombicBlue());
    g.fillRoundedRectangle(headerRect.withBottom(headerRect.getY() + headerH), 16.0f);
    g.setColour(juce::Colours::white);
    g.setFont(OmbicLookAndFeel::getOmbicFontForPainting(13.0f, true));  // §7 Module headers 13px
    g.drawText("COMPRESSOR", static_cast<int>(headerRect.getX()) + 12, static_cast<int>(headerRect.getY() + (headerH - 13.0f) * 0.5f), 200, 14, juce::Justification::left);
}

void CompressorSection::resized()
{
    // Sync visibility from processor so layout always matches current mode (avoids stale Attack/Release/CHARACTER when switching to FET).
    if (auto* raw = proc.getValueTreeState().getRawParameterValue(OmbicCompressorProcessor::paramCompressorMode))
    {
        const int modeIndex = juce::jlimit(0, 3, static_cast<int>(raw->load() * 3.0f + 0.5f));
        setModeControlsVisible(modeIndex);
    }

    auto r = getLocalBounds();
    const bool compact = (r.getHeight() < 110);
    const int headerH = compact ? 22 : 36;  // §6 Module card header 36px
    r.removeFromTop(headerH);
    const int bodyPad = compact ? 8 : 14;  // §6 body padding 14px
    r.reduce(bodyPad, 0);
    r.removeFromBottom(bodyPad);

    const int labelH = compact ? 12 : 18;
    const int gap = compact ? 10 : 16;  // Spec §6: 16px gap between FET knobs
    modeCombo.setBounds(0, 0, 1, 1);
    int x = r.getX();
    bool pwmVisible = speedSlider.isVisible();
    bool vcaVisible = ratioSlider.isVisible() && !pwmVisible && !attackSlider.isVisible();
    bool fetVisible = ratioSlider.isVisible() && !pwmVisible && attackSlider.isVisible();
    bool optoVisible = !fetVisible && !pwmVisible && !vcaVisible;
    if (optoVisible)
    {
        const int switchW = compact ? 100 : 120;
        const int switchH = compact ? 22 : 26;
        compressLimitToggle_.setBounds(x, r.getY() + labelH, switchW, switchH);
        x += switchW + gap;
    }
    auto placeKnob = [&](juce::Slider& sl, juce::Label& lb, int size) {
        lb.setBounds(x, r.getY(), size + gap, labelH);
        sl.setBounds(x, r.getY() + labelH, size, size);
        x += size + gap;
    };
    const int knobSizeOpto = compact ? 52 : 80;   // §6 Opto (single) 80px diameter
    int knobSizeFet = compact ? 46 : 60;         // §6 FET (each) 60px diameter
    const int numKnobs = pwmVisible ? 3 : (fetVisible ? 4 : (vcaVisible ? 2 : 1));
    if (fetVisible || pwmVisible || vcaVisible)
    {
        int availableW = r.getWidth();
        int requiredForKnobs = numKnobs * (knobSizeFet + gap);
        if (fetVisible)
            requiredForKnobs += (3 * (compact ? 44 : 56) + 2 * (compact ? 6 : 8)) + gap;  // CHARACTER pills
        if (showGrMeter_)
        {
            const int grBlockW = gap + (compact ? 20 : 24) + 8;
            requiredForKnobs += grBlockW;
        }
        if (availableW < requiredForKnobs && knobSizeFet > 40)
            knobSizeFet = juce::jmax(40, (availableW - (showGrMeter_ ? (gap + (compact ? 20 : 24) + 8) : 0) - (fetVisible ? (3 * (compact ? 44 : 56) + 2 * (compact ? 6 : 8) + gap) : 0) - numKnobs * gap) / numKnobs);
    }
    if (pwmVisible)
    {
        placeKnob(thresholdSlider, thresholdLabel, knobSizeFet);
        placeKnob(ratioSlider, ratioLabel, knobSizeFet);
        placeKnob(speedSlider, speedLabel, knobSizeFet);
    }
    else if (fetVisible)
    {
        placeKnob(thresholdSlider, thresholdLabel, knobSizeFet);
        placeKnob(ratioSlider, ratioLabel, knobSizeFet);
        placeKnob(attackSlider, attackLabel, knobSizeFet);
        placeKnob(releaseSlider, releaseLabel, knobSizeFet);
        const int pillW = compact ? 44 : 56;
        const int pillH = compact ? 20 : 26;
        const int pillGap = compact ? 6 : 8;
        fetCharacterLabel.setBounds(x, r.getY(), (3 * pillW + 2 * pillGap) + gap, labelH);
        fetCharacterPillOff_.setBounds(x, r.getY() + labelH, pillW, pillH);
        fetCharacterPillRevA_.setBounds(x + pillW + pillGap, r.getY() + labelH, pillW, pillH);
        fetCharacterPillLN_.setBounds(x + 2 * (pillW + pillGap), r.getY() + labelH, pillW, pillH);
        fetCharacterCombo.setBounds(x, r.getY() + labelH, 1, 1);  // hidden; keep for attachment
        x += 3 * pillW + 2 * pillGap + gap;
    }
    else if (vcaVisible)
    {
        placeKnob(thresholdSlider, thresholdLabel, knobSizeFet);
        placeKnob(ratioSlider, ratioLabel, knobSizeFet);
    }
    else
    {
        int optoAreaW = r.getWidth() - (x - r.getX());
        int threshX = x + (optoAreaW - knobSizeOpto) / 2;
        thresholdLabel.setBounds(threshX, r.getY(), knobSizeOpto + gap, labelH);
        thresholdSlider.setBounds(threshX, r.getY() + labelH, knobSizeOpto, knobSizeOpto);
        x = threshX + knobSizeOpto + gap;
    }

    int knobH = (fetVisible || pwmVisible || vcaVisible) ? knobSizeFet : knobSizeOpto;
    if (showGrMeter_)
    {
        const int grMeterW = compact ? 20 : 24;
        const int grReadoutH = compact ? 14 : 18;
        grMeter.setBounds(x + gap, r.getY() + labelH, grMeterW, juce::jmax(20, knobH - grReadoutH));
        grReadoutLabel.setBounds(x + gap, r.getY() + labelH + juce::jmax(20, knobH - grReadoutH), grMeterW + 8, grReadoutH);
    }

    if (optoVisible)
        thresholdSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, compact ? 44 : 56, compact ? 16 : 24);
    else
        thresholdSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, compact ? 40 : 52, compact ? 14 : 18);
    float valueFontSize = (fetVisible || pwmVisible || vcaVisible) ? (compact ? 12.0f : 14.0f) : (compact ? 14.0f : 20.0f);
    for (int i = 0; i < thresholdSlider.getNumChildComponents(); ++i)
    {
        if (auto* lb = dynamic_cast<juce::Label*>(thresholdSlider.getChildComponent(i)))
        {
            lb->setFont(OmbicLookAndFeel::getOmbicFontForPainting(valueFontSize, true));
            break;
        }
    }
}
