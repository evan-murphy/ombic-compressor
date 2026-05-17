# Ombic Compressor — JUCE Implementation Guide

> **For use with:** Cursor / AI code editors implementing the JUCE plugin editor.
> **Read alongside:** `OMBIC_COMPRESSOR_JUCE_SPEC.md` (design spec with all colors, sizes, values).
> **Reference mockup:** `ombic-compressor.html` (open in browser to see target visually).
> **This document:** Maps every visual element to exact JUCE API calls with correct conventions.

---

## Architecture Overview

```
PluginEditor (AudioProcessorEditor)
├── OmbicLookAndFeel (LookAndFeel_V4 subclass)
│   ├── Trash font loading (Regular + Bold as separate typefaces)
│   └── drawRotarySlider() override (arc knobs with glow)
├── HeaderBar (Component)
├── ModeSwitcher (Component with TextButtons)
├── CompressorModule (Component)
│   ├── TransferCurveDisplay (Component)
│   └── 1 or 4 ArcKnob sliders (mode-dependent)
├── SaturationModule (Component)
│   ├── SaturationScope (Component + Timer)
│   └── 4 ArcKnob sliders
├── OutputModule (Component)
│   ├── MeterBar x2 (Component + Timer)
│   ├── 1 ArcKnob slider
│   └── GR readout (Label)
└── FooterBar (Component)
```

---

## 1. Font Loading — OmbicLookAndFeel

**Source:** JUCE forum "Modern Custom Font Guide" pattern.
**Gotcha:** Regular and Bold are separate .ttf files = separate typefaces in JUCE. Variable font weights are NOT supported — load each weight file explicitly.

### Setup

Add all four Trash .ttf files to Projucer as binary resources (or `juce_add_binary_resources()` in CMake). This generates `BinaryData::TrashRegular_ttf`, `BinaryData::TrashBold_ttf`, etc.

```cpp
// In OmbicLookAndFeel constructor:
trashRegular = juce::Typeface::createSystemTypefaceFor(
    BinaryData::TrashRegular_ttf, BinaryData::TrashRegular_ttfSize);
trashBold = juce::Typeface::createSystemTypefaceFor(
    BinaryData::TrashBold_ttf, BinaryData::TrashBold_ttfSize);

// Set as default so all child components inherit it:
setDefaultSansSerifTypeface(trashBold); // Bold is the minimum weight per brand guide
```

### Using specific weights

```cpp
// For 900-weight headers:
juce::Font headerFont(trashBold);
headerFont.setHeight(13.0f);

// For 700-weight labels:
juce::Font labelFont(trashRegular);
labelFont.setHeight(9.0f);
```

### High-DPI Gotcha
Font heights in JUCE are in logical pixels. On Retina/HiDPI, JUCE scales automatically via the `AffineTransform` on the `Graphics` context — do NOT multiply font size by scale factor manually. Just use the design spec pixel values directly.

### Cross-Platform Gotcha
FreeType (Linux) vs CoreText (macOS) vs DirectWrite (Windows) render slightly differently. Test on all targets. Trash is a bold geometric face so differences should be minimal, but check letter-spacing and descender alignment.

---

## 2. Rotary Knobs — drawRotarySlider() Override

This is the most critical custom drawing. The HTML mockup uses SVG arcs; JUCE uses `Path::addArc()` / `Graphics::drawArc()` with different angle conventions.

### JUCE Angle Convention (CRITICAL)

```
JUCE radians:
- 0 = 12 o'clock (top center)
- Positive = clockwise
- π = 6 o'clock (bottom)
- 2π = back to 12 o'clock

HTML/SVG mockup used:
- 150° start (about 7 o'clock) to 390° end (about 1 o'clock)
- That's a 240° sweep

JUCE equivalent:
- rotaryStartAngle = -2.356f  (about -3π/4, i.e. 7 o'clock)
- rotaryEndAngle   =  2.356f  (about  3π/4, i.e. 5 o'clock)
- Total sweep = 4.712 radians = 270°
  (Note: the mockup used 240° but 270° is more standard; adjust to taste)
```

### Set on each slider:

```cpp
slider.setRotaryParameters(
    juce::Slider::RotaryParameters{
        -2.356f,   // startAngleRadians (7 o'clock)
         2.356f,   // endAngleRadians (5 o'clock)
         true      // stopAtEnd
    });
slider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
```

### drawRotarySlider() Implementation

