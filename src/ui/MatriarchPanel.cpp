#include "ui/Panels.h"
#include "ui/LookAndFeel.h"

#include <cmath>

namespace triad::ui
{

namespace
{
juce::Colour matriarchAccent() { return juce::Colour (0xffcf4b2e); }

int rowsFor (int count, int columns)
{
    return (count + columns - 1) / columns;
}
} // namespace

MatriarchPanel::MatriarchPanel (EditorHost& host)
    : PanelBase (host, "Matriarch", 1800, 500, matriarchAccent())
{
    setBranding ("MATRIARCH", "SEMI-MODULAR ANALOG SYNTHESIZER");

    const auto accent = matriarchAccent();

    const juce::Rectangle<int> arpArea { 8, 8, 142, 440 };
    const juce::Rectangle<int> modArea { 154, 8, 120, 440 };
    const juce::Rectangle<int> utilArea { 278, 8, 96, 440 };
    const juce::Rectangle<int> vco1Area { 378, 8, 112, 440 };
    const juce::Rectangle<int> vco2Area { 494, 8, 112, 440 };
    const juce::Rectangle<int> vco3Area { 610, 8, 112, 440 };
    const juce::Rectangle<int> vco4Area { 726, 8, 112, 440 };
    const juce::Rectangle<int> mixArea { 842, 8, 124, 440 };
    const juce::Rectangle<int> filterArea { 970, 8, 196, 440 };
    const juce::Rectangle<int> util2Area { 1170, 8, 108, 440 };
    const juce::Rectangle<int> envArea { 1282, 8, 236, 440 };
    const juce::Rectangle<int> delayArea { 1522, 8, 146, 440 };
    const juce::Rectangle<int> outArea { 1672, 8, 118, 440 };

    addModule ("ARP / SEQ", arpArea);
    addModule ("MODULATION", modArea);
    addModule ("UTILITIES", utilArea);
    addModule ("OSCILLATOR 1", vco1Area);
    addModule ("OSCILLATOR 2", vco2Area);
    addModule ("OSCILLATOR 3", vco3Area);
    addModule ("OSCILLATOR 4", vco4Area);
    addModule ("MIXER", mixArea);
    addModule ("FILTERS", filterArea);
    addModule ("UTILITIES 2", util2Area);
    addModule ("ENVELOPE GENERATORS", envArea);
    addModule ("STEREO DELAY", delayArea);
    addModule ("OUTPUT", outArea);

    // Place jacks in a grid below each module header. Returns the y after the grid.
    auto jackGrid = [this] (const std::vector<juce::String>& ids,
                            juce::Rectangle<int> module, int columns, int startY)
    {
        const auto count = (int) ids.size();
        const auto colWidth = module.getWidth() / columns;
        int index = 0;

        for (const auto& id : ids)
        {
            const auto column = index % columns;
            const auto row = index / columns;
            const auto centreX = module.getX() + column * colWidth + colWidth / 2;
            const auto topY = startY + row * 27;

            addJack (id, { centreX - 8, topY, 16, 16 });
            ++index;
        }

        return startY + rowsFor (count, columns) * 27 + 4;
    };

    // -----------------------------------------------------------------------
    // ARP / SEQ
    // -----------------------------------------------------------------------
    {
        const auto y = jackGrid ({ "mat.patch.clock.in", "mat.patch.clock.out",
                                   "mat.patch.clock.step1", "mat.patch.clock.step2",
                                   "mat.patch.clock.resetIn", "mat.patch.clock.resetOut" },
                                 arpArea, 3, 32);

        auto& arp = addLed ("mat.arp.enabled", "ARP / SEQ",
                            { arpArea.getX() + 8, y + 4, arpArea.getWidth() - 16, 24 });
        arp.setAccent (accent);

        addChoice ("mat.arp.mode", "MODE", { arpArea.getX() + 8, y + 34,
                                            arpArea.getWidth() - 16, 34 });
        addChoice ("mat.arp.rate", "RATE", { arpArea.getX() + 8, y + 74,
                                            arpArea.getWidth() - 16, 34 });
        addSwitch ("mat.arp.octaves", "OCTAVES", { arpArea.getX() + 8, y + 114,
                                                   arpArea.getWidth() - 16, 34 });
        auto& hold = addLed ("mat.arp.hold", "HOLD",
                             { arpArea.getX() + 8, y + 156, arpArea.getWidth() - 16, 24 });
        hold.setAccent (accent);
    }

    // -----------------------------------------------------------------------
    // Modulation
    // -----------------------------------------------------------------------
    {
        const auto y = jackGrid ({ "mat.patch.lfo.rateIn", "mat.patch.lfo.syncIn",
                                   "mat.patch.lfo.out", "mat.patch.noise.out",
                                   "mat.patch.sh.out" },
                                 modArea, 2, 32);

        addKnob ("mat.lfo.rate", "RATE", { modArea.getX() + 6, y + 2, modArea.getWidth() - 12, 56 });
        addSwitch ("mat.lfo.wave", "WAVEFORM", { modArea.getX() + 6, y + 60,
                                                 modArea.getWidth() - 12, 36 });
        addKnob ("mat.lfo.pitchAmt", "PITCH AMT",
                 { modArea.getX() + 6, y + 100, modArea.getWidth() - 12, 56 });
        addSwitch ("mat.lfo.pitchAssign", "PITCH MOD ASSIGN",
                   { modArea.getX() + 6, y + 158, modArea.getWidth() - 12, 36 });
        addKnob ("mat.lfo.cutoffAmt", "CUTOFF AMT",
                 { modArea.getX() + 6, y + 198, modArea.getWidth() - 12, 56 });
        addKnob ("mat.lfo.pwAmt", "PULSE WIDTH AMT",
                 { modArea.getX() + 6, y + 256, modArea.getWidth() - 12, 56 });


    }

    // -----------------------------------------------------------------------
    // Utilities 1 (mults + two attenuators)
    // -----------------------------------------------------------------------
    {
        const auto y = jackGrid ({ "mat.patch.mult1.a", "mat.patch.mult1.b",
                                   "mat.patch.mult1.c", "mat.patch.mult1.d",
                                   "mat.patch.att1.in", "mat.patch.att1.out",
                                   "mat.patch.att2.in", "mat.patch.att2.out" },
                                 utilArea, 2, 32);

        addKnob ("mat.att1", "ATTENUATOR 1",
                 { utilArea.getX() + 4, y + 4, utilArea.getWidth() - 8, 58 });
        addKnob ("mat.att2", "ATTENUATOR 2",
                 { utilArea.getX() + 4, y + 66, utilArea.getWidth() - 8, 58 });
    }

    // -----------------------------------------------------------------------
    // Oscillators
    // -----------------------------------------------------------------------
    auto buildOscillator = [&] (int index, juce::Rectangle<int> area)
    {
        const auto prefix = "mat.vco" + juce::String (index) + ".";
        const auto patchPrefix = "mat.patch.vco" + juce::String (index) + ".";

        const auto y = jackGrid ({ patchPrefix + "pitch", patchPrefix + "fm",
                                   patchPrefix + "pwm", patchPrefix + "out" },
                                 area, 2, 32);

        addSwitch (prefix + "octave", "OCTAVE",
                   { area.getX() + 4, y + 2, area.getWidth() - 8, 40 });

        if (index > 1)
            addKnob (prefix + "freq", "FREQUENCY",
                     { area.getX() + 4, y + 46, area.getWidth() - 8, 56 });
        else
            addCaption ("FIXED", { area.getX() + 4, y + 60, area.getWidth() - 8, 12 },
                        juce::Justification::centred, 7.0f, juce::Colour (0xff6d6a64));

        addSwitch (prefix + "wave", "WAVEFORM",
                   { area.getX() + 4, y + 106, area.getWidth() - 8, 48 });

        if (index > 1)
        {
            auto& sync = addLed (prefix + "sync", "SYNC",
                                 { area.getX() + 10, y + 164, area.getWidth() - 20, 24 });
            sync.setAccent (accent);
        }
        else
        {
            auto& sync = addLed ("mat.vco.syncEnable", "SYNC ENABLE",
                                 { area.getX() + 8, y + 164, area.getWidth() - 16, 24 });
            sync.setAccent (accent);
        }
    };

    buildOscillator (1, vco1Area);
    buildOscillator (2, vco2Area);
    buildOscillator (3, vco3Area);
    buildOscillator (4, vco4Area);

    // -----------------------------------------------------------------------
    // Mixer
    // -----------------------------------------------------------------------
    {
        const auto y = jackGrid ({ "mat.patch.mix.noiseIn", "mat.patch.mix.osc1In",
                                   "mat.patch.mix.osc2In", "mat.patch.mix.osc3In",
                                   "mat.patch.mix.osc4In", "mat.patch.mix.out" },
                                 mixArea, 3, 32);

        const auto cellWidth = (mixArea.getWidth() - 16) / 2;
        addKnob ("mat.mix.osc1", "OSC 1", { mixArea.getX() + 6, y + 6, cellWidth, 58 });
        addKnob ("mat.mix.osc2", "OSC 2", { mixArea.getX() + 10 + cellWidth, y + 6, cellWidth, 58 });
        addKnob ("mat.mix.osc3", "OSC 3", { mixArea.getX() + 6, y + 68, cellWidth, 58 });
        addKnob ("mat.mix.osc4", "OSC 4", { mixArea.getX() + 10 + cellWidth, y + 68, cellWidth, 58 });
        addKnob ("mat.mix.noise", "NOISE", { mixArea.getX() + 6, y + 130, mixArea.getWidth() - 12, 58 });
    }

    // -----------------------------------------------------------------------
    // Filters
    // -----------------------------------------------------------------------
    {
        const auto y = jackGrid ({ "mat.patch.flt1.in", "mat.patch.flt2.in",
                                   "mat.patch.flt1.cutoff", "mat.patch.flt2.cutoff",
                                   "mat.patch.flt.envAmt", "mat.patch.flt1.out",
                                   "mat.patch.flt2.out" },
                                 filterArea, 4, 32);

        addChoice ("mat.flt.mode", "FILTER MODE",
                   { filterArea.getX() + 6, y + 2, filterArea.getWidth() - 12, 36 });

        const auto cellWidth = (filterArea.getWidth() - 16) / 2;
        const auto x1 = filterArea.getX() + 6;
        const auto x2 = x1 + cellWidth + 4;
        auto rowY = y + 44;

        addKnob ("mat.flt.cutoff", "CUTOFF", { x1, rowY, cellWidth, 58 });
        addKnob ("mat.flt.spacing", "SPACING", { x2, rowY, cellWidth, 58 });
        rowY += 62;
        addKnob ("mat.flt.res1", "RESONANCE 1", { x1, rowY, cellWidth, 58 });
        addKnob ("mat.flt.res2", "RESONANCE 2", { x2, rowY, cellWidth, 58 });
        rowY += 62;
        addKnob ("mat.flt.envAmt", "ENVELOPE AMT", { x1, rowY, cellWidth, 58 });
        addKnob ("mat.flt.kbTrack", "KB TRACKING", { x2, rowY, cellWidth, 58 });
    }

    // -----------------------------------------------------------------------
    // Utilities 2 (mult, LFO 2, attenuator 3)
    // -----------------------------------------------------------------------
    {
        const auto y = jackGrid ({ "mat.patch.mult2.a", "mat.patch.mult2.b",
                                   "mat.patch.mult2.c", "mat.patch.mult2.d",
                                   "mat.patch.lfo2.rateIn", "mat.patch.lfo2.out",
                                   "mat.patch.att3.in", "mat.patch.att3.out" },
                                 util2Area, 2, 32);

        addKnob ("mat.lfo2.rate", "LFO RATE",
                 { util2Area.getX() + 4, y + 4, util2Area.getWidth() - 8, 58 });
        addKnob ("mat.att3", "ATTENUATOR 3",
                 { util2Area.getX() + 4, y + 68, util2Area.getWidth() - 8, 58 });
        addSwitch ("mat.lfo2.bipolar", "BIPOLAR",
                   { util2Area.getX() + 4, y + 132, util2Area.getWidth() - 8, 40 });
    }

    // -----------------------------------------------------------------------
    // Envelope generators
    // -----------------------------------------------------------------------
    {
        const auto y = jackGrid ({ "mat.patch.amp.trig", "mat.patch.amp.out",
                                   "mat.patch.amp.end", "mat.patch.fltEnv.trig",
                                   "mat.patch.fltEnv.out", "mat.patch.fltEnv.end" },
                                 envArea, 3, 32);

        const auto columnWidth = (envArea.getWidth() - 20) / 2;
        const auto ampX = envArea.getX() + 8;
        const auto filterX = ampX + columnWidth + 4;

        addCaption ("AMPLITUDE", { ampX, y, columnWidth, 12 }, juce::Justification::centred,
                    8.0f, juce::Colour (0xffc7c3ba), true);
        addCaption ("FILTER", { filterX, y, columnWidth, 12 }, juce::Justification::centred,
                    8.0f, juce::Colour (0xffc7c3ba), true);

        auto buildEnvelope = [&] (const juce::String& prefix, int x)
        {
            auto top = y + 16;
            addKnob (prefix + "attack", "ATTACK", { x, top, columnWidth, 50 });
            addKnob (prefix + "decay", "DECAY", { x, top + 48, columnWidth, 50 });
            addFader (prefix + "sustain", "SUSTAIN", { x + columnWidth / 2 - 30, top + 96, 60, 76 });
            addKnob (prefix + "release", "RELEASE", { x, top + 176, columnWidth, 50 });
        };

        buildEnvelope ("mat.env1.", ampX);
        buildEnvelope ("mat.env2.", filterX);
    }

    // -----------------------------------------------------------------------
    // Stereo delay
    // -----------------------------------------------------------------------
    {
        const auto y = jackGrid ({ "mat.patch.delay.in1", "mat.patch.delay.in2",
                                   "mat.patch.delay.fbCv", "mat.patch.delay.mixCv",
                                   "mat.patch.delay.time1Cv", "mat.patch.delay.time2Cv",
                                   "mat.patch.delay.out1", "mat.patch.delay.out2" },
                                 delayArea, 2, 32);

        const auto x = delayArea.getX() + 8;
        const auto width = delayArea.getWidth() - 16;

        addKnob ("mat.delay.time", "TIME", { x, y + 2, width, 54 });
        addKnob ("mat.delay.spacing", "SPACING", { x, y + 58, width, 54 });
        addKnob ("mat.delay.feedback", "FEEDBACK", { x, y + 114, width, 54 });
        addKnob ("mat.delay.mix", "MIX", { x, y + 170, width, 54 });

        auto& sync = addLed ("mat.delay.sync", "SYNC / TAP",
                             { x, y + 230, width, 24 });
        sync.setAccent (juce::Colour (0xffd8b03c));

        auto& ping = addLed ("mat.delay.pingPong", "PING PONG",
                             { x, y + 258, width, 24 });
        ping.setAccent (juce::Colour (0xff3f8ed6));
    }

    // -----------------------------------------------------------------------
    // Output
    // -----------------------------------------------------------------------
    {
        const auto y = jackGrid ({ "mat.patch.vca1.in", "mat.patch.vca2.in",
                                   "mat.patch.vca1.cv", "mat.patch.vca2.cv" },
                                 outArea, 2, 32);

        const auto x = outArea.getX() + 6;
        const auto width = outArea.getWidth() - 12;

        addKnob ("mat.master.volume", "MAIN VOLUME", { x, y + 4, width, 54 });
        addKnob ("mat.output.velocity", "VELOCITY", { x, y + 62, width, 54 });
        addKnob ("mat.output.aftertouch", "AFTERTOUCH", { x, y + 120, width, 54 });
        addKnob ("mat.global.fine", "FINE TUNE", { x, y + 180, width, 54 });
        addSwitch ("mat.master.vcaMode", "VCA MODE", { x, y + 240, width, 42 }); 
        addSwitch ("mat.master.paraphony", "PARAPHONY", { x, y + 290, width, 42 });
    }

    onPatchChanged = [this] { refreshPatchCables(); };
}

} // namespace triad::ui
