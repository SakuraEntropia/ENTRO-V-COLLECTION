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
    consoleControls_.clear();
    modulationControls_.clear();
    clearControls();
    const auto part = juce::jlimit (1, 3, host().selectedPart());
    const auto prefix = "one.p" + juce::String (part) + ".";
    auto knob = [this] (const juce::String& id, const char* label, int x, int y, int d = 44)
    { addKnob (id, label, { x, y }, d); };

    // Same spatial hierarchy as the instrument: LFO strip at the far left,
    // three oscillators beside it, noise on their right, performance below.
    for (int lfo = 1; lfo <= 4; ++lfo)
    {
        const int y = 24 + (lfo - 1) * 115;
        const auto stem = prefix + "lfo" + juce::String (lfo) + ".";
        addModule ("LFO " + juce::String (lfo), { 24, y, 150, 107 });
        knob (stem + "rate", "RATE", 62, y + 46, 36);
        knob (stem + "fade", "FADE", 132, y + 46, 36);
        addChoice (stem + "wave", "", { 32, y + 80, 62, 22 });
        addLed (stem + "sync", "SYNC", { 100, y + 80, 30, 20 });
        addLed (stem + "retrig", "R", { 136, y + 80, 30, 20 });
    }
    for (int i = 0; i < 3; ++i)
    {
        const int x = 182 + i * 210;
        const auto stem = prefix + "vco" + juce::String (i + 1) + ".";
        addModule ("OSCILLATOR " + juce::String (i + 1), { x, 24, 202, 350 });
        // Point-up hexagon around the OLED, from the hardware close-up.
        auto& octave = addKnob (stem + "octave", "OCTAVE", { x + 101, 79 }, 50);
        octave.getProperties().set ("chickenHead", true);
        knob (stem + "tune", "FREQUENCY", x + 35, 137, 44);
        knob (stem + "beatHz", "BEAT FREQ", x + 167, 137, 44);
        knob (stem + "waveAngle", "WAVE ANGLE", x + 35, 253, 44);
        knob (stem + "pw", "PULSE WIDTH", x + 167, 253, 44);
        knob (stem + "shape", "MIX", x + 101, 311, 44);
        auto& display = waveformDisplays_[(size_t) i];
        display = std::make_unique<OneWaveDisplay> (apvts(), stem);
        display->setBounds (x + 69, 172, 64, 46);
        addAndMakeVisible (*display);
        addLed (stem + (i == 0 ? "on" : "sync"), i == 0 ? "ON" : "HARD SYNC",
                { x + 5, 345, 60, 22 });
    }
    addModule ("NOISE", { 820, 24, 66, 350 });
    knob (prefix + "mix.noise", "LEVEL", 853, 94, 38);
    addSwitch (prefix + "mix.noiseColor", "COLOR", { 826, 178, 54, 40 });
    addModule ("PITCH MOD", { 182, 386, 202, 98 });
    knob (prefix + "lfo1.pitchAmt", "LFO 1", 283, 431, 38);
    addModule ("WAVEFORM MODULATION", { 392, 386, 412, 98 });
    knob (prefix + "lfo2.pwmAmt", "PULSE MOD", 474, 431, 38);
    knob (prefix + "lfo3.cutoffAmt", "FILTER MOD", 597, 431, 38);
    knob (prefix + "vco3.fm", "3 > 1 FM", 726, 431, 38);

    addModule ("ARPEGGIATOR", { 24, 496, 862, 148 });
    addLed (prefix + "arp.enabled", "ARP", { 42, 542, 72, 25 });
    addLed (prefix + "arp.hold", "HOLD", { 42, 594, 72, 24 });
    addChoice (prefix + "arp.mode", "MODE", { 138, 555, 180, 38 });
    addChoice (prefix + "arp.rate", "RATE / DIV", { 336, 555, 176, 38 });
    addSwitch (prefix + "arp.octaves", "OCTAVES", { 530, 555, 158, 38 });
    addChoice (prefix + "arp.length", "LENGTH", { 706, 555, 160, 38 });

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
    const int consoleStart = getNumChildComponents();
    auto& lcd = addLcd ({ 918, 104, 290, 108 });
    lcd.setText ("ONE V   /   PART " + juce::String (part),
                 "TIMBRE " + juce::String (part) + "    MIDI CH " + juce::String (part), "SHARED 8 / 16 VOICE POOL");
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

    addChoice ("one.master.voices", "VOICE BUDGET", { 30, 661, 140, 30 });
    addChoice (prefix + "voice.mode", "VOICE MODE", { 190, 661, 144, 30 });
    addNumber (prefix + "voice.unisonVoices", "UNISON VOICES", { 360, 661, 110, 30 });
    addNumber (prefix + "voice.unisonDetune", "DETUNE", { 500, 661, 110, 30 });

    for (int i = consoleStart; i < getNumChildComponents(); ++i)
        consoleControls_.push_back (getChildComponent (i));
    const int modStart = getNumChildComponents();
    for (int slot = 1; slot <= 8; ++slot)
    {
        const auto stem = prefix + "mod.slot" + juce::String (slot) + ".";
        const int y = 116 + (slot - 1) * 61;
        addCaption (juce::String (slot), { 916, y + 10, 16, 20 });
        addChoice (stem + "source", "", { 936, y, 172, 24 });
        addChoice (stem + "dest", "", { 936, y + 28, 172, 24 });
        knob (stem + "amount", "AMT", 1164, y + 20, 34);
    }
    for (int i = modStart; i < getNumChildComponents(); ++i)
        modulationControls_.push_back (getChildComponent (i));
    addButton ("MOD / HOME", { 936, 632, 254, 26 }, [this]
    {
        modulationPage_ = ! modulationPage_;
        updateConsolePage();
    });
    updateConsolePage();

    addModule ("MIXER", { 1240, 24, 140, 460 });
    for (int i = 1; i <= 3; ++i)
        knob (prefix + "vco" + juce::String (i) + ".level", ("OSC " + juce::String (i)).toRawUTF8(),
              1310, 78 + (i - 1) * 80, 42);
    knob (prefix + "mix.ring", "RING MOD", 1310, 318, 42);
    knob (prefix + "mix.ext", "EXTERNAL", 1310, 414, 42);

    addModule ("STATE VARIABLE FILTER", { 1388, 24, 436, 250 });
    knob (prefix + "svf.cutoff", "CUTOFF", 1444, 90, 58);
    knob (prefix + "svf.resonance", "RESONANCE", 1546, 90, 44);
    addChoice (prefix + "svf.mode", "MODE", { 1618, 74, 88, 34 });
    addChoice (prefix + "svf.slope", "SLOPE", { 1720, 74, 88, 34 });
    knob (prefix + "svf.envAmt", "ENV AMT", 1444, 196, 40);
    knob (prefix + "svf.keyTrack", "KEY TRACK", 1546, 196, 40);
    knob (prefix + "svf.level", "LEVEL", 1648, 196, 32);
    addChoice (prefix + "filters.routing", "", { 1618, 238, 88, 24 });
    knob (prefix + "filters.mix", "MIX", 1750, 196, 32);

    addModule ("LADDER FILTER", { 1388, 286, 436, 198 });
    knob (prefix + "ladder.cutoff", "CUTOFF", 1444, 346, 58);
    knob (prefix + "ladder.resonance", "RESONANCE", 1546, 346, 44);
    knob (prefix + "ladder.drive", "DRIVE", 1648, 346, 44);
    addChoice (prefix + "ladder.slope", "SLOPE", { 1718, 328, 90, 34 });
    knob (prefix + "ladder.envAmt", "ENV AMT", 1444, 434, 44);
    knob (prefix + "ladder.keyTrack", "KEY TRACK", 1546, 434, 44);

    addModule ("ENVELOPE GENERATORS", { 1832, 24, 384, 460 });
    const char* envIds[] = { "delay", "attack", "hold", "decay", "sustain", "release", "velocity" };
    const char* envLabels[] = { "DELAY", "ATT", "HOLD", "DEC", "SUS", "REL", "VELO" };
    for (int env = 1; env <= 3; ++env)
    {
        const auto y = 100 + (env - 1) * 145;
        const auto stem = prefix + "env" + juce::String (env) + ".";
        addCaption (env == 1 ? "AMP" : (env == 2 ? "FILTER" : "MOD"), { 1840, y - 29, 50, 12 },
                    juce::Justification::centredLeft, 9.0f);
        for (int i = 0; i < 7; ++i)
            knob (stem + envIds[i], envLabels[i], 1860 + i * 54, y, 36);
        if (env < 3)
            addLed (stem + "loop", "LOOP", { 2128, y + 30, 72, 18 });
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
void OnePanel::updateConsolePage()
{
    for (auto* c : consoleControls_) c->setVisible (! modulationPage_);
    for (auto* c : modulationControls_) c->setVisible (modulationPage_);
}
} // namespace triad::ui
