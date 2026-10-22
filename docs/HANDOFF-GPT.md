# HANDOFF — ENTRO V COLLECTION UI rewrite

## 0. Project path (absolute)

Work in this repository, not in the current shell directory:

```text
/Users/faputa/Documents/Codex/2026-10-20/ni/outputs/analog-triad
```

Git remote:

```text
https://github.com/SakuraEntropia/ENTRO-V-COLLECTION
branch: main
handoff base commit: f78a71d
```

Build directory:

```text
/Users/faputa/Documents/Codex/2026-10-20/ni/work/build-triad
```

JUCE path:

```text
/Users/faputa/Documents/Codex/2026-10-20/ni/work/JUCE
```

Installed app destination:

```text
/Applications/MATRIARCH V.app
/Applications/POLYBRUTE V.app
/Applications/ONE V.app
/Applications/CONTINUUM V.app
```

Also mirror the four apps into:

```text
/Users/faputa/Documents/Codex/2026-10-20/ni/outputs/analog-triad/dist/
```

---

## 1. Read this before touching UI

- This is a **UI fidelity handoff**, not a request to rewrite the DSP.
- Do **not** change parameter IDs, preset names, APVTS layout, state format,
  MIDI/MPE behavior, or audio algorithms unless a UI change strictly requires it.
- Preserve every existing control binding and callback.
- Do not claim “1:1” or “exactly like the hardware” unless the result was
  actually measured from a straight-on reference. This project has a history of
  over-claiming approximate work.
- Use manufacturer photos/manuals only as **reference**. Do not ship copyrighted
  panel photos or manual raster art as UI assets. Redraw with JUCE vectors.
- Keep the existing original product identity: MATRIARCH V, POLYBRUTE V, ONE V,
  CONTINUUM V. Never reintroduce Moog/Arturia/Haken logos or names as if
  affiliated.

The user is highly visual and rejects generic plugin panels. The target is
physical-instrument realism for the three analog synths and a Haken
EaganMatrix-like editor for Continuum.

---

## 2. Product state

Four independent plugins share a C++20/JUCE codebase:

| Product | Engine | APVTS params | Presets |
|---|---|---:|---:|
| MATRIARCH V | Matriarch | 63 | 15 |
| POLYBRUTE V | PolyBrute | 117 | 17 |
| ONE V | Moog One-inspired | 408 | 15 |
| CONTINUUM V | Continuum/EIGEN MATRIX | 126 | 615 |

Continuum uses the restored Haken 10.09 name/category library: 594 system + 16
USR entries, plus custom presets including:

- `Kevin Penkin Style Eharp`
- `Kevin Penkin Style E Koto`
- modern `BA - Reese Surface`, `BA - Sub Surface`, `PT - Glass Harp`

The Haken PDF contains names and categories only, not EaganMatrix parameter
data. The current preset sounds are original approximations.

---

## 3. Current UI architecture

All UI is in `src/ui`. Design coordinates are absolute; panels are scaled by
the editor viewport.

| File | Responsibility |
|---|---|
| `PanelBase.h/.cpp` | Base panel, module frames, knobs/faders/choices, branding, finish palettes |
| `LookAndFeel.h/.cpp` | Rotary/linear/combo/button drawing, hardware styles |
| `Editor.h/.cpp` | Header bar, viewport/fit zoom, on-screen keyboard, Continuum surface + monitor, physical key mapping |
| `KeyboardPanel.h/.cpp` | Piano keyboard + pitch/mod wheels |
| `MatriarchPanel.cpp` | Matriarch layout |
| `PolyBrutePanel.cpp` | PolyBrute layout + 3D MorphPad |
| `OnePanel.cpp` | ONE V layout |
| `ContinuumPanel.h/.cpp` | Continuum matrix/surface/monitor/effect faders |
| `Widgets.h/.cpp` | Switches, LEDs, LCD, patch cables |
| `PresetBrowser.*` | Preset browser overlay |

### Design sizes and finishes

- Matriarch: 1800 × 500, `Finish::Matriarch`
- PolyBrute: 1900 × 1000, `Finish::PolyBrute` (Minibrute-style dark panel)
- One: 2240 × 760, `Finish::MoogOne`
- Continuum: 2800 × 1260, `Finish::Continuum` (light Haken-like palette)

`PanelBase::Finish` is chosen from the engine id in `PanelBase.cpp`.

### Continuum current special behavior

- EIGEN MATRIX: 16 × 16, 13 formulas including user formula.
- User formula VM: expression editor under matrix, supports `s/x/y/z`,
  arithmetic, `sin/cos/tan/abs/sqrt/exp/log`, `APPLY`.
- CVC 1–4: software proxy via MIDI CC 20–23, not hardware CV jacks.
- Surface: fixed base note 60 + continuous ±24 st `NoteExpression`; no note
  retrigger at semitone boundaries.
- PITCH SNAP: 0 = continuous, 1 = semitone quantized, intermediate = blend.
- Pressure comes from `MouseEvent::pressure` when valid; wheel/slider fallback.
- On-screen piano is hidden for Continuum; physical keys A W S E D F T G Y H U J K
  map to notes 48–60.
