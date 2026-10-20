# Panel layout references (MATRIARCH V / POLYBRUTE V / ONE V)

The editor panels are redrawn in code; no photograph or manufacturer artwork is embedded. The module order and the relative positions below come from the official manuals and product photographs.

## Reference sources

| Instrument | Layout reference | Colour/style reference |
|---|---|---|
| Matriarch | [Matriarch manual 2023-01](https://cdn.inmusicbrands.com/Moog/Matriarch/Matriarch_Manual_012023.pdf), panel illustration on manual pages 6–7 (PDF pages 6–7), keyboard/left-hand controller pages 10–11 | Moog product photograph; black panel, white silkscreen, red illuminated buttons, 49-key F1–F5 keybed |
| PolyBrute | [PolyBrute manual 3.0.1](https://www.arturia.com/api/download/products/polybrute/manual/polybrute_Manual_3_0_1_EN.pdf), overview pages 6–7 (PDF pages 13–14), matrix description page 7 | Arturia product photograph; slate-grey panel, wood cheeks and front rail, aluminium envelope/LFO faceplates, blue button LEDs, black knobs with silver rings |
| Moog One | [Moog One manual 2019](https://cdn.inmusicbrands.com/Moog/MoogOne/Moog_One_Manual_8_2_19.pdf), front-panel description pages 9–10 | Moog product photograph; wooden cabinet, three black panel sections with silver trim, silver knobs, red buttons, purple/blue LCD |

The product photographs were used locally as visual reference only and are not committed to this repository or shipped in the plugin.

## Matriarch

One long row, matching the hardware left-to-right order:

```
ARP/SEQ | MODULATION | UTILITIES | OSC 1 | OSC 2 | OSC 3 | OSC 4 |
MIXER | FILTERS | UTILITIES 2 | ENVELOPE GENERATORS | STEREO DELAY | OUTPUT
```

Below the panel: left-hand controller (pitch/mod wheels, transport buttons) and the 49-key F1–F5 keybed. Patch jacks sit in rows at the top of each module; patch cables are drawn over the panel.

Details taken from the manual illustration:

- the four oscillators are one group with a single `SYNC ENABLE` button in the oscillator block;
- VCO 2–4 have a dedicated frequency knob, VCO 1 does not;
- the mixer has the four oscillator levels plus noise and can be overridden per jack;
- the filter block carries the large cutoff knob, spacing, two resonance controls, envelope amount, keyboard tracking and the three-position mode switch;
- the two ADSR columns are labelled `AMPLITUDE` and `FILTER`;
- the delay module has `SYNC/TAP` and `PING PONG` buttons;
- the output block has main volume, velocity, aftertouch, VCA mode and paraphony.

## PolyBrute

Left side, top to bottom, as in the manual overview:

- performance controls at the far left (wheels, octave, Morphée area on hardware);
- voice-management column (`VOICE MODE`, `UNISON`, `GLIDE`, `BEND RANGE`, `SPREAD`, `ANALOG`);
- three-row synthesis block:

```
row 1:  VCO 1      | MIXER  | STEINER FILTER | VCF ENV  | LFO 1
row 2:  VCO 2      | MORPH  | LADDER FILTER  | VCA ENV  | LFO 2
row 3:  FILTER FM  | ROUTING| MOD ENV        | LFO 3    | (spare)
```

Right side, left to right:

- matrix: 8 × 12 grid with LCD above;
- master controls / effects: distortion, chorus, delay, reverb;
- arpeggiator and transport at the far right.

The 61-key C2–C7 keybed sits below; the ribbon/Morphée strip is represented by the performance area and the header controls rather than a decorative non-functional pad.

## Moog One

Three front-panel sections separated by silver trim:

```
LEFT PANEL                 CENTER CONSOLE          RIGHT PANEL
VCO 1 / VCO 2 / VCO 3      screen                  MIXER
LFO 1 / LFO 2              part buttons            STATE VARIABLE FILTER
LFO 3 / LFO 4              part controls           LADDER FILTER
ARP                        master controls         ENVELOPE GENERATORS 1–3
                                                   MOD MATRIX
                                                   EFFECTS
```

Only the synthesis parameters of the currently selected part are shown, with `PART 1` / `PART 2` / `PART 3` buttons at the top of the console. The 61-key C2–C7 keybed and the wooden cabinet frame sit below/around the panel.

## What is still a plugin convention

- The preset selector and save/load controls live in the window header; the hardware has no equivalent project-preset bar.
- Matrix destination and amount rows use combo boxes and knobs instead of a second level of hardware buttons.
- The Morphée touchpad and ribbon controller are not redrawn as non-functional surfaces; the implemented performance controls are the wheels, aftertouch, bend range and Morph A/B.
- Envelopes use vertical faders on PolyBrute and knobs on One/Matriarch to match the visible hardware control type.
