#include "ui/KeyboardPanel.h"

namespace triad::ui
{

KeyboardPanel::KeyboardPanel (EditorHost& host)
    : host_ (host),
      keyboard_ (host.keyboardState(), juce::MidiKeyboardComponent::horizontalKeyboard)
{
    addAndMakeVisible (keyboard_);
    const auto instrument = juce::String (host_.definition().id);
    lowestNote_ = instrument == "matriarch" ? 29 : 36;
    highestNote_ = instrument == "matriarch" ? 77 : 96;

    keyboard_.setKeyWidth (20.0f);
    keyboard_.setAvailableRange (lowestNote_, highestNote_);
    keyboard_.setLowestVisibleKey (lowestNote_);
    keyboard_.setOctaveForMiddleC (4);
    keyboard_.setScrollButtonsVisible (false);
    keyboard_.setMidiChannel (1);

    keyboard_.setColour (juce::MidiKeyboardComponent::whiteNoteColourId, juce::Colour (0xffe7e2d8));
    keyboard_.setColour (juce::MidiKeyboardComponent::blackNoteColourId, juce::Colour (0xff141416));
    keyboard_.setColour (juce::MidiKeyboardComponent::keySeparatorLineColourId, juce::Colour (0xff55524c));
    keyboard_.setColour (juce::MidiKeyboardComponent::mouseOverKeyOverlayColourId, juce::Colour (0x33cf4b2e));
    keyboard_.setColour (juce::MidiKeyboardComponent::keyDownOverlayColourId, juce::Colour (0x66cf4b2e));
    keyboard_.setColour (juce::MidiKeyboardComponent::shadowColourId, juce::Colour (0x44000000));

    pitchWheel_.setSliderStyle (juce::Slider::LinearVertical);
    pitchWheel_.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    pitchWheel_.setRange (-1.0, 1.0, 0.0);
    pitchWheel_.setValue (0.0, juce::dontSendNotification);
    pitchWheel_.onValueChange = [this]
    {
        host_.setUiPitchBend ((float) pitchWheel_.getValue());
    };

    modWheel_.setSliderStyle (juce::Slider::LinearVertical);
    modWheel_.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    modWheel_.setRange (0.0, 1.0, 0.0);
    modWheel_.setValue (0.0, juce::dontSendNotification);
    modWheel_.onValueChange = [this]
    {
        host_.setUiModWheel ((float) modWheel_.getValue());
    };

    addAndMakeVisible (pitchWheel_);
    addAndMakeVisible (modWheel_);

    pitchLabel_.setText ("PITCH", juce::dontSendNotification);
    modLabel_.setText ("MOD", juce::dontSendNotification);
    for (auto* label : { &pitchLabel_, &modLabel_ })
    {
        label->setJustificationType (juce::Justification::centred);
        label->setColour (juce::Label::textColourId, juce::Colour (0xff9c9992));
        label->setFont (juce::Font (juce::FontOptions (8.5f, juce::Font::bold)));
        label->setInterceptsMouseClicks (false, false);
        addAndMakeVisible (*label);
    }

    octaveDown_.onClick = [this]
    {
        octave_ = juce::jmax (-2, octave_ - 1);
        keyboard_.setLowestVisibleKey (juce::jlimit (lowestNote_, highestNote_ - 12,
                                                     lowestNote_ + octave_ * 12));
    };

    octaveUp_.onClick = [this]
    {
        octave_ = juce::jmin (2, octave_ + 1);
        keyboard_.setLowestVisibleKey (juce::jlimit (lowestNote_, highestNote_ - 12,
                                                     lowestNote_ + octave_ * 12));
    };

    addAndMakeVisible (octaveDown_);
    addAndMakeVisible (octaveUp_);

    partLabel_.setText ("MIDI PART", juce::dontSendNotification);
    partLabel_.setJustificationType (juce::Justification::centred);
    partLabel_.setColour (juce::Label::textColourId, juce::Colour (0xff9c9992));
    partLabel_.setFont (juce::Font (juce::FontOptions (8.5f, juce::Font::bold)));
    addAndMakeVisible (partLabel_);

    for (int i = 0; i < 3; ++i)
        partBox_.addItem ("PART " + juce::String (i + 1), i + 1);

    partBox_.setSelectedId (1, juce::dontSendNotification);
    partBox_.onChange = [this]
    {
        const auto part = partBox_.getSelectedId();
        host_.setSelectedPart (part);
        keyboard_.setMidiChannel (part);
    };

    addAndMakeVisible (partBox_);

    const auto showParts = host_.hasParts();
    partBox_.setVisible (showParts);
    partLabel_.setVisible (showParts);

    startTimerHz (12);
}

KeyboardPanel::~KeyboardPanel()
{
    stopTimer();
}

void KeyboardPanel::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    juce::ColourGradient background (juce::Colour (0xff1a1a1d), 0.0f, 0.0f,
                                     juce::Colour (0xff0b0b0c), 0.0f, bounds.getBottom(), false);
    g.setGradientFill (background);
    g.fillRect (bounds);

