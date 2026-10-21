#include "engines/Engines.h"

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

void testContinuumUserFormula()
{
    auto engine = triad::makeEngine (triad::InstrumentKind::Continuum);
    const auto& definition = engine->definition();

    std::vector<float> values (definition.params.size());
    for (size_t i = 0; i < values.size(); ++i)
        values[i] = definition.params[i].def;

    engine->prepare (48000.0, 256);
    engine->setParameters (values.data(), (int) values.size());

    CHECK (engine->matrixFormulaCount() == 13);
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
    testContinuumUserFormula();
    testContinuumPitchExpression();
    testMatriarchPatching();

    if (failures == 0)
    {
        std::printf ("All engine checks passed.\n");
        return 0;
    }

    std::printf ("%d check(s) failed.\n", failures);
    return 1;
}
