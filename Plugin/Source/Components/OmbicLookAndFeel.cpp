#include "OmbicLookAndFeel.h"
#include "OmbicAssets.h"

juce::Typeface::Ptr OmbicLookAndFeel::s_trashBold;
juce::Typeface::Ptr OmbicLookAndFeel::s_trashRegular;

OmbicLookAndFeel::OmbicLookAndFeel()
{
    setDefaultSansSerifTypefaceName("Segoe UI");
    setColour(juce::Label::textColourId, pluginText());
    setColour(juce::Slider::textBoxTextColourId, pluginText());
    setColour(juce::Slider::textBoxBackgroundColourId, pluginRaised());
    setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    setColour(juce::Slider::rotarySliderFillColourId, ombicBlue());
}

juce::File OmbicLookAndFeel::findStyleFolder()
{
    juce::String dataPath = juce::SystemStats::getEnvironmentVariable("OMBIC_COMPRESSOR_DATA_PATH", {});
    if (dataPath.isNotEmpty())
    {
        juce::File root = dataPath.startsWithChar('/') ? juce::File(dataPath) : juce::File::getCurrentWorkingDirectory().getChildFile(dataPath);
        juce::File styleDir = root.getChildFile("style");
        if (styleDir.exists() && styleDir.getChildFile("Trash-Bold.ttf").existsAsFile())
            return styleDir;
    }
    juce::File cwdStyle = juce::File::getCurrentWorkingDirectory().getChildFile("style");
    if (cwdStyle.exists() && cwdStyle.getChildFile("Trash-Bold.ttf").existsAsFile())
        return cwdStyle;
    juce::String fontPath = juce::SystemStats::getEnvironmentVariable("OMBIC_FONT_PATH", {});
    if (fontPath.isNotEmpty())
    {
        juce::File f = fontPath.startsWithChar('/') ? juce::File(fontPath) : juce::File::getCurrentWorkingDirectory().getChildFile(fontPath);
        if (f.exists() && f.getChildFile("Trash-Bold.ttf").existsAsFile())
            return f;
        if (f.getChildFile("style").getChildFile("Trash-Bold.ttf").existsAsFile())
            return f.getChildFile("style");
    }
    return {};
}

juce::File OmbicLookAndFeel::findFontFolder()
{
    juce::String dataPath = juce::SystemStats::getEnvironmentVariable("OMBIC_COMPRESSOR_DATA_PATH", {});
    if (dataPath.isNotEmpty())
    {
        juce::File root = dataPath.startsWithChar('/') ? juce::File(dataPath) : juce::File::getCurrentWorkingDirectory().getChildFile(dataPath);
        juce::File fontsDir = root.getChildFile("fonts");
        if (fontsDir.exists() && fontsDir.getChildFile("Trash-Bold.ttf").existsAsFile())
            return fontsDir;
        fontsDir = root.getChildFile("Plugin").getChildFile("fonts");
        if (fontsDir.exists() && fontsDir.getChildFile("Trash-Bold.ttf").existsAsFile())
            return fontsDir;
    }
    juce::File cwd = juce::File::getCurrentWorkingDirectory();
    juce::File fontsDir = cwd.getChildFile("fonts");
    if (fontsDir.exists() && fontsDir.getChildFile("Trash-Bold.ttf").existsAsFile())
        return fontsDir;
    fontsDir = cwd.getChildFile("Plugin").getChildFile("fonts");
    if (fontsDir.exists() && fontsDir.getChildFile("Trash-Bold.ttf").existsAsFile())
        return fontsDir;
    juce::String fontPath = juce::SystemStats::getEnvironmentVariable("OMBIC_FONT_PATH", {});
    if (fontPath.isNotEmpty())
    {
        juce::File f = fontPath.startsWithChar('/') ? juce::File(fontPath) : cwd.getChildFile(fontPath);
        if (f.exists() && (f.getChildFile("Trash-Bold.ttf").existsAsFile() || f.getChildFile("Trash-Regular.ttf").existsAsFile()))
            return f;
    }
    return {};
}

