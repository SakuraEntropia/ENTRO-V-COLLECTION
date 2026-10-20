#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace triad::ui
{

enum class HardwareStyle
{
    Matriarch,
    PolyBrute,
    MoogOne
};

class TriadLookAndFeel final : public juce::LookAndFeel_V4
{
public:
    TriadLookAndFeel();

    void drawRotarySlider (juce::Graphics&, int x, int y, int width, int height,
                           float sliderPosProportional, float rotaryStartAngle,
                           float rotaryEndAngle, juce::Slider&) override;

    void drawLinearSlider (juce::Graphics&, int x, int y, int width, int height,
                           float sliderPos, float minSliderPos, float maxSliderPos,
                           juce::Slider::SliderStyle, juce::Slider&) override;

    void drawComboBox (juce::Graphics&, int width, int height, bool isButtonDown,
                       int buttonX, int buttonY, int buttonW, int buttonH,
                       juce::ComboBox&) override;

    void drawButtonBackground (juce::Graphics&, juce::Button&, const juce::Colour& backgroundColour,
                               bool shouldDrawButtonAsHighlighted,
                               bool shouldDrawButtonAsDown) override;

    juce::Font getComboBoxFont (juce::ComboBox&) override;
    juce::Font getPopupMenuFont() override;
    juce::Font getTextButtonFont (juce::TextButton&, int buttonHeight) override;

    static juce::Colour panelBackground();
    static juce::Colour panelEdge();
    static juce::Colour text();
    static juce::Colour dimText();
    static juce::Colour metalDark();
    static juce::Colour metalLight();
    static juce::Colour accentFor (const juce::String& instrument);

    juce::Colour accent { 0xffcf4b2e };

    void setHardwareStyle (HardwareStyle style) { style_ = style; }
    HardwareStyle hardwareStyle() const { return style_; }

private:
    HardwareStyle style_ = HardwareStyle::Matriarch;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TriadLookAndFeel)
};

} // namespace triad::ui
