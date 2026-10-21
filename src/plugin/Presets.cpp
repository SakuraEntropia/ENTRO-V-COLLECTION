#include "plugin/Presets.h"

#include <algorithm>
#include <set>

namespace triad
{

namespace
{
using Values = std::vector<std::pair<juce::String, float>>;

std::vector<FactoryPreset> sortedPresets (std::vector<FactoryPreset> presets)
{
    const juce::StringArray order { "Bass", "Lead", "Brass", "Pad", "Strings", "Winds",
                                     "Vocal", "Keyboard", "Classic", "Other", "Percussion",
                                     "Tuned Percussion", "Processor", "Drone", "Midi",
                                     "CVC", "User", "Surface", "Texture", "Pluck", "Keys", "Organ", "Arp",
                                     "Sequence", "Split", "Utility", "FX" };

    std::stable_sort (presets.begin(), presets.end(),
                      [&order] (const FactoryPreset& a, const FactoryPreset& b)
                      {
                          return order.indexOf (a.category) < order.indexOf (b.category);
                      });

    return presets;
}

std::vector<FactoryPreset> matriarchPresets()
{
    return sortedPresets ({
        { "BA - Sub Bass", "Bass",
          Values {
              { "mat.master.volume", 0.78f }, { "mat.master.vcaMode", 0.0f },
              { "mat.master.paraphony", 0.0f }, { "mat.output.velocity", 0.4f },
              { "mat.vco1.octave", 0.0f }, { "mat.vco1.wave", 2.0f },
              { "mat.vco2.octave", 0.0f }, { "mat.vco2.freq", 0.0f },
              { "mat.vco2.wave", 1.0f },
              { "mat.mix.osc1", 0.85f }, { "mat.mix.osc2", 0.45f },
              { "mat.mix.noise", 0.01f },
              { "mat.flt.mode", 0.0f }, { "mat.flt.cutoff", 260.0f },
              { "mat.flt.spacing", -0.2f }, { "mat.flt.res1", 0.25f },
              { "mat.flt.res2", 0.3f }, { "mat.flt.envAmt", 0.35f },
              { "mat.flt.kbTrack", 0.4f },
              { "mat.env1.attack", 0.002f }, { "mat.env1.decay", 0.28f },
              { "mat.env1.sustain", 0.55f }, { "mat.env1.release", 0.18f },
              { "mat.env2.attack", 0.002f }, { "mat.env2.decay", 0.22f },
              { "mat.env2.sustain", 0.15f }, { "mat.env2.release", 0.18f },
              { "mat.delay.mix", 0.04f }
          } },
        { "BA - Rubber Bass", "Bass",
          Values {
              { "mat.master.volume", 0.8f }, { "mat.master.paraphony", 0.0f },
              { "mat.vco1.octave", 0.0f }, { "mat.vco1.wave", 1.0f },
              { "mat.vco2.octave", 0.0f }, { "mat.vco2.freq", -0.1f },
              { "mat.vco2.wave", 1.0f },
              { "mat.mix.osc1", 0.8f }, { "mat.mix.osc2", 0.6f },
              { "mat.flt.mode", 0.0f }, { "mat.flt.cutoff", 420.0f },
              { "mat.flt.spacing", 0.3f }, { "mat.flt.res1", 0.35f },
              { "mat.flt.res2", 0.45f }, { "mat.flt.envAmt", 0.55f },
              { "mat.flt.kbTrack", 0.5f },
              { "mat.env1.attack", 0.003f }, { "mat.env1.decay", 0.35f },
              { "mat.env1.sustain", 0.7f }, { "mat.env1.release", 0.25f },
              { "mat.env2.attack", 0.003f }, { "mat.env2.decay", 0.3f },
              { "mat.env2.sustain", 0.1f }, { "mat.env2.release", 0.2f },
              { "mat.delay.mix", 0.06f }
          } },
        { "LD - Sync Lead", "Lead",
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
        { "LD - Solo Saw", "Lead",
          Values {
              { "mat.master.volume", 0.68f }, { "mat.master.paraphony", 0.0f },
              { "mat.vco1.wave", 1.0f }, { "mat.vco2.octave", 1.0f },
              { "mat.vco2.freq", 7.0f }, { "mat.vco2.wave", 1.0f },
              { "mat.mix.osc1", 0.8f }, { "mat.mix.osc2", 0.5f },
              { "mat.flt.mode", 2.0f }, { "mat.flt.cutoff", 2200.0f },
              { "mat.flt.spacing", 0.15f }, { "mat.flt.res1", 0.3f },
              { "mat.flt.res2", 0.25f }, { "mat.flt.envAmt", 0.3f },
              { "mat.env1.attack", 0.01f }, { "mat.env1.decay", 0.5f },
              { "mat.env1.sustain", 0.9f }, { "mat.env1.release", 0.3f },
              { "mat.env2.attack", 0.01f }, { "mat.env2.decay", 0.6f },
              { "mat.env2.sustain", 0.4f }, { "mat.env2.release", 0.4f },
              { "mat.delay.mix", 0.2f }, { "mat.delay.feedback", 0.28f }
          } },
        { "BR - Paraphonic Brass", "Brass",
          Values {
              { "mat.master.volume", 0.72f }, { "mat.master.paraphony", 2.0f },
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
        { "PD - Drone Cathedral", "Pad",
          Values {
              { "mat.master.volume", 0.6f }, { "mat.master.vcaMode", 2.0f },
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
        { "STR - Slow Strings", "Strings",
          Values {
              { "mat.master.volume", 0.68f }, { "mat.master.paraphony", 2.0f },
              { "mat.vco1.wave", 1.0f }, { "mat.vco2.octave", 1.0f },
              { "mat.vco2.freq", 0.05f }, { "mat.vco2.wave", 1.0f },
              { "mat.vco3.octave", 0.0f }, { "mat.vco3.freq", -0.1f },
              { "mat.vco3.wave", 1.0f }, { "mat.vco4.octave", 0.0f },
              { "mat.vco4.freq", 0.1f }, { "mat.vco4.wave", 1.0f },
              { "mat.mix.osc1", 0.5f }, { "mat.mix.osc2", 0.5f },
              { "mat.mix.osc3", 0.3f }, { "mat.mix.osc4", 0.3f },
              { "mat.mix.noise", 0.02f },
              { "mat.flt.mode", 1.0f }, { "mat.flt.cutoff", 700.0f },
              { "mat.flt.spacing", 0.2f }, { "mat.flt.res1", 0.2f },
              { "mat.flt.res2", 0.25f }, { "mat.flt.envAmt", 0.25f },
              { "mat.env1.attack", 1.2f }, { "mat.env1.decay", 1.5f },
              { "mat.env1.sustain", 0.95f }, { "mat.env1.release", 2.5f },
              { "mat.env2.attack", 0.8f }, { "mat.env2.decay", 1.5f },
              { "mat.env2.sustain", 0.6f }, { "mat.env2.release", 2.0f },
              { "mat.lfo.rate", 0.18f }, { "mat.lfo.cutoffAmt", 0.25f },
              { "mat.delay.mix", 0.3f }, { "mat.delay.feedback", 0.4f },
              { "mat.delay.pingPong", 1.0f }
          } },
        { "SEQ - Sequence Pluck", "Sequence",
          Values {
              { "mat.master.volume", 0.7f },
              { "mat.arp.enabled", 1.0f }, { "mat.arp.mode", 0.0f },
              { "mat.arp.rate", 3.0f }, { "mat.arp.octaves", 1.0f },
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
          } },
        { "FX - Space Drone", "FX",
          Values {
              { "mat.master.volume", 0.55f }, { "mat.master.vcaMode", 2.0f },
              { "mat.master.paraphony", 2.0f },
              { "mat.vco1.wave", 2.0f }, { "mat.vco2.freq", 0.07f },
              { "mat.vco2.wave", 1.0f }, { "mat.vco3.freq", -0.12f },
              { "mat.vco3.wave", 3.0f }, { "mat.vco4.freq", 0.12f },
              { "mat.vco4.wave", 1.0f },
              { "mat.mix.osc1", 0.55f }, { "mat.mix.osc2", 0.5f },
              { "mat.mix.osc3", 0.35f }, { "mat.mix.osc4", 0.35f },
              { "mat.flt.mode", 1.0f }, { "mat.flt.cutoff", 520.0f },
              { "mat.flt.spacing", 0.5f }, { "mat.flt.res1", 0.65f },
              { "mat.flt.res2", 0.4f }, { "mat.flt.envAmt", 0.1f },
              { "mat.env1.attack", 2.0f }, { "mat.env1.sustain", 1.0f },
              { "mat.env1.release", 4.0f },
              { "mat.lfo.rate", 0.08f }, { "mat.lfo.wave", 4.0f },
              { "mat.lfo.cutoffAmt", 0.4f },
              { "mat.delay.time", 0.5f }, { "mat.delay.feedback", 0.62f },
              { "mat.delay.mix", 0.5f }, { "mat.delay.pingPong", 1.0f }
          } },
        { "BA - Reese Bass", "Bass",
          Values {
              { "mat.master.volume", 0.7f }, { "mat.master.paraphony", 0.0f },
              { "mat.vco1.octave", 1.0f }, { "mat.vco1.wave", 1.0f },
              { "mat.vco2.octave", 1.0f }, { "mat.vco2.freq", 0.12f },
              { "mat.vco2.wave", 1.0f },
              { "mat.vco3.octave", 1.0f }, { "mat.vco3.freq", -0.1f },
              { "mat.vco3.wave", 1.0f },
              { "mat.vco4.octave", 0.0f }, { "mat.vco4.freq", 0.08f },
              { "mat.vco4.wave", 1.0f },
              { "mat.mix.osc1", 0.7f }, { "mat.mix.osc2", 0.65f },
              { "mat.mix.osc3", 0.45f }, { "mat.mix.osc4", 0.4f },
              { "mat.flt.mode", 0.0f }, { "mat.flt.cutoff", 420.0f },
              { "mat.flt.spacing", 0.15f }, { "mat.flt.res1", 0.3f },
              { "mat.flt.res2", 0.35f }, { "mat.flt.envAmt", 0.25f },
              { "mat.flt.kbTrack", 0.2f },
              { "mat.env1.attack", 0.005f }, { "mat.env1.decay", 0.5f },
              { "mat.env1.sustain", 0.95f }, { "mat.env1.release", 0.3f },
              { "mat.env2.attack", 0.005f }, { "mat.env2.decay", 0.35f },
              { "mat.env2.sustain", 0.4f }, { "mat.env2.release", 0.3f },
              { "mat.lfo.rate", 0.12f }, { "mat.lfo.cutoffAmt", 0.06f },
              { "mat.delay.mix", 0.05f }
          } },
        { "BA - Acid Bass", "Bass",
          Values {
              { "mat.master.volume", 0.65f }, { "mat.master.paraphony", 0.0f },
              { "mat.vco1.octave", 1.0f }, { "mat.vco1.wave", 1.0f },
              { "mat.vco2.octave", 1.0f }, { "mat.vco2.freq", 0.0f },
              { "mat.vco2.wave", 2.0f },
              { "mat.mix.osc1", 0.8f }, { "mat.mix.osc2", 0.5f },
              { "mat.flt.mode", 0.0f }, { "mat.flt.cutoff", 450.0f },
              { "mat.flt.spacing", 0.1f }, { "mat.flt.res1", 0.78f },
              { "mat.flt.res2", 0.4f }, { "mat.flt.envAmt", 0.85f },
              { "mat.flt.kbTrack", 0.7f },
              { "mat.env1.attack", 0.002f }, { "mat.env1.decay", 0.6f },
              { "mat.env1.sustain", 0.9f }, { "mat.env1.release", 0.2f },
              { "mat.env2.attack", 0.002f }, { "mat.env2.decay", 0.28f },
              { "mat.env2.sustain", 0.0f }, { "mat.env2.release", 0.18f },
              { "mat.delay.mix", 0.1f }
          } },
        { "LD - Supersaw Lead", "Lead",
          Values {
              { "mat.master.volume", 0.65f }, { "mat.master.paraphony", 2.0f },
              { "mat.vco1.octave", 1.0f }, { "mat.vco1.wave", 1.0f },
              { "mat.vco2.octave", 1.0f }, { "mat.vco2.freq", 0.12f },
              { "mat.vco2.wave", 1.0f },
              { "mat.vco3.octave", 1.0f }, { "mat.vco3.freq", -0.1f },
              { "mat.vco3.wave", 1.0f },
              { "mat.vco4.octave", 1.0f }, { "mat.vco4.freq", 0.18f },
              { "mat.vco4.wave", 1.0f },
              { "mat.mix.osc1", 0.5f }, { "mat.mix.osc2", 0.5f },
              { "mat.mix.osc3", 0.5f }, { "mat.mix.osc4", 0.5f },
              { "mat.flt.mode", 2.0f }, { "mat.flt.cutoff", 3800.0f },
              { "mat.flt.spacing", 0.1f }, { "mat.flt.res1", 0.2f },
              { "mat.flt.res2", 0.2f }, { "mat.flt.envAmt", 0.2f },
              { "mat.env1.attack", 0.01f }, { "mat.env1.decay", 0.6f },
              { "mat.env1.sustain", 0.9f }, { "mat.env1.release", 0.45f },
              { "mat.env2.attack", 0.01f }, { "mat.env2.decay", 0.8f },
              { "mat.env2.sustain", 0.6f }, { "mat.env2.release", 0.5f },
              { "mat.lfo.rate", 0.2f }, { "mat.lfo.pitchAmt", 0.02f },
              { "mat.delay.sync", 1.0f }, { "mat.delay.mix", 0.25f },
              { "mat.delay.feedback", 0.35f }, { "mat.delay.pingPong", 1.0f }
          } },
        { "PD - Warm Analog Pad", "Pad",
          Values {
              { "mat.master.volume", 0.6f }, { "mat.master.paraphony", 2.0f },
              { "mat.vco1.wave", 0.0f }, { "mat.vco2.freq", 0.08f },
              { "mat.vco2.wave", 1.0f }, { "mat.vco3.octave", 0.0f },
              { "mat.vco3.freq", -0.12f }, { "mat.vco3.wave", 1.0f },
              { "mat.vco4.freq", 0.15f }, { "mat.vco4.wave", 0.0f },
              { "mat.mix.osc1", 0.5f }, { "mat.mix.osc2", 0.5f },
              { "mat.mix.osc3", 0.35f }, { "mat.mix.osc4", 0.35f },
              { "mat.flt.mode", 1.0f }, { "mat.flt.cutoff", 800.0f },
              { "mat.flt.spacing", 0.2f }, { "mat.flt.res1", 0.2f },
              { "mat.flt.res2", 0.25f }, { "mat.flt.envAmt", 0.2f },
              { "mat.env1.attack", 1.5f }, { "mat.env1.decay", 2.0f },
              { "mat.env1.sustain", 0.9f }, { "mat.env1.release", 3.0f },
              { "mat.env2.attack", 1.0f }, { "mat.env2.decay", 2.0f },
              { "mat.env2.sustain", 0.6f }, { "mat.env2.release", 2.5f },
              { "mat.lfo.rate", 0.15f }, { "mat.lfo.cutoffAmt", 0.2f },
              { "mat.delay.sync", 1.0f }, { "mat.delay.mix", 0.35f },
              { "mat.delay.feedback", 0.4f }
          } },
                                        { "OR - Organ", "Organ",
          Values {
              { "mat.master.volume", 0.62f }, { "mat.master.paraphony", 2.0f },
              { "mat.output.velocity", 0.1f },
              { "mat.vco1.octave", 1.0f }, { "mat.vco1.wave", 1.0f },
              { "mat.vco2.octave", 2.0f }, { "mat.vco2.freq", 0.0f },
              { "mat.vco2.wave", 1.0f },
              { "mat.vco3.octave", 0.0f }, { "mat.vco3.freq", 7.0f },
              { "mat.vco3.wave", 0.0f },
              { "mat.vco4.octave", 3.0f }, { "mat.vco4.freq", 0.0f },
              { "mat.vco4.wave", 1.0f },
              { "mat.mix.osc1", 0.8f }, { "mat.mix.osc2", 0.5f },
              { "mat.mix.osc3", 0.25f }, { "mat.mix.osc4", 0.25f },
              { "mat.mix.noise", 0.0f },
              { "mat.flt.mode", 0.0f }, { "mat.flt.cutoff", 9000.0f },
              { "mat.flt.res1", 0.02f }, { "mat.flt.res2", 0.02f },
              { "mat.flt.envAmt", 0.0f }, { "mat.flt.kbTrack", 0.0f },
              { "mat.env1.attack", 0.003f }, { "mat.env1.decay", 0.1f },
              { "mat.env1.sustain", 1.0f }, { "mat.env1.release", 0.1f },
              { "mat.env2.attack", 0.003f }, { "mat.env2.decay", 0.1f },
              { "mat.env2.sustain", 1.0f }, { "mat.env2.release", 0.1f },
              { "mat.lfo.rate", 5.5f }, { "mat.lfo.pitchAmt", 0.008f },
              { "mat.lfo.cutoffAmt", 0.0f },
              { "mat.delay.time", 0.28f }, { "mat.delay.mix", 0.06f },
              { "mat.delay.feedback", 0.15f }
          } },
        { "PL - Analog Pluck", "Pluck",
          Values {
              { "mat.master.volume", 0.65f }, { "mat.master.paraphony", 2.0f },
              { "mat.vco1.wave", 1.0f }, { "mat.vco2.freq", 7.0f },
              { "mat.vco2.wave", 2.0f }, { "mat.vco3.freq", -0.1f },
              { "mat.vco3.wave", 1.0f },
              { "mat.mix.osc1", 0.7f }, { "mat.mix.osc2", 0.4f },
              { "mat.mix.osc3", 0.3f },
              { "mat.flt.mode", 2.0f }, { "mat.flt.cutoff", 1400.0f },
              { "mat.flt.res1", 0.35f }, { "mat.flt.res2", 0.3f },
              { "mat.flt.envAmt", 0.7f }, { "mat.flt.kbTrack", 0.4f },
              { "mat.env1.attack", 0.002f }, { "mat.env1.decay", 0.2f },
              { "mat.env1.sustain", 0.0f }, { "mat.env1.release", 0.15f },
              { "mat.env2.attack", 0.002f }, { "mat.env2.decay", 0.16f },
              { "mat.env2.sustain", 0.0f }, { "mat.env2.release", 0.12f },
              { "mat.delay.sync", 1.0f }, { "mat.delay.mix", 0.25f },
              { "mat.delay.feedback", 0.35f }, { "mat.delay.pingPong", 1.0f }
          } }
    });
}

std::vector<FactoryPreset> polyBrutePresets()
{
    return sortedPresets ({
        { "BA - Metalizer Bass", "Bass",
          Values {
              { "poly.voice.mode", 1.0f },
              { "poly.vcoA.octave", 0.0f }, { "poly.vcoA.wave", 0.1f },
              { "poly.vcoA.pw", 0.5f }, { "poly.vcoA.pulse", 0.3f },
              { "poly.vcoA.level", 0.8f }, { "poly.vcoA.metalizer", 0.45f },
              { "poly.vcoB.octave", 0.0f }, { "poly.vcoB.tune", -12.0f },
              { "poly.vcoB.level", 0.5f }, { "poly.vcoB.sub", 0.35f },
              { "poly.steiner.mode", 0.15f }, { "poly.steiner.cutoff", 520.0f },
              { "poly.steiner.resonance", 0.35f }, { "poly.steiner.envAmt", 0.4f },
              { "poly.ladder.cutoff", 380.0f }, { "poly.ladder.resonance", 0.2f },
              { "poly.ladder.drive", 2.2f }, { "poly.ladder.envAmt", 0.35f },
              { "poly.env1.attack", 0.002f }, { "poly.env1.decay", 0.3f },
              { "poly.env1.sustain", 0.25f }, { "poly.env1.release", 0.2f },
              { "poly.env2.attack", 0.002f }, { "poly.env2.decay", 0.4f },
              { "poly.env2.sustain", 0.3f }, { "poly.env2.release", 0.25f },
              { "poly.fx.dist.type", 0.0f }, { "poly.fx.dist.drive", 1.8f },
              { "poly.fx.dist.mix", 0.2f }
          } },
        { "BA - Sub Bass", "Bass",
          Values {
              { "poly.voice.mode", 1.0f },
              { "poly.vcoA.octave", 0.0f }, { "poly.vcoA.wave", 0.0f },
              { "poly.vcoA.pw", 0.5f }, { "poly.vcoA.pulse", 0.6f },
              { "poly.vcoA.level", 0.7f },
              { "poly.vcoB.octave", 0.0f }, { "poly.vcoB.level", 0.4f },
              { "poly.vcoB.sub", 0.6f },
              { "poly.steiner.cutoff", 350.0f }, { "poly.steiner.resonance", 0.2f },
              { "poly.steiner.envAmt", 0.2f },
              { "poly.ladder.cutoff", 600.0f }, { "poly.ladder.resonance", 0.15f },
              { "poly.ladder.drive", 1.2f }, { "poly.ladder.envAmt", 0.2f },
              { "poly.env1.attack", 0.003f }, { "poly.env1.decay", 0.5f },
              { "poly.env1.sustain", 0.6f }, { "poly.env1.release", 0.3f },
              { "poly.env2.attack", 0.002f }, { "poly.env2.decay", 0.3f },
              { "poly.env2.sustain", 0.2f }, { "poly.env2.release", 0.2f }
          } },
        { "LD - Metalizer Mono", "Lead",
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
        { "LD - Blue Lead", "Lead",
          Values {
              { "poly.voice.mode", 0.0f },
              { "poly.vcoA.wave", 0.2f }, { "poly.vcoA.level", 0.8f },
              { "poly.vcoB.tune", 7.0f }, { "poly.vcoB.level", 0.5f },
              { "poly.steiner.cutoff", 1800.0f }, { "poly.steiner.resonance", 0.35f },
              { "poly.steiner.envAmt", 0.3f },
              { "poly.ladder.cutoff", 2600.0f }, { "poly.ladder.resonance", 0.2f },
              { "poly.ladder.drive", 1.4f },
              { "poly.env1.attack", 0.004f }, { "poly.env1.decay", 0.35f },
              { "poly.env1.sustain", 0.75f }, { "poly.env1.release", 0.3f },
              { "poly.fx.chorus.mix", 0.15f },
              { "poly.fx.delay.time", 0.3f }, { "poly.fx.delay.feedback", 0.35f },
              { "poly.fx.delay.mix", 0.25f }, { "poly.fx.delay.sync", 1.0f }
          } },
        { "BR - Palace Brass", "Brass",
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
        { "PD - Morph Pad", "Pad",
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
          } },
        { "PD - Chorus Pad", "Pad",
          Values {
              { "poly.voice.spread", 0.45f }, { "poly.voice.analog", 0.5f },
              { "poly.vcoA.wave", 0.7f }, { "poly.vcoA.pulse", 0.35f },
              { "poly.vcoB.wave", 0.8f }, { "poly.vcoB.sub", 0.2f },
              { "poly.steiner.mode", 0.35f }, { "poly.steiner.cutoff", 900.0f },
              { "poly.steiner.resonance", 0.3f }, { "poly.steiner.envAmt", 0.2f },
              { "poly.ladder.cutoff", 1400.0f }, { "poly.ladder.resonance", 0.2f },
              { "poly.env1.attack", 1.2f }, { "poly.env1.decay", 2.0f },
              { "poly.env1.sustain", 0.9f }, { "poly.env1.release", 3.0f },
              { "poly.env2.attack", 0.8f }, { "poly.env2.decay", 1.5f },
              { "poly.env2.sustain", 0.6f }, { "poly.env2.release", 2.5f },
              { "poly.lfo2.pwmAmt", 0.3f }, { "poly.lfo3.cutoffAmt", 0.3f },
              { "poly.fx.chorus.rate", 0.3f }, { "poly.fx.chorus.depth", 0.5f },
              { "poly.fx.chorus.mix", 0.5f }, { "poly.fx.reverb.mix", 0.35f },
              { "poly.fx.reverb.size", 0.8f }, { "poly.fx.delay.mix", 0.15f }
          } },
        { "KY - Brute Keys", "Keys",
          Values {
              { "poly.vcoA.wave", 0.1f }, { "poly.vcoA.pw", 0.4f },
              { "poly.vcoA.pulse", 0.35f }, { "poly.vcoA.level", 0.8f },
              { "poly.vcoB.octave", 0.0f }, { "poly.vcoB.wave", 0.1f },
              { "poly.vcoB.level", 0.4f },
              { "poly.steiner.cutoff", 1600.0f }, { "poly.steiner.resonance", 0.25f },
              { "poly.steiner.envAmt", 0.3f },
              { "poly.ladder.cutoff", 2000.0f }, { "poly.ladder.resonance", 0.1f },
              { "poly.env1.attack", 0.003f }, { "poly.env1.decay", 0.4f },
              { "poly.env1.sustain", 0.35f }, { "poly.env1.release", 0.3f },
              { "poly.env2.attack", 0.002f }, { "poly.env2.decay", 0.3f },
              { "poly.env2.sustain", 0.2f }, { "poly.env2.release", 0.25f },
              { "poly.fx.chorus.mix", 0.2f }, { "poly.fx.reverb.mix", 0.15f }
          } },
        { "ARP - Glass Arp", "Arp",
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
        { "FX - Ribbon Drift", "FX",
          Values {
              { "poly.voice.spread", 0.5f }, { "poly.voice.analog", 0.6f },
              { "poly.vcoA.wave", 0.8f }, { "poly.vcoB.tune", 12.0f },
              { "poly.vcoB.sub", 0.3f },
              { "poly.steiner.cutoff", 1200.0f }, { "poly.steiner.resonance", 0.5f },
              { "poly.steiner.envAmt", 0.1f },
              { "poly.ladder.cutoff", 900.0f }, { "poly.ladder.drive", 1.5f },
              { "poly.env1.attack", 1.5f }, { "poly.env1.sustain", 0.9f },
              { "poly.env1.release", 3.5f },
              { "poly.lfo3.rate", 0.12f }, { "poly.lfo3.cutoffAmt", 0.4f },
              { "poly.fx.delay.time", 0.45f }, { "poly.fx.delay.feedback", 0.55f },
              { "poly.fx.delay.mix", 0.4f }, { "poly.fx.delay.pingPong", 1.0f },
              { "poly.fx.reverb.size", 0.85f }, { "poly.fx.reverb.mix", 0.45f }
          } },
        { "BA - Reese Bass", "Bass",
          Values {
              { "poly.voice.mode", 1.0f }, { "poly.voice.spread", 0.1f },
              { "poly.voice.analog", 0.4f },
              { "poly.vcoA.octave", 1.0f }, { "poly.vcoA.wave", 0.0f },
              { "poly.vcoA.level", 0.8f },
              { "poly.vcoB.octave", 1.0f }, { "poly.vcoB.wave", 0.0f },
              { "poly.vcoB.tune", 0.12f }, { "poly.vcoB.level", 0.7f },
              { "poly.vcoB.sub", 0.35f },
              { "poly.steiner.mode", 0.1f }, { "poly.steiner.cutoff", 420.0f },
              { "poly.steiner.resonance", 0.2f }, { "poly.steiner.envAmt", 0.25f },
              { "poly.ladder.cutoff", 500.0f }, { "poly.ladder.resonance", 0.15f },
              { "poly.ladder.drive", 1.6f }, { "poly.ladder.envAmt", 0.25f },
              { "poly.filters.routing", 0.0f }, { "poly.filters.balance", 0.5f },
              { "poly.env1.attack", 0.003f }, { "poly.env1.decay", 0.5f },
              { "poly.env1.sustain", 0.95f }, { "poly.env1.release", 0.3f },
              { "poly.env2.attack", 0.003f }, { "poly.env2.decay", 0.4f },
              { "poly.env2.sustain", 0.4f }, { "poly.env2.release", 0.3f },
              { "poly.env3.attack", 0.01f }, { "poly.env3.decay", 0.6f },
              { "poly.env3.sustain", 0.2f }, { "poly.env3.release", 0.4f },
              { "poly.fx.dist.type", 0.0f }, { "poly.fx.dist.drive", 1.4f },
              { "poly.fx.dist.mix", 0.15f }
          } },
        { "BA - Acid Bass", "Bass",
          Values {
              { "poly.voice.mode", 1.0f }, { "poly.voice.glide", 0.045f },
              { "poly.vcoA.wave", 0.0f }, { "poly.vcoA.level", 0.8f },
              { "poly.vcoB.wave", 0.0f }, { "poly.vcoB.level", 0.5f },
              { "poly.steiner.cutoff", 500.0f }, { "poly.steiner.resonance", 0.75f },
              { "poly.steiner.envAmt", 0.8f },
              { "poly.ladder.cutoff", 900.0f }, { "poly.ladder.drive", 2.5f },
              { "poly.ladder.envAmt", 0.3f },
              { "poly.filters.routing", 0.0f },
              { "poly.env1.attack", 0.002f }, { "poly.env1.decay", 0.25f },
              { "poly.env1.sustain", 0.0f }, { "poly.env1.release", 0.15f },
              { "poly.env2.attack", 0.002f }, { "poly.env2.decay", 0.2f },
              { "poly.env2.sustain", 0.0f }, { "poly.env2.release", 0.15f },
              { "poly.env3.attack", 0.01f }, { "poly.env3.decay", 0.3f },
              { "poly.env3.sustain", 0.0f }, { "poly.env3.release", 0.2f },
              { "poly.fx.dist.type", 3.0f }, { "poly.fx.dist.drive", 2.2f },
              { "poly.fx.dist.mix", 0.3f }
          } },
        { "LD - Supersaw Lead", "Lead",
          Values {
              { "poly.voice.mode", 2.0f }, { "poly.voice.unison", 2.0f },
              { "poly.voice.spread", 0.7f },
              { "poly.vcoA.wave", 0.0f }, { "poly.vcoA.level", 0.7f },
              { "poly.vcoB.wave", 0.0f }, { "poly.vcoB.tune", 0.1f },
              { "poly.vcoB.level", 0.7f },
              { "poly.steiner.cutoff", 3800.0f }, { "poly.steiner.resonance", 0.15f },
              { "poly.steiner.envAmt", 0.2f },
              { "poly.ladder.cutoff", 4200.0f }, { "poly.ladder.drive", 1.2f },
              { "poly.env1.attack", 0.01f }, { "poly.env1.decay", 0.7f },
              { "poly.env1.sustain", 0.9f }, { "poly.env1.release", 0.5f },
              { "poly.fx.chorus.depth", 0.45f }, { "poly.fx.chorus.mix", 0.45f },
              { "poly.fx.delay.time", 0.25f }, { "poly.fx.delay.mix", 0.2f },
              { "poly.fx.delay.sync", 1.0f },
              { "poly.fx.reverb.mix", 0.2f }
          } },
        { "LD - Hoover Lead", "Lead",
          Values {
              { "poly.voice.mode", 1.0f }, { "poly.voice.glide", 0.06f },
              { "poly.vcoA.wave", 0.0f }, { "poly.vcoA.pulse", 0.6f },
              { "poly.vcoA.pw", 0.45f }, { "poly.vcoA.level", 0.8f },
              { "poly.vcoB.wave", 0.0f }, { "poly.vcoB.tune", 7.0f },
              { "poly.vcoB.level", 0.5f },
              { "poly.steiner.cutoff", 1800.0f }, { "poly.steiner.resonance", 0.5f },
              { "poly.steiner.envAmt", 0.3f },
              { "poly.ladder.cutoff", 1200.0f }, { "poly.ladder.drive", 2.0f },
              { "poly.ladder.envAmt", 0.3f },
              { "poly.lfo2.rate", 4.5f }, { "poly.lfo2.pwmAmt", 0.4f },
              { "poly.env1.attack", 0.01f }, { "poly.env1.decay", 0.5f },
              { "poly.env1.sustain", 0.8f }, { "poly.env1.release", 0.4f },
              { "poly.fx.chorus.mix", 0.3f },
              { "poly.fx.delay.time", 0.28f }, { "poly.fx.delay.mix", 0.2f }
          } },
        { "PD - Warm Analog Pad", "Pad",
          Values {
              { "poly.voice.spread", 0.5f }, { "poly.voice.analog", 0.5f },
              { "poly.vcoA.wave", 0.7f }, { "poly.vcoA.pulse", 0.3f },
              { "poly.vcoB.wave", 0.8f }, { "poly.vcoB.sub", 0.2f },
              { "poly.steiner.mode", 0.3f }, { "poly.steiner.cutoff", 900.0f },
              { "poly.steiner.resonance", 0.25f }, { "poly.steiner.envAmt", 0.2f },
              { "poly.ladder.cutoff", 1400.0f }, { "poly.ladder.resonance", 0.2f },
              { "poly.env1.attack", 1.5f }, { "poly.env1.decay", 2.0f },
              { "poly.env1.sustain", 0.9f }, { "poly.env1.release", 3.0f },
              { "poly.env2.attack", 1.0f }, { "poly.env2.decay", 2.0f },
              { "poly.env2.sustain", 0.6f }, { "poly.env2.release", 2.5f },
              { "poly.env3.attack", 0.8f }, { "poly.env3.decay", 1.5f },
              { "poly.env3.sustain", 0.5f }, { "poly.env3.release", 2.0f },
              { "poly.lfo3.rate", 0.12f }, { "poly.lfo3.cutoffAmt", 0.2f },
              { "poly.fx.chorus.rate", 0.3f }, { "poly.fx.chorus.depth", 0.4f },
              { "poly.fx.chorus.mix", 0.4f }, { "poly.fx.reverb.mix", 0.35f },
              { "poly.fx.reverb.size", 0.75f }, { "poly.fx.delay.mix", 0.15f }
          } },
                { "OR - Organ", "Organ",
          Values {
              { "poly.voice.mode", 0.0f }, { "poly.voice.analog", 0.05f },
              { "poly.voice.spread", 0.15f },
              { "poly.vcoA.octave", 1.0f }, { "poly.vcoA.wave", 1.0f },
              { "poly.vcoA.pw", 0.5f }, { "poly.vcoA.pulse", 0.5f },
              { "poly.vcoA.level", 0.8f }, { "poly.vcoA.metalizer", 0.0f },
              { "poly.vcoB.octave", 2.0f }, { "poly.vcoB.wave", 1.0f },
              { "poly.vcoB.tune", 7.0f }, { "poly.vcoB.pulse", 0.5f },
              { "poly.vcoB.level", 0.45f }, { "poly.vcoB.sub", 0.2f },
              { "poly.steiner.mode", 0.0f }, { "poly.steiner.cutoff", 8000.0f },
              { "poly.steiner.resonance", 0.02f }, { "poly.steiner.envAmt", 0.0f },
              { "poly.ladder.cutoff", 9000.0f }, { "poly.ladder.resonance", 0.02f },
              { "poly.ladder.drive", 1.0f }, { "poly.ladder.envAmt", 0.0f },
              { "poly.env1.attack", 0.003f }, { "poly.env1.decay", 0.1f },
              { "poly.env1.sustain", 1.0f }, { "poly.env1.release", 0.1f },
              { "poly.env2.attack", 0.003f }, { "poly.env2.decay", 0.1f },
              { "poly.env2.sustain", 1.0f }, { "poly.env2.release", 0.1f },
              { "poly.env3.attack", 0.003f }, { "poly.env3.decay", 0.1f },
              { "poly.env3.sustain", 1.0f }, { "poly.env3.release", 0.1f },
              { "poly.fx.chorus.rate", 0.25f }, { "poly.fx.chorus.depth", 0.3f },
              { "poly.fx.chorus.mix", 0.35f }, { "poly.fx.reverb.mix", 0.12f },
              { "poly.fx.delay.mix", 0.05f }
          } },
        { "PL - Analog Pluck", "Pluck",
          Values {
              { "poly.vcoA.wave", 0.0f }, { "poly.vcoA.level", 0.8f },
              { "poly.vcoB.tune", 12.0f }, { "poly.vcoB.level", 0.4f },
              { "poly.steiner.cutoff", 1200.0f }, { "poly.steiner.resonance", 0.4f },
              { "poly.steiner.envAmt", 0.7f },
              { "poly.ladder.cutoff", 1600.0f },
              { "poly.env1.attack", 0.002f }, { "poly.env1.decay", 0.18f },
              { "poly.env1.sustain", 0.0f }, { "poly.env1.release", 0.12f },
              { "poly.env2.attack", 0.002f }, { "poly.env2.decay", 0.2f },
              { "poly.env2.sustain", 0.0f }, { "poly.env2.release", 0.15f },
              { "poly.env3.attack", 0.002f }, { "poly.env3.decay", 0.2f },
              { "poly.env3.sustain", 0.0f }, { "poly.env3.release", 0.1f },
              { "poly.fx.delay.time", 0.25f }, { "poly.fx.delay.mix", 0.25f },
              { "poly.fx.delay.pingPong", 1.0f },
              { "poly.fx.reverb.mix", 0.2f }
          } }
    });
}

std::vector<FactoryPreset> onePresets()
{
    return sortedPresets ({
        { "BA - One Bass", "Bass",
          Values {
              { "one.p1.enabled", 1.0f },
              { "one.p1.vco1.octave", 0.0f }, { "one.p1.vco1.waveAngle", 0.1f },
              { "one.p1.vco1.shape", 0.4f }, { "one.p1.vco1.level", 0.8f },
              { "one.p1.vco2.octave", 0.0f }, { "one.p1.vco2.tune", -12.0f },
              { "one.p1.vco2.level", 0.5f }, { "one.p1.vco3.level", 0.0f },
              { "one.p1.svf.cutoff", 450.0f }, { "one.p1.svf.resonance", 0.2f },
              { "one.p1.svf.envAmt", 0.4f }, { "one.p1.svf.keyTrack", 0.35f },
              { "one.p1.ladder.cutoff", 600.0f }, { "one.p1.ladder.resonance", 0.15f },
              { "one.p1.ladder.drive", 0.3f }, { "one.p1.ladder.envAmt", 0.3f },
              { "one.p1.env1.attack", 0.002f }, { "one.p1.env1.decay", 0.3f },
              { "one.p1.env1.sustain", 0.4f }, { "one.p1.env1.release", 0.2f },
              { "one.p1.env2.attack", 0.002f }, { "one.p1.env2.decay", 0.25f },
              { "one.p1.env2.sustain", 0.2f }, { "one.p1.env2.release", 0.2f },
              { "one.p1.env3.attack", 0.003f }, { "one.p1.env3.decay", 0.4f },
              { "one.p1.env3.sustain", 0.7f }, { "one.p1.env3.release", 0.25f },
              { "one.p1.fx.dist.type", 0.0f }, { "one.p1.fx.dist.drive", 0.2f },
              { "one.p1.fx.dist.mix", 0.15f }
          } },
        { "BA - Sub One", "Bass",
          Values {
              { "one.p1.enabled", 1.0f },
              { "one.p1.vco1.octave", 0.0f }, { "one.p1.vco1.level", 0.75f },
              { "one.p1.vco2.octave", 0.0f }, { "one.p1.vco2.tune", -12.0f },
              { "one.p1.vco2.level", 0.6f }, { "one.p1.vco3.level", 0.4f },
              { "one.p1.vco3.tune", -12.0f },
              { "one.p1.svf.cutoff", 300.0f }, { "one.p1.svf.resonance", 0.15f },
              { "one.p1.svf.envAmt", 0.3f },
              { "one.p1.ladder.cutoff", 450.0f }, { "one.p1.ladder.drive", 0.2f },
              { "one.p1.env1.attack", 0.003f }, { "one.p1.env1.decay", 0.4f },
              { "one.p1.env1.sustain", 0.5f }, { "one.p1.env1.release", 0.25f },
              { "one.p1.env3.attack", 0.003f }, { "one.p1.env3.sustain", 0.8f }
          } },
        { "LD - One Lead", "Lead",
          Values {
              { "one.p1.enabled", 1.0f },
              { "one.p1.vco1.waveAngle", 0.2f }, { "one.p1.vco3.tune", 12.0f },
              { "one.p1.vco3.level", 0.4f },
              { "one.p1.svf.cutoff", 2400.0f }, { "one.p1.svf.resonance", 0.3f },
              { "one.p1.svf.envAmt", 0.3f },
              { "one.p1.ladder.cutoff", 3000.0f }, { "one.p1.ladder.envAmt", 0.2f },
              { "one.p1.env1.attack", 0.003f }, { "one.p1.env1.decay", 0.3f },
              { "one.p1.env1.sustain", 0.7f }, { "one.p1.env1.release", 0.25f },
              { "one.master.glide", 0.12f },
              { "one.p1.fx.delay.time", 0.28f }, { "one.p1.fx.delay.feedback", 0.35f },
              { "one.p1.fx.delay.mix", 0.2f }
          } },
        { "BR - Poly Brass", "Brass",
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
        { "PD - Big Pad", "Pad",
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
          } },
        { "KY - One Keys", "Keys",
          Values {
              { "one.p1.enabled", 1.0f },
              { "one.p1.vco1.waveAngle", 0.4f }, { "one.p1.vco1.shape", 0.5f },
              { "one.p1.vco2.tune", 12.0f }, { "one.p1.vco2.shape", 0.3f },
              { "one.p1.svf.cutoff", 1600.0f }, { "one.p1.svf.resonance", 0.2f },
              { "one.p1.svf.envAmt", 0.35f },
              { "one.p1.ladder.cutoff", 2200.0f },
              { "one.p1.env1.attack", 0.005f }, { "one.p1.env1.decay", 0.6f },
              { "one.p1.env1.sustain", 0.4f }, { "one.p1.env1.release", 0.5f },
              { "one.p1.env2.attack", 0.01f }, { "one.p1.env2.decay", 0.5f },
              { "one.p1.env2.sustain", 0.3f }, { "one.p1.env2.release", 0.4f },
              { "one.p1.fx.chorus.mix", 0.25f }, { "one.p1.fx.reverb.send", 0.2f }
          } },
        { "ARP - Glass Arp", "Arp",
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
        { "SPL - Three-Zone Split", "Split",
          Values {
              { "one.p1.enabled", 1.0f }, { "one.p1.keyboardLow", 0.0f },
              { "one.p1.keyboardHigh", 47.0f }, { "one.p1.vco1.octave", 0.0f },
              { "one.p1.svf.cutoff", 700.0f }, { "one.p1.ladder.cutoff", 900.0f },
              { "one.p2.enabled", 1.0f }, { "one.p2.keyboardLow", 48.0f },
              { "one.p2.keyboardHigh", 71.0f }, { "one.p2.svf.cutoff", 1600.0f },
              { "one.p2.vco2.tune", 7.0f }, { "one.p2.midiChannel", 0.0f },
              { "one.p3.enabled", 1.0f }, { "one.p3.keyboardLow", 72.0f },
              { "one.p3.keyboardHigh", 127.0f }, { "one.p3.vco1.octave", 2.0f },
              { "one.p3.midiChannel", 0.0f },
              { "one.p3.svf.cutoff", 2600.0f }, { "one.p3.fx.delay.mix", 0.25f },
              { "one.master.reverbMix", 0.2f }
          } },
        { "FX - Deep Space", "FX",
          Values {
              { "one.p1.enabled", 1.0f }, { "one.p1.vco1.octave", 0.0f },
              { "one.p1.vco1.waveAngle", 0.6f }, { "one.p1.vco3.tune", -12.0f },
              { "one.p1.svf.cutoff", 800.0f }, { "one.p1.svf.resonance", 0.4f },
              { "one.p1.ladder.cutoff", 600.0f }, { "one.p1.ladder.drive", 0.4f },
              { "one.p1.env1.attack", 2.5f }, { "one.p1.env1.sustain", 0.9f },
              { "one.p1.env1.release", 5.0f },
              { "one.p1.env3.attack", 1.5f }, { "one.p1.env3.sustain", 0.9f },
              { "one.p1.env3.release", 5.0f },
              { "one.p1.lfo3.rate", 0.08f }, { "one.p1.lfo3.cutoffAmt", 0.4f },
              { "one.p1.fx.delay.time", 0.5f }, { "one.p1.fx.delay.feedback", 0.5f },
              { "one.p1.fx.delay.mix", 0.35f },
              { "one.p1.fx.reverb.send", 0.6f },
              { "one.master.reverbSize", 0.9f }, { "one.master.reverbMix", 0.5f }
          } },
        { "BA - Reese Bass", "Bass",
          Values {
              { "one.p1.enabled", 1.0f },
              { "one.p1.vco1.waveAngle", 0.0f }, { "one.p1.vco1.level", 0.75f },
              { "one.p1.vco2.tune", 0.12f }, { "one.p1.vco2.level", 0.65f },
              { "one.p1.vco3.tune", -0.1f }, { "one.p1.vco3.level", 0.5f },
              { "one.p1.svf.cutoff", 450.0f }, { "one.p1.svf.resonance", 0.2f },
              { "one.p1.svf.envAmt", 0.25f }, { "one.p1.svf.keyTrack", 0.2f },
              { "one.p1.ladder.cutoff", 500.0f }, { "one.p1.ladder.drive", 0.4f },
              { "one.p1.ladder.envAmt", 0.25f },
              { "one.p1.filters.routing", 0.0f },
              { "one.p1.env1.attack", 0.003f }, { "one.p1.env1.decay", 0.5f },
              { "one.p1.env1.sustain", 0.95f }, { "one.p1.env1.release", 0.3f },
              { "one.p1.env2.attack", 0.003f }, { "one.p1.env2.decay", 0.4f },
              { "one.p1.env2.sustain", 0.4f }, { "one.p1.env2.release", 0.3f },
              { "one.p1.env3.attack", 0.01f }, { "one.p1.env3.decay", 0.6f },
              { "one.p1.env3.sustain", 0.2f }, { "one.p1.env3.release", 0.4f },
              { "one.p1.fx.dist.type", 0.0f }, { "one.p1.fx.dist.drive", 0.3f },
              { "one.p1.fx.dist.mix", 0.15f }
          } },
        { "BA - Acid Bass", "Bass",
          Values {
              { "one.p1.enabled", 1.0f },
              { "one.p1.vco1.waveAngle", 0.0f }, { "one.p1.vco1.level", 0.8f },
              { "one.p1.svf.cutoff", 500.0f }, { "one.p1.svf.resonance", 0.75f },
              { "one.p1.svf.envAmt", 0.8f },
              { "one.p1.ladder.cutoff", 900.0f }, { "one.p1.ladder.drive", 0.6f },
              { "one.p1.ladder.envAmt", 0.4f },
              { "one.p1.env1.attack", 0.002f }, { "one.p1.env1.decay", 0.25f },
              { "one.p1.env1.sustain", 0.0f }, { "one.p1.env1.release", 0.15f },
              { "one.p1.env2.attack", 0.002f }, { "one.p1.env2.decay", 0.2f },
              { "one.p1.env2.sustain", 0.0f }, { "one.p1.env2.release", 0.15f },
              { "one.p1.env3.attack", 0.003f }, { "one.p1.env3.decay", 0.3f },
              { "one.p1.env3.sustain", 0.0f }, { "one.p1.env3.release", 0.2f },
              { "one.p1.fx.dist.type", 3.0f }, { "one.p1.fx.dist.drive", 0.6f },
              { "one.p1.fx.dist.mix", 0.25f },
              { "one.master.glide", 0.05f }
          } },
        { "LD - Supersaw Lead", "Lead",
          Values {
              { "one.p1.enabled", 1.0f },
              { "one.p1.vco1.waveAngle", 0.0f }, { "one.p1.vco2.tune", 0.12f },
              { "one.p1.vco3.tune", -0.1f },
              { "one.p1.svf.cutoff", 3800.0f }, { "one.p1.svf.resonance", 0.15f },
              { "one.p1.ladder.cutoff", 4200.0f }, { "one.p1.ladder.drive", 0.3f },
              { "one.p1.env1.attack", 0.01f }, { "one.p1.env1.decay", 0.7f },
              { "one.p1.env1.sustain", 0.9f }, { "one.p1.env1.release", 0.5f },
              { "one.p1.fx.chorus.depth", 0.45f }, { "one.p1.fx.chorus.mix", 0.45f },
              { "one.p1.fx.delay.time", 0.25f }, { "one.p1.fx.delay.mix", 0.2f },
              { "one.p1.fx.reverb.send", 0.25f }, { "one.master.reverbMix", 0.2f }
          } },
        { "PD - Warm Analog Pad", "Pad",
          Values {
              { "one.p1.enabled", 1.0f },
              { "one.p1.vco1.waveAngle", 0.5f }, { "one.p1.vco2.waveAngle", 0.6f },
              { "one.p1.vco3.tune", -12.0f },
              { "one.p1.svf.cutoff", 900.0f }, { "one.p1.svf.resonance", 0.25f },
              { "one.p1.svf.envAmt", 0.2f },
              { "one.p1.ladder.cutoff", 1400.0f }, { "one.p1.ladder.resonance", 0.2f },
              { "one.p1.env1.attack", 1.5f }, { "one.p1.env1.decay", 2.0f },
              { "one.p1.env1.sustain", 0.9f }, { "one.p1.env1.release", 3.0f },
              { "one.p1.env2.attack", 1.0f }, { "one.p1.env2.decay", 2.0f },
              { "one.p1.env2.sustain", 0.6f }, { "one.p1.env2.release", 2.5f },
              { "one.p1.env3.attack", 1.2f }, { "one.p1.env3.decay", 2.0f },
              { "one.p1.env3.sustain", 0.8f }, { "one.p1.env3.release", 3.0f },
              { "one.p1.lfo3.rate", 0.12f }, { "one.p1.lfo3.cutoffAmt", 0.2f },
              { "one.p1.fx.chorus.depth", 0.45f }, { "one.p1.fx.chorus.mix", 0.45f },
              { "one.p1.fx.reverb.send", 0.4f },
              { "one.master.reverbSize", 0.8f }, { "one.master.reverbMix", 0.3f }
          } },
                { "OR - Organ", "Organ",
          Values {
              { "one.p1.enabled", 1.0f },
              { "one.p1.vco1.octave", 1.0f }, { "one.p1.vco1.waveAngle", 0.5f },
              { "one.p1.vco1.shape", 0.0f }, { "one.p1.vco1.level", 0.8f },
              { "one.p1.vco2.octave", 2.0f }, { "one.p1.vco2.waveAngle", 0.5f },
              { "one.p1.vco2.shape", 0.0f }, { "one.p1.vco2.tune", 7.0f },
              { "one.p1.vco2.level", 0.5f },
              { "one.p1.vco3.octave", 0.0f }, { "one.p1.vco3.waveAngle", 0.5f },
              { "one.p1.vco3.shape", 0.0f }, { "one.p1.vco3.level", 0.35f },
              { "one.p1.svf.cutoff", 8000.0f }, { "one.p1.svf.resonance", 0.05f },
              { "one.p1.svf.envAmt", 0.0f },
              { "one.p1.ladder.cutoff", 9000.0f }, { "one.p1.ladder.drive", 0.1f },
              { "one.p1.env1.attack", 0.003f }, { "one.p1.env1.decay", 0.1f },
              { "one.p1.env1.sustain", 1.0f }, { "one.p1.env1.release", 0.1f },
              { "one.p1.env2.attack", 0.003f }, { "one.p1.env2.decay", 0.1f },
              { "one.p1.env2.sustain", 1.0f }, { "one.p1.env2.release", 0.1f },
              { "one.p1.env3.attack", 0.003f }, { "one.p1.env3.decay", 0.1f },
              { "one.p1.env3.sustain", 1.0f }, { "one.p1.env3.release", 0.1f },
              { "one.p1.fx.chorus.mix", 0.35f },
              { "one.p1.fx.reverb.send", 0.12f },
              { "one.master.reverbMix", 0.15f }, { "one.master.reverbSize", 0.5f }
          } },
        { "PL - Analog Pluck", "Pluck",
          Values {
              { "one.p1.enabled", 1.0f },
              { "one.p1.vco1.waveAngle", 0.0f }, { "one.p1.vco1.level", 0.8f },
              { "one.p1.vco2.tune", 12.0f }, { "one.p1.vco2.level", 0.4f },
              { "one.p1.svf.cutoff", 1200.0f }, { "one.p1.svf.resonance", 0.4f },
              { "one.p1.svf.envAmt", 0.7f },
              { "one.p1.ladder.cutoff", 1600.0f },
              { "one.p1.env1.attack", 0.002f }, { "one.p1.env1.decay", 0.18f },
              { "one.p1.env1.sustain", 0.0f }, { "one.p1.env1.release", 0.12f },
              { "one.p1.env2.attack", 0.002f }, { "one.p1.env2.decay", 0.2f },
              { "one.p1.env2.sustain", 0.0f }, { "one.p1.env2.release", 0.15f },
              { "one.p1.env3.attack", 0.002f }, { "one.p1.env3.decay", 0.2f },
              { "one.p1.env3.sustain", 0.0f }, { "one.p1.env3.release", 0.1f },
              { "one.p1.fx.delay.time", 0.25f }, { "one.p1.fx.delay.feedback", 0.3f },
              { "one.p1.fx.delay.mix", 0.25f },
              { "one.p1.fx.reverb.send", 0.2f }
          } }
    });
}


// The supplied Haken soundlist contains names and two-letter categories only.
// These presets reproduce that library structure with original sound design;
// no EaganMatrix parameter data is present in the PDF.
const char* hakenNamesRaw = R"HAKEN(
MD|Kontakt 1 Mono
MD|Kontakt 1 Perform
MD|Kyma 1
MD|Kyma 2 Round
MD|Kyma 3 Initial Round
MD|Kyma 4 Release Round
MD|MPE 04 Voice
MD|MPE 06 Voice
MD|MPE 08 Voice
MD|MPE 12 Voice
MD|MPE+ 08 Voice
MD|Omnisphere 1 Perform
MD|Omnisphere 2 Round
MD|Omnisphere 3 Initial Round
MD|Omnisphere 4 Semitone
MD|Omnisphere 5 Mono
CV|CVC 10v Linear Z
CV|CVC 10v Square Z
CV|CVC 5v C0 Linear Z
CV|CVC 5v C0 Square Z
CV|CVC 5v C2 Linear Z
CV|CVC 5v C2 Square Z
CV|CVC 5v C4 Linear Z
CV|CVC 5v C4 Square Z
CV|CVC Buchla Linear Z
CV|CVC Buchla Square Z
CV|CVC Four Shape Generators
CV|CVC Voyager Linear Z
CV|CVC Voyager Square Z
CV|CVC Y Shelf Linear Z
CV|CVC Y Shelf Square Z
UT|Audio Through
UT|Basic Bowed Spring
UT|Basic Jenny
UT|Basic Spring Bell
UT|BiqBank - Basic
UT|BiqGraph - Basic
UT|BiqMouth - Basic
UT|Empty
UT|Filter - The Ladder
UT|HarMan - Basic
UT|JennyBasic FixRes
UT|Karplus - 1
UT|Karplus - 2
UT|Karplus - Pipe
UT|Kinetic - Bouncing
UT|Kinetic - Bowed Spring
UT|Kinetic - Bowed Waveguide
UT|Kinetic - Crackling Noise
UT|Kinetic - Filter
UT|Kinetic - Spring Bell
UT|Kinetic - StickSlip Filter
UT|Kinetic - Tracker
UT|Kinetic - Vinyl
UT|Kinetic - Waveguide
UT|Kinetic as Filter
UT|ModMan - Pulsed
UT|Noise - Out of Phase
UT|Noise - Pink
UT|Noise - White Stereo
UT|Noise - White at -35 RMS
UT|Osc - A440 at -35
UT|Osc - Formula Delay
UT|Osc - Pitch via Z
UT|Osc - Random Pitch
UT|Osc - Sine Wave
UT|Osc - Subtractive Synth
UT|Osc - Waveshaping
UT|Ring Mod - Basic
UT|SineBank - Basic
UT|SineSpray - Basic
UT|WaveBank – Basic
USR|FollowDly
USR|CrossMod1
USR|Woodwind
USR|Jaymar Toy Piano
USR|Snap Bass
USR|Bass Monster
USR|Choir
USR|Cumulus
USR|Shimmer
USR|Living Pad
USR|Bowed Mood
USR|Ishango Bone
USR|Marlin Perkins 1
USR|Mojo of FDN
USR|Zipper
USR|SineSpray Rain via Surface
ST|Bowed BiqBank
ST|Bowed Double Reed
ST|Bowed KinBanjo
ST|Bowed Mood
ST|BuzzStrings
ST|Cimbalom - Continuous
ST|Cimbalom - Z Pitch
ST|Country Resonator
ST|Dulcimer Ghost
ST|Eastern Slider Ch1
ST|Electric Guitar
ST|Electric Guitar Saturated
ST|Feedback String
ST|Haegum
ST|Harmonic Break
ST|Harmonic Viol
ST|Harmonoid Spark
ST|Harp
ST|Hoop Lute
ST|Hyper Pizz
ST|Jupiter Mission
ST|Karplus & ModMan
ST|Kinetic Cabinet
ST|Kinetic Kokyu
ST|Kinetic Soundboard
ST|Kinetic WG AkouBass
ST|Kinetic WG AkouBass - Var1
ST|Koto
ST|Lap Steel
ST|Melow Pedal Steel
ST|MicroDelay WaveGuide v1
ST|MicroDelay WaveGuide v2
ST|MicroDelay WaveGuide v3
ST|Model String Wind
ST|Mountain Slider
ST|Mountain Slider Ch1
ST|NGoni
ST|NGoni - Kinetic
ST|Northern Lights
ST|Plucked Soup Can
ST|Pusher
ST|Resizable Guitar
ST|Resodynamic
ST|Rhythm And Bass
ST|Roto String
ST|Rub
ST|Rub String
ST|SiTan
ST|SiloString Pizz
ST|Silver Mirror
ST|SineBank FM String
ST|Singing Bamboo
ST|Singing Bamboo 2
ST|Sinseong-ri
ST|SlideyPizz
ST|Small Steel
ST|Snap Bass
ST|Snap String
ST|Spiccato Tremolo Dual
ST|Spiccato Tremolo Single
ST|Squeaky Balloon String
ST|Stellar Bowls
ST|Stretch String m1 T
ST|Stretch String m2 T
ST|Stretch String m3 T
ST|Stretch String v2 T
ST|Strummed Gtr 3HRSolo
ST|Strummed Gtr Rythm T 950
ST|Sympathy String
ST|Synchronous Orbits
ST|Tap Sitar
ST|Tapestry
ST|The Touch Guitar
ST|Tunable Tanpura
ST|Uki Pizz 1
ST|Uki Pizz 2
ST|Uki Pizz w Snap
ST|Vln Vla Cel Bass 1
ST|Vln Vla Cel Bass 2
ST|Vln Vla Cel Bass 2 Ch1
ST|Vln Vla Cel Bass 3
ST|Vln Vla Cel Bass 4
ST|Vln Vla Cel Bass Bridge
ST|Vln Vla Cel Bass KeySwitch
ST|VlnVlaVlcCbFull
ST|VlnVlaVlcCbPlus
ST|Waterphone Strings
ST|xpStrings
WI|Bagpipes
WI|Brass Mute
WI|Calliope
WI|Chinese Clarinet
WI|Clarinet
WI|Crumb Horn
WI|Double Reed
WI|Empyrean
WI|Flip Tube
WI|Fragaria Fields
WI|French Sax
WI|GrainSilo Woodwind
WI|Jemonica
WI|Jenny Trumpet
WI|JennyDo
WI|Jerrys Horn
WI|Metal Reed
WI|MicroDelay PipeWG
WI|Miles
WI|MilesBird
WI|Morphing Reed
WI|OvertoneWind
WI|PanPipe Solo
WI|Panpipe Loops
WI|Pipedreams
WI|Saxmosis
WI|Sine2Sax
WI|SineBank Horn
WI|Single Reed
WI|Slide Wind
WI|Slide Wind EM
WI|Soprano Recorder
WI|Space Flute
WI|Spinning Oboe
WI|Spit Tube
WI|Tin Whistle
WI|Touch Reed T
WI|VBrass 1
WI|VBrass 2
WI|Wind
WI|Windtube Air Reed
WI|Windtube Double Reed
WI|Windtube Single Reed
WI|Woodwind
WI|ZawiFlute8
VO|Additive Gnilham
VO|Additive Vocal 1
VO|Additive Vocal 1 Transform
VO|Choir
VO|Choir on Kepler-452b
VO|Choir-Ah
VO|Distant Transmission Choir
VO|Dual Spectra Voice
VO|FOF FixRes
VO|FOF VariableRes
VO|Funny Voice
VO|Instant Reich
VO|Jenny Voice
VO|Kinetic - Vinyl
VO|Larynx Horn
VO|Lost Choir
VO|Mahling 1
VO|Mahling 2
VO|Mahling 3
VO|Mantra Voice
VO|Morphing Voice
VO|Mouth Sequence Ch1
VO|Ring Mod - Voice
VO|Ring Mod - Voice - Var1
VO|Ring Mod - Voice 1
VO|Ring Mod - Voice 2
VO|Should I Stay or Should I Go
VO|Singing Oscillators
VO|Tibetan Throat Stick
VO|Touch Voice T
VO|Two Handed Voice
VO|Vocalise
VO|Vocalized Buzzard
VO|Vowel
KY|Anthophila Organ
KY|Apiary
KY|Electric Harpsychord
KY|FM DreamPiano
KY|FM DreamPiano - Var1
KY|Grinding Stone Calliope
KY|Heavenly Corporation Ch1
KY|Ice Calliope
KY|Jaymar Toy Piano
KY|Jenerator T
KY|Kinetic - Clavinet
KY|Kinetic - Clavinet - Var1
KY|Kinetic Contioline
KY|LegacyDX EP
KY|Morphing Church Organ
KY|Morphing Organ
KY|Music Box Because
KY|Music Box Because Ch1
KY|Musica Minima
KY|Old Pad Machine
KY|Organo Espressivo
KY|Organzilla
KY|PCS Fixed
KY|Plane Tiv Organ
KY|Portable Reed
KY|Positiv Organ
KY|Quaverer
KY|Simple Organ
KY|Tine
KY|Vibrato Organ T
KY|Vintage Electro
CL|Ambiant Analog
CL|Analog ADSR
CL|Analog ADSR - Var1
CL|Analog Overload
CL|Another Big One
CL|Arpeggiator 4 Step
CL|Arpeggiator Resonant
CL|Autocille
CL|Bass DistoSine
CL|Bass Monster
CL|Bass Pad
CL|Boson Particles
CL|CS80+Ladder
CL|CS80+Ladder II
CL|CZ Dirt Bass
CL|CZ Dirt Bass - Var1
CL|Celestial Following
CL|Chord Generator
CL|Chrono Z
CL|Clinical Oscillator 1
CL|Clinical Oscillator 2
CL|CrossMod2
CL|CrossMod3
CL|Dirty Oscillator v2
CL|Distortion Man
CL|Dual 24
CL|Dual Resonators
CL|Dual Sinespray
CL|Echo 8va
CL|FM 1
CL|FM 2
CL|FM Trails
CL|Follower
CL|Four Point FM
CL|Jenny Pulse
CL|Jenny Shepard Down
CL|JennyBasicPad
CL|Kinetic - Dirty Osc
CL|Kinetic - Dirty Osc - Var1
CL|Kinetic - U-Bass
CL|Kinetic AnaSeq
CL|Kinetic SatPad
CL|Ladder Bass
CL|Lain Heart
CL|Living Pad
CL|Music Box Bells
CL|NFS Train 900
CL|Noisy Old Oscillator
CL|Notch Lightning
CL|Pleasantness
CL|Plutonium
CL|Sawz
CL|Sine Chaser
CL|Sineysitus
CL|Square Bass
CL|Square FM
CL|Squaresville
CL|Synth 01 T
CL|Synth Brass
CL|Synth Lead 1
CL|Synth Lead 2
CL|Synth Lead 3
CL|Synth Lead 4
CL|Synth Lead 5
CL|Synth Lead 6
CL|Three Saws
CL|Voice Jumper
CL|Zwei Baende
CL|Zwei Baende with Noise
OT|Acrylic Clock
OT|Acrylic Clock 2
OT|Around the Periapsis
OT|Around the Periapsis Ch1
OT|Bacteria
OT|Bajaron Light Ship
OT|Beautiful Pursuit
OT|Beautiful Pursuit Ch1
OT|Belle Isle
OT|Bird Echoer
OT|Bird Whistler
OT|Bullroarer
OT|Buzzard
OT|Celestial Basin
OT|Centrifuge
OT|Cork the Bottle
OT|Cowell Triangles
OT|CrossMod1
OT|Cyncro Ecro
OT|Echo Star
OT|Electric Comb T
OT|Electric Wazoo
OT|Entanglement
OT|Explosion in A
OT|Explosion in A Minus
OT|Explosion in A.3
OT|Exposure Ensemble
OT|FDN Crazyness
OT|Falling Brook
OT|Fluter
OT|Flutter Blossom T
OT|FollowDly
OT|GaBuZoMeu
OT|Geiger Insects
OT|Ghost Moth
OT|Glass Chorus
OT|Glass Chorus Reverse
OT|Golliclock
OT|Happy Birthday Ed 20
OT|Harman Morph
OT|Harmonic Board
OT|Harmonic Looper
OT|HarmonicSkin
OT|Helix T
OT|Jenny Attack
OT|Jenny Dark Acid
OT|Jenny Dark Acid - Var1
OT|Jenny FromTo
OT|Jenny FromTo - Var1
OT|Jenny Loops
OT|Jenny Random
OT|Jenny Shepard Down
OT|Jenny Shepard Down - Var1
OT|Jenny Shepard Up
OT|Kinetic - Overtones
OT|Kinetic Bowed FDN
OT|Kinetic Disto Analog
OT|Kinetic Dragon
OT|Kinetic MicroMotor
OT|Kinetic Rubber Skin
OT|Kinetic-Wavebank Morph
OT|Lisithean Motor
OT|Lonely Data
OT|Lost Beacon
OT|M-220
OT|Major Vision
OT|Maple Key
OT|Martian Beach
OT|Martian Landing Pad
OT|Meccano
OT|Mini Shepard
OT|Mini Shepard Breathing
OT|Mini Shepard Resonant
OT|Mojo of FDN
OT|Moon Witch
OT|Morphing Wavebank Pad 1
OT|Morphing Wavebank Pad 2
OT|Mr Kyte
OT|Mutate Looper
OT|Natural Law
OT|Octavator
OT|Octave Phases
OT|Orchester
OT|Overtones
OT|PCS Dandelion
OT|Pad Tie
OT|Patville
OT|Phase Controled ModMan
OT|Philco Chromatic
OT|Pinched FM
OT|Ping Pong
OT|Polychromatic
OT|Pterodactyl
OT|Pulse
OT|Random Fun
OT|Random Texture
OT|Random Trigger
OT|Razor Loops
OT|Rosin Oscillator
OT|Rubber Band Stars
OT|Scratching FDN
OT|Seasons Ch1
OT|Sequencer 1 Ch1
OT|Shimmer
OT|Simple and Nice
OT|Sine Wave T
OT|SinePhases
OT|SineSpray Rain via Surface
OT|Singing Noise
OT|Slip Zen
OT|Slow Evolver
OT|Sonogram
OT|Soundboard
OT|Space
OT|Space Async
OT|Spectrum with MidiClock
OT|Spiritus Subteranne
OT|Sputnik's Dream
OT|Stasis Field T
OT|Sub-Harmonic Generator
OT|Submit Job to Mainframe
OT|SuperWave
OT|Sweet Triangle
OT|Swept Delay
OT|Swirl
OT|Synthetic Cathedral
OT|Tesla Coil
OT|Tesla's Dream
OT|The Long Goodbye
OT|The Slow Descent
OT|The Wind on Callisto
OT|Three Cycles
OT|Three Sines
OT|Through the Photodiode
OT|Tick Bodies
OT|Transistor
OT|Transporter
OT|TriMod
OT|Unanswered Question
OT|Unstable Wave
OT|Venusian Beach
OT|Victrola
OT|Victrola 2
OT|Voice of the Woods
OT|Voyager
OT|Waterphonie
OT|WaveShaped
OT|Whirligigs
OT|Wind Beach
OT|Winter Skipping Pond
OT|Wobble Plate
OT|Zazipad
OT|Zipper
PE|Creaker
PE|Drum-Machine Windowed
PE|Drum-Set
PE|Icicles 1
PE|Icicles 2
PE|Jaw Stick 1
PE|Jaw Stick 2
PE|Kinetic - Kick
PE|Metal Rainstick
PE|Resonant Drum 1
PE|Resonant Drum 2
PE|Wind Drum
PT|Bell Rub
PT|Bells in the Fields
PT|Bells of Digul
PT|Bouncer
PT|Bowed Bells
PT|Carbon Marimba
PT|Cumulus
PT|Cycle Kalimba
PT|Dolce Cristallo
PT|Dolce Cristallo Space
PT|Dripaphone
PT|Dueling BiqBanks
PT|Echo of a Marimba
PT|FM Bell T
PT|FM Bells
PT|Gamelan Spinner
PT|Genie Bottle
PT|Glas Pfeifen
PT|Glass Rings
PT|Grandfather Clock
PT|Ishango Bone
PT|JPR Marimba
PT|Marlin Perkins 1
PT|Marlin Perkins 2
PT|Metal Bar
PT|Metallic Glass
PT|Metallic Pattern Gen
PT|Mutable Bowl
PT|Mutable Bowl.9
PT|OvertoneBar
PT|Plinklies
PT|Pluck Tine
PT|Pulsar
PT|Remembrance Bells
PT|Sine Kalimba
PT|Small Cloud Gamelan
PT|Spectral Marimba
PT|Spinning Metal Rings
PT|Spring Shimming Bell
PT|Steel Pan Kalimba
PT|Sympathetica
PT|Tick Tube
PT|Timbre Morph
PT|Tremolo Resonator
PT|Tubular Bells
PT|Uki Bells
PT|Zuie
PT|mBiraski
PT|mBiraski Ch1
PR|Ambient Delay
PR|Analog Echo
PR|Analog Echo Ext
PR|Chorus1
PR|Chorus2
PR|Chorus3
PR|Effect Modman
PR|Effect Modman 1
PR|FDN Base
PR|FDN Space
PR|Flanger
PR|Harmonic Resonator
PR|Karplus Effect
PR|Kinetic Friction
PR|Leslie
PR|MtoStereo Delay
PR|Multitap
PR|Phaser
PR|Pitch Shifter
PR|RingMod Voice
PR|Shaper
PR|The Satur
PR|Tuned Reverb
DO|Bad Weather
DO|Countdown
DO|Drum-Machine
DO|Earthquake
DO|Feelings Drone
DO|Hal Dreams
DO|Hals Meditation
DO|Kinetic Evil Bell
DO|Magic Carillon
DO|Magic Carillon - Var1
DO|Magic Carillon - Var2
DO|Roller
DO|Rosetta Alarm
DO|SineSpray Rain
DO|Splat
DO|Throat Battle
DO|Two Chords
DO|TwoOneDrone
)HAKEN";

juce::String hakenCategory (const juce::String& code)
{
    if (code == "MD") return "Midi";
    if (code == "CV") return "CVC";
    if (code == "UT") return "Utility";
    if (code == "USR") return "User";
    if (code == "ST") return "Strings";
    if (code == "WI") return "Winds";
    if (code == "VO") return "Vocal";
    if (code == "KY") return "Keyboard";
    if (code == "CL") return "Classic";
    if (code == "PE") return "Percussion";
    if (code == "PT") return "Tuned Percussion";
    if (code == "PR") return "Processor";
    if (code == "DO") return "Drone";
    return "Other";
}

void setPresetValue (Values& values, const char* id, float value)
{
    const juce::String key (id);
    for (auto& entry : values)
    {
        if (entry.first == key)
        {
            entry.second = value;
            return;
        }
    }
    values.emplace_back (key, value);
}

Values hakenValues (const juce::String& code, const juce::String& name)
{
    Values values;
    const auto hash = (uint32_t) name.hashCode();
    const auto vary = [hash] (int range)
    {
        return (float) (hash % (uint32_t) juce::jmax (1, range));
    };

    setPresetValue (values, "con.master.volume", 0.58f + 0.04f * vary (3));
    setPresetValue (values, "con.master.yDepth", 0.65f);
    setPresetValue (values, "con.master.zDepth", 0.82f);
    setPresetValue (values, "con.master.reverbMix", 0.26f + 0.008f * vary (6));
    setPresetValue (values, "con.master.reverbSize", 0.72f);
    setPresetValue (values, "con.master.delayMix", 0.10f);
    setPresetValue (values, "con.master.delayTime", 0.28f);
    setPresetValue (values, "con.master.delayFeedback", 0.28f);
    setPresetValue (values, "con.osc1.wave", 0.0f);
    setPresetValue (values, "con.osc1.level", 0.58f);
    setPresetValue (values, "con.osc2.wave", 1.0f);
    setPresetValue (values, "con.osc2.ratio", 2.0f);
    setPresetValue (values, "con.osc2.level", 0.28f);
    setPresetValue (values, "con.osc3.wave", 0.0f);
    setPresetValue (values, "con.osc3.ratio", 3.0f);
    setPresetValue (values, "con.osc3.level", 0.16f);
    setPresetValue (values, "con.osc4.level", 0.0f);
    setPresetValue (values, "con.osc5.level", 0.0f);
    setPresetValue (values, "con.noise.level", 0.0f);
    setPresetValue (values, "con.flt1.cutoff", 3200.0f + 200.0f * vary (8));
    setPresetValue (values, "con.flt2.cutoff", 3800.0f);
    setPresetValue (values, "con.flt3.cutoff", 4400.0f);
    setPresetValue (values, "con.flt4.cutoff", 5000.0f);
    setPresetValue (values, "con.flt5.cutoff", 5600.0f);
    setPresetValue (values, "con.bank1.freq", 440.0f);
    setPresetValue (values, "con.bank1.spread", 0.55f);
    setPresetValue (values, "con.bank1.bandwidth", 0.16f);
    setPresetValue (values, "con.bank1.mix", 0.22f);
    setPresetValue (values, "con.bank1.level", 0.72f);
    setPresetValue (values, "con.bank2.freq", 880.0f);
    setPresetValue (values, "con.bank2.bandwidth", 0.14f);
    setPresetValue (values, "con.bank2.mix", 0.14f);
    setPresetValue (values, "con.bank2.level", 0.65f);
    setPresetValue (values, "con.env1.attack", 0.006f + 0.004f * vary (5));
    setPresetValue (values, "con.env1.decay", 0.55f + 0.08f * vary (5));
    setPresetValue (values, "con.env1.sustain", 0.45f);
    setPresetValue (values, "con.env1.release", 1.1f);
    setPresetValue (values, "con.env2.attack", 0.004f);
    setPresetValue (values, "con.env2.decay", 0.45f);
    setPresetValue (values, "con.env2.sustain", 0.15f);
    setPresetValue (values, "con.env2.release", 0.9f);

    if (name.containsIgnoreCase ("organ"))
    {
        // Additive organ: no saw, full sustain, octave/fifth layers.
        setPresetValue (values, "con.master.volume", 0.62f);
        setPresetValue (values, "con.master.yDepth", 0.7f);
        setPresetValue (values, "con.master.zDepth", 0.85f);
        setPresetValue (values, "con.master.reverbMix", 0.16f);
        setPresetValue (values, "con.master.reverbSize", 0.6f);
        setPresetValue (values, "con.master.delayMix", 0.04f);
        setPresetValue (values, "con.osc1.wave", 0.0f);
        setPresetValue (values, "con.osc1.level", 0.58f);
        setPresetValue (values, "con.osc2.wave", 1.0f);
        setPresetValue (values, "con.osc2.ratio", 2.0f);
        setPresetValue (values, "con.osc2.level", 0.32f);
        setPresetValue (values, "con.osc3.wave", 0.0f);
        setPresetValue (values, "con.osc3.ratio", 0.5f);
        setPresetValue (values, "con.osc3.level", 0.5f);
        setPresetValue (values, "con.osc4.wave", 1.0f);
        setPresetValue (values, "con.osc4.ratio", 3.0f);
        setPresetValue (values, "con.osc4.level", 0.16f);
        setPresetValue (values, "con.osc5.wave", 0.0f);
        setPresetValue (values, "con.osc5.ratio", 4.0f);
        setPresetValue (values, "con.osc5.level", 0.1f);
        setPresetValue (values, "con.noise.level", 0.003f);
        setPresetValue (values, "con.flt1.cutoff", 7500.0f);
        setPresetValue (values, "con.flt1.res", 0.04f);
        setPresetValue (values, "con.flt2.cutoff", 8500.0f);
        setPresetValue (values, "con.flt3.cutoff", 9500.0f);
        setPresetValue (values, "con.flt4.cutoff", 10500.0f);
        setPresetValue (values, "con.flt5.cutoff", 11500.0f);
        setPresetValue (values, "con.bank1.freq", 440.0f);
        setPresetValue (values, "con.bank1.mix", 0.12f);
        setPresetValue (values, "con.bank2.freq", 880.0f);
        setPresetValue (values, "con.bank2.mix", 0.10f);
        setPresetValue (values, "con.env1.attack", 0.004f);
        setPresetValue (values, "con.env1.decay", 0.1f);
        setPresetValue (values, "con.env1.sustain", 1.0f);
        setPresetValue (values, "con.env1.release", 0.2f);
        setPresetValue (values, "con.env2.attack", 0.004f);
        setPresetValue (values, "con.env2.decay", 0.1f);
        setPresetValue (values, "con.env2.sustain", 1.0f);
        setPresetValue (values, "con.env2.release", 0.2f);
        return values;
    }

    if (name.containsIgnoreCase ("koto"))
    {
        // Plucked Chinese/Japanese zither body: paired detuned strings,
        // low wooden cavity banks, filter envelope and a longer string ring.
        setPresetValue (values, "con.master.volume", 0.64f);
        setPresetValue (values, "con.master.yDepth", 0.70f);
        setPresetValue (values, "con.master.zDepth", 0.9f);
        setPresetValue (values, "con.master.reverbMix", 0.34f);
        setPresetValue (values, "con.master.reverbSize", 0.84f);
        setPresetValue (values, "con.master.delayMix", 0.14f);
        setPresetValue (values, "con.master.delayTime", 0.25f);
        setPresetValue (values, "con.master.delayFeedback", 0.28f);
        setPresetValue (values, "con.osc1.wave", 1.0f);
        setPresetValue (values, "con.osc1.level", 0.56f);
        setPresetValue (values, "con.osc2.wave", 1.0f);
        setPresetValue (values, "con.osc2.ratio", 1.0f);
        setPresetValue (values, "con.osc2.fine", 3.0f);
        setPresetValue (values, "con.osc2.level", 0.30f);
        setPresetValue (values, "con.osc3.wave", 1.0f);
        setPresetValue (values, "con.osc3.ratio", 2.0f);
        setPresetValue (values, "con.osc3.fine", -2.0f);
        setPresetValue (values, "con.osc3.level", 0.18f);
        setPresetValue (values, "con.osc4.wave", 0.0f);
        setPresetValue (values, "con.osc4.ratio", 0.5f);
        setPresetValue (values, "con.osc4.level", 0.38f);
        setPresetValue (values, "con.osc5.wave", 1.0f);
        setPresetValue (values, "con.osc5.ratio", 3.0f);
        setPresetValue (values, "con.osc5.level", 0.10f);
        setPresetValue (values, "con.noise.level", 0.025f);
        setPresetValue (values, "con.noise.color", 0.55f);
        setPresetValue (values, "con.flt1.cutoff", 4500.0f);
        setPresetValue (values, "con.flt1.res", 0.20f);
        setPresetValue (values, "con.flt1.env", 0.35f);
        setPresetValue (values, "con.flt2.cutoff", 5200.0f);
        setPresetValue (values, "con.flt2.env", 0.25f);
        setPresetValue (values, "con.flt3.cutoff", 6000.0f);
        setPresetValue (values, "con.flt4.cutoff", 6800.0f);
        setPresetValue (values, "con.flt5.cutoff", 7600.0f);
        setPresetValue (values, "con.bank1.freq", 196.0f);
        setPresetValue (values, "con.bank1.spread", 0.58f);
        setPresetValue (values, "con.bank1.bandwidth", 0.12f);
        setPresetValue (values, "con.bank1.mix", 0.46f);
        setPresetValue (values, "con.bank1.level", 0.88f);
        setPresetValue (values, "con.bank2.freq", 392.0f);
        setPresetValue (values, "con.bank2.spread", 0.54f);
        setPresetValue (values, "con.bank2.bandwidth", 0.11f);
        setPresetValue (values, "con.bank2.mix", 0.32f);
        setPresetValue (values, "con.bank2.level", 0.78f);
        setPresetValue (values, "con.env1.attack", 0.0010f);
        setPresetValue (values, "con.env1.decay", 1.05f);
        setPresetValue (values, "con.env1.sustain", 0.05f);
        setPresetValue (values, "con.env1.release", 1.8f);
        setPresetValue (values, "con.env2.attack", 0.001f);
        setPresetValue (values, "con.env2.decay", 0.45f);
        setPresetValue (values, "con.env2.sustain", 0.0f);
        setPresetValue (values, "con.env2.release", 1.1f);
        return values;
    }

    if (name.containsIgnoreCase ("harp"))
    {
        // Full-bodied plucked/electric harp base: sub-octave body, two
        // resonant banks and a short amplitude envelope.
        setPresetValue (values, "con.master.volume", 0.68f);
        setPresetValue (values, "con.master.yDepth", 0.68f);
        setPresetValue (values, "con.master.zDepth", 0.9f);
        setPresetValue (values, "con.master.reverbMix", 0.34f);
        setPresetValue (values, "con.master.reverbSize", 0.84f);
        setPresetValue (values, "con.master.delayMix", 0.14f);
        setPresetValue (values, "con.master.delayTime", 0.25f);
        setPresetValue (values, "con.osc1.wave", 0.0f);
        setPresetValue (values, "con.osc1.level", 0.68f);
        setPresetValue (values, "con.osc2.wave", 1.0f);
        setPresetValue (values, "con.osc2.ratio", 1.0f);
        setPresetValue (values, "con.osc2.fine", 0.06f);
        setPresetValue (values, "con.osc2.level", 0.48f);
        setPresetValue (values, "con.osc3.wave", 0.0f);
        setPresetValue (values, "con.osc3.ratio", 0.5f);
        setPresetValue (values, "con.osc3.level", 0.42f);
        setPresetValue (values, "con.osc4.wave", 1.0f);
        setPresetValue (values, "con.osc4.ratio", 2.0f);
        setPresetValue (values, "con.osc4.level", 0.22f);
        setPresetValue (values, "con.osc5.wave", 0.0f);
        setPresetValue (values, "con.osc5.ratio", 3.0f);
        setPresetValue (values, "con.osc5.level", 0.12f);
        setPresetValue (values, "con.noise.level", 0.008f);
        setPresetValue (values, "con.flt1.cutoff", 4200.0f);
        setPresetValue (values, "con.flt1.res", 0.16f);
        setPresetValue (values, "con.flt2.cutoff", 5000.0f);
        setPresetValue (values, "con.flt3.cutoff", 5800.0f);
        setPresetValue (values, "con.flt4.cutoff", 6600.0f);
        setPresetValue (values, "con.flt5.cutoff", 7400.0f);
        setPresetValue (values, "con.bank1.freq", 261.63f);
        setPresetValue (values, "con.bank1.spread", 0.55f);
        setPresetValue (values, "con.bank1.bandwidth", 0.18f);
        setPresetValue (values, "con.bank1.mix", 0.50f);
        setPresetValue (values, "con.bank1.level", 0.85f);
        setPresetValue (values, "con.bank2.freq", 523.25f);
        setPresetValue (values, "con.bank2.spread", 0.52f);
        setPresetValue (values, "con.bank2.bandwidth", 0.14f);
        setPresetValue (values, "con.bank2.mix", 0.38f);
        setPresetValue (values, "con.bank2.level", 0.76f);
        setPresetValue (values, "con.env1.attack", 0.002f);
        setPresetValue (values, "con.env1.decay", 0.78f);
        setPresetValue (values, "con.env1.sustain", 0.12f);
        setPresetValue (values, "con.env1.release", 1.9f);
        setPresetValue (values, "con.env2.attack", 0.001f);
        setPresetValue (values, "con.env2.decay", 0.35f);
        setPresetValue (values, "con.env2.sustain", 0.0f);
        setPresetValue (values, "con.env2.release", 1.0f);
        return values;
    }

    if (code == "ST")
    {
        setPresetValue (values, "con.osc1.wave", 1.0f);
        setPresetValue (values, "con.osc1.level", 0.58f);
        setPresetValue (values, "con.osc2.wave", 2.0f);
        setPresetValue (values, "con.osc2.fine", 0.05f);
        setPresetValue (values, "con.osc2.level", 0.34f);
        setPresetValue (values, "con.osc3.ratio", 2.0f);
        setPresetValue (values, "con.osc3.level", 0.2f);
        setPresetValue (values, "con.flt1.cutoff", 1800.0f + 120.0f * vary (6));
        setPresetValue (values, "con.flt1.res", 0.22f);
        setPresetValue (values, "con.bank1.freq", 440.0f);
        setPresetValue (values, "con.bank1.mix", 0.28f);
        setPresetValue (values, "con.env1.attack", 0.20f + 0.10f * vary (5));
        setPresetValue (values, "con.env1.decay", 1.1f);
        setPresetValue (values, "con.env1.sustain", 0.72f);
        setPresetValue (values, "con.env1.release", 2.5f);
        setPresetValue (values, "con.master.reverbMix", 0.38f);
    }
    else if (code == "WI")
    {
        setPresetValue (values, "con.noise.level", 0.15f);
        setPresetValue (values, "con.noise.color", 0.32f);
        setPresetValue (values, "con.osc1.wave", 0.0f);
        setPresetValue (values, "con.osc2.wave", 4.0f);
        setPresetValue (values, "con.osc2.ratio", 2.0f);
        setPresetValue (values, "con.osc2.level", 0.18f);
        setPresetValue (values, "con.flt1.type", 2.0f);
        setPresetValue (values, "con.flt1.cutoff", 2200.0f);
        setPresetValue (values, "con.flt1.res", 0.28f);
        setPresetValue (values, "con.env1.attack", 0.10f);
        setPresetValue (values, "con.env1.sustain", 0.82f);
        setPresetValue (values, "con.env1.release", 1.2f);
    }
    else if (code == "VO")
    {
        setPresetValue (values, "con.osc1.wave", 0.0f);
        setPresetValue (values, "con.osc2.wave", 0.0f);
        setPresetValue (values, "con.osc2.ratio", 2.0f);
        setPresetValue (values, "con.osc2.level", 0.22f);
        setPresetValue (values, "con.bank1.freq", 500.0f);
        setPresetValue (values, "con.bank1.bandwidth", 0.15f);
        setPresetValue (values, "con.bank1.mix", 0.48f);
        setPresetValue (values, "con.bank2.freq", 1500.0f);
        setPresetValue (values, "con.bank2.mix", 0.38f);
        setPresetValue (values, "con.env1.attack", 0.30f);
        setPresetValue (values, "con.env1.sustain", 0.86f);
        setPresetValue (values, "con.env1.release", 2.0f);
        setPresetValue (values, "con.master.reverbMix", 0.42f);
    }
    else if (code == "KY" || code == "CL")
    {
        setPresetValue (values, "con.osc1.wave", code == "KY" ? 0.0f : 2.0f);
        setPresetValue (values, "con.osc2.wave", code == "KY" ? 1.0f : 3.0f);
        setPresetValue (values, "con.osc2.ratio", 2.0f);
        setPresetValue (values, "con.osc2.level", 0.3f);
        setPresetValue (values, "con.flt1.cutoff", code == "KY" ? 6500.0f : 1800.0f);
        setPresetValue (values, "con.flt1.res", 0.18f);
        setPresetValue (values, "con.env1.attack", code == "KY" ? 0.004f : 0.005f);
        setPresetValue (values, "con.env1.decay", code == "KY" ? 0.65f : 0.42f);
        setPresetValue (values, "con.env1.sustain", code == "KY" ? 0.18f : 0.4f);
        setPresetValue (values, "con.env1.release", code == "KY" ? 1.0f : 0.7f);
        setPresetValue (values, "con.master.reverbMix", 0.26f);
    }
    else if (code == "PE")
    {
        setPresetValue (values, "con.noise.level", 0.14f);
        setPresetValue (values, "con.noise.color", 0.56f);
        setPresetValue (values, "con.osc1.wave", 2.0f);
        setPresetValue (values, "con.osc2.wave", 3.0f);
        setPresetValue (values, "con.osc2.level", 0.22f);
        setPresetValue (values, "con.flt1.cutoff", 3600.0f);
        setPresetValue (values, "con.flt1.res", 0.35f);
        setPresetValue (values, "con.env1.attack", 0.001f);
        setPresetValue (values, "con.env1.decay", 0.22f);
        setPresetValue (values, "con.env1.sustain", 0.0f);
        setPresetValue (values, "con.env1.release", 0.35f);
        setPresetValue (values, "con.master.reverbMix", 0.3f);
    }
    else if (code == "PT")
    {
        setPresetValue (values, "con.osc1.wave", 0.0f);
        setPresetValue (values, "con.osc2.wave", 0.0f);
        setPresetValue (values, "con.osc2.ratio", 2.0f);
        setPresetValue (values, "con.osc2.level", 0.28f);
        setPresetValue (values, "con.bank1.freq", 880.0f);
        setPresetValue (values, "con.bank1.bandwidth", 0.09f);
        setPresetValue (values, "con.bank1.mix", 0.36f);
        setPresetValue (values, "con.flt1.cutoff", 7200.0f);
        setPresetValue (values, "con.env1.attack", 0.001f);
        setPresetValue (values, "con.env1.decay", 0.65f);
        setPresetValue (values, "con.env1.sustain", 0.0f);
        setPresetValue (values, "con.env1.release", 1.2f);
        setPresetValue (values, "con.master.reverbMix", 0.38f);
    }
    else if (code == "PR")
    {
        setPresetValue (values, "con.osc1.wave", 2.0f);
        setPresetValue (values, "con.osc2.wave", 4.0f);
        setPresetValue (values, "con.flt1.cutoff", 1600.0f);
        setPresetValue (values, "con.flt1.res", 0.52f);
        setPresetValue (values, "con.master.reverbMix", 0.52f);
        setPresetValue (values, "con.master.delayMix", 0.32f);
        setPresetValue (values, "con.env1.attack", 0.02f);
        setPresetValue (values, "con.env1.sustain", 0.55f);
        setPresetValue (values, "con.env1.release", 2.0f);
    }
    else if (code == "DO")
    {
        setPresetValue (values, "con.osc1.wave", 1.0f);
        setPresetValue (values, "con.osc1.level", 0.5f);
        setPresetValue (values, "con.osc2.wave", 2.0f);
        setPresetValue (values, "con.osc2.level", 0.28f);
        setPresetValue (values, "con.flt1.cutoff", 850.0f);
        setPresetValue (values, "con.flt1.res", 0.34f);
        setPresetValue (values, "con.env1.attack", 1.8f);
        setPresetValue (values, "con.env1.sustain", 0.9f);
        setPresetValue (values, "con.env1.release", 4.0f);
        setPresetValue (values, "con.master.reverbMix", 0.5f);
    }
    else if (code == "MD" || code == "CV")
    {
        setPresetValue (values, "con.osc1.wave", 0.0f);
        setPresetValue (values, "con.osc2.level", 0.0f);
        setPresetValue (values, "con.osc3.level", 0.0f);
        setPresetValue (values, "con.flt1.cutoff", 9000.0f);
        setPresetValue (values, "con.env1.attack", 0.005f);
        setPresetValue (values, "con.env1.sustain", 1.0f);
        setPresetValue (values, "con.env1.release", 0.2f);
        setPresetValue (values, "con.master.reverbMix", 0.0f);
        setPresetValue (values, "con.master.delayMix", 0.0f);
    }
    else if (code == "UT" || code == "USR")
    {
        setPresetValue (values, "con.osc1.wave", 2.0f);
        setPresetValue (values, "con.osc2.wave", 1.0f);
        setPresetValue (values, "con.osc2.ratio", 2.0f);
        setPresetValue (values, "con.env1.attack", 0.01f);
        setPresetValue (values, "con.env1.sustain", 0.75f);
        setPresetValue (values, "con.env1.release", 1.2f);
    }
    else // OT and any unclassified names
    {
        setPresetValue (values, "con.osc1.wave", (float) (hash % 5));
        setPresetValue (values, "con.osc2.ratio", 1.0f + 0.5f * vary (4));
        setPresetValue (values, "con.osc2.level", 0.24f);
        setPresetValue (values, "con.flt1.cutoff", 1200.0f + 300.0f * vary (10));
        setPresetValue (values, "con.flt1.res", 0.28f);
        setPresetValue (values, "con.env1.attack", 0.02f + 0.05f * vary (8));
        setPresetValue (values, "con.env1.decay", 0.8f);
        setPresetValue (values, "con.env1.sustain", 0.55f);
        setPresetValue (values, "con.env1.release", 2.2f);
        setPresetValue (values, "con.master.reverbMix", 0.4f);
    }

    return values;
}

std::vector<FactoryPreset> makeHakenBank()
{
    std::vector<FactoryPreset> presets;
    juce::StringArray lines;
    lines.addLines (hakenNamesRaw);

    for (const auto& line : lines)
    {
        const auto split = line.indexOfChar ('|');
        if (split <= 0)
            continue;

        const auto code = line.substring (0, split).trim();
        const auto name = line.substring (split + 1).trim();
        if (code.isEmpty() || name.isEmpty())
            continue;

        const auto fullName = code + " - " + name;
        presets.push_back ({ fullName, hakenCategory (code), hakenValues (code, name) });
    }

    return presets;
}

std::vector<FactoryPreset> continuumCustomPresets()
{
    return sortedPresets ({
        { "BA - Reese Surface", "Bass",
          Values {
              { "con.master.volume", 0.66f }, { "con.master.glide", 0.018f },
              { "con.master.yDepth", 0.45f }, { "con.master.zDepth", 0.8f },
              { "con.master.reverbMix", 0.08f }, { "con.master.delayMix", 0.05f },
              { "con.osc1.ratio", 1.0f }, { "con.osc1.wave", 2.0f },
              { "con.osc1.level", 0.7f }, { "con.osc2.ratio", 1.0f },
              { "con.osc2.wave", 2.0f }, { "con.osc2.fine", 0.12f },
              { "con.osc2.level", 0.6f }, { "con.osc3.ratio", 0.5f },
              { "con.osc3.level", 0.45f }, { "con.osc4.ratio", 1.5f },
              { "con.osc4.level", 0.25f }, { "con.osc5.level", 0.0f },
              { "con.flt1.cutoff", 430.0f }, { "con.flt1.res", 0.22f },
              { "con.flt2.cutoff", 520.0f }, { "con.flt2.res", 0.18f },
              { "con.flt3.cutoff", 700.0f }, { "con.flt4.cutoff", 900.0f },
              { "con.flt5.cutoff", 1200.0f },
              { "con.bank1.freq", 220.0f }, { "con.bank1.mix", 0.15f },
              { "con.bank2.freq", 330.0f }, { "con.bank2.mix", 0.12f },
              { "con.env1.attack", 0.004f }, { "con.env1.decay", 0.45f },
              { "con.env1.sustain", 0.92f }, { "con.env1.release", 0.35f },
              { "con.env2.attack", 0.01f }, { "con.env2.decay", 0.4f },
              { "con.env2.sustain", 0.3f }, { "con.env2.release", 0.4f }
          } },
        { "BA - Sub Surface", "Bass",
          Values {
              { "con.master.volume", 0.68f }, { "con.master.glide", 0.012f },
              { "con.master.reverbMix", 0.05f }, { "con.master.delayMix", 0.03f },
              { "con.osc1.ratio", 1.0f }, { "con.osc1.wave", 2.0f },
              { "con.osc1.level", 0.75f }, { "con.osc2.ratio", 0.5f },
              { "con.osc2.wave", 2.0f }, { "con.osc2.level", 0.5f },
              { "con.osc3.ratio", 0.25f }, { "con.osc3.wave", 0.0f },
              { "con.osc3.level", 0.4f }, { "con.osc4.level", 0.15f },
              { "con.osc5.level", 0.0f },
              { "con.flt1.cutoff", 250.0f }, { "con.flt1.res", 0.2f },
              { "con.flt2.cutoff", 340.0f }, { "con.flt3.cutoff", 430.0f },
              { "con.flt4.cutoff", 520.0f }, { "con.flt5.cutoff", 650.0f },
              { "con.bank1.freq", 110.0f }, { "con.bank1.mix", 0.12f },
              { "con.bank2.freq", 165.0f }, { "con.bank2.mix", 0.1f },
              { "con.env1.attack", 0.003f }, { "con.env1.decay", 0.4f },
              { "con.env1.sustain", 0.9f }, { "con.env1.release", 0.25f },
              { "con.env2.attack", 0.002f }, { "con.env2.decay", 0.3f },
              { "con.env2.sustain", 0.2f }, { "con.env2.release", 0.2f }
          } },
        { "CL - Clinical Oscillator 1", "Classic",
          Values {
              { "con.master.volume", 0.68f }, { "con.master.glide", 0.09f },
              { "con.master.yDepth", 0.7f }, { "con.master.reverbMix", 0.18f },
              { "con.master.delayMix", 0.2f }, { "con.master.delayTime", 0.28f },
              { "con.osc1.ratio", 1.0f }, { "con.osc1.wave", 2.0f },
              { "con.osc1.level", 0.75f }, { "con.osc2.ratio", 1.0f },
              { "con.osc2.fine", 0.07f }, { "con.osc2.wave", 2.0f },
              { "con.osc2.level", 0.5f }, { "con.osc3.wave", 4.0f },
              { "con.osc3.ratio", 2.0f }, { "con.osc3.level", 0.25f },
              { "con.flt1.cutoff", 2400.0f }, { "con.flt1.res", 0.35f },
              { "con.flt2.cutoff", 3000.0f }, { "con.flt2.res", 0.2f },
              { "con.flt3.cutoff", 1800.0f }, { "con.flt4.cutoff", 2600.0f },
              { "con.flt5.cutoff", 3400.0f },
              { "con.env1.attack", 0.008f }, { "con.env1.decay", 0.5f },
              { "con.env1.sustain", 0.8f }, { "con.env1.release", 0.35f },
              { "con.sg1.rate", 4.5f }, { "con.sg1.shape", 6.0f },
              { "con.sg1.level", 0.07f }
          } },
        { "KY - LegacyDX EP", "Keyboard",
          Values {
              { "con.master.volume", 0.64f }, { "con.master.glide", 0.04f },
              { "con.osc1.wave", 0.0f }, { "con.osc1.level", 0.65f },
              { "con.osc2.ratio", 2.0f }, { "con.osc2.wave", 0.0f },
              { "con.osc2.level", 0.45f }, { "con.osc3.ratio", 3.0f },
              { "con.osc3.wave", 4.0f }, { "con.osc3.level", 0.2f },
              { "con.flt1.cutoff", 4200.0f }, { "con.flt1.res", 0.2f },
              { "con.flt2.cutoff", 5200.0f }, { "con.flt3.cutoff", 6200.0f },
              { "con.bank1.freq", 880.0f }, { "con.bank1.mix", 0.35f },
              { "con.bank1.bandwidth", 0.2f }, { "con.bank2.freq", 1320.0f },
              { "con.bank2.mix", 0.25f },
              { "con.env1.attack", 0.004f }, { "con.env1.decay", 0.8f },
              { "con.env1.sustain", 0.6f }, { "con.env1.release", 0.8f },
              { "con.master.reverbMix", 0.3f }
          } },
        { "CL - Analog ADSR", "Classic",
          Values {
              { "con.master.volume", 0.64f }, { "con.master.yDepth", 0.55f },
              { "con.osc1.wave", 2.0f }, { "con.osc1.level", 0.7f },
              { "con.osc2.wave", 2.0f }, { "con.osc2.ratio", 1.0f },
              { "con.osc2.fine", 0.05f }, { "con.osc2.level", 0.45f },
              { "con.osc3.wave", 2.0f }, { "con.osc3.ratio", 2.0f },
              { "con.osc3.level", 0.25f },
              { "con.flt1.cutoff", 1500.0f }, { "con.flt1.res", 0.2f },
              { "con.flt1.env", 0.45f }, { "con.flt2.cutoff", 1900.0f },
              { "con.flt3.cutoff", 2200.0f }, { "con.flt4.cutoff", 2600.0f },
              { "con.flt5.cutoff", 3000.0f },
              { "con.env1.attack", 0.03f }, { "con.env1.decay", 0.8f },
              { "con.env1.sustain", 0.8f }, { "con.env1.release", 0.5f },
              { "con.env2.attack", 0.02f }, { "con.env2.decay", 0.6f },
              { "con.env2.sustain", 0.4f }, { "con.env2.release", 0.5f },
              { "con.master.reverbMix", 0.2f }
          } },
        { "DO - Feelings Drone", "Drone",
          Values {
              { "con.master.volume", 0.62f }, { "con.master.yDepth", 0.75f },
              { "con.master.reverbMix", 0.42f }, { "con.master.reverbSize", 0.85f },
              { "con.master.delayMix", 0.22f },
              { "con.osc1.wave", 1.0f }, { "con.osc1.level", 0.55f },
              { "con.osc2.wave", 2.0f }, { "con.osc2.fine", 0.08f },
              { "con.osc2.level", 0.45f }, { "con.osc3.wave", 1.0f },
              { "con.osc3.ratio", 0.5f }, { "con.osc3.level", 0.3f },
              { "con.flt1.cutoff", 750.0f }, { "con.flt1.res", 0.18f },
              { "con.flt2.cutoff", 950.0f }, { "con.flt2.res", 0.22f },
              { "con.flt3.cutoff", 1200.0f }, { "con.flt4.cutoff", 1500.0f },
              { "con.flt5.cutoff", 1800.0f },
              { "con.env1.attack", 1.8f }, { "con.env1.decay", 2.5f },
              { "con.env1.sustain", 0.9f }, { "con.env1.release", 3.8f },
              { "con.env2.attack", 1.2f }, { "con.env2.decay", 2.0f },
              { "con.env2.sustain", 0.6f }, { "con.env2.release", 3.0f },
              { "con.sg1.rate", 0.12f }, { "con.sg1.level", 0.3f },
              { "con.sg2.rate", 0.07f }, { "con.sg2.level", 0.2f }
          } },
        { "VO - Choir", "Vocal",
          Values {
              { "con.master.volume", 0.6f }, { "con.master.reverbMix", 0.5f },
              { "con.master.reverbSize", 0.9f }, { "con.osc1.wave", 0.0f },
              { "con.osc1.level", 0.5f }, { "con.osc2.wave", 0.0f },
              { "con.osc2.ratio", 1.0f }, { "con.osc2.fine", 0.1f },
              { "con.osc2.level", 0.4f }, { "con.osc3.wave", 0.0f },
              { "con.osc3.ratio", 2.0f }, { "con.osc3.level", 0.2f },
              { "con.bank1.freq", 330.0f }, { "con.bank1.mix", 0.45f },
              { "con.bank1.bandwidth", 0.15f }, { "con.bank2.freq", 660.0f },
              { "con.bank2.mix", 0.35f },
              { "con.flt1.cutoff", 1100.0f }, { "con.flt2.cutoff", 1600.0f },
              { "con.flt3.cutoff", 2200.0f }, { "con.flt4.cutoff", 2800.0f },
              { "con.flt5.cutoff", 3400.0f },
              { "con.env1.attack", 2.4f }, { "con.env1.sustain", 0.95f },
              { "con.env1.release", 4.5f },
              { "con.sg3.rate", 0.09f }, { "con.sg3.level", 0.2f }
          } },
        { "KY - Ice Calliope", "Keyboard",
          Values {
              { "con.master.volume", 0.64f }, { "con.master.yDepth", 0.8f },
              { "con.master.reverbMix", 0.3f }, { "con.master.delayMix", 0.18f },
              { "con.osc1.wave", 0.0f }, { "con.osc1.level", 0.55f },
              { "con.osc2.ratio", 2.0f }, { "con.osc2.wave", 0.0f },
              { "con.osc2.level", 0.35f }, { "con.osc3.ratio", 3.0f },
              { "con.osc3.wave", 4.0f }, { "con.osc3.level", 0.18f },
              { "con.flt1.cutoff", 5000.0f }, { "con.flt2.cutoff", 6000.0f },
              { "con.flt3.cutoff", 7000.0f }, { "con.flt4.cutoff", 8000.0f },
              { "con.flt5.cutoff", 9000.0f },
              { "con.bank1.freq", 1200.0f }, { "con.bank1.mix", 0.3f },
              { "con.bank2.freq", 2400.0f }, { "con.bank2.mix", 0.25f },
              { "con.env1.attack", 0.01f }, { "con.env1.decay", 1.2f },
              { "con.env1.sustain", 0.7f }, { "con.env1.release", 1.5f }
          } },
        { "PE - Creaker", "Percussion",
          Values {
              { "con.master.volume", 0.6f }, { "con.master.zDepth", 0.95f },
              { "con.master.yDepth", 0.7f }, { "con.master.reverbMix", 0.38f },
              { "con.osc1.wave", 2.0f }, { "con.osc1.level", 0.6f },
              { "con.osc2.wave", 3.0f }, { "con.osc2.fine", 0.15f },
              { "con.osc2.level", 0.4f }, { "con.osc3.ratio", 0.5f },
              { "con.osc3.level", 0.25f },
              { "con.flt1.cutoff", 1300.0f }, { "con.flt1.res", 0.45f },
              { "con.flt1.env", 0.5f }, { "con.flt2.cutoff", 1700.0f },
              { "con.flt3.cutoff", 2100.0f }, { "con.flt4.cutoff", 2500.0f },
              { "con.flt5.cutoff", 3000.0f },
              { "con.env1.attack", 0.06f }, { "con.env1.decay", 0.9f },
              { "con.env1.sustain", 0.75f }, { "con.env1.release", 1.2f },
              { "con.sg1.rate", 0.4f }, { "con.sg1.level", 0.3f }
          } },
        { "OT - Entanglement", "Other",
          Values {
              { "con.master.volume", 0.58f }, { "con.master.reverbMix", 0.45f },
              { "con.master.reverbSize", 0.92f }, { "con.master.delayMix", 0.25f },
              { "con.osc1.wave", 4.0f }, { "con.osc1.level", 0.45f },
              { "con.osc2.wave", 3.0f }, { "con.osc2.ratio", 1.5f },
              { "con.osc2.level", 0.35f }, { "con.osc3.wave", 0.0f },
              { "con.osc3.ratio", 3.0f }, { "con.osc3.level", 0.2f },
              { "con.bank1.freq", 520.0f }, { "con.bank1.spread", 0.8f },
              { "con.bank1.mix", 0.55f }, { "con.bank1.bandwidth", 0.2f },
              { "con.bank2.freq", 1100.0f }, { "con.bank2.spread", 0.7f },
              { "con.bank2.mix", 0.45f },
              { "con.flt1.cutoff", 2000.0f }, { "con.flt1.res", 0.5f },
              { "con.flt2.cutoff", 2800.0f }, { "con.flt3.cutoff", 3400.0f },
              { "con.flt4.cutoff", 4000.0f }, { "con.flt5.cutoff", 4800.0f },
              { "con.env1.attack", 1.0f }, { "con.env1.sustain", 0.8f },
              { "con.env1.release", 3.0f },
              { "con.sg1.rate", 0.15f }, { "con.sg1.level", 0.35f },
              { "con.sg2.rate", 0.23f }, { "con.sg2.level", 0.25f }
          } },
        { "ST - Bowed Mood", "Strings",
          Values {
              { "con.master.volume", 0.62f }, { "con.master.yDepth", 0.75f },
              { "con.master.reverbMix", 0.35f }, { "con.osc1.wave", 1.0f },
              { "con.osc1.level", 0.55f }, { "con.osc2.wave", 2.0f },
              { "con.osc2.fine", 0.06f }, { "con.osc2.level", 0.4f },
              { "con.osc3.wave", 1.0f }, { "con.osc3.ratio", 2.0f },
              { "con.osc3.level", 0.25f },
              { "con.flt1.cutoff", 1400.0f }, { "con.flt1.res", 0.25f },
              { "con.flt2.cutoff", 1800.0f }, { "con.flt3.cutoff", 2200.0f },
              { "con.flt4.cutoff", 2600.0f }, { "con.flt5.cutoff", 3000.0f },
              { "con.env1.attack", 0.8f }, { "con.env1.decay", 1.5f },
              { "con.env1.sustain", 0.7f }, { "con.env1.release", 2.5f },
              { "con.env2.attack", 0.5f }, { "con.env2.sustain", 0.5f },
              { "con.sg1.rate", 5.2f }, { "con.sg1.level", 0.04f },
              { "con.bank1.freq", 440.0f }, { "con.bank1.mix", 0.2f }
          } },
        { "OT - Celestial Basin", "Other",
          Values {
              { "con.master.volume", 0.56f }, { "con.master.glide", 0.2f },
              { "con.master.reverbMix", 0.58f }, { "con.master.reverbSize", 0.95f },
              { "con.master.delayMix", 0.38f }, { "con.master.delayTime", 0.7f },
              { "con.master.delayFeedback", 0.55f },
              { "con.osc1.wave", 0.0f }, { "con.osc1.level", 0.45f },
              { "con.osc2.wave", 0.0f }, { "con.osc2.ratio", 0.5f },
              { "con.osc2.level", 0.3f }, { "con.osc3.wave", 4.0f },
              { "con.osc3.ratio", 3.0f }, { "con.osc3.level", 0.15f },
              { "con.flt1.cutoff", 900.0f }, { "con.flt1.res", 0.6f },
              { "con.flt2.cutoff", 1300.0f }, { "con.flt3.cutoff", 1800.0f },
              { "con.flt4.cutoff", 2400.0f }, { "con.flt5.cutoff", 3200.0f },
              { "con.env1.attack", 1.5f }, { "con.env1.sustain", 0.85f },
              { "con.env1.release", 5.0f },
              { "con.sg1.rate", 0.08f }, { "con.sg1.level", 0.4f },
              { "con.sg2.rate", 0.11f }, { "con.sg2.level", 0.25f }
          },
          { { 0, 0, 1, 1.0f }, { 0, 5, 4, 0.4f }, { 1, 1, 1, 1.0f },
            { 2, 2, 1, 1.0f }, { 6, 15, 4, 0.3f }, { 7, 15, 1, 0.2f },
            { 8, 15, 1, 0.15f }, { 9, 15, 1, 0.12f }, { 10, 15, 1, 0.1f } } },
        { "PR - Harmonic Resonator", "Processor",
          Values {
              { "con.master.volume", 0.55f }, { "con.master.reverbMix", 0.4f },
              { "con.osc1.wave", 2.0f }, { "con.osc1.level", 0.6f },
              { "con.osc2.wave", 3.0f }, { "con.osc2.ratio", 1.5f },
              { "con.osc2.level", 0.5f }, { "con.osc3.wave", 4.0f },
              { "con.osc3.ratio", 0.5f }, { "con.osc3.level", 0.4f },
              { "con.flt1.cutoff", 800.0f }, { "con.flt1.res", 0.75f },
              { "con.flt2.cutoff", 1200.0f }, { "con.flt3.cutoff", 1600.0f },
              { "con.env1.attack", 0.02f }, { "con.env1.decay", 1.2f },
              { "con.env1.sustain", 0.3f }, { "con.env1.release", 1.5f },
              { "con.sg1.rate", 9.0f }, { "con.sg1.shape", 5.0f },
              { "con.sg1.level", 0.5f }
          },
          { { 0, 0, 1, 1.0f }, { 1, 1, 1, 0.8f }, { 2, 2, 7, 0.7f },
            { 6, 3, 1, 0.5f }, { 7, 4, 1, 0.4f }, { 8, 15, 4, 0.4f },
            { 9, 15, 2, 0.3f }, { 10, 15, 1, 0.25f }, { 11, 5, 11, 0.6f } } },
        { "UT - Empty", "Utility",
          Values {
              { "con.master.volume", 0.65f }, { "con.master.reverbMix", 0.22f },
              { "con.osc1.wave", 2.0f }, { "con.osc1.level", 0.6f },
              { "con.osc2.wave", 1.0f }, { "con.osc2.ratio", 2.0f },
              { "con.osc2.level", 0.35f }, { "con.osc3.wave", 0.0f },
              { "con.osc3.ratio", 3.0f }, { "con.osc3.level", 0.2f },
              { "con.flt1.cutoff", 1800.0f }, { "con.flt2.cutoff", 2400.0f },
              { "con.flt3.cutoff", 3000.0f }, { "con.flt4.cutoff", 3600.0f },
              { "con.flt5.cutoff", 4200.0f },
              { "con.env1.attack", 0.02f }, { "con.env1.decay", 0.8f },
              { "con.env1.sustain", 0.75f }, { "con.env1.release", 1.0f }
          } },
        { "WI - Wind", "Winds",
          Values {
              { "con.master.volume", 0.6f }, { "con.master.yDepth", 0.75f },
              { "con.master.reverbMix", 0.32f }, { "con.master.delayMix", 0.12f },
              { "con.noise.level", 0.2f }, { "con.noise.color", 0.3f },
              { "con.osc1.wave", 0.0f }, { "con.osc1.level", 0.5f },
              { "con.osc2.wave", 4.0f }, { "con.osc2.ratio", 2.0f },
              { "con.osc2.level", 0.22f }, { "con.osc3.wave", 0.0f },
              { "con.osc3.ratio", 0.5f }, { "con.osc3.level", 0.3f },
              { "con.flt1.cutoff", 2200.0f }, { "con.flt2.cutoff", 2800.0f },
              { "con.flt3.cutoff", 3400.0f }, { "con.flt4.cutoff", 4000.0f },
              { "con.flt5.cutoff", 4600.0f },
              { "con.env1.attack", 0.14f }, { "con.env1.decay", 0.8f },
              { "con.env1.sustain", 0.8f }, { "con.env1.release", 1.2f },
              { "con.env2.attack", 0.2f }, { "con.env2.sustain", 0.4f },
              { "con.sg1.rate", 5.2f }, { "con.sg1.level", 0.05f }
          } },
        { "WI - Double Reed", "Winds",
          Values {
              { "con.master.volume", 0.58f }, { "con.master.reverbMix", 0.38f },
              { "con.osc1.wave", 2.0f }, { "con.osc1.level", 0.5f },
              { "con.osc2.wave", 4.0f }, { "con.osc2.ratio", 1.0f },
              { "con.osc2.fine", 0.08f }, { "con.osc2.level", 0.35f },
              { "con.osc3.wave", 2.0f }, { "con.osc3.ratio", 2.0f },
              { "con.osc3.level", 0.2f }, { "con.noise.level", 0.08f },
              { "con.flt1.type", 2.0f }, { "con.flt1.cutoff", 1200.0f },
              { "con.flt1.res", 0.25f }, { "con.flt2.type", 2.0f },
              { "con.flt2.cutoff", 1600.0f }, { "con.flt3.type", 2.0f },
              { "con.flt3.cutoff", 2200.0f }, { "con.flt4.cutoff", 2800.0f },
              { "con.flt5.cutoff", 3400.0f },
              { "con.bank1.freq", 600.0f }, { "con.bank1.mix", 0.3f },
              { "con.env1.attack", 0.3f }, { "con.env1.decay", 1.2f },
              { "con.env1.sustain", 0.85f }, { "con.env1.release", 2.0f },
              { "con.env2.attack", 0.4f }, { "con.env2.sustain", 0.5f }
          } },
        { "VO - Vowel", "Vocal",
          Values {
              { "con.master.volume", 0.58f }, { "con.master.yDepth", 0.8f },
              { "con.master.reverbMix", 0.4f }, { "con.master.delayMix", 0.15f },
              { "con.osc1.wave", 0.0f }, { "con.osc1.level", 0.5f },
              { "con.osc2.wave", 0.0f }, { "con.osc2.ratio", 2.0f },
              { "con.osc2.level", 0.25f }, { "con.osc3.wave", 1.0f },
              { "con.osc3.ratio", 3.0f }, { "con.osc3.level", 0.15f },
              { "con.bank1.freq", 500.0f }, { "con.bank1.spread", 0.7f },
              { "con.bank1.bandwidth", 0.15f }, { "con.bank1.mix", 0.55f },
              { "con.bank2.freq", 1500.0f }, { "con.bank2.spread", 0.5f },
              { "con.bank2.bandwidth", 0.2f }, { "con.bank2.mix", 0.45f },
              { "con.flt1.cutoff", 1800.0f }, { "con.flt2.cutoff", 2400.0f },
              { "con.flt3.cutoff", 3000.0f },
              { "con.env1.attack", 0.4f }, { "con.env1.sustain", 0.8f },
              { "con.env1.release", 2.5f }, { "con.env2.attack", 0.6f },
              { "con.sg3.rate", 0.12f }, { "con.sg3.level", 0.25f }
          },
          { { 0, 0, 1, 1.0f }, { 1, 1, 1, 0.8f }, { 2, 2, 1, 0.7f },
            { 6, 15, 4, 0.3f }, { 7, 15, 1, 0.22f }, { 8, 15, 1, 0.18f },
            { 9, 15, 1, 0.14f }, { 10, 15, 1, 0.12f } } },
        { "PT - Bowed Bells", "Tuned Percussion",
          Values {
              { "con.master.volume", 0.6f }, { "con.master.reverbMix", 0.35f },
              { "con.master.delayMix", 0.25f }, { "con.master.delayTime", 0.21f },
              { "con.osc1.wave", 0.0f }, { "con.osc1.level", 0.6f },
              { "con.osc2.wave", 0.0f }, { "con.osc2.ratio", 2.0f },
              { "con.osc2.level", 0.35f }, { "con.osc3.wave", 4.0f },
              { "con.osc3.ratio", 3.0f }, { "con.osc3.level", 0.2f },
              { "con.bank1.freq", 1200.0f }, { "con.bank1.mix", 0.45f },
              { "con.bank1.bandwidth", 0.15f }, { "con.bank2.freq", 2400.0f },
              { "con.bank2.mix", 0.35f },
              { "con.flt1.cutoff", 5000.0f }, { "con.flt2.cutoff", 6000.0f },
              { "con.flt3.cutoff", 7000.0f },
              { "con.env1.attack", 0.002f }, { "con.env1.decay", 0.5f },
              { "con.env1.sustain", 0.0f }, { "con.env1.release", 1.2f },
              { "con.env2.attack", 0.002f }, { "con.env2.decay", 0.3f },
              { "con.env2.sustain", 0.0f }, { "con.env2.release", 0.8f }
          } },
        { "MD - MPE 08 Voice", "Midi",
          Values {
              { "con.master.volume", 0.65f }, { "con.master.glide", 0.02f },
              { "con.master.yDepth", 0.9f }, { "con.master.zDepth", 0.95f },
              { "con.master.bendRange", 48.0f }, { "con.master.reverbMix", 0.28f },
              { "con.osc1.wave", 2.0f }, { "con.osc1.level", 0.6f },
              { "con.osc2.wave", 2.0f }, { "con.osc2.fine", 0.1f },
              { "con.osc2.level", 0.4f }, { "con.osc3.wave", 0.0f },
              { "con.osc3.ratio", 2.0f }, { "con.osc3.level", 0.2f },
              { "con.flt1.cutoff", 2000.0f }, { "con.flt2.cutoff", 2600.0f },
              { "con.flt3.cutoff", 3200.0f }, { "con.flt4.cutoff", 3800.0f },
              { "con.flt5.cutoff", 4400.0f },
              { "con.env1.attack", 0.01f }, { "con.env1.decay", 0.7f },
              { "con.env1.sustain", 0.8f }, { "con.env1.release", 0.8f },
              { "con.sg1.rate", 0.5f }, { "con.sg1.level", 0.2f }
          },
          { { 0, 0, 1, 1.0f }, { 1, 1, 1, 0.9f }, { 2, 2, 1, 0.8f },
            { 6, 15, 4, 0.22f }, { 7, 15, 1, 0.2f }, { 8, 15, 1, 0.15f },
            { 9, 15, 1, 0.12f }, { 10, 15, 1, 0.1f }, { 11, 5, 3, 0.25f } } },
        { "ST - Harp", "Strings",
          Values {
              { "con.master.volume", 0.60f }, { "con.master.glide", 0.004f },
              { "con.master.yDepth", 0.62f }, { "con.master.zDepth", 0.82f },
              { "con.master.reverbMix", 0.44f }, { "con.master.reverbSize", 0.88f },
              { "con.master.delayMix", 0.18f }, { "con.master.delayTime", 0.26f },
              { "con.master.delayFeedback", 0.28f },
              { "con.osc1.wave", 0.0f }, { "con.osc1.level", 0.60f },
              { "con.osc2.wave", 1.0f }, { "con.osc2.ratio", 2.0f },
              { "con.osc2.level", 0.28f }, { "con.osc3.wave", 0.0f },
              { "con.osc3.ratio", 3.0f }, { "con.osc3.level", 0.18f },
              { "con.osc4.wave", 0.0f }, { "con.osc4.ratio", 4.0f },
              { "con.osc4.level", 0.12f }, { "con.osc5.wave", 0.0f },
              { "con.osc5.ratio", 6.0f }, { "con.osc5.level", 0.08f },
              { "con.flt1.cutoff", 7000.0f }, { "con.flt1.res", 0.12f },
              { "con.flt2.cutoff", 8200.0f }, { "con.flt3.cutoff", 9500.0f },
              { "con.flt4.cutoff", 11000.0f }, { "con.flt5.cutoff", 12500.0f },
              { "con.bank1.freq", 523.25f }, { "con.bank1.spread", 0.62f },
              { "con.bank1.bandwidth", 0.13f }, { "con.bank1.mix", 0.46f },
              { "con.bank1.level", 0.78f }, { "con.bank2.freq", 1046.5f },
              { "con.bank2.spread", 0.55f }, { "con.bank2.bandwidth", 0.11f },
              { "con.bank2.mix", 0.32f }, { "con.bank2.level", 0.7f },
              { "con.env1.attack", 0.0015f }, { "con.env1.decay", 0.48f },
              { "con.env1.sustain", 0.0f }, { "con.env1.release", 1.5f },
              { "con.env2.attack", 0.001f }, { "con.env2.decay", 0.26f },
              { "con.env2.sustain", 0.0f }, { "con.env2.release", 0.9f },
              { "con.sg1.rate", 0.42f }, { "con.sg1.level", 0.04f }
          } },
        { "Kevin Penkin Style Eharp", "Strings",
          Values {
              { "con.master.volume", 0.64f }, { "con.master.glide", 0.008f },
              { "con.master.yDepth", 0.64f }, { "con.master.zDepth", 0.9f },
              { "con.master.reverbMix", 0.48f }, { "con.master.reverbSize", 0.92f },
              { "con.master.delayMix", 0.24f }, { "con.master.delayTime", 0.36f },
              { "con.master.delayFeedback", 0.38f },
              { "con.osc1.wave", 0.0f }, { "con.osc1.level", 0.64f },
              { "con.osc2.wave", 1.0f }, { "con.osc2.ratio", 1.0f },
              { "con.osc2.fine", 0.06f }, { "con.osc2.level", 0.42f },
              { "con.osc3.wave", 0.0f }, { "con.osc3.ratio", 0.5f },
              { "con.osc3.level", 0.36f }, { "con.osc4.wave", 1.0f },
              { "con.osc4.ratio", 2.0f }, { "con.osc4.level", 0.24f },
              { "con.osc5.wave", 0.0f }, { "con.osc5.ratio", 3.0f },
              { "con.osc5.level", 0.12f }, { "con.noise.level", 0.010f },
              { "con.noise.color", 0.70f },
              { "con.flt1.cutoff", 4800.0f }, { "con.flt1.res", 0.16f },
              { "con.flt2.cutoff", 5600.0f }, { "con.flt3.cutoff", 6400.0f },
              { "con.flt4.cutoff", 7200.0f }, { "con.flt5.cutoff", 8000.0f },
              { "con.bank1.freq", 261.63f }, { "con.bank1.spread", 0.58f },
              { "con.bank1.bandwidth", 0.18f }, { "con.bank1.mix", 0.48f },
              { "con.bank1.level", 0.85f }, { "con.bank2.freq", 523.25f },
              { "con.bank2.spread", 0.52f }, { "con.bank2.bandwidth", 0.14f },
              { "con.bank2.mix", 0.36f }, { "con.bank2.level", 0.76f },
              { "con.env1.attack", 0.006f }, { "con.env1.decay", 0.85f },
              { "con.env1.sustain", 0.12f }, { "con.env1.release", 3.0f },
              { "con.env2.attack", 0.002f }, { "con.env2.decay", 0.45f },
              { "con.env2.sustain", 0.05f }, { "con.env2.release", 2.0f },
              { "con.sg1.rate", 0.08f }, { "con.sg1.level", 0.06f },
              { "con.sg2.rate", 0.13f }, { "con.sg2.level", 0.04f }
          } },
        { "Kevin Penkin Style E Koto", "Strings",
          Values {
              { "con.master.volume", 0.64f }, { "con.master.glide", 0.002f },
              { "con.master.yDepth", 0.70f }, { "con.master.zDepth", 0.92f },
              { "con.master.reverbMix", 0.48f }, { "con.master.reverbSize", 0.92f },
              { "con.master.delayMix", 0.24f }, { "con.master.delayTime", 0.36f },
              { "con.master.delayFeedback", 0.38f },
              { "con.osc1.wave", 1.0f }, { "con.osc1.level", 0.56f },
              { "con.osc2.wave", 1.0f }, { "con.osc2.ratio", 1.0f },
              { "con.osc2.fine", 3.2f }, { "con.osc2.level", 0.32f },
              { "con.osc3.wave", 1.0f }, { "con.osc3.ratio", 2.0f },
              { "con.osc3.fine", -2.1f }, { "con.osc3.level", 0.20f },
              { "con.osc4.wave", 0.0f }, { "con.osc4.ratio", 0.5f },
              { "con.osc4.level", 0.40f }, { "con.osc5.wave", 1.0f },
              { "con.osc5.ratio", 3.0f }, { "con.osc5.level", 0.12f },
              { "con.noise.level", 0.030f }, { "con.noise.color", 0.55f },
              { "con.flt1.cutoff", 4600.0f }, { "con.flt1.res", 0.22f },
              { "con.flt1.env", 0.38f }, { "con.flt2.cutoff", 5400.0f },
              { "con.flt2.env", 0.28f }, { "con.flt3.cutoff", 6200.0f },
              { "con.flt4.cutoff", 7000.0f }, { "con.flt5.cutoff", 7800.0f },
              { "con.bank1.freq", 196.0f }, { "con.bank1.spread", 0.60f },
              { "con.bank1.bandwidth", 0.12f }, { "con.bank1.mix", 0.50f },
              { "con.bank1.level", 0.90f }, { "con.bank2.freq", 392.0f },
              { "con.bank2.spread", 0.55f }, { "con.bank2.bandwidth", 0.11f },
              { "con.bank2.mix", 0.36f }, { "con.bank2.level", 0.80f },
              { "con.env1.attack", 0.0010f }, { "con.env1.decay", 1.25f },
              { "con.env1.sustain", 0.06f }, { "con.env1.release", 2.0f },
              { "con.env2.attack", 0.001f }, { "con.env2.decay", 0.50f },
              { "con.env2.sustain", 0.0f }, { "con.env2.release", 1.2f },
              { "con.sg1.rate", 0.22f }, { "con.sg1.level", 0.01f }
          } },
        { "PT - Glass Harp", "Tuned Percussion",
          Values {
              { "con.master.volume", 0.58f }, { "con.master.yDepth", 0.7f },
              { "con.master.zDepth", 0.9f }, { "con.master.reverbMix", 0.48f },
              { "con.master.reverbSize", 0.9f }, { "con.master.delayMix", 0.24f },
              { "con.master.delayTime", 0.33f }, { "con.master.delayFeedback", 0.36f },
              { "con.osc1.wave", 0.0f }, { "con.osc1.level", 0.5f },
              { "con.osc2.wave", 0.0f }, { "con.osc2.ratio", 3.0f },
              { "con.osc2.level", 0.3f }, { "con.osc3.wave", 4.0f },
              { "con.osc3.ratio", 5.0f }, { "con.osc3.level", 0.15f },
              { "con.osc4.wave", 0.0f }, { "con.osc4.ratio", 7.0f },
              { "con.osc4.level", 0.1f },
              { "con.flt1.cutoff", 8000.0f }, { "con.flt1.res", 0.16f },
              { "con.flt2.cutoff", 9500.0f }, { "con.flt3.cutoff", 11000.0f },
              { "con.flt4.cutoff", 12500.0f }, { "con.flt5.cutoff", 14000.0f },
              { "con.bank1.freq", 880.0f }, { "con.bank1.spread", 0.8f },
              { "con.bank1.bandwidth", 0.08f }, { "con.bank1.mix", 0.5f },
              { "con.bank1.level", 0.8f }, { "con.bank2.freq", 2640.0f },
              { "con.bank2.spread", 0.7f }, { "con.bank2.bandwidth", 0.07f },
              { "con.bank2.mix", 0.3f }, { "con.bank2.level", 0.65f },
              { "con.env1.attack", 0.001f }, { "con.env1.decay", 0.32f },
              { "con.env1.sustain", 0.0f }, { "con.env1.release", 1.2f },
              { "con.env2.attack", 0.001f }, { "con.env2.decay", 0.2f },
              { "con.env2.sustain", 0.0f }, { "con.env2.release", 0.8f },
              { "con.sg1.rate", 0.9f }, { "con.sg1.level", 0.03f }
          } }
    });
}
std::vector<FactoryPreset> continuumPresets()
{
    auto presets = continuumCustomPresets();

    std::set<juce::String> names;
    for (const auto& preset : presets)
        names.insert (preset.name);

    for (auto& preset : makeHakenBank())
    {
        if (names.insert (preset.name).second)
            presets.push_back (std::move (preset));
    }

    return sortedPresets (std::move (presets));
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

    if (instrumentId == "continuum")
        return continuumPresets();

    return {};
}


} // namespace triad
