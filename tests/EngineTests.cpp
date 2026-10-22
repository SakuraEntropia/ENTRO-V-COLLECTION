#include "engines/Engines.h"
#include "core/Sequencer.h"
#include "core/ContinuumProcessing.h"
#include "engines/Continuum.h"
#include <numeric>

#include <cmath>
#include <cstdio>
#include <set>
#include <string>
#include <vector>

namespace
{

int failures = 0;

#define CHECK(condition)                                                            \
    do                                                                              \
    {                                                                               \
        if (! (condition))                                                          \
        {                                                                           \
            std::printf ("FAIL %s:%d  %s\n", __FILE__, __LINE__, #condition);       \
            ++failures;                                                             \
        }                                                                           \
    } while (false)

void checkFinite (const std::vector<float>& left, const std::vector<float>& right,
                  const char* context)
{
    for (size_t i = 0; i < left.size(); ++i)
    {
        if (! std::isfinite (left[i]) || ! std::isfinite (right[i]))
        {
            std::printf ("FAIL non-finite sample in %s at index %zu\n", context, i);
            ++failures;
            return;
        }
    }
}

void renderSeconds (triad::SynthEngine& engine, double seconds, double sampleRate,
                    std::vector<float>& left, std::vector<float>& right)
{
    const auto total = (int) (seconds * sampleRate);
    const auto block = 512;
    left.assign ((size_t) total, 0.0f);
    right.assign ((size_t) total, 0.0f);

    for (int position = 0; position < total; position += block)
    {
        const auto count = std::min (block, total - position);
        engine.render (left.data() + position, right.data() + position, count);
    }
}

double energyOf (const std::vector<float>& left, const std::vector<float>& right)
{
    double energy = 0.0;
    for (size_t i = 0; i < left.size(); ++i)
        energy += (double) left[i] * left[i] + (double) right[i] * right[i];

    return energy;
}

void testDefinition (const triad::ModelDefinition& definition)
{
    CHECK (! definition.params.empty());
    CHECK (! definition.id.empty());

    std::set<std::string> ids;
    for (const auto& spec : definition.params)
    {
        CHECK (! spec.id.empty());
        CHECK (! spec.name.empty());
        CHECK (spec.max >= spec.min);
        CHECK (spec.def >= spec.min && spec.def <= spec.max);
        CHECK (ids.insert (spec.id).second);
    }

    std::set<std::string> jacks;
    for (const auto& jack : definition.patchPoints)
        CHECK (jacks.insert (jack.id).second);
}

void testInstrument (triad::InstrumentKind kind, const char* name)
{
    auto engine = triad::makeEngine (kind);
    CHECK (engine != nullptr);
    if (engine == nullptr)
        return;

    auto& definition = engine->definition();
    testDefinition (definition);

    std::vector<float> values (definition.params.size());
    for (size_t i = 0; i < values.size(); ++i)
        values[i] = definition.params[i].def;

    for (const double sampleRate : { 44100.0, 48000.0, 96000.0 })
    {
        engine->prepare (sampleRate, 512);
        engine->reset();
        engine->setParameters (values.data(), (int) values.size());

        triad::MidiEvent on;
        on.type = triad::MidiEventType::NoteOn;
        on.note = 60;
        on.value = 0.8f;
        on.channel = 0;
        engine->processMidi (on);

        triad::MidiEvent fifth;
        fifth.type = triad::MidiEventType::NoteOn;
        fifth.note = 67;
        fifth.value = 0.7f;
        fifth.channel = 0;
        engine->processMidi (fifth);

        std::vector<float> left, right;
        renderSeconds (*engine, 1.0, sampleRate, left, right);
        checkFinite (left, right, name);

        const auto energy = energyOf (left, right);
        if (energy <= 1.0e-6)
            std::printf ("note: %s produced low energy at %.0f Hz (%.8f)\n", name, sampleRate, energy);
        CHECK (energy > 1.0e-6);

        triad::MidiEvent off;
        off.type = triad::MidiEventType::NoteOff;
        off.note = 60;
        engine->processMidi (off);

        triad::MidiEvent offFifth;
        offFifth.type = triad::MidiEventType::NoteOff;
        offFifth.note = 67;
        engine->processMidi (offFifth);

        renderSeconds (*engine, 0.5, sampleRate, left, right);
        checkFinite (left, right, name);
    }

    // Note overlap, sustain, pitch bend, pressure, all-notes-off.
    engine->prepare (48000.0, 256);
    engine->reset();
    engine->setParameters (values.data(), (int) values.size());

    for (int note = 48; note < 60; ++note)
    {
        triad::MidiEvent on;
        on.type = triad::MidiEventType::NoteOn;
        on.note = note;
        on.value = 0.9f;
        engine->processMidi (on);
    }

    triad::MidiEvent sustain;
    sustain.type = triad::MidiEventType::SustainPedal;
    sustain.value = 1.0f;
    engine->processMidi (sustain);

    for (int note = 48; note < 60; ++note)
    {
        triad::MidiEvent off;
        off.type = triad::MidiEventType::NoteOff;
        off.note = note;
        engine->processMidi (off);
    }

    triad::MidiEvent bend;
    bend.type = triad::MidiEventType::PitchBend;
    bend.value = 0.5f;
    engine->processMidi (bend);

    triad::MidiEvent pressure;
    pressure.type = triad::MidiEventType::ChannelPressure;
    pressure.value = 0.7f;
    engine->processMidi (pressure);

    std::vector<float> left, right;
    renderSeconds (*engine, 0.5, 48000.0, left, right);
    checkFinite (left, right, name);

    triad::MidiEvent allOff;
    allOff.type = triad::MidiEventType::AllNotesOff;
    engine->processMidi (allOff);

    triad::MidiEvent sustainUp;
    sustainUp.type = triad::MidiEventType::SustainPedal;
    sustainUp.value = 0.0f;
    engine->processMidi (sustainUp);

    renderSeconds (*engine, 1.5, 48000.0, left, right);
    checkFinite (left, right, name);

    // Parameter extremes must remain bounded.
    for (int pass = 0; pass < 2; ++pass)
    {
        for (size_t i = 0; i < values.size(); ++i)
            values[i] = pass == 0 ? definition.params[i].min : definition.params[i].max;

        engine->reset();
        engine->setParameters (values.data(), (int) values.size());

        triad::MidiEvent on;
        on.type = triad::MidiEventType::NoteOn;
        on.note = 72;
        on.value = 1.0f;
        engine->processMidi (on);

        renderSeconds (*engine, 0.5, 48000.0, left, right);
        checkFinite (left, right, name);
    }
}

double estimateFundamental (const std::vector<float>& samples, double seconds,
                            double sampleRate)
{
    (void) seconds;

    const auto skip = std::min ((size_t) (0.25 * sampleRate), samples.size());
    if (samples.size() - skip < (size_t) (0.6 * sampleRate))
        return 0.0;

    const auto window = std::min ((size_t) (0.5 * sampleRate), samples.size() - skip);
    std::vector<float> mono (window, 0.0f);

    float mean = 0.0f;
    for (size_t i = 0; i < window; ++i)
    {
        mono[i] = samples[skip + i];
        mean += mono[i];
    }
    mean /= (float) window;

    for (auto& value : mono)
        value -= mean;

    const auto minLag = (int) (sampleRate / 2200.0);
    const auto maxLag = (int) (sampleRate / 50.0);

    if (maxLag >= (int) window || minLag < 1)
        return 0.0;

    std::vector<double> correlation ((size_t) (maxLag + 1), 0.0);
    double best = 0.0;

    for (int lag = minLag; lag <= maxLag; ++lag)
    {
        double numerator = 0.0;
        double energyA = 0.0;
        double energyB = 0.0;
        const auto count = window - (size_t) lag;

        for (size_t i = 0; i < count; ++i)
        {
            const auto a = (double) mono[i];
            const auto b = (double) mono[i + (size_t) lag];
            numerator += a * b;
            energyA += a * a;
            energyB += b * b;
        }

        const auto denominator = std::sqrt (energyA * energyB);
        correlation[(size_t) lag] = denominator > 1.0e-12 ? numerator / denominator : 0.0;
        best = std::max (best, correlation[(size_t) lag]);
    }

    if (best < 0.25)
        return 0.0;

    // Prefer the shortest lag that still correlates strongly, avoiding octave-down errors.
    for (int lag = minLag + 1; lag < maxLag; ++lag)
    {
        const auto value = correlation[(size_t) lag];
        if (value >= best * 0.92
            && value >= correlation[(size_t) lag - 1]
            && value >= correlation[(size_t) lag + 1])
        {
            const auto previous = correlation[(size_t) lag - 1];
            const auto next = correlation[(size_t) lag + 1];
            const auto denominator = previous - 2.0 * value + next;
            const auto offset = std::abs (denominator) > 1.0e-12
                                    ? 0.5 * (previous - next) / denominator
                                    : 0.0;
            return sampleRate / ((double) lag + offset);
        }
    }

    return 0.0;
}

void testTuning (triad::InstrumentKind kind, const char* name)
{
    auto engine = triad::makeEngine (kind);
    auto& definition = engine->definition();

    std::vector<float> values (definition.params.size());
    for (size_t i = 0; i < values.size(); ++i)
        values[i] = definition.params[i].def;

    if (kind == triad::InstrumentKind::Continuum)
    {
        // The default Continuum patch is deliberately layered with sub and
        // fixed banks.  Isolate the fundamental oscillator here so this test
        // measures MIDI-note tracking rather than the colour of the patch.
        auto set = [&] (const char* id, float value)
        {
            const auto index = definition.indexOf (id);
            if (index >= 0)
                values[(size_t) index] = value;
        };

        for (int osc = 2; osc <= 5; ++osc)
            set (("con.osc" + std::to_string (osc) + ".level").c_str(), 0.0f);

        set ("con.bank1.mix", 0.0f);
        set ("con.bank2.mix", 0.0f);
        for (int f = 1; f <= 5; ++f)
            set (("con.flt" + std::to_string (f) + ".cutoff").c_str(), 20000.0f);
        set ("con.master.reverbMix", 0.0f);
        set ("con.master.delayMix", 0.0f);
    }

    engine->prepare (48000.0, 512);
    engine->reset();
    engine->setParameters (values.data(), (int) values.size());

    struct Expected { int note; double frequency; };
    const Expected cases[] = { { 69, 440.0 }, { 57, 220.0 }, { 81, 880.0 } };

    for (const auto& testCase : cases)
    {
        engine->processMidi ([] (int note)
        {
            triad::MidiEvent event;
            event.type = triad::MidiEventType::NoteOn;
            event.note = note;
            event.value = 0.85f;
            return event;
        } (testCase.note));

        std::vector<float> left, right;
        renderSeconds (*engine, 1.25, 48000.0, left, right);

        const auto measured = estimateFundamental (left, 1.0, 48000.0);
        const auto ratio = measured / testCase.frequency;
        const auto cents = 1200.0 * std::log2 (std::max (1.0e-6, ratio));

        if (std::abs (cents) > 60.0)
            std::printf ("note: %s note %d measured %.2f Hz (%+.1f cents, expected %.2f)\n",
                         name, testCase.note, measured, cents, testCase.frequency);

        CHECK (std::abs (cents) <= 60.0);

        triad::MidiEvent off;
        off.type = triad::MidiEventType::NoteOff;
        off.note = testCase.note;
        engine->processMidi (off);
        engine->reset();
        engine->setParameters (values.data(), (int) values.size());
    }
}

void testOneBeatFrequency()
{
    for (int part = 1; part <= 3; ++part)
        for (int note : { 69, 81 })
        {
            auto engine = triad::makeEngine (triad::InstrumentKind::MoogOne);
            const auto& def = engine->definition();
            CHECK (def.params.size() == 417 && def.params[408].id == "one.p1.vco1.beatHz");
            std::vector<float> values; for (const auto& spec : def.params) values.push_back (spec.def);
            auto set = [&] (std::string id, float value) { values[(size_t) def.indexOf (id)] = value; };
            const auto prefix = "one.p" + std::to_string (part) + ".";
            set ("one.master.drift", 0); set ("one.master.reverbMix", 0);
            set (prefix + "vco2.level", 0); set (prefix + "vco3.level", 0);
            set (prefix + "mix.noise", 0); set (prefix + "mix.ring", 0);
            set (prefix + "vco1.beatHz", 7);
            set (prefix + "svf.cutoff", 20000); set (prefix + "ladder.cutoff", 20000);
            engine->prepare (48000, 512); engine->setParameters (values.data(), (int) values.size());
            triad::MidiEvent on; on.type = triad::MidiEventType::NoteOn;
            on.note = note; on.channel = part - 1; on.value = .8f; engine->processMidi (on);
            std::vector<float> l, r; renderSeconds (*engine, 1, 48000, l, r);
            const double expected = (note == 69 ? 440 : 880) + 7;
            CHECK (std::abs (estimateFundamental (l, 1, 48000) - expected) < .5);
        }
}

void testContinuumUserFormula()
{
    auto engine = triad::makeEngine (triad::InstrumentKind::Continuum);
    const auto& definition = engine->definition();

    std::vector<float> values (definition.params.size());
    for (size_t i = 0; i < values.size(); ++i)
        values[i] = definition.params[i].def;

    engine->prepare (48000.0, 256);
    engine->setParameters (values.data(), (int) values.size());

    CHECK (engine->matrixFormulaCount() == 19);
    CHECK (engine->matrixFormulaName (12) == "User");

    engine->setUserFormula ("sin(pi*x)*y + z");
    engine->setMatrixCell (0, 0, 12, 1.0f);

    triad::MidiEvent on;
    on.type = triad::MidiEventType::NoteOn;
    on.note = 60;
    on.value = 0.8f;
    engine->processMidi (on);

    std::vector<float> left, right;
    renderSeconds (*engine, 0.2, 48000.0, left, right);
    checkFinite (left, right, "continuum user formula");

    // Invalid expressions must be rejected without killing the engine.
    engine->setUserFormula ("sin(");
    renderSeconds (*engine, 0.05, 48000.0, left, right);
    checkFinite (left, right, "continuum invalid user formula");
}

void testContinuumExtensions()
{
    // Neutral master controls are sample-exact bypass; compression is stereo linked.
    triad::ContinuumMasterProcessor processor;
    processor.prepare (48000);
    processor.configure (false, -18, 4, .001f, .1f, 0, 0, 1000);
    auto output = processor.process (.6f, -.3f);
    CHECK (output.left == .6f && output.right == -.3f);
    processor.configure (true, -24, 10, .0001f, .1f, 0, 0, 1000);
    for (int i = 0; i < 10000; ++i) output = processor.process (.8f, -.4f);
    CHECK (std::abs (output.left) < .15f);
    CHECK (std::abs (output.left + output.right * 2) < .00001f);
    const auto tiltEnergy = [] (float hz)
    {
        triad::ContinuumMasterProcessor p; p.prepare (48000);
        p.configure (false, -18, 4, .01f, .1f, 0, 12, 1000);
        double energy = 0;
        for (int i = 0; i < 10000; ++i)
        {
            const float value = std::sin (triad::kTwoPi * hz * i / 48000);
            const auto sample = p.process (value, value);
            if (i > 1000) energy += sample.left * sample.left;
        }
        return energy;
    };
    CHECK (tiltEnergy (8000) > tiltEnergy (80) * 6);

    // Generated impulse controls affect the actual convolution, including width.
    const auto impulse = [] (int model, float length, float tuning, float width)
    {
        triad::ConvolutionEffect effect; effect.prepare (48000, 17);
        effect.setMix (1); effect.setControls (model, length, tuning, width);
        std::vector<float> result;
        for (int i = 0; i < 512; ++i)
        {
            const auto sample = effect.process (i == 0 ? 1.0f : 0.0f, i == 0 ? .3f : 0.0f);
            CHECK (std::isfinite (sample.left) && std::isfinite (sample.right));
            if (width == 0) CHECK (sample.left == sample.right);
            result.push_back (sample.left);
        }
        return result;
    };
    const auto original = impulse (0, 1, 1, 1);
    CHECK (original != impulse (1, 1, 1, 1)); CHECK (original != impulse (2, 1, 1, 1));
    CHECK (original != impulse (3, 1, 1, 1)); CHECK (original != impulse (0, .2f, 1, 1));
    CHECK (original != impulse (0, 1, 1.5f, 1)); CHECK (original != impulse (0, 1, 1, 0));

    triad::ContinuumEngine engine;
    const auto& definition = engine.definition();
    CHECK (definition.params.size() == 204);
    CHECK (definition.params[127].id == "con.env2.release");
    CHECK (definition.params[128].id == "con.fx.comp.enabled");
    CHECK (engine.matrixColumns() == 22 && engine.matrixDestinationName (15) == "MASTER");
    CHECK (engine.matrixFormulaName (13) == "A" && engine.matrixFormulaName (18) == "F");
    std::vector<float> values;
    for (const auto& spec : definition.params) values.push_back (spec.def);
    auto set = [&] (const char* id, float value) { values[(size_t) definition.indexOf (id)] = value; };
    engine.prepare (48000, 256);
    for (int i = 1; i <= 6; ++i)
    {
        const auto text = "s*" + std::to_string (i);
        CHECK (engine.setFormulaText (i, text));
        CHECK (! engine.setFormulaText (i, "sin("));
        CHECK (engine.formulaText (i) == text);
    }
    float l[256] {}, r[256] {};
    engine.setParameters (values.data(), (int) values.size()); engine.render (l, r, 1);
    for (int i = 0; i < 6; ++i)
        CHECK (std::abs (engine.evaluateNamedFormula (i, .1f, .2f, .3f, .4f) - .1f * (i + 1)) < .0001f);
    set ("con.formula1.curve", 3); set ("con.formula1.scale", 2); set ("con.formula1.offset", .1f);
    engine.setParameters (values.data(), (int) values.size()); engine.render (l, r, 1);
    CHECK (std::abs (engine.evaluateNamedFormula (0, .5f, 0, 0, 0) - .6f) < .0001f);

    const auto audio = [] (const std::vector<std::pair<const char*, float>>& changes, bool masterColumn = false)
    {
        auto e = triad::makeEngine (triad::InstrumentKind::Continuum);
        const auto& d = e->definition(); std::vector<float> v;
        for (const auto& spec : d.params) v.push_back (spec.def);
        for (const auto& change : changes) v[(size_t) d.indexOf (change.first)] = change.second;
        e->prepare (48000, 256); e->setParameters (v.data(), (int) v.size());
        if (masterColumn) e->setMatrixCell (11, 16, 1, 1);
        triad::MidiEvent on; on.type = triad::MidiEventType::NoteOn; on.note = 60; on.value = .8f; e->processMidi (on);
        std::vector<float> left, right; renderSeconds (*e, .12, 48000, left, right);
        checkFinite (left, right, "Continuum extensions"); return left;
    };
    const auto base = audio ({});
    CHECK (base != audio ({{ "con.fx.comp.enabled", 1 }, { "con.fx.comp.threshold", -50 }}));
    CHECK (base != audio ({{ "con.fx.tilt.gain", 9 }}));
    CHECK (base != audio ({{ "con.bank1.size", 1 }}));
    CHECK (base != audio ({{ "con.bank1.bwSpread", .8f }, { "con.bank1.centre", .8f }, { "con.bank1.weight", 1 }}));
    for (int type = 1; type <= 6; ++type)
        CHECK (base != audio ({{ "con.flt1.model", (float) type }, { "con.flt1.stages", 3 }}));
    CHECK (audio ({{ "con.mastercol1.target", 15 }}) != audio ({{ "con.mastercol1.target", 15 }}, true));

    // A one-shot SG holds its final value; a rising trigger restarts it.
    for (int row = 0; row < engine.matrixRows(); ++row)
        for (int col = 0; col < engine.matrixColumns(); ++col) engine.setMatrixCell (row, col, 0, 0);
    engine.setMatrixCell (11, 15, 1, 1);
    set ("con.sg1.cycle", 2); set ("con.sg1.rate", 30); set ("con.sg1.shape", 0);
    set ("con.master.reverbMix", 0); set ("con.master.delayMix", 0); set ("con.fx.mode", 5);
    engine.setParameters (values.data(), (int) values.size());
    triad::MidiEvent note; note.type = triad::MidiEventType::NoteOn; note.note = 60; note.value = 1; engine.processMidi (note);
    std::vector<float> left, right; renderSeconds (engine, .5, 48000, left, right);
    double tail = 0; for (size_t i = left.size() - 256; i < left.size(); ++i) tail += std::abs (left[i]);
    set ("con.sg1.trigger", 1); engine.setParameters (values.data(), (int) values.size());
    engine.render (l, r, 256); double restarted = 0; for (float v : l) restarted += std::abs (v);
    CHECK (restarted > tail * 2 + .01);
}

void testContinuumPitchExpression()
{
    auto engine = triad::makeEngine (triad::InstrumentKind::Continuum);
    const auto& definition = engine->definition();
    std::vector<float> values (definition.params.size());
    for (size_t i = 0; i < values.size(); ++i)
        values[i] = definition.params[i].def;
    auto set = [&] (const char* id, float value)
    {
        const auto index = definition.indexOf (id);
        if (index >= 0)
            values[(size_t) index] = value;
    };

    // Isolate oscillator 1 so zero crossings measure pitch, not fixed banks.
    set ("con.osc1.level", 0.8f);
    set ("con.osc2.level", 0.0f);
    set ("con.osc3.level", 0.0f);
    set ("con.osc4.level", 0.0f);
    set ("con.osc5.level", 0.0f);
    set ("con.noise.level", 0.0f);
    set ("con.bank1.mix", 0.0f);
    set ("con.bank2.mix", 0.0f);
    set ("con.flt1.cutoff", 18000.0f);
    set ("con.flt2.cutoff", 18000.0f);
    set ("con.flt3.cutoff", 18000.0f);
    set ("con.flt4.cutoff", 18000.0f);
    set ("con.flt5.cutoff", 18000.0f);
    set ("con.env1.attack", 0.001f);
    set ("con.env1.sustain", 1.0f);
    set ("con.master.reverbMix", 0.0f);
    set ("con.master.delayMix", 0.0f);

    engine->prepare (48000.0, 256);
    engine->setParameters (values.data(), (int) values.size());

    triad::MidiEvent on;
    on.type = triad::MidiEventType::NoteOn;
    on.note = 60;
    on.value = 0.8f;
    engine->processMidi (on);

    auto crossings = [&engine] (float semis)
    {
        engine->setNoteExpression (60, semis, 0.5f, 0.8f);
        std::vector<float> l, r;
        renderSeconds (*engine, 0.35, 48000.0, l, r);
        int count = 0;
        for (size_t i = 1; i < l.size(); ++i)
            if (l[i - 1] <= 0.0f && l[i] > 0.0f)
                ++count;
        return count;
    };

    const auto low = crossings (-12.0f);
    engine->processMidi ({ triad::MidiEventType::NoteOff, 0, 60, 0, 0.0f });
    engine->processMidi (on);
    const auto high = crossings (12.0f);
    std::printf ("continuum pitch crossings low=%d high=%d\n", low, high);
    CHECK (high > low * 2);
}

void testMatriarchPatching()
{
    auto engine = triad::makeEngine (triad::InstrumentKind::Matriarch);
    CHECK (engine->supportsPatching());

    auto& definition = engine->definition();
    std::vector<float> values (definition.params.size());
    for (size_t i = 0; i < values.size(); ++i)
        values[i] = definition.params[i].def;

    engine->prepare (48000.0, 256);
    engine->setParameters (values.data(), (int) values.size());

    const auto oscOut = definition.patchIndexOf ("mat.patch.vco1.out");
    const auto filterIn = definition.patchIndexOf ("mat.patch.flt1.in");
    CHECK (oscOut >= 0);
    CHECK (filterIn >= 0);

    engine->setPatchCable (oscOut, filterIn, true);

    triad::MidiEvent on;
    on.type = triad::MidiEventType::NoteOn;
    on.note = 60;
    on.value = 0.8f;
    engine->processMidi (on);

    std::vector<float> left, right;
    renderSeconds (*engine, 0.25, 48000.0, left, right);
    checkFinite (left, right, "matriarch patch");

    engine->clearPatchCables();
}

void testMatriarchGlideAndTap()
{
    triad::TapTempo tap;
    CHECK (tap.tap (0.0) == 0.0);
    CHECK (std::abs (tap.tap (500.0) - 120.0) < 0.001);
    CHECK (std::abs (tap.tap (1000.0) - 120.0) < 0.001);
    CHECK (tap.tap (5000.0) == 0.0); // A pause resets the average.
    CHECK (std::abs (tap.tap (6000.0) - 60.0) < 0.001);
    CHECK (tap.tap (6001.0) == 0.0); // Ignore accidental double-clicks.

    auto crossings = [] (float glide, double duration)
    {
        auto engine = triad::makeEngine (triad::InstrumentKind::Matriarch);
        const auto& def = engine->definition();
        std::vector<float> values;
        for (const auto& p : def.params) values.push_back (p.def);
        auto set = [&] (const char* id, float v) { values[(size_t) def.indexOf (id)] = v; };
        set ("mat.performance.glide", glide); set ("mat.master.paraphony", 0);
        set ("mat.mix.osc2", 0); set ("mat.vco1.wave", 0);
        set ("mat.flt.cutoff", 20000); set ("mat.flt.res1", 0); set ("mat.flt.res2", 0);
        set ("mat.flt.envAmt", 0); set ("mat.delay.mix", 0); set ("mat.env1.sustain", 1);
        engine->prepare (48000, 512); engine->setParameters (values.data(), (int) values.size());
        triad::MidiEvent note; note.type = triad::MidiEventType::NoteOn; note.note = 60; note.value = .8f;
        engine->processMidi (note);
        std::vector<float> left, right; renderSeconds (*engine, .2, 48000, left, right);
        note.note = 72; engine->processMidi (note);
        renderSeconds (*engine, duration, 48000, left, right);
        int result = 0;
        const auto start = duration > .2 ? left.size() - 4800 : (size_t) 0;
        for (size_t i = start + 1; i < left.size(); ++i)
            if (left[i - 1] <= 0 && left[i] > 0) ++result;
        return result;
    };
    const auto immediate = crossings (0, .1), gliding = crossings (.8f, .1), settled = crossings (.8f, 1.5);
    CHECK (immediate > 48 && immediate < 58);
    CHECK (gliding > 24 && gliding < immediate - 5);
    CHECK (std::abs (settled - immediate) <= 2);
}

} // namespace

int main()
{
    testInstrument (triad::InstrumentKind::Matriarch, "Matriarch");
    testInstrument (triad::InstrumentKind::PolyBrute, "PolyBrute");
    testInstrument (triad::InstrumentKind::MoogOne, "Moog One");
    testInstrument (triad::InstrumentKind::Continuum, "Continuum");
    testTuning (triad::InstrumentKind::Matriarch, "Matriarch");
    testTuning (triad::InstrumentKind::PolyBrute, "PolyBrute");
    testTuning (triad::InstrumentKind::MoogOne, "Moog One");
    testTuning (triad::InstrumentKind::Continuum, "Continuum");
    testOneBeatFrequency();
    testContinuumUserFormula();
    testContinuumExtensions();
    testContinuumPitchExpression();
    testMatriarchPatching();
    testMatriarchGlideAndTap();

    if (failures == 0)
    {
        std::printf ("All engine checks passed.\n");
        return 0;
    }

    std::printf ("%d check(s) failed.\n", failures);
    return 1;
}
