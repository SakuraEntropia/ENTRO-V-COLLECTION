# One listening rework — 2026-10-25

Response to listening feedback describing the existing sounds as cheap electronic tones. This batch adds only three experimental patches: Cedar Saw, Hammer Body and Bow Section, all tagged `[Rework]`. These are synthesis studies, not sampled acoustic instruments or verified hardware emulations. Existing presets remain available.

One previously reset all three oscillator phases to zero at note-on. A regression test with settled parameter smoothing reproduced identical retrigger waveforms. Independent seeded random note-on phases now vary those onsets; this approximates independent starting phases, not continuously simulated free-running oscillators.

Cedar Saw uses two parts, restrained detuning, moderate resonance and darker 24 dB filtering without chorus or delay. Hammer Body separates a short bright attack from a decaying triangle-based body. Bow Section reduces partial layering and uses slower attacks and delayed mild vibrato. No claim of subjective quality follows from these choices.

Nine CTest suites pass. All 238 One presets complete the eight-second numerical render audit without failures. Three new full auditions also pass. These checks establish stability, not good sound.

The local listening folder `outputs/One等响度对照` contains old/new versions rendered with the same MIDI protocol. Each combined file plays eight seconds old, one second silence, then eight seconds new. Measurement uses integrated LUFS; louder versions receive linear attenuation only, with no EQ, compression or loudness boost. Targets: pad -26.56 LUFS, piano -33.80 LUFS, strings -25.48 LUFS. Both phase behavior and preset parameters changed, so this is a combined revision comparison. Human listening approval remains pending.

Factory totals: Matriarch 163, PolyBrute 167, One 238, Continuum 757; combined 1,325.