juce::Typeface::Ptr OmbicLookAndFeel::loadTrashTypeface(bool bold)
{
    juce::Typeface::Ptr& cached = bold ? s_trashBold : s_trashRegular;
    if (cached != nullptr)
        return cached;

    // 1) Try embedded fonts (only present if Plugin/fonts/Trash-*.ttf existed at build time)
    int size = 0;
    const char* data = OmbicAssets::getNamedResource(bold ? "Trash_Bold_ttf" : "Trash_Regular_ttf", size);
    if (data != nullptr && size > 0)
    {
        cached = juce::Typeface::createSystemTypefaceFor(data, static_cast<size_t>(size));
        if (cached != nullptr)
            return cached;
    }

    // 2) Load from fonts/ (Plugin/fonts/, OMBIC_FONT_PATH, cwd/fonts) then style/
    juce::File fontDir = findFontFolder();
    if (!fontDir.exists())
        fontDir = findStyleFolder();
    if (!fontDir.exists())
        return {};
    juce::File fontFile = fontDir.getChildFile(bold ? "Trash-Bold.ttf" : "Trash-Regular.ttf");
    if (!fontFile.existsAsFile())
        fontFile = fontDir.getChildFile("Trash-Regular.ttf");
    if (!fontFile.existsAsFile())
        return {};
    juce::MemoryBlock mb;
    if (!fontFile.loadFileAsData(mb))
        return {};
    cached = juce::Typeface::createSystemTypefaceFor(mb.getData(), mb.getSize());
    return cached;
}

juce::Font OmbicLookAndFeel::getOmbicFontForPainting(float height, bool bold)
{
    juce::Typeface::Ptr tf = loadTrashTypeface(bold);
    if (tf != nullptr)
        return juce::Font(juce::FontOptions().withTypeface(tf).withHeight(height));
    return juce::Font(juce::FontOptions(height, bold ? juce::Font::bold : juce::Font::plain));
}

juce::Font OmbicLookAndFeel::getOmbicFont(float height, float weight)
{
    bool bold = weight >= 700.0f;
    juce::Typeface::Ptr tf = loadTrashTypeface(bold);
    if (tf != nullptr)
        return juce::Font(juce::FontOptions().withTypeface(tf).withHeight(height));
    return juce::Font(juce::FontOptions(juce::Font::getDefaultSansSerifFontName(), height, juce::Font::bold));
}

void OmbicLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& button,
                                            const juce::Colour&, bool isMouseOver, bool isButtonDown)
{
    auto bounds = button.getLocalBounds().toFloat();
    const bool isPill = button.getName().contains("Pill");
    const bool isMainVuToggle = button.getName().contains("main_vu_");

    if (isMainVuToggle)
    {
        const bool selected = button.getToggleState();
        const float radius = 6.0f;
        if (selected)
        {
            g.setColour(ombicBlue());
            g.fillRoundedRectangle(bounds, radius);
            g.setColour(ombicBlue());
            g.drawRoundedRectangle(bounds.reduced(1.0f), radius - 1.0f, 2.0f);
        }
        else
        {
            g.setColour(juce::Colours::transparentBlack);
            g.fillRoundedRectangle(bounds, radius);
            g.setColour(isMouseOver ? ombicBlue() : pluginBorderStrong());
            g.drawRoundedRectangle(bounds.reduced(1.0f), radius - 1.0f, 2.0f);
        }
        return;
    }

    if (isPill)
    {
        // Spec §6: mode pills — active = ombicBlue fill; inactive = transparent, pluginBorderStrong
        const bool selected = button.getToggleState();
        const float radius = bounds.getHeight() * 0.5f;
        if (selected)
        {
            g.setColour(ombicBlue());
            g.fillRoundedRectangle(bounds, radius);
            g.setColour(ombicBlue());
            g.drawRoundedRectangle(bounds.reduced(1.0f), radius - 1.0f, 2.0f);
        }
        else
        {
            g.setColour(juce::Colours::transparentBlack);
            g.fillRoundedRectangle(bounds, radius);
            g.setColour(isMouseOver ? ombicBlue() : pluginBorderStrong());
            g.drawRoundedRectangle(bounds.reduced(1.0f), radius - 1.0f, 2.0f);
        }
        return;
    }

    const bool isScListen = button.getName().contains("sc_listen") || button.getName().contains("LISTEN");
    if (isScListen)
    {
        const bool selected = button.getToggleState();
        const float radius = 6.0f;
        if (selected)
        {
            float pulse = 0.85f + 0.15f * 0.5f * (1.0f + std::sin(static_cast<float>(juce::Time::getMillisecondCounter()) * 0.004f));
            g.setColour(ombicTeal().withAlpha(pulse));
            g.fillRoundedRectangle(bounds, radius);
            g.setColour(ombicTeal());
            g.drawRoundedRectangle(bounds.reduced(0.75f), radius - 0.75f, 1.5f);
        }
        else
        {
            g.setColour(pluginBg());
            g.fillRoundedRectangle(bounds, radius);
            g.setColour(isMouseOver ? ombicTeal() : pluginBorder());
            g.drawRoundedRectangle(bounds.reduced(0.75f), radius - 0.75f, 1.5f);
        }
        return;
    }

    const int borderW = 3;
    const int shadowOffset = 4;
    const bool selected = button.getToggleState();
    juce::Colour fill = selected ? ombicBlue() : ombicBlue();
    if (button.getName().contains("danger") || button.getName().contains("Delete"))
        fill = ombicRed();
    if (!button.isEnabled())
        fill = fill.withAlpha(0.5f);

    if (button.isEnabled() && !isButtonDown)
        g.setColour(pluginShadow());
    else
        g.setColour(juce::Colours::transparentBlack);
    g.fillRoundedRectangle(bounds.reduced(borderW).translated(static_cast<float>(shadowOffset), static_cast<float>(shadowOffset)), 12.0f);
    g.setColour(pluginBg());
    g.fillRoundedRectangle(bounds.reduced(borderW), 12.0f);
    g.setColour(fill);
    g.fillRoundedRectangle(bounds.reduced(borderW + 1), 11.0f);
    if (isMouseOver && button.isEnabled())
        g.setColour(ombicBlue().withAlpha(0.3f));
    g.drawRoundedRectangle(bounds.reduced(borderW), 12.0f, 2.0f);
}

void OmbicLookAndFeel::drawButtonText(juce::Graphics& g, juce::TextButton& button,
                                      bool isMouseOver, bool)
{
    if (button.getName().contains("Pill"))
    {
        g.setColour(button.getToggleState() ? juce::Colours::white : (isMouseOver ? pluginText() : pluginMuted()));
        g.setFont(getOmbicFont(11.0f, 900.0f));
        g.drawText(button.getButtonText(), button.getLocalBounds(), juce::Justification::centred);
        return;
    }
    if (button.getName().contains("sc_listen") || button.getName().contains("LISTEN"))
    {
        g.setColour(button.getToggleState() ? juce::Colours::white : (isMouseOver ? pluginText() : pluginMuted()));
        g.setFont(getOmbicFont(8.0f, 900.0f));
        g.drawText(button.getButtonText(), button.getLocalBounds(), juce::Justification::centred);
        return;
    }
    juce::Colour c = button.getToggleState() ? juce::Colours::white : pluginText();
    if (!button.isEnabled())
        c = c.withAlpha(0.6f);
    g.setColour(c);
    g.setFont(getOmbicFont(14.0f, 900.0f));
    g.drawText(button.getButtonText(), button.getLocalBounds(), juce::Justification::centred);
}

