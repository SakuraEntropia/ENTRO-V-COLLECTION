#include "ui/Panels.h"

namespace triad::ui
{
MatriarchPanel::MatriarchPanel (EditorHost& editorHost)
    : PanelBase (editorHost, "MATRIARCH V", 1900, 480, juce::Colour (0xffd44529))
{
    setBranding ("MATRIARCH V", "SEMI-MODULAR ANALOG SYNTHESIZER");

    // Relative widths from the two front-panel line drawings in the manual.
    // The software-only expression controls live below the hardware stations.
    const juce::Rectangle<int> arp { 24, 14, 150, 402 }, mod { 180, 14, 160, 402 },
        util { 346, 14, 80, 402 }, mix { 808, 14, 160, 402 },
        filter { 974, 14, 210, 402 }, util2 { 1190, 14, 84, 402 },
        env { 1280, 14, 300, 402 }, delay { 1586, 14, 166, 402 },
        out { 1758, 14, 118, 402 };
    addModule ("ARP / SEQ", arp);
    addModule ("MODULATION", mod);
    addModule ("UTILITIES", util);
    addModule ("OSCILLATORS", { 432, 14, 370, 402 });
    addModule ("MIXER", mix);
    addModule ("FILTERS", filter);
    addModule ("UTILITIES", util2);
    addModule ("ENVELOPE GENERATORS", env);
    addModule ("STEREO DELAY", delay);
    addModule ("OUTPUT", out);

    auto patch = [this] (const char* id, int x, int y) { addJack (id, { x, y }, 18); };
    auto bay = [this] (const std::vector<juce::String>& ids, juce::Rectangle<int> area, int columns)
    {
        for (int i = 0; i < (int) ids.size(); ++i)
            addJack (ids[(size_t) i], { area.getX() + (i % columns * 2 + 1) * area.getWidth() / (columns * 2),
                                       48 + (i / columns) * 34 }, 18);
    };
    auto knob = [this] (const juce::String& id, const char* name, int x, int y, int d = 48)
    { addKnob (id, name, { x, y }, d); };

    bay ({ "mat.patch.clock.in", "mat.patch.clock.out", "mat.patch.clock.step1",
           "mat.patch.clock.step2", "mat.patch.clock.resetIn", "mat.patch.clock.resetOut" }, arp, 3);
    knob ("mat.arp.rate", "RATE / DIV", 99, 150);
    knob ("mat.arp.mode", "DIRECTION", 99, 218, 34);
    addSwitch ("mat.arp.octaves", "OCT / BANK", { 46, 255, 106, 38 });
    addLed ("mat.arp.enabled", "PLAY", { 40, 357, 58, 28 });
    addLed ("mat.arp.hold", "HOLD", { 104, 357, 58, 28 });

    // Staggered jacks and two-column modulation controls match the panel stations.
    patch ("mat.patch.lfo.rateIn", 206, 48); patch ("mat.patch.noise.out", 292, 48);
    patch ("mat.patch.lfo.syncIn", 200, 82); patch ("mat.patch.sh.out", 260, 82);
    patch ("mat.patch.lfo.out", 320, 82);
    knob ("mat.lfo.rate", "RATE", 224, 150);
    knob ("mat.lfo.wave", "WAVEFORM", 224, 254);
    knob ("mat.lfo.pitchAmt", "PITCH AMT", 298, 254);
    addSwitch ("mat.lfo.pitchAssign", "PITCH MOD ASSIGN", { 254, 293, 78, 34 });
    knob ("mat.lfo.cutoffAmt", "CUTOFF AMT", 220, 366);
    knob ("mat.lfo.pwAmt", "PULSE WIDTH AMT", 300, 366);

    bay ({ "mat.patch.mult1.a", "mat.patch.mult1.b", "mat.patch.mult1.c", "mat.patch.mult1.d" }, util, 2);
    knob ("mat.att1", "ATTENUATOR", 386, 150);
    patch ("mat.patch.att1.in", 366, 203); patch ("mat.patch.att1.out", 406, 203);
    knob ("mat.att2", "ATTENUATOR", 386, 302);
    patch ("mat.patch.att2.in", 366, 357); patch ("mat.patch.att2.out", 406, 357);

    for (int i = 1; i <= 4; ++i)
    {
        const auto x = 432 + (i - 1) * 94;
        const auto id = "mat.vco" + juce::String (i) + ".";
        const auto p = "mat.patch.vco" + juce::String (i) + ".";
        addCaption (juce::String (i), { x + 3, 33, 12, 12 }, juce::Justification::centred, 10.0f);
        bay ({ p + "pitch", p + "out", p + "pwm", p + "fm" }, { x, 0, 88, 0 }, 2);
        knob (id + "octave", "OCTAVE", x + 44, 150, 46);
        if (i > 1)
        {
            knob (id + "freq", "FREQUENCY", x + 44, 254);
            addLed (id + "sync", "SYNC", { x + 13, 293, 62, 22 });
        }
        else
            addLed ("mat.vco.syncEnable", "SYNC ENABLE", { x + 8, 242, 72, 28 });
        knob (id + "wave", "WAVEFORM", x + 44, 366, 46);
    }

    bay ({ "mat.patch.mix.noiseIn", "mat.patch.mix.osc1In", "mat.patch.mix.osc2In",
           "mat.patch.mix.osc3In", "mat.patch.mix.osc4In", "mat.patch.mix.out" }, mix, 3);
    knob ("mat.mix.noise", "NOISE", 888, 150);
    knob ("mat.mix.osc1", "OSCILLATOR 1", 849, 254);
    knob ("mat.mix.osc2", "OSCILLATOR 2", 927, 254);
    knob ("mat.mix.osc3", "OSCILLATOR 3", 849, 366);
    knob ("mat.mix.osc4", "OSCILLATOR 4", 927, 366);

    bay ({ "mat.patch.flt1.in", "mat.patch.flt2.in", "mat.patch.flt1.out", "mat.patch.flt2.out" }, filter, 4);
    patch ("mat.patch.flt1.cutoff", 1014, 82); patch ("mat.patch.flt2.cutoff", 1079, 82);
    patch ("mat.patch.flt.envAmt", 1144, 82);
    knob ("mat.flt.cutoff", "CUTOFF", 1079, 158, 72);
    knob ("mat.flt.res1", "RESONANCE 1", 1014, 225);
    knob ("mat.flt.res2", "RESONANCE 2", 1144, 225);
    addSwitch ("mat.flt.mode", "FILTER MODE", { 1019, 262, 120, 34 });
    knob ("mat.flt.spacing", "SPACING", 1014, 324);
    knob ("mat.flt.envAmt", "ENVELOPE AMT", 1144, 324);
    knob ("mat.flt.kbTrack", "KB TRACKING", 1079, 380, 40);

    bay ({ "mat.patch.mult2.a", "mat.patch.mult2.b", "mat.patch.mult2.c", "mat.patch.mult2.d" }, util2, 2);
    knob ("mat.att3", "ATTENUATOR", 1232, 150);
    patch ("mat.patch.att3.in", 1211, 203); patch ("mat.patch.att3.out", 1253, 203);
    knob ("mat.lfo2.rate", "LFO RATE", 1232, 302);
    patch ("mat.patch.lfo2.rateIn", 1211, 357); patch ("mat.patch.lfo2.out", 1253, 357);
    addLed ("mat.lfo2.bipolar", "BIPOLAR", { 1198, 386, 68, 20 });

    // Each envelope: three knobs on the left, a long sustain slider on the right.
    for (int i = 0; i < 2; ++i)
    {
        const auto x = 1280 + i * 150;
        const auto stem = i == 0 ? juce::String ("mat.env2.") : juce::String ("mat.env1.");
        const auto p = i == 0 ? juce::String ("mat.patch.fltEnv.") : juce::String ("mat.patch.amp.");
        addCaption (i == 0 ? "FILTER" : "AMPLITUDE", { x + 20, 33, 110, 12 }, juce::Justification::centred, 10.0f);
        addJack (p + "trig", { x + 32, 56 }, 18); addJack (p + "out", { x + 119, 56 }, 18);
        addJack (p + "end", { x + 75, 88 }, 18);
        knob (stem + "attack", "ATTACK", x + 36, 150);
        knob (stem + "decay", "DECAY", x + 36, 254);
        knob (stem + "release", "RELEASE", x + 36, 366);
        addFader (stem + "sustain", "SUSTAIN", { x + 90, 135, 50, 248 });
    }

    bay ({ "mat.patch.delay.in1", "mat.patch.delay.in2", "mat.patch.delay.out1", "mat.patch.delay.out2",
           "mat.patch.delay.fbCv", "mat.patch.delay.time1Cv", "mat.patch.delay.time2Cv", "mat.patch.delay.mixCv" }, delay, 4);
    knob ("mat.delay.time", "TIME", 1669, 150);
    knob ("mat.delay.spacing", "SPACING", 1627, 247);
    knob ("mat.delay.feedback", "FEEDBACK", 1711, 247);
    knob ("mat.delay.mix", "MIX", 1669, 336);
    addLed ("mat.delay.sync", "SYNC / TAP", { 1595, 382, 74, 24 });
    addLed ("mat.delay.pingPong", "PING PONG", { 1675, 382, 69, 24 });

    bay ({ "mat.patch.vca1.in", "mat.patch.vca2.in", "mat.patch.vca1.cv", "mat.patch.vca2.cv" }, out, 2);
    knob ("mat.master.volume", "MAIN VOLUME", 1817, 150);
    addSwitch ("mat.master.vcaMode", "VCA MODE", { 1772, 207, 90, 34 });
    addSwitch ("mat.master.paraphony", "PARAPHONY", { 1772, 268, 90, 34 });
    knob ("mat.output.velocity", "VELOCITY", 1788, 346, 36);
    knob ("mat.output.aftertouch", "PRESSURE", 1844, 346, 36);
    knob ("mat.global.fine", "FINE TUNE", 1817, 391, 28);
    // The five keyboard outputs already exist in the engine patch map; expose
    // them on the lower rail so all 79 patch points can actually be connected.
    addCaption ("KEYBOARD OUT", { 1190, 437, 120, 20 }, juce::Justification::centred, 10.0f);
    const char* keyboardOutputs[] = { "pitch", "gate", "velocity", "aftertouch", "mod" };
    for (int i = 0; i < 5; ++i)
        addJack ("mat.patch.kb." + juce::String (keyboardOutputs[i]), { 1340 + i * 75, 445 }, 18);
    onPatchChanged = [this] { refreshPatchCables(); };
}
} // namespace triad::ui