```cpp
void OmbicLookAndFeel::drawRotarySlider(
    juce::Graphics& g, int x, int y, int width, int height,
    float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
    juce::Slider& slider)
{
    // --- Geometry ---
    auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat();
    auto centre = bounds.getCentre();
    auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f - 4.0f;
    auto lineW = (radius > 30.0f) ? 5.0f : 4.0f;  // Thicker stroke for large knobs
    auto arcRadius = radius - lineW * 0.5f;

    // Current angle from slider position
    auto toAngle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

    // --- Determine accent color from slider property ---
    // Store the accent color as a Slider property or use ColourIds
    auto accentColour = slider.findColour(juce::Slider::rotarySliderFillColourId);

    // --- 1. Track arc (inactive, full sweep) ---
    juce::Path trackArc;
    trackArc.addCentredArc(centre.x, centre.y, arcRadius, arcRadius,
                           0.0f, rotaryStartAngle, rotaryEndAngle, true);
    g.setColour(juce::Colour(0x14ffffff));  // 8% white
    juce::PathStrokeType trackStroke(lineW, juce::PathStrokeType::curved,
                                     juce::PathStrokeType::rounded);
    g.strokePath(trackArc, trackStroke);

    // --- 2. Glow arc (behind value arc, wider + semi-transparent) ---
    if (sliderPos > 0.005f)
    {
        juce::Path glowArc;
        glowArc.addCentredArc(centre.x, centre.y, arcRadius, arcRadius,
                              0.0f, rotaryStartAngle, toAngle, true);
        g.setColour(accentColour.withAlpha(0.15f));
        juce::PathStrokeType glowStroke(lineW + 6.0f, juce::PathStrokeType::curved,
                                        juce::PathStrokeType::rounded);
        g.strokePath(glowArc, glowStroke);
    }

    // --- 3. Value arc (colored sweep from start to current) ---
    if (sliderPos > 0.005f)
    {
        juce::Path valueArc;
        valueArc.addCentredArc(centre.x, centre.y, arcRadius, arcRadius,
                               0.0f, rotaryStartAngle, toAngle, true);
        g.setColour(accentColour);
        juce::PathStrokeType valueStroke(lineW, juce::PathStrokeType::curved,
                                        juce::PathStrokeType::rounded);
        g.strokePath(valueArc, valueStroke);
    }

    // --- 4. Center circle ---
    auto innerRadius = arcRadius - lineW - 2.0f;
    g.setColour(juce::Colour(0xFF222840));  // pluginRaised
    g.fillEllipse(centre.x - innerRadius, centre.y - innerRadius,
                  innerRadius * 2.0f, innerRadius * 2.0f);
    g.setColour(juce::Colour(0x0fffffff));  // subtle border
    g.drawEllipse(centre.x - innerRadius, centre.y - innerRadius,
                  innerRadius * 2.0f, innerRadius * 2.0f, 1.0f);

    // --- 5. Pointer dot ---
    auto pointerRadius = (radius > 30.0f) ? 3.0f : 2.5f;
    auto pointerDistance = innerRadius - (radius > 30.0f ? 8.0f : 5.0f);
    juce::Point<float> pointerPos(
        centre.x + pointerDistance * std::sin(toAngle),
        centre.y - pointerDistance * std::cos(toAngle)
        // NOTE: sin for x, -cos for y because JUCE 0 = top, clockwise
    );
    g.setColour(accentColour);
    g.fillEllipse(pointerPos.x - pointerRadius, pointerPos.y - pointerRadius,
                  pointerRadius * 2.0f, pointerRadius * 2.0f);
}
```

### Setting accent colors per module

```cpp
// In CompressorModule setup:
thresholdSlider.setColour(Slider::rotarySliderFillColourId, Colour(0xFF076dc3)); // ombicBlue

// In SaturationModule:
driveSlider.setColour(Slider::rotarySliderFillColourId, Colour(0xFFe85590)); // pink

// In OutputModule:
outputSlider.setColour(Slider::rotarySliderFillColourId, Colour(0xFF5300aa)); // ombicPurple
```

---

## 3. Transfer Curve Display

Simple custom `Component` with `paint()` override. No animation needed — redraws when parameters change.

