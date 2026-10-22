#include "ui/KeyboardPanel.h"

namespace triad::ui
{

void HardwareKeyboard::drawWhiteNote (int midiNoteNumber, juce::Graphics& g,
                                      juce::Rectangle<float> area, bool isDown, bool isOver,
                                      juce::Colour lineColour, juce::Colour textColour)
{
    (void) lineColour;
    auto key = area;

    juce::ColourGradient white (isDown ? juce::Colour (0xffcfc5b1) : juce::Colour (0xfffffdf6),
                                key.getX(), key.getY(),
                                isDown ? juce::Colour (0xffb9ae98) : juce::Colour (0xffd8d2c5),
                                key.getX(), key.getBottom(), false);
    white.addColour (0.7, isDown ? juce::Colour (0xffc6bba6) : juce::Colour (0xffe8e2d5));
    g.setGradientFill (white);
    g.fillRect (key);

    if (isOver)
    {
        g.setColour (juce::Colour (0x22000000));
        g.fillRect (key);
    }

    // A real gap between white keys, not just a hairline.
    g.setColour (juce::Colour (0xff151410));
    g.fillRect (key.getRight() - 2.0f, key.getY(), 2.0f, key.getHeight());

    g.setColour (juce::Colour (0xff625d53));
    g.drawRect (key.reduced (0.5f), 1.0f);

    g.setColour (juce::Colour (0xffd6d0c4).withAlpha (0.7f));
    g.drawHorizontalLine ((int) key.getBottom() - 2, key.getX() + 1.0f, key.getRight() - 3.0f);

    if (midiNoteNumber % 12 == 0)
    {
        g.setColour (textColour.withAlpha (0.7f));
        g.setFont (juce::Font (juce::FontOptions (8.0f)));
        g.drawText ("C" + juce::String (midiNoteNumber / 12 - 1),
                    key.removeFromBottom (13.0f).toNearestInt(),
                    juce::Justification::centred);
    }
}

void HardwareKeyboard::drawBlackNote (int midiNoteNumber, juce::Graphics& g,
                                      juce::Rectangle<float> area, bool isDown, bool isOver,
                                      juce::Colour noteFillColour)
{
    (void) midiNoteNumber;
    auto key = area;

    juce::ColourGradient black (isDown ? juce::Colour (0xff303034) : juce::Colour (0xff26262a),
                                key.getX(), key.getY(),
                                juce::Colour (0xff030304), key.getX(), key.getBottom(), false);
    black.addColour (0.55, juce::Colour (0xff0b0b0d));
    g.setGradientFill (black);
    g.fillRect (key);

    if (isOver || isDown)
    {
        g.setColour (juce::Colours::white.withAlpha (isDown ? 0.18f : 0.08f));
        g.fillRect (key);
    }

    g.setColour (juce::Colour (0xff050505));
    g.drawRect (key.reduced (0.5f), 1.0f);
    g.setColour (juce::Colours::white.withAlpha (0.22f));
    g.drawHorizontalLine ((int) key.getY() + 2, key.getX() + 2.0f, key.getRight() - 2.0f);
}

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

    const auto accent = TriadLookAndFeel::accentFor (host_.instrumentName());
    keyboard_.setColour (juce::MidiKeyboardComponent::whiteNoteColourId, juce::Colour (0xfff0ebdf));
    keyboard_.setColour (juce::MidiKeyboardComponent::blackNoteColourId, juce::Colour (0xff0f0f11));
    keyboard_.setColour (juce::MidiKeyboardComponent::keySeparatorLineColourId, juce::Colour (0xff8d887d));
    keyboard_.setColour (juce::MidiKeyboardComponent::mouseOverKeyOverlayColourId, accent.withAlpha (0.18f));
    keyboard_.setColour (juce::MidiKeyboardComponent::keyDownOverlayColourId, accent.withAlpha (0.42f));
    keyboard_.setColour (juce::MidiKeyboardComponent::shadowColourId, juce::Colour (0x66000000));

    pitchWheel_.setSliderStyle (juce::Slider::LinearVertical);
    pitchWheel_.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    pitchWheel_.setRange (-1.0, 1.0, 0.0);
    pitchWheel_.setValue (0.0, juce::dontSendNotification);
    pitchWheel_.getProperties().set ("wheel", true);
    pitchWheel_.onValueChange = [this]
    {
        host_.setUiPitchBend ((float) pitchWheel_.getValue());
    };

    modWheel_.setSliderStyle (juce::Slider::LinearVertical);
    modWheel_.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    modWheel_.setRange (0.0, 1.0, 0.0);
    modWheel_.setValue (0.0, juce::dontSendNotification);
    modWheel_.getProperties().set ("wheel", true);
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

