# Studio bank — 2026-10-24

All four instruments receive 64 additional original patches (256 total), labelled `[Studio]` in the preset browser. Search `Studio` to isolate the new bank. Existing presets and the 22 documented reference studies remain available.

| Instrument | Previous | Studio additions | Total |
|---|---:|---:|---:|
| MATRIARCH V | 19 | 64 | 83 |
| POLYBRUTE V | 23 | 64 | 87 |
| ONE V | 27 | 64 | 91 |
| CONTINUUM V | 613 | 64 | 677 |

Each instrument has eight articulation families with eight deterministic timbral variations per family. Analog instruments: Bass, Lead, Pad, Keys, Pluck, Arp, Drone, FX. Continuum uses Strings in place of Arp. The variations change oscillator spectra/intervals, detuning, filter frequency/resonance, envelope times, modulation and ambience. No random bank generator, copied hardware files, or name-only duplicates are included. These are related sound-design families, not 256 independently recorded acoustic instruments.

- Matriarch: four-oscillator mixtures, mono/paraphonic articulation, stereo filter spacing and delay. Drone/FX patches include actual attenuated modulation cables.
- PolyBrute: oscillator wave/metalizer/pulse/sync variants, parallel filters, aftertouch-to-cutoff and slow modulation. Pad/Drone/FX patches include separately stored B-side values; turn MORPH to hear the second timbre.
- One: three oscillator recipes, mono leads/basses, one unison lead, and two-part layered pads/drones. Arps use the existing shared part-1 arpeggiator. No independent three-part sequencer is implied.
- Continuum: five oscillator/filter paths, harmonic and inharmonic ratios, resonator banks, SG motion and Y/Z expression. Post gain compensates for the quieter matrix routes. These are original EIGEN MATRIX patches, not Haken factory files or verified acoustic replicas.

## Verification

`triad_preset_tests` validates all metadata and all Studio audio paths, including every new Continuum patch rather than sampling the Studio bank. It also checks 64 unique parameter payloads per instrument. Existing large legacy Continuum banks retain sampled audio tests, with complete metadata checks.

```
triad_render_presets OUTPUT_DIRECTORY studio
```

Renders all 256 additions as eight-second 48 kHz / 16-bit stereo WAVs and `audit.csv` (peak, RMS, DC, derivative RMS). PolyBrute sweeps Morph; Continuum sweeps CC74/pressure. Enabled arps now use the shared `PatternSequencer` with event-offset rendering at 110 BPM. This checks engine/sequence output; it does not replace DAW or hardware A/B testing. `reference` (default) renders the 22 Ref studies; `all` renders every factory patch.

Output is not peak-normalised during export. Audio checks reject non-finite samples, silence and clipping. Different sustain lengths and bass/percussive spectra still produce different average loudness. No listening comparison with physical instruments has been completed.

Sources for the preceding hardware-reference work: [PRESET-REFERENCES.md](PRESET-REFERENCES.md). The Studio bank extends that sound-design direction with independent software patches; it does not add new exact factory reconstructions.

## Local results

All 256 full auditions pass finite/non-silent/unclipped checks. SHA-256 comparison finds 256 distinct WAV payloads. Exported measurements are in [STUDIO-AUDIT.csv](STUDIO-AUDIT.csv). Peak maxima: Matriarch 0.458, PolyBrute 0.397, One 0.254, Continuum 0.362. These figures apply to the documented test phrase, not every possible MIDI performance.

All four macOS arm64 VST3 bundles pass pluginval strictness 5 (including its default GUI tests). AU and standalone bundles were rebuilt. New `.pkg` version: 0.2.1; no installation was performed. Cross-platform build for source `f9de6ac`: https://github.com/SakuraEntropia/ENTRO-V-COLLECTION/actions/runs/40090499646 (pending at the time of recording).

The `triad_audition_smoke` regression renders one enabled arp per analog instrument plus an expressive Continuum pad, in addition to the existing engine/preset/UI/host tests.
