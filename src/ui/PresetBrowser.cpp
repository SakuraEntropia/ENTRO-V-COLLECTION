#include "ui/PresetBrowser.h"
#include "ui/LookAndFeel.h"

namespace triad::ui
{

PresetBrowser::PresetBrowser (EditorHost& host, std::function<void()> onClose)
    : host_ (host), onClose_ (std::move (onClose))
{
    setOpaque (false);
    setWantsKeyboardFocus (true);

    search_.setTextToShowWhenEmpty ("Search presets...", juce::Colour (0xff7d7a74));
    search_.setColour (juce::TextEditor::backgroundColourId, juce::Colour (0xff101013));
    search_.setColour (juce::TextEditor::textColourId, juce::Colour (0xffe9e6df));
    search_.setColour (juce::TextEditor::outlineColourId, juce::Colour (0xff45454c));
    search_.setColour (juce::TextEditor::focusedOutlineColourId, juce::Colour (0xff8f8c85));
    search_.setFont (juce::Font (juce::FontOptions (12.0f)));
    search_.onTextChange = [this] { rebuild(); };
    addAndMakeVisible (search_);

    category_.addItem ("All categories", 1);

    juce::StringArray categories;
    int nextId = 2;

    for (int i = 0; i < host_.factoryPresetCount(); ++i)
    {
        const auto category = host_.factoryPresetCategory (i);

        if (category.isNotEmpty() && ! categories.contains (category))
        {
            categories.add (category);
            category_.addItem (category, nextId++);
        }
    }

    category_.setSelectedId (1, juce::dontSendNotification);
    category_.onChange = [this] { rebuild(); };
    addAndMakeVisible (category_);

    list_.setRowHeight (26);
    list_.setColour (juce::ListBox::backgroundColourId, juce::Colour (0xff111114));
    list_.setColour (juce::ListBox::outlineColourId, juce::Colour (0xff38383e));
    list_.setOutlineThickness (1);
    addAndMakeVisible (list_);

    detail_.setJustificationType (juce::Justification::topLeft);
    detail_.setColour (juce::Label::textColourId, juce::Colour (0xffd8d4cb));
    detail_.setFont (juce::Font (juce::FontOptions (12.0f)));
    addAndMakeVisible (detail_);

    loadButton_.onClick = [this] { loadSelected(); };
    closeButton_.onClick = [this]
    {
        if (onClose_ != nullptr)
            onClose_();
    };
    addAndMakeVisible (loadButton_);
    addAndMakeVisible (closeButton_);

    rebuild();
}

void PresetBrowser::rebuild()
{
    entries_.clear();

    const auto category = category_.getSelectedId() > 1
                              ? category_.getText()
                              : juce::String();
    const auto term = search_.getText().trim();

    for (int i = 0; i < host_.factoryPresetCount(); ++i)
    {
        const auto name = host_.factoryPresetName (i);
        const auto presetCategory = host_.factoryPresetCategory (i);

        if (category.isNotEmpty() && presetCategory != category)
            continue;

        if (term.isNotEmpty() && ! name.containsIgnoreCase (term)
            && ! presetCategory.containsIgnoreCase (term))
            continue;

        entries_.push_back ({ i, name, presetCategory });
    }

    selected_ = -1;
    list_.updateContent();
    loadButton_.setEnabled (false);

    if (! entries_.empty())
        list_.selectRow (0, true, true);
    else
    {
        detail_.setText ("No presets match this search.\n\nTry another category or clear the search field.",
                         juce::dontSendNotification);
    }
}

int PresetBrowser::getNumRows()
{
    return (int) entries_.size();
}

void PresetBrowser::paintListBoxItem (int row, juce::Graphics& g, int width, int height,
                                      bool rowIsSelected)
{
    if (row < 0 || row >= (int) entries_.size())
        return;

    const auto& entry = entries_[(size_t) row];

    if (rowIsSelected)
    {
        g.setColour (juce::Colour (0xff2c3a46));
        g.fillRect (0, 0, width, height);
        g.setColour (juce::Colour (0xff5aa7e8));
        g.fillRect (0, 0, 2, height);
    }
    else if (row % 2 == 1)
    {
        g.setColour (juce::Colours::white.withAlpha (0.025f));
        g.fillRect (0, 0, width, height);
    }

    g.setColour (rowIsSelected ? juce::Colour (0xfff2eee5) : juce::Colour (0xffd2cec5));
    g.setFont (juce::Font (juce::FontOptions (11.5f)));
    g.drawText (entry.name, 10, 0, width - 90, height, juce::Justification::centredLeft);

    g.setColour (juce::Colour (0xff8f8c85));
    g.setFont (juce::Font (juce::FontOptions (9.0f)));
    g.drawText (entry.category, width - 90, 0, 80, height, juce::Justification::centredRight);
}

void PresetBrowser::selectedRowsChanged (int lastRowSelected)
{
    selected_ = lastRowSelected;

    if (selected_ >= 0 && selected_ < (int) entries_.size())
    {
        const auto& entry = entries_[(size_t) selected_];
        detail_.setText (entry.name + "\n\n" + entry.category
                             + "\nFactory preset for " + host_.instrumentName(),
                         juce::dontSendNotification);
        loadButton_.setEnabled (true);
    }
    else
    {
        loadButton_.setEnabled (false);
    }
}

void PresetBrowser::listBoxItemDoubleClicked (int row, const juce::MouseEvent&)
{
    list_.selectRow (row, true, true);
    loadSelected();
}

void PresetBrowser::loadSelected()
{
    if (selected_ < 0 || selected_ >= (int) entries_.size())
        return;

    host_.loadFactoryPreset (entries_[(size_t) selected_].presetIndex);

    detail_.setText (entries_[(size_t) selected_].name + "\n\nLoaded. "
                         + entries_[(size_t) selected_].category
                         + " bank · " + host_.instrumentName(),
                     juce::dontSendNotification);
}

void PresetBrowser::visibilityChanged()
{
    if (isVisible())
        search_.grabKeyboardFocus();
}

bool PresetBrowser::keyPressed (const juce::KeyPress& key)
{
    if (key == juce::KeyPress::escapeKey)
    {
        if (onClose_ != nullptr)
            onClose_();
        return true;
    }

    if (key == juce::KeyPress::returnKey)
    {
        loadSelected();
        return true;
    }

    return false;
}

void PresetBrowser::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    g.setColour (juce::Colours::black.withAlpha (0.55f));
    g.fillRoundedRectangle (bounds.translated (0.0f, 3.0f), 7.0f);

