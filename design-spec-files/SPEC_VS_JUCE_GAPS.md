# OMBIC Compressor — Spec / HTML vs JUCE: Gaps and Remaining Work

> **Purpose:** So you can direct improvements. Compare `OMBIC_COMPRESSOR_JUCE_SPEC.md`, `ombic-compressor.html`, and the JUCE plugin and close gaps systematically.

---

## Applied in this pass (layout / fit)

- **Default size:** 900×480 (spec §1). Was 980×540; now matches spec and fits REAPER.
- **Main VU height:** 100px (was 172). Frees vertical space for module knobs.
- **Main VU readouts:** Compact centered row (fixed-width labels + gap), not 1/3 width each; small font per main-vu-toggle-demo.html.
- **High impact (spec match):** Output meters 6×80px (§8); Opto 80px, FET 60px, Output knob 56px (§5); Compressor FET gap 16px (§6).
- **Pills row:** 32px height, 26px pill height; grid gap 12px; bottom padding 16px.
- **Saturator:** Scope 82px, body padding 14px; knobs 48px, gap 14px (spec §7).
- **Compressor:** Body padding 14px (spec §4).
- **Resize limits:** Minimum 900×480 so users can’t shrink below usable height.
- **Knob display:** Any 0–1 parameter must show as **0–100%** in the UI (e.g. Neon Drive/Intensity/Tone/Mix). Use slider textFromValue/valueFromText and call `SaturatorSection::applyPercentDisplay()` after attachments so the display is not overwritten.

---

## Layout / structure

| Item | Spec / HTML | JUCE now | Action |
|------|-------------|----------|--------|
| **Default size** | 900×480 §1 | 900×480 | Done |
| **Grid columns** | 3 (Compressor \| Neon \| Output) §3 | 4 (SC Filter + Compressor + Neon + Output) | Spec doesn’t define SC Filter; HTML has 3 modules. Either document SC as “add-on” or consider folding SC into Compressor (e.g. top of compressor card). |
| **Main VU strip** | Not in spec/HTML | Fancy/Simple strip above modules | Design evolution; keep. Height now 100px. |
| **Transfer curve** | §6: inside Compressor card, 90px tall | In Main VU (Fancy) only; not in Compressor card | Intentional; curve is global. Optional: add small 90px curve inside Compressor card per spec if you want pixel-perfect match. |
| **Grid gap** | 12px §3 | 12px | Done |
| **Footer** | 8px vertical padding, 20px horizontal §10 | Similar; height 26px | OK |

---

## Typography (spec §11 / HTML)

| Element | Spec | JUCE | Gap |
|---------|------|------|-----|
| Plugin title | 18px, 900, white, letter-spacing 1.5px | 18px bold (700), white | No Trash 900 loaded; letter-spacing not set in JUCE (would need per-char draw). |
| Module headers | 13px, 900, white | 13px bold | Same as above. |
| Param labels | 9px, 700, uppercase, pluginMuted | 9px bold, uppercase | OK. |
| Param values | 14px, 900; Opto 20px | 14px / 20px bold | OK. |
| Mode pills | 11px, 900 | Uses default L&F | Consider explicit 11px bold for pill text. |
| Footer | 9px, 700, pluginMuted | 9px bold | OK. |

**Action:** Load Trash Black (900) if available and use for 18px/13px/11px headers and pills; optionally draw title with manual letter-spacing.

---

## Knobs (§5 / HTML)

| Item | Spec | JUCE | Action |
|------|------|------|--------|
| Sweep angles | 240° (150°–390°); GUIDE -2.356, 2.356 rad | setRotaryParameters(-2.356f, 2.356f, true) | Done |
| Opto threshold | 80px diameter | 80px | Done |
| FET knobs | 60px | 60px | Done |
| Saturation knobs | 48px | 48px | Done |
| Output knob | 56px | 56px | Done |
| Stroke / glow / pointer | Per GUIDE | drawRotarySlider per GUIDE | Done |
| Accent colours | Blue / #e85590 / Purple / Teal | setColour(rotarySliderFillColourId) | Done |

---

## Module cards (§4 / HTML)

| Item | Spec | JUCE | Action |
|------|------|------|--------|
| Border radius | 16px | 16px | OK |
| Header height | 8px vert padding, 14px horiz; 13px text | 36px header; 14px padding in places | Header is taller than “8px padding”; match if you want exact spec. |
| Body padding | 14px | 14px (Compressor, Saturator); 18px (Output) | Set Output body padding to 14px. |

---

## Output module (§8 / HTML)

| Item | Spec / HTML | JUCE | Action |
|------|-------------|------|--------|
| Meter width | 6px | 6px | Done |
| Meter height | 80px | 80px | Done |
| GR value | 16px, 900, color-coded | 16px bold, color-coded | OK. |

---

## Neon saturation (§7 / HTML)

| Item | Spec / HTML | JUCE | Action |
|------|-------------|------|--------|
| Scope height | 110px §7 | 82px (compact) | 82px for fit; increase toward 90–110 if vertical space allows after other tweaks. |
| Scope background | #080a12 | Same | OK |
| Waveform | Clean → saturated, gradient stroke, glow | Scope with gradient/glow | Implement full gradient (pluginMuted → #e85590 → ombicRed) and inner glow per GUIDE §10 if not already. |
| Knobs | 48px, 16px gap | 48px, 14px gap | OK (gap 14 vs 16 minor). |

---

## Compressor (§6)

| Item | Spec | JUCE | Action |
|------|------|------|--------|
| Opto: single 80px threshold | 80px centered | 84px | Use 80px. |
| FET: 60px, 16px gap | 60px, 16px gap | 60px, 16px gap | Done |
| GR meter in card | — | Present | OK. |
| Curve in card | 90px tall | Only in Main VU | Optional: add 90px curve block inside Compressor card for spec match. |

---

## Shadow / brand (§12)

| Item | Spec | JUCE | Action |
|------|------|------|--------|
| Plugin shadow | 8px 8px, 0 blur, pluginShadow | 8px offset, fillRoundedRectangle | OK. |
| Outer border | 3px pluginBorderStrong | Not drawn | **Add** 3px outer border on main plugin rect if desired for spec. |

---

## Summary: what’s left to feel “complete” vs spec/HTML

**High impact (visual match)** — *done*  
1. ~~**Output meters:** Width 6px, height 80px (spec §8 / HTML).~~  
2. ~~**Knob sizes:** Opto 80px, FET 60px, Output 56px (spec §5).~~  
3. ~~**Compressor FET:** 16px gap between knobs (spec §6).~~

**Medium impact**  
4. **Output module body padding:** 14px (spec §4).  
5. **Header bar:** Spec “8px vertical padding” → could reduce header height slightly and align padding.  
6. **Trash 900:** Load Black weight for titles/headers if you have the font; otherwise 700 is acceptable.

**Lower / optional**  
7. **Letter-spacing:** 1.5px on “OMBIC COMPRESSOR” (per-char drawing in JUCE).  
8. **Transfer curve inside Compressor card:** 90px block if you want exact §6 layout.  
9. **Outer border:** 3px pluginBorderStrong around main plugin.  
10. **SC Filter column:** Document as extension to spec or consider merging into Compressor layout.

**How to use this**  
- Pick one section (e.g. “Output meters” or “Knob sizes”) and say: “Match spec for Output” or “Match spec for knobs.”  
- Or: “Implement all High impact items” and we do 1–3 in code.  
- After that, we can do “Medium” and “Optional” in order of priority.
