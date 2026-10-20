#include "ui/LookAndFeel.h"

namespace triad::ui
{

TriadLookAndFeel::TriadLookAndFeel()
{
    setColour (juce::Label::textColourId, text());
    setColour (juce::Slider::textBoxTextColourId, text());
    setColour (juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
    setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    setColour (juce::ComboBox::backgroundColourId, juce::Colour (0xff202024));
    setColour (juce::ComboBox::textColourId, text());
    setColour (juce::ComboBox::outlineColourId, juce::Colour (0xff4a4a50));
    setColour (juce::ComboBox::arrowColourId, dimText());
    setColour (juce::PopupMenu::backgroundColourId, juce::Colour (0xff1b1b1f));
    setColour (juce::PopupMenu::textColourId, text());
    setColour (juce::PopupMenu::highlightedBackgroundColourId, accent.withAlpha (0.35f));
    setColour (juce::PopupMenu::highlightedTextColourId, juce::Colours::white);
    setColour (juce::TextButton::buttonColourId, juce::Colour (0xff26262b));
    setColour (juce::TextButton::buttonOnColourId, accent.withAlpha (0.5f));
    setColour (juce::TextButton::textColourOffId, text());
    setColour (juce::TextButton::textColourOnId, juce::Colours::white);
    setColour (juce::TooltipWindow::backgroundColourId, juce::Colour (0xff2a2a2e));
    setColour (juce::TooltipWindow::textColourId, text());
}

juce::Colour TriadLookAndFeel::panelBackground() { return juce::Colour (0xff141416); }
juce::Colour TriadLookAndFeel::panelEdge() { return juce::Colour (0xff2c2c31); }
juce::Colour TriadLookAndFeel::text() { return juce::Colour (0xffe9e6df); }
juce::Colour TriadLookAndFeel::dimText() { return juce::Colour (0xff9c9992); }
juce::Colour TriadLookAndFeel::metalDark() { return juce::Colour (0xff17171a); }
juce::Colour TriadLookAndFeel::metalLight() { return juce::Colour (0xff5a5a60); }

juce::Colour TriadLookAndFeel::accentFor (const juce::String& instrument)
{
    if (instrument.containsIgnoreCase ("polybrute"))
        return juce::Colour (0xff3f93e8);
    if (instrument.containsIgnoreCase ("moog one") || instrument.containsIgnoreCase ("one"))
        return juce::Colour (0xffd9a441);
    return juce::Colour (0xffcf4b2e);
}

void TriadLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                                         float sliderPos, float startAngle, float endAngle,
                                         juce::Slider& slider)
{
    const auto bounds = juce::Rectangle<float> ((float) x, (float) y, (float) width, (float) height)
                            .reduced (2.0f);
    const auto radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;
    const auto centre = bounds.getCentre();
    const auto angle = startAngle + sliderPos * (endAngle - startAngle);

    const auto knobRadius = radius * 0.78f;
    const auto arcRadius = radius * 0.92f;

    // Arc track.
    juce::Path track;
    track.addCentredArc (centre.x, centre.y, arcRadius, arcRadius, 0.0f,
                         startAngle, endAngle, true);
    g.setColour (juce::Colour (0xff3a3a3f));
    g.strokePath (track, juce::PathStrokeType (2.2f, juce::PathStrokeType::curved,
                                               juce::PathStrokeType::rounded));

    // Value arc (from the centre for bipolar parameters).
    const auto isBipolar = slider.getMinimum() < 0.0 && slider.getMaximum() > 0.0;
    const auto centreAngle = (startAngle + endAngle) * 0.5f;
    const auto arcStart = isBipolar ? centreAngle : startAngle;
    if (std::abs (angle - arcStart) > 0.01f)
    {
        juce::Path valueArc;
        valueArc.addCentredArc (centre.x, centre.y, arcRadius, arcRadius, 0.0f,
                                juce::jmin (arcStart, angle), juce::jmax (arcStart, angle), true);
        g.setColour (accent.withAlpha (0.95f));
        g.strokePath (valueArc, juce::PathStrokeType (2.4f, juce::PathStrokeType::curved,
                                                      juce::PathStrokeType::rounded));
    }

    // Knob body.
    const auto body = juce::Rectangle<float> (knobRadius * 2.0f, knobRadius * 2.0f)
                          .withCentre (centre);
    g.setColour (juce::Colours::black.withAlpha (0.55f));
    g.fillEllipse (body.translated (0.0f, 1.6f));

    juce::ColourGradient bodyGradient (juce::Colour (0xff4a4a4f), centre.x, body.getY(),
                                       juce::Colour (0xff18181b), centre.x, body.getBottom(), false);
    bodyGradient.addColour (0.45, juce::Colour (0xff303035));
    g.setGradientFill (bodyGradient);
    g.fillEllipse (body);

    g.setColour (juce::Colour (0xff77777e).withAlpha (0.75f));
    g.drawEllipse (body.reduced (0.6f), 1.0f);

    // Pointer.
    const auto pointerLength = knobRadius * 0.78f;
    const auto pointerStart = knobRadius * 0.24f;
    const auto pointerEnd = juce::Point<float> (centre.x + std::sin (angle) * pointerLength,
                                                centre.y - std::cos (angle) * pointerLength);
    const auto pointerBegin = juce::Point<float> (centre.x + std::sin (angle) * pointerStart,
                                                  centre.y - std::cos (angle) * pointerStart);
    g.setColour (juce::Colour (0xfff3efe6));
    g.drawLine ({ pointerBegin, pointerEnd }, juce::jmax (1.4f, radius * 0.055f));

    // Tick marks.
    g.setColour (juce::Colour (0xff8f8c86).withAlpha (0.55f));
    for (int i = 0; i <= 10; ++i)
    {
        const auto tickAngle = startAngle + (endAngle - startAngle) * (float) i / 10.0f;
        const auto inner = juce::Point<float> (centre.x + std::sin (tickAngle) * radius * 0.72f,
                                               centre.y - std::cos (tickAngle) * radius * 0.72f);
        const auto outer = juce::Point<float> (centre.x + std::sin (tickAngle) * radius * 0.79f,
                                               centre.y - std::cos (tickAngle) * radius * 0.79f);
        g.drawLine ({ inner, outer }, 1.0f);
    }
}

void TriadLookAndFeel::drawLinearSlider (juce::Graphics& g, int x, int y, int width, int height,
                                         float sliderPos, float, float,
                                         juce::Slider::SliderStyle style, juce::Slider&)
{
    const auto bounds = juce::Rectangle<int> (x, y, width, height).toFloat();

    if (style == juce::Slider::LinearVertical)
    {
        const auto track = juce::Rectangle<float> (bounds.getCentreX() - 2.0f, bounds.getY() + 4.0f,
                                                   4.0f, bounds.getHeight() - 8.0f);
        g.setColour (juce::Colour (0xff2b2b30));
        g.fillRoundedRectangle (track, 2.0f);
        g.setColour (accent.withAlpha (0.8f));
        g.fillRoundedRectangle (track.withTop (sliderPos), 2.0f);

        const auto grip = juce::Rectangle<float> (bounds.getWidth() - 8.0f, 18.0f)
                              .withCentre ({ bounds.getCentreX(), sliderPos });
        g.setColour (juce::Colour (0xffd9d5cc));
        g.fillRoundedRectangle (grip, 3.0f);
        g.setColour (juce::Colours::black.withAlpha (0.6f));
        g.drawRoundedRectangle (grip, 3.0f, 1.0f);
    }
    else
    {
        g.setColour (juce::Colour (0xff2b2b30));
        g.fillRoundedRectangle (bounds.reduced (2.0f), 3.0f);
    }
}

void TriadLookAndFeel::drawComboBox (juce::Graphics& g, int width, int height, bool,
                                     int, int, int, int, juce::ComboBox& box)
{
    const auto bounds = juce::Rectangle<int> (0, 0, width, height).toFloat();
    g.setColour (juce::Colour (0xff1d1d21));
    g.fillRoundedRectangle (bounds, 3.0f);
    g.setColour (juce::Colour (0xff4b4b52));
    g.drawRoundedRectangle (bounds.reduced (0.5f), 3.0f, 1.0f);

    juce::Path arrow;
    const auto centreX = (float) width - 12.0f;
    const auto centreY = (float) height * 0.5f;
    arrow.addTriangle (centreX - 3.5f, centreY - 2.0f, centreX + 3.5f, centreY - 2.0f,
                       centreX, centreY + 3.0f);
    g.setColour (dimText());
    g.fillPath (arrow);
}

void TriadLookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& button,
                                             const juce::Colour& backgroundColour,
                                             bool highlighted, bool down)
{
    auto bounds = button.getLocalBounds().toFloat();
    auto base = backgroundColour;
    if (down)
        base = base.brighter (0.15f);
    else if (highlighted)
        base = base.brighter (0.08f);

    g.setColour (base);
    g.fillRoundedRectangle (bounds, 3.0f);
    g.setColour ((button.getToggleState() ? accent : juce::Colour (0xff505057)).withAlpha (0.9f));
    g.drawRoundedRectangle (bounds.reduced (0.5f), 3.0f, 1.0f);
}

juce::Font TriadLookAndFeel::getComboBoxFont (juce::ComboBox&)
{
    return juce::Font (juce::FontOptions (12.0f));
}

juce::Font TriadLookAndFeel::getPopupMenuFont()
{
    return juce::Font (juce::FontOptions (12.0f));
}

juce::Font TriadLookAndFeel::getTextButtonFont (juce::TextButton&, int buttonHeight)
{
    return juce::Font (juce::FontOptions (juce::jmin (13.0f, (float) buttonHeight * 0.55f)));
}

} // namespace triad::ui
