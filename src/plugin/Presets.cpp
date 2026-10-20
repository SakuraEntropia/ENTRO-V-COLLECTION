#include "plugin/Presets.h"

namespace triad
{

namespace
{
using Values = std::vector<std::pair<juce::String, float>>;

std::vector<FactoryPreset> matriarchPresets()
{
    return {
        { "Paraphonic Brass",
          Values {
              { "mat.master.volume", 0.72f },
              { "mat.master.paraphony", 2.0f },
              { "mat.vco1.octave", 1.0f }, { "mat.vco1.wave", 1.0f },
              { "mat.vco2.octave", 1.0f }, { "mat.vco2.freq", -0.12f },
              { "mat.vco2.wave", 1.0f },
              { "mat.vco3.octave", 1.0f }, { "mat.vco3.freq", 0.08f },
              { "mat.vco3.wave", 1.0f },
              { "mat.vco4.octave", 0.0f }, { "mat.vco4.freq", -0.05f },
              { "mat.vco4.wave", 2.0f },
              { "mat.mix.osc1", 0.75f }, { "mat.mix.osc2", 0.7f },
              { "mat.mix.osc3", 0.55f }, { "mat.mix.osc4", 0.45f },
              { "mat.flt.mode", 2.0f }, { "mat.flt.cutoff", 1250.0f },
              { "mat.flt.spacing", 0.12f }, { "mat.flt.res1", 0.28f },
              { "mat.flt.res2", 0.24f }, { "mat.flt.envAmt", 0.45f },
              { "mat.flt.kbTrack", 0.45f },
              { "mat.env1.attack", 0.012f }, { "mat.env1.decay", 0.5f },
              { "mat.env1.sustain", 0.85f }, { "mat.env1.release", 0.45f },
              { "mat.env2.attack", 0.025f }, { "mat.env2.decay", 0.8f },
              { "mat.env2.sustain", 0.4f }, { "mat.env2.release", 0.4f },
              { "mat.delay.sync", 0.0f }, { "mat.delay.time", 0.29f },
              { "mat.delay.feedback", 0.24f }, { "mat.delay.mix", 0.16f }
          } },
        { "Drone Cathedral",
          Values {
              { "mat.master.volume", 0.6f },
              { "mat.master.vcaMode", 2.0f },
              { "mat.master.paraphony", 2.0f },
              { "mat.vco1.wave", 2.0f }, { "mat.vco2.wave", 1.0f },
              { "mat.vco2.freq", 0.05f }, { "mat.vco3.wave", 3.0f },
              { "mat.vco3.freq", -7.0f }, { "mat.vco4.wave", 1.0f },
              { "mat.vco4.freq", 7.0f },
              { "mat.mix.osc1", 0.6f }, { "mat.mix.osc2", 0.55f },
              { "mat.mix.osc3", 0.3f }, { "mat.mix.osc4", 0.3f },
              { "mat.mix.noise", 0.04f },
              { "mat.flt.mode", 1.0f }, { "mat.flt.cutoff", 620.0f },
              { "mat.flt.spacing", 0.35f }, { "mat.flt.res1", 0.55f },
              { "mat.flt.res2", 0.35f }, { "mat.flt.envAmt", 0.1f },
              { "mat.lfo.rate", 0.22f }, { "mat.lfo.wave", 0.0f },
              { "mat.lfo.cutoffAmt", 0.32f }, { "mat.lfo.pitchAmt", 0.04f },
              { "mat.env1.attack", 1.2f }, { "mat.env1.decay", 2.0f },
              { "mat.env1.sustain", 1.0f }, { "mat.env1.release", 3.5f },
              { "mat.env2.attack", 2.0f }, { "mat.env2.decay", 2.0f },
              { "mat.env2.sustain", 0.7f }, { "mat.env2.release", 4.0f },
              { "mat.delay.sync", 0.1f }, { "mat.delay.time", 0.55f },
              { "mat.delay.feedback", 0.58f }, { "mat.delay.mix", 0.45f },
              { "mat.delay.pingPong", 1.0f }
          } },
        { "Sync Lead",
          Values {
              { "mat.master.volume", 0.66f },
              { "mat.vco.syncEnable", 1.0f },
              { "mat.vco2.sync", 1.0f }, { "mat.vco2.freq", 5.0f },
              { "mat.vco2.wave", 1.0f },
              { "mat.mix.osc1", 0.85f }, { "mat.mix.osc2", 0.6f },
              { "mat.flt.mode", 0.0f }, { "mat.flt.cutoff", 1800.0f },
              { "mat.flt.spacing", 0.4f }, { "mat.flt.res1", 0.35f },
              { "mat.flt.res2", 0.45f }, { "mat.flt.envAmt", 0.6f },
              { "mat.env1.attack", 0.003f }, { "mat.env1.decay", 0.25f },
              { "mat.env1.sustain", 0.9f }, { "mat.env1.release", 0.2f },
              { "mat.env2.attack", 0.005f }, { "mat.env2.decay", 0.4f },
              { "mat.env2.sustain", 0.25f }, { "mat.env2.release", 0.25f },
              { "mat.delay.mix", 0.22f }, { "mat.delay.time", 0.2f }
          } },
        { "Sequence Pluck",
          Values {
              { "mat.master.volume", 0.7f },
              { "mat.arp.enabled", 1.0f }, { "mat.arp.mode", 0.0f },
              { "mat.arp.rate", 3.0f }, { "mat.arp.octaves", 2.0f },
              { "mat.arp.hold", 0.0f },
              { "mat.vco1.wave", 2.0f }, { "mat.vco2.octave", 0.0f },
              { "mat.vco2.freq", 7.0f },
              { "mat.mix.osc1", 0.8f }, { "mat.mix.osc2", 0.45f },
              { "mat.flt.mode", 2.0f }, { "mat.flt.cutoff", 700.0f },
              { "mat.flt.res1", 0.45f }, { "mat.flt.res2", 0.3f },
              { "mat.flt.envAmt", 0.75f }, { "mat.flt.kbTrack", 0.55f },
              { "mat.env1.attack", 0.002f }, { "mat.env1.decay", 0.18f },
              { "mat.env1.sustain", 0.0f }, { "mat.env1.release", 0.14f },
              { "mat.env2.attack", 0.002f }, { "mat.env2.decay", 0.12f },
              { "mat.env2.sustain", 0.0f }, { "mat.env2.release", 0.12f },
              { "mat.delay.mix", 0.3f }, { "mat.delay.feedback", 0.4f },
              { "mat.delay.pingPong", 1.0f }
          } }
    };
}

std::vector<FactoryPreset> polyBrutePresets()
{
    return {
        { "Palace Brass",
          Values {
              { "poly.voice.mode", 0.0f }, { "poly.voice.spread", 0.35f },
              { "poly.voice.analog", 0.5f },
              { "poly.vcoA.wave", 0.1f }, { "poly.vcoA.pulse", 0.25f },
              { "poly.vcoA.level", 0.72f },
              { "poly.vcoB.wave", 0.1f }, { "poly.vcoB.pulse", 0.2f },
              { "poly.vcoB.level", 0.6f }, { "poly.vcoB.sub", 0.08f },
              { "poly.mixer.noise", 0.02f },
              { "poly.steiner.mode", 0.1f }, { "poly.steiner.cutoff", 1350.0f },
              { "poly.steiner.resonance", 0.24f }, { "poly.steiner.envAmt", 0.45f },
              { "poly.ladder.cutoff", 1900.0f }, { "poly.ladder.drive", 1.3f },
              { "poly.ladder.envAmt", 0.3f },
              { "poly.env1.attack", 0.02f }, { "poly.env1.decay", 0.8f },
              { "poly.env1.sustain", 0.8f }, { "poly.env1.release", 0.4f },
              { "poly.env2.attack", 0.01f }, { "poly.env2.decay", 0.6f },
              { "poly.env2.sustain", 0.5f }, { "poly.env2.release", 0.35f },
              { "poly.env3.attack", 0.1f }, { "poly.env3.decay", 1.0f },
              { "poly.env3.sustain", 0.2f }, { "poly.env3.release", 0.5f },
              { "poly.lfo2.rate", 0.32f }, { "poly.lfo2.pwmAmt", 0.2f },
              { "poly.fx.chorus.rate", 0.35f }, { "poly.fx.chorus.mix", 0.25f },
              { "poly.fx.reverb.size", 0.6f }, { "poly.fx.reverb.mix", 0.2f },
              { "poly.fx.delay.time", 0.35f }, { "poly.fx.delay.feedback", 0.28f },
              { "poly.fx.delay.mix", 0.12f }, { "poly.fx.delay.sync", 1.0f }
          } },
        { "Metalizer Mono",
          Values {
              { "poly.voice.mode", 1.0f },
              { "poly.vcoA.metalizer", 0.65f }, { "poly.vcoA.wave", 0.05f },
              { "poly.vcoA.level", 0.8f },
              { "poly.vcoB.tune", 7.0f }, { "poly.vcoB.level", 0.5f },
              { "poly.steiner.mode", 0.8f }, { "poly.steiner.cutoff", 2400.0f },
              { "poly.steiner.resonance", 0.42f }, { "poly.steiner.envAmt", 0.5f },
              { "poly.ladder.cutoff", 900.0f }, { "poly.ladder.drive", 3.4f },
              { "poly.ladder.envAmt", 0.4f },
              { "poly.env1.attack", 0.003f }, { "poly.env1.decay", 0.3f },
              { "poly.env1.sustain", 0.6f }, { "poly.env1.release", 0.25f },
              { "poly.fx.dist.type", 2.0f }, { "poly.fx.dist.drive", 2.6f },
              { "poly.fx.dist.mix", 0.3f },
              { "poly.fx.delay.time", 0.22f }, { "poly.fx.delay.mix", 0.18f },
              { "poly.fx.delay.pingPong", 1.0f }
          } },
        { "Glass Arp",
          Values {
              { "poly.arp.enabled", 1.0f }, { "poly.arp.mode", 0.0f },
              { "poly.arp.rate", 3.0f }, { "poly.arp.octaves", 1.0f },
              { "poly.voice.spread", 0.4f },
              { "poly.vcoA.wave", 0.8f }, { "poly.vcoA.pulse", 0.2f },
              { "poly.vcoB.octave", 0.0f }, { "poly.vcoB.tune", 12.0f },
              { "poly.steiner.cutoff", 2800.0f }, { "poly.steiner.resonance", 0.35f },
              { "poly.steiner.envAmt", 0.2f },
              { "poly.ladder.cutoff", 3200.0f }, { "poly.ladder.resonance", 0.2f },
              { "poly.env1.attack", 0.002f }, { "poly.env1.decay", 0.16f },
              { "poly.env1.sustain", 0.0f }, { "poly.env1.release", 0.12f },
              { "poly.env2.attack", 0.002f }, { "poly.env2.decay", 0.2f },
              { "poly.env2.sustain", 0.1f }, { "poly.env2.release", 0.15f },
              { "poly.fx.delay.time", 0.28f }, { "poly.fx.delay.feedback", 0.45f },
              { "poly.fx.delay.mix", 0.35f }, { "poly.fx.delay.pingPong", 1.0f },
              { "poly.fx.delay.sync", 1.0f },
              { "poly.fx.reverb.size", 0.55f }, { "poly.fx.reverb.mix", 0.25f }
          } },
        { "Morph Pad",
          Values {
              { "poly.voice.spread", 0.55f }, { "poly.voice.analog", 0.6f },
              { "poly.vcoA.wave", 0.65f }, { "poly.vcoA.pulse", 0.4f },
              { "poly.vcoB.wave", 0.75f }, { "poly.vcoB.sub", 0.2f },
              { "poly.steiner.mode", 0.35f }, { "poly.steiner.cutoff", 900.0f },
              { "poly.steiner.resonance", 0.3f }, { "poly.steiner.envAmt", 0.2f },
              { "poly.ladder.cutoff", 1400.0f }, { "poly.ladder.resonance", 0.18f },
              { "poly.env1.attack", 1.4f }, { "poly.env1.decay", 2.0f },
              { "poly.env1.sustain", 0.9f }, { "poly.env1.release", 3.2f },
              { "poly.env2.attack", 0.8f }, { "poly.env2.decay", 1.5f },
              { "poly.env2.sustain", 0.6f }, { "poly.env2.release", 2.5f },
              { "poly.lfo3.rate", 0.14f }, { "poly.lfo3.cutoffAmt", 0.3f },
              { "poly.fx.chorus.mix", 0.45f }, { "poly.fx.reverb.size", 0.8f },
              { "poly.fx.reverb.mix", 0.4f }
          } }
    };
}

std::vector<FactoryPreset> onePresets()
{
    return {
        { "Poly Brass",
          Values {
              { "one.p1.enabled", 1.0f }, { "one.p1.vco1.waveAngle", 0.2f },
              { "one.p1.vco2.waveAngle", 0.1f }, { "one.p1.vco3.shape", 0.35f },
              { "one.p1.svf.cutoff", 1400.0f }, { "one.p1.svf.envAmt", 0.4f },
              { "one.p1.ladder.cutoff", 2200.0f }, { "one.p1.ladder.envAmt", 0.25f },
              { "one.p1.env1.attack", 0.012f }, { "one.p1.env1.decay", 0.6f },
              { "one.p1.env1.sustain", 0.6f }, { "one.p1.env1.release", 0.4f },
              { "one.p1.env2.attack", 0.01f }, { "one.p1.env2.decay", 0.5f },
              { "one.p1.env2.sustain", 0.5f }, { "one.p1.env2.release", 0.35f },
              { "one.p1.fx.chorus.mix", 0.2f }, { "one.p1.fx.reverb.send", 0.2f },
              { "one.master.reverbMix", 0.18f }
          } },
        { "Three-Zone Split",
          Values {
              { "one.p1.enabled", 1.0f }, { "one.p1.keyboardLow", 0.0f },
              { "one.p1.keyboardHigh", 47.0f },
              { "one.p1.vco1.octave", 0.0f },
              { "one.p1.svf.cutoff", 700.0f }, { "one.p1.ladder.cutoff", 900.0f },
              { "one.p2.enabled", 1.0f }, { "one.p2.keyboardLow", 48.0f },
              { "one.p2.keyboardHigh", 71.0f },
              { "one.p2.svf.cutoff", 1600.0f },
              { "one.p2.vco2.tune", 7.0f },
              { "one.p3.enabled", 1.0f }, { "one.p3.keyboardLow", 72.0f },
              { "one.p3.keyboardHigh", 127.0f },
              { "one.p3.vco1.octave", 2.0f },
              { "one.p3.svf.cutoff", 2600.0f },
              { "one.p3.fx.delay.mix", 0.25f },
              { "one.master.reverbMix", 0.2f }
          } },
        { "Glass Arp",
          Values {
              { "one.p1.arp.enabled", 1.0f }, { "one.p1.arp.mode", 0.0f },
              { "one.p1.arp.rate", 3.0f }, { "one.p1.arp.octaves", 1.0f },
              { "one.p1.env1.attack", 0.002f }, { "one.p1.env1.decay", 0.18f },
              { "one.p1.env1.sustain", 0.0f }, { "one.p1.env1.release", 0.14f },
              { "one.p1.env2.attack", 0.002f }, { "one.p1.env2.decay", 0.22f },
              { "one.p1.env2.sustain", 0.1f }, { "one.p1.env2.release", 0.2f },
              { "one.p1.env3.attack", 0.002f }, { "one.p1.env3.decay", 0.2f },
              { "one.p1.env3.sustain", 0.0f }, { "one.p1.env3.release", 0.14f },
              { "one.p1.vco1.waveAngle", 0.7f }, { "one.p1.vco3.tune", 12.0f },
              { "one.p1.svf.cutoff", 2800.0f }, { "one.p1.svf.resonance", 0.3f },
              { "one.p1.fx.delay.time", 0.28f }, { "one.p1.fx.delay.feedback", 0.45f },
              { "one.p1.fx.delay.mix", 0.3f },
              { "one.master.reverbMix", 0.22f }
          } },
        { "Big Pad",
          Values {
              { "one.p1.enabled", 1.0f },
              { "one.p1.vco1.waveAngle", 0.4f }, { "one.p1.vco2.waveAngle", 0.55f },
              { "one.p1.vco3.tune", -12.0f },
              { "one.p1.env1.attack", 2.2f }, { "one.p1.env1.decay", 3.0f },
              { "one.p1.env1.sustain", 0.8f }, { "one.p1.env1.release", 4.0f },
              { "one.p1.env2.attack", 1.0f }, { "one.p1.env2.decay", 2.0f },
              { "one.p1.env2.sustain", 0.7f }, { "one.p1.env2.release", 3.0f },
              { "one.p1.env3.attack", 1.2f }, { "one.p1.env3.decay", 2.0f },
              { "one.p1.env3.sustain", 0.85f }, { "one.p1.env3.release", 4.5f },
              { "one.p1.svf.cutoff", 900.0f }, { "one.p1.ladder.cutoff", 1200.0f },
              { "one.p1.lfo3.rate", 0.12f }, { "one.p1.lfo3.cutoffAmt", 0.25f },
              { "one.p1.fx.chorus.depth", 0.5f }, { "one.p1.fx.chorus.mix", 0.4f },
              { "one.p1.fx.reverb.send", 0.5f },
              { "one.master.reverbSize", 0.8f }, { "one.master.reverbMix", 0.4f }
          } }
    };
}
} // namespace

std::vector<FactoryPreset> factoryPresetsFor (const std::string& instrumentId)
{
    if (instrumentId == "matriarch")
        return matriarchPresets();

    if (instrumentId == "polybrute")
        return polyBrutePresets();

    if (instrumentId == "moogone")
        return onePresets();

    return {};
}

} // namespace triad
