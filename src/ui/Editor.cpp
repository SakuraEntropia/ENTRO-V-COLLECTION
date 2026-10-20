#include "ui/Editor.h"

#include <juce_gui_extra/juce_gui_extra.h>

#include <cstdio>
#include <cstdlib>

namespace triad::ui
{

class TriadEditor::HeaderBar final : public juce::Component, private juce::Timer
{
public:
    explicit HeaderBar (EditorHost& host)
        : host_ (host)
    {
        presetBox_.setTooltip ("Factory presets by category");
        presetBox_.addItem ("INIT", 1);

        juce::String lastCategory;

        for (int i = 0; i < host_.factoryPresetCount(); ++i)
        {
            const auto category = host_.factoryPresetCategory (i);

            if (category.isNotEmpty() && category != lastCategory)
            {
                presetBox_.addSectionHeading (category);
                lastCategory = category;
            }

            presetBox_.addItem (host_.factoryPresetName (i), i + 2);
        }

        presetBox_.setSelectedId (1, juce::dontSendNotification);

        if (std::getenv ("TRIAD_LAYOUT") != nullptr)
        {
            std::fprintf (stderr, "PRESET ITEMS for %s:\n", host_.instrumentName().toRawUTF8());
            for (int i = 0; i < presetBox_.getNumItems(); ++i)
                std::fprintf (stderr, "  id=%d  %s\n", presetBox_.getItemId (i),
                              presetBox_.getItemText (i).toRawUTF8());
        }
        presetBox_.onChange = [this]
        {
            const auto id = presetBox_.getSelectedId();
            if (id == 1)
                host_.initPatch();
            else if (id > 1)
                host_.loadFactoryPreset (id - 2);
        };

        addAndMakeVisible (presetBox_);

        browseButton_.setButtonText ("BROWSE");
        browseButton_.setTooltip ("Open the searchable factory preset browser");
        browseButton_.onClick = [this] { if (onBrowse != nullptr) onBrowse(); };
        addAndMakeVisible (browseButton_);

        initButton_.setButtonText ("INIT");
        initButton_.setTooltip ("Reset every parameter to its default value");
        initButton_.onClick = [this]
        {
            host_.initPatch();
            presetBox_.setSelectedId (1, juce::dontSendNotification);

        if (std::getenv ("TRIAD_LAYOUT") != nullptr)
        {
            std::fprintf (stderr, "PRESET ITEMS for %s:\n", host_.instrumentName().toRawUTF8());
            for (int i = 0; i < presetBox_.getNumItems(); ++i)
                std::fprintf (stderr, "  id=%d  %s\n", presetBox_.getItemId (i),
                              presetBox_.getItemText (i).toRawUTF8());
        }
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

        zoomOutButton_.setButtonText ("-");
        zoomOutButton_.setTooltip ("Zoom out");
        zoomOutButton_.onClick = [this] { if (onZoomStep != nullptr) onZoomStep (-1); };
        fitButton_.setButtonText ("FIT");
        fitButton_.setTooltip ("Fit the whole panel");
        fitButton_.onClick = [this] { if (onFit != nullptr) onFit(); };
        zoomInButton_.setButtonText ("+");
        zoomInButton_.setTooltip ("Zoom in");
        zoomInButton_.onClick = [this] { if (onZoomStep != nullptr) onZoomStep (1); };
        zoomLabel_.setJustificationType (juce::Justification::centred);
        zoomLabel_.setColour (juce::Label::textColourId, juce::Colour (0xff9a968d));
        zoomLabel_.setFont (juce::Font (juce::FontOptions (9.0f)));

        addAndMakeVisible (zoomOutButton_);
        addAndMakeVisible (fitButton_);
        addAndMakeVisible (zoomInButton_);
        addAndMakeVisible (zoomLabel_);

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

        juce::ignoreUnused (presetBox_);
    }

    void resized() override
    {
        auto area = getLocalBounds().reduced (12, 11);

        loadButton_.setBounds (area.removeFromRight (54).reduced (2, 0));
        saveButton_.setBounds (area.removeFromRight (58).reduced (2, 0));
        initButton_.setBounds (area.removeFromRight (50).reduced (2, 0));
        browseButton_.setBounds (area.removeFromRight (68).reduced (2, 0));
        presetBox_.setBounds (area.removeFromRight (190).reduced (4, 0));

        auto zoomArea = area.removeFromRight (154).reduced (4, 0);
        zoomOutButton_.setBounds (zoomArea.removeFromLeft (30));
        fitButton_.setBounds (zoomArea.removeFromLeft (42).reduced (2, 0));
        zoomInButton_.setBounds (zoomArea.removeFromLeft (30));
        zoomLabel_.setBounds (zoomArea.reduced (2, 0));
    }

    void refresh()
    {
        const auto name = host_.currentPresetName();

        for (int i = 0; i < presetBox_.getNumItems(); ++i)
        {
            if (presetBox_.getItemText (i) == name && presetBox_.getItemId (i) != 0)
            {
                if (presetBox_.getSelectedId() != presetBox_.getItemId (i))
                    presetBox_.setSelectedId (presetBox_.getItemId (i),
                                              juce::dontSendNotification);
                return;
            }
        }
    }

    void setAccent (juce::Colour colour)
    {
        accent_ = colour;
        repaint();
    }

    void setZoomText (float zoom)
    {
        zoomLabel_.setText (juce::String (juce::roundToInt (zoom * 100.0f)) + "%",
                            juce::dontSendNotification);
    }

    std::function<void (int)> onZoomStep;
    std::function<void()> onFit;
    std::function<void()> onBrowse;

private:
    void timerCallback() override
    {
        refresh();
    }

    EditorHost& host_;
    juce::ComboBox presetBox_;
    juce::TextButton initButton_, saveButton_, loadButton_, browseButton_;
    juce::TextButton zoomOutButton_, fitButton_, zoomInButton_;
    juce::Label zoomLabel_;
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
    canvas_.addAndMakeVisible (*panel_);
    viewport_.setViewedComponent (&canvas_, false);
    viewport_.setScrollBarsShown (true, true);
    viewport_.setColour (juce::ScrollBar::thumbColourId, juce::Colour (0xff5a5a5f));
    viewport_.setColour (juce::ScrollBar::trackColourId, juce::Colour (0xff151517));
    addAndMakeVisible (viewport_);

    header_->onZoomStep = [this] (int direction)
    {
        const auto base = zoom_ > 0.0f ? zoom_ : currentFitScale_;
        setZoom (base + 0.1f * (float) direction);
    };
    header_->onFit = [this] { setZoom (0.0f); };
    header_->onBrowse = [this] { openPresetBrowser(); };

    browserBackdrop_.setVisible (false);
    browserBackdrop_.setInterceptsMouseClicks (true, true);
    addChildComponent (browserBackdrop_);

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

    if (std::getenv ("TRIAD_BROWSER") != nullptr)
    {
        juce::Component::SafePointer<TriadEditor> safe (this);
        juce::Timer::callAfterDelay (500, [safe]
        {
            if (safe != nullptr)
                safe->openPresetBrowser();
        });
    }
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

    auto panelArea = area.reduced (0, 4);
    viewport_.setBounds (panelArea);

    if (panel_ == nullptr || panel_->designWidth() <= 0 || panel_->designHeight() <= 0)
        return;

    const auto designW = (float) panel_->designWidth();
    const auto designH = (float) panel_->designHeight();
    const auto fitScale = juce::jmin ((float) panelArea.getWidth() / designW,
                                      (float) panelArea.getHeight() / designH);
    currentFitScale_ = fitScale;

    const auto scale = zoom_ > 0.0f ? zoom_ : fitScale;
    const auto scaledW = designW * scale;
    const auto scaledH = designH * scale;

    if (zoom_ > 0.0f)
    {
        canvas_.setSize (juce::jmax (1, juce::roundToInt (scaledW)),
                         juce::jmax (1, juce::roundToInt (scaledH)));
        panel_->setBounds (0, 0, panel_->designWidth(), panel_->designHeight());
        panel_->setTransform (juce::AffineTransform::scale (scale));
    }
    else
    {
        canvas_.setSize (panelArea.getWidth(), panelArea.getHeight());
        const auto offsetX = ((float) panelArea.getWidth() - scaledW) * 0.5f;
        const auto offsetY = ((float) panelArea.getHeight() - scaledH) * 0.5f;
        panel_->setBounds (0, 0, panel_->designWidth(), panel_->designHeight());
        panel_->setTransform (juce::AffineTransform::scale (scale).translated (offsetX, offsetY));
    }

    viewport_.setViewPosition (0, 0);

    if (presetBrowser_ != nullptr)
    {
        browserBackdrop_.setBounds (getLocalBounds());
        const auto width = juce::jmin (840, getWidth() - 80);
        const auto height = juce::jmin (580, getHeight() - 100);
        presetBrowser_->setBounds ((getWidth() - width) / 2,
                                   (getHeight() - height) / 2, width, height);
    }

    if (header_ != nullptr)
        header_->setZoomText (scale);

    if (std::getenv ("TRIAD_LAYOUT") != nullptr)
        std::fprintf (stderr, "EDITOR %dx%d viewport %dx%d design %dx%d scale %.4f\n",
                      getWidth(), getHeight(), panelArea.getWidth(), panelArea.getHeight(),
                      panel_->designWidth(), panel_->designHeight(), scale);
}

void TriadEditor::Backdrop::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black.withAlpha (0.58f));
}

