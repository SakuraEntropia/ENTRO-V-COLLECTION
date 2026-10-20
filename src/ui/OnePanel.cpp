#include "ui/Panels.h"
#include "ui/LookAndFeel.h"

namespace triad::ui
{

namespace
{
juce::Colour oneAccent() { return juce::Colour (0xffd9a441); }
}

OnePanel::OnePanel (EditorHost& host)
    : PanelBase (host, "ONE V", 2240, 760, oneAccent())
{
    setBranding ("ONE V", "POLYPHONIC ANALOG SYNTHESIZER");
    rebuildForPart();
}

void OnePanel::rebuildForPart()
{
    clearControls();

    const auto accent = oneAccent();
    const auto part = juce::jlimit (1, 3, host().selectedPart());
    const auto prefix = "one.p" + juce::String (part) + ".";

    // Three hardware panels: left (oscillators, LFOs, arpeggiator), centre
    // console, right (mixer, filters, envelopes, matrix, effects).
    const juce::Rectangle<int> vco1Area { 8, 8, 150, 350 };
    const juce::Rectangle<int> vco2Area { 162, 8, 150, 350 };
    const juce::Rectangle<int> vco3Area { 316, 8, 150, 350 };

    const juce::Rectangle<int> lfo1Area { 8, 362, 150, 350 };
    const juce::Rectangle<int> lfo2Area { 162, 362, 150, 350 };
    const juce::Rectangle<int> lfo3Area { 316, 362, 150, 350 };
    const juce::Rectangle<int> lfo4Area { 470, 362, 150, 350 };
    const juce::Rectangle<int> arpArea { 624, 362, 146, 350 };

    const juce::Rectangle<int> centerArea { 778, 8, 300, 704 };
    const juce::Rectangle<int> mixerArea { 1086, 8, 120, 704 };
    const juce::Rectangle<int> svfArea { 1210, 8, 160, 704 };
    const juce::Rectangle<int> ladderArea { 1374, 8, 150, 704 };
    const juce::Rectangle<int> envArea { 1528, 8, 230, 704 };
    const juce::Rectangle<int> modArea { 1762, 8, 220, 704 };
    const juce::Rectangle<int> fxArea { 1986, 8, 246, 704 };

    addModule ("VCO 1", vco1Area);
    addModule ("VCO 2", vco2Area);
    addModule ("VCO 3", vco3Area);
    addModule ("LFO 1", lfo1Area);
    addModule ("LFO 2", lfo2Area);
    addModule ("LFO 3", lfo3Area);
    addModule ("LFO 4", lfo4Area);
    addModule ("ARP", arpArea);
    addModule ("CENTER CONSOLE", centerArea);
    addModule ("MIXER", mixerArea);
    addModule ("SVF", svfArea);
    addModule ("LADDER FILTER", ladderArea);
    addModule ("ENVELOPES", envArea);
    addModule ("MOD MATRIX", modArea);
    addModule ("EFFECTS", fxArea);

    // -----------------------------------------------------------------------
    // Oscillators
    // -----------------------------------------------------------------------
    auto buildOscillator = [&] (int osc, juce::Rectangle<int> area)
    {
        const auto x = area.getX() + 6;
        const auto width = area.getWidth() - 12;
        const auto cellWidth = width / 2;
        const auto stem = prefix + "vco" + juce::String (osc) + ".";
        auto y = area.getY() + 30;

        if (osc == 1)
        {
            auto& on = addLed (stem + "on", "ON", { x, y, width, 20 });
            on.setAccent (accent);
            y += 24;
        }

        addSwitch (stem + "octave", "OCTAVE", { x, y, width, 32 }); y += 36;

        addKnob (stem + "tune", "TUNE", { x, y, cellWidth, 46 });
        addKnob (stem + "waveAngle", "ANGLE", { x + cellWidth, y, cellWidth, 46 }); y += 48;
        addKnob (stem + "shape", "SHAPE", { x, y, cellWidth, 46 });
        addKnob (stem + "pw", "PULSE W", { x + cellWidth, y, cellWidth, 46 }); y += 48;
        addKnob (stem + "level", "LEVEL", { x, y, cellWidth, 46 });

        if (osc > 1)
        {
            auto& sync = addLed (stem + "sync", "SYNC", { x + cellWidth, y + 12, cellWidth, 20 });
            sync.setAccent (accent);
        }

        y += 48;

        if (osc == 3)
            addKnob (stem + "fm", "3 > 1 FM", { x, y, width, 46 });
    };

    buildOscillator (1, vco1Area);
    buildOscillator (2, vco2Area);
    buildOscillator (3, vco3Area);

    // -----------------------------------------------------------------------
    // LFOs
    // -----------------------------------------------------------------------
    auto buildLfo = [&] (int lfo, juce::Rectangle<int> area,
                         const juce::String& amountId, const juce::String& amountLabel)
    {
        const auto x = area.getX() + 6;
        const auto width = area.getWidth() - 12;
        const auto stem = prefix + "lfo" + juce::String (lfo) + ".";
        auto y = area.getY() + 28;

        addKnob (stem + "rate", "RATE", { x, y, width, 50 }); y += 54;
        addChoice (stem + "wave", "WAVE", { x, y, width, 32 }); y += 36;
        auto& sync = addLed (stem + "sync", "SYNC", { x, y, width, 20 });
        sync.setAccent (accent);
        y += 24;
        auto& retrig = addLed (stem + "retrigger", "RETRIG", { x, y, width, 20 });
        retrig.setAccent (accent);
        y += 24;
        addKnob (stem + "fade", "FADE", { x, y, width, 48 }); y += 52;

        if (amountId.isNotEmpty())
            addKnob (amountId, amountLabel, { x, y, width, 48 });
    };

    buildLfo (1, lfo1Area, prefix + "lfo1.pitchAmt", "PITCH AMT");
    buildLfo (2, lfo2Area, prefix + "lfo2.pwmAmt", "PWM AMT");
    buildLfo (3, lfo3Area, prefix + "lfo3.cutoffAmt", "CUTOFF AMT");
    buildLfo (4, lfo4Area, {}, {});

    // -----------------------------------------------------------------------
    // Arpeggiator
    // -----------------------------------------------------------------------
    {
        const auto x = arpArea.getX() + 6;
        const auto width = arpArea.getWidth() - 12;
        auto y = arpArea.getY() + 30;

        auto& enabled = addLed (prefix + "arp.enabled", "ARP", { x, y, width, 22 });
        enabled.setAccent (accent);
        y += 30;
        addChoice (prefix + "arp.mode", "MODE", { x, y, width, 38 }); y += 44;
        addChoice (prefix + "arp.rate", "RATE", { x, y, width, 38 }); y += 44;
        addSwitch (prefix + "arp.octaves", "OCTAVES", { x, y, width, 32 }); y += 38;
        auto& hold = addLed (prefix + "arp.hold", "HOLD", { x, y, width, 20 });
        hold.setAccent (accent);
        y += 28;
        addChoice (prefix + "arp.length", "LENGTH", { x, y, width, 38 });
    }

    // -----------------------------------------------------------------------
    // Centre console
    // -----------------------------------------------------------------------
    {
        const auto x = centerArea.getX() + 10;
        const auto width = centerArea.getWidth() - 20;

        for (int i = 0; i < 3; ++i)
        {
            auto& button = addButton ("PART " + juce::String (i + 1),
                                      { x + i * 94, 34, 88, 26 },
                                      [this, i]
                                      {
                                          host().setSelectedPart (i + 1);
                                          juce::Component::SafePointer<OnePanel> safe (this);
                                          juce::MessageManager::callAsync ([safe]
                                          {
                                              if (safe != nullptr)
                                                  safe->rebuildForPart();
                                          });
                                      });

            if (i + 1 == part)
                button.setToggleState (true, juce::dontSendNotification);
        }

        auto& lcd = addLcd ({ x, 70, width, 78 });
        lcd.setText ("PART " + juce::String (part),
                     "TIMBRE " + juce::String (part) + "  /  MIDI CH "
                         + juce::String (part),
                     "SHARED 8-VOICE POOL");

        const auto leftX = x;
        const auto rightX = x + 146;
        const auto columnWidth = 136;
        auto leftY = 162;
        auto rightY = 162;

        auto& enabled = addLed (prefix + "enabled", "PART ENABLE",
                                { leftX, leftY, columnWidth, 22 });
        enabled.setAccent (accent);
        leftY += 30;
        addKnob (prefix + "level", "LEVEL", { leftX, leftY, columnWidth, 56 }); leftY += 60;
        addKnob (prefix + "pan", "PAN", { leftX, leftY, columnWidth, 56 }); leftY += 60;
        addChoice (prefix + "midiChannel", "MIDI CHANNEL",
                   { leftX, leftY, columnWidth, 38 }); leftY += 44;
        addKnob (prefix + "keyboardLow", "KEY LOW", { leftX, leftY, columnWidth, 56 }); leftY += 60;
        addKnob (prefix + "keyboardHigh", "KEY HIGH", { leftX, leftY, columnWidth, 56 });

        addKnob ("one.master.volume", "MASTER", { rightX, rightY, columnWidth, 56 }); rightY += 60;
        addKnob ("one.master.reverbSize", "REV SIZE", { rightX, rightY, columnWidth, 56 }); rightY += 60;
        addKnob ("one.master.reverbDamping", "REV DAMP", { rightX, rightY, columnWidth, 56 }); rightY += 60;
        addKnob ("one.master.reverbMix", "REV MIX", { rightX, rightY, columnWidth, 56 }); rightY += 60;
        addKnob ("one.master.drift", "DRIFT", { rightX, rightY, columnWidth, 56 });

        const auto bottomY = 560;
        const auto smallWidth = 66;
        addKnob ("one.master.glide", "GLIDE", { x, bottomY, smallWidth, 54 });
        addKnob ("one.master.aftertouch", "AFTERTOUCH", { x + 70, bottomY, smallWidth, 54 });
        addKnob ("one.master.pitchBendRange", "BEND", { x + 140, bottomY, smallWidth, 54 });
        addCaption ("SHARED 8-VOICE POOL", { x + 208, bottomY + 12, 78, 24 },
                    juce::Justification::centred, 7.0f, juce::Colour (0xffd0ccc3));
    }

    // -----------------------------------------------------------------------
    // Mixer
    // -----------------------------------------------------------------------
    {
        const auto x = mixerArea.getX() + 6;
        const auto width = mixerArea.getWidth() - 12;
        auto y = 34;

        addKnob (prefix + "mix.noise", "NOISE", { x, y, width, 58 }); y += 64;
        addSwitch (prefix + "mix.noiseColor", "COLOR", { x, y, width, 40 }); y += 48;
        addKnob (prefix + "mix.ring", "RING MOD", { x, y, width, 58 }); y += 64;
        addKnob (prefix + "mix.ext", "EXTERNAL", { x, y, width, 58 });
    }

    // -----------------------------------------------------------------------
    // Filters
    // -----------------------------------------------------------------------
    {
        const auto x = svfArea.getX() + 6;
        const auto width = svfArea.getWidth() - 12;
        auto y = 34;

        addKnob (prefix + "svf.cutoff", "CUTOFF", { x, y, width, 54 }); y += 58;
        addKnob (prefix + "svf.resonance", "RESO", { x, y, width, 54 }); y += 58;
        addChoice (prefix + "svf.mode", "MODE", { x, y, width, 36 }); y += 42;
        addChoice (prefix + "svf.slope", "SLOPE", { x, y, width, 36 }); y += 42;
        addKnob (prefix + "svf.envAmt", "ENV AMT", { x, y, width, 54 }); y += 58;
        addKnob (prefix + "svf.keyTrack", "KEY TRK", { x, y, width, 54 }); y += 58;
        addKnob (prefix + "svf.level", "LEVEL", { x, y, width, 54 }); y += 58;
        addChoice (prefix + "filters.routing", "ROUTING", { x, y, width, 36 }); y += 42;
        addKnob (prefix + "filters.mix", "FILTER", { x, y, width, 54 });
    }

    {
        const auto x = ladderArea.getX() + 6;
        const auto width = ladderArea.getWidth() - 12;
        auto y = 34;

        addKnob (prefix + "ladder.cutoff", "CUTOFF", { x, y, width, 54 }); y += 58;
        addKnob (prefix + "ladder.resonance", "RESO", { x, y, width, 54 }); y += 58;
        addKnob (prefix + "ladder.drive", "DRIVE", { x, y, width, 54 }); y += 58;
        addChoice (prefix + "ladder.slope", "SLOPE", { x, y, width, 36 }); y += 42;
        addKnob (prefix + "ladder.envAmt", "ENV AMT", { x, y, width, 54 }); y += 58;
        addKnob (prefix + "ladder.keyTrack", "KEY TRK", { x, y, width, 54 });
    }

    // -----------------------------------------------------------------------
    // Envelopes
    // -----------------------------------------------------------------------
    {
        const auto columnWidth = (envArea.getWidth() - 20) / 3;

        for (int env = 1; env <= 3; ++env)
        {
            const auto x = envArea.getX() + 6 + (env - 1) * (columnWidth + 4);
            const auto stem = prefix + "env" + juce::String (env) + ".";

            addCaption ("ENV " + juce::String (env), { x, 30, columnWidth, 10 },
                        juce::Justification::centred, 7.5f, juce::Colour (0xffe4e0d7), true);

            auto y = envArea.getY() + 44;
            addKnob (stem + "delay", "DELAY", { x, y, columnWidth, 48 }); y += 52;
            addKnob (stem + "attack", "ATTACK", { x, y, columnWidth, 48 }); y += 52;
            addKnob (stem + "hold", "HOLD", { x, y, columnWidth, 48 }); y += 52;
            addKnob (stem + "decay", "DECAY", { x, y, columnWidth, 48 }); y += 52;
            addKnob (stem + "sustain", "SUSTAIN", { x, y, columnWidth, 48 }); y += 52;
            addKnob (stem + "release", "RELEASE", { x, y, columnWidth, 48 }); y += 52;
            addKnob (stem + "velocity", "VELOCITY", { x, y, columnWidth, 48 });

            if (env < 3)
            {
                auto& loop = addLed (stem + "loop", "LOOP",
                                     { x, envArea.getY() + 438, columnWidth, 20 });
                loop.setAccent (accent);
            }
        }
    }

    // -----------------------------------------------------------------------
    // Mod matrix
    // -----------------------------------------------------------------------
    {
        const auto x = modArea.getX() + 6;
        const auto width = modArea.getWidth() - 12;

        for (int slot = 1; slot <= 8; ++slot)
        {
            const auto y = 34 + (slot - 1) * 76;
            const auto stem = prefix + "mod.slot" + juce::String (slot) + ".";

            addCaption (juce::String (slot), { x, y + 4, 12, 10 },
                        juce::Justification::centred, 7.0f, juce::Colour (0xff8f8c85));
            addChoice (stem + "source", "", { x + 14, y, 82, 30 });
            addChoice (stem + "dest", "", { x + 98, y, 82, 30 });
            addKnob (stem + "amount", "AMT", { x + 182, y - 10, 42, 50 });
        }
    }

    // -----------------------------------------------------------------------
    // Effects
    // -----------------------------------------------------------------------
    {
        const auto columnWidth = (fxArea.getWidth() - 16) / 2;
        const auto x1 = fxArea.getX() + 6;
        const auto x2 = x1 + columnWidth + 4;

        addCaption ("DIST", { x1, 30, columnWidth, 10 }, juce::Justification::centred,
                    7.0f, juce::Colour (0xffe4e0d7), true);
        addChoice (prefix + "fx.dist.type", "", { x1, 42, columnWidth - 2, 30 });
        addKnob (prefix + "fx.dist.drive", "DRIVE", { x1, 76, columnWidth, 50 });
        addKnob (prefix + "fx.dist.mix", "MIX", { x1, 128, columnWidth, 50 });

        addCaption ("CHORUS", { x2, 30, columnWidth, 10 }, juce::Justification::centred,
                    7.0f, juce::Colour (0xffe4e0d7), true);
        addKnob (prefix + "fx.chorus.rate", "RATE", { x2, 42, columnWidth, 50 });
        addKnob (prefix + "fx.chorus.depth", "DEPTH", { x2, 94, columnWidth, 50 });
        addKnob (prefix + "fx.chorus.mix", "MIX", { x2, 146, columnWidth, 50 });

        addCaption ("DELAY", { x1, 196, columnWidth, 10 }, juce::Justification::centred,
                    7.0f, juce::Colour (0xffe4e0d7), true);
        addKnob (prefix + "fx.delay.time", "TIME", { x1, 208, columnWidth, 50 });
        addKnob (prefix + "fx.delay.feedback", "FEEDBACK", { x1, 260, columnWidth, 50 });
        addKnob (prefix + "fx.delay.mix", "MIX", { x1, 312, columnWidth, 50 });

        addCaption ("MASTER SEND", { x2, 196, columnWidth, 10 }, juce::Justification::centred,
                    7.0f, juce::Colour (0xffe4e0d7), true);
        addKnob (prefix + "fx.reverb.send", "REV SEND", { x2, 208, columnWidth, 50 });
    }
}

} // namespace triad::ui
