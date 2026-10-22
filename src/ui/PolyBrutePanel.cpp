#include "ui/Panels.h"
#include "ui/LookAndFeel.h"

namespace triad::ui
{

namespace
{
juce::Colour polyAccent() { return juce::Colour (0xff3f93e8); }
}

class MorphPad final : public juce::Component, private juce::Timer
{
public:
    explicit MorphPad (EditorHost& host)
        : host_ (host)
    {
        pressure_.setSliderStyle (juce::Slider::LinearVertical);
        pressure_.setRange (0.0, 1.0, 0.001);
        pressure_.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        pressure_.setValue (0.0, juce::dontSendNotification);
        pressure_.setColour (juce::Slider::backgroundColourId, juce::Colour (0xff15181a));
        pressure_.setColour (juce::Slider::trackColourId, juce::Colour (0xff2fb6a8));
        pressure_.setColour (juce::Slider::thumbColourId, juce::Colour (0xffe9e6df));
        pressure_.setTooltip ("Pressure / Z (mouse fallback; force-touch updates automatically)");
        pressure_.onValueChange = [this]
        {
            pressureValue_ = (float) pressure_.getValue();
            host_.setMorphPad (x_, y_, pressureValue_);
            repaint();
        };
        addAndMakeVisible (pressure_);
        setWantsKeyboardFocus (false);
        startTimerHz (15);
    }

    ~MorphPad() override { stopTimer(); }

    void paint (juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        auto pad = padArea();

        g.setColour (juce::Colour (0xff07090b));
        g.fillRoundedRectangle (pad, 4.0f);
        g.setColour (juce::Colour (0xff2fb6a8).withAlpha (0.18f));
        g.drawRoundedRectangle (pad.reduced (0.5f), 4.0f, 1.0f);

        for (int i = 1; i < 4; ++i)
        {
            const auto x = pad.getX() + pad.getWidth() * (float) i / 4.0f;
            const auto y = pad.getY() + pad.getHeight() * (float) i / 4.0f;
            g.setColour (juce::Colour (0xff5fd6d0).withAlpha (0.11f));
            g.drawVerticalLine ((int) x, pad.getY(), pad.getBottom());
            g.drawHorizontalLine ((int) y, pad.getX(), pad.getRight());
        }

        const auto point = juce::Point<float> (pad.getX() + x_ * pad.getWidth(),
                                                pad.getY() + (1.0f - y_) * pad.getHeight());
        const auto radius = 5.0f + pressureValue_ * 10.0f;
        g.setColour (juce::Colour (0xff5fd6d0).withAlpha (0.14f));
        g.fillEllipse (point.x - radius, point.y - radius, radius * 2.0f, radius * 2.0f);
        g.setColour (juce::Colour (0xffdffbf6));
        g.fillEllipse (point.x - 4.0f, point.y - 4.0f, 8.0f, 8.0f);
        g.setColour (juce::Colour (0xff2fb6a8));
        g.drawLine (pad.getX(), point.y, pad.getRight(), point.y, 0.7f);
        g.drawLine (point.x, pad.getY(), point.x, pad.getBottom(), 0.7f);

        g.setColour (juce::Colour (0xff7fd8d0));
        g.setFont (juce::Font (juce::FontOptions (7.5f)));
        g.drawText ("X MORPH", (int) pad.getX() + 5, (int) pad.getBottom() - 13, 60, 10,
                    juce::Justification::centredLeft);
        g.drawText ("Y SPREAD", (int) pad.getRight() - 64, (int) pad.getY() + 3, 60, 10,
                    juce::Justification::centredRight);
        g.drawText ("Z", (int) pad.getRight() + 12, (int) pad.getY() + 2, 20, 10,
                    juce::Justification::centredRight);
    }

    void resized() override
    {
        auto area = getLocalBounds().reduced (4);
        pressure_.setBounds (area.removeFromRight (34).reduced (3, 0));
    }

    void mouseDown (const juce::MouseEvent& event) override
    {
        if (! padArea().contains (event.position))
            return;

        dragging_ = true;
        updateFromEvent (event);
    }

    void mouseDrag (const juce::MouseEvent& event) override
    {
        if (dragging_)
            updateFromEvent (event);
    }

    void mouseUp (const juce::MouseEvent&) override
    {
        if (! dragging_)
            return;

        dragging_ = false;

        // On a pressure-capable touchpad, releasing the finger ends the
        // expression.  A mouse-only user keeps the manual pressure slider.
        if (pressureWasValid_)
        {
            pressureValue_ = 0.0f;
            pressure_.setValue (0.0, juce::dontSendNotification);
            host_.setMorphPad (x_, y_, 0.0f);
        }

        repaint();
    }