void OmbicLookAndFeel::drawComboBox(juce::Graphics& g, int width, int height, bool,
                                    int, int, int, int, juce::ComboBox& box)
{
    auto bounds = juce::Rectangle<float>(0, 0, static_cast<float>(width), static_cast<float>(height));
    g.setColour(ink());
    g.fillRoundedRectangle(bounds.reduced(1), 12.0f);
    g.setColour(box.isEnabled() ? bg() : line());
    g.fillRoundedRectangle(bounds.reduced(3), 10.0f);
    g.setColour(ink());
    g.drawRoundedRectangle(bounds.reduced(1.5f), 12.0f, 3.0f);
}

void OmbicLookAndFeel::positionComboBoxText(juce::ComboBox& box, juce::Label& label)
{
    const int leftPad = 10;
    const int rightPad = 28; // space for dropdown arrow
    const int vertPad = 2;
    label.setBounds(leftPad, vertPad,
                    juce::jmax(0, box.getWidth() - leftPad - rightPad),
                    box.getHeight() - vertPad * 2);
    label.setFont(getComboBoxFont(box));
}

void OmbicLookAndFeel::drawLabel(juce::Graphics& g, juce::Label& label)
{
    g.setColour(label.findColour(juce::Label::textColourId));
    g.setFont(getOmbicFont(static_cast<float>(label.getHeight()) * 0.6f, 700.0f));
    g.drawText(label.getText(), label.getLocalBounds(), label.getJustificationType());
}

void OmbicLookAndFeel::drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                                        float sliderPos, float, float,
                                        juce::Slider::SliderStyle style, juce::Slider& slider)
{
    if (style == juce::Slider::LinearHorizontal || style == juce::Slider::LinearVertical)
    {
        auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat();
        float trackThickness = 6.0f;
        bool isMouseOver = slider.isMouseOverOrDragging();
        if (style == juce::Slider::LinearVertical)
        {
            float trackLeft = bounds.getCentreX() - trackThickness * 0.5f;
            g.setColour(line());
            g.fillRoundedRectangle(trackLeft, bounds.getY(), trackThickness, bounds.getHeight(), 3.0f);
            g.setColour(ombicBlue());
            float fillH = sliderPos * bounds.getHeight();
            if (fillH > 0)
                g.fillRoundedRectangle(trackLeft, bounds.getBottom() - fillH, trackThickness, fillH, 3.0f);
            g.setColour(ink());
            g.drawRoundedRectangle(trackLeft, bounds.getY(), trackThickness, bounds.getHeight(), 3.0f, 2.0f);
            float thumbY = bounds.getY() + (1.0f - sliderPos) * bounds.getHeight();
            g.setColour(isMouseOver ? ombicBlue() : ink());
            g.fillEllipse(bounds.getCentreX() - 8, thumbY - 8, 16, 16);
            g.setColour(ink());
            g.drawEllipse(bounds.getCentreX() - 8, thumbY - 8, 16, 16, 2.0f);
        }
        else
        {
            float trackY = bounds.getCentreY() - trackThickness * 0.5f;
            g.setColour(line());
            g.fillRoundedRectangle(bounds.getX(), trackY, bounds.getWidth(), trackThickness, 3.0f);
            g.setColour(ombicBlue());
            g.fillRoundedRectangle(bounds.getX(), trackY, sliderPos * bounds.getWidth(), trackThickness, 3.0f);
            g.setColour(ink());
            g.drawRoundedRectangle(bounds.getX(), trackY, bounds.getWidth(), trackThickness, 3.0f, 2.0f);
            g.setColour(isMouseOver ? ombicBlue() : ink());
            g.fillEllipse(sliderPos * bounds.getWidth() + bounds.getX() - 8, bounds.getCentreY() - 8, 16, 16);
            g.setColour(ink());
            g.drawEllipse(sliderPos * bounds.getWidth() + bounds.getX() - 8, bounds.getCentreY() - 8, 16, 16, 2.0f);
        }
    }
    else
        LookAndFeel_V4::drawLinearSlider(g, x, y, width, height, sliderPos, 0, 1, style, slider);
}

void OmbicLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                        float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                                        juce::Slider& slider)
{
    // GUIDE §2: Use slider's rotary angles (setRotaryParameters(-2.356f, 2.356f, true) = 240° sweep).
    // JUCE: 0 = 12 o'clock, positive = clockwise.
    auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat();
    auto centre = bounds.getCentre();
    auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f - 4.0f;
    auto lineW = (radius > 30.0f) ? 5.0f : 4.0f;
    auto arcRadius = radius - lineW * 0.5f;

    auto toAngle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

    juce::Colour accent = slider.findColour(juce::Slider::rotarySliderFillColourId);

    juce::Path trackArc;
    trackArc.addCentredArc(centre.x, centre.y, arcRadius, arcRadius,
                           0.0f, rotaryStartAngle, rotaryEndAngle, true);
    g.setColour(knobTrack());
    g.strokePath(trackArc, juce::PathStrokeType(lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    if (sliderPos > 0.005f)
    {
        juce::Path glowArc;
        glowArc.addCentredArc(centre.x, centre.y, arcRadius, arcRadius,
                              0.0f, rotaryStartAngle, toAngle, true);
        g.setColour(accent.withAlpha(0.15f));
        g.strokePath(glowArc, juce::PathStrokeType(lineW + 6.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        juce::Path valueArc;
        valueArc.addCentredArc(centre.x, centre.y, arcRadius, arcRadius,
                              0.0f, rotaryStartAngle, toAngle, true);
        g.setColour(accent);
        g.strokePath(valueArc, juce::PathStrokeType(lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    auto innerRadius = arcRadius - lineW - 2.0f;
    if (innerRadius > 2.0f)
    {
        g.setColour(pluginRaised());
        g.fillEllipse(centre.x - innerRadius, centre.y - innerRadius, innerRadius * 2.0f, innerRadius * 2.0f);
        g.setColour(juce::Colour(0x0fffffff));
        g.drawEllipse(centre.x - innerRadius, centre.y - innerRadius, innerRadius * 2.0f, innerRadius * 2.0f, 1.0f);
    }

    // GUIDE §2: pointer at toAngle — sin for x, -cos for y (0 = top, clockwise)
    auto pointerRadius = (radius > 30.0f) ? 3.0f : 2.5f;
    auto pointerDistance = innerRadius - (radius > 30.0f ? 8.0f : 5.0f);
    juce::Point<float> pointerPos(
        centre.x + pointerDistance * std::sin(toAngle),
        centre.y - pointerDistance * std::cos(toAngle));
    g.setColour(accent);
    g.fillEllipse(pointerPos.x - pointerRadius, pointerPos.y - pointerRadius,
                  pointerRadius * 2.0f, pointerRadius * 2.0f);
}

void OmbicLookAndFeel::drawGroupComponentOutline(juce::Graphics& g, int width, int height,
                                                 const juce::String& text, const juce::Justification& position,
                                                 juce::GroupComponent&)
{
    g.setColour(ink());
    g.drawRoundedRectangle(2, 2, static_cast<float>(width - 4), static_cast<float>(height - 4), 20.0f, 3.0f);
    if (text.isNotEmpty())
    {
        g.setFont(getOmbicFont(14.0f, 900.0f));
        g.setColour(ink());
        g.drawText(text, 12, 0, width - 24, 18, position);
    }
}

juce::Font OmbicLookAndFeel::getLabelFont(juce::Label&)
{
    // Spec §11: param values 14px weight 900 (slider text box uses this)
    return getOmbicFont(14.0f, 900.0f);
}

juce::Font OmbicLookAndFeel::getComboBoxFont(juce::ComboBox&)
{
    return getOmbicFont(14.0f, 700.0f);
}
