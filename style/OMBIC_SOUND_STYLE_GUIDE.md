# OMBIC Sound — Design System & Style Guide

> **Purpose:** Machine-readable style guide for LLMs and AI code editors (Cursor, Copilot, Claude, etc.) to produce UI that is visually consistent with the OMBIC Sound brand. Follow these specifications exactly when building any OMBIC-branded tool, page, or component.

---

## 1. Color Palette

### CSS Custom Properties

```css
:root {
  /* Brand Colors */
  --ombic-red:    #ff001f;   /* Primary accent — headers, alerts, delete/close actions */
  --ombic-blue:   #076dc3;   /* Primary actions — buttons, links, selected states, hover borders */
  --ombic-teal:   #338fab;   /* Confirmed/decided states — success indicators */
  --ombic-yellow: #ffce00;   /* Highlights — selected cards, attention, progress */
  --ombic-purple: #5300aa;   /* Dark accent — finalize actions, secondary accents */
  --ombic-pink:   #ffb3c9;   /* Soft accent — sparingly, decorative only */

  /* Neutrals */
  --ink:          #0f1220;   /* Primary text, borders, shadows */
  --muted:        #586070;   /* Secondary text, descriptions */
  --line:         #e6e9ef;   /* Dividers, subtle borders */
  --bg:           #ffffff;   /* Page and card backgrounds */
}
```

### Color Usage Rules

