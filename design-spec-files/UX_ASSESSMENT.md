# OMBIC Compressor — UX assessment (current UI)

Quick view of what the UI is doing well vs poorly from a UX perspective, and what was just changed.

---

## What it’s doing well

- **Clear grouping** — Four modules (SC Filter, Compressor, Neon Saturation, Output) with distinct colors (teal, blue, pink, purple) make it easy to scan and know where you are.
- **Consistent pattern** — Each module: colored header → controls → values. Same structure everywhere reduces learning effort.
- **Readable feedback** — Numeric values under knobs and GR readouts (with color coding: teal / yellow / red) give clear state.
- **High-level choices first** — Opto/FET and Fancy/Simple are at the top so mode is chosen before digging into parameters.
- **Primary focus on metering** — The 80/20 split (big VU, compact control strip) puts “what’s happening to the signal” ahead of “what I’m twisting.”
- **Brand consistency** — One palette, one font family, one shadow style so it feels like a single product.

---

## What it was doing poorly (and what we changed)

- **Headers dominated** — 36px headers and 13px bold type took a lot of space and drew more attention than the controls. **Change:** Headers reduced to 28px (22px in compact), 11px font so they identify the module without overpowering it.
- **Knobs too small** — Main interaction targets were smaller than the module names, which is the opposite of “controls first.” **Change:** Knobs increased (e.g. Compressor FET 66→72px, Opto 84→88px; Saturation 48→56px; Output 60→68px; SC 56→62px; compact sizes bumped where layout allows).
- **Inverted hierarchy** — Best practice: the thing you interact with should be the visual hero. **Change:** Smaller headers and larger knobs put emphasis back on the controls.
- **Tight compact strip** — With 80/20, the bottom strip was very dense; small knobs there were hard to use. **Change:** Headers shrunk so more of the strip is usable for knobs; compact knob sizes increased where there’s room.

---

## What could still be improved

- **Touch / precision** — Knobs are still mouse/trackpad sized; for touch or very high-DPI, consider even larger hit areas or a “large controls” mode.
- **Red-line / GR meter** — Compressor GR meter can still look “stuck” when the signal stops; same decay-when-idle idea as the Main VU could be applied there.
- **Affordance** — Knobs read as “twistable”; pills and buttons are clear. Meter fills could better suggest “level” (e.g. gradient or scale) if you want to refine later.
- **Density vs clarity** — In compact mode, some labels are 6px; if users struggle to read them, consider a minimum font size or optional “compact labels off” so only values show.

---

## Summary

The layout was **doing well** at grouping, consistency, and putting metering first, but **doing poorly** at making the primary controls (knobs) the main visual and interaction focus. Headers are now smaller and knobs larger so the UI follows a more standard “controls first” hierarchy and is easier to use.
