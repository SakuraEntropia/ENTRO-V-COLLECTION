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
| `Matriarch.vst3` | arm64 | ad-hoc / linker-signed |
| `Matriarch.component` | arm64 | ad-hoc |
| `PolyBrute.vst3` | arm64 | ad-hoc |
| `PolyBrute.component` | arm64 | ad-hoc |
| `Moog One.vst3` | arm64 | ad-hoc |
| `Moog One.component` | arm64 | ad-hoc |

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
- finite output with energy > 1e-6 at 44.1, 48 and 96 kHz for all three engines;
- 12-note overlap, sustain pedal, pitch bend, channel pressure, all-notes-off;
- every parameter at minimum and maximum;
- Matriarch patch-cable creation and clearing;
- tuning: note 69 = 440 Hz, note 57 = 220 Hz, note 81 = 880 Hz within ±60 cents using autocorrelation (Matriarch, PolyBrute and Moog One).

`triad_factory_presets` checks that every preset parameter ID exists, values are finite and in range, choice indices are valid, and preset names are unique (4 presets per instrument).

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

Result for all three:

```
* * PASS
AU VALIDATION SUCCEEDED.
```

Warnings emitted by `auval` about floating-point parameters “not retaining default value when set” are value-quantisation warnings; every parameter still reports PASS and no validation step failed. An earlier Moog One failure came from a degenerate `VOICES` choice parameter with a single value; that parameter was removed and replaced by a static “shared 8-voice pool” caption, after which AU validation passed.

## VST3 validation

Tool: pluginval 1.0.4 (`Tracktion/pluginval` release).

```bash
pluginval --strictness-level 5 --output-dir /tmp/pv-mat "…/Matriarch.vst3"
pluginval --strictness-level 5 --output-dir /tmp/pv-pb  "…/PolyBrute.vst3"
pluginval --strictness-level 5 --output-dir /tmp/pv-one "…/Moog One.vst3"
```

Result: `SUCCESS` for all three at strictness level 5. Matriarch was additionally run with GUI tests enabled and passed. PolyBrute and Moog One were run with the default test set (GUI tests included) in the final pass.

## Manual smoke checks

- Standalone applications launch and show the editor:
  - Matriarch window 1602 × 668, keyboard F1–F5.
  - PolyBrute window shows performance, voice, VCO A/B, mixer, Steiner, ladder, envelopes, LFOs, matrix, effects and arpeggiator modules.
  - Moog One window shows the three-part console, per-part VCOs/filters/envelopes/LFOs/matrix/effects and the 61-key keyboard.
- Parameter changes update audio and survive project-style state calls (pluginval state tests).
- The `INIT` button resets all parameters.
- The PolyBrute and Moog One panels were re-laid out from the manual diagrams after the last screenshot pass; the rebuilt binaries compile and pass the engine tests, but the final visual pass on those two panels was interrupted by the machine locking. This is recorded as pending visual review.

Screenshots were captured locally for layout review. They are not committed because they contain unrelated desktop content.

## Not verified

- Loading the plugins inside Logic Pro, Ableton Live, Reaper, Cubase or Studio One. No DAW session was run.
- Windows x64 builds. `.github/workflows/build.yml` is written but has never executed.
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