    if (instrument == "matriarch")
    {
        glide_.setName ("mat.performance.glide");
        glide_.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        glide_.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        glide_.setDoubleClickReturnValue (true, 0.0);
        glide_.setTooltip ("Glide time, 0-2 seconds; double-click for no glide");
        addAndMakeVisible (glide_);
        glideAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
            host_.apvts(), "mat.performance.glide", glide_);
        glideLabel_.setText ("GLIDE", juce::dontSendNotification);
        glideLabel_.setJustificationType (juce::Justification::centred);
        glideLabel_.setFont (juce::Font (juce::FontOptions (9.0f)));
        addAndMakeVisible (glideLabel_);
        play_ = std::make_unique<LedButton> (*host_.apvts().getParameter ("mat.arp.enabled"), "PLAY");
        hold_ = std::make_unique<LedButton> (*host_.apvts().getParameter ("mat.arp.hold"), "HOLD");
        play_->setName ("mat.arp.enabled"); hold_->setName ("mat.arp.hold");
        play_->setAccent (juce::Colour (0xff91cf82)); hold_->setAccent (juce::Colour (0xff94d5e3));
        play_->setTooltip ("Start/stop the arpeggiator using held keys");
        hold_->setTooltip ("Latch arpeggiator notes after releasing the keys");
        addAndMakeVisible (*play_); addAndMakeVisible (*hold_);
        tap_.setColour (juce::TextButton::buttonColourId, juce::Colour (0xffe9dc68));
        tap_.setColour (juce::TextButton::textColourOffId, juce::Colour (0xff252a2b));
        tap_.setTooltip ("Tap quarter notes to set internal tempo; DAW tempo takes priority");
        tap_.onClick = [this]
        {
            const auto bpm = tapTempo_.tap (juce::Time::getMillisecondCounterHiRes());
            if (bpm > 0.0) host_.setUiTempoBpm (bpm);
            tap_.setTooltip ("Internal tempo: " + juce::String (host_.uiTempoBpm(), 1)
                             + " BPM; DAW tempo takes priority");
        };
        addAndMakeVisible (tap_);
    }

    startTimerHz (12);
}

KeyboardPanel::~KeyboardPanel()
{
    stopTimer();
}

void KeyboardPanel::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    const auto instrument = juce::String (host_.definition().id);
    const auto wood = instrument != "matriarch";

    juce::ColourGradient background (juce::Colour (0xff1e1e21), 0.0f, 0.0f,
                                     juce::Colour (0xff08080a), 0.0f, bounds.getBottom(), false);
    g.setGradientFill (background);
    g.fillRect (bounds);

    // Wooden end cheeks and front rail.
    if (wood)
    {
        for (const auto& side : { juce::Rectangle<float> (0.0f, 0.0f, 13.0f, bounds.getHeight()),
                                  juce::Rectangle<float> (bounds.getRight() - 13.0f, 0.0f, 13.0f,
                                                          bounds.getHeight()) })
        {
            juce::ColourGradient grain (juce::Colour (0xff6f4224), side.getX(), 0.0f,
                                         juce::Colour (0xff2f1b0f), side.getRight(), 0.0f, false);
            grain.addColour (0.45, juce::Colour (0xff5a331b));
            g.setGradientFill (grain);
            g.fillRect (side);

            g.setColour (juce::Colours::black.withAlpha (0.25f));
            for (float x = side.getX() + 2.0f; x < side.getRight(); x += 3.0f)
                g.drawVerticalLine ((int) x, 0.0f, bounds.getHeight());
        }
    }
    else
    {
        g.setColour (juce::Colour (0xff050506));
        g.fillRect (0.0f, 0.0f, 10.0f, bounds.getHeight());
        g.fillRect (bounds.getRight() - 10.0f, 0.0f, 10.0f, bounds.getHeight());
    }

    // Felt strip above the keybed.
    g.setColour (juce::Colour (0xff3a0d0c));
    g.fillRect (bounds.getX() + 14.0f, 6.0f, bounds.getWidth() - 28.0f, 4.0f);
    g.setColour (juce::Colour (0xff6b1a17).withAlpha (0.7f));
    g.fillRect (bounds.getX() + 14.0f, 6.0f, bounds.getWidth() - 28.0f, 1.2f);

    g.setColour (juce::Colour (0xffffffff).withAlpha (0.06f));
    g.drawLine (0.0f, 0.0f, bounds.getWidth(), 0.0f, 1.0f);

    g.setColour (juce::Colour (0xffb9b5ab).withAlpha (0.8f));
    g.setFont (juce::Font (juce::FontOptions (8.0f)).withExtraKerningFactor (0.22f));
    g.drawText (host_.instrumentName().toUpperCase() + "   /   SAKURAENTROPIA",
                getLocalBounds().removeFromBottom (14).reduced (22, 0),
                juce::Justification::centredLeft);
}

void KeyboardPanel::resized()
{
    auto area = getLocalBounds().reduced (12, 8);

    const auto matriarch = play_ != nullptr;
    auto left = area.removeFromLeft (matriarch ? 216 : 118);
    if (matriarch)
    {
        glide_.setBounds (left.getX() + 6, left.getY() + 2, 36, 36);
        glideLabel_.setBounds (left.getX(), left.getY() + 38, 48, 12);
        play_->setBounds (left.getX() + 58, left.getY() + 8, 46, 28);
        hold_->setBounds (left.getX() + 110, left.getY() + 8, 46, 28);
        tap_.setBounds (left.getX() + 162, left.getY() + 8, 44, 28);
        left.removeFromTop (44);
    }
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
