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
    const auto knobRadius = radius * 0.80f;
    const auto arcRadius = radius * 0.9f;
    const auto accentColour = style == HardwareStyle::PolyBrute
                                  ? juce::Colour (0xff3f93e8)
                                  : (style == HardwareStyle::MoogOne
                                         ? juce::Colour (0xffd9a441)
                                         : juce::Colour (0xffcf3f23));

    // Engraved tick ring.
    g.setColour ((slider.getProperties()["darkInk"] ? juce::Colour (0xff27302f) : juce::Colour (0xffd9d5cb)).withAlpha (style == HardwareStyle::PolyBrute ? 0.68f
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
    if (style != HardwareStyle::Matriarch && ! (bool) slider.getProperties()["panelControl"])
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

    // Hardware-specific grip silhouettes: Moog scallops, Matriarch flutes,
    // and the narrower Arturia rubber grip. These are geometry, not a texture.
    const bool silver = (bool) slider.getProperties()["silverKnob"];
    const bool one = style == HardwareStyle::MoogOne;
    const bool mat = style == HardwareStyle::Matriarch;
    const auto body = juce::Rectangle<float> (knobRadius * 2.0f, knobRadius * 2.0f).withCentre (centre);
    g.setColour (juce::Colours::black.withAlpha (0.65f));
    g.fillEllipse (body.expanded (1.8f).translated (0.0f, 2.0f));

    juce::Path grip;
    const int lobes = one ? 10 : (mat ? 24 : 32);
    constexpr int segments = 256;
    for (int i = 0; i <= segments; ++i)
    {
        const float a = juce::MathConstants<float>::twoPi * (float) i / segments;
        const float depth = one ? 0.065f : (mat ? 0.027f : 0.018f);
        const float r = knobRadius * (1.0f - depth * (0.5f + 0.5f * std::cos (lobes * (a - angle))));
        const juce::Point<float> point (centre.x + r * std::sin (a), centre.y - r * std::cos (a));
        if (i == 0) grip.startNewSubPath (point); else grip.lineTo (point);
    }
    grip.closeSubPath();
    juce::ColourGradient rubber (juce::Colour (silver ? 0xffe1dfd7 : 0xff454649),
                                centre.x, body.getY(),
                                juce::Colour (silver ? 0xff797a78 : 0xff090a0c),
                                centre.x, body.getBottom(), false);
    rubber.addColour (0.45, juce::Colour (silver ? 0xffb9bcb9 : 0xff202125));
    g.setGradientFill (rubber);
    g.fillPath (grip);
    g.setColour (juce::Colour (one ? 0xffaaa9a3 : 0xff55565a));
    g.strokePath (grip, juce::PathStrokeType (one ? 0.85f : 0.5f));

    if ((bool) slider.getProperties()["chickenHead"])
    {
        juce::Path selector;
        selector.startNewSubPath (-knobRadius * .29f, knobRadius * .75f);
        selector.lineTo (-knobRadius * .43f, -knobRadius * .05f);
        selector.lineTo (-knobRadius * .11f, -knobRadius * 1.05f);
        selector.lineTo (knobRadius * .11f, -knobRadius * 1.05f);
        selector.lineTo (knobRadius * .43f, -knobRadius * .05f);
        selector.lineTo (knobRadius * .29f, knobRadius * .75f);
        selector.closeSubPath();
        const auto transform = juce::AffineTransform::rotation (angle).translated (centre.x, centre.y);
        g.setColour (juce::Colour (0xff111215)); g.fillPath (selector, transform);
        g.setColour (juce::Colour (0xffd3d6d1)); g.strokePath (selector, juce::PathStrokeType (1.0f), transform);
        g.fillEllipse (juce::Rectangle<float> (knobRadius * .65f, knobRadius * .65f).withCentre (centre));
        return;
    }

    const auto capRadius = knobRadius * (one ? 0.66f : (mat ? 0.67f : 0.78f));
    const auto cap = juce::Rectangle<float> (capRadius * 2.0f, capRadius * 2.0f).withCentre (centre);
    if (mat || one || silver)
    {
        // Spun aluminium inset, surrounded by the black scalloped skirt.
        juce::ColourGradient metal (juce::Colour (0xffe0e3df), cap.getX(), cap.getY(),
                                   juce::Colour (0xff686f70), cap.getRight(), cap.getBottom(), false);
        metal.addColour (0.40, juce::Colour (0xffaeb6b6));
        metal.addColour (0.53, juce::Colour (0xfff3f3e9));
        metal.addColour (0.65, juce::Colour (0xffb7bfbd));
        g.setGradientFill (metal);
        g.fillEllipse (cap);
        g.setColour (juce::Colour (0xff43494a));
        g.drawEllipse (cap, 0.65f);
        // Fine concentric machining, kept subtle at the smaller control sizes.
        g.setColour (juce::Colours::black.withAlpha (0.07f));
        for (float r = capRadius * 0.25f; r < capRadius; r += 2.2f)
            g.drawEllipse (juce::Rectangle<float> (r * 2, r * 2).withCentre (centre), 0.4f);
    }
    else
    {
        juce::ColourGradient capShade (juce::Colour (0xff2a2c30), cap.getX(), cap.getY(),
                                      juce::Colour (0xff111216), cap.getRight(), cap.getBottom(), false);
        g.setGradientFill (capShade);
        g.fillEllipse (cap);
        g.setColour (juce::Colour (0xff55565a));
        g.drawEllipse (cap, 0.6f);
    }

    const auto polar = [&] (float r) { return juce::Point<float> (centre.x + std::sin (angle) * r,
                                                               centre.y - std::cos (angle) * r); };
    if (one)
    {
        // Traditional One controls have a white index dot in the black skirt.
        g.setColour (juce::Colour (0xfff5f3e9));
        const float dot = juce::jmax (1.8f, knobRadius * 0.11f);
        g.fillEllipse (juce::Rectangle<float> (dot * 2, dot * 2).withCentre (polar (knobRadius * 0.82f)));
    }
    else
    {
        g.setColour (mat || silver ? juce::Colour (0xff25292b) : juce::Colour (0xfff1eee5));
        g.drawLine ({ polar (capRadius * (mat ? 0.08f : 0.30f)),
                      polar (mat ? capRadius * 0.93f : knobRadius * 0.94f) },
                    juce::jmax (1.5f, knobRadius * 0.09f));
    }

}

void TriadLookAndFeel::drawLinearSlider (juce::Graphics& g, int x, int y, int width, int height,
                                         float sliderPos, float, float,
                                         juce::Slider::SliderStyle style, juce::Slider& slider)
{
    const auto bounds = juce::Rectangle<int> (x, y, width, height).toFloat();

    if ((bool) slider.getProperties()["numericField"])
    {
        g.setColour (juce::Colour (0xffeef0e9));
        g.fillRect (bounds);
        g.setColour (juce::Colour (0xff8c9990));
        g.drawRect (bounds.reduced (0.5f), 0.8f);
        g.setColour (juce::Colour (0xffad534e));
        g.fillRect (bounds.getX(), bounds.getBottom() - 2.0f,
                    bounds.getWidth() * (float) slider.valueToProportionOfLength (slider.getValue()), 1.0f);
        return;
    }

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
        else if (style_ == HardwareStyle::Matriarch)
        {
            g.setColour (juce::Colour (0xff0c0d0e));
            g.fillRect (track);
            for (int i = 0; i <= 32; ++i)
            {
                const auto yy = track.getY() + i * track.getHeight() / 32.0f;
                g.setColour (juce::Colour (0xffd9ddd4).withAlpha (i % 4 == 0 ? 0.8f : 0.45f));
                g.drawHorizontalLine ((int) yy, bounds.getCentreX() - 15.0f, bounds.getCentreX() + 15.0f);
            }
            const auto grip = juce::Rectangle<float> (bounds.getWidth() - 4.0f, 17.0f)
                                  .withCentre ({ bounds.getCentreX(), sliderPos });
            g.setColour (juce::Colours::black.withAlpha (0.6f)); g.fillRect (grip.translated (1.0f, 3.0f));
            juce::ColourGradient cap (juce::Colour (0xff696b6c), grip.getX(), grip.getY(),
                                     juce::Colour (0xff08090a), grip.getX(), grip.getBottom(), false);
            g.setGradientFill (cap); g.fillRoundedRectangle (grip, 2.0f);
            g.setColour (juce::Colour (0xffebece2));
            g.drawHorizontalLine ((int) sliderPos, grip.getX() + 2.0f, grip.getRight() - 2.0f);
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
    if ((bool) box.getProperties()["continuumField"])
    {
        g.setColour (juce::Colour (0xffeef0e9));
        g.fillRect (bounds);
        g.setColour (juce::Colour (0xff8c9990));
        g.drawRect (bounds.reduced (0.5f), 0.8f);
    }
    else if (style_ == HardwareStyle::PolyBrute)
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