- 3D display is the top strip (circle X/Y, pressure radius, character line).
- Red/black surface is the bottom strip replacing the keyboard.
- Six vertical Delay-LPF trapezoid faders at the bottom:
  LPF / OFFSET / PINGPONG / FEEDBACK / FULL / TIME.
- Audio effects currently include: FDN reverb, chorus, phaser, modulated echo,
  summed/micro delay banks, two generated-IR convolution stages, six
  Recirculator modes.

### PolyBrute current special behavior

- Lower-left 3D `MorphPad`:
  - X = `poly.voice.morph`
  - Y = `poly.voice.spread`
  - Z = pressure/aftertouch
  - pressure slider is the no-touchpad fallback
- This is not the hardware Morphée/ribbon; it is an M1 performance approximation.

---

## 4. Work requested from GPT

### A. MATRIARCH V — redraw from the physical instrument

Required outcome:

- Orthographic front-panel vector redraw, preserving the real Matriarch
  single-row module order:
  ARP/SEQ, MODULATION, UTILITIES, OSC 1–4, MIXER, FILTERS, UTILITIES 2,
  ENVELOPES, STEREO DELAY, OUTPUT.
- Match the physical module widths, knob/fader sizes, jack positions, patch
  bay, wood/black end cheeks, white silkscreen, panel proportions.
- Keep the current 79 patch points and all cable interactions.
- Do not turn it into the PolyBrute/Minibrute card style.
- Reference:
  `/Users/faputa/Documents/Codex/2026-10-20/ni/work/manuals/mat_product.jpg`
  and `/Users/faputa/Documents/Codex/2026-10-20/ni/work/manuals/matriarch.pdf`
  plus `mat_pages/`.

### B. POLYBRUTE V — real PolyBrute layout + Minibrute V unit style

Required outcome:

- Physical PolyBrute module order and proportions:
  PERFORMANCE / VOICE / VCO 1 / MIXER / STEINER FILTER / VCF ENV / LFO 1;
  VCO 2 / LADDER FILTER / VCA ENV / LFO 2;
  FILTER FM & NOISE / ROUTING / MOD ENV / LFO 3;
  MATRIX / EFFECTS / ARPEGGIATOR.
- Keep the Minibrute-V-like unit visual language (black panel, white outline,
  white pointer knobs, white fader caps, orange Steiner tab).
- Put the display/LCD at the top of the matrix and keep the 3D MorphPad at
  lower-left.
- Reference:
  `/Users/faputa/Documents/Codex/2026-10-20/ni/work/manuals/pb_wide.png`
  (straight-on), `pb_product.jpg`, `hi/pb-013.png`, `hi/pb-014.png`,
  `pb_pages/`.
- Do not copy the Minibrute V one-VCO/one-filter layout onto PolyBrute.

### C. ONE V — redraw from the physical Moog One

Required outcome:

- Three-section physical layout: left Part 1–3 oscillators, centre console /
  mixer / arpeggiator, right filters/envelopes/LFOs/matrix/effects.
- Dark wood cabinet, black module plates, silver trim, gold/cream knob caps,
  realistic fader and LCD treatment.
- Keep the 408-parameter mapping and 3-part UI.
- Reference:
  `/Users/faputa/Documents/Codex/2026-10-20/ni/work/manuals/one_product.jpg`,
  `moogone.pdf`, `one_pages/`.

### D. CONTINUUM V — use the Haken editor reference, not generic cards

Required outcome:

- Follow the real Haken Editor composition:
  - wide matrix as the dominant element;
  - top control-panel parameter rows;
  - **six vertical inverted trapezoid faders** for Delay-LPF (R1–R6);
  - formula/parameter blocks and numeric fields at the bottom, not a generic
    row of equal knobs;
  - light silver-green Light theme by default;
  - matrix destination/source labels organized like the real editor.
- Preserve:
  - red/black surface at the bottom;
  - 3D display at the top;
  - physical keyboard mapping;
  - PITCH SNAP, user formula editor, CVC proxy, six Recirculator modes,
    convolution stages and delay banks.
- Reference images (copied into this repo):
  - `docs/ui-reference/haken-control-panel-2.png` — six vertical trapezoid
    Delay-LPF faders, convolution IR controls, compressor/tilt EQ layout.
  - `docs/ui-reference/haken-editor-themes.png` — original Light/Dark/Neon/
    Black/White editor themes and overall composition.
  - `docs/ui-reference/haken-editor-formula.png` — matrix/formula structure.
  - `docs/ui-reference/current-continuum.png` — current implementation for
    comparison.
