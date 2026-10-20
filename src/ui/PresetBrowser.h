#pragma once

#include "ui/EditorHost.h"

#include <juce_gui_basics/juce_gui_basics.h>

#include <functional>
#include <vector>

namespace triad::ui
{

/** Searchable factory-preset browser, modelled on the browsers in Serum and
    Massive X: category filter, text search, list and detail pane. */
class PresetBrowser final : public juce::Component,
                            private juce::ListBoxModel
{
public:
    PresetBrowser (EditorHost& host, std::function<void()> onClose);
    ~PresetBrowser() override = default;

    void paint (juce::Graphics&) override;
    void resized() override;
    bool keyPressed (const juce::KeyPress&) override;
    void visibilityChanged() override;

private:
    struct Entry
    {
        int presetIndex = -1;
        juce::String name;
        juce::String category;
    };

    void rebuild();
    void loadSelected();

    int getNumRows() override;
    void paintListBoxItem (int row, juce::Graphics&, int width, int height,
                           bool rowIsSelected) override;
    void listBoxItemDoubleClicked (int row, const juce::MouseEvent&) override;
    void selectedRowsChanged (int lastRowSelected) override;

    EditorHost& host_;
    std::function<void()> onClose_;
    juce::TextEditor search_;
    juce::ComboBox category_;
    juce::ListBox list_ { "presets", this };
    juce::Label detail_;
    juce::TextButton loadButton_ { "LOAD" };
    juce::TextButton closeButton_ { "CLOSE" };
    std::vector<Entry> entries_;
    int selected_ = -1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PresetBrowser)
};

} // namespace triad::ui
