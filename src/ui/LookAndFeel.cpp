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
    if (instrument.containsIgnoreCase ("continuum"))
        return juce::Colour (0xff2fb6a8);
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
    const auto isBipolar = slider.getMinimum() < 0.0 && slider.getMaximum() > 0.0;

    const auto style = style_;
    const auto knobRadius = radius * 0.74f;
    const auto arcRadius = radius * 0.9f;
    const auto accentColour = style == HardwareStyle::PolyBrute
                                  ? juce::Colour (0xff3f93e8)
                                  : (style == HardwareStyle::MoogOne
                                         ? juce::Colour (0xffd9a441)
                                         : juce::Colour (0xffcf3f23));

    // Engraved tick ring.
    g.setColour (juce::Colour (0xffd9d5cb).withAlpha (style == HardwareStyle::PolyBrute ? 0.68f
                                                                              : (style == HardwareStyle::Matriarch ? 0.55f : 0.32f)));
    for (int i = 0; i <= 10; ++i)
    {
        const auto tickAngle = startAngle + (endAngle - startAngle) * (float) i / 10.0f;
        const auto inner = juce::Point<float> (centre.x + std::sin (tickAngle) * radius * 0.86f,
                                               centre.y - std::cos (tickAngle) * radius * 0.86f);
        const auto outer = juce::Point<float> (centre.x + std::sin (tickAngle) * radius * 0.95f,
                                               centre.y - std::cos (tickAngle) * radius * 0.95f);
        g.drawLine ({ inner, outer }, i % 5 == 0 ? 1.5f : 0.9f);
    }

    // Illuminated value ring (PolyBrute / One). Matriarch knobs are unlit.
    if (style != HardwareStyle::Matriarch)
    {
        juce::Path track;
        track.addCentredArc (centre.x, centre.y, arcRadius, arcRadius, 0.0f,
                             startAngle, endAngle, true);
        g.setColour (juce::Colour (0xff2c2d31));
        g.strokePath (track, juce::PathStrokeType (2.0f, juce::PathStrokeType::curved,
                                                   juce::PathStrokeType::rounded));

        const auto centreAngle = (startAngle + endAngle) * 0.5f;
        const auto arcStart = isBipolar ? centreAngle : startAngle;
        juce::Path valueArc;
        valueArc.addCentredArc (centre.x, centre.y, arcRadius, arcRadius, 0.0f,
                                juce::jmin (arcStart, angle), juce::jmax (arcStart, angle), true);
        g.setColour (accentColour.withAlpha (style == HardwareStyle::PolyBrute ? 0.52f : 0.9f));
        g.strokePath (valueArc, juce::PathStrokeType (2.1f, juce::PathStrokeType::curved,
                                                      juce::PathStrokeType::rounded));
    }

    // Drop shadow and metal skirt.
    g.setColour (juce::Colours::black.withAlpha (0.6f));
    g.fillEllipse (juce::Rectangle<float> (knobRadius * 2.0f + 4.0f, knobRadius * 2.0f + 4.0f)
                       .withCentre (centre).translated (0.0f, 2.0f));

    const auto silverKnob = (bool) slider.getProperties()["silverKnob"];

    juce::Colour top, bottom, rim, pointer;
    if (silverKnob)
    {
        top = juce::Colour (0xffe6e2d8);
        bottom = juce::Colour (0xff7d786e);
        rim = juce::Colour (0xff35332f);
        pointer = juce::Colour (0xff17161a);
    }
    else if (style == HardwareStyle::Matriarch)
    {
        top = juce::Colour (0xff4a4a4d);
        bottom = juce::Colour (0xff080809);
        rim = juce::Colour (0xff9b9ba1);
        pointer = juce::Colour (0xfff4f1e8);
    }
    else if (style == HardwareStyle::PolyBrute)
    {
        top = juce::Colour (0xff33343a);
        bottom = juce::Colour (0xff050506);
        rim = juce::Colour (0xffe2ded4);
        pointer = juce::Colour (0xfff7f3ea);
    }
    else
    {
        top = juce::Colour (0xffd6d0c2);
        bottom = juce::Colour (0xff5a564d);
        rim = juce::Colour (0xff28282a);
        pointer = juce::Colour (0xff17161a);
    }

    const auto body = juce::Rectangle<float> (knobRadius * 2.0f, knobRadius * 2.0f).withCentre (centre);
    juce::ColourGradient bodyGradient (top, centre.x, body.getY(), bottom, centre.x, body.getBottom(), false);
    bodyGradient.addColour (0.5, style == HardwareStyle::MoogOne
                                     ? juce::Colour (0xffa49e90)
                                     : (style == HardwareStyle::PolyBrute
                                            ? juce::Colour (0xff18191c)
                                            : juce::Colour (0xff2b2b2f)));
    g.setGradientFill (bodyGradient);
    g.fillEllipse (body);

    // Knurled rim.
    g.setColour (rim.withAlpha (0.9f));
    g.drawEllipse (body.reduced (0.7f), 1.2f);
    g.setColour (juce::Colours::black.withAlpha (0.35f));
    for (int i = 0; i < 40; ++i)
    {
        const auto a = juce::MathConstants<float>::twoPi * (float) i / 40.0f;
        const auto p1 = juce::Point<float> (centre.x + std::sin (a) * knobRadius * 0.94f,
                                            centre.y - std::cos (a) * knobRadius * 0.94f);
        const auto p2 = juce::Point<float> (centre.x + std::sin (a) * knobRadius * 1.0f,
                                            centre.y - std::cos (a) * knobRadius * 1.0f);
        g.drawLine ({ p1, p2 }, 0.7f);
    }

    // Soft specular highlight makes the knob read as a machined part.
    g.setColour (juce::Colours::white.withAlpha (style == HardwareStyle::PolyBrute ? 0.16f : 0.10f));
    g.fillEllipse (juce::Rectangle<float> (knobRadius * 0.90f, knobRadius * 0.58f)
                       .withCentre ({ centre.x - knobRadius * 0.18f,
                                      centre.y - knobRadius * 0.30f }));

    // Pointer.
    const auto pointerLength = knobRadius * 0.82f;
    const auto pointerStart = knobRadius * 0.18f;
    const auto pointerEnd = juce::Point<float> (centre.x + std::sin (angle) * pointerLength,
                                                centre.y - std::cos (angle) * pointerLength);
    const auto pointerBegin = juce::Point<float> (centre.x + std::sin (angle) * pointerStart,
                                                  centre.y - std::cos (angle) * pointerStart);
    g.setColour (pointer);
    g.drawLine ({ pointerBegin, pointerEnd }, juce::jmax (1.6f, radius * 0.06f));

    if (style == HardwareStyle::Matriarch)
    {
        g.setColour (juce::Colours::white.withAlpha (0.15f));
        g.drawEllipse (body.reduced (2.0f), 0.8f);
    }
}

