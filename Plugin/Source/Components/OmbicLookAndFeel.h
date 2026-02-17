#pragma once

#include <JuceHeader.h>

/** OMBIC Sound design system — colours and component drawing (OMBIC_COMPRESSOR_JUCE_SPEC.md). */
class OmbicLookAndFeel : public juce::LookAndFeel_V4
{
public:
    OmbicLookAndFeel();

    /** Font for custom paint: Trash from embedded fonts/ or style/ when available. */
    static juce::Font getOmbicFontForPainting(float height, bool bold);

    // Brand accents
    static juce::Colour ombicRed()    { return juce::Colour(0xFFff001f); }
    static juce::Colour ombicBlue()   { return juce::Colour(0xFF076dc3); }
    static juce::Colour ombicTeal()   { return juce::Colour(0xFF338fab); }
    static juce::Colour ombicYellow() { return juce::Colour(0xFFffce00); }
    static juce::Colour ombicPurple() { return juce::Colour(0xFF5300aa); }
    static juce::Colour ombicPink()   { return juce::Colour(0xFFffb3c9); }

    // Dark plugin surfaces (spec §2)
    static juce::Colour pluginBg()         { return juce::Colour(0xFF0f1220); }
    static juce::Colour pluginSurface()   { return juce::Colour(0xFF181c2e); }
    static juce::Colour pluginRaised()    { return juce::Colour(0xFF222840); }
    static juce::Colour pluginText()      { return juce::Colour(0xFFe6e9ef); }
    static juce::Colour pluginMuted()     { return juce::Colour(0xFF6b7280); }
    static juce::Colour pluginBorder()    { return juce::Colour(0x33e6e9ef); }
    static juce::Colour pluginBorderStrong() { return juce::Colour(0x59e6e9ef); }
    static juce::Colour pluginShadow()    { return juce::Colour(0x80000000); }
    static juce::Colour knobTrack()      { return juce::Colour(0x14ffffff); }

    // Legacy aliases (map to spec)
    static juce::Colour ink()         { return pluginBg(); }
    static juce::Colour muted()      { return pluginMuted(); }
    static juce::Colour line()       { return pluginText(); }
    static juce::Colour bg()         { return juce::Colour(0xffffffff); }

    void drawButtonBackground(juce::Graphics& g, juce::Button& button,
                              const juce::Colour&, bool isMouseOver, bool isButtonDown) override;
    void drawButtonText(juce::Graphics& g, juce::TextButton& button,
                       bool isMouseOver, bool isButtonDown) override;
    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                         bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;
    void drawComboBox(juce::Graphics& g, int width, int height, bool isDown, int buttonX,
                     int buttonY, int buttonW, int buttonH, juce::ComboBox& box) override;
    void positionComboBoxText(juce::ComboBox& box, juce::Label& label) override;
    void drawLabel(juce::Graphics& g, juce::Label& label) override;
    void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPos, float minSliderPos, float maxSliderPos,
                          juce::Slider::SliderStyle style, juce::Slider& slider) override;
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                         float sliderPos, float startAngle, float endAngle,
                         juce::Slider& slider) override;
    void drawGroupComponentOutline(juce::Graphics& g, int width, int height,
                                  const juce::String& text, const juce::Justification& position,
                                  juce::GroupComponent& group) override;

    juce::Font getLabelFont(juce::Label&) override;
    juce::Font getComboBoxFont(juce::ComboBox&) override;
    int getSliderThumbRadius(juce::Slider&) override { return 8; }

private:
    static juce::File findStyleFolder();
    /** Directory containing Trash-Bold.ttf / Trash-Regular.ttf (e.g. Plugin/fonts/, OMBIC_FONT_PATH). */
    static juce::File findFontFolder();
    static juce::Typeface::Ptr loadTrashTypeface(bool bold);

    static juce::Typeface::Ptr s_trashBold;
    static juce::Typeface::Ptr s_trashRegular;

    juce::Font getOmbicFont(float height, float weight = 700.0f);
};
