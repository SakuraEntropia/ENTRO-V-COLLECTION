#include "ui/Panels.h"
#include "ui/LookAndFeel.h"

namespace triad::ui
{

namespace
{
juce::Colour polyAccent() { return juce::Colour (0xff3f93e8); }
}

PolyBrutePanel::PolyBrutePanel (EditorHost& editorHost)
    : PanelBase (editorHost, "PolyBrute", 1900, 960, polyAccent())
{
    setBranding ("POLYBRUTE", "SIX-VOICE ANALOG SYNTHESIZER");

    const auto accent = polyAccent();

    // Layout follows the hardware: performance at the far left, voice
    // management next, then a three-row synthesis block, matrix, effects and
    // arpeggiator.
    const juce::Rectangle<int> perfArea { 8, 8, 168, 852 };
    const juce::Rectangle<int> voiceArea { 184, 8, 150, 852 };

    const juce::Rectangle<int> vco1Area { 342, 8, 150, 268 };
    const juce::Rectangle<int> mixerArea { 500, 8, 120, 268 };
    const juce::Rectangle<int> steinerArea { 628, 8, 160, 268 };
    const juce::Rectangle<int> vcfEnvArea { 796, 8, 150, 268 };
    const juce::Rectangle<int> lfo1Area { 954, 8, 128, 268 };

    const juce::Rectangle<int> vco2Area { 342, 284, 150, 268 };
    const juce::Rectangle<int> morphArea { 500, 284, 120, 268 };
    const juce::Rectangle<int> ladderArea { 628, 284, 160, 268 };
    const juce::Rectangle<int> vcaEnvArea { 796, 284, 150, 268 };
    const juce::Rectangle<int> lfo2Area { 954, 284, 128, 268 };

    const juce::Rectangle<int> filterFmArea { 342, 560, 270, 268 };
    const juce::Rectangle<int> routingArea { 628, 560, 160, 268 };
    const juce::Rectangle<int> modEnvArea { 796, 560, 150, 268 };
    const juce::Rectangle<int> lfo3Area { 954, 560, 128, 268 };

    const juce::Rectangle<int> matrixArea { 1092, 8, 330, 852 };
    const juce::Rectangle<int> fxArea { 1430, 8, 230, 852 };
    const juce::Rectangle<int> arpArea { 1668, 8, 224, 852 };

    addModule ("PERFORMANCE", perfArea);
    addModule ("VOICE", voiceArea);
    addModule ("VCO 1", vco1Area);
    addModule ("MIXER", mixerArea);
    addModule ("STEINER FILTER", steinerArea);
    addModule ("VCF ENV", vcfEnvArea, ModuleTone::Light);
    addModule ("LFO 1", lfo1Area);
    addModule ("VCO 2", vco2Area);
    addModule ("MORPH", morphArea);
    addModule ("LADDER FILTER", ladderArea);
    addModule ("VCA ENV", vcaEnvArea, ModuleTone::Light);
    addModule ("LFO 2", lfo2Area);
    addModule ("FILTER FM / NOISE", filterFmArea);
    addModule ("ROUTING", routingArea);
    addModule ("MOD ENV", modEnvArea, ModuleTone::Light);
    addModule ("LFO 3", lfo3Area);
    addModule ("MATRIX", matrixArea, ModuleTone::Light);
    addModule ("EFFECTS", fxArea);
    addModule ("ARPEGGIATOR", arpArea);

    // -----------------------------------------------------------------------
    // Performance and voice management
    // -----------------------------------------------------------------------
    {
        const auto x = perfArea.getX() + 8;
        const auto width = perfArea.getWidth() - 16;
        auto y = 34;

        auto& lcd = addLcd ({ x, y, width, 84 });
        lcd.setText ("MORPH", "A / B", "CAPTURE OR SWAP");
        y += 96;

        addButton ("CAPTURE B", { x, y, width, 30 }, [this] { host().captureMorphB(); });
        y += 36;
        addButton ("SWAP A/B", { x, y, width, 30 }, [this] { host().swapMorphB(); });
        y += 46;

        addCaption ("THE B SIDE IS STORED IN PROJECT STATE.",
                    { x, y, width, 44 }, juce::Justification::topLeft, 7.0f,
                    juce::Colour (0xffd0ccc3));

        addKnob ("poly.voice.spread", "SPREAD", { x, 470, width, 58 });
        addKnob ("poly.voice.analog", "ANALOG", { x, 536, width, 58 });
    }

    {
        const auto x = voiceArea.getX() + 6;
        const auto width = voiceArea.getWidth() - 12;
        auto y = 38;

        addChoice ("poly.voice.mode", "VOICE MODE", { x, y, width, 42 }); y += 50;
        addChoice ("poly.voice.unison", "UNISON", { x, y, width, 42 }); y += 50;
        addKnob ("poly.voice.glide", "GLIDE", { x, y, width, 56 }); y += 62;
        addChoice ("poly.voice.pitchBendRange", "BEND RANGE", { x, y, width, 42 }); y += 50;
        addKnob ("poly.voice.spread", "SPREAD", { x, y, width, 56 }); y += 62;
        addKnob ("poly.voice.analog", "ANALOG", { x, y, width, 56 }); y += 62;
        addKnob ("poly.voice.morph", "MORPH", { x, y, width, 56 });
    }

    // -----------------------------------------------------------------------
    // VCO 1 / VCO 2
    // -----------------------------------------------------------------------
    {
        const auto x = vco1Area.getX() + 6;
        const auto cellWidth = (vco1Area.getWidth() - 12) / 2;
        auto y = 28;

        addSwitch ("poly.vcoA.octave", "OCTAVE", { x, y, vco1Area.getWidth() - 12, 34 }); y += 38;
        addKnob ("poly.vcoA.tune", "TUNE", { x, y, cellWidth, 48 });
        addKnob ("poly.vcoA.wave", "SAW / TRI", { x + cellWidth, y, cellWidth, 48 }); y += 48;
        addKnob ("poly.vcoA.pw", "PULSE WIDTH", { x, y, cellWidth, 48 });
        addKnob ("poly.vcoA.pulse", "PULSE", { x + cellWidth, y, cellWidth, 48 }); y += 48;
        addKnob ("poly.vcoA.level", "LEVEL", { x, y, cellWidth, 48 });
        addKnob ("poly.vcoA.metalizer", "METALIZER", { x + cellWidth, y, cellWidth, 48 });
    }

    {
        const auto x = vco2Area.getX() + 6;
        const auto cellWidth = (vco2Area.getWidth() - 12) / 2;
        auto y = 28;

        addSwitch ("poly.vcoB.octave", "OCTAVE", { x, y, vco2Area.getWidth() - 12, 34 }); y += 38;
        addKnob ("poly.vcoB.tune", "TUNE", { x, y, cellWidth, 46 });
        addKnob ("poly.vcoB.wave", "SAW / TRI", { x + cellWidth, y, cellWidth, 46 }); y += 46;
        addKnob ("poly.vcoB.pw", "PULSE WIDTH", { x, y, cellWidth, 46 });
        addKnob ("poly.vcoB.pulse", "PULSE", { x + cellWidth, y, cellWidth, 46 }); y += 46;
        addKnob ("poly.vcoB.level", "LEVEL", { x, y, cellWidth, 46 });
        addKnob ("poly.vcoB.sub", "SUB OSC", { x + cellWidth, y, cellWidth, 46 }); y += 50;

        auto& sync = addLed ("poly.vcoB.sync", "HARD SYNC", { x, y, vco2Area.getWidth() - 12, 20 });
        sync.setAccent (accent);
        y += 24;
        addKnob ("poly.vcoB.fm", "B > A FM", { x, y, vco2Area.getWidth() - 12, 44 });
    }

    // -----------------------------------------------------------------------
    // Mixer / filters
    // -----------------------------------------------------------------------
    {
        const auto x = mixerArea.getX() + 6;
        const auto width = mixerArea.getWidth() - 12;

        addKnob ("poly.mixer.noise", "NOISE", { x, 34, width, 62 });
        addSwitch ("poly.mixer.noiseColor", "COLOR", { x, 104, width, 40 });
    }

    {
        const auto x = steinerArea.getX() + 6;
        const auto cellWidth = (steinerArea.getWidth() - 12) / 2;
        auto y = 30;

        addKnob ("poly.steiner.mode", "MODE", { x, y, steinerArea.getWidth() - 12, 48 }); y += 52;
        addKnob ("poly.steiner.cutoff", "CUTOFF", { x, y, cellWidth, 48 });
        addKnob ("poly.steiner.resonance", "RESO", { x + cellWidth, y, cellWidth, 48 }); y += 52;
        addKnob ("poly.steiner.envAmt", "ENV AMT", { x, y, cellWidth, 48 });
        addKnob ("poly.steiner.fm", "FILTER FM", { x + cellWidth, y, cellWidth, 48 }); y += 52;
        addKnob ("poly.steiner.keyTrack", "KEY TRACK", { x, y, cellWidth, 48 });
        addKnob ("poly.steiner.level", "LEVEL", { x + cellWidth, y, cellWidth, 48 });
    }

    {
        const auto x = ladderArea.getX() + 6;
        const auto cellWidth = (ladderArea.getWidth() - 12) / 2;
        auto y = 30;

        addKnob ("poly.ladder.cutoff", "CUTOFF", { x, y, cellWidth, 48 });
        addKnob ("poly.ladder.resonance", "RESO", { x + cellWidth, y, cellWidth, 48 }); y += 52;
        addKnob ("poly.ladder.drive", "BRUTE", { x, y, cellWidth, 48 });
        addKnob ("poly.ladder.envAmt", "ENV AMT", { x + cellWidth, y, cellWidth, 48 }); y += 52;
        addKnob ("poly.ladder.keyTrack", "KEY TRACK", { x, y, cellWidth, 48 });
        addKnob ("poly.ladder.level", "LEVEL", { x + cellWidth, y, cellWidth, 48 });
    }

    {
        const auto x = filterFmArea.getX() + 6;
        const auto cellWidth = (filterFmArea.getWidth() - 12) / 3;
        auto y = 30;

        addKnob ("poly.steiner.fm", "VCO 2 > VCF", { x, y, cellWidth, 50 });
        addKnob ("poly.mixer.noise", "NOISE LEVEL", { x + cellWidth, y, cellWidth, 50 });
        addSwitch ("poly.mixer.noiseColor", "NOISE COLOR",
                   { x + cellWidth * 2, y, cellWidth, 34 });
        y += 62;
        addCaption ("NOISE INTO THE MIXER AND FILTER FM INTO THE STEINER CUTOFF.",
                    { x, y, filterFmArea.getWidth() - 12, 52 },
                    juce::Justification::topLeft, 7.0f, juce::Colour (0xffd0ccc3));
    }

    {
        const auto x = routingArea.getX() + 6;
        const auto width = routingArea.getWidth() - 12;

        addChoice ("poly.filters.routing", "FILTER ROUTING", { x, 34, width, 40 });
        addKnob ("poly.filters.balance", "FILTER BALANCE", { x, 88, width, 54 });
        addCaption ("SERIAL / PARALLEL / SPLIT", { x, 152, width, 24 },
                    juce::Justification::centred, 7.0f, juce::Colour (0xffd0ccc3));
    }

    // -----------------------------------------------------------------------
    // Envelopes (hardware-style faders)
    // -----------------------------------------------------------------------
    auto buildEnvelope = [&] (const juce::String& stem, juce::Rectangle<int> area)
    {
        const auto x = area.getX() + 6;
        const auto width = area.getWidth() - 12;
        const auto cellWidth = width / 5;

        const char* names[5] = { "VELO", "ATT", "DEC", "SUS", "REL" };
        const char* suffixes[5] = { "velocity", "attack", "decay", "sustain", "release" };

        for (int i = 0; i < 5; ++i)
            addFader (stem + suffixes[i], names[i], { x + i * cellWidth, 30, cellWidth, 220 });
    };

    buildEnvelope ("poly.env1.", vcfEnvArea);
    buildEnvelope ("poly.env2.", vcaEnvArea);
    buildEnvelope ("poly.env3.", modEnvArea);

    // -----------------------------------------------------------------------
    // LFOs
    // -----------------------------------------------------------------------
    auto buildLfo = [&] (int index, juce::Rectangle<int> area,
                         const juce::String& amountId, const juce::String& amountLabel)
    {
        const auto stem = "poly.lfo" + juce::String (index) + ".";
        const auto x = area.getX() + 5;
        const auto width = area.getWidth() - 10;
        auto y = 28;

        addKnob (stem + "rate", "RATE", { x, y, width, 46 }); y += 50;
        addChoice (stem + "wave", "WAVE", { x, y, width, 30 }); y += 34;
        auto& sync = addLed (stem + "sync", "SYNC", { x, y, width, 20 });
        sync.setAccent (accent);
        y += 24;
        auto& retrig = addLed (stem + "retrigger", "RETRIG", { x, y, width, 20 });
        retrig.setAccent (accent);
        y += 24;
        addKnob (stem + "fade", "FADE", { x, y, width, 44 }); y += 48;

        addKnob (amountId, amountLabel, { x, y, width, 44 });
    };

    buildLfo (1, lfo1Area, "poly.lfo1.pitchAmt", "PITCH");
    buildLfo (2, lfo2Area, "poly.lfo2.pwmAmt", "PWM");
    buildLfo (3, lfo3Area, "poly.lfo3.cutoffAmt", "CUTOFF");

    // -----------------------------------------------------------------------
    // Morph
    // -----------------------------------------------------------------------
    {
        const auto x = morphArea.getX() + 6;
        const auto width = morphArea.getWidth() - 12;

        auto& knob = addKnob ("poly.voice.morph", "MORPH A/B", { x, 34, width, 120 });
        knob.getProperties().set ("silverKnob", true);

        addButton ("CAPTURE B", { x, 166, width, 28 }, [this] { host().captureMorphB(); });
        addButton ("SWAP A/B", { x, 200, width, 28 }, [this] { host().swapMorphB(); });

        addCaption ("A / B PARAMETER INTERPOLATION",
                    { x, 238, width, 22 }, juce::Justification::centred, 7.0f,
                    juce::Colour (0xffe4e0d7));
    }

    // -----------------------------------------------------------------------
    // Matrix
    // -----------------------------------------------------------------------
    {
        const auto x = matrixArea.getX() + 8;
        const auto width = matrixArea.getWidth() - 16;

        auto& screen = addLcd ({ x, 24, width, 54 });
        screen.setText ("MOD MATRIX", "SOURCE x SLOT", "CLICK A CELL TO ASSIGN");

        std::vector<juce::String> sources;
        for (int slot = 1; slot <= 8; ++slot)
            sources.push_back ("poly.mod.slot" + juce::String (slot) + ".source");

        addSourceMatrix (sources, 12, { x, 86, 190, 300 });

        addCaption ("SLOT  DESTINATION", { x + 16, 88, 170, 10 },
                    juce::Justification::centredLeft, 7.0f, juce::Colour (0xff20262a));

        for (int slot = 1; slot <= 8; ++slot)
        {
            const auto y = 406 + (slot - 1) * 54;
            const auto stem = "poly.mod.slot" + juce::String (slot) + ".";

            addCaption (juce::String (slot), { x, y + 6, 16, 12 },
                        juce::Justification::centred, 8.0f, juce::Colour (0xff20262a));
            addChoice (stem + "dest", "", { x + 20, y, 160, 34 });
            addKnob (stem + "amount", "AMT", { x + 186, y - 6, 48, 46 });
        }
    }

    // -----------------------------------------------------------------------
    // Effects
    // -----------------------------------------------------------------------
    {
        const auto x = fxArea.getX() + 6;
        const auto width = fxArea.getWidth() - 12;
        auto y = 28;

        addCaption ("DISTORTION", { x, y, width, 10 }, juce::Justification::centred,
                    7.5f, juce::Colour (0xffe4e0d7), true);
        y += 14;
        addChoice ("poly.fx.dist.type", "", { x, y, width, 28 }); y += 32;
        addKnob ("poly.fx.dist.drive", "DRIVE", { x, y, width, 44 }); y += 46;
        addKnob ("poly.fx.dist.mix", "MIX", { x, y, width, 44 }); y += 52;

        addCaption ("MODULATION", { x, y, width, 10 }, juce::Justification::centred,
                    7.5f, juce::Colour (0xffe4e0d7), true);
        y += 14;
        addKnob ("poly.fx.chorus.rate", "CHORUS RATE", { x, y, width, 44 }); y += 46;
        addKnob ("poly.fx.chorus.depth", "DEPTH", { x, y, width, 44 }); y += 46;
        addKnob ("poly.fx.chorus.mix", "MIX", { x, y, width, 44 }); y += 52;

        addCaption ("DELAY", { x, y, width, 10 }, juce::Justification::centred,
                    7.5f, juce::Colour (0xffe4e0d7), true);
        y += 14;
        addKnob ("poly.fx.delay.time", "TIME", { x, y, width, 44 }); y += 46;
        addKnob ("poly.fx.delay.feedback", "FEEDBACK", { x, y, width, 44 }); y += 46;
        addKnob ("poly.fx.delay.mix", "MIX", { x, y, width, 44 }); y += 46;
        auto& ping = addLed ("poly.fx.delay.pingPong", "PING PONG", { x, y, width, 20 });
        ping.setAccent (accent);
        y += 24;
        auto& sync = addLed ("poly.fx.delay.sync", "TEMPO SYNC", { x, y, width, 20 });
        sync.setAccent (accent);
        y += 30;

        addCaption ("REVERB", { x, y, width, 10 }, juce::Justification::centred,
                    7.5f, juce::Colour (0xffe4e0d7), true);
        y += 14;
        addKnob ("poly.fx.reverb.size", "SIZE", { x, y, width, 44 }); y += 46;
        addKnob ("poly.fx.reverb.damping", "DAMPING", { x, y, width, 44 }); y += 46;
        addKnob ("poly.fx.reverb.mix", "MIX", { x, y, width, 44 });
    }

    // -----------------------------------------------------------------------
    // Arpeggiator
    // -----------------------------------------------------------------------
    {
        const auto x = arpArea.getX() + 6;
        const auto width = arpArea.getWidth() - 12;
        auto y = 34;

        auto& enabled = addLed ("poly.arp.enabled", "ARPEGGIATOR", { x, y, width, 24 });
        enabled.setAccent (accent);
        y += 34;
        addChoice ("poly.arp.mode", "MODE", { x, y, width, 40 }); y += 48;
        addChoice ("poly.arp.rate", "RATE", { x, y, width, 40 }); y += 48;
        addSwitch ("poly.arp.octaves", "OCTAVES", { x, y, width, 34 }); y += 42;
        auto& hold = addLed ("poly.arp.hold", "HOLD", { x, y, width, 22 });
        hold.setAccent (accent);
        y += 30;
        addChoice ("poly.arp.length", "LENGTH", { x, y, width, 40 }); y += 56;

        addCaption ("CLICK A MATRIX CELL TO SET THE SLOT SOURCE. USE THE ROWS BELOW FOR DESTINATION AND AMOUNT.",
                    { x, y, width, 110 }, juce::Justification::topLeft, 7.0f,
                    juce::Colour (0xffd0ccc3));
    }
}

void PolyBrutePanel::paint (juce::Graphics& g)
{
    PanelBase::paint (g);
}

} // namespace triad::ui