void TriadEditor::openPresetBrowser()
{
    if (presetBrowser_ != nullptr)
        return;

    presetBrowser_ = std::make_unique<PresetBrowser> (host_, [this] { closePresetBrowser(); });
    browserBackdrop_.setVisible (true);
    browserBackdrop_.setBounds (getLocalBounds());
    addAndMakeVisible (*presetBrowser_);
    browserBackdrop_.toFront (false);
    presetBrowser_->toFront (true);

    const auto width = juce::jmin (840, getWidth() - 80);
    const auto height = juce::jmin (580, getHeight() - 100);
    presetBrowser_->setBounds ((getWidth() - width) / 2, (getHeight() - height) / 2,
                               width, height);
    presetBrowser_->grabKeyboardFocus();
}

void TriadEditor::closePresetBrowser()
{
    presetBrowser_.reset();
    browserBackdrop_.setVisible (false);
}

bool TriadEditor::keyPressed (const juce::KeyPress& key)
{
    if (presetBrowser_ != nullptr && key == juce::KeyPress::escapeKey)
    {
        closePresetBrowser();
        return true;
    }

    return false;
}

void TriadEditor::setZoom (float zoom)
{
    if (zoom <= 0.0f)
        zoom_ = 0.0f;
    else
        zoom_ = juce::jlimit (0.35f, 2.0f, zoom);

    resized();
}

void TriadEditor::timerCallback()
{
    if (header_ != nullptr)
        header_->refresh();

    keyboard_.refresh();

    if (panel_ != nullptr)
    {
        panel_->pollPatchCables();

        if (! layoutDumped_)
        {
            layoutDumped_ = true;
            panel_->dumpLayout();
        }
    }

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
