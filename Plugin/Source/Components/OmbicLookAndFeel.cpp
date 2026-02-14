#include "OmbicLookAndFeel.h"

juce::Typeface::Ptr OmbicLookAndFeel::s_trashBold;
juce::Typeface::Ptr OmbicLookAndFeel::s_trashRegular;

OmbicLookAndFeel::OmbicLookAndFeel()
{
    setDefaultSansSerifTypefaceName("Segoe UI");
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

juce::Typeface::Ptr OmbicLookAndFeel::loadTrashTypeface(bool bold)
{
    juce::Typeface::Ptr& cached = bold ? s_trashBold : s_trashRegular;
    if (cached != nullptr)
        return cached;
    juce::File styleDir = findStyleFolder();
    if (!styleDir.exists())
        return {};
    juce::File fontFile = styleDir.getChildFile(bold ? "Trash-Bold.ttf" : "Trash-Regular.ttf");
    if (!fontFile.existsAsFile())
        fontFile = styleDir.getChildFile("Trash-Regular.ttf");
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
    const int borderW = 3;
    const int shadowOffset = 4;
    const bool selected = button.getToggleState();

    juce::Colour fill = selected ? ombicYellow() : ombicBlue();
    if (button.getName().contains("danger") || button.getName().contains("Delete"))
        fill = ombicRed();
    if (!button.isEnabled())
        fill = fill.withAlpha(0.5f);

    // Hard shadow (bottom-right)
    if (button.isEnabled() && !isButtonDown)
        g.setColour(ink());
    else
        g.setColour(juce::Colours::transparentBlack);
    g.fillRoundedRectangle(bounds.reduced(borderW).translated(static_cast<float>(shadowOffset), static_cast<float>(shadowOffset)), 12.0f);

    // Border + fill
    g.setColour(ink());
    g.fillRoundedRectangle(bounds.reduced(borderW), 12.0f);
    g.setColour(fill);
    g.fillRoundedRectangle(bounds.reduced(borderW + 1), 11.0f);

    if (isMouseOver && button.isEnabled())
    {
        g.setColour(ombicBlue().withAlpha(0.3f));
        g.drawRoundedRectangle(bounds.reduced(borderW), 12.0f, 2.0f);
    }
}

void OmbicLookAndFeel::drawButtonText(juce::Graphics& g, juce::TextButton& button,
                                      bool, bool)
{
    juce::Colour c = button.getToggleState() ? ink() : bg();
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

void OmbicLookAndFeel::drawLabel(juce::Graphics& g, juce::Label& label)
{
    g.setColour(label.isEnabled() ? ink() : muted());
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
                                        float sliderPos, float startAngle, float endAngle,
                                        juce::Slider& /*slider*/)
{
    auto bounds = juce::Rectangle<float>(static_cast<float>(x), static_cast<float>(y),
                                         static_cast<float>(width), static_cast<float>(height))
                      .reduced(4);
    g.setColour(line());
    g.fillEllipse(bounds);
    g.setColour(ombicBlue());
    float angle = startAngle + sliderPos * (endAngle - startAngle);
    juce::Path p;
    p.addPieSegment(bounds, startAngle, angle, 0.4f);
    g.fillPath(p);
    g.setColour(ink());
    g.drawEllipse(bounds, 3.0f);
    float cx = bounds.getCentreX();
    float cy = bounds.getCentreY();
    float r = bounds.getWidth() * 0.35f;
    float tx = cx + r * std::cos(angle);
    float ty = cy - r * std::sin(angle);
    g.drawLine(cx, cy, tx, ty, 3.0f);
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
    return getOmbicFont(14.0f, 700.0f);
}

juce::Font OmbicLookAndFeel::getComboBoxFont(juce::ComboBox&)
{
    return getOmbicFont(14.0f, 700.0f);
}
