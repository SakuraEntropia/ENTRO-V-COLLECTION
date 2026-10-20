#pragma once

#include "ui/EditorHost.h"
#include "ui/LookAndFeel.h"

#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_gui_basics/juce_gui_basics.h>

namespace triad::ui
{

/** Keyboard, wheels and part selector shown beneath each instrument panel. */
class KeyboardPanel final : public juce::Component, private juce::Timer
{
public:
    explicit KeyboardPanel (EditorHost& host);
    ~KeyboardPanel() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    void refresh();

private:
    void timerCallback() override;

    EditorHost& host_;
    juce::MidiKeyboardComponent keyboard_;
    juce::Slider pitchWheel_;
    juce::Slider modWheel_;
    juce::Label pitchLabel_, modLabel_;
    juce::TextButton octaveDown_ { "OCT -" };
    juce::TextButton octaveUp_ { "OCT +" };
    juce::ComboBox partBox_;
    juce::Label partLabel_;
    int octave_ = 0;
    int lastPart_ = 1;
    int lowestNote_ = 36;
    int highestNote_ = 96;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KeyboardPanel)
};

} // namespace triad::ui
