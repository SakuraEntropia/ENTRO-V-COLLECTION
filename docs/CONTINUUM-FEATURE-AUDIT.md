# Continuum / EaganMatrix feature audit — 2026-10-22

Baseline: 99e35af. Preserve the other agent's 128-parameter engine, 613 presets, fixed-carrier expression repairs and instrument-named physical-model presets.

| Area | Official reference | Current implementation / gap |
|---|---|---|
| Summation oscillators | Five summation/waveshaping oscillators | Five waveform oscillators with ratio/fine/phase/shape/level. Not the full summation oscillator architecture. |
| Filters | Two-pole LP/HP/BP/shelves/unity BP/notch/all-pass, plus single-pole variants and cascading | Four SVF modes only. Shelves/all-pass/single-pole variants absent. |
| Biquad banks | 8 or 48 filters per bank; bandwidth spread, spectral centre/weighting; modal/granular applications | Two banks of eight with frequency/spread/bandwidth/mix/level/key tracking. No 48-filter selection, bandwidth-spread or spectral-weighting controls. Instrument-named presets use these approximations; naming is not evidence of a complete hardware model. |
| Shape generators | Five SGs with cycle/frequency/trigger and eight shapes | Five SGs and eight shapes exist; explicit cycle/trigger controls absent. |
| Delay | Matrix time-delay modules supporting waveguide/FDN designs | Two global delay banks; not general per-voice matrix waveguides or the hardware's full delay topology. |
| Master processing | Convolution, Recirculator, compressor and control-panel processing | Six Recirculator choices, two generated-IR convolution mixes, chorus/phaser/delay exist. Compressor, Tilt EQ and IR length/tuning/width/selection controls absent. |
| Formulas / matrix | Named formulas and editable transfer functions; assignable Master columns | Current 16×16 matrix, 13 formula options and one shared expression. No complete named-formula/transfer-graph system or six assignable Master columns. |
| Voltage outputs | Eight physical CVC outputs with hardware | Four MIDI CC proxies, not hardware CV. |
| Editor functions | Current 10.81 update dated 19 October 2026; 10.70/10.73 add Master columns, descriptions and Undo | Numeric editing exists. Full editor Undo, formula descriptions, hardware preset-slot management and device communication absent. |

This is a researched gap list, not a claim that these functions have been implemented. The current UI must expose only genuinely wired functionality. Extending to the full EaganMatrix requires separate engine, parameter/state-compatibility and audio tests; adding empty control artwork would misrepresent the instrument.

## Sources

- [Haken Sound Engine](https://www.hakenaudio.com/sound-engine): oscillator, filter, biquad, SG, delay, CVC and master architecture.
- [Haken Editor and Firmware](https://www.hakenaudio.com/editor-and-firmware): current version and Master/editor additions.
- [Official user guides](https://www.hakenaudio.com/user-guides): EaganMatrix and Continuum guides.
- Existing `docs/ui-reference/haken-control-panel-2.png`: master-processing control layout.

## UI detail references

- [Moog One manual](https://api.moogmusic.com/sites/default/files/2019-08/Moog_One_Manual_8_2_19.pdf): each oscillator's OLED shows its composite waveform, rather than an output oscilloscope. Our OLEDs follow the existing engine's angle/shape/pulse mix.
- [PolyBrute manual](https://downloads.arturia.net/products/polybrute/manual/polybrute_Manual_1_0_EN.pdf) and existing straight-on `hardware/pb_wide.png`: wheels in the upper-left performance panel, wooden Morphée block beside the keyboard. The software pad retains its previously established morph/spread/pressure mapping.
