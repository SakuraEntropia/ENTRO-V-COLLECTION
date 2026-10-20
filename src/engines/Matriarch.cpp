#include "engines/Matriarch.h"

#include <algorithm>
#include <bit>
#include <cmath>
#include <cstring>

namespace triad
{

namespace
{
constexpr float kJackSignal = 1.0f;

std::vector<std::string> choices (std::initializer_list<const char*> items)
{
    std::vector<std::string> result;
    result.reserve (items.size());
    for (const auto* item : items)
        result.emplace_back (item);
    return result;
}
} // namespace

MatriarchEngine::MatriarchEngine()
{
    buildDefinition();
    cacheIndices();
    defaults_.resize (definition_.params.size());

    for (size_t i = 0; i < definition_.params.size(); ++i)
        defaults_[i] = definition_.params[i].def;

    values_ = defaults_;

    for (auto& mask : cableLow_)
        mask.store (0, std::memory_order_relaxed);
    for (auto& mask : cableHigh_)
        mask.store (0, std::memory_order_relaxed);
    cableSource_.fill (-1);
    bus_.fill (0.0f);
}

void MatriarchEngine::buildDefinition()
{
    definition_.id = "matriarch";
    definition_.displayName = "Matriarch";
    definition_.manufacturer = "SakuraEntropia";

    auto addParam = [this] (const char* id, const char* name, const char* group,
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
        definition_.addParam (std::move (spec));
    };

    addParam ("mat.master.volume", "MAIN VOLUME", "Master", 0.0f, 1.0f, 0.75f);
    addParam ("mat.master.vcaMode", "VCA MODE", "Master", 0.0f, 2.0f, 0.0f,
              ParamCurve::Discrete, choices ({ "Amp Env", "Split", "Drone" }));
    addParam ("mat.master.paraphony", "PARAPHONY", "Master", 0.0f, 2.0f, 2.0f,
              ParamCurve::Discrete, choices ({ "1-Note", "2-Note", "4-Note" }));
    addParam ("mat.output.velocity", "VELOCITY", "Master", 0.0f, 1.0f, 0.5f);
    addParam ("mat.output.aftertouch", "AFTERTOUCH", "Master", 0.0f, 1.0f, 0.3f);
    addParam ("mat.global.fine", "FINE TUNE", "Master", -50.0f, 50.0f, 0.0f,
              ParamCurve::Linear, {}, "cents");

    for (int i = 1; i <= 4; ++i)
    {
        const auto prefix = "mat.vco" + std::to_string (i);
        const auto label = "VCO " + std::to_string (i);
        addParam ((prefix + ".octave").c_str(), "OCTAVE", "Oscillators", 0.0f, 3.0f, 1.0f,
                  ParamCurve::Discrete, choices ({ "16'", "8'", "4'", "2'" }));
        if (i > 1)
        {
            addParam ((prefix + ".freq").c_str(), "FREQUENCY", "Oscillators", -7.0f, 7.0f, 0.0f,
                      ParamCurve::Linear, {}, "st");
            addParam ((prefix + ".sync").c_str(), "SYNC", "Oscillators", 0.0f, 1.0f, 0.0f,
                      ParamCurve::Discrete, choices ({ "Off", "On" }));
        }
        addParam ((prefix + ".wave").c_str(), "WAVEFORM", "Oscillators", 0.0f, 3.0f, 1.0f,
                  ParamCurve::Discrete, choices ({ "Triangle", "Saw", "Square", "Narrow Pulse" }));
    }

    addParam ("mat.vco.syncEnable", "SYNC ENABLE", "Oscillators", 0.0f, 1.0f, 0.0f,
              ParamCurve::Discrete, choices ({ "Off", "On" }));

    addParam ("mat.mix.osc1", "OSCILLATOR 1", "Mixer", 0.0f, 1.0f, 0.8f);
    addParam ("mat.mix.osc2", "OSCILLATOR 2", "Mixer", 0.0f, 1.0f, 0.8f);
    addParam ("mat.mix.osc3", "OSCILLATOR 3", "Mixer", 0.0f, 1.0f, 0.0f);
    addParam ("mat.mix.osc4", "OSCILLATOR 4", "Mixer", 0.0f, 1.0f, 0.0f);
    addParam ("mat.mix.noise", "NOISE", "Mixer", 0.0f, 1.0f, 0.0f);

    addParam ("mat.flt.mode", "FILTER MODE", "Filters", 0.0f, 2.0f, 2.0f,
              ParamCurve::Discrete, choices ({ "HP/LP Series", "HP/LP Parallel", "LP/LP Stereo" }));
    addParam ("mat.flt.cutoff", "CUTOFF", "Filters", 20.0f, 20000.0f, 900.0f, ParamCurve::Log, {}, "Hz");
    addParam ("mat.flt.spacing", "SPACING", "Filters", -1.0f, 1.0f, 0.0f);
    addParam ("mat.flt.res1", "RESONANCE 1", "Filters", 0.0f, 1.0f, 0.2f);
    addParam ("mat.flt.res2", "RESONANCE 2", "Filters", 0.0f, 1.0f, 0.2f);
    addParam ("mat.flt.envAmt", "ENVELOPE AMT", "Filters", -1.0f, 1.0f, 0.25f);
    addParam ("mat.flt.kbTrack", "KB TRACKING", "Filters", 0.0f, 1.0f, 0.3f);

    addParam ("mat.env1.attack", "ATTACK", "Envelopes", 0.002f, 10.0f, 0.004f, ParamCurve::Log, {}, "s");
    addParam ("mat.env1.decay", "DECAY", "Envelopes", 0.002f, 10.0f, 0.35f, ParamCurve::Log, {}, "s");
    addParam ("mat.env1.sustain", "SUSTAIN", "Envelopes", 0.0f, 1.0f, 0.8f);
    addParam ("mat.env1.release", "RELEASE", "Envelopes", 0.002f, 10.0f, 0.35f, ParamCurve::Log, {}, "s");
    addParam ("mat.env2.attack", "ATTACK", "Envelopes", 0.002f, 10.0f, 0.01f, ParamCurve::Log, {}, "s");
    addParam ("mat.env2.decay", "DECAY", "Envelopes", 0.002f, 10.0f, 0.4f, ParamCurve::Log, {}, "s");
    addParam ("mat.env2.sustain", "SUSTAIN", "Envelopes", 0.0f, 1.0f, 0.25f);
    addParam ("mat.env2.release", "RELEASE", "Envelopes", 0.002f, 10.0f, 0.3f, ParamCurve::Log, {}, "s");

    addParam ("mat.lfo.rate", "RATE", "Modulation", 0.07f, 1300.0f, 3.0f, ParamCurve::Log, {}, "Hz");
    addParam ("mat.lfo.wave", "WAVEFORM", "Modulation", 0.0f, 5.0f, 0.0f,
              ParamCurve::Discrete, choices ({ "Sine", "Saw", "Ramp", "Square", "Staircase", "Smooth Random" }));
    addParam ("mat.lfo.pitchAmt", "PITCH AMT", "Modulation", 0.0f, 1.0f, 0.0f);
    addParam ("mat.lfo.pitchAssign", "PITCH MOD ASSIGN", "Modulation", 0.0f, 2.0f, 1.0f,
              ParamCurve::Discrete, choices ({ "1&3", "All", "2&4" }));
    addParam ("mat.lfo.cutoffAmt", "CUTOFF AMT", "Modulation", 0.0f, 1.0f, 0.0f);
    addParam ("mat.lfo.pwAmt", "PULSE WIDTH AMT", "Modulation", 0.0f, 1.0f, 0.0f);

    addParam ("mat.lfo2.rate", "LFO RATE", "Utilities", 0.05f, 100.0f, 0.5f, ParamCurve::Log, {}, "Hz");
    addParam ("mat.lfo2.bipolar", "BIPOLAR", "Utilities", 0.0f, 1.0f, 1.0f,
              ParamCurve::Discrete, choices ({ "Off", "On" }));

    addParam ("mat.att1", "ATTENUATOR 1", "Utilities", -1.0f, 1.0f, 1.0f);
    addParam ("mat.att2", "ATTENUATOR 2", "Utilities", -1.0f, 1.0f, 1.0f);
    addParam ("mat.att3", "ATTENUATOR 3", "Utilities", -1.0f, 1.0f, 1.0f);

    addParam ("mat.delay.time", "TIME", "Stereo Delay", 0.035f, 0.78f, 0.3f, ParamCurve::Log, {}, "s");
    addParam ("mat.delay.spacing", "SPACING", "Stereo Delay", -1.0f, 1.0f, 0.0f);
    addParam ("mat.delay.feedback", "FEEDBACK", "Stereo Delay", 0.0f, 1.0f, 0.3f);
    addParam ("mat.delay.mix", "MIX", "Stereo Delay", 0.0f, 1.0f, 0.2f);
    addParam ("mat.delay.sync", "SYNC", "Stereo Delay", 0.0f, 1.0f, 1.0f,
              ParamCurve::Discrete, choices ({ "Off", "On" }));
    addParam ("mat.delay.pingPong", "PING PONG", "Stereo Delay", 0.0f, 1.0f, 0.0f,
              ParamCurve::Discrete, choices ({ "Off", "On" }));

    addParam ("mat.arp.enabled", "ARP / SEQ", "Arpeggiator", 0.0f, 1.0f, 0.0f,
              ParamCurve::Discrete, choices ({ "Off", "On" }));
    addParam ("mat.arp.mode", "MODE", "Arpeggiator", 0.0f, 4.0f, 0.0f,
              ParamCurve::Discrete, choices ({ "Up", "Down", "Up/Down", "Random", "Order" }));
    addParam ("mat.arp.rate", "RATE", "Arpeggiator", 0.0f, 5.0f, 3.0f,
              ParamCurve::Discrete, choices ({ "1/4", "1/8", "1/8T", "1/16", "1/16T", "1/32" }));
    addParam ("mat.arp.octaves", "OCTAVES", "Arpeggiator", 1.0f, 4.0f, 1.0f,
              ParamCurve::Discrete, choices ({ "1", "2", "3", "4" }));
    addParam ("mat.arp.hold", "HOLD", "Arpeggiator", 0.0f, 1.0f, 0.0f,
              ParamCurve::Discrete, choices ({ "Off", "On" }));

    auto addJack = [this] (const char* id, const char* name, const char* group,
                           bool isOutput, bool audio = false)
    {
        PatchPointSpec spec;
        spec.id = id;
        spec.name = name;
        spec.group = group;
        spec.isOutput = isOutput;
        spec.audioRate = audio;
        definition_.addPatch (std::move (spec));
    };

    for (int i = 1; i <= 4; ++i)
    {
        const auto prefix = "mat.patch.vco" + std::to_string (i);
        const auto group = "VCO " + std::to_string (i);
        addJack ((prefix + ".pitch").c_str(), "PITCH IN", group.c_str(), false);
        addJack ((prefix + ".fm").c_str(), "LIN FM IN", group.c_str(), false, true);
        addJack ((prefix + ".pwm").c_str(), "PWM IN", group.c_str(), false);
        addJack ((prefix + ".out").c_str(), "WAVE OUT", group.c_str(), true, true);
    }

    addJack ("mat.patch.mix.noiseIn", "NOISE IN", "Mixer", false, true);
    addJack ("mat.patch.mix.osc1In", "OSC 1 IN", "Mixer", false, true);
    addJack ("mat.patch.mix.osc2In", "OSC 2 IN", "Mixer", false, true);
    addJack ("mat.patch.mix.osc3In", "OSC 3 IN", "Mixer", false, true);
    addJack ("mat.patch.mix.osc4In", "OSC 4 IN", "Mixer", false, true);
    addJack ("mat.patch.mix.out", "MIXER OUT", "Mixer", true, true);

    addJack ("mat.patch.flt1.in", "VCF 1 IN", "Filters", false, true);
    addJack ("mat.patch.flt2.in", "VCF 2 IN", "Filters", false, true);
    addJack ("mat.patch.flt1.cutoff", "CUTOFF 1 IN", "Filters", false);
    addJack ("mat.patch.flt2.cutoff", "CUTOFF 2 IN", "Filters", false);
    addJack ("mat.patch.flt.envAmt", "ENV AMT IN", "Filters", false);
    addJack ("mat.patch.flt1.out", "VCF 1 OUT", "Filters", true, true);
    addJack ("mat.patch.flt2.out", "VCF 2 OUT", "Filters", true, true);

    addJack ("mat.patch.amp.trig", "TRIGGER IN", "Amplitude EG", false);
    addJack ("mat.patch.amp.out", "ENV OUT", "Amplitude EG", true);
    addJack ("mat.patch.amp.end", "ENV END OUT", "Amplitude EG", true);
    addJack ("mat.patch.fltEnv.trig", "TRIGGER IN", "Filter EG", false);
    addJack ("mat.patch.fltEnv.out", "ENV OUT", "Filter EG", true);
    addJack ("mat.patch.fltEnv.end", "ENV END OUT", "Filter EG", true);

    addJack ("mat.patch.vca1.in", "VCA 1 IN", "Output", false, true);
    addJack ("mat.patch.vca2.in", "VCA 2 IN", "Output", false, true);
    addJack ("mat.patch.vca1.cv", "VCA 1 CV IN", "Output", false);
    addJack ("mat.patch.vca2.cv", "VCA 2 CV IN", "Output", false);

    addJack ("mat.patch.delay.in1", "INPUT 1", "Stereo Delay", false, true);
    addJack ("mat.patch.delay.in2", "INPUT 2", "Stereo Delay", false, true);
    addJack ("mat.patch.delay.fbCv", "FB CV IN", "Stereo Delay", false);
    addJack ("mat.patch.delay.mixCv", "MIX IN", "Stereo Delay", false);
    addJack ("mat.patch.delay.time1Cv", "TIME 1 IN", "Stereo Delay", false);
    addJack ("mat.patch.delay.time2Cv", "TIME 2 IN", "Stereo Delay", false);
    addJack ("mat.patch.delay.out1", "DELAY 1 OUT", "Stereo Delay", true, true);
    addJack ("mat.patch.delay.out2", "DELAY 2 OUT", "Stereo Delay", true, true);

    addJack ("mat.patch.lfo.rateIn", "RATE IN", "Modulation", false);
    addJack ("mat.patch.lfo.syncIn", "SYNC IN", "Modulation", false);
    addJack ("mat.patch.lfo.out", "SIGNAL OUT", "Modulation", true);
    addJack ("mat.patch.noise.out", "NOISE OUT", "Modulation", true, true);
    addJack ("mat.patch.sh.out", "S/H OUT", "Modulation", true);

    addJack ("mat.patch.mult1.a", "MULT 1", "Utilities", true);
    addJack ("mat.patch.mult1.b", "MULT 1", "Utilities", true);
    addJack ("mat.patch.mult1.c", "MULT 1", "Utilities", true);
    addJack ("mat.patch.mult1.d", "MULT 1", "Utilities", true);
    addJack ("mat.patch.mult2.a", "MULT 2", "Utilities", true);
    addJack ("mat.patch.mult2.b", "MULT 2", "Utilities", true);
    addJack ("mat.patch.mult2.c", "MULT 2", "Utilities", true);
    addJack ("mat.patch.mult2.d", "MULT 2", "Utilities", true);

    addJack ("mat.patch.att1.in", "ATTENUATOR 1 IN", "Utilities", false);
    addJack ("mat.patch.att1.out", "ATTENUATOR 1 OUT", "Utilities", true);
    addJack ("mat.patch.att2.in", "ATTENUATOR 2 IN", "Utilities", false);
    addJack ("mat.patch.att2.out", "ATTENUATOR 2 OUT", "Utilities", true);
    addJack ("mat.patch.att3.in", "ATTENUATOR 3 IN", "Utilities", false);
    addJack ("mat.patch.att3.out", "ATTENUATOR 3 OUT", "Utilities", true);

    addJack ("mat.patch.lfo2.rateIn", "LFO RATE IN", "Utilities", false);
    addJack ("mat.patch.lfo2.out", "LFO OUT", "Utilities", true);

    addJack ("mat.patch.clock.in", "CLOCK IN", "Arp / Seq", false);
    addJack ("mat.patch.clock.out", "CLOCK OUT", "Arp / Seq", true);
    addJack ("mat.patch.clock.step1", "STEP 1", "Arp / Seq", true);
    addJack ("mat.patch.clock.step2", "STEP 2", "Arp / Seq", true);
    addJack ("mat.patch.clock.resetIn", "RESET IN", "Arp / Seq", false);
    addJack ("mat.patch.clock.resetOut", "RESET OUT", "Arp / Seq", true);

    addJack ("mat.patch.kb.pitch", "PITCH OUT", "Keyboard", true);
    addJack ("mat.patch.kb.gate", "GATE OUT", "Keyboard", true);
    addJack ("mat.patch.kb.velocity", "VELOCITY OUT", "Keyboard", true);
    addJack ("mat.patch.kb.aftertouch", "AFTERTOUCH OUT", "Keyboard", true);
    addJack ("mat.patch.kb.mod", "MOD WHEEL OUT", "Keyboard", true);
}

void MatriarchEngine::cacheIndices()
{
    static const char* paramIds[PParamCount] = {
        "mat.master.volume", "mat.master.vcaMode", "mat.master.paraphony",
        "mat.output.velocity", "mat.output.aftertouch", "mat.global.fine",
        "mat.vco1.octave", "mat.vco1.wave",
        "mat.vco2.octave", "mat.vco2.freq", "mat.vco2.wave", "mat.vco2.sync",
        "mat.vco3.octave", "mat.vco3.freq", "mat.vco3.wave", "mat.vco3.sync",
        "mat.vco4.octave", "mat.vco4.freq", "mat.vco4.wave", "mat.vco4.sync",
        "mat.vco.syncEnable",
        "mat.mix.osc1", "mat.mix.osc2", "mat.mix.osc3", "mat.mix.osc4", "mat.mix.noise",
        "mat.flt.mode", "mat.flt.cutoff", "mat.flt.spacing", "mat.flt.res1", "mat.flt.res2",
        "mat.flt.envAmt", "mat.flt.kbTrack",
        "mat.env1.attack", "mat.env1.decay", "mat.env1.sustain", "mat.env1.release",
        "mat.env2.attack", "mat.env2.decay", "mat.env2.sustain", "mat.env2.release",
        "mat.lfo.rate", "mat.lfo.wave", "mat.lfo.pitchAmt", "mat.lfo.pitchAssign",
        "mat.lfo.cutoffAmt", "mat.lfo.pwAmt",
        "mat.lfo2.rate", "mat.lfo2.bipolar",
        "mat.att1", "mat.att2", "mat.att3",
        "mat.delay.time", "mat.delay.spacing", "mat.delay.feedback", "mat.delay.mix",
        "mat.delay.sync", "mat.delay.pingPong"
    };

    static const char* jackIds[JJackCount] = {
        "mat.patch.vco1.pitch", "mat.patch.vco1.fm", "mat.patch.vco1.pwm", "mat.patch.vco1.out",
        "mat.patch.vco2.pitch", "mat.patch.vco2.fm", "mat.patch.vco2.pwm", "mat.patch.vco2.out",
        "mat.patch.vco3.pitch", "mat.patch.vco3.fm", "mat.patch.vco3.pwm", "mat.patch.vco3.out",
        "mat.patch.vco4.pitch", "mat.patch.vco4.fm", "mat.patch.vco4.pwm", "mat.patch.vco4.out",
        "mat.patch.mix.noiseIn", "mat.patch.mix.osc1In", "mat.patch.mix.osc2In",
        "mat.patch.mix.osc3In", "mat.patch.mix.osc4In", "mat.patch.mix.out",
        "mat.patch.flt1.in", "mat.patch.flt2.in", "mat.patch.flt1.cutoff", "mat.patch.flt2.cutoff",
        "mat.patch.flt.envAmt", "mat.patch.flt1.out", "mat.patch.flt2.out",
        "mat.patch.amp.trig", "mat.patch.amp.out", "mat.patch.amp.end",
        "mat.patch.fltEnv.trig", "mat.patch.fltEnv.out", "mat.patch.fltEnv.end",
        "mat.patch.vca1.in", "mat.patch.vca2.in", "mat.patch.vca1.cv", "mat.patch.vca2.cv",
        "mat.patch.delay.in1", "mat.patch.delay.in2", "mat.patch.delay.fbCv",
        "mat.patch.delay.mixCv", "mat.patch.delay.time1Cv", "mat.patch.delay.time2Cv",
        "mat.patch.delay.out1", "mat.patch.delay.out2",
        "mat.patch.lfo.rateIn", "mat.patch.lfo.syncIn", "mat.patch.lfo.out",
        "mat.patch.noise.out", "mat.patch.sh.out",
        "mat.patch.mult1.a", "mat.patch.mult1.b", "mat.patch.mult1.c", "mat.patch.mult1.d",
        "mat.patch.mult2.a", "mat.patch.mult2.b", "mat.patch.mult2.c", "mat.patch.mult2.d",
        "mat.patch.att1.in", "mat.patch.att1.out", "mat.patch.att2.in", "mat.patch.att2.out",
        "mat.patch.att3.in", "mat.patch.att3.out",
        "mat.patch.lfo2.rateIn", "mat.patch.lfo2.out",
        "mat.patch.clock.in", "mat.patch.clock.out", "mat.patch.clock.step1",
        "mat.patch.clock.step2", "mat.patch.clock.resetIn", "mat.patch.clock.resetOut",
        "mat.patch.kb.pitch", "mat.patch.kb.gate", "mat.patch.kb.velocity",
        "mat.patch.kb.aftertouch", "mat.patch.kb.mod"
    };

    for (size_t i = 0; i < (size_t) PParamCount; ++i)
        paramIndex_[i] = definition_.indexOf (paramIds[i]);

    for (size_t i = 0; i < (size_t) JJackCount; ++i)
        jackIndex_[i] = definition_.patchIndexOf (jackIds[i]);
}

void MatriarchEngine::prepare (double sampleRate, int maxBlockSize)
{
    sampleRate_ = std::max (8000.0, sampleRate);
    maxBlock_ = std::max (1, maxBlockSize);

    for (auto& osc : oscillators_)
        osc.prepare (sampleRate_);

    ladder1_.prepare (sampleRate_);
    ladder2_.prepare (sampleRate_);
    for (auto& filter : hp1_)
        filter.prepare (sampleRate_);

    ampEnvelope_.prepare (sampleRate_);
    filterEnvelope_.prepare (sampleRate_);
    lfo_.prepare (sampleRate_);
    lfo2_.prepare (sampleRate_);
    noise_.prepare (sampleRate_);
    delay_.prepare (sampleRate_, 2.0f);
    delay_.setMix (1.0f);

    masterSmoother_.prepare (sampleRate_, 20.0f);
    cutoffSmoother_.prepare (sampleRate_, 8.0f);
    dcLeft_.prepare (sampleRate_);
    dcRight_.prepare (sampleRate_);

    reset();
}

void MatriarchEngine::reset()
{
    heldCount_ = 0;
    sustainedCount_ = 0;
    sustainPedal_ = false;
    pitchBend_ = 0.0f;
    modWheel_ = 0.0f;
    aftertouch_ = 0.0f;
    lastVelocity_ = 0.8f;
    currentNote_ = 60.0f;
    clockAccumulator_ = 0.0;
    clockPhase_ = 0;
    sampleHoldValue_ = 0.0f;
    ampGatePrev_ = false;
    filterGatePrev_ = false;
    retriggerPending_ = false;
    meter_ = 0.0f;

    for (auto& osc : oscillators_)
        osc.reset();

    ladder1_.reset();
    ladder2_.reset();
    for (auto& filter : hp1_)
        filter.reset();

    ampEnvelope_.reset();
    filterEnvelope_.reset();
    lfo_.reset();
    lfo2_.reset();
    delay_.reset();
    dcLeft_.reset();
    dcRight_.reset();

    bus_.fill (0.0f);
    cableSource_.fill (-1);
    mult1Bus_ = 0.0f;
    mult2Bus_ = 0.0f;
    masterSmoother_.snap (param (PMasterVolume));
}

void MatriarchEngine::setParameters (const float* values, int count)
{
    if (values == nullptr)
        return;

    const auto n = std::min (count, (int) values_.size());
    for (int i = 0; i < n; ++i)
        values_[(size_t) i] = values[i];
}

void MatriarchEngine::setTransport (const TransportInfo& transport)
{
    transport_ = transport;
}

void MatriarchEngine::setPatchCable (int fromOutput, int toInput, bool connected)
{
    if (fromOutput < 0 || fromOutput >= JJackCount || toInput < 0 || toInput >= JJackCount)
        return;

    if (fromOutput == toInput)
        return;

    if (connected)
    {
        if (fromOutput < 64)
            cableLow_[(size_t) toInput].fetch_or (1ull << (fromOutput & 63));
        else
            cableHigh_[(size_t) toInput].fetch_or (1ull << (fromOutput - 64));
    }
    else
    {
        if (fromOutput < 64)
            cableLow_[(size_t) toInput].fetch_and (~(1ull << (fromOutput & 63)));
        else
            cableHigh_[(size_t) toInput].fetch_and (~(1ull << (fromOutput - 64)));
    }
}

void MatriarchEngine::clearPatchCables()
{
    for (auto& mask : cableLow_)
        mask.store (0);
    for (auto& mask : cableHigh_)
        mask.store (0);
    cableSource_.fill (-1);
}

void MatriarchEngine::latchPatchCables()
{
    for (int jack = 0; jack < JJackCount; ++jack)
    {
        const auto low = cableLow_[(size_t) jack].load (std::memory_order_relaxed);
        const auto high = cableHigh_[(size_t) jack].load (std::memory_order_relaxed);
        int source = -1;

        if (low != 0)
            source = (int) std::countr_zero (low);
        else if (high != 0)
            source = 64 + (int) std::countr_zero (high);

        cableSource_[(size_t) jack] = (int16_t) source;
    }
}

float MatriarchEngine::readJack (int input, float normal) const
{
    if (! supportsPatching())
        return normal;

    const auto source = cableSource_[(size_t) input];
    if (source >= 0)
        return bus_[(size_t) source];

    return normal;
}

void MatriarchEngine::writeJack (int output, float value)
{
    bus_[(size_t) output] = value;
}

int MatriarchEngine::soundingNoteFor (int oscillator) const
{
    if (heldCount_ == 0)
        return -1;

    const auto mode = choice (PParaphony);

    if (mode == 0)
        return heldNotes_[(size_t) (heldCount_ - 1)];

    if (mode == 1)
    {
        const auto voice = std::min (oscillator / 2, heldCount_ - 1);
        return heldNotes_[(size_t) voice];
    }

    return heldNotes_[(size_t) std::min (oscillator, heldCount_ - 1)];
}

bool MatriarchEngine::oscillatorGate (int oscillator) const
{
    if (heldCount_ == 0)
        return false;

    const auto mode = choice (PParaphony);

    if (mode == 0)
        return true;

    if (mode == 1)
        return (oscillator / 2) < heldCount_;

    return oscillator < heldCount_;
}

void MatriarchEngine::processMidi (const MidiEvent& event)
{
    switch (event.type)
    {
        case MidiEventType::NoteOn:
        {
            const auto note = std::clamp (event.note, 0, 127);
            const auto velocity = clampf (event.value, 0.0f, 1.0f);

            if (heldCount_ >= (int) heldNotes_.size())
            {
                for (int i = 1; i < heldCount_; ++i)
                {
                    heldNotes_[(size_t) (i - 1)] = heldNotes_[(size_t) i];
                    heldVelocities_[(size_t) (i - 1)] = heldVelocities_[(size_t) i];
                }
                --heldCount_;
            }

            heldNotes_[(size_t) heldCount_] = note;
            heldVelocities_[(size_t) heldCount_] = velocity;
            ++heldCount_;

            currentNote_ = (float) note;
            lastVelocity_ = velocity;
            retriggerPending_ = true;
            break;
        }

        case MidiEventType::NoteOff:
        {
            const auto note = std::clamp (event.note, 0, 127);
            int found = -1;

            for (int i = 0; i < heldCount_; ++i)
            {
                if (heldNotes_[(size_t) i] == note)
                {
                    found = i;
                    break;
                }
            }

            if (found < 0)
                break;

            if (sustainPedal_)
            {
                if (sustainedCount_ < (int) sustainedNotes_.size())
                    sustainedNotes_[(size_t) sustainedCount_++] = note;
                break;
            }

            for (int i = found; i < heldCount_ - 1; ++i)
            {
                heldNotes_[(size_t) i] = heldNotes_[(size_t) (i + 1)];
                heldVelocities_[(size_t) i] = heldVelocities_[(size_t) (i + 1)];
            }
            --heldCount_;
            break;
        }

        case MidiEventType::AllNotesOff:
        case MidiEventType::AllSoundOff:
            heldCount_ = 0;
            sustainedCount_ = 0;
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
        {
            const auto down = event.value > 0.5f;
            if (sustainPedal_ && ! down)
            {
                for (int s = 0; s < sustainedCount_; ++s)
                {
                    for (int i = 0; i < heldCount_; ++i)
                    {
                        if (heldNotes_[(size_t) i] == sustainedNotes_[(size_t) s])
                        {
                            for (int j = i; j < heldCount_ - 1; ++j)
                            {
                                heldNotes_[(size_t) j] = heldNotes_[(size_t) (j + 1)];
                                heldVelocities_[(size_t) j] = heldVelocities_[(size_t) (j + 1)];
                            }
                            --heldCount_;
                            break;
                        }
                    }
                }
                sustainedCount_ = 0;
            }
            sustainPedal_ = down;
            break;
        }

        case MidiEventType::ControlChange:
            break;
    }
}

void MatriarchEngine::render (float* left, float* right, int numSamples)
{
    if (left == nullptr || right == nullptr || numSamples <= 0)
        return;

    latchPatchCables();

    const auto mode = choice (PFilterMode);
    const auto vcaMode = choice (PVcaMode);
    const auto syncEnabled = choice (PSyncEnable) != 0;

    // Rate and envelope parameters are refreshed once per render call.
    lfo_.setRateHz (param (PLfoRate));
    switch (choice (PLfoWave))
    {
        case 0: lfo_.setWave (LfoWave::Sine); break;
        case 1: lfo_.setWave (LfoWave::SawUp); break;
        case 2: lfo_.setWave (LfoWave::SawDown); break;
        case 3: lfo_.setWave (LfoWave::Square); break;
        case 4: lfo_.setWave (LfoWave::SampleHold); break;
        default: lfo_.setWave (LfoWave::RandomSmooth); break;
    }

    lfo2_.setRateHz (param (PLfo2Rate));
    lfo2_.setBipolar (choice (PLfo2Bipolar) != 0);
    lfo2_.setWave (LfoWave::Triangle);

    ampEnvelope_.setParameters (param (PAmpAttack), param (PAmpDecay),
                                param (PAmpSustain), param (PAmpRelease));
    filterEnvelope_.setParameters (param (PFltAttack), param (PFltDecay),
                                   param (PFltSustain), param (PFltRelease));

    const auto cutoffBase = param (PFilterCutoff);
    const auto spacingOctaves = param (PFilterSpacing) * 4.0f;
    const auto resonance1 = param (PFilterRes1);
    const auto resonance2 = param (PFilterRes2);
    const auto envAmount = param (PFilterEnvAmt);
    const auto keyTrack = param (PFilterKbTrack);
    const auto velocityAmount = param (PVelocity);
    const auto aftertouchAmount = param (PAftertouch);
    const auto fineCents = param (PFineTune);
    const auto pitchAmount = param (PLfoPitchAmt);
    const auto pitchAssign = choice (PLfoPitchAssign);
    const auto cutoffAmount = param (PLfoCutoffAmt);
    const auto pulseWidthAmount = param (PLfoPwAmt);

    const auto delaySync = choice (PDelaySync) != 0;
    const auto bpm = std::max (20.0, transport_.bpm);
    auto delaySeconds = param (PDelayTime);
    if (delaySync)
    {
        static const double divisions[6] = { 0.25, 0.5, 0.75, 1.0, 1.5, 2.0 };
        const auto index = std::clamp ((int) std::lround (delaySeconds * 5.0f), 0, 5);
        delaySeconds = (float) ((60.0 / bpm) * divisions[index]);
    }
    const auto delaySpacing = param (PDelaySpacing);
    delay_.setDelaySeconds (delaySeconds * (1.0f + 0.35f * delaySpacing), delaySeconds);
    delay_.setFeedback (param (PDelayFeedback));
    delay_.setPingPong (choice (PDelayPingPong) != 0);

    const auto mainVolume = param (PMasterVolume);
    masterSmoother_.setTarget (mainVolume);

    ladder1_.setDrive (1.0f + resonance1 * 1.5f);
    ladder2_.setDrive (1.0f + resonance2 * 1.5f);

    // Patchable modulation of the delay and the main LFO is latched per block.
    const auto delayFeedbackCv = readJack (JDelayFbCv, 0.0f);
    delay_.setFeedback (clampf (param (PDelayFeedback) + delayFeedbackCv * 0.5f, 0.0f, 1.0f));

    const auto time1Cv = readJack (JDelayTime1Cv, 0.0f);
    const auto time2Cv = readJack (JDelayTime2Cv, 0.0f);
    if (std::abs (time1Cv) > 0.001f || std::abs (time2Cv) > 0.001f)
    {
        const auto baseLeft = delaySeconds * (1.0f + 0.35f * delaySpacing);
        delay_.setDelaySeconds (clampf (baseLeft * std::pow (2.0f, time1Cv * 2.0f), 0.01f, 1.9f),
                                clampf (delaySeconds * std::pow (2.0f, time2Cv * 2.0f), 0.01f, 1.9f));
    }

    const auto lfoRateCv = readJack (JLfoRateIn, 0.0f);
    lfo_.setRateHz (clampf (param (PLfoRate) * std::pow (2.0f, lfoRateCv * 4.0f), 0.01f, 1200.0f));

    const auto lfoSync = readJack (JLfoSyncIn, 0.0f) > 0.4f;
    if (lfoSync && ! lfoSyncPrev_)
        lfo_.reset (0.0f);
    lfoSyncPrev_ = lfoSync;

    float peak = 0.0f;
    const auto samplesPerStep = std::max (2.0, (60.0 / bpm) * sampleRate_ / 4.0);

    for (int sample = 0; sample < numSamples; ++sample)
    {
        // ---- Keyboard and performance sources --------------------------------
        const auto gateLevel = heldCount_ > 0 ? 1.0f : 0.0f;
        writeJack (JKbPitch, (currentNote_ - 60.0f) / 60.0f);
        writeJack (JKbGate, gateLevel);
        writeJack (JKbVelocity, lastVelocity_);
        writeJack (JKbAftertouch, aftertouch_);
        writeJack (JKbMod, modWheel_);

        // ---- Clock -----------------------------------------------------------
        clockAccumulator_ += 1.0;
        bool clockPulse = false;
        if (clockAccumulator_ >= samplesPerStep)
        {
            clockAccumulator_ -= samplesPerStep;
            clockPhase_ = (clockPhase_ + 1) & 3;
            clockPulse = true;
            sampleHoldValue_ = noise_.white();
        }
        if (readJack (JResetIn, 0.0f) > 0.4f)
            clockPhase_ = 0;

        writeJack (JClockOut, clockPulse ? 1.0f : 0.0f);
        writeJack (JStep1, clockPhase_ == 0 ? 1.0f : 0.0f);
        writeJack (JStep2, clockPhase_ == 2 ? 1.0f : 0.0f);
        writeJack (JResetOut, clockPulse && clockPhase_ == 0 ? 1.0f : 0.0f);
        writeJack (JShOut, sampleHoldValue_);

        // ---- Modulation ------------------------------------------------------
        const auto lfoValue = lfo_.process();
        const auto lfo2Value = lfo2_.process();
        writeJack (JLfoOut, lfoValue);
        writeJack (JLfo2Out, lfo2Value);
        writeJack (JNoiseOut, noise_.white());

        // ---- Envelopes -------------------------------------------------------
        const auto retrigger = retriggerPending_;
        retriggerPending_ = false;

        const auto ampGateInput = readJack (JAmpTrig, gateLevel);
        const auto ampGate = ampGateInput > 0.4f;
        if ((ampGate && ! ampGatePrev_) || (retrigger && ampGate))
            ampEnvelope_.noteOn (false);
        if (! ampGate && ampGatePrev_)
            ampEnvelope_.noteOff();
        ampGatePrev_ = ampGate;

        const auto filterGateInput = readJack (JFltTrig, gateLevel);
        const auto filterGate = filterGateInput > 0.4f;
        if ((filterGate && ! filterGatePrev_) || (retrigger && filterGate))
            filterEnvelope_.noteOn (false);
        if (! filterGate && filterGatePrev_)
            filterEnvelope_.noteOff();
        filterGatePrev_ = filterGate;

        const auto ampEnv = ampEnvelope_.process();
        const auto filterEnv = filterEnvelope_.process();
        writeJack (JAmpOut, ampEnv);
        writeJack (JAmpEnd, ampEnvelope_.isActive() ? 0.0f : 1.0f);
        writeJack (JFltOut, filterEnv);
        writeJack (JFltEnd, filterEnvelope_.isActive() ? 0.0f : 1.0f);

        // ---- Oscillators -----------------------------------------------------
        // Patch pitch CV is summed with each oscillator's own keyboard note
        // (1 V/oct, normalised to +/-60 semitones). The normalled chain carries
        // an upstream PITCH IN cable to later oscillators.
        float pitchChain = 0.0f;

        std::array<bool, 4> wrapped {};
        std::array<int, 4> oscNotes {};
        std::array<float, 4> oscOutputs {};

        for (int i = 0; i < 4; ++i)
        {
            const auto note = soundingNoteFor (i);
            oscNotes[(size_t) i] = note >= 0 ? note : (int) currentNote_;

            const auto octaveId = i == 0 ? PVco1Octave : (ParamId) (PVco2Octave + (i - 1) * 4);
            const auto octaveIndex = std::clamp ((int) std::lround (param (octaveId)), 0, 3);
            static const float octaveSemitones[4] = { -12.0f, 0.0f, 12.0f, 24.0f };
            auto semitones = (float) oscNotes[(size_t) i] - 60.0f + octaveSemitones[octaveIndex]
                           + fineCents / 100.0f + pitchBend_ * 2.0f;

            if (i > 0)
                semitones += param ((ParamId) (PVco2Freq + (i - 1) * 4));

            const auto pitchIn = readJack (JOsc1Pitch + i * 4, pitchChain);
            pitchChain = pitchIn;
            semitones += pitchIn * 60.0f;

            const auto appliesPitchMod = pitchAssign == 1
                                      || (pitchAssign == 0 && (i % 2 == 0))
                                      || (pitchAssign == 2 && (i % 2 == 1));
            if (appliesPitchMod)
                semitones += lfoValue * pitchAmount * modWheel_ * 12.0f;

            const auto fmInput = readJack (JOsc1Fm + i * 4, 0.0f);
            auto frequency = noteToHz (60.0f + semitones) + fmInput * 4000.0f;
            frequency = clampf (frequency, 8.0f, (float) sampleRate_ * 0.45f);

            auto& osc = oscillators_[(size_t) i];
            osc.setFrequency (frequency);
            wrapped[(size_t) i] = osc.advance();
        }

        if (syncEnabled)
        {
            for (int i = 1; i < 4; ++i)
            {
                const auto syncId = (ParamId) (PVco2Sync + (i - 1) * 4);
                if (choice (syncId) != 0 && wrapped[(size_t) (i - 1)])
                    oscillators_[(size_t) i].resetPhase();
            }
        }

        static const Oscillator::Wave waveMap[4] = {
            Oscillator::Wave::Triangle, Oscillator::Wave::Saw,
            Oscillator::Wave::Square, Oscillator::Wave::Pulse
        };

        for (int i = 0; i < 4; ++i)
        {
            const auto waveId = i == 0 ? PVco1Wave : (ParamId) (PVco2Wave + (i - 1) * 4);
            const auto waveIndexValue = std::clamp ((int) std::lround (param (waveId)), 0, 3);
            const auto pwm = readJack (JOsc1Pwm + i * 4, 0.0f);
            auto width = (waveIndexValue == 3 ? 0.25f : 0.5f)
                       + pwm * 0.45f + lfoValue * pulseWidthAmount * modWheel_ * 0.4f;
            width = clampf (width, 0.03f, 0.97f);

            auto value = oscillators_[(size_t) i].sample (waveMap[waveIndexValue], width);

            if (! oscillatorGate (i))
                value = 0.0f;

            oscOutputs[(size_t) i] = value;
            writeJack (JOsc1Out + i * 4, value);
        }

        // ---- Mixer -----------------------------------------------------------
        const float mixLevels[4] = { param (PMixOsc1), param (PMixOsc2),
                                     param (PMixOsc3), param (PMixOsc4) };
        float mix = 0.0f;
        for (int i = 0; i < 4; ++i)
        {
            const auto input = readJack (JMixOsc1In + i, oscOutputs[(size_t) i]);
            mix += input * mixLevels[i];
        }
        const auto noiseLevel = param (PMixNoise);
        mix += readJack (JMixNoiseIn, noise_.white()) * noiseLevel;
        mix *= 0.45f;
        mix = std::tanh (mix * 1.25f);
        writeJack (JMixOut, mix);

        // ---- Filters ---------------------------------------------------------
        const auto filterEnvOct = envAmount * filterEnv * 5.0f
                                + readJack (JFltEnvAmt, 0.0f) * filterEnv * 5.0f;
        const auto keyOct = keyTrack * ((float) currentNote_ - 60.0f) / 12.0f;
        const auto lfoCut = lfoValue * cutoffAmount * modWheel_ * 4.0f;
        const auto cutoff1Mod = readJack (JFlt1Cutoff, 0.0f) * 5.0f;
        const auto cutoff2Mod = readJack (JFlt2Cutoff, 0.0f) * 5.0f;

        auto cutoff1 = cutoffBase * std::pow (2.0f, spacingOctaves + filterEnvOct + keyOct + lfoCut + cutoff1Mod);
        auto cutoff2 = cutoffBase * std::pow (2.0f, filterEnvOct + keyOct + lfoCut + cutoff2Mod);
        cutoff1 = clampf (cutoff1, 20.0f, (float) sampleRate_ * 0.45f);
        cutoff2 = clampf (cutoff2, 20.0f, (float) sampleRate_ * 0.45f);

        ladder1_.setCutoffHz (cutoff1);
        ladder1_.setResonance01 (resonance1);
        ladder2_.setCutoffHz (cutoff2);
        ladder2_.setResonance01 (resonance2);
        hp1_[0].setCutoffHz (cutoff1);
        hp1_[0].setResonance01 (resonance1 * 0.35f);
        hp1_[1].setCutoffHz (cutoff1);
        hp1_[1].setResonance01 (resonance1 * 0.35f);

        const auto filterIn1 = readJack (JFlt1In, mix);
        const auto hpSignal = hp1_[1].process (hp1_[0].process (filterIn1, StateVariableFilter::Mode::HighPass),
                                              StateVariableFilter::Mode::HighPass);

        float outLeft = 0.0f;
        float outRight = 0.0f;

        if (mode == 0)
        {
            const auto filterIn2 = readJack (JFlt2In, hpSignal);
            const auto low = ladder2_.process (filterIn2);
            outLeft = outRight = low;
            writeJack (JFlt1Out, hpSignal);
            writeJack (JFlt2Out, low);
        }
        else if (mode == 1)
        {
            const auto filterIn2 = readJack (JFlt2In, mix);
            const auto low = ladder2_.process (filterIn2);
            const auto sum = hpSignal + low;
            outLeft = outRight = sum;
            writeJack (JFlt1Out, hpSignal);
            writeJack (JFlt2Out, low);
        }
        else
        {
            const auto filterIn2 = readJack (JFlt2In, mix);
            outLeft = ladder1_.process (filterIn1);
            outRight = ladder2_.process (filterIn2);
            writeJack (JFlt1Out, outLeft);
            writeJack (JFlt2Out, outRight);
        }

        // ---- VCAs ------------------------------------------------------------
        const auto velocityGain = (1.0f - velocityAmount) + velocityAmount * lastVelocity_;
        const auto aftertouchGain = 1.0f + aftertouchAmount * (aftertouch_ - 0.3f) * 0.6f;
        const auto ampGain = clampf (ampEnv * velocityGain * aftertouchGain, 0.0f, 1.2f);

        float cv1 = 0.0f;
        float cv2 = 0.0f;
        if (vcaMode == 0)
        {
            cv1 = cv2 = ampGain;
        }
        else if (vcaMode == 1)
        {
            cv1 = clampf (filterEnv * velocityGain, 0.0f, 1.2f);
            cv2 = ampGain;
        }
        else
        {
            cv1 = cv2 = 1.0f;
        }

        cv1 = clampf (cv1 + readJack (JVca1Cv, 0.0f), 0.0f, 1.5f);
        cv2 = clampf (cv2 + readJack (JVca2Cv, 0.0f), 0.0f, 1.5f);

        const auto vcaIn1 = readJack (JVca1In, outLeft);
        const auto vcaIn2 = readJack (JVca2In, outRight);
        const auto vcaOut1 = vcaIn1 * cv1;
        const auto vcaOut2 = vcaIn2 * cv2;

        // ---- Delay -----------------------------------------------------------
        const auto dryWet = clampf (param (PDelayMix) + readJack (JDelayMixCv, 0.0f) * 0.5f, 0.0f, 1.0f);
        const auto delayIn1 = readJack (JDelayIn1, vcaOut1);
        const auto delayIn2 = readJack (JDelayIn2, vcaOut2);
        const auto wet = delay_.process (delayIn1, delayIn2);
        writeJack (JDelayOut1, wet.left);
        writeJack (JDelayOut2, wet.right);

        // ---- Utilities -------------------------------------------------------
        const auto att1 = readJack (JAtt1In, 0.0f) * param (PAtten1);
        const auto att2 = readJack (JAtt2In, 0.0f) * param (PAtten2);
        const auto att3 = readJack (JAtt3In, 0.0f) * param (PAtten3);
        writeJack (JAtt1Out, att1);
        writeJack (JAtt2Out, att2);
        writeJack (JAtt3Out, att3);

        const auto mult1 = readJack (JMult1A, 0.0f) + readJack (JMult1B, 0.0f)
                         + readJack (JMult1C, 0.0f) + readJack (JMult1D, 0.0f);
        const auto mult2 = readJack (JMult2A, 0.0f) + readJack (JMult2B, 0.0f)
                         + readJack (JMult2C, 0.0f) + readJack (JMult2D, 0.0f);
        mult1Bus_ = mult1Bus_ * 0.5f + mult1 * 0.5f;
        mult2Bus_ = mult2Bus_ * 0.5f + mult2 * 0.5f;
        writeJack (JMult1A, mult1Bus_);
        writeJack (JMult1B, mult1Bus_);
        writeJack (JMult1C, mult1Bus_);
        writeJack (JMult1D, mult1Bus_);
        writeJack (JMult2A, mult2Bus_);
        writeJack (JMult2B, mult2Bus_);
        writeJack (JMult2C, mult2Bus_);
        writeJack (JMult2D, mult2Bus_);

        // ---- Output ----------------------------------------------------------
        auto finalLeft = vcaOut1 * (1.0f - dryWet) + wet.left * dryWet;
        auto finalRight = vcaOut2 * (1.0f - dryWet) + wet.right * dryWet;
        const auto volume = masterSmoother_.next();
        finalLeft = dcLeft_.process (finalLeft * volume);
        finalRight = dcRight_.process (finalRight * volume);
        finalLeft = std::tanh (finalLeft * 1.25f) * 0.85f;
        finalRight = std::tanh (finalRight * 1.25f) * 0.85f;

        peak = std::max (peak, std::max (std::abs (finalLeft), std::abs (finalRight)));
        left[sample] = finalLeft;
        right[sample] = finalRight;
    }

    meter_ = std::max (peak, meter_ * 0.85f);
}

std::unique_ptr<SynthEngine> makeMatriarchEngine()
{
    return std::make_unique<MatriarchEngine>();
}

} // namespace triad
