#include "ui/Panels.h"

namespace triad::ui
{
namespace { juce::Colour oneAccent() { return juce::Colour (0xffd9b773); } }

OnePanel::OnePanel (EditorHost& host)
    : PanelBase (host, "ONE V", 2240, 720, oneAccent())
{
    setBranding ("ONE V", "THREE-PART POLYPHONIC ANALOG SYNTHESIZER");
    rebuildForPart();
}

void OnePanel::rebuildForPart()
{
    clearControls();
    const auto part = juce::jlimit (1, 3, host().selectedPart());
    const auto prefix = "one.p" + juce::String (part) + ".";
    auto knob = [this] (const juce::String& id, const char* label, int x, int y, int d = 44)
    { addKnob (id, label, { x, y }, d); };

    // Three continuous black instrument plates, enclosed by silver trim.
    for (int i = 0; i < 3; ++i)
    {
        const auto x = 24 + i * 290;
        const auto stem = prefix + "vco" + juce::String (i + 1) + ".";
        addModule ("OSCILLATOR " + juce::String (i + 1), { x, 24, 282, 296 });
        addChoice (stem + "octave", "OCTAVE", { x + 22, 61, 104, 36 });
        if (i == 0)
            addLed (stem + "on", "ON", { x + 160, 61, 90, 24 });
        else
            addLed (stem + "sync", "SYNC", { x + 160, 61, 90, 24 });
        knob (stem + "tune", "FREQUENCY", x + 72, 145, 60);
        knob (stem + "waveAngle", "WAVE ANGLE", x + 210, 145, 54);
        auto& display = waveformDisplays_[(size_t) i];
        display = std::make_unique<OneWaveDisplay> (apvts(), stem);
        display->setBounds (x + 111, 121, 64, 46);
        addAndMakeVisible (*display);
        knob (stem + "shape", "SHAPE", x + 48, 251);
        knob (stem + "pw", "PULSE WIDTH", x + 141, 251);
        knob (stem + "level", "LEVEL", x + 234, 251);
        if (i == 2)
            knob (stem + "fm", "3 > 1 FM", x + 141, 200, 32);
    }

    addModule ("ARPEGGIATOR", { 24, 332, 862, 122 });
    addLed (prefix + "arp.enabled", "ARP", { 42, 375, 72, 25 });
    addLed (prefix + "arp.hold", "HOLD", { 42, 412, 72, 24 });
    addChoice (prefix + "arp.mode", "MODE", { 138, 382, 180, 38 });
    addChoice (prefix + "arp.rate", "RATE / DIV", { 336, 382, 176, 38 });
    addSwitch (prefix + "arp.octaves", "OCTAVES", { 530, 382, 158, 38 });
    addChoice (prefix + "arp.length", "LENGTH", { 706, 382, 160, 38 });

    addModule ("MODULATION MATRIX", { 24, 466, 862, 178 });
    // Two rows, four slots per row. Every slot retains source/destination/amount.
    for (int slot = 1; slot <= 8; ++slot)
    {
        const auto stem = prefix + "mod.slot" + juce::String (slot) + ".";
        const auto x = 38 + (slot - 1) % 4 * 212;
        const auto y = 503 + (slot - 1) / 4 * 68;
        addCaption (juce::String (slot), { x, y, 16, 16 }, juce::Justification::centred, 10.0f);
        addChoice (stem + "source", "", { x + 18, y, 122, 24 });
        addChoice (stem + "dest", "", { x + 18, y + 29, 122, 24 });
        knob (stem + "amount", "AMT", x + 173, y + 19, 34);
    }

    addModule ("CENTER CONSOLE", { 900, 24, 326, 620 });
    for (int i = 0; i < 3; ++i)
    {
        auto& button = addButton ("PART " + juce::String (i + 1), { 914 + i * 100, 58, 96, 28 },
            [this, i]
            {
                host().setSelectedPart (i + 1);
                juce::Component::SafePointer<OnePanel> safe (this);
                juce::MessageManager::callAsync ([safe] { if (safe != nullptr) safe->rebuildForPart(); });
            });
        button.setToggleState (i + 1 == part, juce::dontSendNotification);
    }
    auto& lcd = addLcd ({ 918, 104, 290, 108 });
    lcd.setText ("ONE V   /   PART " + juce::String (part),
                 "TIMBRE " + juce::String (part) + "    MIDI CH " + juce::String (part), "SHARED 8-VOICE POOL");
    addLed (prefix + "enabled", "PART ENABLE", { 918, 228, 134, 24 });
    knob ("one.master.volume", "MASTER", 1134, 257, 56);
    knob (prefix + "level", "PART LEVEL", 963, 310, 48);
    knob (prefix + "pan", "PAN", 1063, 340, 48);
    addChoice (prefix + "midiChannel", "MIDI CHANNEL", { 918, 361, 118, 36 });
    knob (prefix + "keyboardLow", "KEY LOW", 960, 445, 40);
    knob (prefix + "keyboardHigh", "KEY HIGH", 1063, 445, 40);
    knob ("one.master.drift", "ANALOG DRIFT", 1166, 445, 40);
    knob ("one.master.reverbSize", "REV SIZE", 960, 520, 40);
    knob ("one.master.reverbDamping", "REV DAMP", 1063, 520, 40);
    knob ("one.master.reverbMix", "REV MIX", 1166, 520, 40);
    knob ("one.master.glide", "GLIDE", 960, 594, 40);
    knob ("one.master.aftertouch", "AFTERTOUCH", 1063, 594, 40);
    knob ("one.master.pitchBendRange", "BEND RANGE", 1166, 594, 40);

    addModule ("MIXER", { 1240, 24, 140, 306 });
    knob (prefix + "mix.noise", "NOISE", 1310, 80, 42);
    addSwitch (prefix + "mix.noiseColor", "COLOR", { 1254, 129, 112, 34 });
    knob (prefix + "mix.ring", "RING MOD", 1310, 208, 42);
    knob (prefix + "mix.ext", "EXTERNAL", 1310, 282, 42);

    addModule ("STATE VARIABLE FILTER", { 1388, 24, 214, 306 });
    knob (prefix + "svf.cutoff", "CUTOFF", 1444, 90, 58);
    knob (prefix + "svf.resonance", "RESONANCE", 1546, 90, 44);
    addChoice (prefix + "svf.mode", "MODE", { 1402, 147, 88, 34 });
    addChoice (prefix + "svf.slope", "SLOPE", { 1500, 147, 88, 34 });
    knob (prefix + "svf.envAmt", "ENV AMT", 1444, 225, 40);
    knob (prefix + "svf.keyTrack", "KEY TRACK", 1546, 225, 40);
    knob (prefix + "svf.level", "LEVEL", 1424, 296, 32);
    addChoice (prefix + "filters.routing", "", { 1458, 282, 88, 24 });
    knob (prefix + "filters.mix", "MIX", 1572, 296, 32);

    addModule ("LADDER FILTER", { 1610, 24, 214, 306 });
    knob (prefix + "ladder.cutoff", "CUTOFF", 1666, 90, 58);
    knob (prefix + "ladder.resonance", "RESONANCE", 1768, 90, 44);
    knob (prefix + "ladder.drive", "DRIVE", 1666, 185, 44);
    addChoice (prefix + "ladder.slope", "SLOPE", { 1726, 167, 84, 34 });
    knob (prefix + "ladder.envAmt", "ENV AMT", 1666, 282, 44);
    knob (prefix + "ladder.keyTrack", "KEY TRACK", 1768, 282, 44);

    addModule ("ENVELOPE GENERATORS", { 1832, 24, 384, 306 });
    const char* envIds[] = { "delay", "attack", "hold", "decay", "sustain", "release", "velocity" };
    const char* envLabels[] = { "DELAY", "ATT", "HOLD", "DEC", "SUS", "REL", "VELO" };
    for (int env = 1; env <= 3; ++env)
    {
        const auto y = 86 + (env - 1) * 91;
        const auto stem = prefix + "env" + juce::String (env) + ".";
        addCaption (env == 1 ? "AMP" : (env == 2 ? "FILTER" : "MOD"), { 1840, y - 29, 50, 12 },
                    juce::Justification::centredLeft, 9.0f);
        for (int i = 0; i < 7; ++i)
            knob (stem + envIds[i], envLabels[i], 1860 + i * 54, y, 36);
        if (env < 3)
            addLed (stem + "loop", "LOOP", { 2128, y + 30, 72, 18 });
    }

    for (int lfo = 1; lfo <= 4; ++lfo)
    {
        const auto x = 1240 + (lfo - 1) * 246;
        const auto stem = prefix + "lfo" + juce::String (lfo) + ".";
        addModule ("LFO " + juce::String (lfo), { x, 342, 238, 142 });
        knob (stem + "rate", "RATE", x + 36, 394, 38);
        knob (stem + "fade", "FADE", x + 116, 394, 38);
        const char* amounts[] = { "pitchAmt", "pwmAmt", "cutoffAmt" };
        const char* names[] = { "PITCH", "PWM", "CUTOFF" };
        if (lfo < 4) knob (stem + amounts[lfo - 1], names[lfo - 1], x + 198, 394, 38);
        addChoice (stem + "wave", "", { x + 10, 448, 84, 24 });
        addLed (stem + "sync", "SYNC", { x + 100, 448, 58, 22 });
        addLed (stem + "retrig", "RETRIG", { x + 164, 448, 64, 22 });
    }

    addModule ("TIMBRE EFFECTS", { 1240, 496, 976, 148 });
    addChoice (prefix + "fx.dist.type", "DISTORTION", { 1254, 537, 126, 34 });
    knob (prefix + "fx.dist.drive", "DRIVE", 1420, 562, 40);
    knob (prefix + "fx.dist.mix", "MIX", 1492, 562, 40);
    knob (prefix + "fx.chorus.rate", "CHORUS RATE", 1590, 562, 40);
    knob (prefix + "fx.chorus.depth", "DEPTH", 1664, 562, 40);
    knob (prefix + "fx.chorus.mix", "MIX", 1738, 562, 40);
    knob (prefix + "fx.delay.time", "DELAY TIME", 1836, 562, 40);
    knob (prefix + "fx.delay.feedback", "FEEDBACK", 1912, 562, 40);
    knob (prefix + "fx.delay.mix", "MIX", 1988, 562, 40);
    knob (prefix + "fx.reverb.send", "REVERB SEND", 2132, 562, 54);
}
} // namespace triad::ui