void TriadLookAndFeel::drawLinearSlider (juce::Graphics& g, int x, int y, int width, int height,
                                         float sliderPos, float, float,
                                         juce::Slider::SliderStyle style, juce::Slider& slider)
{
    const auto bounds = juce::Rectangle<int> (x, y, width, height).toFloat();

    if ((bool) slider.getProperties()["wheel"])
    {
        const auto wheel = bounds.reduced (2.0f, 2.0f);
        const auto wheelBody = juce::Rectangle<float> (wheel.getWidth(), wheel.getHeight())
                                   .withCentre (wheel.getCentre());

        juce::ColourGradient rubber (juce::Colour (0xff4a4a4e), wheelBody.getX(), wheelBody.getY(),
                                      juce::Colour (0xff0d0d0f), wheelBody.getRight(), wheelBody.getBottom(), false);
        g.setGradientFill (rubber);
        g.fillRoundedRectangle (wheelBody, 5.0f);

        g.setColour (juce::Colours::black.withAlpha (0.65f));
        for (float yLine = wheelBody.getY() + 6.0f; yLine < wheelBody.getBottom() - 4.0f; yLine += 5.0f)
            g.drawHorizontalLine ((int) yLine, wheelBody.getX() + 3.0f, wheelBody.getRight() - 3.0f);

        g.setColour (juce::Colour (0xffc9c6be));
        g.fillRoundedRectangle (juce::Rectangle<float> (wheelBody.getWidth() - 4.0f, 2.0f)
                                    .withCentre ({ wheelBody.getCentreX(), sliderPos }), 1.0f);

        g.setColour (juce::Colour (0xff8f8b84).withAlpha (0.7f));
        g.drawRoundedRectangle (wheelBody.reduced (0.5f), 5.0f, 1.0f);
        return;
    }

    if (style == juce::Slider::LinearVertical)
    {
        const auto track = juce::Rectangle<float> (bounds.getCentreX() - 2.5f, bounds.getY() + 4.0f,
                                                   5.0f, bounds.getHeight() - 8.0f);

        if (style_ == HardwareStyle::PolyBrute)
        {
            g.setColour (juce::Colour (0xff050506));
            g.fillRoundedRectangle (track, 2.0f);
            g.setColour (juce::Colour (0xff55565b));
            g.drawRoundedRectangle (track.reduced (0.5f), 2.0f, 1.0f);

            for (int i = 0; i <= 8; ++i)
            {
                const auto yPos = track.getY() + track.getHeight() * (float) i / 8.0f;
                g.setColour (juce::Colour (0xffd8d4ca).withAlpha (i % 2 == 0 ? 0.55f : 0.28f));
                g.drawHorizontalLine ((int) yPos,
                                      track.getX() - 5.0f, track.getX() - 1.0f);
                g.drawHorizontalLine ((int) yPos,
                                      track.getRight() + 1.0f, track.getRight() + 5.0f);
            }

            const auto grip = juce::Rectangle<float> (bounds.getWidth() - 10.0f, 20.0f)
                                  .withCentre ({ bounds.getCentreX(), sliderPos });
            g.setColour (juce::Colours::black.withAlpha (0.55f));
            g.fillRoundedRectangle (grip.translated (0.0f, 2.0f), 2.5f);
            g.setColour (juce::Colour (0xffe9e5dc));
            g.fillRoundedRectangle (grip, 2.5f);
            g.setColour (juce::Colour (0xffd2cec4));
            g.fillRect (grip.getX() + 2.0f, grip.getCentreY() - 1.0f,
                        grip.getWidth() - 4.0f, 2.0f);
            g.setColour (juce::Colour (0xff8e8a81));
            g.drawRoundedRectangle (grip, 2.5f, 1.0f);
        }
        else
        {
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
    }
    else if (style == juce::Slider::LinearHorizontal)
    {
        const auto track = juce::Rectangle<float> (bounds.getX() + 5.0f, bounds.getCentreY() - 2.5f,
                                                   bounds.getWidth() - 10.0f, 5.0f);
        g.setColour (juce::Colour (0xff2b2b30));
        g.fillRoundedRectangle (track, 2.5f);
        g.setColour (accent.withAlpha (0.85f));
        g.fillRoundedRectangle (track.withRight (juce::jlimit (track.getX() + 3.0f,
                                                               track.getRight(),
                                                               sliderPos)),
                                2.5f);

        const auto thumbX = juce::jlimit (track.getX(), track.getRight(), sliderPos);
        g.setColour (juce::Colour (0xffe9e6df));
        g.fillEllipse (thumbX - 4.0f, track.getCentreY() - 4.0f, 8.0f, 8.0f);
        g.setColour (juce::Colours::black.withAlpha (0.55f));
        g.drawEllipse (thumbX - 4.0f, track.getCentreY() - 4.0f, 8.0f, 8.0f, 1.0f);
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
    if (style_ == HardwareStyle::PolyBrute)
    {
        g.setColour (juce::Colour (0xff08090b));
        g.fillRoundedRectangle (bounds, 2.0f);
        g.setColour (juce::Colour (0xffd9d5cb).withAlpha (0.55f));
        g.drawRoundedRectangle (bounds.reduced (0.5f), 2.0f, 1.0f);
    }
    else
    {
        juce::ColourGradient background (juce::Colour (0xff2a2a2e), 0.0f, 0.0f,
                                         juce::Colour (0xff141417), 0.0f, bounds.getBottom(), false);
        g.setGradientFill (background);
        g.fillRoundedRectangle (bounds, 2.5f);
        g.setColour (juce::Colour (0xff6d6a63).withAlpha (0.8f));
        g.drawRoundedRectangle (bounds.reduced (0.5f), 2.5f, 1.0f);
    }

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

    if (style_ == HardwareStyle::PolyBrute)
    {
        g.setColour (base.withAlpha (0.96f));
        g.fillRoundedRectangle (bounds, 2.5f);
        g.setColour ((button.getToggleState() ? accent : juce::Colour (0xffe4e0d7)).withAlpha (0.65f));
        g.drawRoundedRectangle (bounds.reduced (0.5f), 2.5f, 1.0f);
    }
    else
    {
        g.setColour (base);
        g.fillRoundedRectangle (bounds, 3.0f);
        g.setColour ((button.getToggleState() ? accent : juce::Colour (0xff505057)).withAlpha (0.9f));
        g.drawRoundedRectangle (bounds.reduced (0.5f), 3.0f, 1.0f);
    }
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