```cpp
void TransferCurveDisplay::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    auto w = bounds.getWidth();
    auto h = bounds.getHeight();

    // Background
    g.setColour(juce::Colour(0xFF0f1220));  // pluginBg
    g.fillRoundedRectangle(bounds, 10.0f);

    // Border
    g.setColour(juce::Colour(0x33e6e9ef));  // pluginBorder
    g.drawRoundedRectangle(bounds, 10.0f, 1.0f);

    // Grid (6x6)
    g.setColour(juce::Colour(0x0affffff));  // 4% white
    for (int i = 1; i < 6; ++i)
    {
        float xLine = (w / 6.0f) * i;
        float yLine = (h / 6.0f) * i;
        g.drawLine(xLine, 0, xLine, h, 1.0f);
        g.drawLine(0, yLine, w, yLine, 1.0f);
    }

    // Unity line (dashed diagonal)
    g.setColour(juce::Colour(0x14ffffff));
    float dashLengths[] = { 3.0f, 3.0f };
    juce::Path unityLine;
    unityLine.startNewSubPath(0, h);
    unityLine.lineTo(w, 0);
    juce::PathStrokeType dashed(1.0f);
    dashed.createDashedStroke(unityLine, unityLine, dashLengths, 2);
    g.strokePath(unityLine, dashed);

    // Compression curve
    auto threshold = thresholdParam->load();  // 0-100 normalized to 0-1
    auto ratio = ratioParam->load();

    juce::Path curve;
    juce::Path glowCurve;
    for (int px = 0; px <= (int)w; ++px)
    {
        float inNorm = px / w;
        float outNorm;
        float threshNorm = threshold / 100.0f;

        if (inNorm <= threshNorm)
            outNorm = inNorm;
        else
            outNorm = threshNorm + (inNorm - threshNorm) / ratio;

        float yPos = h - (outNorm * h);
        if (px == 0)
        {
            curve.startNewSubPath(0, yPos);
            glowCurve.startNewSubPath(0, yPos);
        }
        else
        {
            curve.lineTo((float)px, yPos);
            glowCurve.lineTo((float)px, yPos);
        }
    }

    // Glow
    g.setColour(juce::Colour(0xFF076dc3).withAlpha(0.2f));
    g.strokePath(glowCurve, juce::PathStrokeType(8.0f));

    // Main curve
    g.setColour(juce::Colour(0xFF076dc3));
    g.strokePath(curve, juce::PathStrokeType(2.0f));

    // Axis labels
    g.setColour(juce::Colour(0x40ffffff));
    g.setFont(juce::Font(trashRegular).withHeight(8.0f));
    g.drawText("OUT", 4, 2, 30, 12, juce::Justification::left);
    g.drawText("IN", (int)w - 20, (int)h - 14, 18, 12, juce::Justification::right);
}
```

---

## 4. Saturation Scope — Animated Waveform with Color Gradient

This is the trickiest visual. The HTML version uses canvas gradient strokes. In JUCE, use **segmented path drawing** with color interpolation — this is the recommended approach from JUCE forum.

### Class Structure

```cpp
class SaturationScope : public juce::Component, private juce::Timer
{
public:
    SaturationScope() { startTimerHz(60); }

    void setDrive(float d) { drive = d; }
    void setIntensity(float i) { intensity = i; }

    void timerCallback() override
    {
        phase += 0.03f;
        repaint();  // Only repaints this component's bounds
    }

    void paint(juce::Graphics& g) override;

private:
    float phase = 0.0f;
    float drive = 0.4f;
    float intensity = 0.45f;
};
```

### paint() — Segmented Color Drawing

