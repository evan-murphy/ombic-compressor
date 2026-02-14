#pragma once

#include <JuceHeader.h>

/** OMBIC Sound design system — colours and component drawing. */
class OmbicLookAndFeel : public juce::LookAndFeel_V4
{
public:
    OmbicLookAndFeel();

    /** Font for custom paint: uses Trash from style/ when available (Trash-Bold.ttf, Trash-Regular.ttf). */
    static juce::Font getOmbicFontForPainting(float height, bool bold);

    static juce::Colour ombicRed()    { return juce::Colour(0xffff001f); }
    static juce::Colour ombicBlue()   { return juce::Colour(0xff076dc3); }
    static juce::Colour ombicTeal()   { return juce::Colour(0xff338fab); }
    static juce::Colour ombicYellow() { return juce::Colour(0xffffce00); }
    static juce::Colour ombicPurple() { return juce::Colour(0xff5300aa); }
    static juce::Colour ombicPink()   { return juce::Colour(0xffffb3c9); }
    static juce::Colour ink()         { return juce::Colour(0xff0f1220); }
    static juce::Colour muted()       { return juce::Colour(0xff586070); }
    static juce::Colour line()        { return juce::Colour(0xffe6e9ef); }
    static juce::Colour bg()         { return juce::Colour(0xffffffff); }

    void drawButtonBackground(juce::Graphics& g, juce::Button& button,
                              const juce::Colour&, bool isMouseOver, bool isButtonDown) override;
    void drawButtonText(juce::Graphics& g, juce::TextButton& button,
                       bool isMouseOver, bool isButtonDown) override;
    void drawComboBox(juce::Graphics& g, int width, int height, bool isDown, int buttonX,
                     int buttonY, int buttonW, int buttonH, juce::ComboBox& box) override;
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
    static juce::Typeface::Ptr loadTrashTypeface(bool bold);

    static juce::Typeface::Ptr s_trashBold;
    static juce::Typeface::Ptr s_trashRegular;

    juce::Font getOmbicFont(float height, float weight = 700.0f);
};