- Original Haken guide used for extraction:
  `/tmp/eaganmatrix-guide.pdf` (temporary; if absent, download from
  <https://www.hakenaudio.com/user-guides>).
- Official references:
  - <https://www.hakenaudio.com/editor-and-firmware>
  - <https://www.hakenaudio.com/sound-engine>
  - EaganMatrix guide: <https://drive.google.com/file/d/1cCnvM5pK_Qu9yKvXaBzVR6ik3U3F0iGJ/view>

Important correction already discovered: the “six inverted trapezoids” are
**six vertical Delay-LPF faders**, not six horizontal mode buttons. The
horizontal mode trapezoids can remain as mode selectors, but they are not the
six faders the user meant.

---

## 5. Reference assets in this repo

```text
docs/ui-reference/
  haken-control-panel-2.png
  haken-editor-themes.png
  haken-editor-formula.png
  current-continuum.png
  current-polybrute.png
  hardware/
    mat_product.jpg
    pb_wide.png
    pb_product.jpg
    one_product.jpg
```

High-resolution Haken screenshots were extracted from the official EaganMatrix
User Guide. Use them as visual reference only.

---

## 6. Known gaps / honest status

- Matriarch, PolyBrute and ONE V are **hardware-inspired approximations**, not
  measured 1:1 reproductions.
- PolyBrute has no hardware Morphée/ribbon; the lower-left MorphPad is an M1
  performance control.
- Continuum is a Haken-inspired approximation. It is wide and light-themed, but
  the bottom formula/numeric-field blocks are still represented by knobs and
  faders.
- Continuum CVC output is a MIDI CC proxy (CC 20–23), not DC-coupled CV.
- Convolution uses generated impulse responses, not Haken’s original IR library.
- MicroDelay is a shared/global approximation, not four per-voice taps.
- Windows has only been configured in CI; verify on a real Windows host.
- No hardware A/B/calibration recordings exist.

Do not “fix” these by adding decorative controls that are not wired.

---

## 7. Build, test, install

Configure (only if the build directory is missing or CMake changed):

```bash
cmake -S /Users/faputa/Documents/Codex/2026-10-20/ni/outputs/analog-triad \
      -B /Users/faputa/Documents/Codex/2026-10-20/ni/work/build-triad \
      -G Ninja \
      -DCMAKE_BUILD_TYPE=Release \
      -DTRIAD_JUCE_PATH=/Users/faputa/Documents/Codex/2026-10-20/ni/work/JUCE
```

Build all:

```bash
cmake --build /Users/faputa/Documents/Codex/2026-10-20/ni/work/build-triad -j 2
```

Tests:

```bash
cd /Users/faputa/Documents/Codex/2026-10-20/ni/work/build-triad
ctest --output-on-failure
```

Install apps/plugins:

```bash
cd /Users/faputa/Documents/Codex/2026-10-20/ni/outputs/analog-triad
./scripts/package-macos.sh \
  --build /Users/faputa/Documents/Codex/2026-10-20/ni/work/build-triad \
  --install
```

Validate:

```bash
auval -v aumu Ma01 Tria
auval -v aumu Pb02 Tria
auval -v aumu Mo03 Tria
auval -v aumu Co04 Tria
```

Pluginval (Continuum example):

```bash
/tmp/pluginval/pluginval.app/Contents/MacOS/pluginval \
  --strictness-level 5 --skip-gui-tests \
  --output-dir /tmp/pv-continuum \
  "/Users/faputa/Documents/Codex/2026-10-20/ni/work/build-triad/Continuum_artefacts/Release/VST3/CONTINUUM V.vst3"
```

Screenshot QA (editor snapshot without desktop):

```bash
TRIAD_SNAPSHOT=/tmp/continuum.png \
  "/Users/faputa/Documents/Codex/2026-10-20/ni/work/build-triad/Continuum_artefacts/Release/Standalone/CONTINUUM V.app/Contents/MacOS/CONTINUUM V"
```

Use a fresh filename, wait for the app to create it, then inspect with an image
reader. Do not rely on full-desktop screenshots.

Installed plugin destinations:

```text
~/Library/Audio/Plug-Ins/VST3/<PRODUCT>.vst3
~/Library/Audio/Plug-Ins/Components/<PRODUCT>.component
```

---

## 8. Acceptance checklist

For each analog instrument:

- [ ] Straight-on front panel, correct module order and relative widths.
- [ ] Control centers/radii match the hardware reference closely.
- [ ] Labels are real silkscreen-style labels, not generic parameter names.
- [ ] Knob/fader/jack/switch rendering matches the physical part family.
- [ ] Wood/metal/panel finish has depth, not flat rectangles.
- [ ] No decorative control without a parameter binding.
- [ ] Existing preset names, parameter IDs and host automation still work.
- [ ] Screenshot at a readable zoom, not only fit-to-window.

For Continuum:

- [ ] Matrix is the dominant, wide element.
- [ ] Six vertical inverted trapezoid Delay-LPF faders are present and wired.
- [ ] Formula/numeric-field layout resembles the Haken editor, not generic knobs.
- [ ] Light theme is coherent; labels remain readable.
- [ ] 3D display at top; red/black surface at bottom; physical keys work.
- [ ] Six Recirculator modes, chorus/phaser/echo, delays and convolutions remain.
- [ ] No false claim of exact EaganMatrix reproduction.

---

## 9. Git handoff

Base commit:

```text
f78a71d Add Haken-style six vertical Delay-LPF trapezoid faders
```

Before starting, create a branch:

```bash
git switch -c ui/hardware-redraw
```

Commit UI work in small, reviewable steps. Do not squash everything into one
opaque “UI overhaul” commit.
