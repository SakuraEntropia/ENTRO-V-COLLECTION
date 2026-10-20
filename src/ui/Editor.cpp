#include "ui/Editor.h"

#include <juce_gui_extra/juce_gui_extra.h>

namespace triad::ui
{

class TriadEditor::HeaderBar final : public juce::Component, private juce::Timer
{
public:
    explicit HeaderBar (EditorHost& host)
        : host_ (host)
    {
        presetBox_.setTooltip ("Factory presets");
        presetBox_.addItem ("INIT", 1);

        for (int i = 0; i < host_.factoryPresetCount(); ++i)
            presetBox_.addItem (host_.factoryPresetName (i), i + 2);

        presetBox_.setSelectedId (1, juce::dontSendNotification);
        presetBox_.onChange = [this]
        {
            const auto id = presetBox_.getSelectedId();
            if (id == 1)
                host_.initPatch();
            else if (id > 1)
                host_.loadFactoryPreset (id - 2);
        };

        addAndMakeVisible (presetBox_);

        initButton_.setButtonText ("INIT");
        initButton_.setTooltip ("Reset every parameter to its default value");
        initButton_.onClick = [this]
        {
            host_.initPatch();
            presetBox_.setSelectedId (1, juce::dontSendNotification);
        };

        saveButton_.setButtonText ("SAVE");
        saveButton_.setTooltip ("Save the current panel as a user preset file");
        saveButton_.onClick = [this] { host_.savePreset(); };

        loadButton_.setButtonText ("LOAD");
        loadButton_.setTooltip ("Load a user preset file");
        loadButton_.onClick = [this] { host_.loadPreset(); };

        addAndMakeVisible (initButton_);
        addAndMakeVisible (saveButton_);
        addAndMakeVisible (loadButton_);

        startTimerHz (6);
    }

    ~HeaderBar() override
    {
        stopTimer();
    }

    void paint (juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        juce::ColourGradient background (juce::Colour (0xff1b1b1f), 0.0f, 0.0f,
                                         juce::Colour (0xff101012), 0.0f, bounds.getBottom(), false);
        g.setGradientFill (background);
        g.fillRect (bounds);

        g.setColour (juce::Colour (0xffffffff).withAlpha (0.07f));
        g.drawLine (0.0f, bounds.getBottom() - 1.0f, bounds.getWidth(), bounds.getBottom() - 1.0f, 1.0f);

        // Analog Triad mark: three stacked triangles.
        auto logo = juce::Rectangle<float> (26.0f, 14.0f, 26.0f, 22.0f);
        g.setColour (accent_.withAlpha (0.85f));
        for (int i = 0; i < 3; ++i)
        {
            const auto y = logo.getY() + (float) i * 6.0f;
            juce::Path p;
            p.addTriangle (logo.getX() + (float) i * 2.0f, y + 5.0f,
                           logo.getRight() - (float) i * 2.0f, y + 5.0f,
                           logo.getCentreX(), y);
            g.strokePath (p, juce::PathStrokeType (1.2f));
        }

        g.setColour (juce::Colour (0xffe9e6df));
        g.setFont (juce::Font (juce::FontOptions (17.0f, juce::Font::bold)));
        g.drawText (host_.instrumentName().toUpperCase(), 62, 8, 320, 22,
                    juce::Justification::centredLeft);

        const auto id = juce::String (host_.definition().id);
        const auto subtitle = id == "matriarch" ? "SEMI-MODULAR ANALOG SYNTHESIZER"
                              : (id == "polybrute" ? "SIX-VOICE ANALOG SYNTHESIZER"
                                                   : "POLYPHONIC ANALOG SYNTHESIZER");
        g.setColour (juce::Colour (0xff9a968d));
        g.setFont (juce::Font (juce::FontOptions (8.5f)).withExtraKerningFactor (0.16f));
        g.drawText (subtitle, 62, 27, 420, 12, juce::Justification::centredLeft);

        g.setColour (accent_);
        g.fillRect (29.0f, bounds.getBottom() - 3.0f, bounds.getWidth() - 58.0f, 1.5f);

        g.setColour (juce::Colour (0xff8f8c85));
        g.setFont (juce::Font (juce::FontOptions (8.0f)));
        g.drawText ("PRESET", (int) presetBox_.getX() - 44, presetBox_.getY() + 3, 42, 12,
                    juce::Justification::centredRight);
    }

    void resized() override
    {
        auto area = getLocalBounds().reduced (12, 11);

        loadButton_.setBounds (area.removeFromRight (54).reduced (2, 0));
        saveButton_.setBounds (area.removeFromRight (58).reduced (2, 0));
        initButton_.setBounds (area.removeFromRight (50).reduced (2, 0));
        presetBox_.setBounds (area.removeFromRight (220).reduced (4, 0));
    }

    void refresh()
    {
        const auto name = host_.currentPresetName();

        for (int i = 0; i < presetBox_.getNumItems(); ++i)
        {
            if (presetBox_.getItemText (i) == name)
            {
                if (presetBox_.getSelectedItemIndex() != i)
                    presetBox_.setSelectedItemIndex (i, juce::dontSendNotification);
                return;
            }
        }
    }

