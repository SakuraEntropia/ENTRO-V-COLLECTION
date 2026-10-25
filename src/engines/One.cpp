#include "engines/One.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstring>
#include <string>

namespace triad
{

namespace
{
constexpr float kMaxCutoffFraction = 0.45f;

std::vector<std::string> choicesOf (std::initializer_list<const char*> items)
{
    std::vector<std::string> result;
    result.reserve (items.size());

    for (const auto* item : items)
        result.emplace_back (item);

    return result;
}

std::vector<std::string> numberedChoices (int first, int last)
{
    std::vector<std::string> result;
    result.reserve ((size_t) std::max (0, last - first + 1));

    for (int value = first; value <= last; ++value)
        result.push_back (std::to_string (value));

    return result;
}

float finiteOrZero (float value)
{
    return std::isfinite (value) ? value : 0.0f;
}

LfoWave lfoWaveFromIndex (int index)
{
    switch (std::clamp (index, 0, 6))
    {
        case 0: return LfoWave::Sine;
        case 1: return LfoWave::Triangle;
        case 2: return LfoWave::SawUp;
        case 3: return LfoWave::SawDown;
        case 4: return LfoWave::Square;
        case 5: return LfoWave::SampleHold;
        default: return LfoWave::RandomSmooth;
    }
}

/** Analog oscillator morph: saw -> triangle -> reverse saw, blended into pulse. */
float morphWaveSample (const Oscillator& oscillator, float angle, float shape, float pulseWidth)
{
    const auto saw = oscillator.sample (Oscillator::Wave::Saw);
    const auto triangle = oscillator.sample (Oscillator::Wave::Triangle);

    float analog;

    if (angle < 0.5f)
        analog = lerpf (saw, triangle, angle * 2.0f);
    else
        analog = lerpf (triangle, -saw, (angle - 0.5f) * 2.0f);

    if (shape <= 0.0005f)
        return analog;

    const auto pulse = oscillator.sample (Oscillator::Wave::Pulse, pulseWidth);
    return lerpf (analog, pulse, shape);
}
} // namespace

// ---------------------------------------------------------------------------
// Construction and parameter definition
// ---------------------------------------------------------------------------

OneEngine::OneEngine()
{
    buildDefinition();
    cacheDefaults();
}

void OneEngine::buildDefinition()
{
    definition_.id = "moogone";
    definition_.displayName = "ONE V";
    definition_.manufacturer = "SakuraEntropia";
    definition_.version = "0.1.0";

    // Index tables are filled by the add methods below; -1 marks an unfilled slot
    // so a missing definition is caught by the assertions at the end.
    for (auto& row : partIndex_)
        row.fill (-1);
    masterIndex_.fill (-1);

    // Explicit group order for the UI. addParam() does not reorder existing groups.
    definition_.groups = { "Part 1", "Part 2", "Part 3", "Master", "Arpeggiator" };

    for (int part = 0; part < kPartCount; ++part)
        addPartDefinition (part);

    addMasterDefinition();

    // Arpeggiator metadata lives in one UI group, after the part and master
    // parameters, so the definition's groups are contiguous.
    for (int part = 0; part < kPartCount; ++part)
        addArpeggiatorDefinition (part);

    // Append to the complete original layout, preserving all 408 old indices.
    for (int part = 0; part < kPartCount; ++part)
        for (int osc = 0; osc < 3; ++osc)
        {
            ParamSpec spec;
            spec.id = "one.p" + std::to_string (part + 1) + ".vco" + std::to_string (osc + 1) + ".beatHz";
            spec.name = "VCO " + std::to_string (osc + 1) + " BEAT FREQ";
            spec.group = "Part " + std::to_string (part + 1);
            spec.min = -7; spec.max = 7; spec.def = 0; spec.unit = "Hz";
            partIndex_[(size_t) part][(size_t) (PVco1BeatHz + osc)] = definition_.addParam (std::move (spec));
        }

    // New allocation controls are appended after all 417 existing parameters.
    // In particular, inserting MVoices in addMasterDefinition would renumber
    // host automation for the original arpeggiator and beat-frequency controls.
    ParamSpec budget;
    budget.id = "one.master.voices"; budget.name = "SHARED VOICE BUDGET";
    budget.group = "Master"; budget.min = 0; budget.max = 1; budget.def = 0;
    budget.curve = ParamCurve::Discrete; budget.choices = choicesOf ({ "8", "16" });
    masterIndex_[MVoices] = definition_.addParam (std::move (budget));
    for (int part = 0; part < kPartCount; ++part)
    {
        const auto prefix = "one.p" + std::to_string (part + 1) + ".voice.";
        const auto group = "Part " + std::to_string (part + 1);
        auto add = [this, part, &prefix, &group] (PartParam id, const char* suffix,
                                                 const char* name, float min, float max,
                                                 float def, std::vector<std::string> choices = {})
        {
            ParamSpec spec;
            spec.id = prefix + suffix; spec.name = name; spec.group = group;
            spec.min = min; spec.max = max; spec.def = def;
            spec.curve = choices.empty() ? ParamCurve::Linear : ParamCurve::Discrete;
            spec.choices = std::move (choices);
            partIndex_[(size_t) part][(size_t) id] = definition_.addParam (std::move (spec));
        };
        add (PVoiceMode, "mode", "VOICE MODE", 0, 2, 0, choicesOf ({ "Poly", "Mono", "Unison" }));
        // A natural integer count, matching arp.length's existing convention.
        add (PUnisonVoices, "unisonVoices", "UNISON VOICES", 2, 16, 2);
        add (PUnisonDetune, "unisonDetune", "UNISON DETUNE", 0, 1, .15f);
    }

    struct NamedItem
    {
        const char* id;
        const char* name;
    };

    static const NamedItem sourceNames[] = {
        { "none", "None" }, { "lfo1", "LFO 1" }, { "lfo2", "LFO 2" },
        { "lfo3", "LFO 3" }, { "lfo4", "LFO 4" }, { "env1", "Env 1" },
        { "env2", "Env 2" }, { "env3", "Env 3" }, { "velocity", "Velocity" },
        { "aftertouch", "Aftertouch" }, { "modWheel", "Mod Wheel" },
        { "keyTrack", "Key Track" }, { "random", "Random" }
    };

    static const NamedItem destNames[] = {
        { "none", "None" }, { "vco1Pitch", "VCO 1 Pitch" }, { "vco2Pitch", "VCO 2 Pitch" },
        { "vco3Pitch", "VCO 3 Pitch" }, { "pw", "Pulse Width" },
        { "osc1Level", "Osc 1 Level" }, { "osc2Level", "Osc 2 Level" },
        { "osc3Level", "Osc 3 Level" }, { "noiseLevel", "Noise Level" },
        { "svfCutoff", "SVF Cutoff" }, { "svfResonance", "SVF Resonance" },
        { "ladderCutoff", "Ladder Cutoff" }, { "ladderResonance", "Ladder Resonance" },
        { "filterMix", "Filter Mix" }, { "pan", "Pan" }, { "volume", "Volume" }
    };

    for (const auto& source : sourceNames)
        definition_.modSources.push_back ({ source.id, source.name });

    for (const auto& dest : destNames)
        definition_.modDestinations.push_back ({ dest.id, dest.name });

    for (int part = 0; part < kPartCount; ++part)
        for (int param = 0; param < (int) PParamCount; ++param)
            assert (partIndex_[(size_t) part][(size_t) param] >= 0);

    for (int param = 0; param < (int) MParamCount; ++param)
        assert (masterIndex_[(size_t) param] >= 0);
}

void OneEngine::addPartDefinition (int part)
{
    const auto prefix = "one.p" + std::to_string (part + 1) + ".";
    const auto group = "Part " + std::to_string (part + 1);

    auto addParam = [this, part, &prefix] (PartParam id, const char* suffix,
                                           const char* name, const char* paramGroup,
                                           float min, float max, float def,
                                           ParamCurve curve = ParamCurve::Linear,
                                           std::vector<std::string> choices = {},
                                           const char* unit = "")
    {
        ParamSpec spec;
        spec.id = prefix + suffix;
        spec.name = name;
        spec.group = paramGroup;
        spec.min = min;
        spec.max = max;
        spec.def = def;
        spec.curve = curve;
        spec.choices = std::move (choices);
        spec.unit = unit;
        partIndex_[(size_t) part][(size_t) id] = definition_.addParam (std::move (spec));
    };

    auto addPart = [&addParam, &group] (PartParam id, const char* suffix, const char* name,
                                        float min, float max, float def,
                                        ParamCurve curve = ParamCurve::Linear,
                                        std::vector<std::string> choices = {},
                                        const char* unit = "")
    {
        addParam (id, suffix, name, group.c_str(), min, max, def, curve,
                  std::move (choices), unit);
    };

    // --- Part / keyboard ---------------------------------------------------
    addPart (PEnabled, "enabled", "PART ENABLE", 0.0f, 1.0f, 1.0f,
             ParamCurve::Discrete, choicesOf ({ "Off", "On" }));
    addPart (PKeyboardLow, "keyboardLow", "KEY LOW", 0.0f, 127.0f, 0.0f);
    addPart (PKeyboardHigh, "keyboardHigh", "KEY HIGH", 0.0f, 127.0f, 127.0f);
    addPart (PMidiChannel, "midiChannel", "MIDI CHANNEL", 0.0f, 15.0f, (float) part,
             ParamCurve::Discrete, numberedChoices (1, 16));
    addPart (PLevel, "level", "LEVEL", 0.0f, 1.0f, 0.8f);
    addPart (PPan, "pan", "PAN", -1.0f, 1.0f, 0.0f);

    // --- Oscillators -------------------------------------------------------
    const PartParam octaveIds[3] = { PVco1Octave, PVco2Octave, PVco3Octave };
    const PartParam tuneIds[3] = { PVco1Tune, PVco2Tune, PVco3Tune };
    const PartParam angleIds[3] = { PVco1WaveAngle, PVco2WaveAngle, PVco3WaveAngle };
    const PartParam shapeIds[3] = { PVco1Shape, PVco2Shape, PVco3Shape };
    const PartParam pwIds[3] = { PVco1Pw, PVco2Pw, PVco3Pw };
    const PartParam levelIds[3] = { PVco1Level, PVco2Level, PVco3Level };
    const PartParam syncIds[2] = { PVco2Sync, PVco3Sync };

    addPart (PVco1On, "vco1.on", "VCO 1 ON", 0.0f, 1.0f, 1.0f,
             ParamCurve::Discrete, choicesOf ({ "Off", "On" }));

    for (int osc = 0; osc < 3; ++osc)
    {
        const auto index = std::to_string (osc + 1);
        const auto stem = "vco" + index;
        const auto label = "VCO " + index + " ";

        addPart (octaveIds[osc], (stem + ".octave").c_str(), (label + "OCTAVE").c_str(),
                 0.0f, 3.0f, 1.0f, ParamCurve::Discrete,
                 choicesOf ({ "16'", "8'", "4'", "2'" }));
        addPart (tuneIds[osc], (stem + ".tune").c_str(), (label + "TUNE").c_str(),
                 -24.0f, 24.0f, 0.0f, ParamCurve::Linear, {}, "st");
        addPart (angleIds[osc], (stem + ".waveAngle").c_str(), (label + "WAVE ANGLE").c_str(),
                 0.0f, 1.0f, 0.0f);
        addPart (shapeIds[osc], (stem + ".shape").c_str(), (label + "SHAPE").c_str(),
                 0.0f, 1.0f, 0.0f);
        addPart (pwIds[osc], (stem + ".pw").c_str(), (label + "PULSE WIDTH").c_str(),
                 0.05f, 0.95f, 0.5f);
        addPart (levelIds[osc], (stem + ".level").c_str(), (label + "LEVEL").c_str(),
                 0.0f, 1.0f, osc == 0 ? 0.7f : (osc == 1 ? 0.6f : 0.5f));

        if (osc > 0)
            addPart (syncIds[osc - 1], (stem + ".sync").c_str(), (label + "SYNC").c_str(),
                     0.0f, 1.0f, 0.0f, ParamCurve::Discrete,
                     choicesOf ({ "Off", "On" }));
    }

    addPart (PVco3Fm, "vco3.fm", "VCO 3 -> VCO 1 FM", 0.0f, 1.0f, 0.0f);

    // --- Mixer -------------------------------------------------------------
    addPart (PMixNoise, "mix.noise", "NOISE", 0.0f, 1.0f, 0.0f);
    addPart (PMixNoiseColor, "mix.noiseColor", "NOISE COLOR", 0.0f, 1.0f, 0.0f,
             ParamCurve::Discrete, choicesOf ({ "White", "Pink" }));
    addPart (PMixRing, "mix.ring", "RING MOD", 0.0f, 1.0f, 0.0f);
    addPart (PMixExt, "mix.ext", "EXTERNAL", 0.0f, 1.0f, 0.0f);

    // --- State variable filter --------------------------------------------
    addPart (PSvfCutoff, "svf.cutoff", "SVF CUTOFF", 20.0f, 20000.0f, 1200.0f,
             ParamCurve::Log, {}, "Hz");
    addPart (PSvfResonance, "svf.resonance", "SVF RESONANCE", 0.0f, 1.0f, 0.2f);
    addPart (PSvfMode, "svf.mode", "SVF MODE", 0.0f, 3.0f, 0.0f,
             ParamCurve::Discrete, choicesOf ({ "LP", "HP", "BP", "Notch" }));
    addPart (PSvfSlope, "svf.slope", "SVF SLOPE", 0.0f, 2.0f, 1.0f,
             ParamCurve::Discrete, choicesOf ({ "6 dB", "12 dB", "24 dB" }));
    addPart (PSvfEnvAmt, "svf.envAmt", "SVF ENV AMT", -1.0f, 1.0f, 0.3f);
    addPart (PSvfKeyTrack, "svf.keyTrack", "SVF KEY TRACK", 0.0f, 1.0f, 0.3f);
    addPart (PSvfLevel, "svf.level", "SVF LEVEL", 0.0f, 1.0f, 0.8f);

    // --- Ladder filter -----------------------------------------------------
    addPart (PLadderCutoff, "ladder.cutoff", "LADDER CUTOFF", 20.0f, 20000.0f, 900.0f,
             ParamCurve::Log, {}, "Hz");
    addPart (PLadderResonance, "ladder.resonance", "LADDER RESONANCE", 0.0f, 1.0f, 0.15f);
    addPart (PLadderDrive, "ladder.drive", "LADDER DRIVE", 0.0f, 1.0f, 0.2f);
    addPart (PLadderSlope, "ladder.slope", "LADDER SLOPE", 0.0f, 3.0f, 3.0f,
             ParamCurve::Discrete, choicesOf ({ "6 dB", "12 dB", "18 dB", "24 dB" }));
    addPart (PLadderEnvAmt, "ladder.envAmt", "LADDER ENV AMT", -1.0f, 1.0f, 0.3f);
    addPart (PLadderKeyTrack, "ladder.keyTrack", "LADDER KEY TRACK", 0.0f, 1.0f, 0.3f);

    addPart (PFiltersRouting, "filters.routing", "FILTER ROUTING", 0.0f, 1.0f, 0.0f,
             ParamCurve::Discrete, choicesOf ({ "Serial", "Parallel" }));
    addPart (PFiltersMix, "filters.mix", "FILTER MIX", 0.0f, 1.0f, 0.5f);

    // --- Envelopes ---------------------------------------------------------
    const float envSustainDefaults[3] = { 0.6f, 0.4f, 0.8f };
    const float envVelocityDefaults[3] = { 0.3f, 0.2f, 0.5f };

    for (int env = 0; env < kEnvCount; ++env)
    {
        const auto index = std::to_string (env + 1);
        const auto stem = "env" + index;
        const auto label = "ENV " + index + " ";
        const auto base = (PartParam) kEnvBase[env];

        addPart ((PartParam) (base + 0), (stem + ".delay").c_str(), (label + "DELAY").c_str(),
                 0.0f, 2.0f, 0.0f, ParamCurve::Linear, {}, "s");
        addPart ((PartParam) (base + 1), (stem + ".attack").c_str(), (label + "ATTACK").c_str(),
                 0.001f, 20.0f, 0.005f, ParamCurve::Log, {}, "s");
        addPart ((PartParam) (base + 2), (stem + ".hold").c_str(), (label + "HOLD").c_str(),
                 0.0f, 2.0f, 0.0f, ParamCurve::Linear, {}, "s");
        addPart ((PartParam) (base + 3), (stem + ".decay").c_str(), (label + "DECAY").c_str(),
                 0.001f, 20.0f, 0.35f, ParamCurve::Log, {}, "s");
        addPart ((PartParam) (base + 4), (stem + ".sustain").c_str(), (label + "SUSTAIN").c_str(),
                 0.0f, 1.0f, envSustainDefaults[env]);
        addPart ((PartParam) (base + 5), (stem + ".release").c_str(), (label + "RELEASE").c_str(),
                 0.001f, 20.0f, 0.35f, ParamCurve::Log, {}, "s");
        addPart ((PartParam) (base + 6), (stem + ".velocity").c_str(), (label + "VELOCITY").c_str(),
                 0.0f, 1.0f, envVelocityDefaults[env]);

        if (env < 2)
            addPart ((PartParam) (base + 7), (stem + ".loop").c_str(), (label + "LOOP").c_str(),
                     0.0f, 1.0f, 0.0f, ParamCurve::Discrete, choicesOf ({ "Off", "On" }));
    }

    // --- LFOs --------------------------------------------------------------
    const float lfoRateDefaults[4] = { 2.0f, 1.0f, 0.5f, 0.25f };

    for (int lfo = 0; lfo < kLfoCount; ++lfo)
    {
        const auto index = std::to_string (lfo + 1);
        const auto stem = "lfo" + index;
        const auto label = "LFO " + index + " ";

        addPart ((PartParam) (PLfo1Rate + lfo * 5), (stem + ".rate").c_str(),
                 (label + "RATE").c_str(), 0.01f, 100.0f, lfoRateDefaults[lfo],
                 ParamCurve::Log, {}, "Hz");
        addPart ((PartParam) (PLfo1Wave + lfo * 5), (stem + ".wave").c_str(),
                 (label + "WAVE").c_str(), 0.0f, 6.0f, 0.0f, ParamCurve::Discrete,
                 choicesOf ({ "Sine", "Triangle", "Saw Up", "Saw Down", "Square", "S&H", "Random" }));
        addPart ((PartParam) (PLfo1Sync + lfo * 5), (stem + ".sync").c_str(),
                 (label + "SYNC").c_str(), 0.0f, 1.0f, 0.0f, ParamCurve::Discrete,
                 choicesOf ({ "Off", "On" }));
        addPart ((PartParam) (PLfo1Retrig + lfo * 5), (stem + ".retrig").c_str(),
                 (label + "RETRIGGER").c_str(), 0.0f, 1.0f, 0.0f, ParamCurve::Discrete,
                 choicesOf ({ "Off", "On" }));
        addPart ((PartParam) (PLfo1Fade + lfo * 5), (stem + ".fade").c_str(),
                 (label + "FADE").c_str(), 0.0f, 5.0f, 0.0f, ParamCurve::Linear, {}, "s");
    }

    addPart (PLfo1PitchAmt, "lfo1.pitchAmt", "LFO 1 PITCH", 0.0f, 1.0f, 0.0f);
    addPart (PLfo2PwmAmt, "lfo2.pwmAmt", "LFO 2 PWM", 0.0f, 1.0f, 0.0f);
    addPart (PLfo3CutoffAmt, "lfo3.cutoffAmt", "LFO 3 CUTOFF", 0.0f, 1.0f, 0.0f);

    // --- Modulation matrix -------------------------------------------------
    for (int slot = 0; slot < kModSlotCount; ++slot)
    {
        const auto index = std::to_string (slot + 1);
        const auto stem = "mod.slot" + index;
        const auto label = "MOD " + index + " ";
        const auto base = (PartParam) (PMod1Source + slot * 3);

        // Musical init patch: the wheel opens the SVF and aftertouch adds level.
        const auto defaultSource = slot == 0 ? (float) MsModWheel
                                  : slot == 1 ? (float) MsAftertouch : 0.0f;
        const auto defaultDest = slot == 0 ? (float) MdSvfCutoff
                                : slot == 1 ? (float) MdVolume : 0.0f;
        const auto defaultAmount = slot == 0 ? 0.2f : slot == 1 ? 0.15f : 0.0f;

        addPart ((PartParam) (base + 0), (stem + ".source").c_str(), (label + "SOURCE").c_str(),
                 0.0f, (float) (MsCount - 1), defaultSource, ParamCurve::Discrete,
                 choicesOf ({ "None", "LFO 1", "LFO 2", "LFO 3", "LFO 4", "Env 1", "Env 2",
                              "Env 3", "Velocity", "Aftertouch", "Mod Wheel", "Key Track",
                              "Random" }));
        addPart ((PartParam) (base + 1), (stem + ".dest").c_str(), (label + "DEST").c_str(),
                 0.0f, (float) (MdCount - 1), defaultDest, ParamCurve::Discrete,
                 choicesOf ({ "None", "VCO 1 Pitch", "VCO 2 Pitch", "VCO 3 Pitch",
                              "Pulse Width", "Osc 1 Level", "Osc 2 Level", "Osc 3 Level",
                              "Noise Level", "SVF Cutoff", "SVF Resonance", "Ladder Cutoff",
                              "Ladder Resonance", "Filter Mix", "Pan", "Volume" }));
        addPart ((PartParam) (base + 2), (stem + ".amount").c_str(), (label + "AMOUNT").c_str(),
                 -1.0f, 1.0f, defaultAmount);
    }

    // --- Per-part effects --------------------------------------------------
    addPart (PFxDistType, "fx.dist.type", "DIST TYPE", 0.0f, 3.0f, 0.0f,
             ParamCurve::Discrete, choicesOf ({ "Soft", "Hard", "Fold", "Tube" }));
    addPart (PFxDistDrive, "fx.dist.drive", "DIST DRIVE", 0.0f, 1.0f, 0.0f);
    addPart (PFxDistMix, "fx.dist.mix", "DIST MIX", 0.0f, 1.0f, 0.0f);
    addPart (PFxChorusRate, "fx.chorus.rate", "CHORUS RATE", 0.05f, 8.0f, 0.4f,
             ParamCurve::Log, {}, "Hz");
    addPart (PFxChorusDepth, "fx.chorus.depth", "CHORUS DEPTH", 0.0f, 1.0f, 0.3f);
    addPart (PFxChorusMix, "fx.chorus.mix", "CHORUS MIX", 0.0f, 1.0f, 0.0f);
    addPart (PFxDelayTime, "fx.delay.time", "DELAY TIME", 0.01f, 2.0f, 0.35f,
             ParamCurve::Log, {}, "s");
    addPart (PFxDelayFeedback, "fx.delay.feedback", "DELAY FEEDBACK", 0.0f, 1.0f, 0.3f);
    addPart (PFxDelayMix, "fx.delay.mix", "DELAY MIX", 0.0f, 1.0f, 0.0f);
    addPart (PFxReverbSend, "fx.reverb.send", "REVERB SEND", 0.0f, 1.0f, 0.15f);

}

void OneEngine::addArpeggiatorDefinition (int part)
{
    // These parameters are consumed by the host arpeggiator wrapper; the audio
    // engine itself does not run an arpeggiator.
    const auto prefix = "one.p" + std::to_string (part + 1) + ".";
    const auto labelPrefix = "P" + std::to_string (part + 1) + " ";

    auto addArp = [this, part, &prefix, &labelPrefix] (PartParam id, const char* suffix,
                                                       const char* name,
                                                       float min, float max, float def,
                                                       ParamCurve curve,
                                                       std::vector<std::string> choices)
    {
        ParamSpec spec;
        spec.id = prefix + suffix;
        spec.name = labelPrefix + name;
        spec.group = "Arpeggiator";
        spec.min = min;
        spec.max = max;
        spec.def = def;
        spec.curve = curve;
        spec.choices = std::move (choices);
        partIndex_[(size_t) part][(size_t) id] = definition_.addParam (std::move (spec));
    };

    addArp (PArpEnabled, "arp.enabled", "ARP ENABLE", 0.0f, 1.0f, 0.0f,
            ParamCurve::Discrete, choicesOf ({ "Off", "On" }));
    addArp (PArpMode, "arp.mode", "ARP MODE", 0.0f, 5.0f, 0.0f, ParamCurve::Discrete,
            choicesOf ({ "Up", "Down", "Up/Down", "Random", "Order", "Step" }));
    addArp (PArpRate, "arp.rate", "ARP RATE", 0.0f, 5.0f, 3.0f, ParamCurve::Discrete,
            choicesOf ({ "1/4", "1/8", "1/8T", "1/16", "1/16T", "1/32" }));
    addArp (PArpOctaves, "arp.octaves", "ARP OCTAVES", 0.0f, 3.0f, 0.0f, ParamCurve::Discrete,
            choicesOf ({ "1", "2", "3", "4" }));
    addArp (PArpHold, "arp.hold", "ARP HOLD", 0.0f, 1.0f, 0.0f, ParamCurve::Discrete,
            choicesOf ({ "Off", "On" }));
    // Length is a natural 1..16 value rather than a choice index: the host
    // sequencer consumes it as a step count. The choices are only UI labels.
    addArp (PArpLength, "arp.length", "ARP LENGTH", 1.0f, 16.0f, 16.0f,
            ParamCurve::Linear, numberedChoices (1, 16));
}

void OneEngine::addMasterDefinition()
{
    auto addParam = [this] (MasterParam id, const char* suffix, const char* name,
                            float min, float max, float def,
                            ParamCurve curve = ParamCurve::Linear,
                            std::vector<std::string> choices = {},
                            const char* unit = "")
    {
        ParamSpec spec;
        spec.id = "one.master." + std::string (suffix);
        spec.name = name;
        spec.group = "Master";
        spec.min = min;
        spec.max = max;
        spec.def = def;
        spec.curve = curve;
        spec.choices = std::move (choices);
        spec.unit = unit;
        masterIndex_[(size_t) id] = definition_.addParam (std::move (spec));
    };

    addParam (MMasterVolume, "volume", "MASTER VOLUME", 0.0f, 1.0f, 0.75f);
    addParam (MReverbSize, "reverbSize", "REVERB SIZE", 0.0f, 1.0f, 0.5f);
    addParam (MReverbDamping, "reverbDamping", "REVERB DAMPING", 0.0f, 1.0f, 0.35f);
    addParam (MReverbMix, "reverbMix", "REVERB MIX", 0.0f, 1.0f, 0.15f);
    addParam (MPitchBendRange, "pitchBendRange", "PITCH BEND RANGE", 0.0f, 24.0f, 2.0f,
              ParamCurve::Linear, {}, "st");
    addParam (MDrift, "drift", "ANALOG DRIFT", 0.0f, 1.0f, 0.12f);
    addParam (MSelectedPart, "selectedPart", "SELECTED PART", 1.0f, 3.0f, 1.0f,
              ParamCurve::Linear, {}, "");
    addParam (MGlide, "glide", "GLIDE", 0.0f, 1.5f, 0.0f, ParamCurve::Linear, {}, "s");
    addParam (MAftertouch, "aftertouch", "AFTERTOUCH DEPTH", 0.0f, 1.0f, 0.35f);
}

void OneEngine::cacheDefaults()
{
    values_.resize (definition_.params.size());

    for (size_t index = 0; index < definition_.params.size(); ++index)
        values_[index] = definition_.params[index].def;

    for (int part = 0; part < kPartCount; ++part)
        for (int param = 0; param < (int) PParamCount; ++param)
            partRaw_[(size_t) part][(size_t) param] = value (part, (PartParam) param);

    for (int param = 0; param < (int) MParamCount; ++param)
        masterRaw_[(size_t) param] = masterValue ((MasterParam) param);
}

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

int OneEngine::choiceOf (float value, int maxIndex)
{
    return std::clamp ((int) std::lround (clampf (value, 0.0f, (float) maxIndex)), 0, maxIndex);
}

void OneEngine::prepare (double sampleRate, int maxBlockSize)
{
    sampleRate_ = clampd (sampleRate, 8000.0, 384000.0);
    maxBlockSize_ = std::max (1, maxBlockSize);
    prepared_ = true;

    for (int part = 0; part < kPartCount; ++part)
    {
        auto& runtime = parts_[(size_t) part];

        for (int lfo = 0; lfo < kLfoCount; ++lfo)
            runtime.lfos[(size_t) lfo].prepare (sampleRate_,
                                                0x9e3779b9u + (uint32_t) (part * 131 + lfo * 17));

        runtime.delay.prepare (sampleRate_, 2.0f);
        runtime.chorus.prepare (sampleRate_);

        runtime.levelSmoother.prepare (sampleRate_, 15.0f);
        runtime.panSmoother.prepare (sampleRate_, 15.0f);
        runtime.mixSmoother.prepare (sampleRate_, 12.0f);
        runtime.svfCutoffSmoother.prepare (sampleRate_, 8.0f);
        runtime.ladderCutoffSmoother.prepare (sampleRate_, 8.0f);

        runtime.lfoValue.fill (0.0f);
        runtime.lfoFadeTimer.fill (0);
        runtime.lfoFadeLength.fill (0);
        runtime.busL = runtime.busR = 0.0f;
        runtime.lastNote = 60;
    }

    for (int index = 0; index < kMaxVoices; ++index)
    {
        auto& voice = voices_[(size_t) index];

        for (auto& oscillator : voice.oscillators)
            oscillator.prepare (sampleRate_);

        for (auto& envelope : voice.envelopes)
            envelope.prepare (sampleRate_);

        for (auto& filter : voice.svf)
            filter.prepare (sampleRate_);

        voice.ladder.prepare (sampleRate_);
        voice.noise.prepare (sampleRate_,
                             0xabcdef01u + (uint32_t) (index + 1) * 2654435761u);
        voice.rng.seed (0x1f2e3d4cu + (uint32_t) (index + 1) * 2246822519u);
    }

    reverb_.prepare (sampleRate_);
    reverb_.setMix (1.0f);
    masterSmoother_.prepare (sampleRate_, 20.0f);
    dcLeft_.prepare (sampleRate_);
    dcRight_.prepare (sampleRate_);

    driftInterval_ = std::max (1, (int) (sampleRate_ * 0.35));
    driftCoeff_ = 1.0f - std::exp (-1.0f / (0.18f * (float) sampleRate_));

    reset();
}

void OneEngine::reset()
{
    heldCount_ = 0;
    sustainPedal_.fill (false);
    pitchBend_ = 0.0f;
    modWheel_ = 0.0f;
    aftertouch_ = 0.0f;
    activeVoices_ = 0;
    voiceBudget_ = choiceOf (masterValue (MVoices), 1) == 0 ? 8 : 16;
    voiceSerial_ = 0;
    meter_ = 0.0f;
    externalInput_ = nullptr;
    externalInputSize_ = 0;

    for (auto& voice : voices_)
    {
        voice.active = false;
        voice.gated = false;
        voice.part = 0;
        voice.note = 60;
        voice.channel = 1;
        voice.velocity = 0.8f;
        voice.serial = 0;
        voice.pitch = 60.0;
        voice.targetPitch = 60.0;
        voice.drift = 0.0f;
        voice.driftTarget = 0.0f;
        voice.driftCountdown = 0;
        voice.randomValue = 0.0f;
        voice.fmSample = 0.0f;
        voice.unisonPosition = 0.0f;
        voice.unisonGain = 1.0f;
        voice.lastEnv.fill (0.0f);

        for (auto& oscillator : voice.oscillators)
            oscillator.reset (0.0);

        for (auto& envelope : voice.envelopes)
            envelope.reset();

        for (auto& filter : voice.svf)
            filter.reset();

        voice.ladder.reset();
        voice.noise.reset();
    }

    for (int part = 0; part < kPartCount; ++part)
    {
        auto& runtime = parts_[(size_t) part];

        for (auto& lfo : runtime.lfos)
            lfo.reset (0.0f);

        runtime.lfoValue.fill (0.0f);
        runtime.lfoFadeTimer.fill (0);
        runtime.lfoFadeLength.fill (0);
        runtime.delay.reset();
        runtime.chorus.reset();
        runtime.busL = runtime.busR = 0.0f;
        runtime.lastNote = 60;

        runtime.levelSmoother.snap (clamp01 (value (part, PLevel)));
        runtime.panSmoother.snap (clampf (value (part, PPan), -1.0f, 1.0f));
        runtime.mixSmoother.snap (clamp01 (value (part, PFiltersMix)));
        runtime.svfCutoffSmoother.snap (clampf (value (part, PSvfCutoff), 20.0f, 20000.0f));
        runtime.ladderCutoffSmoother.snap (clampf (value (part, PLadderCutoff), 20.0f, 20000.0f));

        enabledPrev_[(size_t) part] = value (part, PEnabled) >= 0.5f;
        partGain_[(size_t) part] = clamp01 (value (part, PLevel));
        partPan_[(size_t) part] = clampf (value (part, PPan), -1.0f, 1.0f);
        svfBase_[(size_t) part] = clampf (value (part, PSvfCutoff), 20.0f, 20000.0f);
        ladderBase_[(size_t) part] = clampf (value (part, PLadderCutoff), 20.0f, 20000.0f);
        filterMix_[(size_t) part] = clamp01 (value (part, PFiltersMix));

        for (auto& lfoValue : lfoNow_[(size_t) part])
            lfoValue = 0.0f;

        for (auto& ratio : vcoRatio_[(size_t) part])
            ratio = 1.0f;

        for (int param = 0; param < (int) PParamCount; ++param)
            partRaw_[(size_t) part][(size_t) param] = value (part, (PartParam) param);
    }

    for (int param = 0; param < (int) MParamCount; ++param)
        masterRaw_[(size_t) param] = masterValue ((MasterParam) param);

    reverb_.reset();
    masterSmoother_.snap (clamp01 (masterValue (MMasterVolume)));
    dcLeft_.reset();
    dcRight_.reset();
}

void OneEngine::setParameters (const float* values, int count)
{
    if (values == nullptr || count <= 0)
        return;

    std::array<int, kPartCount> previousMode {}, previousUnison {};
    for (int part = 0; part < kPartCount; ++part)
    {
        previousMode[(size_t) part] = voiceMode (part);
        previousUnison[(size_t) part] = choiceOf (value (part, PUnisonVoices), 16);
    }
    const auto previousBudget = voiceBudget_;
    const auto n = std::min (count, (int) values_.size());

    for (int index = 0; index < n; ++index)
    {
        const auto& spec = definition_.params[(size_t) index];
        const auto incoming = values[(size_t) index];
        values_[(size_t) index] = std::isfinite (incoming)
                                    ? clampf (incoming, spec.min, spec.max)
                                    : spec.def;
    }
    voiceBudget_ = choiceOf (masterValue (MVoices), 1) == 0 ? 8 : 16;
    enforceVoiceBudget();
    for (int part = 0; part < kPartCount; ++part)
    {
        const auto mode = voiceMode (part);
        if (mode != previousMode[(size_t) part]
            || (mode == 2 && (previousBudget != voiceBudget_
                || previousUnison[(size_t) part] != choiceOf (value (part, PUnisonVoices), 16))))
            rebuildPartVoices (part);
    }
}

void OneEngine::setTransport (const TransportInfo& transport)
{
    transport_ = transport;
}

// ---------------------------------------------------------------------------
// MIDI
// ---------------------------------------------------------------------------

int OneEngine::normalizeChannel (int channel) const
{
    // MidiEvent::channel is zero-based in this framework: the host adapter maps
    // MIDI channels 1..16 to 0..15. Valid part channels are the same 0..15
    // indices as the "1".."16" choice menu.
    return std::clamp (channel, 0, 15);
}

int OneEngine::partChannel (int part) const
{
    return choiceOf (value (part, PMidiChannel), 15);
}

int OneEngine::selectedPartIndex() const
{
    return std::clamp ((int) std::lround (clampf (masterValue (MSelectedPart), 1.0f, 3.0f)) - 1,
                       0, kPartCount - 1);
}

bool OneEngine::partNoteInRange (int part, int note) const
{
    auto low = choiceOf (value (part, PKeyboardLow), 127);
    auto high = choiceOf (value (part, PKeyboardHigh), 127);

    if (low > high)
        std::swap (low, high);

    return note >= low && note <= high;
}

int OneEngine::targetParts (int channel, int note, int* outParts) const
{
    int count = 0;
    bool channelClaimed = false;

    for (int part = 0; part < kPartCount; ++part)
    {
        const bool enabled = value (part, PEnabled) >= 0.5f;
        const int partMidiChannel = partChannel (part);

        if (enabled && partMidiChannel == channel)
            channelClaimed = true;

        if (! enabled || partMidiChannel != channel || ! partNoteInRange (part, note))
            continue;

        outParts[count++] = part;
    }

    // On-screen keyboard fallback: a channel that no enabled part listens to
    // plays the selected part (still honouring its key range).
    if (count == 0 && ! channelClaimed)
    {
        const auto selected = selectedPartIndex();

        if (value (selected, PEnabled) >= 0.5f && partNoteInRange (selected, note))
            outParts[count++] = selected;
    }

    return count;
}

void OneEngine::addHeldNote (int channel, int note, float velocity, unsigned int partMask)
{
    if (heldCount_ >= kMaxHeldNotes)
    {
        for (int index = 1; index < kMaxHeldNotes; ++index)
            held_[(size_t) (index - 1)] = held_[(size_t) index];

        --heldCount_;
    }

    held_[(size_t) heldCount_] = HeldNote { channel, note, velocity, false, partMask };
    ++heldCount_;
}

void OneEngine::removeHeldNote (int index)
{
    if (index < 0 || index >= heldCount_)
        return;

    for (int current = index; current < heldCount_ - 1; ++current)
        held_[(size_t) current] = held_[(size_t) (current + 1)];

    --heldCount_;
}

void OneEngine::releaseVoice (Voice& voice)
{
    if (! voice.active)
        return;

    voice.gated = false;

    for (auto& envelope : voice.envelopes)
        envelope.noteOff();
}

void OneEngine::releaseVoicesFor (int channel, int note)
{
    for (auto& voice : voices_)
        if (voice.active && voice.gated && voice.note == note && voice.channel == channel)
            releaseVoice (voice);
}

void OneEngine::releaseAllGatedVoices()
{
    for (auto& voice : voices_)
        if (voice.active && voice.gated)
            releaseVoice (voice);
}

void OneEngine::killAllVoices()
{
    for (auto& voice : voices_)
    {
        voice.active = false;
        voice.gated = false;
        voice.serial = 0;
        voice.fmSample = 0.0f;
        voice.lastEnv.fill (0.0f);
        voice.pitch = voice.targetPitch;
        voice.drift = 0.0f;

        for (auto& envelope : voice.envelopes)
            envelope.reset();

        for (auto& filter : voice.svf)
            filter.reset();

        voice.ladder.reset();
    }

    activeVoices_ = 0;
}

int OneEngine::voiceMode (int part) const { return choiceOf (value (part, PVoiceMode), 2); }
int OneEngine::lastHeldForPart (int part) const {
    for (int i = heldCount_ - 1; i >= 0; --i)
        if ((held_[i].partMask & (1u << part)) != 0) return i;
    return -1;
}
void OneEngine::killVoice (Voice& voice) {
    if (!voice.active) return;
    voice.active = voice.gated = false;
    for (auto& env : voice.envelopes) env.reset();
    activeVoices_ = std::max (0, activeVoices_ - 1);
}
void OneEngine::killPartVoices (int part) {
    for (auto& voice : voices_) if (voice.part == part) killVoice(voice);
}
void OneEngine::enforceVoiceBudget() {
    for (int i = voiceBudget_; i < kMaxVoices; ++i) killVoice(voices_[i]);
}
void OneEngine::startPartNote (int part, int note, float velocity, int channel) {
    const int mode = voiceMode(part);
    if (mode != 0) killPartVoices(part);
    const int count = mode == 2 ? std::clamp((int)std::lround(value(part,PUnisonVoices)),2,voiceBudget_) : 1;
    for (int i = 0; i < count; ++i)
        startVoice(part,note,velocity,channel,count > 1 ? (2.0f*i/(count-1)-1.0f) : 0.0f,
                   1.0f/std::sqrt((float)count));
}
void OneEngine::updateMonoPart (int part) {
    const int index=lastHeldForPart(part);
    if (index < 0) {
        for (auto& v : voices_) if (v.part==part && v.gated) releaseVoice(v);
        return;
    }
    const auto& held=held_[index];
    for (auto& v : voices_) if (v.active && v.gated && v.part==part
                               && v.note==held.note && v.channel==held.channel) return;
    startPartNote(part,held.note,held.velocity,held.channel);
}
void OneEngine::rebuildPartVoices (int part) {
    killPartVoices(part);
    if (voiceMode(part)!=0) { updateMonoPart(part); return; }
    for (int i=0;i<heldCount_;++i) if ((held_[i].partMask & (1u<<part))!=0)
        startPartNote(part,held_[i].note,held_[i].velocity,held_[i].channel);
}
void OneEngine::noteOffInternal (int channel, int note) {
    for(int i=heldCount_-1;i>=0;--i) {
        if(held_[i].channel!=channel || held_[i].note!=note) continue;
        if(sustainPedal_[channel]) held_[i].sustained=true;
        else removeHeldNote(i);
    }
    if(sustainPedal_[channel]) return;
    for(auto& v:voices_) if(v.active && v.gated && v.channel==channel && v.note==note && voiceMode(v.part)==0) releaseVoice(v);
    for(int part=0;part<kPartCount;++part) if(voiceMode(part)!=0) updateMonoPart(part);
}
void OneEngine::setSustain (int channel, bool down) {
    channel=normalizeChannel(channel);
    const bool was=sustainPedal_[channel]; sustainPedal_[channel]=down;
    if(!was || down) return;
    for(int i=heldCount_-1;i>=0;--i) if(held_[i].channel==channel && held_[i].sustained) {
        const int note=held_[i].note;
        for(auto& v:voices_) if(v.active && v.gated && v.channel==channel && v.note==note && voiceMode(v.part)==0) releaseVoice(v);
        removeHeldNote(i);
    }
    for(int part=0;part<kPartCount;++part) if(voiceMode(part)!=0) updateMonoPart(part);
}
void OneEngine::processMidi (const MidiEvent& event) {
    const int channel=normalizeChannel(event.channel);
    switch(event.type) {
        case MidiEventType::NoteOn: {
            const int note=std::clamp(event.note,0,127);
            if(event.value<=0) { noteOffInternal(channel,note); break; }
            releaseVoicesFor(channel,note);
            for(int i=heldCount_-1;i>=0;--i) if(held_[i].channel==channel && held_[i].note==note) removeHeldNote(i);
            int targets[kPartCount]{}; const int count=targetParts(channel,note,targets);
            unsigned int mask=0; for(int i=0;i<count;++i) mask|=1u<<targets[i];
            addHeldNote(channel,note,clampf(event.value,0,1),mask);
            for(int i=0;i<count;++i) startPartNote(targets[i],note,event.value,channel);
            break;
        }
        case MidiEventType::NoteOff: noteOffInternal(channel,std::clamp(event.note,0,127)); break;
        case MidiEventType::AllNotesOff: releaseAllGatedVoices(); heldCount_=0; sustainPedal_.fill(false); break;
        case MidiEventType::AllSoundOff:
            killAllVoices(); heldCount_=0; sustainPedal_.fill(false);
            for(auto& r:parts_) { r.delay.reset(); r.chorus.reset(); }
            reverb_.reset(); dcLeft_.reset(); dcRight_.reset(); break;
        case MidiEventType::PitchBend: pitchBend_=clampf(event.value,-1,1); break;
        case MidiEventType::ModWheel: modWheel_=clampf(event.value,0,1); break;
        case MidiEventType::ChannelPressure: aftertouch_=clampf(event.value,0,1); break;
        case MidiEventType::SustainPedal: setSustain(channel,event.value>0.5f); break;
        case MidiEventType::ControlChange:
            if(event.controller==64) setSustain(channel,event.value>0.5f);
            else if(event.controller==1) modWheel_=clampf(event.value,0,1);
            else if(event.controller==123) { releaseAllGatedVoices(); heldCount_=0; }
            else if(event.controller==120) { killAllVoices(); heldCount_=0; }
            break;
        case MidiEventType::NoteExpression: break;
    }
}

// ---------------------------------------------------------------------------
// Voice allocation
// ---------------------------------------------------------------------------

int OneEngine::allocateVoiceSlot() {
    for(int i=0;i<voiceBudget_;++i) if(!voices_[i].active) return i;
    int oldest=0;
    for(int i=1;i<voiceBudget_;++i) if(voices_[i].serial<voices_[oldest].serial) oldest=i;
    return oldest;
}

void OneEngine::startVoice (int part, int note, float velocity, int channel,
                            float unisonPosition, float unisonGain)
{
    const auto slot = allocateVoiceSlot();
    auto& voice = voices_[(size_t) slot];

    if (! voice.active)
        ++activeVoices_;

    voice.active = true;
    voice.gated = true;
    voice.part = std::clamp (part, 0, kPartCount - 1);
    voice.note = note;
    voice.channel = channel;
    voice.velocity = clampf (velocity, 0.0f, 1.0f);
    voice.unisonPosition = unisonPosition;
    voice.unisonGain = unisonGain;
    voice.serial = ++voiceSerial_;
    voice.targetPitch = (double) note;

    const auto glideSeconds = clampf (masterValue (MGlide), 0.0f, 1.5f);
    voice.pitch = glideSeconds > 0.0005f ? (double) parts_[(size_t) voice.part].lastNote
                                         : (double) note;
    parts_[(size_t) voice.part].lastNote = note;

    voice.drift = 0.0f;
    voice.driftTarget = voice.rng.nextBipolar();
    voice.driftCountdown = driftInterval_;
    voice.randomValue = voice.rng.nextBipolar();
    voice.fmSample = 0.0f;
    voice.lastEnv.fill (0.0f);

    for (auto& oscillator : voice.oscillators)
    {
        // A hardware VCO is not phase-reset by a key press. Independent phases
        // avoid identical, coherently summed attacks on every retrigger.
        // The per-voice PRNG is seeded in prepare(), keeping renders reproducible.
        oscillator.reset (voice.rng.nextUnipolar());
        oscillator.setFrequency (440.0);
    }

    for (auto& envelope : voice.envelopes)
        envelope.reset();

    for (auto& filter : voice.svf)
        filter.reset();

    voice.ladder.reset();
    voice.noise.reset();

    for (int env = 0; env < kEnvCount; ++env)
    {
        const auto base = kEnvBase[env];
        auto& envelope = voice.envelopes[(size_t) env];

        envelope.setParameters (value (voice.part, (PartParam) (base + 0)),
                                value (voice.part, (PartParam) (base + 1)),
                                value (voice.part, (PartParam) (base + 2)),
                                value (voice.part, (PartParam) (base + 3)),
                                value (voice.part, (PartParam) (base + 4)),
                                value (voice.part, (PartParam) (base + 5)));
        envelope.noteOn (true);
    }

    auto& runtime = parts_[(size_t) voice.part];

    for (int lfo = 0; lfo < kLfoCount; ++lfo)
    {
        const auto retrigger = choiceOf (value (voice.part,
                                                (PartParam) (PLfo1Retrig + lfo * 5)), 1);

        if (retrigger != 0)
            runtime.lfos[(size_t) lfo].reset (0.0f);

        runtime.lfoFadeTimer[(size_t) lfo] = 0;
    }
}

// ---------------------------------------------------------------------------
// Modulation rate helpers
// ---------------------------------------------------------------------------

float OneEngine::lfoRateFor (int part, int lfo) const
{
    const auto rateId = (PartParam) (PLfo1Rate + lfo * 5);
    const auto syncId = (PartParam) (PLfo1Sync + lfo * 5);

    auto rate = clampf (praw (part, rateId), 0.01f, 100.0f);

    if (praw (part, syncId) >= 0.5f)
    {
        const auto bpm = clampd (transport_.bpm > 1.0 ? transport_.bpm : 120.0, 20.0, 400.0);
        static const double divisions[] = { 0.25, 0.5, 0.75, 1.0, 1.5, 2.0,
                                            3.0, 4.0, 6.0, 8.0, 12.0, 16.0 };
        const auto beatHz = bpm / 60.0;

        double best = divisions[0];
        double bestError = 1.0e9;

        for (const auto division : divisions)
        {
            const auto hz = beatHz * division;

            if (hz <= 0.0)
                continue;

            const auto error = std::abs (std::log (hz / (double) rate));

            if (error < bestError)
            {
                bestError = error;
                best = division;
            }
        }

        rate = (float) (beatHz * best);
    }

    return clampf (rate, 0.01f, 100.0f);
}

// ---------------------------------------------------------------------------
// Rendering
// ---------------------------------------------------------------------------

void OneEngine::render (float* left, float* right, int numSamples)
{
    if (left == nullptr || right == nullptr)
        return;

    if (numSamples <= 0)
        return;

    if (! prepared_)
    {
        std::fill (left, left + numSamples, 0.0f);
        std::fill (right, right + numSamples, 0.0f);
        return;
    }

    // -----------------------------------------------------------------------
    // Block parameter snapshot. No string lookups, no allocation.
    // -----------------------------------------------------------------------
    for (int part = 0; part < kPartCount; ++part)
        for (int param = 0; param < (int) PParamCount; ++param)
            partRaw_[(size_t) part][(size_t) param] =
                values_[(size_t) partIndex_[(size_t) part][(size_t) param]];

    for (int param = 0; param < (int) MParamCount; ++param)
        masterRaw_[(size_t) param] = values_[(size_t) masterIndex_[(size_t) param]];

    const auto driftAmount = clamp01 (masterRaw_[MDrift]);
    const auto aftertouchDepth = clamp01 (masterRaw_[MAftertouch]);
    const auto bendSemitones = clampf (masterRaw_[MPitchBendRange], 0.0f, 24.0f);
    const auto glideSeconds = clampf (masterRaw_[MGlide], 0.0f, 1.5f);

    driftAmount_ = driftAmount;
    aftertouchDepth_ = aftertouchDepth;
    bendSemitones_ = bendSemitones;

    glideCoeff_ = glideSeconds > 0.0005f
                    ? 1.0f - std::exp (-1.0f / (glideSeconds * (float) sampleRate_))
                    : 1.0f;

    static const float octaveSemitones[4] = { -12.0f, 0.0f, 12.0f, 24.0f };

    for (int part = 0; part < kPartCount; ++part)
    {
        auto& runtime = parts_[(size_t) part];
        const bool enabled = praw (part, PEnabled) >= 0.5f;

        if (enabledPrev_[(size_t) part] && ! enabled)
        {
            for (auto& voice : voices_)
                if (voice.active && voice.gated && voice.part == part)
                    releaseVoice (voice);
        }

        enabledPrev_[(size_t) part] = enabled;

        static const PartParam octaveIds[3] = { PVco1Octave, PVco2Octave, PVco3Octave };
        static const PartParam tuneIds[3] = { PVco1Tune, PVco2Tune, PVco3Tune };

        for (int osc = 0; osc < 3; ++osc)
        {
            const auto octave = choiceOf (praw (part, octaveIds[osc]), 3);
            const auto tune = praw (part, tuneIds[osc]);
            vcoRatio_[(size_t) part][(size_t) osc] =
                semitonesToRatio (octaveSemitones[octave] + tune);
        }

        for (int lfo = 0; lfo < kLfoCount; ++lfo)
        {
            runtime.lfos[(size_t) lfo].setRateHz (lfoRateFor (part, lfo));
            runtime.lfos[(size_t) lfo].setWave (lfoWaveFromIndex (
                choiceOf (praw (part, (PartParam) (PLfo1Wave + lfo * 5)), 6)));

            const auto fade = clampf (praw (part, (PartParam) (PLfo1Fade + lfo * 5)),
                                      0.0f, 5.0f);
            runtime.lfoFadeLength[(size_t) lfo] = (int) (fade * (float) sampleRate_);
        }

        runtime.levelSmoother.setTarget (clamp01 (praw (part, PLevel)));
        runtime.panSmoother.setTarget (clampf (praw (part, PPan), -1.0f, 1.0f));
        runtime.mixSmoother.setTarget (clamp01 (praw (part, PFiltersMix)));
        runtime.svfCutoffSmoother.setTarget (clampf (praw (part, PSvfCutoff), 20.0f, 20000.0f));
        runtime.ladderCutoffSmoother.setTarget (
            clampf (praw (part, PLadderCutoff), 20.0f, 20000.0f));

        const auto delayTime = clampf (praw (part, PFxDelayTime), 0.01f, 2.0f);
        runtime.delay.setDelaySeconds (delayTime, std::min (2.0f, delayTime * 1.25f));
        runtime.delay.setFeedback (clamp01 (praw (part, PFxDelayFeedback)));
        runtime.delay.setMix (clamp01 (praw (part, PFxDelayMix)));

        runtime.chorus.setRateHz (clampf (praw (part, PFxChorusRate), 0.05f, 8.0f));
        runtime.chorus.setDepth (clamp01 (praw (part, PFxChorusDepth)));
        runtime.chorus.setMix (clamp01 (praw (part, PFxChorusMix)));

        for (auto& voice : voices_)
        {
            if (! voice.active || voice.part != part)
                continue;

            for (int env = 0; env < kEnvCount; ++env)
            {
                const auto base = kEnvBase[env];
                voice.envelopes[(size_t) env].setParameters (
                    praw (part, (PartParam) (base + 0)),
                    praw (part, (PartParam) (base + 1)),
                    praw (part, (PartParam) (base + 2)),
                    praw (part, (PartParam) (base + 3)),
                    praw (part, (PartParam) (base + 4)),
                    praw (part, (PartParam) (base + 5)));
            }
        }
    }

    reverb_.setSize (clamp01 (masterRaw_[MReverbSize]));
    reverb_.setDamping (clamp01 (masterRaw_[MReverbDamping]));
    reverb_.setMix (1.0f);
    masterSmoother_.setTarget (clamp01 (masterRaw_[MMasterVolume]));

    const auto reverbMix = clamp01 (masterRaw_[MReverbMix]);
    float peak = 0.0f;

    for (int sampleIndex = 0; sampleIndex < numSamples; ++sampleIndex)
    {
        // --- LFOs, part smoothers and per-part buses ----------------------
        for (int part = 0; part < kPartCount; ++part)
        {
            auto& runtime = parts_[(size_t) part];

            for (int lfo = 0; lfo < kLfoCount; ++lfo)
            {
                auto& timer = runtime.lfoFadeTimer[(size_t) lfo];
                const auto length = runtime.lfoFadeLength[(size_t) lfo];

                if (length > 0 && timer < length)
                    ++timer;

                const auto fade = length > 0 ? (float) timer / (float) length : 1.0f;
                lfoNow_[(size_t) part][(size_t) lfo] =
                    runtime.lfos[(size_t) lfo].process() * fade;
                runtime.lfoValue[(size_t) lfo] = lfoNow_[(size_t) part][(size_t) lfo];
            }

            partGain_[(size_t) part] = runtime.levelSmoother.next();
            partPan_[(size_t) part] = runtime.panSmoother.next();
            svfBase_[(size_t) part] = runtime.svfCutoffSmoother.next();
            ladderBase_[(size_t) part] = runtime.ladderCutoffSmoother.next();
            filterMix_[(size_t) part] = runtime.mixSmoother.next();
            runtime.busL = 0.0f;
            runtime.busR = 0.0f;
        }

        // --- Voices -------------------------------------------------------
        for (auto& voice : voices_)
        {
            if (! voice.active)
                continue;

            const auto sample = renderVoice (voice, sampleIndex);
            auto& bus = parts_[(size_t) voice.part];
            bus.busL += sample.left;
            bus.busR += sample.right;

            if (! voice.gated && ! voice.envelopes[2].isActive())
            {
                voice.active = false;
                --activeVoices_;
            }
        }

        // --- Per-part effects --------------------------------------------
        float dryLeft = 0.0f;
        float dryRight = 0.0f;
        float sendLeft = 0.0f;
        float sendRight = 0.0f;

        for (int part = 0; part < kPartCount; ++part)
        {
            auto& runtime = parts_[(size_t) part];
            auto partLeft = runtime.busL;
            auto partRight = runtime.busR;

            const auto distMix = clamp01 (praw (part, PFxDistMix));

            if (distMix > 0.0001f)
            {
                const auto type = (DistortionType) choiceOf (praw (part, PFxDistType), 3);
                const auto drive = 1.0f + 20.0f * clamp01 (praw (part, PFxDistDrive));
                const auto wetLeft = applyDistortion (partLeft, type, drive);
                const auto wetRight = applyDistortion (partRight, type, drive);

                partLeft = lerpf (partLeft, wetLeft * 0.85f, distMix);
                partRight = lerpf (partRight, wetRight * 0.85f, distMix);
            }

            const auto chorus = runtime.chorus.process (partLeft, partRight);
            partLeft = chorus.left;
            partRight = chorus.right;

            const auto delay = runtime.delay.process (partLeft, partRight);
            partLeft = finiteOrZero (delay.left);
            partRight = finiteOrZero (delay.right);

            dryLeft += partLeft;
            dryRight += partRight;

            const auto send = clamp01 (praw (part, PFxReverbSend));
            sendLeft += partLeft * send;
            sendRight += partRight * send;
        }

        // --- Master reverb, volume and safety stage -----------------------
        const auto reverb = reverb_.process (sendLeft, sendRight);
        const auto masterVolume = masterSmoother_.next();

        auto outLeft = (dryLeft + reverb.left * reverbMix) * masterVolume;
        auto outRight = (dryRight + reverb.right * reverbMix) * masterVolume;

        outLeft = dcLeft_.process (outLeft);
        outRight = dcRight_.process (outRight);
        outLeft = std::tanh (outLeft * 1.15f) * 0.92f;
        outRight = std::tanh (outRight * 1.15f) * 0.92f;
        outLeft = finiteOrZero (outLeft);
        outRight = finiteOrZero (outRight);

        left[sampleIndex] = outLeft;
        right[sampleIndex] = outRight;

        peak = std::max (peak, std::max (std::abs (outLeft), std::abs (outRight)));
    }

    meter_ = std::max (peak, meter_ * 0.9f);
}

StereoSample OneEngine::renderVoice (Voice& voice, int sampleIndex)
{
    const int part = voice.part;

    static const PartParam angleIds[3] = { PVco1WaveAngle, PVco2WaveAngle, PVco3WaveAngle };
    static const PartParam shapeIds[3] = { PVco1Shape, PVco2Shape, PVco3Shape };
    static const PartParam pwIds[3] = { PVco1Pw, PVco2Pw, PVco3Pw };
    static const PartParam levelIds[3] = { PVco1Level, PVco2Level, PVco3Level };

    // --- Glide and analog drift -------------------------------------------
    voice.pitch += (voice.targetPitch - voice.pitch) * (double) glideCoeff_;

    if (std::abs (voice.targetPitch - voice.pitch) < 0.0001)
        voice.pitch = voice.targetPitch;

    if (voice.driftCountdown <= 0)
    {
        voice.driftTarget = voice.rng.nextBipolar();
        voice.driftCountdown = driftInterval_;
    }
    else
    {
        --voice.driftCountdown;
    }

    voice.drift += (voice.driftTarget - voice.drift) * driftCoeff_;

    // --- Envelopes --------------------------------------------------------
    float rawEnv[kEnvCount] = { 0.0f, 0.0f, 0.0f };

    for (int env = 0; env < kEnvCount; ++env)
        rawEnv[env] = voice.envelopes[(size_t) env].process();

    // Loop the two modulation envelopes: retrigger when the decay reaches the
    // sustain plateau from above.
    for (int env = 0; env < 2; ++env)
    {
        const auto loopEnabled = praw (part, (PartParam) (kEnvBase[env] + 7)) >= 0.5f;
        const auto sustain = clamp01 (praw (part, (PartParam) (kEnvBase[env] + 4)));

        if (loopEnabled && voice.gated && rawEnv[env] > 0.0f
            && voice.lastEnv[(size_t) env] > sustain + 0.002f
            && rawEnv[env] <= sustain + 0.002f)
        {
            voice.envelopes[(size_t) env].noteOn (false);
        }
    }

    for (int env = 0; env < kEnvCount; ++env)
        voice.lastEnv[(size_t) env] = rawEnv[env];

    float envLevel[kEnvCount] = { 0.0f, 0.0f, 0.0f };

    for (int env = 0; env < kEnvCount; ++env)
    {
        const auto amount = clamp01 (praw (part, (PartParam) (kEnvBase[env] + 6)));
        const auto scale = (1.0f - amount) + amount * voice.velocity;
        envLevel[env] = rawEnv[env] * scale;
    }

    const auto filterEnv1 = clampf (envLevel[0], 0.0f, 1.2f);
    const auto filterEnv2 = clampf (envLevel[1], 0.0f, 1.2f);
    const auto ampEnv = clampf (envLevel[2], 0.0f, 1.2f);

    // --- Modulation matrix -------------------------------------------------
    float modValues[MdCount] = {};
    const auto keyTrack = clampf (((float) voice.note - 60.0f) / 48.0f, -1.0f, 1.0f);

    for (int slot = 0; slot < kModSlotCount; ++slot)
    {
        const auto slotBase = PMod1Source + slot * 3;
        const auto source = choiceOf (praw (part, (PartParam) slotBase), MsCount - 1);
        const auto dest = choiceOf (praw (part, (PartParam) (slotBase + 1)), MdCount - 1);
        const auto amount = clampf (praw (part, (PartParam) (slotBase + 2)), -1.0f, 1.0f);

        if (source <= 0 || dest <= 0 || amount == 0.0f)
            continue;

        float sourceValue = 0.0f;

        switch (source)
        {
            case MsLfo1:
            case MsLfo2:
            case MsLfo3:
            case MsLfo4:
                sourceValue = lfoNow_[(size_t) part][(size_t) (source - MsLfo1)];
                break;

            case MsEnv1:
            case MsEnv2:
            case MsEnv3:
                sourceValue = envLevel[source - MsEnv1];
                break;

            case MsVelocity:
                sourceValue = voice.velocity;
                break;

            case MsAftertouch:
                sourceValue = aftertouch_;
                break;

            case MsModWheel:
                sourceValue = modWheel_;
                break;

            case MsKeyTrack:
                sourceValue = keyTrack;
                break;

            case MsRandom:
                sourceValue = voice.randomValue;
                break;

            default:
                break;
        }

        modValues[dest] += sourceValue * amount;
    }

    const auto pitchMod1 = clampf (modValues[MdVco1Pitch], -1.5f, 1.5f) * 24.0f;
    const auto pitchMod2 = clampf (modValues[MdVco2Pitch], -1.5f, 1.5f) * 24.0f;
    const auto pitchMod3 = clampf (modValues[MdVco3Pitch], -1.5f, 1.5f) * 24.0f;
    const auto pulseWidthMod = clampf (modValues[MdPw], -1.0f, 1.0f) * 0.45f;
    const auto lfoPwm = lfoNow_[(size_t) part][1] * praw (part, PLfo2PwmAmt) * 0.45f;

    // --- Oscillator frequencies -------------------------------------------
    auto baseSemitones = (float) voice.pitch
                       + voice.unisonPosition * praw(part, PUnisonDetune) * 0.5f
                       + voice.drift * driftAmount_ * 0.12f
                       + pitchBend_ * bendSemitones_
                       + lfoNow_[(size_t) part][0] * praw (part, PLfo1PitchAmt) * 12.0f;
    baseSemitones = clampf (baseSemitones, -72.0f, 120.0f);

    const auto baseHz = noteToHz (baseSemitones);
    const auto pitchRatio1 = pitchMod1 != 0.0f ? std::exp2 (pitchMod1 * (1.0f / 12.0f)) : 1.0f;
    const auto pitchRatio2 = pitchMod2 != 0.0f ? std::exp2 (pitchMod2 * (1.0f / 12.0f)) : 1.0f;
    const auto pitchRatio3 = pitchMod3 != 0.0f ? std::exp2 (pitchMod3 * (1.0f / 12.0f)) : 1.0f;

    auto frequency1 = baseHz * vcoRatio_[(size_t) part][0] * pitchRatio1 + praw (part, PVco1BeatHz);
    auto frequency2 = baseHz * vcoRatio_[(size_t) part][1] * pitchRatio2 + praw (part, PVco2BeatHz);
    auto frequency3 = baseHz * vcoRatio_[(size_t) part][2] * pitchRatio3 + praw (part, PVco3BeatHz);

    const auto fmAmount = clamp01 (praw (part, PVco3Fm));

    if (fmAmount > 0.0001f)
        frequency1 += voice.fmSample * fmAmount * baseHz
                    * vcoRatio_[(size_t) part][0] * 1.5f;

    const auto maxFrequency = (float) sampleRate_ * kMaxCutoffFraction;
    frequency1 = clampf (frequency1, 1.0f, maxFrequency);
    frequency2 = clampf (frequency2, 1.0f, maxFrequency);
    frequency3 = clampf (frequency3, 1.0f, maxFrequency);

    auto& oscillator1 = voice.oscillators[0];
    auto& oscillator2 = voice.oscillators[1];
    auto& oscillator3 = voice.oscillators[2];

    oscillator1.setFrequency (frequency1);
    oscillator2.setFrequency (frequency2);
    oscillator3.setFrequency (frequency3);

    const auto masterWrap = oscillator1.advance();
    oscillator2.advance();
    oscillator3.advance();

    const auto sync2 = choiceOf (praw (part, PVco2Sync), 1) != 0;
    const auto sync3 = choiceOf (praw (part, PVco3Sync), 1) != 0;

    if (masterWrap)
    {
        if (sync2)
            oscillator2.resetPhase();

        if (sync3)
            oscillator3.resetPhase();
    }

    const auto pulseWidth1 = clampf (praw (part, pwIds[0]) + lfoPwm + pulseWidthMod,
                                      0.05f, 0.95f);
    const auto pulseWidth2 = clampf (praw (part, pwIds[1]) + lfoPwm + pulseWidthMod,
                                      0.05f, 0.95f);
    const auto pulseWidth3 = clampf (praw (part, pwIds[2]) + lfoPwm + pulseWidthMod,
                                      0.05f, 0.95f);

    const auto osc1 = morphWaveSample (oscillator1, praw (part, angleIds[0]),
                                        praw (part, shapeIds[0]), pulseWidth1);
    const auto osc2 = morphWaveSample (oscillator2, praw (part, angleIds[1]),
                                        praw (part, shapeIds[1]), pulseWidth2);
    const auto osc3 = morphWaveSample (oscillator3, praw (part, angleIds[2]),
                                        praw (part, shapeIds[2]), pulseWidth3);

    voice.fmSample = finiteOrZero (osc3);

    // --- Mixer -------------------------------------------------------------
    const auto vco1Enabled = praw (part, PVco1On) >= 0.5f;
    const auto level1 = vco1Enabled ? clamp01 (praw (part, levelIds[0])
                                                + modValues[MdOsc1Level]) : 0.0f;
    const auto level2 = clamp01 (praw (part, levelIds[1]) + modValues[MdOsc2Level]);
    const auto level3 = clamp01 (praw (part, levelIds[2]) + modValues[MdOsc3Level]);
    const auto ringAmount = clamp01 (praw (part, PMixRing));
    const auto noiseLevel = clamp01 (praw (part, PMixNoise) + modValues[MdNoiseLevel]);
    const auto noiseIsPink = choiceOf (praw (part, PMixNoiseColor), 1) != 0;

    auto signal = osc1 * level1 + osc2 * level2 + osc3 * level3;
    signal += osc1 * osc2 * ringAmount;

    const auto noiseSample = noiseIsPink ? voice.noise.pink() : voice.noise.white();
    signal += noiseSample * noiseLevel;

    const auto externalLevel = clamp01 (praw (part, PMixExt));

    if (externalLevel > 0.0f && externalInput_ != nullptr && sampleIndex < externalInputSize_)
        signal += externalInput_[sampleIndex] * externalLevel;

    signal *= 0.32f;

    // --- Filters -----------------------------------------------------------
    const auto svfKeyOctaves = praw (part, PSvfKeyTrack)
                             * ((float) voice.note - 60.0f) / 12.0f;
    const auto svfExponent = filterEnv1 * praw (part, PSvfEnvAmt) * 6.0f
                           + svfKeyOctaves
                           + lfoNow_[(size_t) part][2] * praw (part, PLfo3CutoffAmt) * 4.0f
                           + aftertouch_ * aftertouchDepth_ * 2.0f
                           + clampf (modValues[MdSvfCutoff], -2.0f, 2.0f) * 5.0f;
    const auto svfCutoff = clampf (svfBase_[(size_t) part]
                                     * std::exp2 (clampf (svfExponent, -20.0f, 20.0f)),
                                   20.0f, maxFrequency);
    const auto svfResonance = clamp01 (praw (part, PSvfResonance)
                                        + modValues[MdSvfResonance]);
    const auto svfMode = (StateVariableFilter::Mode)
        choiceOf (praw (part, PSvfMode), 3);
    const auto svfSlope = choiceOf (praw (part, PSvfSlope), 2);
    const auto svfOutputLevel = clamp01 (praw (part, PSvfLevel));

    voice.svf[0].setCutoffHz (svfCutoff);
    voice.svf[0].setResonance01 (svfResonance);

    // A whisper of noise keeps a very high-Q SVF ringing (self-oscillation).
    const auto excitation = voice.noise.white() * svfResonance * svfResonance
                          * svfResonance * 0.0012f;
    const auto svfInput = signal + excitation;
    const auto stage1 = voice.svf[0].process (svfInput, svfMode);

    float svfFiltered;

    if (svfSlope == 0)
    {
        svfFiltered = 0.5f * (svfInput + stage1);
    }
    else if (svfSlope == 2)
    {
        voice.svf[1].setCutoffHz (svfCutoff);
        voice.svf[1].setResonance01 (svfResonance * 0.75f);
        svfFiltered = voice.svf[1].process (stage1, svfMode);
    }
    else
    {
        svfFiltered = stage1;
    }

    const auto svfOutput = svfFiltered * svfOutputLevel;

    const auto ladderKeyOctaves = praw (part, PLadderKeyTrack)
                                * ((float) voice.note - 60.0f) / 12.0f;
    const auto ladderExponent = filterEnv2 * praw (part, PLadderEnvAmt) * 6.0f
                              + ladderKeyOctaves
                              + aftertouch_ * aftertouchDepth_ * 2.0f
                              + clampf (modValues[MdLadderCutoff], -2.0f, 2.0f) * 5.0f;
    const auto ladderCutoff = clampf (ladderBase_[(size_t) part]
                                        * std::exp2 (clampf (ladderExponent, -20.0f, 20.0f)),
                                      20.0f, maxFrequency);
    const auto ladderResonance = clamp01 (praw (part, PLadderResonance)
                                           + modValues[MdLadderResonance]);

    voice.ladder.setCutoffHz (ladderCutoff);
    voice.ladder.setResonance01 (ladderResonance);
    voice.ladder.setDrive (1.0f + 8.0f * clamp01 (praw (part, PLadderDrive)));
    voice.ladder.setSlope (std::clamp (choiceOf (praw (part, PLadderSlope), 3) + 1, 1, 4));

    const auto serialRouting = choiceOf (praw (part, PFiltersRouting), 1) == 0;
    auto ladderInput = serialRouting ? svfOutput : signal;

    // A whisper of noise also keeps the ladder ringing at extreme resonance.
    ladderInput += voice.noise.white() * ladderResonance * ladderResonance
                 * ladderResonance * 0.0010f;

    const auto ladderOutput = voice.ladder.process (ladderInput);

    const auto filterMix = clamp01 (filterMix_[(size_t) part] + modValues[MdFilterMix]);
    const auto filtered = lerpf (svfOutput, ladderOutput, filterMix);

    // --- VCA and pan -------------------------------------------------------
    const auto volumeMod = clampf (1.0f + modValues[MdVolume], 0.0f, 2.0f);
    const auto vca = clampf (ampEnv * partGain_[(size_t) part] * volumeMod, 0.0f, 2.0f);
    const auto pan = clampf (partPan_[(size_t) part] + modValues[MdPan], -1.0f, 1.0f);
    const auto gainLeft = std::sqrt (0.5f * (1.0f - pan));
    const auto gainRight = std::sqrt (0.5f * (1.0f + pan));

    StereoSample output;
    output.left = finiteOrZero (filtered * vca * voice.unisonGain * gainLeft);
    output.right = finiteOrZero (filtered * vca * voice.unisonGain * gainRight);
    return output;
}

std::unique_ptr<SynthEngine> makeOneEngine()
{
    return std::make_unique<OneEngine>();
}

} // namespace triad
