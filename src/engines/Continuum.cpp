#include "engines/Continuum.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <string>
#include <utility>
#include <vector>

namespace triad
{

namespace
{
std::vector<std::string> choices (std::initializer_list<const char*> items)
{
    std::vector<std::string> result;
    result.reserve (items.size());
    for (const auto* item : items)
        result.emplace_back (item);
    return result;
}

const char* const kSourceNames[ContinuumEngine::kSources] =
{
    "OSC 1", "OSC 2", "OSC 3", "OSC 4", "OSC 5", "NOISE",
    "FLT 1", "FLT 2", "FLT 3", "FLT 4", "FLT 5",
    "SG 1", "SG 2", "SG 3", "SG 4", "SG 5"
};

const char* const kDestinationNames[ContinuumEngine::kDestinations] =
{
    "FLT 1 IN", "FLT 2 IN", "FLT 3 IN", "FLT 4 IN", "FLT 5 IN",
    "FLT 1 CUT", "FLT 2 CUT", "FLT 3 CUT", "FLT 4 CUT", "FLT 5 CUT",
    "OSC 1 PITCH", "OSC 2 PITCH", "OSC 3 PITCH", "OSC 4 PITCH", "OSC 5 PITCH",
    "MASTER", "MASTER 1", "MASTER 2", "MASTER 3", "MASTER 4", "MASTER 5", "MASTER 6"
};

const char* const kFormulaNames[ContinuumEngine::kFormulaCount] =
{
    "Off", "Direct", "X", "Y", "Z", "X*Y", "X*Z", "Y*Z", "X*Y*Z",
    "Invert", "Soft", "Gate", "User", "A", "B", "C", "D", "E", "F"
};

template <typename T>
T clampValue (T value, T low, T high)
{
    return value < low ? low : (value > high ? high : value);
}

float clamp01 (float value)
{
    return clampValue (value, 0.0f, 1.0f);
}
} // namespace

ContinuumEngine::ContinuumEngine()
{
    buildDefinition();
    values_ = defaults_;

    for (auto& cell : matrixCells_)
        cell.store (0, std::memory_order_relaxed);

    resetMatrix();
}

void ContinuumEngine::buildDefinition()
{
    definition_.id = "continuum";
    definition_.displayName = "CONTINUUM V";
    definition_.manufacturer = "SakuraEntropia";

    auto add = [this] (const char* id, const char* name, const char* group,
                       float min, float max, float def,
                       ParamCurve curve = ParamCurve::Linear,
                       std::vector<std::string> list = {},
                       const char* unit = "")
    {
        ParamSpec spec;
        spec.id = id;
        spec.name = name;
        spec.group = group;
        spec.min = min;
        spec.max = max;
        spec.def = def;
        spec.curve = curve;
        spec.choices = std::move (list);
        spec.unit = unit;
        return definition_.addParam (std::move (spec));
    };

    scalarIndex_[(size_t) PMasterVolume] = add ("con.master.volume", "VOLUME", "Master", 0.0f, 1.0f, 0.7f);
    scalarIndex_[(size_t) PMasterGlide] = add ("con.master.glide", "GLIDE", "Master", 0.001f, 2.0f, 0.02f, ParamCurve::Log, {}, "s");
    scalarIndex_[(size_t) PMasterY] = add ("con.master.yDepth", "Y DEPTH", "Master", 0.0f, 1.0f, 0.6f);
    scalarIndex_[(size_t) PMasterZ] = add ("con.master.zDepth", "Z DEPTH", "Master", 0.0f, 1.0f, 0.8f);
    scalarIndex_[(size_t) PMasterBendRange] = add ("con.master.bendRange", "MPE BEND", "Master", 0.0f, 48.0f, 48.0f, ParamCurve::Linear, {}, "st");
    scalarIndex_[(size_t) PMasterNoise] = add ("con.master.noise", "NOISE LEVEL", "Master", 0.0f, 1.0f, 0.02f);
    scalarIndex_[(size_t) PMasterReverbMix] = add ("con.master.reverbMix", "REVERB MIX", "Master", 0.0f, 1.0f, 0.22f);
    scalarIndex_[(size_t) PMasterReverbSize] = add ("con.master.reverbSize", "REVERB SIZE", "Master", 0.0f, 1.0f, 0.62f);
    scalarIndex_[(size_t) PMasterDelayMix] = add ("con.master.delayMix", "DELAY MIX", "Master", 0.0f, 1.0f, 0.15f);
    scalarIndex_[(size_t) PMasterDelayTime] = add ("con.master.delayTime", "DELAY TIME", "Master", 0.02f, 1.5f, 0.35f, ParamCurve::Log, {}, "s");
    scalarIndex_[(size_t) PMasterDelayFeedback] = add ("con.master.delayFeedback", "DELAY FB", "Master", 0.0f, 0.9f, 0.32f);
    for (int cvc = 0; cvc < 4; ++cvc)
    {
        const auto id = std::string ("con.cvc.") + std::to_string (cvc + 1);
        const auto name = std::string ("CVC ") + std::to_string (cvc + 1) + " OUT";
        scalarIndex_[(size_t) (PCvc1 + cvc)] = add (id.c_str(), name.c_str(), "CVC",
                                                    0.0f, 1.0f, 0.0f);
    }

    scalarIndex_[(size_t) PSurfaceQuantize] = add ("con.surface.quantize", "PITCH SNAP",
                                                  "Surface", 0.0f, 1.0f, 0.0f);

    scalarIndex_[(size_t) PFxMode] = add ("con.fx.mode", "REC MODE", "FX", 0.0f, 5.0f, 0.0f,
                                          ParamCurve::Discrete,
                                          choices ({ "Short Reverb", "Long Reverb", "ModDelay",
                                                     "Analog Echo", "Swept Echo", "Digital Echo LPF" }));
    scalarIndex_[(size_t) PFxReverbDamping] = add ("con.fx.reverb.damping", "REVERB DAMP", "FX", 0.0f, 1.0f, 0.35f);
    scalarIndex_[(size_t) PFxConv1Mix] = add ("con.fx.conv1.mix", "CONV 1 MIX", "FX", 0.0f, 1.0f, 0.0f);
    scalarIndex_[(size_t) PFxDelayLpf] = add ("con.fx.delay.lpf", "DELAY LPF", "FX", 0.0f, 1.0f, 0.7f);
    scalarIndex_[(size_t) PFxDelayOffset] = add ("con.fx.delay.offset", "DELAY OFFSET", "FX", 0.0f, 0.5f, 0.0f);
    scalarIndex_[(size_t) PFxDelayPingPong] = add ("con.fx.delay.pingpong", "DELAY PINGPONG", "FX", 0.0f, 1.0f, 0.0f, ParamCurve::Discrete, choices ({ "Off", "On" }));
    scalarIndex_[(size_t) PFxDelayFeedback] = add ("con.fx.delay.fb", "DELAY FB", "FX", 0.0f, 0.9f, 0.35f);
    scalarIndex_[(size_t) PFxDelayFull] = add ("con.fx.delay.full", "DELAY FULL", "FX", 0.0f, 1.0f, 0.0f, ParamCurve::Discrete, choices ({ "None", "Full" }));
    scalarIndex_[(size_t) PFxDelayTime] = add ("con.fx.delay.time", "DELAY TIME", "FX", 0.01f, 1.5f, 0.3f, ParamCurve::Log, {}, "s");
    scalarIndex_[(size_t) PFxConv2Mix] = add ("con.fx.conv2.mix", "CONV 2 MIX", "FX", 0.0f, 1.0f, 0.0f);

    scalarIndex_[(size_t) PFxChorusRate] = add ("con.fx.chorus.rate", "CHORUS RATE", "FX", 0.05f, 8.0f, 0.35f, ParamCurve::Log, {}, "Hz");
    scalarIndex_[(size_t) PFxChorusDepth] = add ("con.fx.chorus.depth", "CHORUS DEPTH", "FX", 0.0f, 1.0f, 0.3f);
    scalarIndex_[(size_t) PFxChorusMix] = add ("con.fx.chorus.mix", "CHORUS MIX", "FX", 0.0f, 1.0f, 0.0f);
    scalarIndex_[(size_t) PFxPhaserRate] = add ("con.fx.phaser.rate", "PHASER RATE", "FX", 0.05f, 8.0f, 0.25f, ParamCurve::Log, {}, "Hz");
    scalarIndex_[(size_t) PFxPhaserDepth] = add ("con.fx.phaser.depth", "PHASER DEPTH", "FX", 0.0f, 1.0f, 0.6f);
    scalarIndex_[(size_t) PFxPhaserFeedback] = add ("con.fx.phaser.feedback", "PHASER FB", "FX", 0.0f, 0.95f, 0.4f);
    scalarIndex_[(size_t) PFxPhaserMix] = add ("con.fx.phaser.mix", "PHASER MIX", "FX", 0.0f, 1.0f, 0.0f);
    scalarIndex_[(size_t) PFxDelayModDepth] = add ("con.fx.delay.mod", "DELAY MOD", "FX", 0.0f, 1.0f, 0.0f);
    scalarIndex_[(size_t) PFxDelayModRate] = add ("con.fx.delay.rate", "DELAY MOD RATE", "FX", 0.02f, 8.0f, 0.3f, ParamCurve::Log, {}, "Hz");

    scalarIndex_[(size_t) PNoiseLevel] = add ("con.noise.level", "NOISE", "Noise", 0.0f, 1.0f, 0.06f);
    scalarIndex_[(size_t) PNoiseColor] = add ("con.noise.color", "COLOR", "Noise", 0.0f, 1.0f, 0.5f);

    for (int i = 0; i < kOscCount; ++i)
    {
        const auto prefix = std::string ("con.osc") + std::to_string (i + 1) + ".";
        const auto group = "OSC " + std::to_string (i + 1);
        const float defaults[] = { 1.0f, 0.0f, 0.0f, 0.5f, 0.6f };
        const float levelDefaults[] = { 0.65f, 0.35f, 0.12f, 0.2f, 0.15f };
        const float ratios[] = { 1.0f, 2.0f, 0.5f, 1.5f, 3.0f };

        oscIndex_[(size_t) i][(size_t) OscRatio] = add ((prefix + "ratio").c_str(), "RATIO", group.c_str(), 0.25f, 8.0f, ratios[i], ParamCurve::Log);
        oscIndex_[(size_t) i][(size_t) OscFine] = add ((prefix + "fine").c_str(), "FINE", group.c_str(), -100.0f, 100.0f, defaults[1], ParamCurve::Linear, {}, "ct");
        oscIndex_[(size_t) i][(size_t) OscPhase] = add ((prefix + "phase").c_str(), "PHASE", group.c_str(), 0.0f, 1.0f, defaults[2]);
        oscIndex_[(size_t) i][(size_t) OscShape] = add ((prefix + "shape").c_str(), "SHAPE", group.c_str(), 0.0f, 1.0f, defaults[3]);
        oscIndex_[(size_t) i][(size_t) OscLevel] = add ((prefix + "level").c_str(), "LEVEL", group.c_str(), 0.0f, 1.0f, levelDefaults[i]);
        oscIndex_[(size_t) i][(size_t) OscWave] = add ((prefix + "wave").c_str(), "WAVE", group.c_str(), 0.0f, 4.0f, (i == 0 ? 2.0f : 1.0f),
                                                        ParamCurve::Discrete,
                                                        choices ({ "Sine", "Triangle", "Saw", "Square", "Pulse" }));
    }

    for (int i = 0; i < kFilterCount; ++i)
    {
        const auto prefix = std::string ("con.flt") + std::to_string (i + 1) + ".";
        const auto group = "FILTER " + std::to_string (i + 1);
        const float cutoffs[] = { 1200.0f, 1800.0f, 2400.0f, 900.0f, 3000.0f };

        filterIndex_[(size_t) i][(size_t) FltType] = add ((prefix + "type").c_str(), "TYPE", group.c_str(), 0.0f, 3.0f, 0.0f,
                                                         ParamCurve::Discrete,
                                                         choices ({ "LowPass", "HighPass", "BandPass", "Notch" }));
        filterIndex_[(size_t) i][(size_t) FltCutoff] = add ((prefix + "cutoff").c_str(), "CUTOFF", group.c_str(), 20.0f, 20000.0f, cutoffs[i], ParamCurve::Log, {}, "Hz");
        filterIndex_[(size_t) i][(size_t) FltResonance] = add ((prefix + "res").c_str(), "RESONANCE", group.c_str(), 0.0f, 1.0f, 0.2f);
        filterIndex_[(size_t) i][(size_t) FltEnv] = add ((prefix + "env").c_str(), "ENV 2 AMT", group.c_str(), -1.0f, 1.0f, 0.15f);
    }

    for (int i = 0; i < kBankCount; ++i)
    {
        const auto prefix = std::string ("con.bank") + std::to_string (i + 1) + ".";
        const auto group = "BIQUAD BANK " + std::to_string (i + 1);
        bankIndex_[(size_t) i][(size_t) BankFreq] = add ((prefix + "freq").c_str(), "FREQ", group.c_str(), 80.0f, 8000.0f, i == 0 ? 440.0f : 880.0f, ParamCurve::Log, {}, "Hz");
        bankIndex_[(size_t) i][(size_t) BankSpread] = add ((prefix + "spread").c_str(), "SPREAD", group.c_str(), 0.0f, 1.0f, 0.5f);
        bankIndex_[(size_t) i][(size_t) BankBandwidth] = add ((prefix + "bandwidth").c_str(), "BANDWIDTH", group.c_str(), 0.0f, 1.0f, 0.35f);
        bankIndex_[(size_t) i][(size_t) BankMix] = add ((prefix + "mix").c_str(), "MIX", group.c_str(), 0.0f, 1.0f, 0.2f);
        bankIndex_[(size_t) i][(size_t) BankLevel] = add ((prefix + "level").c_str(), "LEVEL", group.c_str(), 0.0f, 1.0f, 0.7f);
        bankIndex_[(size_t) i][(size_t) BankKeyTrack] = add ((prefix + "keytrack").c_str(), "KEY TRACK", group.c_str(), 0.0f, 1.0f, 0.5f);
    }

    for (int i = 0; i < kDelayCount; ++i)
    {
        const auto prefix = std::string ("con.delay") + std::to_string (i + 1) + ".";
        const auto group = "DELAY " + std::to_string (i + 1);
        delayIndex_[(size_t) i][(size_t) DelayTime] = add ((prefix + "time").c_str(), "TIME", group.c_str(), 0.01f, 1.5f, i == 0 ? 0.3f : 0.47f, ParamCurve::Log, {}, "s");
        delayIndex_[(size_t) i][(size_t) DelayFeedback] = add ((prefix + "feedback").c_str(), "FEEDBACK", group.c_str(), 0.0f, 0.9f, 0.3f);
        delayIndex_[(size_t) i][(size_t) DelayMix] = add ((prefix + "mix").c_str(), "MIX", group.c_str(), 0.0f, 1.0f, i == 0 ? 0.18f : 0.12f);
    }

    for (int i = 0; i < kSgCount; ++i)
    {
        const auto prefix = std::string ("con.sg") + std::to_string (i + 1) + ".";
        const auto group = "SHAPE GEN " + std::to_string (i + 1);
        const float rates[] = { 0.18f, 0.31f, 0.53f, 0.83f, 1.27f };
        sgIndex_[(size_t) i][(size_t) SgRate] = add ((prefix + "rate").c_str(), "RATE", group.c_str(), 0.02f, 30.0f, rates[i], ParamCurve::Log, {}, "Hz");
        sgIndex_[(size_t) i][(size_t) SgShape] = add ((prefix + "shape").c_str(), "SHAPE", group.c_str(), 0.0f, 7.0f, (i % 2 == 0 ? 0.0f : 3.0f),
                                                         ParamCurve::Discrete,
                                                         choices ({ "Ramp Up", "Ramp Down", "Pulse", "Triangle", "Hann", "Square", "Sine", "S&H" }));
        sgIndex_[(size_t) i][(size_t) SgLevel] = add ((prefix + "level").c_str(), "LEVEL", group.c_str(), 0.0f, 1.0f, 0.5f);
    }

    for (int i = 0; i < kEnvCount; ++i)
    {
        const auto prefix = std::string ("con.env") + std::to_string (i + 1) + ".";
        const auto group = "ENVELOPE " + std::to_string (i + 1);
        envIndex_[(size_t) i][(size_t) EnvAttack] = add ((prefix + "attack").c_str(), "ATTACK", group.c_str(), 0.001f, 10.0f, i == 0 ? 0.004f : 0.05f, ParamCurve::Log, {}, "s");
        envIndex_[(size_t) i][(size_t) EnvDecay] = add ((prefix + "decay").c_str(), "DECAY", group.c_str(), 0.002f, 10.0f, i == 0 ? 0.55f : 0.8f, ParamCurve::Log, {}, "s");
        envIndex_[(size_t) i][(size_t) EnvSustain] = add ((prefix + "sustain").c_str(), "SUSTAIN", group.c_str(), 0.0f, 1.0f, i == 0 ? 0.75f : 0.35f);
        envIndex_[(size_t) i][(size_t) EnvRelease] = add ((prefix + "release").c_str(), "RELEASE", group.c_str(), 0.002f, 10.0f, i == 0 ? 0.7f : 0.5f, ParamCurve::Log, {}, "s");
    }

    // Append extensions after the original 128 slots. Existing automation and
    // preset parameter ordering, including the four legacy filter types, stay stable.
    scalarIndex_[(size_t) PCompEnabled] = add ("con.fx.comp.enabled", "COMPRESSOR", "Master Processing", 0.0f, 1.0f, 0.0f, ParamCurve::Discrete, choices ({ "Off", "On" }), "");
    scalarIndex_[(size_t) PCompThreshold] = add ("con.fx.comp.threshold", "THRESHOLD", "Master Processing", -60.0f, 0.0f, -18.0f, ParamCurve::Linear, {}, "dB");
    scalarIndex_[(size_t) PCompRatio] = add ("con.fx.comp.ratio", "RATIO", "Master Processing", 1.0f, 20.0f, 4.0f, ParamCurve::Linear, {}, "");
    scalarIndex_[(size_t) PCompAttack] = add ("con.fx.comp.attack", "ATTACK", "Master Processing", 0.0001f, 0.2f, 0.01f, ParamCurve::Log, {}, "s");
    scalarIndex_[(size_t) PCompRelease] = add ("con.fx.comp.release", "RELEASE", "Master Processing", 0.01f, 2.0f, 0.15f, ParamCurve::Log, {}, "s");
    scalarIndex_[(size_t) PCompMakeup] = add ("con.fx.comp.makeup", "MAKEUP", "Master Processing", 0.0f, 24.0f, 0.0f, ParamCurve::Linear, {}, "dB");
    scalarIndex_[(size_t) PTilt] = add ("con.fx.tilt.gain", "TILT", "Master Processing", -12.0f, 12.0f, 0.0f, ParamCurve::Linear, {}, "dB");
    scalarIndex_[(size_t) PTiltFrequency] = add ("con.fx.tilt.frequency", "TILT PIVOT", "Master Processing", 20.0f, 20000.0f, 1000.0f, ParamCurve::Log, {}, "Hz");
    scalarIndex_[(size_t) PPreGain] = add ("con.fx.preGain", "PRE GAIN", "Master Processing", -24.0f, 24.0f, 0.0f, ParamCurve::Linear, {}, "dB");
    scalarIndex_[(size_t) PPostGain] = add ("con.fx.postGain", "POST GAIN", "Master Processing", -24.0f, 24.0f, 0.0f, ParamCurve::Linear, {}, "dB");
    for (int i = 0; i < 2; ++i)
    {
        const auto prefix = "con.fx.conv" + std::to_string (i + 1) + ".";
        irIndices_[(size_t) i][0] = add ((prefix + "model").c_str(), "IR MODEL", "Convolution", 0, 3, 0, ParamCurve::Discrete,
            choices ({ "Original", "Damped Wood", "Metal", "Early Room" }));
        irIndices_[(size_t) i][1] = add ((prefix + "length").c_str(), "IR LENGTH", "Convolution", .05f, 1, 1);
        irIndices_[(size_t) i][2] = add ((prefix + "tuning").c_str(), "IR TUNING", "Convolution", .5f, 2, 1);
        irIndices_[(size_t) i][3] = add ((prefix + "width").c_str(), "IR WIDTH", "Convolution", 0, 2, 1);
        const auto bank = "con.bank" + std::to_string (i + 1) + ".";
        bankIndex_[(size_t) i][BankSize] = add ((bank + "size").c_str(), "FILTER COUNT", "Biquad", 0, 1, 0, ParamCurve::Discrete, choices ({ "8", "48" }));
        bankIndex_[(size_t) i][BankBWSpread] = add ((bank + "bwSpread").c_str(), "BW SPREAD", "Biquad", -1, 1, 0);
        bankIndex_[(size_t) i][BankCentre] = add ((bank + "centre").c_str(), "SPECTRAL CENTRE", "Biquad", 0, 1, .5f);
        bankIndex_[(size_t) i][BankWeight] = add ((bank + "weight").c_str(), "SPECTRAL WEIGHT", "Biquad", 0, 1, 0);
    }
    for (int i = 0; i < 5; ++i)
    {
        const auto f = "con.flt" + std::to_string (i + 1) + ".";
        filterIndex_[(size_t) i][FltModel] = add ((f + "model").c_str(), "EXTENDED TYPE", "Filter", 0, 6, 0, ParamCurve::Discrete,
            choices ({ "Legacy Type", "Low Shelf", "High Shelf", "Unity Bandpass", "Allpass", "1-Pole LP", "1-Pole HP" }));
        filterIndex_[(size_t) i][FltStages] = add ((f + "stages").c_str(), "CASCADE", "Filter", 0, 3, 0, ParamCurve::Discrete, choices ({ "1", "2", "3", "4" }));
        const auto g = "con.sg" + std::to_string (i + 1) + ".";
        sgIndex_[(size_t) i][SgCycle] = add ((g + "cycle").c_str(), "CYCLE", "Shape Generator", 0, 3, 0, ParamCurve::Discrete,
            choices ({ "Free", "Retrigger", "One Shot", "Gate" }));
        sgIndex_[(size_t) i][SgTrigger] = add ((g + "trigger").c_str(), "TRIGGER", "Shape Generator", 0, 1, 0, ParamCurve::Discrete, choices ({ "Low", "High" }));
    }
    for (int i = 0; i < 6; ++i)
    {
        const auto f = "con.formula" + std::to_string (i + 1) + ".";
        formulaIndices_[(size_t) i][0] = add ((f + "curve").c_str(), "TRANSFER", "Formula", 0, 7, 0, ParamCurve::Discrete,
            choices ({ "Linear", "Smooth S", "Steep S", "Square", "Square Root", "Absolute", "Bipolar", "Steps" }));
        formulaIndices_[(size_t) i][1] = add ((f + "offset").c_str(), "OFFSET", "Formula", -1, 1, 0);
        formulaIndices_[(size_t) i][2] = add ((f + "scale").c_str(), "SCALE", "Formula", -2, 2, 1);
        const auto m = "con.mastercol" + std::to_string (i + 1) + ".";
        masterIndices_[(size_t) i][0] = add ((m + "target").c_str(), "MASTER TARGET", "Master Columns", 0, 20, 0, ParamCurve::Discrete,
            choices ({ "None", "Volume", "Reverb Mix", "Reverb Size", "Reverb Damping", "Delay Mix", "Delay Time", "Delay Feedback", "Delay LPF", "Chorus Mix", "Phaser Mix", "IR 1 Mix", "IR 2 Mix", "Delay Mod", "Delay Rate", "Pre Gain", "Post Gain", "Tilt", "Tilt Pivot", "Comp Threshold", "Comp Ratio" }));
        masterIndices_[(size_t) i][1] = add ((m + "depth").c_str(), "MASTER DEPTH", "Master Columns", -1, 1, 1);
    }

    defaults_.resize (definition_.params.size());
    for (size_t i = 0; i < defaults_.size(); ++i)
        defaults_[i] = definition_.params[i].def;

    setUserFormula ("sin(pi*x)*y");
    for (int i = 1; i <= 6; ++i) setFormulaText (i, "s*z");
}

void ContinuumEngine::prepare (double sampleRate, int maxBlockSize)
{
    sampleRate_ = sampleRate;
    maxBlock_ = maxBlockSize;
    noise_.prepare (sampleRate, 0x6d2b79f5u);
    delay_.prepare (sampleRate, 1.6f);
    delayBank1_.prepare (sampleRate, 1.6f);
    delayBank2_.prepare (sampleRate, 1.6f);
    convPre_.prepare (sampleRate, 17);
    convPost_.prepare (sampleRate, 29);
    chorus_.prepare (sampleRate);
    phaser_.prepare (sampleRate);
    reverb_.prepare (sampleRate);
    masterProcessor_.prepare (sampleRate);

    for (auto& voice : voices_)
    {
        for (auto& osc : voice.oscillators)
            osc.prepare (sampleRate);

        for (auto& filter : voice.filters)
            filter.prepare (sampleRate);

        for (auto& cascade : voice.extraFilters)
            for (auto& filter : cascade) filter.prepare (sampleRate);

        for (auto& band : voice.bank1)
            band.prepare (sampleRate);

        for (auto& band : voice.bank2)
            band.prepare (sampleRate);

        for (auto& env : voice.envelopes)
            env.prepare (sampleRate);
    }

    volumeSmoother_.prepare (sampleRate, 25.0f);
    volumeSmoother_.snap (param (PMasterVolume));
    dcLeft_.prepare (sampleRate);
    dcRight_.prepare (sampleRate);
    reset();
}

void ContinuumEngine::reset()
{
    for (auto& voice : voices_)
    {
        voice.active = false;
        voice.held = false;
        voice.note = -1;
        voice.currentHz = 440.0f;
        voice.targetHz = 440.0f;

        for (auto& osc : voice.oscillators)
            osc.reset();

        for (auto& filter : voice.filters)
            filter.reset();

        for (auto& cascade : voice.extraFilters)
            for (auto& filter : cascade) filter.reset();
        for (auto& gen : voice.shapes) { gen.phase = 0; gen.finished = false; gen.triggerHigh = false; }
        for (auto& band : voice.bank1)
            band.reset();

        for (auto& band : voice.bank2)
            band.reset();

        for (auto& env : voice.envelopes)
            env.reset();

        voice.lastFilter.fill (0.0f);
    }

    noise_.reset();
    delay_.reset();
    delayBank1_.reset();
    delayBank2_.reset();
    convPre_.reset();
    convPost_.reset();
    chorus_.reset();
    phaser_.reset();
    reverb_.reset();
    masterProcessor_.reset();
    dcLeft_.reset();
    dcRight_.reset();
    meter_ = 0.0f;
}

void ContinuumEngine::setParameters (const float* values, int count)
{
    if (values == nullptr)
        return;

    const auto n = std::min (count, (int) values_.size());
    for (int i = 0; i < n; ++i)
        values_[(size_t) i] = values[(size_t) i];

    volumeSmoother_.setTarget (param (PMasterVolume));
}

void ContinuumEngine::processMidi (const MidiEvent& event)
{
    switch (event.type)
    {
        case MidiEventType::NoteOn:
        {
            const auto channel = clampValue (event.channel, 0, 15);
            for (auto& voice : voices_)
                if (voice.active && voice.held && voice.note == event.note)
                    releaseVoice (voice);

            auto index = allocateVoice (event.note, channel);
            if (index >= 0)
            {
                auto& voice = voices_[(size_t) index];
                startVoice (voice, event.note, channel, clamp01 (event.value));
            }
            break;
        }

        case MidiEventType::NoteOff:
            // The surface uses one fixed carrier note.  Release every voice
            // with that note, including release tails, so a later touch cannot
            // inherit the previous touch's pitch.
            for (auto& voice : voices_)
                if (voice.active && voice.note == event.note)
                    releaseVoice (voice);
            break;

        case MidiEventType::AllNotesOff:
            for (auto& voice : voices_)
                releaseVoice (voice);
            break;

        case MidiEventType::AllSoundOff:
            for (auto& voice : voices_)
            {
                voice.active = false;
                voice.held = false;
                voice.note = -1;
                for (auto& env : voice.envelopes)
                    env.reset();
            }
            break;

        case MidiEventType::PitchBend:
            channelBend_[(size_t) clampValue (event.channel, 0, 15)] = clampValue (event.value, -1.0f, 1.0f);
            break;

        case MidiEventType::ChannelPressure:
        {
            const auto channel = clampValue (event.channel, 0, 15);
            const auto value = clamp01 (event.value);
            channelPressure_[(size_t) channel] = value;
            for (auto& voice : voices_)
                if (voice.active && voice.channel == channel)
                    voice.pressure = value;
            break;
        }

        case MidiEventType::NoteExpression:
            updateVoicesForNote (event.note, [&] (Voice& voice)
            {
                if (event.controller == 0)
                    voice.exprPitch = clampValue (event.value, -48.0f, 48.0f);
                else if (event.controller == 1)
                    voice.timbre = clamp01 (event.value);
                else if (event.controller == 2)
                    voice.pressure = clamp01 (event.value);
            });
            break;

        case MidiEventType::ControlChange:
            if (event.controller == 74)
            {
                const auto channel = clampValue (event.channel, 0, 15);
                const auto value = clamp01 (event.value);
                channelTimbre_[(size_t) channel] = value;
                for (auto& voice : voices_)
                    if (voice.active && voice.channel == channel)
                        voice.timbre = value;
            }
            break;

        case MidiEventType::ModWheel:
            channelTimbre_[(size_t) clampValue (event.channel, 0, 15)] = clamp01 (event.value);
            break;

        default:
            break;
    }
}

int ContinuumEngine::allocateVoice (int note, int channel)
{
    (void) note; (void) channel;

    for (int i = 0; i < kVoices; ++i)
        if (! voices_[(size_t) i].active)
            return i;

    int oldest = 0;
    for (int i = 1; i < kVoices; ++i)
        if (voices_[(size_t) i].age < voices_[(size_t) oldest].age)
            oldest = i;

    return oldest;
}

ContinuumEngine::Voice* ContinuumEngine::findVoice (int note)
{
    for (auto& voice : voices_)
        if (voice.active && voice.note == note)
            return &voice;

    return nullptr;
}

void ContinuumEngine::startVoice (Voice& voice, int note, int channel, float velocity)
{
    voice.active = true;
    voice.held = true;
    voice.note = note;
    voice.channel = channel;
    voice.age = (int) ++ageCounter_;
    voice.velocity = velocity;
    voice.exprPitch = 0.0f;
    voice.bend = channelBend_[(size_t) channel] * param (PMasterBendRange);
    voice.timbre = channelTimbre_[(size_t) channel] > 0.0f ? channelTimbre_[(size_t) channel] : 0.5f;
    voice.pressure = channelPressure_[(size_t) channel] > 0.0f ? channelPressure_[(size_t) channel] : velocity;

    voice.targetHz = 440.0f * std::pow (2.0f, (float (note) + voice.bend - 69.0f) / 12.0f);
    voice.currentHz = voice.targetHz;

    for (int i = 0; i < kOscCount; ++i)
    {
        voice.oscillators[(size_t) i].reset ((double) ovalue (i, OscPhase));
        voice.oscillators[(size_t) i].setFrequency (voice.currentHz * ovalue (i, OscRatio));
    }

    for (int i = 0; i < kEnvCount; ++i)
    {
        voice.envelopes[(size_t) i].setParameters (evalue (i, EnvAttack), evalue (i, EnvDecay),
                                                   evalue (i, EnvSustain), evalue (i, EnvRelease));
        voice.envelopes[(size_t) i].noteOn();
    }

    voice.lastFilter.fill (0.0f);
    for (int i = 0; i < kSgCount; ++i)
        if (svalue (i, SgCycle) > .5f)
        {
            auto& gen = voice.shapes[(size_t) i];
            gen.phase = 0; gen.finished = false;
        }

}

void ContinuumEngine::releaseVoice (Voice& voice)
{
    if (! voice.active)
        return;

    voice.held = false;
    for (auto& env : voice.envelopes)
        env.noteOff();
}

template <typename Fn>
void ContinuumEngine::updateVoicesForNote (int note, Fn&& fn)
{
    for (auto& voice : voices_)
        if (voice.active && voice.note == note)
            fn (voice);
}

void ContinuumEngine::setNoteExpression (int note, float pitchSemitones,
                                         float timbre01, float pressure01)
{
    updateVoicesForNote (note, [&] (Voice& voice)
    {
        voice.exprPitch = clampValue (pitchSemitones, -48.0f, 48.0f);
        voice.timbre = clamp01 (timbre01);
        voice.pressure = clamp01 (pressure01);
    });
}

int ContinuumEngine::activeVoiceCount() const
{
    int count = 0;
    for (const auto& voice : voices_)
        if (voice.active)
            ++count;
    return count;
}

std::string ContinuumEngine::matrixSourceName (int source) const
{
    return source >= 0 && source < kSources ? kSourceNames[source] : std::string();
}

std::string ContinuumEngine::matrixDestinationName (int destination) const
{
    return destination >= 0 && destination < kDestinations
               ? kDestinationNames[destination] : std::string();
}

std::string ContinuumEngine::matrixFormulaName (int formula) const
{
    return formula >= 0 && formula < kFormulaCount ? kFormulaNames[formula] : std::string();
}

uint32_t ContinuumEngine::packCell (int formula, float amount)
{
    const auto f = (uint32_t) clampValue (formula, 0, kFormulaCount - 1);
    const auto a = (uint32_t) clampValue ((int) std::lround (clamp01 (amount) * 65535.0f), 0, 65535);
    return f | (a << 8);
}

void ContinuumEngine::setMatrixCell (int source, int destination, int formula, float amount)
{
    if (source < 0 || source >= kSources || destination < 0 || destination >= kDestinations)
        return;

    matrixCells_[(size_t) (source * kDestinations + destination)]
        .store (packCell (formula, amount), std::memory_order_relaxed);
    matrixDirty_.store (true, std::memory_order_release);
}

int ContinuumEngine::matrixCellFormula (int source, int destination) const
{
    if (source < 0 || source >= kSources || destination < 0 || destination >= kDestinations)
        return 0;

    return cellFormula (matrixCells_[(size_t) (source * kDestinations + destination)]
                            .load (std::memory_order_relaxed));
}

float ContinuumEngine::matrixCellAmount (int source, int destination) const
{
    if (source < 0 || source >= kSources || destination < 0 || destination >= kDestinations)
        return 0.0f;

    return cellAmount (matrixCells_[(size_t) (source * kDestinations + destination)]
                           .load (std::memory_order_relaxed));
}

void ContinuumEngine::resetMatrix()
{
    for (auto& cell : matrixCells_)
        cell.store (0, std::memory_order_relaxed);

    auto set = [this] (int source, int destination, int formula, float amount)
    {
        matrixCells_[(size_t) (source * kDestinations + destination)]
            .store (packCell (formula, amount), std::memory_order_relaxed);
    };

    for (int i = 0; i < kOscCount; ++i)
        set (i, i, 1, 1.0f);

    for (int i = 0; i < kFilterCount; ++i)
        set (6 + i, 15, 1, 0.22f);

    for (int i = 0; i < kSgCount; ++i)
        set (11 + i, 5 + i, 4, 0.12f);

    matrixDirty_.store (true, std::memory_order_release);
}

void ContinuumEngine::refreshMatrix()
{
    if (! matrixDirty_.exchange (false, std::memory_order_acquire))
        return;

    for (size_t i = 0; i < matrixCells_.size(); ++i)
    {
        const auto packed = matrixCells_[i].load (std::memory_order_relaxed);
        matrixFormula_[i] = (uint8_t) cellFormula (packed);
        matrixAmount_[i] = cellAmount (packed);
    }
}


namespace
{
struct FormulaToken
{
    enum Type { Number, Variable, Function, Operator, LParen, RParen } type = Number;
    float number = 0.0f;
    int code = 0;
    int precedence = 0;
    bool rightAssociative = false;
};

std::vector<FormulaToken> tokenizeFormula (const std::string& text, bool& ok)
{
    std::vector<FormulaToken> tokens;
    ok = true;
    const auto* p = text.c_str();

    auto skipSpace = [&p]
    {
        while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
            ++p;
    };

    while (true)
    {
        skipSpace();
        if (*p == '\0')
            break;

        if ((*p >= '0' && *p <= '9') || *p == '.')
        {
            char* end = nullptr;
            const auto value = std::strtof (p, &end);
            if (end == p)
            {
                ok = false;
                return {};
            }
            FormulaToken token;
            token.type = FormulaToken::Number;
            token.number = value;
            tokens.push_back (token);
            p = end;
            continue;
        }

        if ((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z'))
        {
            std::string name;
            while ((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z')
                   || (*p >= '0' && *p <= '9') || *p == '_')
            {
                name.push_back ((char) std::tolower ((unsigned char) *p));
                ++p;
            }

            FormulaToken token;
            if (name == "x" || name == "y" || name == "z" || name == "s")
            {
                token.type = FormulaToken::Variable;
                token.code = name == "s" ? 1 : (name == "x" ? 2 : (name == "y" ? 3 : 4));
            }
            else if (name == "pi")
            {
                token.type = FormulaToken::Number;
                token.number = 3.14159265358979323846f;
            }
            else
            {
                static const std::pair<const char*, int> functions[] = {
                    { "sin", 1 }, { "cos", 2 }, { "tan", 3 }, { "abs", 4 },
                    { "sqrt", 5 }, { "exp", 6 }, { "log", 7 }
                };

                int code = 0;
                for (const auto& fn : functions)
                    if (name == fn.first)
                        code = fn.second;

                if (code == 0)
                {
                    ok = false;
                    return {};
                }

                token.type = FormulaToken::Function;
                token.code = code;
            }
            tokens.push_back (token);
            continue;
        }

        if (*p == '(')
        {
            FormulaToken token;
            token.type = FormulaToken::LParen;
            tokens.push_back (token);
            ++p;
            continue;
        }

        if (*p == ')')
        {
            FormulaToken token;
            token.type = FormulaToken::RParen;
            tokens.push_back (token);
            ++p;
            continue;
        }

        if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '^')
        {
            FormulaToken token;
            token.type = FormulaToken::Operator;
            const auto op = *p++;

            // Unary minus / plus.
            const auto previousIsOperand = ! tokens.empty()
                && (tokens.back().type == FormulaToken::Number
                    || tokens.back().type == FormulaToken::Variable
                    || tokens.back().type == FormulaToken::RParen);

            if (op == '-' && ! previousIsOperand)
            {
                token.code = 10; // Neg
                token.precedence = 5;
                token.rightAssociative = true;
            }
            else if (op == '+')
            {
                if (! previousIsOperand)
                    continue;
                token.code = 1; // Add
                token.precedence = 2;
            }
            else if (op == '-')
            {
                token.code = 2; // Sub
                token.precedence = 2;
            }
            else if (op == '*')
            {
                token.code = 3;
                token.precedence = 3;
            }
            else if (op == '/')
            {
                token.code = 4;
                token.precedence = 3;
            }
            else
            {
                token.code = 5; // Pow
                token.precedence = 4;
                token.rightAssociative = true;
            }

            tokens.push_back (token);
            continue;
        }

        ok = false;
        return {};
    }

    return tokens;
}

bool compileUserFormula (const std::string& text, ContinuumEngine::UserFormula& formula)
{
    formula = {};
    bool ok = false;
    const auto tokens = tokenizeFormula (text, ok);
    if (! ok || tokens.empty())
        return false;

    std::vector<FormulaToken> output;
    std::vector<FormulaToken> stack;

    auto popOperator = [&output, &stack]
    {
        if (stack.empty() || stack.back().type == FormulaToken::LParen)
            return false;
        output.push_back (stack.back());
        stack.pop_back();
        return true;
    };

    for (const auto& token : tokens)
    {
        if (token.type == FormulaToken::Number || token.type == FormulaToken::Variable)
        {
            output.push_back (token);
        }
        else if (token.type == FormulaToken::Function)
        {
            stack.push_back (token);
        }
        else if (token.type == FormulaToken::Operator)
        {
            while (! stack.empty() && stack.back().type == FormulaToken::Operator
                   && (stack.back().precedence > token.precedence
                       || (stack.back().precedence == token.precedence
                           && ! token.rightAssociative)))
                popOperator();
            stack.push_back (token);
        }
        else if (token.type == FormulaToken::LParen)
        {
            stack.push_back (token);
        }
        else if (token.type == FormulaToken::RParen)
        {
            while (! stack.empty() && stack.back().type != FormulaToken::LParen)
                popOperator();

            if (stack.empty())
                return false;
            stack.pop_back();

            if (! stack.empty() && stack.back().type == FormulaToken::Function)
                popOperator();
        }
    }

    while (! stack.empty())
    {
        if (stack.back().type == FormulaToken::LParen)
            return false;
        popOperator();
    }

    int constCount = 0;

    for (const auto& token : output)
    {
        if (formula.opCount >= (int) formula.ops.size())
            return false;

        if (token.type == FormulaToken::Number)
        {
            if (constCount >= (int) formula.constants.size())
                return false;
            formula.ops[(size_t) formula.opCount++] = (uint8_t) ContinuumEngine::UserFormula::Const;
            formula.constants[(size_t) constCount++] = token.number;
        }
        else if (token.type == FormulaToken::Variable)
        {
            const auto op = token.code == 1 ? ContinuumEngine::UserFormula::Source
                          : token.code == 2 ? ContinuumEngine::UserFormula::X
                          : token.code == 3 ? ContinuumEngine::UserFormula::Y
                                            : ContinuumEngine::UserFormula::Z;
            formula.ops[(size_t) formula.opCount++] = (uint8_t) op;
        }
        else if (token.type == FormulaToken::Function)
        {
            const auto op = token.code == 1 ? ContinuumEngine::UserFormula::Sin
                          : token.code == 2 ? ContinuumEngine::UserFormula::Cos
                          : token.code == 3 ? ContinuumEngine::UserFormula::Tan
                          : token.code == 4 ? ContinuumEngine::UserFormula::Abs
                          : token.code == 5 ? ContinuumEngine::UserFormula::Sqrt
                          : token.code == 6 ? ContinuumEngine::UserFormula::Exp
                          : token.code == 7 ? ContinuumEngine::UserFormula::Log
                          : token.code == 8 ? ContinuumEngine::UserFormula::Min
                                            : ContinuumEngine::UserFormula::Max;
            formula.ops[(size_t) formula.opCount++] = (uint8_t) op;
        }
        else
        {
            const auto op = token.code == 1 ? ContinuumEngine::UserFormula::Add
                          : token.code == 2 ? ContinuumEngine::UserFormula::Sub
                          : token.code == 3 ? ContinuumEngine::UserFormula::Mul
                          : token.code == 4 ? ContinuumEngine::UserFormula::Div
                          : token.code == 5 ? ContinuumEngine::UserFormula::Pow
                                            : ContinuumEngine::UserFormula::Neg;
            formula.ops[(size_t) formula.opCount++] = (uint8_t) op;
        }
    }

    formula.valid = formula.opCount > 0;
    return formula.valid;
}
} // namespace

float ContinuumEngine::UserFormula::eval (float source, float x, float y, float z) const
{
    if (! valid)
        return 0.0f;

    float stack[32] {};
    int sp = 0;
    int ci = 0;

    auto pop = [&stack, &sp] { return sp > 0 ? stack[--sp] : 0.0f; };

    for (int i = 0; i < opCount; ++i)
    {
        const auto op = (Op) ops[(size_t) i];
        switch (op)
        {
            case Const: if (ci < (int) constants.size() && sp < 32) stack[sp++] = constants[(size_t) ci++]; break;
            case Source: if (sp < 32) stack[sp++] = source; break;
            case X: if (sp < 32) stack[sp++] = x; break;
            case Y: if (sp < 32) stack[sp++] = y; break;
            case Z: if (sp < 32) stack[sp++] = z; break;
            case Neg: { const auto a = pop(); if (sp < 32) stack[sp++] = -a; break; }
            case Sin: { const auto a = pop(); if (sp < 32) stack[sp++] = std::sin (a); break; }
            case Cos: { const auto a = pop(); if (sp < 32) stack[sp++] = std::cos (a); break; }
            case Tan: { const auto a = pop(); if (sp < 32) stack[sp++] = std::tan (a); break; }
            case Abs: { const auto a = pop(); if (sp < 32) stack[sp++] = std::abs (a); break; }
            case Sqrt: { const auto a = pop(); if (sp < 32) stack[sp++] = std::sqrt (std::max (0.0f, a)); break; }
            case Exp: { const auto a = pop(); if (sp < 32) stack[sp++] = std::exp (clampValue (a, -8.0f, 8.0f)); break; }
            case Log: { const auto a = pop(); if (sp < 32) stack[sp++] = std::log (std::max (1.0e-6f, a)); break; }
            case Add: { const auto b = pop(); const auto a = pop(); if (sp < 32) stack[sp++] = a + b; break; }
            case Sub: { const auto b = pop(); const auto a = pop(); if (sp < 32) stack[sp++] = a - b; break; }
            case Mul: { const auto b = pop(); const auto a = pop(); if (sp < 32) stack[sp++] = a * b; break; }
            case Div: { const auto b = pop(); const auto a = pop(); if (sp < 32) stack[sp++] = std::abs (b) > 1.0e-8f ? a / b : 0.0f; break; }
            case Pow: { const auto b = pop(); const auto a = pop(); if (sp < 32) stack[sp++] = std::pow (std::abs (a), b); break; }
            case Min: { const auto b = pop(); const auto a = pop(); if (sp < 32) stack[sp++] = std::min (a, b); break; }
            case Max: { const auto b = pop(); const auto a = pop(); if (sp < 32) stack[sp++] = std::max (a, b); break; }
        }
    }

    return sp > 0 ? stack[sp - 1] : 0.0f;
}

void ContinuumEngine::setUserFormula (const std::string& text) { setFormulaText (0, text); }

bool ContinuumEngine::setFormulaText (int slot, const std::string& text)
{
    if (slot < 0 || slot > 6 || text.size() > 1024) return false;
    UserFormula parsed;
    if (! compileUserFormula (text, parsed)) return false;
    const std::lock_guard<std::mutex> lock (formulaMutex_);
    pendingFormulas_[(size_t) slot] = parsed;
    formulaTexts_[(size_t) slot] = text;
    return true;
}

std::string ContinuumEngine::formulaText (int slot) const
{
    if (slot < 0 || slot > 6) return {};
    const std::lock_guard<std::mutex> lock (formulaMutex_);
    return formulaTexts_[(size_t) slot];
}

float ContinuumEngine::transfer (float input, int curve)
{
    const float sign = input < 0 ? -1.0f : 1.0f;
    const float v = std::min (1.0f, std::abs (input));
    switch (curve)
    {
        case 1: return sign * v * v * (3 - 2 * v);
        case 2: return std::tanh (input * 3.0f) / std::tanh (3.0f);
        case 3: return sign * v * v;
        case 4: return sign * std::sqrt (v);
        case 5: return std::abs (input);
        case 6: return 2 * input - 1;
        case 7: return std::round (input * 7) / 7;
        default: return input;
    }
}

float ContinuumEngine::evaluateNamedFormula (int slot, float source, float x, float y, float z)
{
    if (slot < 0 || slot >= 6) return 0;
    const auto& indices = formulaIndices_[(size_t) slot];
    float value = renderFormulas_[(size_t) slot + 1].eval (source, x, y, z);
    value = transfer (value, (int) std::lround (values_[(size_t) indices[0]]));
    value = value * values_[(size_t) indices[2]] + values_[(size_t) indices[1]];
    return std::isfinite (value) ? clampValue (value, -16.0f, 16.0f) : 0;
}

float ContinuumEngine::evalFormula (int formula, float source, float x, float y, float z)
{
    switch (formula)
    {
        case 1:  return source;
        case 2:  return source * x;
        case 3:  return source * y;
        case 4:  return source * z;
        case 5:  return source * x * y;
        case 6:  return source * x * z;
        case 7:  return source * y * z;
        case 8:  return source * x * y * z;
        case 9:  return -source;
        case 10: return source * (0.2f + 0.8f * z);
        case 11: return source > 0.5f ? 1.0f : 0.0f;
        case 12: return renderFormulas_[0].eval (source, x, y, z);
        case 13: case 14: case 15: case 16: case 17: case 18:
            return evaluateNamedFormula (formula - 13, source, x, y, z);
        default: return 0.0f;
    }
}

void ContinuumEngine::render (float* left, float* right, int numSamples)
{
    if (left == nullptr || right == nullptr || numSamples <= 0)
        return;

    refreshMatrix();
    // Compile and allocate on the control thread. Audio takes a nonblocking snapshot.
    if (formulaMutex_.try_lock())
    {
        renderFormulas_ = pendingFormulas_;
        formulaMutex_.unlock();
    }
    for (int i = 0; i < 2; ++i)
    {
        const auto& p = irIndices_[(size_t) i];
        auto& conv = i == 0 ? convPre_ : convPost_;
        conv.setControls ((int) values_[(size_t) p[0]], values_[(size_t) p[1]], values_[(size_t) p[2]], values_[(size_t) p[3]]);
    }

    const auto glide = std::max (0.001f, param (PMasterGlide));
    const auto glideCoef = 1.0f - std::exp (-1.0f / (float) (sampleRate_ * glide));
    const auto bendRange = param (PMasterBendRange);
    const auto yDepth = param (PMasterY);
    const auto zDepth = param (PMasterZ);
    const auto noiseLevel = param (PNoiseLevel);
    const auto noiseColor = param (PNoiseColor);
    const auto masterGain = volumeSmoother_.current();

    const auto fxMode = clampValue ((int) std::lround (param (PFxMode)), 0, 5);
    auto delayMix = param (PMasterDelayMix);
    auto reverbMix = param (PMasterReverbMix);
    auto chorusMix = param (PFxChorusMix);
    auto phaserMix = param (PFxPhaserMix);

    if (fxMode <= 1)
    {
        delayMix *= 0.15f;
        reverbMix = std::max (reverbMix, fxMode == 0 ? 0.35f : 0.55f);
    }
    else
    {
        delayMix = std::max (delayMix, 0.35f);
        reverbMix *= 0.45f;
        if (fxMode == 2) { chorusMix = std::max (chorusMix, 0.25f); }
        if (fxMode == 3 || fxMode == 5) { delay_.setDampingHz (fxMode == 5 ? 2600.0f : 4200.0f); }
    }

    const auto delayTime = param (PFxDelayTime);
    const auto delayOffset = param (PFxDelayOffset);
    delayBank1_.setDelaySeconds (delayTime, delayTime * (1.0f + delayOffset));
    delayBank1_.setFeedback (param (PFxDelayFeedback));
    delayBank1_.setPingPong (param (PFxDelayPingPong) > 0.5f);
    delayBank1_.setDampingHz (600.0f + param (PFxDelayLpf) * 11000.0f);
    delayBank1_.setMix (param (PFxDelayFull) > 0.5f ? 0.65f : dvalue (0, DelayMix));
    delayBank2_.setDelaySeconds (dvalue (1, DelayTime), dvalue (1, DelayTime) * 1.23f);
    delayBank2_.setFeedback (dvalue (1, DelayFeedback));
    delayBank2_.setMix (dvalue (1, DelayMix));
    convPre_.setMix (param (PFxConv1Mix));
    convPost_.setMix (param (PFxConv2Mix));

    delay_.setDelaySeconds (param (PMasterDelayTime), param (PMasterDelayTime) * 1.18f);
    delay_.setFeedback (param (PMasterDelayFeedback));
    delay_.setMix (delayMix);
    delay_.setModulation (param (PFxDelayModDepth), param (PFxDelayModRate));
    chorus_.setRateHz (param (PFxChorusRate));
    chorus_.setDepth (param (PFxChorusDepth));
    chorus_.setMix (chorusMix);
    phaser_.setRateHz (param (PFxPhaserRate));
    phaser_.setDepth (param (PFxPhaserDepth));
    phaser_.setFeedback (param (PFxPhaserFeedback));
    phaser_.setMix (phaserMix);
    reverb_.setSize (param (PMasterReverbSize));
    reverb_.setDamping (param (PFxReverbDamping));
    reverb_.setMix (reverbMix);

    for (int env = 0; env < kEnvCount; ++env)
        for (auto& voice : voices_)
            voice.envelopes[(size_t) env].setParameters (evalue (env, EnvAttack), evalue (env, EnvDecay),
                                                         evalue (env, EnvSustain), evalue (env, EnvRelease));

    const auto filterCount = kFilterCount;
    const auto sourceCount = kSources;

    std::array<float, 20> masterOffsets {};
    const ScalarParam targets[] = { PMasterVolume, PMasterReverbMix, PMasterReverbSize,
        PFxReverbDamping, PMasterDelayMix, PMasterDelayTime, PMasterDelayFeedback,
        PFxDelayLpf, PFxChorusMix, PFxPhaserMix, PFxConv1Mix, PFxConv2Mix,
        PFxDelayModDepth, PFxDelayModRate, PPreGain, PPostGain, PTilt, PTiltFrequency,
        PCompThreshold, PCompRatio };
    const auto controlled = [&] (int target)
    {
        const auto index = scalarIndex_[(size_t) targets[target]];
        const auto& spec = definition_.params[(size_t) index];
        const float base = values_[(size_t) index];
        const float offset = clampValue (masterOffsets[(size_t) target], -1.0f, 1.0f);
        return spec.curve == ParamCurve::Log
            ? clampValue (base * std::pow (spec.max / spec.min, offset), spec.min, spec.max)
            : clampValue (base + offset * (spec.max - spec.min), spec.min, spec.max);
    };
    float peak = 0.0f;
    float preGain = 1, postGain = 1;

    for (int sample = 0; sample < numSamples; ++sample)
    {
        const auto noiseWhite = noise_.white();
        const auto noisePink = noise_.pink();
        const auto noiseSample = (noiseWhite * (1.0f - noiseColor) + noisePink * noiseColor)
                                 * noiseLevel;

        float mixLeft = 0.0f;
        float mixRight = 0.0f;
        std::array<float, 20> nextOffsets {};
        int contributingVoices = 0;

        for (int vi = 0; vi < kVoices; ++vi)
        {
            auto& voice = voices_[(size_t) vi];
            if (! voice.active)
                continue;

            const auto ampEnv = voice.envelopes[0].process();
            const auto modEnv = voice.envelopes[1].process();

            if (! voice.envelopes[0].isActive())
            {
                voice.active = false;
                voice.held = false;
                voice.note = -1;
                continue;
            }

            voice.bend = channelBend_[(size_t) clampValue (voice.channel, 0, 15)] * bendRange;
            const auto pitchSemis = voice.bend + voice.exprPitch;
            voice.targetHz = 440.0f * std::pow (2.0f, (float (voice.note) + pitchSemis - 69.0f) / 12.0f);
            voice.currentHz += (voice.targetHz - voice.currentHz) * glideCoef;

            const auto x = clamp01 (0.5f + voice.exprPitch / 48.0f);
            const auto y = clamp01 (voice.timbre * yDepth + 0.5f * (1.0f - yDepth));
            const auto z = clamp01 (voice.pressure * zDepth + 0.5f * (1.0f - zDepth));

            std::array<float, kSgCount> sg {};
            for (int i = 0; i < kSgCount; ++i)
            {
                auto& gen = voice.shapes[(size_t) i];
                const auto rate = std::max (0.01f, svalue (i, SgRate));
                const int cycle = (int) svalue (i, SgCycle);
                const bool trigger = svalue (i, SgTrigger) > .5f;
                if (trigger && ! gen.triggerHigh) { gen.phase = 0; gen.finished = false; }
                gen.triggerHigh = trigger;
                if (! gen.finished && (cycle != 3 || voice.held)) gen.phase += (double) rate / sampleRate_;
                if (gen.phase >= 1.0 && cycle == 2) { gen.phase = 1; gen.finished = true; }
                else if (gen.phase >= 1.0)
                {
                    gen.phase -= 1.0;
                    gen.sampleHold = gen.rng.nextBipolar();
                }

                const auto shape = clampValue ((int) std::lround (svalue (i, SgShape)), 0, 7);
                float value = 0.0f;
                switch (shape)
                {
                    case 0: value = (float) gen.phase; break;
                    case 1: value = 1.0f - (float) gen.phase; break;
                    case 2: value = gen.phase < 0.5 ? 1.0f : 0.0f; break;
                    case 3: value = 1.0f - 4.0f * std::abs ((float) gen.phase - 0.5f); break;
                    case 4: value = 0.5f - 0.5f * std::cos (float (gen.phase) * kTwoPi); break;
                    case 5: value = gen.phase < 0.5 ? 1.0f : -1.0f; break;
                    case 6: value = std::sin (float (gen.phase) * kTwoPi) * 0.5f + 0.5f; break;
                    case 7: value = gen.sampleHold; break;
                    default: break;
                }

                gen.last = value;
                sg[(size_t) i] = value * svalue (i, SgLevel);
            }

            std::array<float, kSources> sourceAudio {};
            std::array<float, kFilterCount> filterOut {};
            std::array<float, kFilterCount> pitchMod {};
            std::array<float, kFilterCount> cutoffMod {};

            for (int i = 0; i < sourceCount; ++i)
            {
                if (i >= 11)
                    sourceAudio[(size_t) i] = sg[(size_t) (i - 11)];
                else if (i < 5)
                    sourceAudio[(size_t) i] = 0.0f;
                else if (i == 5)
                    sourceAudio[(size_t) i] = noiseSample;
                else
                    sourceAudio[(size_t) i] = voice.lastFilter[(size_t) (i - 6)];
            }

            for (int f = 0; f < filterCount; ++f)
            {
                for (int s = 11; s < 16; ++s)
                {
                    const auto cell = (size_t) (s * kDestinations + 5 + f);
                    const auto formula = matrixFormula_[cell];
                    if (formula != 0)
                    {
                        const auto mod = evalFormula (formula, sourceAudio[(size_t) s], x, y, z)
                                         * matrixAmount_[cell];
                        cutoffMod[(size_t) f] += mod;
                    }
                }

            }

            for (int o = 0; o < kOscCount; ++o)
            {
                for (int s = 11; s < 16; ++s)
                {
                    const auto cell = (size_t) (s * kDestinations + 10 + o);
                    const auto formula = matrixFormula_[cell];
                    if (formula != 0)
                        pitchMod[(size_t) o] += evalFormula (formula, sourceAudio[(size_t) s], x, y, z)
                                                * matrixAmount_[cell];
                }
            }

            for (int o = 0; o < kOscCount; ++o)
            {
                const auto ratio = ovalue (o, OscRatio);
                const auto fine = ovalue (o, OscFine) / 1200.0f;
                const auto semis = pitchMod[(size_t) o] * 24.0f;
                const auto hz = clampValue (voice.currentHz * ratio
                                                * std::pow (2.0f, fine + semis / 12.0f),
                                            20.0f, 20000.0f);
                auto& osc = voice.oscillators[(size_t) o];
                osc.setFrequency (hz);
                osc.advance();
                const auto waveIndex = clampValue ((int) std::lround (ovalue (o, OscWave)), 0, 4);
                const auto wave = static_cast<Oscillator::Wave> (waveIndex);
                const auto shape = clampValue (ovalue (o, OscShape), 0.05f, 0.95f);
                sourceAudio[(size_t) o] = osc.sample (wave, shape) * ovalue (o, OscLevel);
            }

            for (int f = 0; f < filterCount; ++f)
            {
                float input = 0.0f;

                for (int s = 0; s < sourceCount; ++s)
                {
                    const auto cell = (size_t) (s * kDestinations + f);
                    const auto formula = matrixFormula_[cell];
                    if (formula == 0)
                        continue;

                    input += evalFormula (formula, sourceAudio[(size_t) s], x, y, z)
                             * matrixAmount_[cell];
                }

                const auto baseCutoff = fvalue (f, FltCutoff);
                const auto envAmt = fvalue (f, FltEnv);
                const auto cutoff = clampValue (baseCutoff
                                                    * std::pow (2.0f, cutoffMod[(size_t) f] * 4.0f
                                                                         + modEnv * envAmt * 4.0f),
                                                20.0f, 20000.0f);
                auto& filter = voice.filters[(size_t) f];
                filter.setCutoffHz (cutoff);
                filter.setResonance01 (clampValue (fvalue (f, FltResonance), 0.0f, 0.98f));
                const int extended = clampValue ((int) fvalue (f, FltModel), 0, 6);
                const auto mode = static_cast<StateVariableFilter::Mode> (extended == 0
                    ? clampValue ((int) std::lround (fvalue (f, FltType)), 0, 3) : extended + 3);
                float output = filter.process (input, mode);
                const int stages = clampValue ((int) fvalue (f, FltStages), 0, 3);
                for (int stage = 0; stage < stages; ++stage)
                {
                    auto& extra = voice.extraFilters[(size_t) f][(size_t) stage];
                    extra.setCutoffHz (cutoff);
                    extra.setResonance01 (clampValue (fvalue (f, FltResonance), 0.0f, .98f));
                    output = extra.process (output, mode);
                }
                filterOut[(size_t) f] = output;
            }

            // Two modal/biquad banks on filter outputs 1 and 2.
            for (int b = 0; b < kBankCount; ++b)
            {
                auto& bank = b == 0 ? voice.bank1 : voice.bank2;
                const auto freq = bvalue (b, BankFreq);
                const auto keyTrack = bvalue (b, BankKeyTrack);
                const auto keyScale = std::pow (2.0f, (float) (voice.note - 60) / 12.0f * keyTrack);
                const auto spread = bvalue (b, BankSpread);
                const auto bandwidth = bvalue (b, BankBandwidth);
                const auto mix = bvalue (b, BankMix);
                const auto level = bvalue (b, BankLevel);
                const auto dry = filterOut[(size_t) b];
                float wet = 0.0f;

                const int count = bvalue (b, BankSize) > .5f ? 48 : 8;
                for (int band = 0; band < count; ++band)
                {
                    const float position = (float) band / (count - 1);
                    const auto ratio = count == 8 ? 0.5f + 0.5f * (float) band : 0.5f + 3.5f * position;
                    const auto bandFreq = freq * keyScale * std::pow (2.0f, spread * (ratio - 1.0f) * 3.0f);
                    auto& filter = bank[(size_t) band];
                    filter.setCutoffHz (clampValue (bandFreq, 20.0f, 20000.0f));
                    filter.setResonance01 (clampValue (0.9f - clamp01 (bandwidth + bvalue (b, BankBWSpread) * (position - .5f)) * 0.6f, 0.0f, 0.95f));
                    const float distance = position - bvalue (b, BankCentre);
                    const float weight = std::exp (-bvalue (b, BankWeight) * 20.0f * distance * distance);
                    wet += filter.process (dry, StateVariableFilter::Mode::BandPass) * weight;
                }

                wet *= level * 0.22f * (8.0f / count);
                filterOut[(size_t) b] = dry * (1.0f - mix) + wet * mix;
            }

            for (int f = 0; f < filterCount; ++f)
                voice.lastFilter[(size_t) f] = filterOut[(size_t) f];

            for (int f = 0; f < filterCount; ++f)
                sourceAudio[(size_t) (6 + f)] = filterOut[(size_t) f];

            float master = 0.0f;
            for (int s = 0; s < sourceCount; ++s)
            {
                const auto cell = (size_t) (s * kDestinations + 15);
                const auto formula = matrixFormula_[cell];
                if (formula != 0)
                    master += evalFormula (formula, sourceAudio[(size_t) s], x, y, z)
                              * matrixAmount_[cell];
            }

            ++contributingVoices;
            for (int column = 0; column < 6; ++column)
            {
                const auto& p = masterIndices_[(size_t) column];
                const int target = (int) values_[(size_t) p[0]] - 1;
                if (target < 0 || target >= 20) continue;
                for (int source = 0; source < sourceCount; ++source)
                {
                    const size_t cell = (size_t) (source * kDestinations + 16 + column);
                    if (matrixFormula_[cell] != 0)
                        nextOffsets[(size_t) target] += evalFormula (matrixFormula_[cell], sourceAudio[(size_t) source], x, y, z)
                            * matrixAmount_[cell] * values_[(size_t) p[1]];
                }
            }

            const auto amp = ampEnv * (0.15f + 0.85f * z);
            const auto pan = clampValue ((x - 0.5f) * 1.2f, -1.0f, 1.0f);
            const auto signal = master * amp;
            mixLeft += signal * (1.0f - pan) * 0.7f;
            mixRight += signal * (1.0f + pan) * 0.7f;
        }

        for (int i = 0; i < 20; ++i)
            masterOffsets[(size_t) i] = nextOffsets[(size_t) i] / std::max (1, contributingVoices);
        // Master assignment controls update at 1/32 audio rate; oscillator/filter
        // routes retain their existing per-sample behavior.
        if (sample % 32 == 0)
        {
            const float dt = controlled (5);
            delay_.setDelaySeconds (dt, dt * 1.18f);
            delay_.setFeedback (controlled (6));
            float dm = controlled (4), rm = controlled (1), cm = controlled (8);
            if (fxMode <= 1) { dm *= .15f; rm = std::max (rm, fxMode == 0 ? .35f : .55f); }
            else { dm = std::max (dm, .35f); rm *= .45f; if (fxMode == 2) cm = std::max (cm, .25f); }
            delay_.setMix (dm);
            delay_.setModulation (controlled (12), controlled (13));
            delayBank1_.setDampingHz (600 + controlled (7) * 11000);
            chorus_.setMix (cm); phaser_.setMix (controlled (9));
            reverb_.setMix (rm); reverb_.setSize (controlled (2)); reverb_.setDamping (controlled (3));
            convPre_.setMix (controlled (10)); convPost_.setMix (controlled (11));
            preGain = std::pow (10.0f, controlled (14) / 20);
            postGain = std::pow (10.0f, controlled (15) / 20);
            masterProcessor_.configure (param (PCompEnabled) > .5f, controlled (18), controlled (19),
                param (PCompAttack), param (PCompRelease), param (PCompMakeup), controlled (16), controlled (17));
        }
        auto stereo = masterProcessor_.processCompressor (mixLeft * preGain, mixRight * preGain);
        stereo = convPre_.process (stereo.left, stereo.right);
        stereo = delay_.process (stereo.left, stereo.right);
        stereo = delayBank1_.process (stereo.left, stereo.right);
        stereo = delayBank2_.process (stereo.left, stereo.right);
        stereo = chorus_.process (stereo.left, stereo.right);
        stereo = phaser_.process (stereo.left, stereo.right);
        stereo = reverb_.process (stereo.left, stereo.right);
        stereo = convPost_.process (stereo.left, stereo.right);

        stereo = masterProcessor_.processTilt (stereo.left, stereo.right);
        const auto gain = clampValue (volumeSmoother_.next() + masterOffsets[0], 0.0f, 1.0f) * postGain;
        const auto outLeft = dcLeft_.process (softClip (stereo.left * gain));
        const auto outRight = dcRight_.process (softClip (stereo.right * gain));

        left[sample] = outLeft;
        right[sample] = outRight;
        peak = std::max (peak, std::max (std::abs (outLeft), std::abs (outRight)));
    }

    lastGain_ = masterGain;
    meter_ = peak;
}

std::unique_ptr<SynthEngine> makeContinuumEngine()
{
    return std::unique_ptr<SynthEngine> (new ContinuumEngine());
}

} // namespace triad
