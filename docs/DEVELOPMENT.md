# Development notes

## Build layout

- The repository only contains source, tests and docs.
- JUCE and CMake build trees live outside the repository (`../work/JUCE`, `../work/build-triad`).
- `triad_core` is framework-independent and has no JUCE include path.
- `triad_engines` links `triad_core` and contains the four independent engines, including CONTINUUM V / EIGEN MATRIX.
- `triad_ui` links JUCE modules and the engines; the plugin targets compile the host adapter.

## Real-time rules

- `prepare()` allocates; `render()` and `processMidi()` do not allocate, lock, touch files, or touch the message thread.
- Parameter snapshots are read once per block. Audio never reads `AudioProcessorValueTreeState` directly.
- Patch cable edits update atomics; the engine latches the connection masks at the start of each render call.
- Sequencer pattern edits use atomics; note events are generated into a fixed-size array.
- The state save/load path may allocate and is not called from the audio callback.

## Parameter rules

- IDs are stable, dotted and instrument-prefixed (`mat.`, `poly.`, `one.p1.`).
- Natural units are stored in the definition: Hz, seconds, semitones, choice indices.
- `ParamCurve::Log` gives frequency/time knobs a musical taper; `Discrete` becomes `AudioParameterChoice`.
- Adding a parameter changes `ModelDefinition` order. Engines must resolve ids through the definition, not hard-coded indices.
- All parameters are stored in the APVTS and therefore in project state and host automation.

## State format

`getStateInformation()` writes the APVTS tree plus:

- `instrument` — rejects state written by another plugin.
- `formatVersion` — currently `3`.
- `patchCables` — `from:to;...` for Matriarch.
- `MORPH_B` — PolyBrute's B-side parameter snapshot.
- `SEQUENCER` — banked polyphonic pattern data shared by the arpeggiators.
- `presetName` — last selected preset.

Unknown properties are ignored. A malformed tree, wrong instrument id or unreadable XML leaves the current state untouched.

## Editing the UI

- Each panel is built from `PanelBase`: module frames, knobs, switches, LEDs, jacks and LCD.
- Design coordinates are fixed per instrument; the editor scales the whole panel with an affine transform and letterboxes it, so module positions remain faithful at every window size.
- Bind every control to a definition id. `PanelBase` asserts on missing ids; do not attach decorative controls to nothing.
- Matriarch's jack labels are drawn below the sockets. Cables are drawn in `paintOverChildren` from the host's cable list; the processor owns the authoritative list.

## Adding presets

`src/plugin/Presets.cpp` returns per-instrument factory presets. A preset is a list of `id → natural value` overrides applied after an INIT. Keep names descriptive and avoid claiming hardware factory sound fidelity.

## Tests

```bash
ctest --test-dir ../work/build-triad --output-on-failure
```

`tests/EngineTests.cpp` checks:

- unique parameter/jack ids, defaults inside ranges;
- finite output with energy at 44.1/48/96 kHz;
- note overlap, sustain, pitch bend, pressure and all-notes-off;
- every parameter at its minimum and maximum;
- Matriarch's patching API.

When adding an engine feature, add a deterministic check that fails if the feature is silent or unstable.

## Platform notes

- macOS builds are ad-hoc signed (`CODE_SIGN_IDENTITY = "-"`). No Developer ID is configured; notarisation is out of scope for local builds.
- Windows is configured in CI only. A Windows artifact is not marked verified until it is built and loaded in a DAW on a Windows machine.
- The standalone application is a convenience practice host; the plugin formats are the deliverable.

## Verification discipline

- Only write “tested” for commands actually run, with the date and build directory.
- Distinguish “implemented” (code path exists), “tested” (automated/DAW evidence) and “calibrated” (compared against hardware recordings).
- Do not use marketing copy for the sound; the manuals do not contain full circuit models and this project does not claim equivalence.


## Brand assets

The four instrument marks are defined twice from the same 128-unit grid:

- vector UI: `src/ui/Logo.cpp`;
- design/export source: `scripts/generate-logo-assets.py` and `assets/logos/*.svg`.

Regenerate PNG, `.icns` and `.ico` files on macOS with:

```bash
python3 scripts/generate-logo-assets.py
```

The script uses headless Chrome for transparent SVG rasterisation, `sips` and
`iconutil` for macOS icons, and Pillow for Windows `.ico` files. Generated
assets are committed so CI does not need those tools.

## Install after each validated batch

User preference: replace installed versions after each completed batch. Build and test, stage using `package-macos.sh`, then run `python3 scripts/install-macos-current.py` with permission to update user plugin folders and /Applications. This backs up previous versions and verifies signatures/hashes. Preserve user DAW sessions; never stop them automatically.