| Context                | Color                     | Notes                                    |
|------------------------|---------------------------|------------------------------------------|
| Page background        | `--bg` (#ffffff)          | Always white. Never gray, never dark.    |
| All borders            | `--ink` (#0f1220)         | Black. Not colored, not gray.            |
| All box shadows        | `--ink` (#0f1220)         | Hard offset only (see Shadows section).  |
| Primary text           | `--ink` (#0f1220)         | Body copy, labels, headings.             |
| Secondary text         | `--muted` (#586070)       | Descriptions, hints, metadata.           |
| App header bar         | `--ombic-red` background  | White text on red.                       |
| Section headers        | `--ombic-blue` background | White text on blue.                      |
| Primary action buttons | `--ombic-blue` background | White text. Black border + shadow.       |
| Finalize/confirm       | `--ombic-purple` bg       | White text. Black border + shadow.       |
| Selected card state    | `--ombic-yellow` bg       | Black text. Card fills yellow.           |
| Hover border highlight | `--ombic-blue` border     | On interactive elements.                 |
| Confirmed/decided      | `--ombic-teal`            | Checkmarks, status indicators.           |
| Delete/close/alert     | `--ombic-red`             | Destructive actions, warnings.           |
| Dividers               | `--line` (#e6e9ef)        | Thin horizontal rules inside cards.      |

---

## 2. Typography

### Font Stack

```css
/* Primary — use when Trash font files are available */
font-family: 'Trash', system-ui, -apple-system, 'Segoe UI', sans-serif;

/* Fallback — use when Trash is not embedded */
font-family: system-ui, -apple-system, 'Segoe UI', sans-serif;
```

### Trash Font Family

The brand typeface is **Trash** with four variants:
- Trash Regular
- Trash Regular Italic
- Trash Bold
- Trash Bold Italic

If font files (.ttf) are available, embed via `@font-face`. If not, use the system font fallback — the design still works because it relies on weight and size, not the typeface alone.

### Type Scale

| Element             | Size   | Weight | Notes                        |
|---------------------|--------|--------|------------------------------|
| App title / H1      | 24px   | 900    | Uppercase optional            |
| Section headers     | 20px   | 900    | Inside colored bar            |
| Card titles         | 20px   | 900    | Option labels, card headings  |
| Body text           | 14px   | 700    | Default for all content       |
| Small labels/badges | 11px   | 900    | Uppercase, letter-spacing 0.3px |
| Descriptions        | 12px   | 700    | Muted color                   |

### Key Rules

- **Minimum weight is 700 (bold).** Never use 400/normal weight. Everything is bold or black (900).
- Text transform: uppercase for badges, labels, and small UI elements.
- Line height: 1.3–1.4 for body text.

---

## 3. Borders

```css
/* Standard border for all cards, inputs, buttons */
border: 3px solid var(--ink);

/* Small elements (badges, pills) */
border: 2px solid var(--ink);

/* Dividers inside cards */
border-top: 2px solid var(--line);
```

### Rules

- **Always black** (`--ink`). Never colored borders (except hover state → blue).
- **Always 2–3px.** Never 1px hairline borders.
- Border radius: `20px` for cards and large containers, `12px` for buttons and small elements, `999px` for pill badges.

---

## 4. Shadows

```css
/* Standard hard offset shadow */
box-shadow: 8px 8px 0 0 var(--ink);

/* Smaller elements */
box-shadow: 6px 6px 0 0 var(--ink);

/* Hover lift state */
box-shadow: 6px 6px 0 0 var(--ink);
/* combined with transform: translate(-2px, -2px) */
```

### Rules

- **Hard offset only.** Zero blur. Zero spread. This is the signature OMBIC look.
- **Never use soft/blurred shadows.** No `box-shadow: 0 4px 12px rgba(...)`.
- Shadow color is always `--ink` (black).
- Shadow direction is always bottom-right (positive x, positive y).

---

## 5. Cards

```css
.card {
  background: var(--bg);           /* white */
  border: 3px solid var(--ink);    /* black */
  border-radius: 20px;
  padding: 24px;                   /* 24px–32px depending on content */
  box-shadow: 8px 8px 0 0 var(--ink);
  position: relative;
}
```

### Card States

```css
/* Default */
.card {
  background: white;
  border: 3px solid var(--ink);
  box-shadow: 8px 8px 0 0 var(--ink);
}

/* Hover (interactive cards only) */
.card:hover {
  transform: translate(-2px, -2px);
  box-shadow: 6px 6px 0 0 var(--ink);
  cursor: pointer;
}

/* Selected */
.card.selected {
  background: var(--ombic-yellow);
  transform: translate(-2px, -2px);
  box-shadow: 6px 6px 0 0 var(--ink);
}

/* Disabled */
.card:disabled, .card.disabled {
  background: #f0f0f0;
  opacity: 0.6;
  cursor: not-allowed;
  box-shadow: none;
}
```

---

## 6. Buttons

### Primary Button

```css
.btn-primary {
  background: var(--ombic-blue);
  color: white;
  font-size: 14px;
  font-weight: 900;
  border: 3px solid var(--ink);
  border-radius: 12px;
  padding: 10px 20px;
  box-shadow: 4px 4px 0 0 var(--ink);
  cursor: pointer;
  transition: all 0.2s;
}

.btn-primary:hover {
  transform: translate(-2px, -2px);
  box-shadow: 6px 6px 0 0 var(--ink);
}

.btn-primary:active {
  transform: translate(1px, 1px);
  box-shadow: 2px 2px 0 0 var(--ink);
}
```

### Secondary / Edit Button

```css
.btn-secondary {
  background: white;
  color: var(--ink);
  font-size: 12px;
  font-weight: 900;
  border: 2px solid var(--ink);
  border-radius: 999px;           /* pill shape */
  padding: 6px 14px;
  cursor: pointer;
  transition: all 0.2s;
}

.btn-secondary:hover {
  background: var(--ombic-yellow);
  border-color: var(--ink);
}
```

### Finalize / Confirm Button

```css
.btn-finalize {
  background: var(--ombic-purple);
  color: white;
  font-size: 14px;
  font-weight: 900;
  border: 3px solid var(--ink);
  border-radius: 12px;
  padding: 10px 20px;
  box-shadow: 4px 4px 0 0 var(--ink);
}
```

### Destructive / Delete Button

```css
.btn-danger {
  background: var(--ombic-red);
  color: white;
  /* Same border/shadow pattern as primary */
}
```

### Button State Summary

| State    | Transform              | Shadow                          |
|----------|------------------------|---------------------------------|
| Default  | none                   | `4px 4px 0 0 var(--ink)`        |
| Hover    | `translate(-2px, -2px)`| `6px 6px 0 0 var(--ink)`       |
| Active   | `translate(1px, 1px)`  | `2px 2px 0 0 var(--ink)`       |
| Disabled | none                   | none                            |

---

## 7. Badges & Pills

```css
.badge {
  display: inline-block;
  font-size: 11px;
  font-weight: 900;
  text-transform: uppercase;
  letter-spacing: 0.3px;
  color: var(--ink);
  background: white;
  border: 2px solid var(--ink);
  border-radius: 999px;
  padding: 4px 10px;
}

/* Inverted state (e.g., inside a selected card) */
.badge.inverted,
.card.selected .badge {
  background: var(--ink);
  color: var(--ombic-yellow);
}
```

---

## 8. Layout & Spacing

### Spacing Scale

| Token     | Value | Usage                              |
|-----------|-------|------------------------------------|
| `--sp-xs` | 4px   | Tight gaps (badge padding)         |
| `--sp-sm` | 8px   | Small gaps (between inline items)  |
| `--sp-md` | 12px  | Element gaps (within cards)        |
| `--sp-lg` | 16px  | Section internal padding           |
| `--sp-xl` | 20px  | Between section cards              |
| `--sp-2xl`| 24px  | Card padding                       |
| `--sp-3xl`| 32px  | Large card padding, page margins   |

### Page Structure

```
┌──────────────────────────────────────────────────┐
│  APP HEADER (--ombic-red bg, white text, 900)    │
│  Subtitle (white text, 700, smaller)             │
├──────────────────────────────────────────────────┤
│                                                  │
│  ┌──────────────────────────────────────────┐    │
│  │  SECTION HEADER (--ombic-blue bg, white) │    │
│  ├──────────────────────────────────────────┤    │
│  │                                          │    │
│  │  Card content with 24px padding          │    │
│  │                                          │    │
│  └──────────────────────────────────────────┘    │
│                     ↕ 20px gap                    │
│  ┌──────────────────────────────────────────┐    │
│  │  NEXT SECTION                            │    │
│  └──────────────────────────────────────────┘    │
│                                                  │
└──────────────────────────────────────────────────┘
```

### Max Width

```css
.container {
  max-width: 900px;    /* single-column tools */
  margin: 0 auto;
  padding: 20px;
}
```

---

## 9. Interactive States & Transitions

```css
/* Global transition for all interactive elements */
transition: all 0.2s;
```

### Hover Pattern (Cards & Buttons)

The signature interaction: element "lifts" toward top-left, shadow grows.

```css
.interactive:hover {
  transform: translate(-2px, -2px);
  box-shadow: 6px 6px 0 0 var(--ink);
}
```

### Active / Press Pattern

Element "presses down" — moves toward bottom-right, shadow shrinks.

```css
.interactive:active {
  transform: translate(1px, 1px);
  box-shadow: 2px 2px 0 0 var(--ink);
}
```

---

## 10. Form Inputs

```css
input, textarea, select {
  font-family: inherit;
  font-size: 14px;
  font-weight: 700;
  color: var(--ink);
  background: white;
  border: 3px solid var(--ink);
  border-radius: 12px;
  padding: 10px 14px;
  outline: none;
  transition: all 0.2s;
}

input:focus, textarea:focus, select:focus {
  border-color: var(--ombic-blue);
  box-shadow: 4px 4px 0 0 var(--ombic-blue);
}
```

---

## 11. Modals & Overlays

```css
.modal-overlay {
  background: rgba(15, 18, 32, 0.5);   /* --ink at 50% */
}

.modal {
  background: white;
  border: 3px solid var(--ink);
  border-radius: 20px;
  padding: 32px;
  box-shadow: 12px 12px 0 0 var(--ink);
  max-width: 600px;
}
```

---

## 12. Chord Diagrams (Domain-Specific)

When rendering guitar chord diagrams as SVG:

```css
/* Diagram container */
.chord-diagram {
  background: white;
  /* No border or shadow on individual diagrams */
}

/* Fret grid lines */
stroke: var(--ink);
stroke-width: 2;

/* Finger dots */
fill: var(--ink);
r: 8;  /* radius */

/* Finger number text inside dots */
fill: white;
font-size: 11px;
font-weight: 900;

/* Open (○) and mute (×) markers */
stroke: var(--ink);
font-size: 14px;
font-weight: 900;

/* Chord name below diagram */
font-size: 14px;
font-weight: 900;
fill: var(--ink);
text-anchor: middle;
```

---

## 13. Do / Don't Quick Reference

### DO ✓

- Use hard offset black shadows (`8px 8px 0 0`)
- Use 3px solid black borders on everything
- Use font-weight 700 minimum, 900 for headings
- Use white backgrounds with color accents
- Use the lift-on-hover / press-on-active interaction pattern
- Use pill badges for category labels
- Use yellow for selected states
- Keep layouts clean with generous whitespace

### DON'T ✗

- Don't use soft/blurred shadows (e.g., `0 4px 12px rgba(...)`)
- Don't use 1px hairline borders
- Don't use gray backgrounds for pages or cards
- Don't use gradients anywhere
- Don't use font-weight below 700
- Don't use colored borders (except hover → blue)
- Don't use rounded-full on cards (that's for badges only)
- Don't center everything — left-align text content
- Don't use opacity for hover states — use transform + shadow

---

## 14. Component Checklist

When building a new OMBIC-branded component, verify:

- [ ] Background is white
- [ ] Borders are 3px solid black (#0f1220)
- [ ] Shadows are hard offset (no blur)
- [ ] Font weight is ≥ 700
- [ ] Interactive elements have hover lift + active press
- [ ] Selected state uses yellow background
- [ ] Buttons follow the primary/secondary/finalize pattern
- [ ] Color palette matches the six brand colors exactly
- [ ] No soft shadows, no gradients, no hairline borders
- [ ] Border radius is 20px (cards) or 12px (buttons)