    void setAccent (juce::Colour colour)
    {
        accent_ = colour;
        repaint();
    }

private:
    void timerCallback() override
    {
        refresh();
    }

    EditorHost& host_;
    juce::ComboBox presetBox_;
    juce::TextButton initButton_, saveButton_, loadButton_;
    juce::Colour accent_ { 0xffcf4b2e };
};

// ---------------------------------------------------------------------------
// Instrument panel factory
// ---------------------------------------------------------------------------

std::unique_ptr<PanelBase> createInstrumentPanel (EditorHost& host)
{
    const auto id = juce::String (host.definition().id);

    if (id == "polybrute")
        return std::make_unique<PolyBrutePanel> (host);

    if (id == "moogone" || id == "one")
        return std::make_unique<OnePanel> (host);

    return std::make_unique<MatriarchPanel> (host);
}

// ---------------------------------------------------------------------------
// Editor
// ---------------------------------------------------------------------------

TriadEditor::TriadEditor (EditorHost& host)
    : juce::AudioProcessorEditor (&host.apvts().processor),
      host_ (host),
      keyboard_ (host)
{
    const auto instrumentId = juce::String (host.definition().id);
    lookAndFeel_.accent = TriadLookAndFeel::accentFor (host.definition().displayName);
    lookAndFeel_.setHardwareStyle (instrumentId == "polybrute"
                                       ? HardwareStyle::PolyBrute
                                       : (instrumentId == "moogone" ? HardwareStyle::MoogOne
                                                                     : HardwareStyle::Matriarch));
    setLookAndFeel (&lookAndFeel_);

    header_ = std::make_unique<HeaderBar> (host);
    header_->setAccent (lookAndFeel_.accent);
    addAndMakeVisible (*header_);

    panel_ = createInstrumentPanel (host);
    addAndMakeVisible (*panel_);

    addAndMakeVisible (keyboard_);

    const auto id = juce::String (host.definition().id);

    int defaultWidth = 1600;
    int defaultHeight = 640;

    if (id == "polybrute")
    {
        defaultWidth = 1680;
        defaultHeight = 1000;
    }
    else if (id == "moogone" || id == "one")
    {
        defaultWidth = 1680;
        defaultHeight = 800;
    }

    auto maxWidth = 3000;
    auto maxHeight = 1800;

    if (const auto* display = juce::Desktop::getInstance().getDisplays().getPrimaryDisplay())
    {
        maxWidth = juce::jmax (1100, display->userArea.getWidth() - 24);
        maxHeight = juce::jmax (640, display->userArea.getHeight() - 100);
        defaultWidth = juce::jmin (defaultWidth, maxWidth);
        defaultHeight = juce::jmin (defaultHeight, maxHeight);
    }

    setResizable (true, true);
    setResizeLimits (1000, 560, maxWidth, maxHeight);
    setSize (juce::jmax (1000, defaultWidth), juce::jmax (560, defaultHeight));
    getConstrainer()->setMinimumOnscreenAmounts (300, 300, 300, 300);

    startTimerHz (8);
}

TriadEditor::~TriadEditor()
{
    stopTimer();
    setLookAndFeel (nullptr);
}

void TriadEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff0a0a0b));
}

void TriadEditor::resized()
{
    auto area = getLocalBounds();
    header_->setBounds (area.removeFromTop (52));
    keyboard_.setBounds (area.removeFromBottom (132));

    auto panelArea = area.reduced (10, 6);

    if (panel_ == nullptr || panel_->designWidth() <= 0 || panel_->designHeight() <= 0)
        return;

    const auto designW = (float) panel_->designWidth();
    const auto designH = (float) panel_->designHeight();
    const auto scale = juce::jmin ((float) panelArea.getWidth() / designW,
                                   (float) panelArea.getHeight() / designH);

    const auto scaledW = designW * scale;
    const auto scaledH = designH * scale;
    const auto offsetX = (float) panelArea.getX() + ((float) panelArea.getWidth() - scaledW) * 0.5f;
    const auto offsetY = (float) panelArea.getY() + ((float) panelArea.getHeight() - scaledH) * 0.5f;

    panel_->setBounds (0, 0, panel_->designWidth(), panel_->designHeight());
    panel_->setTransform (juce::AffineTransform::scale (scale).translated (offsetX, offsetY));
}

void TriadEditor::timerCallback()
{
    if (header_ != nullptr)
        header_->refresh();

    keyboard_.refresh();

    if (panel_ != nullptr)
        panel_->pollPatchCables();

    if (auto* onePanel = dynamic_cast<OnePanel*> (panel_.get()))
    {
        static int lastPart = host_.selectedPart();
        if (lastPart != host_.selectedPart())
        {
            lastPart = host_.selectedPart();
            onePanel->rebuildForPart();
        }
    }
}

} // namespace triad::ui
