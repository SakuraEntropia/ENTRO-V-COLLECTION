#include "engines/PolyBrute.h"

#include <algorithm>
#include <cmath>
#include <string>

namespace triad
{

namespace
{
constexpr float pbPi = 3.14159265358979323846f;

std::vector<std::string> choices (std::initializer_list<const char*> items)
{
    std::vector<std::string> result;
    result.reserve (items.size());

    for (const auto* item : items)
        result.emplace_back (item);

    return result;
}

constexpr float kOctaveSemitones[4] = { -12.0f, 0.0f, 12.0f, 24.0f };
constexpr int kBendRanges[6] = { 1, 2, 3, 5, 7, 12 };

LfoWave lfoWaveFromChoice (int index)
{
    switch (index)
    {
        case 0: return LfoWave::Sine;
        case 1: return LfoWave::Triangle;
        case 2: return LfoWave::SawUp;
        case 3: return LfoWave::Square;
        case 4: return LfoWave::SampleHold;
        default: return LfoWave::RandomSmooth;
    }
}

} // namespace

PolyBruteEngine::PolyBruteEngine()
{
    buildDefinition();

    const auto count = definition_.params.size();
    snapshotA_.resize (count);
    snapshotB_.resize (count);
    effective_.resize (count);

    for (size_t i = 0; i < count; ++i)
        snapshotA_[i] = definition_.params[i].def;

    snapshotB_ = snapshotA_;
    effective_ = snapshotA_;

    for (size_t i = 0; i < voices_.size(); ++i)
    {
        voices_[i].rng.seed (0x9e3779b9u + (uint32_t) i * 0x85ebca6bu);
        voices_[i].driftPitchA = voices_[i].rng.nextBipolar() * 0.5f;
        voices_[i].driftPitchB = voices_[i].rng.nextBipolar() * 0.5f;
        voices_[i].driftCutoff = voices_[i].rng.nextBipolar() * 0.5f;
        voices_[i].driftPw = voices_[i].rng.nextBipolar() * 0.5f;
    }
}

void PolyBruteEngine::buildDefinition()
{
    definition_.id = "polybrute";
    definition_.displayName = "POLYBRUTE V";
    definition_.manufacturer = "SakuraEntropia";

    auto add = [this] (ParamId id, const char* paramId, const char* name,
                       const char* group, float minimum, float maximum, float def,
                       ParamCurve curve = ParamCurve::Linear,
                       std::vector<std::string> list = {}, const char* unit = "")
    {
        ParamSpec spec;
        spec.id = paramId;
        spec.name = name;
        spec.group = group;
        spec.min = minimum;
        spec.max = maximum;
        spec.def = def;
        spec.curve = curve;
        spec.choices = std::move (list);
        spec.unit = unit;
        paramIndex_[(size_t) id] = definition_.addParam (std::move (spec));
    };

    // ---- Voice -----------------------------------------------------------
    add (PVoiceMode, "poly.voice.mode", "VOICE MODE", "Voice", 0.0f, 2.0f, 0.0f,
         ParamCurve::Discrete, choices ({ "Poly", "Mono", "Unison" }));
    add (PVoiceUnison, "poly.voice.unison", "UNISON VOICES", "Voice", 0.0f, 2.0f, 0.0f,
         ParamCurve::Discrete, choices ({ "2 Voices", "3 Voices", "4 Voices" }));
    add (PVoiceGlide, "poly.voice.glide", "GLIDE", "Voice", 0.002f, 2.0f, 0.002f,
         ParamCurve::Log, {}, "s");
    add (PVoiceSpread, "poly.voice.spread", "SPREAD", "Voice", 0.0f, 1.0f, 0.25f);
    add (PVoiceAnalog, "poly.voice.analog", "ANALOG DRIFT", "Voice", 0.0f, 1.0f, 0.45f);
    add (PVoiceBendRange, "poly.voice.pitchBendRange", "PITCH BEND RANGE", "Voice",
         0.0f, 5.0f, 1.0f, ParamCurve::Discrete,
         choices ({ "1 st", "2 st", "3 st", "5 st", "7 st", "12 st" }));
    add (PVoiceMorph, "poly.voice.morph", "MORPH A/B", "Voice", 0.0f, 1.0f, 0.0f);

    // ---- Oscillator A ----------------------------------------------------
    add (PVcoAOctave, "poly.vcoA.octave", "OCTAVE", "Oscillator A", 0.0f, 3.0f, 1.0f,
         ParamCurve::Discrete, choices ({ "16'", "8'", "4'", "2'" }));
    add (PVcoATune, "poly.vcoA.tune", "TUNE", "Oscillator A", -24.0f, 24.0f, 0.0f,
         ParamCurve::Linear, {}, "st");
    add (PVcoAWave, "poly.vcoA.wave", "SAW / TRIANGLE", "Oscillator A", 0.0f, 1.0f, 0.0f);
    add (PVcoAPw, "poly.vcoA.pw", "PULSE WIDTH", "Oscillator A", 0.05f, 0.95f, 0.5f);
    add (PVcoAPulse, "poly.vcoA.pulse", "PULSE LEVEL", "Oscillator A", 0.0f, 1.0f, 0.0f);
    add (PVcoALevel, "poly.vcoA.level", "LEVEL", "Oscillator A", 0.0f, 1.0f, 0.75f);
    add (PVcoAMetalizer, "poly.vcoA.metalizer", "METALIZER", "Oscillator A", 0.0f, 1.0f, 0.0f);

    // ---- Oscillator B ----------------------------------------------------
    add (PVcoBOctave, "poly.vcoB.octave", "OCTAVE", "Oscillator B", 0.0f, 3.0f, 1.0f,
         ParamCurve::Discrete, choices ({ "16'", "8'", "4'", "2'" }));
    add (PVcoBTune, "poly.vcoB.tune", "TUNE", "Oscillator B", -24.0f, 24.0f, 0.0f,
         ParamCurve::Linear, {}, "st");
    add (PVcoBWave, "poly.vcoB.wave", "SAW / TRIANGLE", "Oscillator B", 0.0f, 1.0f, 0.0f);
    add (PVcoBPw, "poly.vcoB.pw", "PULSE WIDTH", "Oscillator B", 0.05f, 0.95f, 0.5f);
    add (PVcoBPulse, "poly.vcoB.pulse", "PULSE LEVEL", "Oscillator B", 0.0f, 1.0f, 0.0f);
    add (PVcoBLevel, "poly.vcoB.level", "LEVEL", "Oscillator B", 0.0f, 1.0f, 0.65f);
    add (PVcoBSub, "poly.vcoB.sub", "SUB OSC", "Oscillator B", 0.0f, 1.0f, 0.0f);
    add (PVcoBSync, "poly.vcoB.sync", "HARD SYNC", "Oscillator B", 0.0f, 1.0f, 0.0f);
    add (PVcoBFm, "poly.vcoB.fm", "VCO B > VCO A FM", "Oscillator B", 0.0f, 1.0f, 0.0f);

    // ---- Mixer -----------------------------------------------------------
    add (PMixNoise, "poly.mixer.noise", "NOISE", "Mixer", 0.0f, 1.0f, 0.0f);
    add (PMixNoiseColor, "poly.mixer.noiseColor", "NOISE COLOR", "Mixer", 0.0f, 1.0f, 0.0f,
         ParamCurve::Discrete, choices ({ "White", "Pink" }));

    // ---- Steiner filter --------------------------------------------------
    add (PSteinerMode, "poly.steiner.mode", "MODE", "Steiner Filter", 0.0f, 1.0f, 0.0f);
    add (PSteinerCutoff, "poly.steiner.cutoff", "CUTOFF", "Steiner Filter", 20.0f, 18000.0f,
         1200.0f, ParamCurve::Log, {}, "Hz");
    add (PSteinerResonance, "poly.steiner.resonance", "RESONANCE", "Steiner Filter",
         0.0f, 1.0f, 0.2f);
    add (PSteinerEnvAmt, "poly.steiner.envAmt", "ENV AMOUNT", "Steiner Filter",
         -1.0f, 1.0f, 0.35f);
    add (PSteinerLevel, "poly.steiner.level", "LEVEL", "Steiner Filter", 0.0f, 1.0f, 0.8f);
    add (PSteinerKeyTrack, "poly.steiner.keyTrack", "KEY TRACK", "Steiner Filter",
         0.0f, 1.0f, 0.3f);
    add (PSteinerFm, "poly.steiner.fm", "FILTER FM", "Steiner Filter", 0.0f, 1.0f, 0.0f);

    // ---- Ladder filter ---------------------------------------------------
    add (PLadderCutoff, "poly.ladder.cutoff", "CUTOFF", "Ladder Filter", 20.0f, 18000.0f,
         2000.0f, ParamCurve::Log, {}, "Hz");
    add (PLadderResonance, "poly.ladder.resonance", "RESONANCE", "Ladder Filter",
         0.0f, 1.0f, 0.15f);
    add (PLadderDrive, "poly.ladder.drive", "BRUTE FACTOR", "Ladder Filter",
         1.0f, 10.0f, 1.0f);
    add (PLadderEnvAmt, "poly.ladder.envAmt", "ENV AMOUNT", "Ladder Filter",
         -1.0f, 1.0f, 0.2f);
    add (PLadderLevel, "poly.ladder.level", "LEVEL", "Ladder Filter", 0.0f, 1.0f, 0.8f);
    add (PLadderKeyTrack, "poly.ladder.keyTrack", "KEY TRACK", "Ladder Filter",
         0.0f, 1.0f, 0.3f);

    add (PFiltersRouting, "poly.filters.routing", "ROUTING", "Steiner Filter", 0.0f, 2.0f,
         0.0f, ParamCurve::Discrete, choices ({ "Serial", "Parallel", "Split" }));
    add (PFiltersBalance, "poly.filters.balance", "FILTER BALANCE", "Steiner Filter",
         0.0f, 1.0f, 0.5f);

    // ---- Envelopes -------------------------------------------------------
    const float envDefs[3][5] = {
        { 0.005f, 0.4f, 0.75f, 0.35f, 0.5f },
        { 0.003f, 0.5f, 0.8f, 0.4f, 0.4f },
        { 0.02f, 0.8f, 0.2f, 0.5f, 0.3f }
    };

    for (int env = 0; env < 3; ++env)
    {
        const auto base = PEnv1Attack + env * 5;
        const auto prefix = std::string ("poly.env") + std::to_string (env + 1);
        const char* group = "Envelopes";
        const char* names[5] = { "ATTACK", "DECAY", "SUSTAIN", "RELEASE", "VELOCITY" };

        add ((ParamId) (base + 0), (prefix + ".attack").c_str(), names[0], group,
             0.002f, 10.0f, envDefs[env][0], ParamCurve::Log, {}, "s");
        add ((ParamId) (base + 1), (prefix + ".decay").c_str(), names[1], group,
             0.002f, 10.0f, envDefs[env][1], ParamCurve::Log, {}, "s");
        add ((ParamId) (base + 2), (prefix + ".sustain").c_str(), names[2], group,
             0.0f, 1.0f, envDefs[env][2]);
        add ((ParamId) (base + 3), (prefix + ".release").c_str(), names[3], group,
             0.002f, 10.0f, envDefs[env][3], ParamCurve::Log, {}, "s");
        add ((ParamId) (base + 4), (prefix + ".velocity").c_str(), names[4], group,
             0.0f, 1.0f, envDefs[env][4]);
    }

    // ---- LFOs ------------------------------------------------------------
    const float lfoRates[3] = { 4.5f, 0.35f, 0.2f };
    const int lfoWaves[3] = { 0, 1, 0 };
    const int lfoRetrig[3] = { 1, 1, 1 };

    for (int lfo = 0; lfo < 3; ++lfo)
    {
        const auto base = PLfo1Rate + lfo * 5;
        const auto prefix = std::string ("poly.lfo") + std::to_string (lfo + 1);

        add ((ParamId) (base + 0), (prefix + ".rate").c_str(), "RATE", "LFOs",
             0.01f, 100.0f, lfoRates[lfo], ParamCurve::Log, {}, "Hz");
        add ((ParamId) (base + 1), (prefix + ".wave").c_str(), "WAVEFORM", "LFOs",
             0.0f, 5.0f, (float) lfoWaves[lfo], ParamCurve::Discrete,
             choices ({ "Sine", "Triangle", "Saw", "Square", "S&H", "Random" }));
        add ((ParamId) (base + 2), (prefix + ".sync").c_str(), "TEMPO SYNC", "LFOs",
             0.0f, 1.0f, 0.0f, ParamCurve::Discrete, choices ({ "Off", "On" }));
        add ((ParamId) (base + 3), (prefix + ".retrigger").c_str(), "RETRIGGER", "LFOs",
             0.0f, 1.0f, (float) lfoRetrig[lfo], ParamCurve::Discrete,
             choices ({ "Off", "On" }));
        add ((ParamId) (base + 4), (prefix + ".fade").c_str(), "FADE IN", "LFOs",
             0.0f, 5.0f, 0.0f, ParamCurve::Linear, {}, "s");
    }

    add (PLfo1PitchAmt, "poly.lfo1.pitchAmt", "LFO 1 > PITCH", "LFOs", 0.0f, 1.0f, 0.0f);
    add (PLfo2PwmAmt, "poly.lfo2.pwmAmt", "LFO 2 > PWM", "LFOs", 0.0f, 1.0f, 0.0f);
    add (PLfo3CutoffAmt, "poly.lfo3.cutoffAmt", "LFO 3 > CUTOFF", "LFOs",
         0.0f, 1.0f, 0.0f);

    // ---- Modulation matrix ----------------------------------------------
    const auto modSourceNames = choices ({ "None", "LFO 1", "LFO 2", "LFO 3", "Env 1",
                                           "Env 2", "Env 3", "Velocity", "Aftertouch",
                                           "Mod Wheel", "Key Track", "Random" });
    const auto modDestNames = choices ({ "None", "VCO A Pitch", "VCO B Pitch",
                                         "VCO A Pulse Width", "VCO B Pulse Width",
                                         "Metalizer", "VCO A Level", "VCO B Level",
                                         "Noise Level", "Steiner Cutoff", "Ladder Cutoff",
                                         "Steiner Resonance", "Ladder Resonance",
                                         "Filter A Level", "Filter B Level", "Pan",
                                         "LFO Rates", "Volume" });

    for (int slot = 0; slot < kNumModSlots; ++slot)
    {
        const auto base = PMod1Source + slot * 3;
        const auto prefix = std::string ("poly.mod.slot") + std::to_string (slot + 1);

        add ((ParamId) (base + 0), (prefix + ".source").c_str(), "SOURCE", "Mod Matrix",
             0.0f, (float) (SrcCount - 1), 0.0f, ParamCurve::Discrete, modSourceNames);
        add ((ParamId) (base + 1), (prefix + ".dest").c_str(), "DESTINATION", "Mod Matrix",
             0.0f, (float) (DestCount - 1), 0.0f, ParamCurve::Discrete, modDestNames);
        add ((ParamId) (base + 2), (prefix + ".amount").c_str(), "AMOUNT", "Mod Matrix",
             -1.0f, 1.0f, 0.0f);
    }

    // ---- Effects ---------------------------------------------------------
    add (PFxDistType, "poly.fx.dist.type", "DISTORTION TYPE", "Effects", 0.0f, 3.0f, 0.0f,
         ParamCurve::Discrete, choices ({ "Soft", "Hard", "Fold", "Tube" }));
    add (PFxDistDrive, "poly.fx.dist.drive", "DISTORTION DRIVE", "Effects",
         1.0f, 10.0f, 1.0f);
    add (PFxDistMix, "poly.fx.dist.mix", "DISTORTION MIX", "Effects", 0.0f, 1.0f, 0.0f);

    add (PFxChorusRate, "poly.fx.chorus.rate", "CHORUS RATE", "Effects",
         0.05f, 8.0f, 0.4f, ParamCurve::Log, {}, "Hz");
    add (PFxChorusDepth, "poly.fx.chorus.depth", "CHORUS DEPTH", "Effects",
         0.0f, 1.0f, 0.35f);
    add (PFxChorusMix, "poly.fx.chorus.mix", "CHORUS MIX", "Effects", 0.0f, 1.0f, 0.0f);

    add (PFxDelayTime, "poly.fx.delay.time", "DELAY TIME", "Effects", 0.02f, 1.5f, 0.35f,
         ParamCurve::Log, {}, "s");
    add (PFxDelayFeedback, "poly.fx.delay.feedback", "DELAY FEEDBACK", "Effects",
         0.0f, 0.95f, 0.3f);
    add (PFxDelayMix, "poly.fx.delay.mix", "DELAY MIX", "Effects", 0.0f, 1.0f, 0.0f);
    add (PFxDelayPingPong, "poly.fx.delay.pingPong", "DELAY PING PONG", "Effects",
         0.0f, 1.0f, 0.0f, ParamCurve::Discrete, choices ({ "Off", "On" }));
    add (PFxDelaySync, "poly.fx.delay.sync", "DELAY TEMPO SYNC", "Effects",
         0.0f, 1.0f, 0.0f, ParamCurve::Discrete, choices ({ "Off", "On" }));

    add (PFxReverbSize, "poly.fx.reverb.size", "REVERB SIZE", "Effects",
         0.0f, 1.0f, 0.5f);
    add (PFxReverbDamping, "poly.fx.reverb.damping", "REVERB DAMPING", "Effects",
         0.0f, 1.0f, 0.4f);
    add (PFxReverbMix, "poly.fx.reverb.mix", "REVERB MIX", "Effects", 0.0f, 1.0f, 0.0f);

    // ---- Arpeggiator (host-driven; persisted only) -----------------------
    add (PArpEnabled, "poly.arp.enabled", "ARPEGGIATOR", "Arpeggiator", 0.0f, 1.0f, 0.0f,
         ParamCurve::Discrete, choices ({ "Off", "On" }));
    add (PArpMode, "poly.arp.mode", "MODE", "Arpeggiator", 0.0f, 5.0f, 0.0f,
         ParamCurve::Discrete,
         choices ({ "Up", "Down", "Up/Down", "Random", "Order", "Step" }));
    add (PArpRate, "poly.arp.rate", "RATE", "Arpeggiator", 0.0f, 5.0f, 3.0f,
         ParamCurve::Discrete, choices ({ "1/4", "1/8", "1/8T", "1/16", "1/16T", "1/32" }));
    add (PArpOctaves, "poly.arp.octaves", "OCTAVES", "Arpeggiator", 0.0f, 3.0f, 0.0f,
         ParamCurve::Discrete, choices ({ "1", "2", "3", "4" }));
    add (PArpHold, "poly.arp.hold", "HOLD", "Arpeggiator", 0.0f, 1.0f, 0.0f,
         ParamCurve::Discrete, choices ({ "Off", "On" }));

    std::vector<std::string> arpLengths;
    arpLengths.reserve (16);
    for (int step = 1; step <= 16; ++step)
        arpLengths.push_back (std::to_string (step));

    add (PArpLength, "poly.arp.length", "LENGTH", "Arpeggiator", 1.0f, 16.0f, 16.0f,
         ParamCurve::Discrete, std::move (arpLengths));

    definition_.modSources.push_back ({ "none", "None" });
    definition_.modSources.push_back ({ "lfo1", "LFO 1" });
    definition_.modSources.push_back ({ "lfo2", "LFO 2" });
    definition_.modSources.push_back ({ "lfo3", "LFO 3" });
    definition_.modSources.push_back ({ "env1", "Env 1" });
    definition_.modSources.push_back ({ "env2", "Env 2" });
    definition_.modSources.push_back ({ "env3", "Env 3" });
    definition_.modSources.push_back ({ "velocity", "Velocity" });
    definition_.modSources.push_back ({ "aftertouch", "Aftertouch" });
    definition_.modSources.push_back ({ "modwheel", "Mod Wheel" });
    definition_.modSources.push_back ({ "keytrack", "Key Track" });
    definition_.modSources.push_back ({ "random", "Random" });

    definition_.modDestinations.push_back ({ "none", "None" });
    definition_.modDestinations.push_back ({ "vcoAPitch", "VCO A Pitch" });
    definition_.modDestinations.push_back ({ "vcoBPitch", "VCO B Pitch" });
    definition_.modDestinations.push_back ({ "vcoAPw", "VCO A Pulse Width" });
    definition_.modDestinations.push_back ({ "vcoBPw", "VCO B Pulse Width" });
    definition_.modDestinations.push_back ({ "metalizer", "Metalizer" });
    definition_.modDestinations.push_back ({ "vcoALevel", "VCO A Level" });
    definition_.modDestinations.push_back ({ "vcoBLevel", "VCO B Level" });
    definition_.modDestinations.push_back ({ "noiseLevel", "Noise Level" });
    definition_.modDestinations.push_back ({ "steinerCutoff", "Steiner Cutoff" });
    definition_.modDestinations.push_back ({ "ladderCutoff", "Ladder Cutoff" });
    definition_.modDestinations.push_back ({ "steinerResonance", "Steiner Resonance" });
    definition_.modDestinations.push_back ({ "ladderResonance", "Ladder Resonance" });
    definition_.modDestinations.push_back ({ "filterALevel", "Filter A Level" });
    definition_.modDestinations.push_back ({ "filterBLevel", "Filter B Level" });
    definition_.modDestinations.push_back ({ "pan", "Pan" });
    definition_.modDestinations.push_back ({ "lfoRates", "LFO Rates" });
    definition_.modDestinations.push_back ({ "volume", "Volume" });
}

void PolyBruteEngine::prepare (double sampleRate, int maxBlockSize)
{
    sampleRate_ = std::max (8000.0, sampleRate);
    maxBlockSize_ = std::max (1, maxBlockSize);

    const auto sr = (float) sampleRate_;

    chorus_.prepare (sampleRate_);
    delay_.prepare (sampleRate_, 2.5f);
    reverb_.prepare (sampleRate_);
    dcLeft_.prepare (sampleRate_);
    dcRight_.prepare (sampleRate_);

    for (size_t i = 0; i < voices_.size(); ++i)
    {
        auto& v = voices_[i];
        const auto seed = 0x2545f491u + (uint32_t) i * 0x9e3779b9u;

        v.oscA.prepare (sampleRate_);
        v.oscBFree.prepare (sampleRate_);
        v.oscBSync.prepare (sampleRate_);
        v.subOsc.prepare (sampleRate_);
        v.steiner.prepare (sampleRate_);
        v.steinerOsc.prepare (sampleRate_);
        v.ladder.prepare (sampleRate_);
        v.noise.prepare (sampleRate_, seed);
        v.rng.seed (seed);

        for (auto& env : v.env)
            env.prepare (sampleRate_);

        for (int lfo = 0; lfo < kNumLfos; ++lfo)
            v.lfo[(size_t) lfo].prepare (sampleRate_, seed + 0x1000u * (uint32_t) (lfo + 1));

        v.driftPitchA = v.rng.nextBipolar() * 0.5f;
        v.driftPitchB = v.rng.nextBipolar() * 0.5f;
        v.driftCutoff = v.rng.nextBipolar() * 0.5f;
        v.driftPw = v.rng.nextBipolar() * 0.5f;
        (void) sr;
    }

    reset();
}

void PolyBruteEngine::reset()
{
    heldCount_ = 0;
    sustainedCount_ = 0;
    sustainPedal_ = false;
    pitchBend_ = 0.0f;
    modWheel_ = 0.0f;
    aftertouch_ = 0.0f;
    lastPlayedNote_ = -1.0f;
    ageCounter_ = 0;
    lastMode_ = -1;
    lastMaxVoices_ = -1;
    meter_ = 0.0f;

    for (auto& v : voices_)
    {
        v.active = false;
        v.held = false;
        v.sustained = false;
        v.note = 60;
        v.targetNote = 60.0f;
        v.glideNote = 60.0f;
        v.detune = 0.0f;
        v.pan = 0.0f;
        v.randomValue = 0.0f;
        v.age = 0;
        v.lastSteinerOsc = 0.0f;
        v.lastVcoB = 0.0f;

        for (auto& env : v.env)
            env.reset();

        for (int lfo = 0; lfo < kNumLfos; ++lfo)
        {
            v.lfo[(size_t) lfo].reset();
            v.lfoFade[(size_t) lfo] = 1.0f;
            v.lfoFadeInc[(size_t) lfo] = 1.0f;
        }

        v.oscA.reset();
        v.oscBFree.reset();
        v.oscBSync.reset();
        v.subOsc.reset();
        v.steiner.reset();
        v.steinerOsc.reset();
        v.ladder.reset();
        v.noise.reset();
    }

    chorus_.reset();
    delay_.reset();
    reverb_.reset();
    dcLeft_.reset();
    dcRight_.reset();
    meter_ = 0.0f;
}

void PolyBruteEngine::setParameters (const float* values, int count)
{
    if (values == nullptr || count <= 0)
        return;

    const auto n = std::min (count, (int) snapshotA_.size());

    for (int i = 0; i < n; ++i)
        snapshotA_[(size_t) i] = values[i];
}

void PolyBruteEngine::setMorphSnapshot (const float* values, int count)
{
    snapshotB_ = snapshotA_;

    if (values == nullptr || count <= 0)
        return;

    const auto n = std::min (count, (int) snapshotB_.size());

    for (int i = 0; i < n; ++i)
        snapshotB_[(size_t) i] = values[i];
}

void PolyBruteEngine::setMorphAmount (float amount)
{
    morphAmount_ = clampf (amount, 0.0f, 1.0f);
    morphExplicit_ = true;
}

void PolyBruteEngine::setTransport (const TransportInfo& transport)
{
    transport_ = transport;
}

void PolyBruteEngine::updateMorph()
{
    const auto morphIndex = (size_t) paramIndex_[(size_t) PVoiceMorph];
    auto morph = morphExplicit_ ? morphAmount_ : snapshotA_[morphIndex];
    morph = clampf (std::isfinite (morph) ? morph : 0.0f, 0.0f, 1.0f);

    for (size_t i = 0; i < effective_.size(); ++i)
    {
        const auto& spec = definition_.params[i];
        const auto a = snapshotA_[i];
        const auto b = snapshotB_[i];

        if (spec.curve == ParamCurve::Discrete)
            effective_[i] = morph < 0.5f ? a : b;
        else
            effective_[i] = lerpf (a, b, morph);
    }

    effective_[morphIndex] = morph;
    clampEffectiveValues();
}

void PolyBruteEngine::clampEffectiveValues()
{
    for (size_t i = 0; i < effective_.size(); ++i)
    {
        const auto& spec = definition_.params[i];
        auto value = effective_[i];

        if (! std::isfinite (value))
            value = spec.def;

        effective_[i] = clampf (value, spec.min, spec.max);
    }
}

void PolyBruteEngine::reconcileVoiceMode (int mode, int maxVoices)
{
    // Poly mode keeps its individual voices; mono and unison collapse the
    // stack onto the most recent held note when the mode changes.
    if (mode == 0)
        return;

    for (int i = maxVoices; i < kNumVoices; ++i)
    {
        auto& v = voices_[(size_t) i];

        if (v.active && v.held)
            releaseVoice (v, false);
    }

    if (heldCount_ == 0)
        return;

    const auto top = heldNotes_[(size_t) (heldCount_ - 1)];
    const auto velocity = heldVelocities_[(size_t) (heldCount_ - 1)];

    for (int i = 0; i < maxVoices; ++i)
    {
        auto& v = voices_[(size_t) i];

        if (v.active)
            retargetVoice (v, top, velocity, false, i, maxVoices);
        else
            startVoice (i, top, velocity, true, i, maxVoices);
    }
}

void PolyBruteEngine::updateModSlots()
{
    for (int slot = 0; slot < kNumModSlots; ++slot)
    {
        const auto base = PMod1Source + slot * 3;
        auto& target = modSlots_[(size_t) slot];
        target.source = std::clamp ((int) std::lround (param ((ParamId) (base + 0))),
                                    0, SrcCount - 1);
        target.dest = std::clamp ((int) std::lround (param ((ParamId) (base + 1))),
                                  0, DestCount - 1);
        target.amount = clampf (param ((ParamId) (base + 2)), -1.0f, 1.0f);
    }
}

void PolyBruteEngine::updateDrift()
{
    const auto analog = param (PVoiceAnalog);

    if (analog <= 0.0f)
        return;

    for (auto& v : voices_)
    {
        if (! v.active)
            continue;

        v.driftPitchA += (v.rng.nextBipolar() - v.driftPitchA) * 0.015f;
        v.driftPitchB += (v.rng.nextBipolar() - v.driftPitchB) * 0.015f;
        v.driftCutoff += (v.rng.nextBipolar() - v.driftCutoff) * 0.012f;
        v.driftPw += (v.rng.nextBipolar() - v.driftPw) * 0.02f;
    }
}

void PolyBruteEngine::configureVoice (Voice& v)
{
    const auto sr = (float) sampleRate_;

    for (int env = 0; env < kNumEnvelopes; ++env)
    {
        const auto base = PEnv1Attack + env * 5;
        v.env[(size_t) env].setParameters (param ((ParamId) (base + 0)),
                                           param ((ParamId) (base + 1)),
                                           param ((ParamId) (base + 2)),
                                           param ((ParamId) (base + 3)));
    }

    const auto bpm = std::max (20.0, transport_.bpm);

    for (int lfo = 0; lfo < kNumLfos; ++lfo)
    {
        const auto base = PLfo1Rate + lfo * 5;
        const auto wave = lfoWaveFromChoice (choice ((ParamId) (base + 1)));
        const auto sync = choice ((ParamId) (base + 2)) != 0;
        const auto rate = lfoRateFor (param ((ParamId) (base + 0)), sync, bpm);
        const auto fade = param ((ParamId) (base + 4));

        v.lfo[(size_t) lfo].setWave (wave);
        v.lfoBaseRate[(size_t) lfo] = rate;
        v.lfo[(size_t) lfo].setRateHz (rate);
        v.lfoFadeInc[(size_t) lfo] = fade > 0.0001f
                                   ? 1.0f / (fade * sr)
                                   : 1.0f;

        if (fade <= 0.0001f)
            v.lfoFade[(size_t) lfo] = 1.0f;
    }
}

float PolyBruteEngine::lfoRateFor (float rateHz, bool tempoSync, double bpm)
{
    const auto clamped = clampf (rateHz, 0.01f, 100.0f);

    if (! tempoSync)
        return clamped;

    // Map the free-running rate control onto musical divisions of the beat.
    static const float divisions[] = { 0.25f, 1.0f / 3.0f, 0.5f, 2.0f / 3.0f, 1.0f,
                                       1.5f, 2.0f, 3.0f, 4.0f, 6.0f, 8.0f, 12.0f, 16.0f };
    constexpr int numDivisions = (int) (sizeof (divisions) / sizeof (divisions[0]));

    const auto norm = (std::log (clamped) - std::log (0.01f))
                    / (std::log (100.0f) - std::log (0.01f));
    const auto index = std::clamp ((int) std::lround (clampf (norm, 0.0f, 1.0f)
                                                      * (float) (numDivisions - 1)),
                                   0, numDivisions - 1);
    const auto secondsPerBeat = 60.0 / std::max (20.0, bpm);
    return clampf ((float) (divisions[index] / secondsPerBeat), 0.01f, 100.0f);
}

float PolyBruteEngine::oscillatorSample (const Oscillator& osc, float waveMorph,
                                         float pulseWidth, float pulseLevel) noexcept
{
    const auto saw = osc.sample (Oscillator::Wave::Saw);
    const auto triangle = osc.sample (Oscillator::Wave::Triangle);
    const auto pulse = osc.sample (Oscillator::Wave::Pulse,
                                   clampf (pulseWidth, 0.05f, 0.95f));
    const auto base = lerpf (saw, triangle, clampf (waveMorph, 0.0f, 1.0f));
    return lerpf (base, pulse, clampf (pulseLevel, 0.0f, 1.0f));
}

float PolyBruteEngine::metalize (float input, float amount) noexcept
{
    const auto depth = clampf (amount, 0.0f, 1.0f);

    if (depth <= 0.0001f)
        return input;

    const auto driven = input * (1.0f + 6.0f * depth);
    const auto folded = std::sin (clampf (driven, -8.0f, 8.0f) * (pbPi * 0.5f));
    return lerpf (input, folded, depth);
}

int PolyBruteEngine::polyphonyForMode() const
{
    switch (choice (PVoiceMode))
    {
        case 1: return 1;
        case 2: return std::clamp (choice (PVoiceUnison) + 2, 2, 4);
        default: return kNumVoices;
    }
}

void PolyBruteEngine::startVoice (int index, int note, float velocity, bool forceReset,
                                  int unisonIndex, int unisonCount)
{
    if (index < 0 || index >= kNumVoices)
        return;

    auto& v = voices_[(size_t) index];
    const bool fresh = forceReset || ! v.active;
    const auto analog = param (PVoiceAnalog);

    if (fresh)
    {
        for (auto& env : v.env)
            env.reset();

        v.steiner.reset();
        v.steinerOsc.reset();
        v.ladder.reset();
        v.lastSteinerOsc = 0.0f;
        v.lastVcoB = 0.0f;

        const auto phase = analog * v.rng.nextUnipolar();
        v.oscA.reset ((double) phase);
        v.oscBFree.reset ((double) (phase * 0.73f));
        v.oscBSync.reset ((double) (phase * 0.73f));
        v.subOsc.reset();
        v.noise.reset();
    }

    v.active = true;
    v.held = true;
    v.sustained = false;
    v.note = std::clamp (note, 0, 127);
    v.targetNote = (float) v.note;
    v.velocity = clampf (velocity, 0.0f, 1.0f);

    if (fresh)
    {
        const auto glide = param (PVoiceGlide);
        v.glideNote = (glide > 0.0025f && lastPlayedNote_ >= 0.0f)
                    ? lastPlayedNote_
                    : (float) v.note;
    }
    else if (param (PVoiceGlide) <= 0.0025f)
    {
        v.glideNote = (float) v.note;
    }

    const auto spread = param (PVoiceSpread);

    if (unisonCount > 1)
    {
        const auto position = (float) unisonIndex - 0.5f * (float) (unisonCount - 1);
        v.detune = clampf (spread * 0.3f * position, -1.0f, 1.0f);
        v.pan = clampf (spread * 1.4f * position
                        / std::max (1.0f, (float) (unisonCount - 1)), -1.0f, 1.0f);
    }
    else
    {
        v.detune = spread * v.rng.nextBipolar() * 0.04f;
        v.pan = spread * v.rng.nextBipolar() * 0.45f;
    }

    v.randomValue = v.rng.nextBipolar();
    v.age = ++ageCounter_;

    for (auto& env : v.env)
        env.noteOn (true);

    for (int lfo = 0; lfo < kNumLfos; ++lfo)
    {
        const auto base = PLfo1Rate + lfo * 5;
        const auto retrigger = choice ((ParamId) (base + 3)) != 0;
        const auto fade = param ((ParamId) (base + 4));

        if (fresh || retrigger)
            v.lfo[(size_t) lfo].reset (retrigger ? 0.0f : v.rng.nextUnipolar());

        v.lfoFade[(size_t) lfo] = fade > 0.0001f ? 0.0f : 1.0f;
    }
}

void PolyBruteEngine::retargetVoice (Voice& v, int note, float velocity,
                                     bool retrigger, int unisonIndex, int unisonCount)
{
    v.active = true;
    v.held = true;
    v.sustained = false;
    v.note = std::clamp (note, 0, 127);
    v.targetNote = (float) v.note;
    v.velocity = clampf (velocity, 0.0f, 1.0f);

    if (param (PVoiceGlide) <= 0.0025f)
        v.glideNote = (float) v.note;

    const auto spread = param (PVoiceSpread);

    if (unisonCount > 1)
    {
        const auto position = (float) unisonIndex - 0.5f * (float) (unisonCount - 1);
        v.detune = clampf (spread * 0.3f * position, -1.0f, 1.0f);
        v.pan = clampf (spread * 1.4f * position
                        / std::max (1.0f, (float) (unisonCount - 1)), -1.0f, 1.0f);
    }

    if (retrigger)
    {
        for (auto& env : v.env)
            env.noteOn (true);
    }
}

void PolyBruteEngine::releaseVoice (Voice& v, bool hard)
{
    if (hard)
    {
        v.active = false;
        v.held = false;
        v.sustained = false;
        v.lastSteinerOsc = 0.0f;
        v.lastVcoB = 0.0f;

        for (auto& env : v.env)
            env.reset();

        v.steiner.reset();
        v.steinerOsc.reset();
        v.ladder.reset();
    }
    else
    {
        v.held = false;
        v.sustained = false;

        for (auto& env : v.env)
            env.noteOff();
    }
}

int PolyBruteEngine::findVoiceToSteal() const
{
    int best = -1;
    float quietest = 1.0e9f;
    uint64_t oldest = UINT64_MAX;

    for (int i = 0; i < kNumVoices; ++i)
    {
        if (! voices_[(size_t) i].active)
            return i;
    }

    // Prefer voices that are already released, then the quietest, then oldest.
    for (int i = 0; i < kNumVoices; ++i)
    {
        const auto& v = voices_[(size_t) i];

        if (v.held)
            continue;

        const auto level = v.env[1].level();

        if (best < 0 || level < quietest - 1.0e-6f
            || (std::abs (level - quietest) <= 1.0e-6f && v.age < oldest))
        {
            best = i;
            quietest = level;
            oldest = v.age;
        }
    }

    if (best >= 0)
        return best;

    for (int i = 0; i < kNumVoices; ++i)
    {
        const auto& v = voices_[(size_t) i];
        const auto level = v.env[1].level();

        if (best < 0 || level < quietest - 1.0e-6f
            || (std::abs (level - quietest) <= 1.0e-6f && v.age < oldest))
        {
            best = i;
            quietest = level;
            oldest = v.age;
        }
    }

    return best < 0 ? 0 : best;
}

void PolyBruteEngine::noteOn (int note, float velocity)
{
    note = std::clamp (note, 0, 127);
    velocity = clampf (velocity, 0.0f, 1.0f);

    if (velocity <= 0.0f)
    {
        noteOff (note);
        return;
    }

    // Keep the held-note stack at most 16 entries (drop the oldest).
    if (heldCount_ >= kMaxHeldNotes)
    {
        for (int i = 1; i < heldCount_; ++i)
        {
            heldNotes_[(size_t) (i - 1)] = heldNotes_[(size_t) i];
            heldVelocities_[(size_t) (i - 1)] = heldVelocities_[(size_t) i];
        }

        --heldCount_;
    }

    // Retriggering the same key does not duplicate stack entries.
    for (int i = 0; i < heldCount_; ++i)
    {
        if (heldNotes_[(size_t) i] != note)
            continue;

        for (int j = i; j < heldCount_ - 1; ++j)
        {
            heldNotes_[(size_t) j] = heldNotes_[(size_t) (j + 1)];
            heldVelocities_[(size_t) j] = heldVelocities_[(size_t) (j + 1)];
        }

        --heldCount_;
        break;
    }

    heldNotes_[(size_t) heldCount_] = note;
    heldVelocities_[(size_t) heldCount_] = velocity;
    ++heldCount_;

    for (int i = 0; i < sustainedCount_; ++i)
    {
        if (sustainedNotes_[(size_t) i] != note)
            continue;

        for (int j = i; j < sustainedCount_ - 1; ++j)
            sustainedNotes_[(size_t) j] = sustainedNotes_[(size_t) (j + 1)];

        --sustainedCount_;
        break;
    }

    const auto mode = choice (PVoiceMode);

    if (mode == 0)
    {
        int target = -1;

        for (int i = 0; i < kNumVoices; ++i)
        {
            if (voices_[(size_t) i].active && voices_[(size_t) i].held
                && voices_[(size_t) i].note == note)
            {
                target = i;
                break;
            }
        }

        const bool sameHeldNote = target >= 0;

        if (target < 0)
        {
            for (int i = 0; i < kNumVoices; ++i)
            {
                if (! voices_[(size_t) i].active)
                {
                    target = i;
                    break;
                }
            }
        }

        if (target < 0)
            target = findVoiceToSteal();

        const bool forceReset = ! sameHeldNote;
        startVoice (target, note, velocity, forceReset, 0, 1);
    }
    else
    {
        const auto count = mode == 2 ? std::clamp (choice (PVoiceUnison) + 2, 2, 4) : 1;

        for (int i = 0; i < count; ++i)
        {
            auto& v = voices_[(size_t) i];
            startVoice (i, note, velocity, ! v.active, i, count);
        }

        for (int i = count; i < kNumVoices; ++i)
        {
            auto& v = voices_[(size_t) i];

            if (v.active && v.held)
                releaseVoice (v, false);
        }
    }

    lastPlayedNote_ = (float) note;
}

void PolyBruteEngine::noteOff (int note)
{
    note = std::clamp (note, 0, 127);

    bool removed = false;

    for (int i = 0; i < heldCount_; ++i)
    {
        if (heldNotes_[(size_t) i] != note)
            continue;

        for (int j = i; j < heldCount_ - 1; ++j)
        {
            heldNotes_[(size_t) j] = heldNotes_[(size_t) (j + 1)];
            heldVelocities_[(size_t) j] = heldVelocities_[(size_t) (j + 1)];
        }

        --heldCount_;
        removed = true;
        break;
    }

    if (! removed)
        return;

    if (sustainPedal_)
    {
        bool listed = false;

        for (int i = 0; i < sustainedCount_; ++i)
        {
            if (sustainedNotes_[(size_t) i] == note)
            {
                listed = true;
                break;
            }
        }

        if (! listed && sustainedCount_ < kMaxHeldNotes)
            sustainedNotes_[(size_t) sustainedCount_++] = note;
    }

    bool stillHeld = false;

    for (int i = 0; i < heldCount_; ++i)
    {
        if (heldNotes_[(size_t) i] == note)
        {
            stillHeld = true;
            break;
        }
    }

    const auto mode = choice (PVoiceMode);

    if (mode == 0)
    {
        if (stillHeld)
            return;

        for (auto& v : voices_)
        {
            if (! v.active || v.note != note || ! v.held)
                continue;

            v.held = false;

            if (sustainPedal_)
            {
                v.sustained = true;
            }
            else
            {
                v.sustained = false;
                releaseVoice (v, false);
            }
        }

        return;
    }

    if (heldCount_ > 0)
    {
        const auto top = heldNotes_[(size_t) (heldCount_ - 1)];
        const auto velocity = heldVelocities_[(size_t) (heldCount_ - 1)];
        const auto count = mode == 2 ? std::clamp (choice (PVoiceUnison) + 2, 2, 4) : 1;

        for (int i = 0; i < count; ++i)
        {
            if (voices_[(size_t) i].active)
                retargetVoice (voices_[(size_t) i], top, velocity, false, i, count);
        }
    }
    else if (sustainPedal_)
    {
        for (auto& v : voices_)
        {
            if (! v.active)
                continue;

            v.held = false;
            v.sustained = true;
        }
    }
    else
    {
        for (auto& v : voices_)
        {
            if (v.active)
                releaseVoice (v, false);
        }
    }
}

void PolyBruteEngine::allNotesOff (bool hard)
{
    if (hard)
    {
        allSoundOff();
        return;
    }

    heldCount_ = 0;
    sustainedCount_ = 0;
    sustainPedal_ = false;

    for (auto& v : voices_)
    {
        if (v.active)
            releaseVoice (v, false);
    }
}

void PolyBruteEngine::allSoundOff()
{
    heldCount_ = 0;
    sustainedCount_ = 0;
    sustainPedal_ = false;

    for (auto& v : voices_)
        releaseVoice (v, true);

    chorus_.reset();
    delay_.reset();
    reverb_.reset();
    dcLeft_.reset();
    dcRight_.reset();
    meter_ = 0.0f;
}

void PolyBruteEngine::handleSustain (bool down)
{
    if (down)
    {
        sustainPedal_ = true;
        return;
    }

    sustainPedal_ = false;

    const auto mode = choice (PVoiceMode);

    if (mode == 0)
    {
        for (auto& v : voices_)
        {
            if (v.active && v.sustained && ! v.held)
            {
                v.sustained = false;
                releaseVoice (v, false);
            }
        }
    }
    else if (heldCount_ > 0)
    {
        const auto top = heldNotes_[(size_t) (heldCount_ - 1)];
        const auto velocity = heldVelocities_[(size_t) (heldCount_ - 1)];
        const auto count = mode == 2 ? std::clamp (choice (PVoiceUnison) + 2, 2, 4) : 1;

        for (int i = 0; i < count; ++i)
        {
            if (voices_[(size_t) i].active)
                retargetVoice (voices_[(size_t) i], top, velocity, false, i, count);
        }
    }
    else
    {
        for (auto& v : voices_)
        {
            if (! v.active)
                continue;

            v.sustained = false;
            releaseVoice (v, false);
        }
    }

    sustainedCount_ = 0;
}

void PolyBruteEngine::processMidi (const MidiEvent& event)
{
    // MIDI can arrive before the next render call, so make parameter edits
    // visible to note allocation immediately (this does not allocate).
    updateMorph();

    switch (event.type)
    {
        case MidiEventType::NoteOn:
            noteOn (event.note, event.value);
            break;

        case MidiEventType::NoteOff:
            noteOff (event.note);
            break;

        case MidiEventType::AllNotesOff:
            allNotesOff (false);
            break;

        case MidiEventType::AllSoundOff:
            allSoundOff();
            break;

        case MidiEventType::PitchBend:
            pitchBend_ = clampf (event.value, -1.0f, 1.0f);
            break;

        case MidiEventType::ModWheel:
            modWheel_ = clampf (event.value, 0.0f, 1.0f);
            break;

        case MidiEventType::ChannelPressure:
            aftertouch_ = clampf (event.value, 0.0f, 1.0f);
            break;

        case MidiEventType::SustainPedal:
            handleSustain (event.value > 0.5f);
            break;

        case MidiEventType::ControlChange:
            if (event.controller == 64)
                handleSustain (event.value > 0.5f);
            else if (event.controller == 1)
                modWheel_ = clampf (event.value, 0.0f, 1.0f);
            else if (event.controller == 123)
                allNotesOff (false);
            else if (event.controller == 120)
                allSoundOff();
            break;
    }
}

int PolyBruteEngine::activeVoiceCount() const
{
    int count = 0;

    for (const auto& v : voices_)
    {
        if (v.active)
            ++count;
    }

    return count;
}

void PolyBruteEngine::render (float* left, float* right, int numSamples)
{
    if (left == nullptr || right == nullptr || numSamples <= 0)
        return;

    updateMorph();
    updateModSlots();
    updateDrift();

    const auto mode = choice (PVoiceMode);
    const auto maxVoices = polyphonyForMode();

    if (mode != lastMode_ || maxVoices != lastMaxVoices_)
    {
        lastMode_ = mode;
        lastMaxVoices_ = maxVoices;
        reconcileVoiceMode (mode, maxVoices);
    }

    for (auto& v : voices_)
        configureVoice (v);

    const auto sr = (float) sampleRate_;
    const auto maxCutoff = sr * 0.45f;
    const auto glideSeconds = param (PVoiceGlide);
    const auto glideCoefficient = glideSeconds <= 0.0025f
                                ? 1.0f
                                : 1.0f - std::exp (-1.0f / (0.35f * glideSeconds * sr));

    const auto octaveA = std::clamp (choice (PVcoAOctave), 0, 3);
    const auto octaveB = std::clamp (choice (PVcoBOctave), 0, 3);
    const auto semisA = kOctaveSemitones[octaveA] + param (PVcoATune);
    const auto semisB = kOctaveSemitones[octaveB] + param (PVcoBTune);

    const auto waveA = param (PVcoAWave);
    const auto waveB = param (PVcoBWave);
    const auto pwBaseA = param (PVcoAPw);
    const auto pwBaseB = param (PVcoBPw);
    const auto pulseA = param (PVcoAPulse);
    const auto pulseB = param (PVcoBPulse);
    const auto levelBaseA = param (PVcoALevel);
    const auto levelBaseB = param (PVcoBLevel);
    const auto subBase = param (PVcoBSub);
    const auto syncBase = param (PVcoBSync);
    const auto fmAmount = param (PVcoBFm);
    const auto metalBase = param (PVcoAMetalizer);

    const auto noiseBase = param (PMixNoise);
    const auto noiseColor = choice (PMixNoiseColor);

    const auto steinerMode = param (PSteinerMode) * 3.0f;
    const auto steinerCutBase = param (PSteinerCutoff);
    const auto steinerResBase = param (PSteinerResonance);
    const auto steinerEnvAmt = param (PSteinerEnvAmt);
    const auto steinerLevelBase = param (PSteinerLevel);
    const auto steinerKeyTrack = param (PSteinerKeyTrack);
    const auto steinerFm = param (PSteinerFm);

    const auto ladderCutBase = param (PLadderCutoff);
    const auto ladderResBase = param (PLadderResonance);
    const auto ladderDrive = param (PLadderDrive);
    const auto ladderEnvAmt = param (PLadderEnvAmt);
    const auto ladderLevelBase = param (PLadderLevel);
    const auto ladderKeyTrack = param (PLadderKeyTrack);

    const auto routing = choice (PFiltersRouting);
    const auto balance = param (PFiltersBalance);

    const auto env1Velocity = param (PEnv1Velocity);
    const auto env2Velocity = param (PEnv2Velocity);
    const auto env3Velocity = param (PEnv3Velocity);

    const auto lfo1PitchAmt = param (PLfo1PitchAmt);
    const auto lfo2PwmAmt = param (PLfo2PwmAmt);
    const auto lfo3CutoffAmt = param (PLfo3CutoffAmt);
    const auto wheelScale = 0.2f + 0.8f * modWheel_;

    const auto analog = param (PVoiceAnalog);
    const auto bendRangeIndex = std::clamp (choice (PVoiceBendRange), 0, 5);
    const auto bendSemitones = pitchBend_ * (float) kBendRanges[bendRangeIndex];

    for (auto& v : voices_)
        v.ladder.setDrive (ladderDrive);

    // Nonlinear stages (Metalizer, ladder/Steiner tanh, distortion) run at the
    // host sample rate. 2x oversampling was judged too expensive for six
    // voices at 96 kHz, so it is deliberately not applied; the oscillators are
    // PolyBLEP band-limited instead.

    // ---- Global effects --------------------------------------------------
    const auto distType = choice (PFxDistType);
    const auto distDrive = param (PFxDistDrive);
    const auto distMix = param (PFxDistMix);

    chorus_.setRateHz (param (PFxChorusRate));
    chorus_.setDepth (param (PFxChorusDepth));
    chorus_.setMix (param (PFxChorusMix));

    auto delayTime = param (PFxDelayTime);

    if (choice (PFxDelaySync) != 0)
    {
        const auto norm = (std::log (clampf (delayTime, 0.02f, 1.5f)) - std::log (0.02f))
                        / (std::log (1.5f) - std::log (0.02f));
        static const double divisions[] = { 0.125, 1.0 / 6.0, 0.25, 0.375,
                                            0.5, 0.75, 1.0, 1.5 };
        const auto index = std::clamp ((int) std::lround (clampf (norm, 0.0f, 1.0f) * 7.0f),
                                       0, 7);
        const auto bpm = std::max (20.0, transport_.bpm);
        delayTime = clampf ((float) (60.0 / bpm * divisions[index]), 0.02f, 1.5f);
    }

    delay_.setDelaySeconds (delayTime, delayTime);
    delay_.setFeedback (param (PFxDelayFeedback));
    delay_.setMix (param (PFxDelayMix));
    delay_.setPingPong (choice (PFxDelayPingPong) != 0);
    delay_.setDampingHz (7000.0f);

    reverb_.setSize (param (PFxReverbSize));
    reverb_.setDamping (param (PFxReverbDamping));
    reverb_.setMix (param (PFxReverbMix));

    float peak = 0.0f;

    for (int sample = 0; sample < numSamples; ++sample)
    {
        float sumLeft = 0.0f;
        float sumRight = 0.0f;

        for (int voiceIndex = 0; voiceIndex < kNumVoices; ++voiceIndex)
        {
            auto& v = voices_[(size_t) voiceIndex];

            if (! v.active)
                continue;

            // ---- Glide / portamento -------------------------------------
            v.glideNote += (v.targetNote - v.glideNote) * glideCoefficient;

            if (std::abs (v.targetNote - v.glideNote) < 1.0e-4f)
                v.glideNote = v.targetNote;

            // ---- LFOs ---------------------------------------------------
            float lfoValue[kNumLfos] {};

            for (int lfo = 0; lfo < kNumLfos; ++lfo)
            {
                if (v.lfoFade[(size_t) lfo] < 1.0f)
                    v.lfoFade[(size_t) lfo] = std::min (1.0f, v.lfoFade[(size_t) lfo]
                                                              + v.lfoFadeInc[(size_t) lfo]);

                lfoValue[(size_t) lfo] = v.lfo[(size_t) lfo].process()
                                       * v.lfoFade[(size_t) lfo];
            }

            // ---- Envelopes ----------------------------------------------
            const auto velocityScale1 = (1.0f - env1Velocity) + env1Velocity * v.velocity;
            const auto velocityScale2 = (1.0f - env2Velocity) + env2Velocity * v.velocity;
            const auto velocityScale3 = (1.0f - env3Velocity) + env3Velocity * v.velocity;
            const auto env1 = v.env[0].process() * velocityScale1;
            const auto env2 = v.env[1].process() * velocityScale2;
            const auto env3 = v.env[2].process() * velocityScale3;

            // ---- Modulation matrix (all eight slots) --------------------
            float modDest[DestCount] {};

            for (const auto& slot : modSlots_)
            {
                if (slot.source == SrcNone || slot.dest == DestNone)
                    continue;

                float source = 0.0f;

                switch (slot.source)
                {
                    case SrcLfo1: source = lfoValue[0]; break;
                    case SrcLfo2: source = lfoValue[1]; break;
                    case SrcLfo3: source = lfoValue[2]; break;
                    case SrcEnv1: source = env1; break;
                    case SrcEnv2: source = env2; break;
                    case SrcEnv3: source = env3; break;
                    case SrcVelocity: source = v.velocity; break;
                    case SrcAftertouch: source = aftertouch_; break;
                    case SrcModWheel: source = modWheel_; break;
                    case SrcKeyTrack:
                        source = clampf (((float) v.note - 60.0f) / 24.0f, -1.0f, 1.0f);
                        break;
                    case SrcRandom: source = v.randomValue; break;
                    default: break;
                }

                modDest[slot.dest] += source * slot.amount;
            }

            const auto rateFactor = std::exp2 (clampf (modDest[DestLfoRates], -2.0f, 2.0f)
                                               * 2.0f);

            for (int lfo = 0; lfo < kNumLfos; ++lfo)
            {
                v.lfo[(size_t) lfo].setRateHz (clampf (v.lfoBaseRate[(size_t) lfo]
                                                        * rateFactor, 0.01f, 100.0f));
            }

            // ---- Oscillators --------------------------------------------
            const auto lfoPitch = lfoValue[0] * lfo1PitchAmt * wheelScale * 4.0f;
            const auto keyTrack = clampf (((float) v.note - 60.0f) / 12.0f, -8.0f, 8.0f);

            auto pitchA = v.glideNote + bendSemitones + v.detune
                        + v.driftPitchA * analog * 0.12f + lfoPitch
                        + modDest[DestVcoAPitch] * 24.0f + semisA;
            auto pitchB = v.glideNote + bendSemitones + v.detune
                        + v.driftPitchB * analog * 0.12f + lfoPitch
                        + modDest[DestVcoBPitch] * 24.0f + semisB;

            pitchA = clampf (pitchA, -96.0f, 180.0f);
            pitchB = clampf (pitchB, -96.0f, 180.0f);

            auto freqA = noteToHz (pitchA);
            auto freqB = noteToHz (pitchB);

            // VCO B -> VCO A linear FM (one sample of B feedback).
            freqA += v.lastVcoB * fmAmount * 1500.0f;
            freqA = clampf (freqA, 1.0f, maxCutoff);
            freqB = clampf (freqB, 1.0f, maxCutoff);

            v.oscA.setFrequency ((double) freqA);
            v.oscBFree.setFrequency ((double) freqB);
            v.oscBSync.setFrequency ((double) freqB);
            v.subOsc.setFrequency ((double) (freqB * 0.5f));

            const auto wrappedA = v.oscA.advance();
            v.oscBFree.advance();

            if (wrappedA)
                v.oscBSync.resetPhase();
            else
                v.oscBSync.advance();

            v.subOsc.advance();

            const auto lfoPwm = lfoValue[1] * lfo2PwmAmt * wheelScale * 0.45f;
            const auto pwA = clampf (pwBaseA + lfoPwm + modDest[DestVcoAPw] * 0.45f
                                      + v.driftPw * analog * 0.02f, 0.05f, 0.95f);
            const auto pwB = clampf (pwBaseB + lfoPwm + modDest[DestVcoBPw] * 0.45f
                                      + v.driftPw * analog * 0.02f, 0.05f, 0.95f);

            auto signalA = oscillatorSample (v.oscA, waveA, pwA, pulseA);
            const auto metal = clampf (metalBase + modDest[DestMetalizer], 0.0f, 1.0f);

            if (metal > 0.0001f)
                signalA = metalize (signalA, metal);

            // Continuous soft-to-hard sync: the free-running and the
            // A-triggered reset oscillator are crossfaded. The reset itself is
            // a phase reset (no BLEP correction), which is intentional for the
            // aggressive sync character.
            const auto freeB = oscillatorSample (v.oscBFree, waveB, pwB, pulseB);
            const auto syncedB = oscillatorSample (v.oscBSync, waveB, pwB, pulseB);
            auto signalB = lerpf (freeB, syncedB, clampf (syncBase, 0.0f, 1.0f));
            signalB += v.subOsc.sample (Oscillator::Wave::Sine) * subBase;
            signalB = clampf (signalB, -2.0f, 2.0f);
            v.lastVcoB = signalB;

            const auto noiseSample = noiseColor == 1 ? v.noise.pink() : v.noise.white();
            const auto levelA = clampf (levelBaseA + modDest[DestVcoALevel], 0.0f, 1.5f);
            const auto levelB = clampf (levelBaseB + modDest[DestVcoBLevel], 0.0f, 1.5f);
            const auto noiseLevel = clampf (noiseBase + modDest[DestNoiseLevel], 0.0f, 1.5f);
            auto mixed = signalA * levelA + signalB * levelB + noiseSample * noiseLevel;
            mixed = clampf (mixed * 0.4f, -6.0f, 6.0f);

            // ---- Steiner filter ----------------------------------------
            auto steinerOctaves = steinerEnvAmt * env1 * 5.0f
                                + steinerKeyTrack * keyTrack
                                + lfoValue[2] * lfo3CutoffAmt * wheelScale * 4.0f
                                + aftertouch_ * 0.5f
                                + modDest[DestSteinerCutoff] * 5.0f
                                + v.driftCutoff * analog * 0.25f
                                + steinerFm * v.lastVcoB * 2.0f;
            const auto steinerCutoff = clampf (steinerCutBase
                                                * std::exp2 (clampf (steinerOctaves, -8.0f, 8.0f)),
                                                20.0f, maxCutoff);
            const auto steinerRes = clampf (steinerResBase + modDest[DestSteinerResonance],
                                            0.0f, 1.0f);
            const auto steinerLevel = clampf (steinerLevelBase + modDest[DestFilterALevel],
                                              0.0f, 1.5f);

            v.steiner.setCutoffHz (steinerCutoff);
            v.steiner.setResonance01 (steinerRes);

            auto steinerInput = mixed;
            const auto selfOsc = clampf ((steinerRes - 0.85f) / 0.15f, 0.0f, 1.0f);

            if (selfOsc > 0.0f)
            {
                // Above ~85% resonance a band-pass feedback loop around a
                // second SVF core turns the Steiner into a true self-oscillator.
                // The tanh inside StateVariableFilter and the clamps bound it.
                v.steinerOsc.setCutoffHz (steinerCutoff);
                v.steinerOsc.setResonance01 (steinerRes);

                const auto feedback = clampf (v.lastSteinerOsc, -1.5f, 1.5f) * 0.65f;
                const auto seed = 2.0e-4f * noiseSample;
                const auto band = v.steinerOsc.process (feedback + seed,
                                                        StateVariableFilter::Mode::BandPass);
                v.lastSteinerOsc = clampf (band, -4.0f, 4.0f);
                steinerInput += v.lastSteinerOsc * selfOsc * 0.1f;
                steinerInput = clampf (steinerInput, -8.0f, 8.0f);
            }

            auto filterA = v.steiner.process (steinerInput, steinerMode) * steinerLevel;
            filterA = clampf (filterA, -8.0f, 8.0f);

            // ---- Ladder filter -----------------------------------------
            auto ladderOctaves = ladderEnvAmt * env1 * 5.0f
                               + ladderKeyTrack * keyTrack
                               + lfoValue[2] * lfo3CutoffAmt * wheelScale * 4.0f
                               + aftertouch_ * 0.35f
                               + modDest[DestLadderCutoff] * 5.0f
                               + v.driftCutoff * analog * 0.2f;
            const auto ladderCutoff = clampf (ladderCutBase
                                               * std::exp2 (clampf (ladderOctaves, -8.0f, 8.0f)),
                                               20.0f, maxCutoff);
            const auto ladderRes = clampf (ladderResBase + modDest[DestLadderResonance],
                                           0.0f, 1.0f);
            const auto ladderLevel = clampf (ladderLevelBase + modDest[DestFilterBLevel],
                                             0.0f, 1.5f);

            v.ladder.setCutoffHz (ladderCutoff);
            v.ladder.setResonance01 (ladderRes);

            auto ladderInput = routing == 0 ? filterA : mixed;

            if (ladderRes > 0.95f)
            {
                // Seed the ladder so it can self-oscillate from silence when
                // the resonance is pushed into the feedback region.
                ladderInput += (ladderRes - 0.95f) * 20.0f * 1.0e-4f * noiseSample;
            }

            auto filterB = v.ladder.process (ladderInput) * ladderLevel;
            filterB = clampf (filterB, -8.0f, 8.0f);

            if (! std::isfinite (filterA) || ! std::isfinite (filterB))
            {
                v.steiner.reset();
                v.ladder.reset();
                        v.lastVcoB = 0.0f;
                filterA = filterB = 0.0f;
                v.lastSteinerOsc = 0.0f;
            }

            // ---- Routing ------------------------------------------------
            float voiceLeft = 0.0f;
            float voiceRight = 0.0f;

            if (routing == 0)
            {
                voiceLeft = voiceRight = filterB + filterA * (1.0f - balance) * 0.5f;
            }
            else if (routing == 1)
            {
                voiceLeft = voiceRight = (filterA * (1.0f - balance)
                                          + filterB * balance) * 1.4f;
            }
            else
            {
                voiceLeft = filterA * (1.0f - 0.5f * balance);
                voiceRight = filterB * (0.5f + 0.5f * balance);
            }

            // ---- VCA / pan ---------------------------------------------
            const auto volumeMod = clampf (1.0f + modDest[DestVolume], 0.0f, 2.0f);
            const auto aftertouchGain = 1.0f + 0.15f * aftertouch_;
            const auto amplitude = clampf (env2 * volumeMod * aftertouchGain * 0.8f,
                                           0.0f, 2.0f);
            const auto pan = clampf (v.pan + modDest[DestPan], -1.0f, 1.0f);
            const auto panAngle = (pan + 1.0f) * 0.25f * pbPi;
            const auto gainLeft = std::cos (panAngle) * 1.41421356f;
            const auto gainRight = std::sin (panAngle) * 1.41421356f;

            sumLeft += voiceLeft * amplitude * gainLeft;
            sumRight += voiceRight * amplitude * gainRight;

            // A voice is finished once the VCA envelope and its key are done.
            if (! v.held && ! v.sustained && ! v.env[1].isActive())
            {
                v.active = false;
                        v.lastVcoB = 0.0f;
            }
        }

        sumLeft = std::isfinite (sumLeft) ? clampf (sumLeft, -8.0f, 8.0f) : 0.0f;
        sumRight = std::isfinite (sumRight) ? clampf (sumRight, -8.0f, 8.0f) : 0.0f;

        // ---- Effects -------------------------------------------------
        auto fxLeft = sumLeft;
        auto fxRight = sumRight;

        if (distMix > 0.0001f)
        {
            const auto type = distType == 1 ? DistortionType::Hard
                            : distType == 2 ? DistortionType::Fold
                            : distType == 3 ? DistortionType::Tube
                                           : DistortionType::Soft;
            fxLeft = lerpf (fxLeft, applyDistortion (fxLeft, type, distDrive), distMix);
            fxRight = lerpf (fxRight, applyDistortion (fxRight, type, distDrive), distMix);
        }

        const auto chorusOut = chorus_.process (fxLeft, fxRight);
        const auto delayOut = delay_.process (chorusOut.left, chorusOut.right);
        const auto reverbOut = reverb_.process (delayOut.left, delayOut.right);

        auto finalLeft = dcLeft_.process (reverbOut.left);
        auto finalRight = dcRight_.process (reverbOut.right);

        finalLeft = std::tanh (clampf (finalLeft, -8.0f, 8.0f) * 1.1f) * 0.95f;
        finalRight = std::tanh (clampf (finalRight, -8.0f, 8.0f) * 1.1f) * 0.95f;

        if (! std::isfinite (finalLeft))
            finalLeft = 0.0f;
        if (! std::isfinite (finalRight))
            finalRight = 0.0f;

        peak = std::max (peak, std::max (std::abs (finalLeft), std::abs (finalRight)));
        left[sample] = finalLeft;
        right[sample] = finalRight;
    }

    meter_ = std::max (peak, meter_ * 0.85f);
}

std::unique_ptr<SynthEngine> makePolyBruteEngine()
{
    return std::make_unique<PolyBruteEngine>();
}

} // namespace triad
