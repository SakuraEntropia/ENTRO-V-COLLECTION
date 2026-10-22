#pragma once

#include "ui/EditorHost.h"
#include "ui/LookAndFeel.h"
#include "ui/Widgets.h"

#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_gui_basics/juce_gui_basics.h>

namespace triad::ui
{

/** Keyboard with explicit physical key gaps and bevels. */
class HardwareKeyboard final : public juce::MidiKeyboardComponent
{
public:
    using juce::MidiKeyboardComponent::MidiKeyboardComponent;

    void drawWhiteNote (int midiNoteNumber, juce::Graphics& g, juce::Rectangle<float> area,
                        bool isDown, bool isOver, juce::Colour lineColour,
                        juce::Colour textColour) override;
    void drawBlackNote (int midiNoteNumber, juce::Graphics& g, juce::Rectangle<float> area,
                        bool isDown, bool isOver, juce::Colour noteFillColour) override;
};

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
    HardwareKeyboard keyboard_;
    juce::Slider pitchWheel_;
    juce::Slider modWheel_;
    juce::Label pitchLabel_, modLabel_;
    juce::TextButton octaveDown_ { "OCT -" };
    juce::TextButton octaveUp_ { "OCT +" };
    juce::ComboBox partBox_;
    juce::Label partLabel_;
    juce::Slider glide_;
    juce::Label glideLabel_;
    std::unique_ptr<LedButton> play_, hold_;
    juce::TextButton tap_ { "TAP" };
    TapTempo tapTempo_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> glideAttachment_;
    std::unique_ptr<juce::Component> morphPad_;
    int octave_ = 0;
    int lastPart_ = 1;
    int lowestNote_ = 36;
    int highestNote_ = 96;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KeyboardPanel)
};

} // namespace triad::ui