    juce::ColourGradient background (juce::Colour (0xff232429), 0.0f, 0.0f,
                                     juce::Colour (0xff141518), 0.0f, bounds.getBottom(), false);
    g.setGradientFill (background);
    g.fillRoundedRectangle (bounds, 6.0f);

    g.setColour (juce::Colour (0xff8f8c85).withAlpha (0.8f));
    g.drawRoundedRectangle (bounds.reduced (0.5f), 6.0f, 1.0f);

    g.setColour (juce::Colour (0xfff2eee5));
    g.setFont (juce::Font (juce::FontOptions (14.0f, juce::Font::bold))
                   .withExtraKerningFactor (0.16f));
    g.drawText ("PRESET BROWSER", 18, 10, 300, 20, juce::Justification::centredLeft);

    g.setColour (juce::Colour (0xff8f8c85));
    g.setFont (juce::Font (juce::FontOptions (9.0f)));
    g.drawText (host_.instrumentName() + "  /  FACTORY BANK", 18, 28, 400, 14,
                juce::Justification::centredLeft);
}

void PresetBrowser::resized()
{
    auto area = getLocalBounds().reduced (14);

    auto header = area.removeFromTop (26);
    header.removeFromLeft (220);
    category_.setBounds (header.removeFromLeft (150).reduced (2, 0));
    search_.setBounds (header.removeFromLeft (240).reduced (6, 1));

    area.removeFromTop (10);

    auto bottom = area.removeFromBottom (34);
    closeButton_.setBounds (bottom.removeFromRight (78).reduced (2, 2));
    loadButton_.setBounds (bottom.removeFromRight (78).reduced (2, 2));

    auto listArea = area.removeFromLeft (330);
    list_.setBounds (listArea);
    area.removeFromLeft (12);
    detail_.setBounds (area.reduced (4, 4));
}

} // namespace triad::ui
