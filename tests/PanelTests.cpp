#include "ui/Panels.h"
#include "ui/LookAndFeel.h"
#include "ui/KeyboardPanel.h"
#include "engines/Engines.h"

#include <iostream>
#include <set>
#include <stdexcept>

using namespace triad;
using namespace triad::ui;

namespace
{
void require (bool condition, const juce::String& message)
{
    if (! condition) throw std::runtime_error (message.toStdString());
}

// A device-free host: exercises the real panel attachments without opening audio.
class PanelHost final : public juce::AudioProcessor, public EditorHost
{
public:
    explicit PanelHost (InstrumentKind kind)
        : engine_ (makeEngine (kind)), state_ (*this, nullptr, "PARAMETERS", layout (engine_->definition())) {}
    static juce::AudioProcessorValueTreeState::ParameterLayout layout (const ModelDefinition& definition)
    {
        juce::AudioProcessorValueTreeState::ParameterLayout result;
        for (const auto& p : definition.params)
        {
            const juce::ParameterID id { p.id, 1 };
            if (p.curve == ParamCurve::Discrete)
            {
                juce::StringArray choices;
                for (const auto& c : p.choices) choices.add (c);
                result.add (std::make_unique<juce::AudioParameterChoice> (id, p.name, choices, (int) p.def));
            }
            else
            {
                juce::NormalisableRange<float> range (p.min, p.max);
                if (p.curve == ParamCurve::Log && p.min > 0.0f) range.setSkewForCentre (std::sqrt (p.min * p.max));
                result.add (std::make_unique<juce::AudioParameterFloat> (id, p.name, range, p.def));
            }
        }
        return result;
    }
    juce::AudioProcessorValueTreeState& apvts() override { return state_; }
    const ModelDefinition& definition() const override { return engine_->definition(); }
    SynthEngine& engine() override { return *engine_; }
    juce::MidiKeyboardState& keyboardState() override { return keyboard_; }
    PatternSequencer& sequencer() override { return sequencer_; }
    juce::String instrumentName() const override { return definition().displayName; }
    int selectedPart() const override { return part_; }
    void setSelectedPart (int n) override { part_ = n; }
    bool hasPatching() const override { return ! definition().patchPoints.empty(); }
    std::vector<PatchCable> patchCables() const override { return cables_; }
    void setPatchCable (int from, int to, bool on) override
    {
        if (on) cables_.push_back ({ from, to });
        else std::erase_if (cables_, [=] (auto c) { return c.from == from && c.to == to; });
    }
    void captureMorphB() override { ++captures; }
    void swapMorphB() override { ++swaps; }
    void setMorphPad (float x, float y, float z) override { morph = { x, y, z }; }
    float uiPitchBend() const override { return pitch; }
    float uiModWheel() const override { return modulation; }
    void setUiPitchBend (float v) override { pitch = v; }
    void setUiModWheel (float v) override { modulation = v; }
    float pitch = 0, modulation = 0;
    std::array<float, 3> morph {};
    int captures = 0, swaps = 0;

