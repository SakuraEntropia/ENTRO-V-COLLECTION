# Clean Pad and Sampled Acoustics Implementation Plan

User approved real recorded samples; realism takes priority over package size.

1. Replace factory Pad high resonance and time-varying filter modulation with static low-resonance filtering. Preserve amplitude envelopes, width and saw oscillators; remove filter cables/morph/modulation routes. Test all final factory pads against this contract, then render them for numerical stability.
2. Add a shared sample playback adapter around existing synth engines, preserving their parameter order and all routing APIs. Append an explicit sample-program parameter. Load licensed sample banks outside the audio callback, preallocate voices, and support velocity, note release, sustain, pitch bend and all-sound-off. Sample programs replace the synth sound, preventing accidental double layers. Old patches default to synthesis. Save/restore uses the existing parameter state mechanism.
3. Bundle CC0 Upright Piano KW and selected CC0 VSCO 2 CE sustained string samples with provenance and checksums. No vendor hardware presets or proprietary samples. Install portable assets with each plugin, so projects do not depend on the development checkout.
4. Expose sampled piano and strings in all four factory browsers. Keep old synth studies honestly labeled; do not imply sample playback recreates the original analog hardware.
5. Exercise sample dynamics, sustain, immediate mute, preset switching and state restoration. Render listening examples. Build macOS bundles, install with backups, validate installed AU/VST3, and push GitHub. Report remaining articulation and subjective listening limitations.