    void mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails& wheel) override
    {
        pressureValue_ = juce::jlimit (0.0f, 1.0f, pressureValue_ + wheel.deltaY * 0.6f);
        pressure_.setValue (pressureValue_, juce::dontSendNotification);
        host_.setMorphPad (x_, y_, pressureValue_);
        repaint();
    }

private:
    juce::Rectangle<float> padArea() const
    {
        return getLocalBounds().toFloat().reduced (4.0f).withTrimmedRight (40.0f);
    }

    void updateFromEvent (const juce::MouseEvent& event)
    {
        auto pad = padArea();
        x_ = juce::jlimit (0.0f, 1.0f, (event.position.x - pad.getX()) / pad.getWidth());
        y_ = juce::jlimit (0.0f, 1.0f, 1.0f - (event.position.y - pad.getY()) / pad.getHeight());

        pressureWasValid_ = event.isPressureValid();
        if (pressureWasValid_)
        {
            pressureValue_ = juce::jlimit (0.0f, 1.0f, event.pressure);
            pressure_.setValue (pressureValue_, juce::dontSendNotification);
        }

        host_.setMorphPad (x_, y_, pressureValue_);
        repaint();
    }

    void timerCallback() override
    {
        if (dragging_)
            return;

        if (auto* morph = host_.apvts().getRawParameterValue ("poly.voice.morph"))
            x_ = juce::jlimit (0.0f, 1.0f, morph->load());
        if (auto* spread = host_.apvts().getRawParameterValue ("poly.voice.spread"))
            y_ = juce::jlimit (0.0f, 1.0f, spread->load());

        repaint();
    }

    EditorHost& host_;
    juce::Slider pressure_;
    float x_ = 0.5f;
    float y_ = 0.5f;
    float pressureValue_ = 0.0f;
    bool dragging_ = false;
    bool pressureWasValid_ = false;
};