    const juce::String getName() const override { return "Panel tests"; }
    void prepareToPlay (double, int) override {}
    void releaseResources() override {}
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override {}
    double getTailLengthSeconds() const override { return 0.0; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    juce::AudioProcessorEditor* createEditor() override { return nullptr; }
    bool hasEditor() const override { return false; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}
    void getStateInformation (juce::MemoryBlock&) override {}
    void setStateInformation (const void*, int) override {}
private:
    std::unique_ptr<SynthEngine> engine_;
    juce::AudioProcessorValueTreeState state_;
    juce::MidiKeyboardState keyboard_;
    PatternSequencer sequencer_;
    std::vector<PatchCable> cables_;
    int part_ = 1;
};

juce::MouseEvent eventAt (juce::Component& c, float x, float y)
{
    const juce::Point<float> p { x, y };
    const auto now = juce::Time::getCurrentTime();
    return { juce::Desktop::getInstance().getMainMouseSource(), p, juce::ModifierKeys::leftButtonModifier,
             0.7f, 0, 0, 0, 0, &c, &c, now, p, now, 1, false };
}

void verifyPanel (PanelHost& host, PanelBase& panel)
{
    std::set<std::string> bound, jacks;
    std::vector<juce::Component*> interactive;
    int oleds = 0;
    for (auto* c : panel.getChildren())
    {
        require (panel.getLocalBounds().contains (c->getBounds()), "Outside panel: " + c->getName());
        const auto id = c->getName();
        if (auto* oled = dynamic_cast<OneWaveDisplay*> (c))
        {
            const auto stem = "one.p" + juce::String (host.selectedPart()) + ".vco" + juce::String (++oleds) + ".";
            auto* angle = host.apvts().getParameter (stem + "waveAngle");
            auto* shape = host.apvts().getParameter (stem + "shape");
            auto* pw = host.apvts().getParameter (stem + "pw");
            const auto centre = oled->getBounds().getCentre();
            const auto position = [&] (const char* suffix) { return panel.knobFor (stem + suffix).getBounds().getCentre(); };
            const auto octave = position ("octave"), frequency = position ("tune"), beat = position ("beatHz");
            const auto wave = position ("waveAngle"), pulse = position ("pw"), mix = position ("shape");
            require (octave.x == centre.x && mix.x == centre.x && octave.y < frequency.y && mix.y > wave.y,
                     "Point-up oscillator hexagon, top octave / bottom mix");
            require (frequency.x < centre.x && beat.x > centre.x && frequency.y == beat.y && frequency.y < centre.y,
                     "Hexagon upper left frequency / upper right beat");
            require (wave.x == frequency.x && pulse.x == beat.x && wave.y == pulse.y && wave.y > centre.y,
                     "Hexagon lower left wave angle / lower right pulse width");
            const auto a = angle->getValue(), s = shape->getValue(), w = pw->getValue();
            shape->setValueNotifyingHost (0); angle->setValueNotifyingHost (0);
            const auto saw = oled->waveform();
            angle->setValueNotifyingHost (0.5f); const auto triangle = oled->waveform();
            for (int n = 1; n < 85; ++n)
                require (std::abs (saw[(size_t) n] - saw[(size_t) n + 85]) < .0001f
                      && std::abs (saw[(size_t) n] - saw[(size_t) n + 170]) < .0001f,
                         "OLED contains three identical periods");
            require (saw != triangle, "OLED follows oscillator angle / selected part");
            shape->setValueNotifyingHost (1); pw->setValueNotifyingHost (0.25f);
            const auto narrow = oled->waveform(); pw->setValueNotifyingHost (0.75f);
            require (narrow != oled->waveform() && narrow != triangle, "OLED follows pulse shape/width");
            angle->setValueNotifyingHost (a); shape->setValueNotifyingHost (s); pw->setValueNotifyingHost (w);
        }
        for (auto* child : c->getChildren())
            if (auto* wheel = dynamic_cast<juce::Slider*> (child))
            {
                if (wheel->getName() == "Pitch wheel")
                {
                    wheel->setValue (0.5f, juce::sendNotificationSync);
                    require (host.pitch == 0.5f, "Upper panel pitch wheel");
                    wheel->setValue (0, juce::sendNotificationSync);
                }
                if (wheel->getName() == "Mod wheel")
                {
                    wheel->setValue (0.75f, juce::sendNotificationSync);
                    require (host.modulation == 0.75f, "Upper panel modulation wheel");
                    wheel->setValue (0, juce::sendNotificationSync);
                }
            }
        if (auto* jack = dynamic_cast<Jack*> (c))
        {
            require (jack->patchIndex() >= 0, "Unknown jack: " + id);
            jacks.insert (id.toStdString());
        }
        if (id.startsWith ("mat.") || id.startsWith ("poly.") || id.startsWith ("one.") || id.startsWith ("con."))
        {
            if (dynamic_cast<Jack*> (c) != nullptr) continue;
            auto* parameter = host.apvts().getParameter (id);
            require (parameter != nullptr, "Unbound control: " + id);
            bound.insert (id.toStdString());
            interactive.push_back (c);
            if (auto* slider = dynamic_cast<juce::Slider*> (c))
            {
                const auto saved = parameter->getValue();
                parameter->setValueNotifyingHost (0.0f);
                require (std::abs (slider->getValue() - parameter->convertFrom0to1 (0.0f)) < 0.01, "Host -> slider: " + id);
                slider->setValue (parameter->convertFrom0to1 (1.0f), juce::sendNotificationSync);
                require (std::abs (parameter->getValue() - 1.0f) < 0.001f, "Slider -> host: " + id);
                if ((bool) slider->getProperties()["numericField"])
                {
                    const auto v = parameter->convertFrom0to1 (0.57f);
                    const auto roundtrip = slider->getValueFromText (slider->getTextFromValue (v));
                    require (std::abs (roundtrip - v) <= std::max (0.001, std::abs (v) * 0.001), "Numeric units: " + id);
                }
                parameter->setValueNotifyingHost (saved);
            }
            if (auto* combo = dynamic_cast<juce::ComboBox*> (c))
            {
                const auto saved = parameter->getValue();
                combo->setSelectedId (combo->getNumItems(), juce::sendNotificationSync);
                require (std::abs (parameter->getValue() - 1.0f) < 0.001f, "Choice -> host: " + id);
                parameter->setValueNotifyingHost (saved);
            }
        }
        if (auto* matrix = dynamic_cast<SourceMatrix*> (c))
        {
            for (int i = 1; i <= 8; ++i) bound.insert ("poly.mod.slot" + std::to_string (i) + ".source");
            matrix->mouseDown (eventAt (*matrix, 4, 1));
            require (host.apvts().getRawParameterValue ("poly.mod.slot1.source")->load() == 0, "Source matrix assignment");
        }
        if (id == "Delay-LPF R1-R6")
        {
            const char* ids[] = { "con.fx.delay.lpf", "con.fx.delay.offset", "con.fx.delay.pingpong", "con.fx.delay.fb", "con.fx.delay.full", "con.fx.delay.time" };
            for (int i = 0; i < 6; ++i)
            {
                bound.insert (ids[i]);
                auto* p = host.apvts().getParameter (ids[i]); const auto saved = p->getValue();
                const auto x = (i + .5f) * c->getWidth() / 6.0f;
                c->mouseDown (eventAt (*c, x, 0));
                require (p->getValue() == 1.0f, "Delay-LPF upper endpoint");
                // Dragging across columns must keep editing the originally grabbed fader.
                c->mouseDrag (eventAt (*c, c->getWidth() - 1.0f, (float) c->getHeight()));
                require (p->getValue() == 0.0f, "Delay-LPF lower endpoint / captured column");
                c->mouseUp (eventAt (*c, x, (float) c->getHeight())); p->setValueNotifyingHost (saved);
            }
        }
    }
    // Custom controls: mode strip, performance pad and source assignment grid.
    require (host.definition().id != "moogone" || oleds == 3, "Three oscillator OLEDs per part");
    if (host.definition().id == "polybrute")
    {
        KeyboardPanel keyboard (host); keyboard.setSize (1680, 210);
        auto* pad = keyboard.findChildWithID ("morph-performance");
        require (pad != nullptr && pad->getRight() < 230, "MorphPad moved to keyboard-left recess");
        pad->mouseDown (eventAt (*pad, pad->getWidth() * .4f, pad->getHeight() * .4f));
        require (host.morph[0] > .1f && host.morph[0] < .9f && host.morph[2] == .7f, "Relocated pad X/Y/Z");
        juce::MouseWheelDetails wheel; wheel.deltaY = .1f;
        pad->mouseWheelMove (eventAt (*pad, 20, 20), wheel);
        require (host.morph[2] > .7f, "Relocated pad pressure fallback");
        pad->mouseUp (eventAt (*pad, 20, 20));
    }
    if (host.definition().id == "continuum") bound.insert ("con.fx.mode");
    if (host.definition().id == "moogone") bound.insert ("one.master.selectedPart");
    if (host.definition().id == "matriarch")
    {
        KeyboardPanel keyboard (host);
        keyboard.setSize (1200, 176);
        for (auto* c : keyboard.getChildren())
        {
            if (c->getName() == "mat.performance.glide")
            {
                auto* slider = dynamic_cast<juce::Slider*> (c);
                require (slider != nullptr, "Performance glide slider");
                slider->setValue (1.25, juce::sendNotificationSync);
                require (std::abs (host.apvts().getRawParameterValue (c->getName())->load() - 1.25f) < 0.001f,
                         "Performance glide binding");
                slider->setValue (0.0, juce::sendNotificationSync);
                bound.insert (c->getName().toStdString());
            }
            if (c->getName() == "mat.arp.enabled" || c->getName() == "mat.arp.hold")
            {
                auto* p = host.apvts().getParameter (c->getName()); const auto saved = p->getValue();
                c->mouseDown (eventAt (*c, 10, 10));
                require (p->getValue() != saved, "Performance PLAY/HOLD binding");
                p->setValueNotifyingHost (saved);
            }
        }
    }
    for (const auto& p : host.definition().params)
    {
        if (p.id.starts_with ("one.p") && ! p.id.starts_with ("one.p" + std::to_string (host.selectedPart()) + ".")) continue;
        require (bound.contains (p.id), "Missing UI parameter: " + juce::String (p.id));
    }
    for (size_t a = 0; a < interactive.size(); ++a)
        for (size_t b = a + 1; b < interactive.size(); ++b)
            require (! interactive[a]->isVisible() || ! interactive[b]->isVisible()
                     || ! interactive[a]->getBounds().intersects (interactive[b]->getBounds()),
                     "Controls overlap: " + interactive[a]->getName() + " / " + interactive[b]->getName());
    require (jacks.size() == host.definition().patchPoints.size(), "Patch point coverage");
    if (! jacks.empty())
    {
        Jack* from = nullptr; Jack* to = nullptr;
        for (auto* c : panel.getChildren())
            if (auto* j = dynamic_cast<Jack*> (c))
            {
                if (j->isOutput()) from = j; else to = j;
            }
        require (from != nullptr && to != nullptr, "Patch directions");
        from->onJackClicked (*from); to->onJackClicked (*to);
        require (host.patchCables().size() == 1, "Patch connection callback");
        to->onJackRightClicked (*to);
        require (host.patchCables().empty(), "Patch disconnect callback");
    }
    std::cout << host.instrumentName() << " part " << host.selectedPart() << ": " << bound.size()
              << " parameters covered, " << jacks.size() << " patch points; bindings/geometry OK\n";
}
} // namespace

int main (int argc, char** argv)
{
    try
    {
        juce::ScopedJuceInitialiser_GUI gui;
        TriadLookAndFeel look;
        for (int kind = 0; kind < 4; ++kind)
        {
            PanelHost host ((InstrumentKind) kind);
            look.setHardwareStyle (kind == 1 ? HardwareStyle::PolyBrute : (kind == 0 ? HardwareStyle::Matriarch : HardwareStyle::MoogOne));
            look.accent = TriadLookAndFeel::accentFor (host.instrumentName());
            auto panel = createInstrumentPanel (host); panel->setLookAndFeel (&look);
            for (int part = 1; part <= (kind == 2 ? 3 : 1); ++part)
            {
                host.setSelectedPart (part);
                if (auto* one = dynamic_cast<OnePanel*> (panel.get())) one->rebuildForPart();
                verifyPanel (host, *panel);
                if (argc > 1)
                {
                    const auto folder = juce::File (juce::String (argv[1]));
                    folder.createDirectory();
                    const auto file = folder.getChildFile (host.definition().id + "-part" + std::to_string (part) + ".png");
                    if (auto stream = file.createOutputStream())
                    {
                        stream->setPosition (0); stream->truncate();
                        juce::PNGImageFormat png;
                        png.writeImageToStream (panel->createComponentSnapshot (panel->getLocalBounds()), *stream);
                    }
                    if (kind == 0 || kind == 1)
                    {
                        KeyboardPanel keyboard (host); keyboard.setLookAndFeel (&look);
                        keyboard.setSize (1680, kind == 1 ? 210 : 176);
                        if (auto stream = folder.getChildFile (host.definition().id + "-keyboard.png").createOutputStream())
                        {
                            stream->setPosition (0); stream->truncate();
                            juce::PNGImageFormat png;
                            png.writeImageToStream (keyboard.createComponentSnapshot (keyboard.getLocalBounds()), *stream);
                        }
                        keyboard.setLookAndFeel (nullptr);
                    }
                }
            }
            if (auto* continuum = dynamic_cast<ContinuumPanel*> (panel.get()))
                for (int page = 1; page < 4; ++page)
                {
                    continuum->showDetailPage (page); verifyPanel (host, *panel);
                    if (argc > 1)
                    {
                        const auto file = juce::File (juce::String (argv[1])).getChildFile ("continuum-page" + juce::String (page) + ".png");
                        if (auto stream = file.createOutputStream())
                        {
                            stream->setPosition (0); stream->truncate(); juce::PNGImageFormat png;
                            png.writeImageToStream (panel->createComponentSnapshot (panel->getLocalBounds()), *stream);
                        }
                    }
                }
            if (auto* one = dynamic_cast<OnePanel*> (panel.get()))
            {
                for (auto* c : one->getChildren())
                    if (auto* button = dynamic_cast<juce::TextButton*> (c); button != nullptr && button->getButtonText() == "MOD / HOME")
                    {
                        button->onClick(); verifyPanel (host, *panel);
                        require (panel->knobFor ("one.p3.mod.slot1.amount").isVisible(), "MOD page exposes bindings");
                        if (argc > 1)
                        {
                            const auto file = juce::File (juce::String (argv[1])).getChildFile ("moogone-mod.png");
                            if (auto stream = file.createOutputStream())
                            {
                                stream->setPosition (0); stream->truncate(); juce::PNGImageFormat png;
                                png.writeImageToStream (panel->createComponentSnapshot (panel->getLocalBounds()), *stream);
                            }
                        }
                        button->onClick();
                    }
                require (panel->knobFor ("one.p3.lfo1.rate").getX() < panel->knobFor ("one.p3.vco1.tune").getX(), "LFO strip left of oscillators");
                require (panel->knobFor ("one.p3.ladder.cutoff").getY() > panel->knobFor ("one.p3.svf.cutoff").getY(), "Ladder below SVF");
            }
            panel->setLookAndFeel (nullptr);
        }
        return 0;
    }
    catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
}