```cpp
void SaturationScope::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    auto w = bounds.getWidth();
    auto h = bounds.getHeight();

    // Background
    g.setColour(juce::Colour(0xFF080a12));
    g.fillRoundedRectangle(bounds, 10.0f);

    // Border
    g.setColour(juce::Colour(0x33e6e9ef));
    g.drawRoundedRectangle(bounds, 10.0f, 1.0f);

    // Center line
    g.setColour(juce::Colour(0x0affffff));
    g.drawHorizontalLine((int)(h * 0.5f), 0, w);

    // --- Waveform rendering in segments for color gradient ---
    float midPoint = w * 0.35f;
    float satAmount = drive * 2.0f + intensity;
    int segmentWidth = 4;  // Draw in 4px segments for color transitions

    auto inputColour   = juce::Colour(0xFF6b7280).withAlpha(0.6f);
    auto satPink       = juce::Colour(0xFFe85590);
    auto satRed        = juce::Colour(0xFFff001f);

    for (int startPx = 0; startPx < (int)w; startPx += segmentWidth)
    {
        int endPx = juce::jmin(startPx + segmentWidth, (int)w);

        // Determine color for this segment
        juce::Colour segColour;
        float glowAlpha = 0.0f;

        if ((float)startPx < midPoint)
        {
            segColour = inputColour;
        }
        else
        {
            float progress = ((float)startPx - midPoint) / (w - midPoint);
            // Interpolate: gray -> pink -> red
            if (progress < 0.3f)
                segColour = inputColour.interpolatedWith(
                    satPink.withAlpha(0.5f + drive * 0.5f), progress / 0.3f);
            else
                segColour = satPink.interpolatedWith(
                    satRed.withAlpha(0.6f + drive * 0.4f), (progress - 0.3f) / 0.7f);
            glowAlpha = drive * 0.25f * progress;
        }

        // Build path segment
        juce::Path segment;
        juce::Path glowSegment;
        bool first = true;

        for (int px = startPx; px <= endPx; ++px)
        {
            float t = ((float)px / w) * juce::MathConstants<float>::pi * 6.0f + phase;
            float amp = 0.35f;
            float y = std::sin(t) * amp;

            if ((float)px > midPoint)
            {
                float progress = ((float)px - midPoint) / (w - midPoint);
                float clipGain = 1.0f + satAmount * progress * 3.0f;
                y = std::tanh(y * clipGain) / clipGain * clipGain * 0.7f;
                y = juce::jlimit(-0.45f, 0.45f, y);
            }

            float screenY = h * 0.5f + y * h;

            if (first)
            {
                segment.startNewSubPath((float)px, screenY);
                if (glowAlpha > 0.0f)
                    glowSegment.startNewSubPath((float)px, screenY);
                first = false;
            }
            else
            {
                segment.lineTo((float)px, screenY);
                if (glowAlpha > 0.0f)
                    glowSegment.lineTo((float)px, screenY);
            }
        }

        // Draw glow behind
        if (glowAlpha > 0.01f)
        {
            g.setColour(segColour.withAlpha(glowAlpha));
            g.strokePath(glowSegment, juce::PathStrokeType(10.0f));
        }

        // Draw main line
        g.setColour(segColour);
        g.strokePath(segment, juce::PathStrokeType(
            (float)startPx < midPoint ? 1.5f : 2.0f));
    }

    // --- Inner glow overlay (concentric radial gradient) ---
    float glowIntensity = drive * 0.12f + intensity * 0.06f;
    if (glowIntensity > 0.01f)
    {
        juce::ColourGradient glow(
            juce::Colour(0xFFe85590).withAlpha(glowIntensity),
            bounds.getCentreX(), bounds.getCentreY(),
            juce::Colours::transparentBlack,
            bounds.getX(), bounds.getCentreY(),
            true  // radial
        );
        g.setGradientFill(glow);
        g.fillRoundedRectangle(bounds, 10.0f);
    }
}
```

### Performance Notes
- `startTimerHz(60)` gives 60fps target. On constrained systems, use 30.
- `repaint()` only repaints this component's bounds, not the entire editor.
- The segmented approach (4px segments) gives ~100-150 path segments per frame — well within JUCE's software renderer capacity.
- If CPU is a concern: render to a `juce::Image` and only rebuild when params change, blitting the cached image each frame with just the phase offset adjusted.

---

## 5. Layout — resized() with Grid

Use `juce::Grid` for the three-column module layout. This is cleaner than manual arithmetic and handles the Opto/FET column proportion switch naturally.

```cpp
void PluginEditor::resized()
{
    auto area = getLocalBounds();

    // Header
    headerBar.setBounds(area.removeFromTop(38));

    // Mode switcher
    auto switcherArea = area.removeFromTop(32);
    modeSwitcher.setBounds(switcherArea.reduced(16, 4));

    // Footer
    footerBar.setBounds(area.removeFromBottom(30));

    // Module grid
    auto moduleArea = area.reduced(16, 6);

    juce::Grid grid;
    grid.templateRows = { juce::Grid::TrackInfo(juce::Grid::Fr(1)) };

    if (currentMode == Mode::Opto)
    {
        grid.templateColumns = {
            juce::Grid::TrackInfo(juce::Grid::Fr(85)),   // 0.85fr
            juce::Grid::TrackInfo(juce::Grid::Fr(160)),  // 1.6fr
            juce::Grid::TrackInfo(juce::Grid::Fr(70))    // 0.7fr
        };
    }
    else // FET
    {
        // Ensure minimum 320px for compressor column
        int compWidth = juce::jmax(320, (int)(moduleArea.getWidth() * 0.43f));
        grid.templateColumns = {
            juce::Grid::TrackInfo(juce::Grid::Px(compWidth)),
            juce::Grid::TrackInfo(juce::Grid::Fr(130)),
            juce::Grid::TrackInfo(juce::Grid::Fr(60))
        };
    }

    grid.columnGap = juce::Grid::Px(12);

    grid.items = {
        juce::GridItem(compressorModule),
        juce::GridItem(saturationModule),
        juce::GridItem(outputModule)
    };

    grid.performLayout(moduleArea);
}
```

