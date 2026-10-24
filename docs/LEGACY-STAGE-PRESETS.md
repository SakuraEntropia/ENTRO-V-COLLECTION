# Legacy repairs and Stage bank — 2026-10-24

The complete pre-update bank contains 938 presets. This pass audits every entry; legacy Continuum audio checks are no longer sampled. It also adds 32 Stage patches per instrument, retaining old names and keeping existing Studio presets.

| Instrument | Previous | Added Stage | Total |
|---|---:|---:|---:|
| MATRIARCH V | 83 | 32 | 115 |
| POLYBRUTE V | 87 | 32 | 119 |
| ONE V | 91 | 32 | 123 |
| CONTINUUM V | 677 | 32 | 709 |

Search `Stage` in BROWSE. The bank extends the eight articulation families with interval changes, oscillator spectra, filter balance, different decays, layering, additional modulation and effects. PolyBrute Stage patches retain independent B snapshots where applicable; One includes octave-layered keys/plucks; Continuum adds inharmonic ratios and a second SG-to-filter route. The 128 Stage auditions are original software patches, not factory imports.

## Repairs traced to measured issues

- Matriarch Rubber Bass, Reese Bass, Acid Bass and Organ used HP/LP series routing. This suppressed fundamentals; the organ's high cutoff removed nearly all its useful spectrum. These presets now use the existing stereo LP mode.
- PolyBrute Metalizer Mono chained a high-pass-like Steiner response into a lower ladder cutoff. Parallel routing restores the ladder foundation while preserving Steiner bite.
- One reference conversion incorrectly mapped the auxiliary envelope into the engine's actual VCA (ENV 3). Hardware filter envelope 1 now maps to both engine filter envelopes 1/2, and hardware VCA envelope 2 maps to engine envelope 3. The Stacked Foundation filter adaptation stays at 180 Hz; the sustained-body regression tests the actual amplitude problem. Studio/Stage One articulation now also writes ENV 3, so pads swell and plucks decay correctly.
- Legacy Continuum bank recipes were substantially quieter than the Studio bank. Post gain is adjusted without altering the independent global OUT control: 12 dB for the legacy recipes, 6 dB for the redesigned winds.
- Several Continuum woodwinds reused one narrow bandpass recipe. Flute, clarinet, oboe and sax now use different oscillator spectra, harmonic ratios, cutoff, resonator frequencies, breath levels and attacks. Fundamental-preserving low-pass routing replaces the shared narrow bandpass. A regression compares four previously matching woodwind outputs.

These repairs address software patch design and level problems. They do not establish physical-instrument or hardware-emulation fidelity.

## Verification

```
triad_render_presets OUTPUT all --audit-only
triad_render_presets OUTPUT stage
```

The full audit uses eight seconds per preset, including arps, PolyBrute A/B and Continuum expression. The Stage bank has 128 distinct rendered WAV payloads, with finite/non-silent/unclipped output. The preset test now renders **every** legacy and new patch rather than sampling the large bank.

## Installation preference

The user requests replacement of installed versions after each completed development batch. After tests and the final build pass, stage the four instruments and run `scripts/install-macos-current.py`. It signs and verifies all 12 bundles, backs up previous installed versions, then replaces VST3/AU/apps and checks executable hashes. DAW/standalone processes must be restarted to load the changed binaries. No user projects or DAW processes are closed automatically.

The corrected One bank was separately re-rendered after the VCA mapping fix (123 patches). Its results replace the earlier One rows in the final combined audit. Previous Studio and new Stage One audition files were updated to match.
