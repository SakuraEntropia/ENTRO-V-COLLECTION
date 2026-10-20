#include "ui/Panels.h"
#include "ui/LookAndFeel.h"

namespace triad::ui
{

namespace
{
juce::Colour polyAccent() { return juce::Colour (0xff3f93e8); }
}


PolyBrutePanel::PolyBrutePanel (EditorHost& editorHost)
    : PanelBase (editorHost, "PolyBrute", 2200, 660, polyAccent())
{
    setBranding ("POLYBRUTE", "SIX-VOICE ANALOG SYNTHESIZER");

    const auto accent = polyAccent();

    const juce::Rectangle<int> perfArea { 8, 8, 150, 608 };
    const juce::Rectangle<int> voiceArea { 162, 8, 140, 608 };
    const juce::Rectangle<int> vcoAArea { 306, 8, 166, 608 };
    const juce::Rectangle<int> vcoBArea { 476, 8, 166, 608 };
    const juce::Rectangle<int> mixerArea { 646, 8, 96, 608 };
    const juce::Rectangle<int> steinerArea { 746, 8, 178, 608 };
    const juce::Rectangle<int> ladderArea { 928, 8, 158, 608 };
    const juce::Rectangle<int> envArea { 1090, 8, 226, 608 };
    const juce::Rectangle<int> lfoArea { 1320, 8, 206, 608 };
    const juce::Rectangle<int> matrixArea { 1530, 8, 300, 608 };
    const juce::Rectangle<int> fxArea { 1834, 8, 214, 608 };
    const juce::Rectangle<int> arpArea { 2052, 8, 140, 608 };

    addModule ("PERFORMANCE", perfArea);
    addModule ("VOICE", voiceArea);
    addModule ("OSCILLATOR A", vcoAArea);
    addModule ("OSCILLATOR B", vcoBArea);
    addModule ("MIXER", mixerArea);
    addModule ("STEINER FILTER", steinerArea);
    addModule ("LADDER FILTER", ladderArea);
    addModule ("ENVELOPES", envArea);
    addModule ("LFOs", lfoArea);
    addModule ("MATRIX", matrixArea);
    addModule ("EFFECTS", fxArea);
    addModule ("ARPEGGIATOR", arpArea);

    // ---- Performance ------------------------------------------------------
    {
        const auto x = perfArea.getX() + 8;
        const auto width = perfArea.getWidth() - 16;

        addKnob ("poly.voice.morph", "MORPH A/B", { x, 40, width, 96 });
        addCaption ("A", { x - 2, 132, 16, 12 }, juce::Justification::centred, 8.0f,
                    juce::Colour (0xff8f8c85));
        addCaption ("B", { x + width - 14, 132, 16, 12 }, juce::Justification::centred, 8.0f,
                    juce::Colour (0xff8f8c85));

        addButton ("CAPTURE B", { x, 156, width, 30 }, [this] { host().captureMorphB(); });
        addButton ("SWAP A/B", { x, 192, width, 30 }, [this] { host().swapMorphB(); });

        addCaption ("CAPTURE STORES THE CURRENT PANEL AS THE B SIDE.",
                    { x, 236, width, 48 }, juce::Justification::topLeft, 7.0f,
                    juce::Colour (0xff7d7a74));
        addCaption ("MORPH INTERPOLATES THE SYNTHESIS PARAMETERS.",
                    { x, 288, width, 48 }, juce::Justification::topLeft, 7.0f,
                    juce::Colour (0xff7d7a74));

        addKnob ("poly.voice.spread", "SPREAD", { x, 350, width, 58 });
        addKnob ("poly.voice.analog", "ANALOG", { x, 412, width, 58 });
    }

    // ---- Voice ------------------------------------------------------------
    {
        const auto x = voiceArea.getX() + 6;
        const auto width = voiceArea.getWidth() - 12;

        addChoice ("poly.voice.mode", "VOICE MODE", { x, 38, width, 40 });
        addChoice ("poly.voice.unison", "UNISON", { x, 86, width, 40 });
        addKnob ("poly.voice.glide", "GLIDE", { x, 132, width, 58 });
        addChoice ("poly.voice.pitchBendRange", "BEND RANGE", { x, 196, width, 40 });
        addKnob ("poly.voice.spread", "SPREAD", { x, 250, width, 58 });
        addKnob ("poly.voice.analog", "ANALOG", { x, 312, width, 58 });
        addKnob ("poly.voice.morph", "MORPH", { x, 374, width, 58 });
    }

    // ---- VCO A ------------------------------------------------------------
    {
        const auto x = vcoAArea.getX() + 6;
        const auto width = vcoAArea.getWidth() - 12;
        auto y = 38;

        addSwitch ("poly.vcoA.octave", "OCTAVE", { x, y, width, 40 });
        y += 44;
        addKnob ("poly.vcoA.tune", "TUNE", { x, y, width, 58 }); y += 62;
        addKnob ("poly.vcoA.wave", "SAW / TRI", { x, y, width, 58 }); y += 62;
        addKnob ("poly.vcoA.pw", "PULSE WIDTH", { x, y, width, 58 }); y += 62;
        addKnob ("poly.vcoA.pulse", "PULSE LEVEL", { x, y, width, 58 }); y += 62;
        addKnob ("poly.vcoA.metalizer", "METALIZER", { x, y, width, 58 }); y += 62;
        addKnob ("poly.vcoA.level", "LEVEL", { x, y, width, 58 });
    }

    // ---- VCO B ------------------------------------------------------------
    {
        const auto x = vcoBArea.getX() + 6;
        const auto width = vcoBArea.getWidth() - 12;
        auto y = 38;

        addSwitch ("poly.vcoB.octave", "OCTAVE", { x, y, width, 40 });
        y += 44;
        addKnob ("poly.vcoB.tune", "TUNE", { x, y, width, 58 }); y += 62;
        addKnob ("poly.vcoB.wave", "SAW / TRI", { x, y, width, 58 }); y += 62;
        addKnob ("poly.vcoB.pw", "PULSE WIDTH", { x, y, width, 58 }); y += 62;
        addKnob ("poly.vcoB.pulse", "PULSE LEVEL", { x, y, width, 58 }); y += 62;
        addKnob ("poly.vcoB.sub", "SUB OSC", { x, y, width, 58 }); y += 62;
        addKnob ("poly.vcoB.level", "LEVEL", { x, y, width, 58 }); y += 62;
        auto& sync = addLed ("poly.vcoB.sync", "HARD SYNC", { x, y, width, 24 });
        sync.setAccent (accent);
        y += 30;
        addKnob ("poly.vcoB.fm", "B > A FM", { x, y, width, 58 });
    }

    // ---- Mixer ------------------------------------------------------------
    {
        const auto x = mixerArea.getX() + 6;
        const auto width = mixerArea.getWidth() - 12;

        addKnob ("poly.mixer.noise", "NOISE", { x, 38, width, 62 });
        addSwitch ("poly.mixer.noiseColor", "COLOR", { x, 108, width, 40 });
    }

    // ---- Steiner filter ---------------------------------------------------
    {
        const auto x = steinerArea.getX() + 6;
        const auto width = steinerArea.getWidth() - 12;
        auto y = 38;

        addKnob ("poly.steiner.mode", "MODE", { x, y, width, 58 }); y += 62;
        addKnob ("poly.steiner.cutoff", "CUTOFF", { x, y, width, 58 }); y += 62;
        addKnob ("poly.steiner.resonance", "RESONANCE", { x, y, width, 58 }); y += 62;
        addKnob ("poly.steiner.envAmt", "ENV AMOUNT", { x, y, width, 58 }); y += 62;
        addKnob ("poly.steiner.fm", "FILTER FM", { x, y, width, 58 }); y += 62;
        addKnob ("poly.steiner.keyTrack", "KEY TRACK", { x, y, width, 58 }); y += 62;
        addKnob ("poly.steiner.level", "LEVEL", { x, y, width, 58 }); y += 62;
        addChoice ("poly.filters.routing", "ROUTING", { x, y, width, 40 }); y += 48;
        addKnob ("poly.filters.balance", "BALANCE", { x, y, width, 58 });
    }

    // ---- Ladder filter ----------------------------------------------------
    {
        const auto x = ladderArea.getX() + 6;
        const auto width = ladderArea.getWidth() - 12;
        auto y = 38;

        addKnob ("poly.ladder.cutoff", "CUTOFF", { x, y, width, 62 }); y += 66;
        addKnob ("poly.ladder.resonance", "RESONANCE", { x, y, width, 62 }); y += 66;
        addKnob ("poly.ladder.drive", "BRUTE FACTOR", { x, y, width, 62 }); y += 66;
        addKnob ("poly.ladder.envAmt", "ENV AMOUNT", { x, y, width, 62 }); y += 66;
        addKnob ("poly.ladder.keyTrack", "KEY TRACK", { x, y, width, 62 }); y += 66;
        addKnob ("poly.ladder.level", "LEVEL", { x, y, width, 62 });
    }

    // ---- Envelopes --------------------------------------------------------
    {
        const auto columnWidth = (envArea.getWidth() - 20) / 3;
        const juce::String prefixes[3] = { "poly.env1.", "poly.env2.", "poly.env3." };
        const char* names[3] = { "VCF ENV", "VCA ENV", "MOD ENV" };

        for (int env = 0; env < 3; ++env)
        {
            const auto x = envArea.getX() + 6 + env * (columnWidth + 4);
            addCaption (names[env], { x, 32, columnWidth, 12 }, juce::Justification::centred,
                        7.5f, juce::Colour (0xffc7c3ba), true);

            auto y = 50;
            addKnob (prefixes[env] + "attack", "ATTACK", { x, y, columnWidth, 54 }); y += 58;
            addKnob (prefixes[env] + "decay", "DECAY", { x, y, columnWidth, 54 }); y += 58;
            addKnob (prefixes[env] + "sustain", "SUSTAIN", { x, y, columnWidth, 54 }); y += 58;
            addKnob (prefixes[env] + "release", "RELEASE", { x, y, columnWidth, 54 }); y += 58;
            addKnob (prefixes[env] + "velocity", "VELOCITY", { x, y, columnWidth, 54 });
        }
    }

    // ---- LFOs -------------------------------------------------------------
    {
        const auto columnWidth = (lfoArea.getWidth() - 20) / 3;

        for (int lfo = 0; lfo < 3; ++lfo)
        {
            const auto x = lfoArea.getX() + 6 + lfo * (columnWidth + 4);
            const auto prefix = "poly.lfo" + juce::String (lfo + 1) + ".";

            addCaption ("LFO " + juce::String (lfo + 1), { x, 32, columnWidth, 12 },
                        juce::Justification::centred, 7.5f, juce::Colour (0xffc7c3ba), true);

            auto y = 50;
            addKnob (prefix + "rate", "RATE", { x, y, columnWidth, 52 }); y += 56;
            addChoice (prefix + "wave", "WAVE", { x, y, columnWidth, 34 }); y += 40;
            auto& sync = addLed (prefix + "sync", "SYNC", { x, y, columnWidth, 22 });
            sync.setAccent (accent);
            y += 26;
            auto& retrig = addLed (prefix + "retrigger", "RETRIG", { x, y, columnWidth, 22 });
            retrig.setAccent (accent);
            y += 26;
            addKnob (prefix + "fade", "FADE", { x, y, columnWidth, 52 });
        }

        const auto y = 380;
        const auto width = (lfoArea.getWidth() - 20) / 3;
        addKnob ("poly.lfo1.pitchAmt", "PITCH AMT", { lfoArea.getX() + 6, y, width, 58 });
        addKnob ("poly.lfo2.pwmAmt", "PWM AMT", { lfoArea.getX() + 6 + width + 4, y, width, 58 });
        addKnob ("poly.lfo3.cutoffAmt", "CUTOFF AMT",
                 { lfoArea.getX() + 6 + (width + 4) * 2, y, width, 58 });
    }

    // ---- Matrix -----------------------------------------------------------
    {
        const auto x = matrixArea.getX() + 8;
        const auto width = matrixArea.getWidth() - 16;

        addCaption ("SOURCE", { x, 30, 120, 10 }, juce::Justification::centredLeft, 7.0f,
                    juce::Colour (0xff8f8c85));
        addCaption ("DEST / AMOUNT", { x + 128, 30, 150, 10 }, juce::Justification::centredLeft,
                    7.0f, juce::Colour (0xff8f8c85));

        std::vector<juce::String> sources;
        for (int slot = 1; slot <= 8; ++slot)
            sources.push_back ("poly.mod.slot" + juce::String (slot) + ".source");

        auto& matrix = addSourceMatrix (sources, 12, { x, 44, 118, 168 });
        matrix.onColumnSelected = [] (int) {};

        for (int slot = 1; slot <= 8; ++slot)
        {
            const auto rowY = 44 + (slot - 1) * 60;
            const auto prefix = "poly.mod.slot" + juce::String (slot) + ".";

            addCaption (juce::String (slot), { x + 122, rowY + 4, 14, 12 },
                        juce::Justification::centred, 7.5f, juce::Colour (0xff8f8c85));

            addChoice (prefix + "dest", "", { x + 138, rowY, 100, 32 });
            addKnob (prefix + "amount", "AMT", { x + 238, rowY - 10, 44, 52 });
        }

        addCaption ("CLICK A MATRIX CELL TO SET A SLOT'S SOURCE.",
                    { x, 220, 118, 44 }, juce::Justification::topLeft, 6.5f,
                    juce::Colour (0xff7d7a74));
    }

    // ---- Effects ----------------------------------------------------------
    {
        const auto columnWidth = (fxArea.getWidth() - 16) / 2;
        const auto x1 = fxArea.getX() + 6;
        const auto x2 = x1 + columnWidth + 4;

        addCaption ("DISTORTION", { x1, 30, columnWidth, 10 }, juce::Justification::centred,
                    7.0f, juce::Colour (0xff8f8c85), true);
        addChoice ("poly.fx.dist.type", "", { x1, 42, columnWidth - 2, 30 });
        addKnob ("poly.fx.dist.drive", "DRIVE", { x1, 76, columnWidth, 50 });
        addKnob ("poly.fx.dist.mix", "MIX", { x1, 128, columnWidth, 50 });

        addCaption ("CHORUS", { x2, 30, columnWidth, 10 }, juce::Justification::centred,
                    7.0f, juce::Colour (0xff8f8c85), true);
        addKnob ("poly.fx.chorus.rate", "RATE", { x2, 42, columnWidth, 50 });
        addKnob ("poly.fx.chorus.depth", "DEPTH", { x2, 94, columnWidth, 50 });
        addKnob ("poly.fx.chorus.mix", "MIX", { x2, 146, columnWidth, 50 });

        addCaption ("DELAY", { x1, 196, columnWidth, 10 }, juce::Justification::centred,
                    7.0f, juce::Colour (0xff8f8c85), true);
        addKnob ("poly.fx.delay.time", "TIME", { x1, 208, columnWidth, 50 });
        addKnob ("poly.fx.delay.feedback", "FEEDBACK", { x1, 260, columnWidth, 50 });
        addKnob ("poly.fx.delay.mix", "MIX", { x1, 312, columnWidth, 50 });
        auto& ping = addLed ("poly.fx.delay.pingPong", "PING PONG",
                             { x1, 366, columnWidth, 22 });
        ping.setAccent (accent);
        auto& sync = addLed ("poly.fx.delay.sync", "TEMPO SYNC",
                             { x1, 392, columnWidth, 22 });
        sync.setAccent (accent);

        addCaption ("REVERB", { x2, 196, columnWidth, 10 }, juce::Justification::centred,
                    7.0f, juce::Colour (0xff8f8c85), true);
        addKnob ("poly.fx.reverb.size", "SIZE", { x2, 208, columnWidth, 50 });
        addKnob ("poly.fx.reverb.damping", "DAMPING", { x2, 260, columnWidth, 50 });
        addKnob ("poly.fx.reverb.mix", "MIX", { x2, 312, columnWidth, 50 });
    }

    // ---- Arpeggiator ------------------------------------------------------
    {
        const auto x = arpArea.getX() + 6;
        const auto width = arpArea.getWidth() - 12;
        auto y = 36;

        auto& enabled = addLed ("poly.arp.enabled", "ARPEGGIATOR", { x, y, width, 24 });
        enabled.setAccent (accent);
        y += 32;
        addChoice ("poly.arp.mode", "MODE", { x, y, width, 40 }); y += 46;
        addChoice ("poly.arp.rate", "RATE", { x, y, width, 40 }); y += 46;
        addSwitch ("poly.arp.octaves", "OCTAVES", { x, y, width, 34 }); y += 42;
        auto& hold = addLed ("poly.arp.hold", "HOLD", { x, y, width, 24 });
        hold.setAccent (accent);
        y += 32;
        addChoice ("poly.arp.length", "LENGTH", { x, y, width, 40 });
    }
}

void PolyBrutePanel::paint (juce::Graphics& g)
{
    PanelBase::paint (g);

    // Wooden cheeks, as on the hardware.
    const auto height = (float) getHeight();
    for (const auto& side : { juce::Rectangle<float> (0.0f, 0.0f, 7.0f, height),
                              juce::Rectangle<float> ((float) getWidth() - 7.0f, 0.0f, 7.0f, height) })
    {
        juce::ColourGradient wood (juce::Colour (0xff7a4c2a), side.getX(), 0.0f,
                                   juce::Colour (0xff3a2414), side.getRight(), 0.0f, false);
        wood.addColour (0.35, juce::Colour (0xff5b3620));
        g.setGradientFill (wood);
        g.fillRect (side);
    }
}

} // namespace triad::ui