### Animated Layout Transition (Opto ↔ FET)

```cpp
void PluginEditor::switchMode(Mode newMode)
{
    currentMode = newMode;

    // Capture target bounds via a temporary layout pass
    resized();  // This sets the final positions

    // For each module, animate from old bounds to new bounds
    auto& animator = juce::Desktop::getInstance().getAnimator();
    animator.animateComponent(&compressorModule, compressorModule.getBounds(),
                              1.0f, 300, false, 1.0, 0.0);
    animator.animateComponent(&saturationModule, saturationModule.getBounds(),
                              1.0f, 300, false, 1.0, 0.0);
    animator.animateComponent(&outputModule, outputModule.getBounds(),
                              1.0f, 300, false, 1.0, 0.0);
}
```

**Note:** In practice, you'll need to store the old bounds before calling `resized()`, then animate from old to new. `ComponentAnimator` interpolates position and size.

---

## 6. Module Card Drawing

Each module is a `Component` that draws its own background, border, and header bar in `paint()`:

```cpp
void CompressorModule::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // Card background
    g.setColour(juce::Colour(0xFF181c2e));  // pluginSurface
    g.fillRoundedRectangle(bounds, 16.0f);

    // Card border
    g.setColour(juce::Colour(0x33e6e9ef));  // pluginBorder
    g.drawRoundedRectangle(bounds, 16.0f, 2.0f);

    // Header bar (blue for compressor)
    auto headerBounds = bounds.removeFromTop(30.0f);
    juce::Path headerPath;
    headerPath.addRoundedRectangle(headerBounds.getX(), headerBounds.getY(),
                                    headerBounds.getWidth(), headerBounds.getHeight(),
                                    16.0f, 16.0f, true, true, false, false);
    g.setColour(juce::Colour(0xFF076dc3));  // ombicBlue
    g.fillPath(headerPath);

    // Header text
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(trashBold).withHeight(13.0f));
    g.drawText("COMPRESSOR", headerBounds.reduced(14, 0), juce::Justification::centredLeft);
}
```

For the Neon Saturation header gradient:

```cpp
// Pink gradient header
juce::ColourGradient headerGrad(
    juce::Colour(0xFFcc3a6e), headerBounds.getX(), headerBounds.getY(),
    juce::Colour(0xFFe85590), headerBounds.getRight(), headerBounds.getY(),
    false  // linear, not radial
);
g.setGradientFill(headerGrad);
g.fillPath(headerPath);
```

---

## 7. Meter Bars

Simple component, updated via Timer from the audio processor:

```cpp
void MeterBar::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // Background
    g.setColour(juce::Colour(0xFF0f1220));
    g.fillRoundedRectangle(bounds, 3.0f);

    // Border
    g.setColour(juce::Colour(0x33e6e9ef));
    g.drawRoundedRectangle(bounds, 3.0f, 1.0f);

    // Fill (from bottom)
    float fillHeight = bounds.getHeight() * level;  // level is 0.0-1.0
    auto fillBounds = bounds.withTop(bounds.getBottom() - fillHeight);

    g.setColour(fillColour);  // ombicBlue for input, ombicTeal for output
    g.fillRoundedRectangle(fillBounds, 2.0f);
}
```

---

## 8. Logo Watermark

Once you have a high-res white-on-transparent PNG:

```cpp
// Load from BinaryData
auto logoImage = juce::ImageCache::getFromMemory(
    BinaryData::OmbicLogo_png, BinaryData::OmbicLogo_pngSize);

// In PluginEditor::paint(), draw behind everything else:
void PluginEditor::paint(juce::Graphics& g)
{
    // Background
    g.fillAll(juce::Colour(0xFF0f1220));

    // Logo watermark — centered, 260px tall, 6% opacity
    if (logoImage.isValid())
    {
        float logoHeight = 260.0f;
        float logoWidth = logoImage.getWidth() * (logoHeight / logoImage.getHeight());
        float logoX = (getWidth() - logoWidth) * 0.5f;
        float logoY = (getHeight() - logoHeight) * 0.5f;

        g.setOpacity(0.06f);
        g.drawImage(logoImage, logoX, logoY, (int)logoWidth, (int)logoHeight,
                    0, 0, logoImage.getWidth(), logoImage.getHeight());
        g.setOpacity(1.0f);  // Reset
    }
}
```

