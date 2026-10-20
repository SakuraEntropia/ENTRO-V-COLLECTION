# Validation log

## Environment

- Date: 2026-10-20
- Machine: Apple Silicon (arm64), macOS 27.2 (26B5101f)
- Compiler: Apple Clang 21.0.0
- CMake: 4.4.3, Ninja
- JUCE: 9.0.4, commit `3fb5cbc0340758596dac39f7c8de47d562b4896e`
- Build tree: `/Users/faputa/Documents/Codex/2026-10-20/ni/work/build-triad` (outside the repository)
- Source tree: `/Users/faputa/Documents/Codex/2026-10-20/ni/outputs/analog-triad`

## Build

```bash
cmake -S . -B ../work/build-triad -G Ninja \
      -DCMAKE_BUILD_TYPE=Release \
      -DTRIAD_JUCE_PATH=../work/JUCE
cmake --build ../work/build-triad -j 8
```

Result: success for `Matriarch`, `PolyBrute`, `MoogOne` in VST3, AU and Standalone formats, plus `triad_tests` and `triad_preset_tests`.

Artifacts inspected:

| Bundle | Architecture | Signature |
|---|---|---|
| `MATRIARCH V.vst3` | arm64 | ad-hoc / linker-signed |
| `MATRIARCH V.component` | arm64 | ad-hoc |
| `POLYBRUTE V.vst3` | arm64 | ad-hoc |
| `POLYBRUTE V.component` | arm64 | ad-hoc |
| `ONE V.vst3` | arm64 | ad-hoc |
| `ONE V.component` | arm64 | ad-hoc |

No Developer ID identity is configured; hardened runtime and notarisation were not attempted.

## Automated tests

```bash
ctest --test-dir ../work/build-triad --output-on-failure
```

```
Start 1: triad_engine_contracts ... Passed
Start 2: triad_factory_presets ... Passed
100% tests passed, 0 tests failed out of 2
```

`triad_engine_contracts` covers:

- unique parameter IDs, unique patch-point IDs, defaults inside ranges;
- finite output with energy > 1e-6 at 44.1, 48 and 96 kHz for all four engines;
- 12-note overlap, sustain pedal, pitch bend, channel pressure, all-notes-off;
- every parameter at minimum and maximum;
- Matriarch patch-cable creation and clearing;
- tuning: note 69 = 440 Hz, note 57 = 220 Hz, note 81 = 880 Hz within ±60 cents using autocorrelation (Matriarch, PolyBrute and Moog One).

`triad_factory_presets` checks that every preset parameter ID exists, values are finite and in range, choice indices are valid, preset names are unique, categories are known and grouped, and each instrument has at least 12 presets including a Reese bass (15 / 17 / 15 presets for MATRIARCH V / POLYBRUTE V / ONE V).

### Pitch fix recorded during validation

The first Matriarch build computed frequency from semitone offsets as if they were MIDI note numbers (`noteToHz(note - 60)`), producing a pitch roughly five octaves too low and a keyboard that did not match the played note. It now uses `noteToHz(60 + semitones)`, and the on-screen range matches the hardware F1–F5 49-key layout. The regression is covered by the tuning test.

## CPU measurement (offline benchmark)

Rendered 10 seconds of audio per instrument at 256-sample blocks on the Apple Silicon host (single process, default patches, notes held across the case):

| Instrument | Voices | 48 kHz | 96 kHz |
|---|---:|---:|---:|
| Matriarch | 4 (paraphonic) | 0.041 s → 242× realtime | 0.080 s → 125× realtime |
| PolyBrute | 6 | 0.289 s → 35× realtime | 0.600 s → 17× realtime |
| Moog One | 8 | 0.355 s → 28× realtime | 0.730 s → 14× realtime |

This is a synthetic offline benchmark, not a DAW CPU-meter reading. Host buffer size, effects, automation and multiple instances will change the result.

## AU validation

Installed to `~/Library/Audio/Plug-Ins/Components`, then:

```bash
auval -v aumu Ma01 Tria   # Matriarch
auval -v aumu Pb02 Tria   # PolyBrute
auval -v aumu Mo03 Tria   # Moog One
```

Result for all four:

```
* * PASS
AU VALIDATION SUCCEEDED.
```