PolyBrutePanel::PolyBrutePanel (EditorHost& editorHost)
    : PanelBase (editorHost, "POLYBRUTE V", 2360, 600, polyAccent())
{
    setBranding ("POLYBRUTE V", "SIX-VOICE ANALOG SYNTHESIZER");
    const juce::Rectangle<int> performance { 24, 16, 176, 516 }, voice { 206, 16, 140, 516 },
        vco1 { 352, 16, 228, 168 }, mixer { 586, 16, 88, 168 },
        steiner { 680, 16, 236, 168 }, env1 { 922, 16, 230, 168 }, lfo1 { 1158, 16, 242, 168 },
        vco2 { 352, 190, 228, 168 }, ladder { 586, 190, 324, 168 },
        env2 { 916, 190, 230, 168 }, lfo2 { 1152, 190, 248, 168 },
        fm { 352, 364, 318, 168 }, routing { 676, 364, 240, 168 },
        env3 { 922, 364, 230, 168 }, lfo3 { 1158, 364, 242, 168 },
        matrix { 1410, 16, 430, 516 }, fx { 1850, 16, 486, 344 }, arp { 1850, 368, 486, 164 };
    for (const auto& m : std::vector<std::pair<juce::String, juce::Rectangle<int>>> {
        { "PERFORMANCE", performance }, { "VOICE", voice }, { "VCO 1", vco1 }, { "MIXER", mixer },
        { "STEINER FILTER", steiner }, { "VCF ENV", env1 }, { "LFO 1", lfo1 },
        { "VCO 2", vco2 }, { "LADDER FILTER", ladder }, { "VCA ENV", env2 }, { "LFO 2", lfo2 },
        { "FILTER FM / NOISE", fm }, { "ROUTING", routing }, { "MOD ENV", env3 }, { "LFO 3", lfo3 },
        { "MATRIX", matrix }, { "EFFECTS", fx }, { "ARPEGGIATOR", arp } })
        addModule (m.first, m.second);

    auto knob = [this] (const juce::String& id, const char* label, int x, int y, int d = 42)
    { addKnob (id, label, { x, y }, d); };
    addButton ("CAPTURE B", { 38, 52, 148, 26 }, [this] { host().captureMorphB(); });
    addButton ("SWAP A/B", { 38, 86, 148, 26 }, [this] { host().swapMorphB(); });
    knob ("poly.voice.morph", "MORPH A / B", 112, 165, 60);
    knob ("poly.voice.spread", "SPREAD", 72, 248);
    knob ("poly.voice.analog", "ANALOG", 152, 248);
    addCaption ("MORPH PERFORMANCE", { 32, 298, 160, 18 }, juce::Justification::centred, 11.0f,
                juce::Colour (0xffe6e1d6), true);
    morphPad_ = std::make_unique<MorphPad> (host());
    morphPad_->setBounds (32, 322, 160, 158);
    addAndMakeVisible (*morphPad_);
    addCaption ("X MORPH   Y SPREAD\nZ PRESSURE / WHEEL", { 32, 488, 160, 32 },
                juce::Justification::centred, 10.0f);
    addChoice ("poly.voice.mode", "VOICE MODE", { 218, 52, 116, 36 });
    addChoice ("poly.voice.unison", "UNISON", { 218, 113, 116, 36 });
    knob ("poly.voice.glide", "GLIDE", 276, 201, 48);
    addChoice ("poly.voice.pitchBendRange", "BEND RANGE", { 218, 257, 116, 36 });
    knob ("poly.voice.spread", "SPREAD", 276, 350, 48);
    knob ("poly.voice.analog", "ANALOG", 276, 447, 48);

    auto oscillator = [&] (const char* prefix, juce::Rectangle<int> area, bool second)
    {
        const juce::String stem (prefix);
        const auto x = area.getX(); const auto y = area.getY();
        knob (stem + "tune", "TUNE", x + 40, y + 52);
        knob (stem + (second ? "sub" : "metalizer"), second ? "SUB OSC" : "METALIZER", x + 114, y + 52);
        knob (stem + "pw", "PULSE WIDTH", x + 188, y + 52);
        knob (stem + "wave", "SAW / TRI", x + 40, y + 113);
        knob (stem + "pulse", "PULSE", x + 114, y + 113);
        knob (stem + "level", "LEVEL", x + 188, y + 113);
        addChoice (stem + "octave", "", { x + 12, y + 147, 84, 18 });
        if (second)
        {
            addLed (stem + "sync", "SYNC", { x + 102, y + 147, 54, 18 });
            // FM amount is in the FILTER FM block, alongside filter modulation.
            knob (stem + "fm", "VCO 2 > 1", fm.getX() + 52, fm.getY() + 70, 48);
        }
    };
    oscillator ("poly.vcoA.", vco1, false);
    oscillator ("poly.vcoB.", vco2, true);
    knob ("poly.mixer.noise", "NOISE", 630, 65);
    addSwitch ("poly.mixer.noiseColor", "COLOR", { 594, 118, 72, 35 });

    const char* steinerIds[] = { "mode", "cutoff", "resonance", "envAmt", "fm", "keyTrack", "level" };
    const char* steinerLabels[] = { "MODE", "CUTOFF", "RESO", "ENV AMT", "FILTER FM", "KEY TRACK", "LEVEL" };
    for (int i = 0; i < 7; ++i)
        knob ("poly.steiner." + juce::String (steinerIds[i]), steinerLabels[i],
              710 + i % 4 * 58, 66 + i / 4 * 68, 38);
    const char* ladderIds[] = { "cutoff", "resonance", "drive", "envAmt", "keyTrack", "level" };
    const char* ladderLabels[] = { "CUTOFF", "RESO", "BRUTE", "ENV AMT", "KEY TRACK", "LEVEL" };
    for (int i = 0; i < 6; ++i)
        knob ("poly.ladder." + juce::String (ladderIds[i]), ladderLabels[i],
              640 + i % 3 * 108, 240 + i / 3 * 68, i == 0 ? 48 : 42);
    knob ("poly.steiner.fm", "VCO 2 > VCF", 510, 434, 48);
    knob ("poly.mixer.noise", "NOISE", 614, 434, 48);
    addSwitch ("poly.mixer.noiseColor", "NOISE COLOR", { 454, 491, 128, 32 });
    addChoice ("poly.filters.routing", "FILTER ROUTING", { 690, 411, 146, 36 });
    knob ("poly.filters.balance", "BALANCE", 793, 482, 48);

    auto envelope = [&] (int n, juce::Rectangle<int> area)
    {
        const char* ids[] = { "velocity", "attack", "decay", "sustain", "release" };
        const char* names[] = { "VELO", "ATT", "DEC", "SUS", "REL" };
        for (int i = 0; i < 5; ++i)
            addFader ("poly.env" + juce::String (n) + "." + ids[i], names[i],
                      { area.getX() + 8 + i * 43, area.getY() + 32, 42, 128 });
    };
    envelope (1, env1); envelope (2, env2); envelope (3, env3);
    auto lfo = [&] (int n, juce::Rectangle<int> area, const char* amount, const char* label)
    {
        const auto stem = "poly.lfo" + juce::String (n) + ".";
        const auto x = area.getX(); const auto y = area.getY();
        knob (stem + "rate", "RATE", x + 42, y + 56);
        knob (stem + "fade", "FADE", x + 121, y + 56);
        knob (amount, label, x + 200, y + 56);
        addChoice (stem + "wave", "WAVEFORM", { x + 14, y + 106, 108, 35 });
        addLed (stem + "sync", "SYNC", { x + 132, y + 108, 94, 21 });
        addLed (stem + "retrigger", "RETRIGGER", { x + 132, y + 138, 94, 21 });
    };
    lfo (1, lfo1, "poly.lfo1.pitchAmt", "PITCH");
    lfo (2, lfo2, "poly.lfo2.pwmAmt", "PWM");
    lfo (3, lfo3, "poly.lfo3.cutoffAmt", "CUTOFF");

    auto& lcd = addLcd ({ 1424, 44, 402, 70 });
    lcd.setText ("POLYBRUTE V  /  MOD MATRIX", "SOURCE x SLOT", "MORPH A / B     6 VOICE     8 SLOTS");
    std::vector<juce::String> sources;
    for (int slot = 1; slot <= 8; ++slot)
        sources.push_back ("poly.mod.slot" + juce::String (slot) + ".source");
    addSourceMatrix (sources, 12, { 1490, 144, 184, 360 });
    // Labels come from the actual source enumeration, including None.
    if (const auto* spec = definition().find ("poly.mod.slot1.source"))
        for (int i = 0; i < (int) spec->choices.size() && i < 12; ++i)
            addCaption (spec->choices[(size_t) i], { 1418, 144 + i * 30, 68, 30 },
                        juce::Justification::centredRight, 9.0f);
    for (int slot = 1; slot <= 8; ++slot)
    {
        const auto y = 138 + (slot - 1) * 47;
        const auto stem = "poly.mod.slot" + juce::String (slot) + ".";
        addChoice (stem + "dest", "", { 1684, y + 4, 100, 26 });
        knob (stem + "amount", "AMT", 1807, y + 19, 30);
    }

    // Right-hand effects block: rows of controls above the sequencer/arp.
    addChoice ("poly.fx.dist.type", "DISTORTION", { 1868, 54, 154, 35 });
    knob ("poly.fx.dist.drive", "DRIVE", 2090, 73);
    knob ("poly.fx.dist.mix", "MIX", 2246, 73);
    const char* suffix[] = { "rate", "depth", "mix" };
    const char* labels[] = { "CHORUS RATE", "DEPTH", "MIX" };
    for (int i = 0; i < 3; ++i)
        knob ("poly.fx.chorus." + juce::String (suffix[i]), labels[i], 1926 + i * 160, 143);
    const char* delayIds[] = { "time", "feedback", "mix" };
    const char* delayLabels[] = { "DELAY TIME", "FEEDBACK", "MIX" };
    for (int i = 0; i < 3; ++i)
        knob ("poly.fx.delay." + juce::String (delayIds[i]), delayLabels[i], 1926 + i * 160, 213);
    addLed ("poly.fx.delay.pingPong", "PING PONG", { 1868, 252, 132, 22 });
    addLed ("poly.fx.delay.sync", "TEMPO SYNC", { 2010, 252, 132, 22 });
    knob ("poly.fx.reverb.size", "REVERB SIZE", 1926, 306);
    knob ("poly.fx.reverb.damping", "DAMPING", 2086, 306);
    knob ("poly.fx.reverb.mix", "MIX", 2246, 306);

    addLed ("poly.arp.enabled", "ARP", { 1868, 407, 74, 23 });
    addLed ("poly.arp.hold", "HOLD", { 1950, 407, 74, 23 });
    addChoice ("poly.arp.mode", "MODE", { 2040, 405, 130, 36 });
    addChoice ("poly.arp.rate", "RATE", { 2182, 405, 136, 36 });
    addSwitch ("poly.arp.octaves", "OCTAVES", { 1868, 470, 198, 36 });
    addChoice ("poly.arp.length", "LENGTH", { 2100, 470, 218, 36 });
}

void PolyBrutePanel::paint (juce::Graphics& g) { PanelBase::paint (g); }
PolyBrutePanel::~PolyBrutePanel() = default;
} // namespace triad::ui