---

## 9. Hard Offset Shadow (Brand Signature)

JUCE's `DropShadow` doesn't do zero-blur hard offsets well. Draw it manually:

```cpp
// In PluginEditor::paint(), before drawing the main plugin rect:
void PluginEditor::paint(juce::Graphics& g)
{
    auto pluginBounds = getLocalBounds().toFloat().reduced(12.0f);  // Leave room for shadow

    // Shadow (8px offset, no blur)
    g.setColour(juce::Colour(0x80000000));  // 50% black
    g.fillRoundedRectangle(pluginBounds.translated(8.0f, 8.0f), 20.0f);

    // Main plugin background
    g.setColour(juce::Colour(0xFF0f1220));
    g.fillRoundedRectangle(pluginBounds, 20.0f);

    // Plugin border
    g.setColour(juce::Colour(0x59e6e9ef));
    g.drawRoundedRectangle(pluginBounds, 20.0f, 3.0f);

    // Logo watermark (see section 8)
    // ...
}
```

---

## 10. Inner Glow for Scope (CSS box-shadow: inset equivalent)

JUCE has no inset shadow. Use radial `ColourGradient` — this is the recommended approach from JUCE forum.

```cpp
// At the end of SaturationScope::paint(), after drawing the waveform:
float glowIntensity = drive * 0.12f + intensity * 0.06f;
if (glowIntensity > 0.01f)
{
    // Primary pink inner glow
    juce::ColourGradient innerGlow(
        juce::Colour(0xFFe85590).withAlpha(glowIntensity),
        bounds.getCentreX(), bounds.getCentreY(),
        juce::Colours::transparentBlack,
        bounds.getX(), bounds.getCentreY(),
        true  // radial
    );
    g.setGradientFill(innerGlow);
    g.fillRoundedRectangle(bounds, 10.0f);

    // Secondary red deeper glow
    juce::ColourGradient deepGlow(
        juce::Colour(0xFFff001f).withAlpha(glowIntensity * 0.5f),
        bounds.getCentreX(), bounds.getCentreY(),
        juce::Colours::transparentBlack,
        bounds.getX(), bounds.getY(),
        true
    );
    g.setGradientFill(deepGlow);
    g.fillRoundedRectangle(bounds, 10.0f);
}
```

---

## 11. Key JUCE API Reference

| HTML/CSS Concept | JUCE Equivalent | Notes |
|---|---|---|
| `border-radius: 16px` | `fillRoundedRectangle()` / `drawRoundedRectangle()` | |
| `background: #181c2e` | `g.setColour(); g.fillRect()` | |
| `box-shadow: 8px 8px 0 0` | Manual: draw shadow rect offset behind main rect | |
| `box-shadow: inset 0 0 30px` | `ColourGradient` radial fill | |
| CSS Grid `fr` units | `juce::Grid` with `Fr()` tracks | |
| CSS `transition: 0.3s` | `ComponentAnimator::animateComponent()` | |
| SVG arc stroke | `Path::addCentredArc()` + `strokePath()` | Watch angle conventions |
| Canvas gradient stroke | Segmented path drawing, color per segment | |
| `filter: blur(4px)` | `ImageConvolutionKernel` or wider semi-transparent stroke | |
| `@font-face` | `Typeface::createSystemTypefaceFor(BinaryData::...)` | |
| `requestAnimationFrame` | `juce::Timer` with `startTimerHz(60)` | |
| `opacity: 0.06` | `g.setOpacity(0.06f)` or `colour.withAlpha(0.06f)` | |

---

## 12. Build Order for Cursor

Implement in this order, testing each before moving on:

1. **OmbicLookAndFeel** — font loading, color constants, `drawRotarySlider()` override
2. **PluginEditor shell** — background, shadow, header, footer, grid layout
3. **CompressorModule** — card drawing, transfer curve, mode switching with 1/4 knobs
4. **OutputModule** — card, meter bars, output knob, GR readout
5. **SaturationModule** — card, knobs, scope (static first, then animated)
6. **Logo watermark** — last, once you have the asset
7. **Polish** — layout animation, glow refinement, interaction tuning

Prompt Cursor with one component at a time, referencing this file for the exact API calls.