Warnings emitted by `auval` about floating-point parameters “not retaining default value when set” are value-quantisation warnings; every parameter still reports PASS and no validation step failed. An earlier Moog One failure came from a degenerate `VOICES` choice parameter with a single value; that parameter was removed and replaced by a static “shared 8-voice pool” caption, after which AU validation passed.

## VST3 validation

Tool: pluginval 1.0.4 (`Tracktion/pluginval` release).

```bash
pluginval --strictness-level 5 --output-dir /tmp/pv-mat "…/MATRIARCH V.vst3"
pluginval --strictness-level 5 --output-dir /tmp/pv-pb  "…/POLYBRUTE V.vst3"
pluginval --strictness-level 5 --output-dir /tmp/pv-one "…/ONE V.vst3"
```

Result: `SUCCESS` for all four at strictness level 5. Matriarch was additionally run with GUI tests enabled and passed. PolyBrute, ONE V and CONTINUUM V were run with the default test set (GUI tests included) in the final pass.

## Manual smoke checks

- Standalone applications launch and show the editor:
  - Matriarch window 1602 × 668, keyboard F1–F5.
  - PolyBrute window shows performance, voice, VCO A/B, mixer, Steiner, ladder, envelopes, LFOs, matrix, effects and arpeggiator modules.
  - Moog One window shows the three-part console, per-part VCOs/filters/envelopes/LFOs/matrix/effects and the 61-key keyboard.
- Parameter changes update audio and survive project-style state calls (pluginval state tests).
- The `INIT` button resets all parameters.
- CONTINUUM V launches with the surface and EIGEN MATRIX; its preset browser shows the Bass/Lead/Brass/Pad/Surface/Texture/Strings/FX bank.
- `BROWSE` opens the searchable preset browser; the category filter, text search, list, detail pane, Load and Escape/Close paths were exercised in the standalone build.
- The PolyBrute and ONE V panels were re-laid out from the manual diagrams and visually checked after the last build; both run with zero overlapping control bounds in the layout dump.
- The header and panel-footer instrument marks render from the same vector grid as the SVG assets. Standalone bundles carry a real `AppIcon.icns` (443,685 / 473,960 / 501,764 bytes), not JUCE's empty 8-byte placeholder; `MATRIARCH V`, `POLYBRUTE V` and `ONE V` all resolve their icon in `Info.plist`. JUCE `ICON_BIG` is fed a 1024 PNG and generates the `.icns`/`.ico` itself.
- The legacy names `Matriarch.app`, `PolyBrute.app` and `Moog One.app` (and their old VST3/AU bundles) were removed from `/Applications`, `dist`, build artefacts and the user plug-in folders.
- CONTINUUM V's surface uses the original red/black stripe pattern; the EIGEN MATRIX preset bank exposes the original Continuum two-letter categories and 22 original presets, including the requested `Kevin Penkin Style Eharp`. No Haken factory preset data is embedded.
- The `BROWSE` browser opens with category headings (`Bass`, `Lead`, `Brass`, `Pad`, `Pluck`, `Keys`, `Organ`, `Arp`, `FX`) and prefixed names such as `BA - Reese Bass`.

Screenshots were captured locally for layout review. They are not committed because they contain unrelated desktop content.

## Not verified

- Loading the plugins inside Logic Pro, Ableton Live, Reaper, Cubase or Studio One. No DAW session was run.
- Windows x64 builds have been produced by GitHub Actions, but no Windows machine/DAW was available for a manual smoke test.
- Universal (arm64 + x86_64) builds.
- Hardware A/B: no recordings or measurements of a real Matriarch, PolyBrute or Moog One.
- Code signing and notarisation.
- Long-duration (hours) stability, multiple instances in one project, and offline bounce in a DAW.
- Real MIDI controller mapping (only host/on-screen MIDI was exercised).

These are gaps, not passes. The coverage table marks the corresponding features accordingly.

## Reproduce

```bash
cd /Users/faputa/Documents/Codex/2026-10-20/ni/outputs/analog-triad
cmake -S . -B ../work/build-triad -G Ninja -DCMAKE_BUILD_TYPE=Release \
      -DTRIAD_JUCE_PATH=../work/JUCE
cmake --build ../work/build-triad -j 8
ctest --test-dir ../work/build-triad --output-on-failure
./scripts/package-macos.sh --build ../work/build-triad
```