    g.setColour (juce::Colour (0xffffffff).withAlpha (0.06f));
    g.drawLine (0.0f, 0.0f, bounds.getWidth(), 0.0f, 1.0f);

    g.setColour (juce::Colour (0xff9c9992).withAlpha (0.7f));
    g.setFont (juce::Font (juce::FontOptions (8.0f)));
    g.drawText ("ANALOG TRIAD  /  " + host_.instrumentName().toUpperCase(),
                getLocalBounds().removeFromBottom (14).reduced (18, 0),
                juce::Justification::centredLeft);
}

void KeyboardPanel::resized()
{
    auto area = getLocalBounds().reduced (12, 8);

    auto left = area.removeFromLeft (118);
    auto topLeft = left.removeFromTop (18);
    juce::ignoreUnused (topLeft);

    pitchWheel_.setBounds (left.getX() + 8, left.getY(), 24, left.getHeight() - 16);
    pitchLabel_.setBounds (left.getX() + 2, left.getBottom() - 16, 36, 12);
    modWheel_.setBounds (left.getX() + 54, left.getY(), 24, left.getHeight() - 16);
    modLabel_.setBounds (left.getX() + 48, left.getBottom() - 16, 36, 12);

    auto right = area.removeFromRight (120);
    auto buttons = right.removeFromBottom (24);
    octaveDown_.setBounds (buttons.removeFromLeft (56));
    octaveUp_.setBounds (buttons.removeFromLeft (56));

    partLabel_.setBounds (right.removeFromTop (14));
    partBox_.setBounds (right.removeFromTop (22).withWidth (100));

    keyboard_.setBounds (area);

    int whiteKeys = 0;
    for (int note = lowestNote_; note <= highestNote_; ++note)
    {
        const auto pitchClass = note % 12;
        if (pitchClass != 1 && pitchClass != 3 && pitchClass != 6 && pitchClass != 8 && pitchClass != 10)
            ++whiteKeys;
    }

    if (whiteKeys > 0)
        keyboard_.setKeyWidth (juce::jmax (12.0f, (float) area.getWidth() / (float) whiteKeys));
}

void KeyboardPanel::refresh()
{
    const auto part = host_.selectedPart();
    if (part != lastPart_)
    {
        lastPart_ = part;
        partBox_.setSelectedId (part, juce::dontSendNotification);
        keyboard_.setMidiChannel (part);
    }

    if (pitchWheel_.isMouseButtonDown() == false)
        pitchWheel_.setValue ((double) host_.uiPitchBend(), juce::dontSendNotification);

    if (modWheel_.isMouseButtonDown() == false)
        modWheel_.setValue ((double) host_.uiModWheel(), juce::dontSendNotification);
}

void KeyboardPanel::timerCallback()
{
    refresh();
}

} // namespace triad::ui
