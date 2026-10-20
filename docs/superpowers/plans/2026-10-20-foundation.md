# Analog Triad Foundation Implementation Plan

> **For agentic workers:** execute the tasks with the available collaboration subagents and root integration. The optional superpowers execution skills are not installed; use the same task boundaries and independent review directly.

**Goal:** Build and validate the first usable native instrument suite, with truthful coverage tracking toward the approved full-function target.

**Architecture:** A framework-independent DSP library feeds three JUCE plugin targets. Separate engines share bounded primitives and the host adapter; a parameter-driven editor preserves each instrument's module arrangement.

**Tech Stack:** C++20, JUCE 9.0.4, CMake, CTest, Apple Clang, Windows MSVC, VST3 and AUv2.

**Spec:** `docs/superpowers/specs/2026-10-20-analog-triad-design.md`.

## Global Constraints

- Three independent, self-contained audio instruments built from one C++20 repository.
- JUCE is pinned to 9.0.4 (3fb5cbc0340758596dac39f7c8de47d562b4896e).
- Preparation allocates resources; rendering does not allocate, access files, or wait for locks.
- The initial host wrapper applies parameter snapshots per block; do not claim sample-accurate host automation.
- Only platforms actually built and tested are marked verified.

## Task 1: Core contracts and primitives

Files: `src/core/Engine.h`, `src/core/Dsp.h`, `tests/EngineTests.cpp`, `CMakeLists.txt`.

Interfaces: `ModelDefinition`, `SynthEngine`, `StereoSample`, `Oscillator::process`, `Envelope::process`, `LadderFilter::process`, `StateVariableFilter::process`, `StereoDelay::process`.

- [ ] Create explicit parameter ranges and bounded engine snapshots; check ID uniqueness.
- [ ] Add deterministic signal tests: `CHECK(std::isfinite(sample.left)); CHECK(energy > 1.0e-6);` after `prepare(48000, 512)`, `setParameters(defaults)`, `noteOn(60, 0.8f)` and 48000 renders.
- [ ] Implement bandlimited oscillators, envelopes, nonlinear ladder/SVF and stereo delay; rate changes reset bounded state.
- [ ] Run `cmake --build <build> --target triad_tests` then `ctest --test-dir <build> --output-on-failure`.

## Task 2: Separate instruments

Files: `src/engines/Matriarch.*`, `src/engines/PolyBrute.*`, `src/engines/One.*`.

Consumes core contracts. Produces `matriarchDefinition()/makeMatriarch()`, `polyBruteDefinition()/makePolyBrute()`, `oneDefinition()/makeOne()`.

- [ ] Add definitions with stable IDs and distinct hardware groups; provide actionable factory presets.
- [ ] Implement separate topology, voice allocation, modulation and effects for each instrument.
- [ ] Verify `noteOff`, sustain and `allNotesOff`, extreme parameters and deterministic reset at 44100/48000/96000.
- [ ] Record each hardware feature as implemented/tested/partial/not-implemented; do not count metadata as implementation.

## Task 3: Native host adapter and panel

Files: `src/plugin/Processor.*`, `src/ui/Editor.*`, `src/ui/PanelLookAndFeel.*`.

Consumes engine factories and definitions. Produces `juce::AudioProcessor* createPluginFilter()` per target and native editors.

- [ ] Expose typed parameters via APVTS and cache atomic pointers before rendering.
- [ ] Split processing at MIDI sample offsets; process stereo input/output without realtime allocation.
- [ ] Persist all parameters, routing, preset and UI state with a version stamp; reject malformed/oversized state safely.
- [ ] Build physical module layouts, knobs/switches/readouts, keyboard, preset controls and patch interactions. Every attachment uses a definition ID.
- [ ] Add plugin round-trip checks using `getStateInformation`/`setStateInformation`, reopen editor and compare parameter and patch values.

## Task 4: Build, install and inspect

Files: `CMakePresets.json`, `scripts/package-macos.sh`, `.github/workflows/build.yml`, `docs/DEVELOPMENT.md`.

- [ ] Fetch pinned JUCE outside source; configure `-DJUCE_PATH=<dependency> -DCMAKE_BUILD_TYPE=Release`.
- [ ] Build VST3/AU/standalone and run offline tests. Inspect actual bundles, architecture and signatures.
- [ ] Run VST3/AU validators where available, fix crashes and save logs. Inspect screenshots and adjust unreadable/clipped UI.
- [ ] Create a concrete macOS installer and Windows CI packaging; distinguish local verification from unrun CI.

## Task 5: Review and GitHub delivery

Files: `README.md`, `docs/COVERAGE.md`, `docs/VALIDATION.md`, repository commits.

- [ ] Review realtime behavior, state persistence and coverage independently; fix actionable defects.
- [ ] Commit only source/docs/scripts, excluding dependencies/builds/caches and secrets.
- [ ] Push through the available authorized channel to the private target repository; verify remote commit and CI results.
- [ ] Deliver artifact links and a concise statement of implemented functionality, tests and remaining work. Do not label the entire hardware recreation complete unless coverage and validation support it.
