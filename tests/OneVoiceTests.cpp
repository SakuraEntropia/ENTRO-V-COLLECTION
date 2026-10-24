#include "engines/One.h"

#include <array>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

namespace
{
int failures = 0;
#define CHECK(condition) do { if (!(condition)) { std::printf ("FAIL %s:%d: %s\n", __FILE__, __LINE__, #condition); ++failures; } } while (false)

struct Fixture
{
    triad::OneEngine engine;
    std::vector<float> values;

    Fixture()
    {
        for (const auto& spec : engine.definition().params) values.push_back (spec.def);
        engine.prepare (48000, 256);
        set ("one.master.reverbMix", 0);
        set ("one.master.drift", 0);
        for (int part = 1; part <= 3; ++part)
        {
            const auto p = "one.p" + std::to_string (part) + ".";
            for (int env = 1; env <= 3; ++env)
                set (p + "env" + std::to_string (env) + ".release", .002f);
            set (p + "fx.chorus.mix", 0); set (p + "fx.delay.mix", 0);
        }
        apply();
    }

    void set (const std::string& id, float value)
    {
        const auto index = engine.definition().indexOf (id);
        if (index < 0) { std::printf ("Missing parameter: %s\n", id.c_str()); std::exit (2); }
        values[(size_t) index] = value;
    }
    void apply() { engine.setParameters (values.data(), (int) values.size()); }
    void on (int note, int channel = 0) { engine.processMidi ({ triad::MidiEventType::NoteOn, channel, note, 0, .8f }); }
    void off (int note, int channel = 0) { engine.processMidi ({ triad::MidiEventType::NoteOff, channel, note }); }
    void pedal (bool down, int channel = 0) { engine.processMidi ({ triad::MidiEventType::SustainPedal, channel, 0, 0, down ? 1.0f : 0.0f }); }
    int render (int count = 4800)
    {
        std::array<float, 256> l {}, r {};
        int crossings = 0; float previous = 0;
        for (int offset = 0; offset < count; offset += 256)
        {
            const auto n = std::min (256, count - offset);
            engine.render (l.data(), r.data(), n);
            for (int i = 0; i < n; ++i)
            {
                CHECK (std::isfinite (l[(size_t) i]) && std::isfinite (r[(size_t) i]));
                CHECK (std::abs (l[(size_t) i]) <= 2 && std::abs (r[(size_t) i]) <= 2);
                if (previous <= 0 && l[(size_t) i] > 0) ++crossings;
                previous = l[(size_t) i];
            }
        }
        return crossings;
    }
    void layer()
    {
        set ("one.p2.midiChannel", 0); set ("one.p3.midiChannel", 0); apply();
    }
    void cleanPitch()
    {
        set ("one.p1.vco2.level", 0); set ("one.p1.vco3.level", 0);
        set ("one.p1.mix.noise", 0); set ("one.p1.mix.ring", 0);
        set ("one.p1.filters.mix", 0); set ("one.p1.svf.cutoff", 20000);
        set ("one.p1.svf.resonance", 0); set ("one.p1.svf.envAmt", 0);
        set ("one.p1.env1.sustain", 1); apply();
    }
};

void sharedBudget()
{
    Fixture f;
    const auto& d = f.engine.definition();
    CHECK (d.params[407].id == "one.p3.arp.length");
    CHECK (d.params[408].id == "one.p1.vco1.beatHz");
    CHECK (d.params[416].id == "one.p3.vco3.beatHz");
    CHECK (d.indexOf ("one.master.voices") >= 417);
    CHECK (d.find ("one.master.voices") != nullptr);
    if (d.find ("one.master.voices") == nullptr) return;
    CHECK (d.find ("one.master.voices")->def == 0);
    f.layer();
    for (int note = 48; note < 68; ++note)
    {
        f.on (note); CHECK (f.engine.activeVoiceCount() <= 8);
    }
    CHECK (f.engine.activeVoiceCount() == 8); f.render();
    f.engine.reset(); f.set ("one.master.voices", 1); f.apply();
    for (int note = 48; note < 68; ++note)
    {
        f.on (note); CHECK (f.engine.activeVoiceCount() <= 16);
    }
    CHECK (f.engine.activeVoiceCount() == 16); f.render();
    f.set ("one.master.voices", 0); f.apply();
    CHECK (f.engine.activeVoiceCount() == 8);
    for (int note = 48; note < 68; ++note) f.off (note);
    f.render (12000); CHECK (f.engine.activeVoiceCount() == 0);
}

void monoAndUnison()
{
    Fixture f;
    f.cleanPitch(); f.set ("one.p1.voice.mode", 1); f.apply();
    f.on (60); f.on (72); CHECK (f.engine.activeVoiceCount() == 1);
    f.render(); const auto high = f.render();
    f.off (72); CHECK (f.engine.activeVoiceCount() == 1);
    f.render(); const auto low = f.render();
    CHECK (high > low * 1.7f); CHECK (low > 20 && low < 32);
    f.off (60); f.render (12000); CHECK (f.engine.activeVoiceCount() == 0);

    f.set ("one.p1.voice.mode", 2); f.set ("one.p1.voice.unisonVoices", 4); f.apply();
    f.on (60); CHECK (f.engine.activeVoiceCount() == 4);
    f.on (64); CHECK (f.engine.activeVoiceCount() == 4);
    f.off (64); CHECK (f.engine.activeVoiceCount() == 4); f.render();
    f.off (60); f.render (12000); CHECK (f.engine.activeVoiceCount() == 0);
    f.set ("one.p1.voice.unisonVoices", 16); f.apply();
    f.on (60); CHECK (f.engine.activeVoiceCount() == 8); f.render();
    f.set ("one.master.voices", 1); f.set ("one.p1.voice.unisonVoices", 15); f.apply();
    CHECK (f.engine.activeVoiceCount() == 15); f.render();
    f.off (60); f.render (12000); CHECK (f.engine.activeVoiceCount() == 0);
}

void sustainAndModeSwitches()
{
    Fixture f;
    f.set ("one.p1.voice.mode", 1); f.apply();
    f.on (60); f.on (64); f.pedal (true); f.off (64); f.render();
    CHECK (f.engine.activeVoiceCount() == 1);
    f.pedal (false); CHECK (f.engine.activeVoiceCount() == 1);
    f.off (60); f.render (12000); CHECK (f.engine.activeVoiceCount() == 0);
    // Pedal state belongs to its MIDI channel, including CC64's code path.
    f.set ("one.p1.voice.mode", 0); f.apply();
    f.pedal (true); f.on (60, 0); f.on (67, 1); f.off (60, 0); f.off (67, 1);
    f.render (12000); CHECK (f.engine.activeVoiceCount() == 1);
    f.engine.processMidi ({ triad::MidiEventType::ControlChange, 0, 0, 64, 0 });
    f.render (12000); CHECK (f.engine.activeVoiceCount() == 0);

    f.on (60); f.on (64); f.on (67); CHECK (f.engine.activeVoiceCount() == 3);
    f.set ("one.p1.voice.mode", 1); f.apply(); CHECK (f.engine.activeVoiceCount() == 1);
    f.set ("one.p1.voice.mode", 2); f.set ("one.p1.voice.unisonVoices", 4); f.apply();
    CHECK (f.engine.activeVoiceCount() == 4); f.render();
    f.set ("one.p1.voice.mode", 0); f.apply(); CHECK (f.engine.activeVoiceCount() == 3);
    f.off (60); f.off (64); f.off (67); f.render (12000); CHECK (f.engine.activeVoiceCount() == 0);
    f.layer();
    for (int part = 1; part <= 3; ++part)
    {
        f.set ("one.p" + std::to_string (part) + ".voice.mode", 2);
        f.set ("one.p" + std::to_string (part) + ".voice.unisonVoices", 16);
    }
    f.apply();
    for (int note = 48; note < 68; ++note)
    {
        f.on (note); CHECK (f.engine.activeVoiceCount() <= 8); f.render (256);
    }
    f.engine.processMidi ({ triad::MidiEventType::AllNotesOff });
    f.render (12000); CHECK (f.engine.activeVoiceCount() == 0);
}
}

int main()
{
    sharedBudget();
    if (failures != 0) return 1;
    monoAndUnison(); sustainAndModeSwitches();
    std::printf ("One voice checks: %d failure(s).\n", failures);
    return failures == 0 ? 0 : 1;
}
