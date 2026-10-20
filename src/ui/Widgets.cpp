#include "ui/Widgets.h"
#include "ui/LookAndFeel.h"

#include <cmath>

namespace triad::ui
{

// ---------------------------------------------------------------------------
// HardwareSwitch
// ---------------------------------------------------------------------------

HardwareSwitch::HardwareSwitch (juce::RangedAudioParameter& parameter, Orientation orientation)
    : parameter_ (parameter), orientation_ (orientation)
{
    labels_ = parameter.getAllValueStrings();

    if (labels_.isEmpty())
    {
        labels_.add ("Off");
        labels_.add ("On");
    }

    positions_ = juce::jlimit (2, 4, labels_.size());

    attachment_ = std::make_unique<juce::ParameterAttachment> (
        parameter, [this] (float value) { setPositionFromValue (value); }, nullptr);

    setPositionFromValue (parameter.convertFrom0to1 (parameter.getValue()));
    setMouseCursor (juce::MouseCursor::PointingHandCursor);
    setRepaintsOnMouseActivity (true);
}

HardwareSwitch::~HardwareSwitch() = default;

void HardwareSwitch::setAccent (juce::Colour colour)
{
    accent_ = colour;
    repaint();
}

void HardwareSwitch::setPositionFromValue (float value)
{
    const auto index = juce::jlimit (0, positions_ - 1, (int) std::lround (value));
    if (index != current_)
    {
        current_ = index;
        repaint();
    }
}

void HardwareSwitch::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced (1.0f);

    juce::ColourGradient plate (juce::Colour (0xff3a3a3f), bounds.getX(), bounds.getY(),
                                 juce::Colour (0xff121214), bounds.getX(), bounds.getBottom(), false);
    g.setGradientFill (plate);
    g.fillRoundedRectangle (bounds, 3.0f);
    g.setColour (juce::Colour (0xff6f6f76).withAlpha (0.85f));
    g.drawRoundedRectangle (bounds.reduced (0.5f), 3.0f, 1.0f);
    g.setColour (juce::Colours::white.withAlpha (0.08f));
    g.drawHorizontalLine ((int) bounds.getY() + 1, bounds.getX() + 5.0f, bounds.getRight() - 5.0f);

    const auto dotArea = bounds.removeFromRight (15.0f).reduced (3.0f, 4.0f);
    const auto count = positions_;
    const auto step = count > 1 ? dotArea.getHeight() / (float) (count - 1) : 0.0f;
    const auto pivot = juce::Point<float> (dotArea.getCentreX(), bounds.getCentreY());

    for (int i = 0; i < count; ++i)
    {
        const auto point = juce::Point<float> (dotArea.getCentreX(), dotArea.getY() + step * (float) i + 1.0f);
        const auto active = i == current_;

        g.setColour (active ? accent_.withAlpha (0.28f) : juce::Colours::white.withAlpha (0.04f));
        g.fillEllipse (juce::Rectangle<float> (11.0f, 11.0f).withCentre (point));
        g.setColour (active ? accent_ : juce::Colour (0xff8a8790));
        g.fillEllipse (juce::Rectangle<float> (active ? 5.5f : 3.6f, active ? 5.5f : 3.6f)
                           .withCentre (point));
    }

    // Chrome toggle lever.
    const auto activePoint = juce::Point<float> (dotArea.getCentreX(),
                                                 dotArea.getY() + step * (float) current_ + 1.0f);
    g.setColour (juce::Colours::black.withAlpha (0.55f));
    g.drawLine ({ pivot, activePoint }, 3.6f);
    g.setColour (juce::Colour (0xffd8d5cd));
    g.drawLine ({ pivot, activePoint }, 2.0f);
    g.setColour (juce::Colour (0xfff4f1e8));
    g.fillEllipse (juce::Rectangle<float> (6.0f, 6.0f)
                       .withCentre (pivot.translated (0.0f, 0.0f)));

    const auto textArea = bounds.reduced (6.0f, 0.0f).withTrimmedRight (3.0f);
    g.setColour (accent_.brighter (0.35f));
    g.setFont (juce::Font (juce::FontOptions (9.0f, juce::Font::bold)));
    g.drawFittedText (labels_[juce::jlimit (0, labels_.size() - 1, current_)], textArea.toNearestInt(),
                      juce::Justification::centredLeft, 1, 0.62f);
}

void HardwareSwitch::mouseDown (const juce::MouseEvent& event)
{
    handleClick (event.position);
    if (event.mods.isRightButtonDown() && positions_ > 2)
        attachment_->setValueAsCompleteGesture (0.0f);
}

void HardwareSwitch::mouseDrag (const juce::MouseEvent& event)
{
    handleClick (event.position);
}

void HardwareSwitch::handleClick (juce::Point<float> position)
{
    auto bounds = getLocalBounds().toFloat().reduced (5.0f);
    const auto t = orientation_ == Orientation::Horizontal
                     ? juce::jlimit (0.0f, 1.0f, (position.x - bounds.getX()) / juce::jmax (1.0f, bounds.getWidth()))
                     : juce::jlimit (0.0f, 1.0f, (position.y - bounds.getY()) / juce::jmax (1.0f, bounds.getHeight()));

    const auto index = juce::jlimit (0, positions_ - 1, (int) std::lround (t * (float) (positions_ - 1)));
    if (index != current_)
        attachment_->setValueAsCompleteGesture ((float) index);
}

// ---------------------------------------------------------------------------
// LedButton
// ---------------------------------------------------------------------------

LedButton::LedButton (juce::RangedAudioParameter& parameter, juce::String label)
    : parameter_ (parameter), label_ (std::move (label))
{
    attachment_ = std::make_unique<juce::ParameterAttachment> (
        parameter, [this] (float value) { setOn (value); }, nullptr);

    setOn (parameter.convertFrom0to1 (parameter.getValue()));
    setMouseCursor (juce::MouseCursor::PointingHandCursor);
    setRepaintsOnMouseActivity (true);
}

LedButton::~LedButton() = default;

void LedButton::setAccent (juce::Colour colour)
{
    accent_ = colour;
    repaint();
}

void LedButton::setOn (float value)
{
    const auto on = value > 0.5f;
    if (on != on_)
    {
        on_ = on;
        repaint();
    }
}

void LedButton::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced (1.0f);

    // Metal bezel.
    juce::ColourGradient plate (juce::Colour (0xff44444a), bounds.getX(), bounds.getY(),
                                 juce::Colour (0xff151518), bounds.getX(), bounds.getBottom(), false);
    g.setGradientFill (plate);
    g.fillRoundedRectangle (bounds, 3.5f);

    auto inner = bounds.reduced (2.5f);
    if (on_)
    {
        juce::ColourGradient lit (accent_.brighter (0.35f), inner.getCentreX(), inner.getY(),
                                   accent_.darker (0.45f), inner.getCentreX(), inner.getBottom(), false);
        g.setGradientFill (lit);
        g.fillRoundedRectangle (inner, 2.5f);

        g.setColour (accent_.brighter (0.6f).withAlpha (0.35f));
        g.drawRoundedRectangle (inner.expanded (1.0f), 3.0f, 1.2f);
    }
    else
    {
        g.setColour (juce::Colour (0xff1a1a1d));
        g.fillRoundedRectangle (inner, 2.5f);
        g.setColour (juce::Colour (0xff3a3a40));
        g.drawRoundedRectangle (inner, 2.5f, 0.8f);
    }

    g.setColour (juce::Colours::white.withAlpha (0.09f));
    g.drawHorizontalLine ((int) bounds.getY() + 1, bounds.getX() + 4.0f, bounds.getRight() - 4.0f);

    g.setColour (on_ ? juce::Colour (0xfff6f1e6) : juce::Colour (0xffbdb9b0));
    g.setFont (juce::Font (juce::FontOptions (9.5f, juce::Font::bold)));
    g.drawFittedText (label_, inner.toNearestInt().reduced (7, 0),
                      juce::Justification::centredLeft, 1, 0.72f);

    // Small status jewel.
    const auto jewel = juce::Rectangle<float> (5.0f, 5.0f)
                           .withCentre ({ bounds.getRight() - 8.0f, bounds.getCentreY() });
    g.setColour (on_ ? juce::Colours::white.withAlpha (0.9f) : juce::Colour (0xff3a3a3e));
    g.fillEllipse (jewel);
}

void LedButton::mouseDown (const juce::MouseEvent&)
{
    attachment_->setValueAsCompleteGesture (on_ ? 0.0f : 1.0f);
}

// ---------------------------------------------------------------------------
// Jack
// ---------------------------------------------------------------------------

Jack::Jack (int patchIndex, bool isOutput, juce::String name, juce::Colour accent)
    : patchIndex_ (patchIndex), isOutput_ (isOutput), name_ (std::move (name)), accent_ (accent)
{
    setMouseCursor (juce::MouseCursor::PointingHandCursor);
    setTooltip (name_ + (isOutput_ ? " (output)" : " (input)")
                + " - click, then click another jack to patch. Right-click clears.");
}

void Jack::paint (juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat().reduced (0.5f);
    const auto radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;
    const auto centre = bounds.getCentre();

    if (selected_ || hover_)
    {
        g.setColour (accent_.withAlpha (selected_ ? 0.5f : 0.25f));
        g.fillEllipse (juce::Rectangle<float> (radius * 2.0f + 7.0f, radius * 2.0f + 7.0f)
                           .withCentre (centre));
    }

    if (cableCount_ > 0)
    {
        g.setColour (accent_);
        g.fillEllipse (juce::Rectangle<float> (5.0f, 5.0f)
                           .withCentre ({ bounds.getRight() - 1.0f, bounds.getY() + 1.0f }));
    }

    const auto outer = juce::Rectangle<float> (radius * 2.0f, radius * 2.0f).withCentre (centre);

    juce::ColourGradient ring (juce::Colour (0xffd8d4ca), outer.getX(), outer.getY(),
                                juce::Colour (0xff3b3a38), outer.getRight(), outer.getBottom(), false);
    ring.addColour (0.45, juce::Colour (0xff8b877f));
    g.setGradientFill (ring);
    g.fillEllipse (outer);

    g.setColour (juce::Colours::black.withAlpha (0.95f));
    g.fillEllipse (outer.reduced (radius * 0.3f));

    juce::ColourGradient hole (juce::Colour (0xff161618), centre.x, centre.y,
                                juce::Colour (0xff000000), centre.x, bounds.getBottom(), false);
    g.setGradientFill (hole);
    g.fillEllipse (outer.reduced (radius * 0.58f));

    g.setColour (juce::Colours::white.withAlpha (0.18f));
    g.drawEllipse (outer.reduced (radius * 0.42f), 0.7f);
}

void Jack::mouseDown (const juce::MouseEvent& event)
{
    if (event.mods.isRightButtonDown())
    {
        if (onJackRightClicked)
            onJackRightClicked (*this);
        return;
    }

    if (onJackClicked)
        onJackClicked (*this);
}

void Jack::mouseEnter (const juce::MouseEvent&)
{
    hover_ = true;
    repaint();
}

void Jack::mouseExit (const juce::MouseEvent&)
{
    hover_ = false;
    repaint();
}

void Jack::setSelected (bool selected)
{
    if (selected_ != selected)
    {
        selected_ = selected;
        repaint();
    }
}

void Jack::setCableCount (int count)
{
    if (cableCount_ != count)
    {
        cableCount_ = count;
        repaint();
    }
}

// ---------------------------------------------------------------------------
// SourceMatrix
// ---------------------------------------------------------------------------

SourceMatrix::SourceMatrix (juce::AudioProcessorValueTreeState& state,
                            std::vector<juce::String> sourceParameterIds,
                            int rowCount, int columnCount)
    : state_ (state), ids_ (std::move (sourceParameterIds)), rows_ (rowCount), columns_ (columnCount)
{
    for (const auto& id : ids_)
        pointers_.push_back (state_.getRawParameterValue (id));

    setTooltip ("Click a cell to assign that source to the slot in that column.");
    startTimerHz (8);
}

void SourceMatrix::timerCallback()
{
    repaint();
}

int SourceMatrix::rowAt (juce::Point<int> position) const
{
    const auto height = juce::jmax (1, getHeight());
    return juce::jlimit (0, rows_ - 1, position.y * rows_ / height);
}

int SourceMatrix::columnAt (juce::Point<int> position) const
{
    const auto width = juce::jmax (1, getWidth());
    return juce::jlimit (0, columns_ - 1, position.x * columns_ / width);
}

void SourceMatrix::paint (juce::Graphics& g)
{
    g.setColour (juce::Colour (0xff0d0d10));
    g.fillRoundedRectangle (getLocalBounds().toFloat(), 2.0f);

    const auto cellWidth = (float) getWidth() / (float) columns_;
    const auto cellHeight = (float) getHeight() / (float) rows_;

    for (int column = 0; column < columns_; ++column)
    {
        const auto source = (int) std::lround (pointers_[(size_t) column] != nullptr
                                                   ? pointers_[(size_t) column]->load()
                                                   : 0.0f);

        for (int row = 0; row < rows_; ++row)
        {
            auto cell = juce::Rectangle<float> ((float) column * cellWidth,
                                                (float) row * cellHeight,
                                                cellWidth, cellHeight).reduced (1.2f);

            const auto active = row == source;
            g.setColour (active ? juce::Colour (0xff3f93e8)
                                : juce::Colour (0xff232328));
            g.fillRoundedRectangle (cell, 1.5f);

            if (active)
            {
                g.setColour (juce::Colours::white.withAlpha (0.35f));
                g.drawRoundedRectangle (cell, 1.5f, 0.8f);
            }
        }

        if (column == selectedColumn_)
        {
            g.setColour (juce::Colour (0xffffffff).withAlpha (0.35f));
            g.drawRect (juce::Rectangle<float> ((float) column * cellWidth, 0.0f,
                                                cellWidth, (float) getHeight()).reduced (0.6f),
                        1.0f);
        }
    }

    g.setColour (juce::Colour (0xff8f8c85));
    g.setFont (juce::Font (juce::FontOptions (6.5f)));
    for (int column = 0; column < columns_; ++column)
        g.drawText (juce::String (column + 1),
                    (int) ((float) column * cellWidth), getHeight() - 9, (int) cellWidth, 9,
                    juce::Justification::centred);
}

void SourceMatrix::mouseDown (const juce::MouseEvent& event)
{
    const auto column = columnAt (event.getPosition());
    const auto row = rowAt (event.getPosition());

    selectedColumn_ = column;

    if (auto* parameter = state_.getParameter (ids_[(size_t) column]))
        parameter->setValueNotifyingHost (parameter->convertTo0to1 ((float) row));

    if (onColumnSelected != nullptr)
        onColumnSelected (column);

    repaint();
}

// ---------------------------------------------------------------------------
// LcdDisplay
// ---------------------------------------------------------------------------

void LcdDisplay::setText (juce::String header, juce::String body, juce::String footer)
{
    header_ = std::move (header);
    body_ = std::move (body);
    footer_ = std::move (footer);
    repaint();
}

void LcdDisplay::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    g.setColour (juce::Colour (0xff071016));
    g.fillRoundedRectangle (bounds, 3.0f);

    juce::ColourGradient glow (juce::Colour (0xff0f2a33), bounds.getCentreX(), bounds.getY(),
                               juce::Colour (0xff03080b), bounds.getCentreX(), bounds.getBottom(), false);
    g.setGradientFill (glow);
    g.fillRoundedRectangle (bounds.reduced (1.5f), 3.0f);

    g.setColour (juce::Colour (0xff5fd6d0).withAlpha (0.12f));
    for (float y = bounds.getY() + 2.0f; y < bounds.getBottom(); y += 3.0f)
        g.drawHorizontalLine ((int) y, bounds.getX() + 2.0f, bounds.getRight() - 2.0f);

    g.setColour (juce::Colour (0xff8ff3ea));
    g.setFont (juce::Font (juce::FontOptions (11.0f, juce::Font::bold)));
    g.drawText (header_, getLocalBounds().reduced (7, 4).removeFromTop (16),
                juce::Justification::centredLeft);

    g.setColour (juce::Colour (0xffbdfaf3).withAlpha (0.85f));
    g.setFont (juce::Font (juce::FontOptions (11.0f)));
    g.drawFittedText (body_, getLocalBounds().reduced (7, 4).withTrimmedTop (18).withTrimmedBottom (14),
                      juce::Justification::topLeft, 4);

    g.setColour (juce::Colour (0xff5fd6d0).withAlpha (0.7f));
    g.setFont (juce::Font (juce::FontOptions (9.0f)));
    g.drawText (footer_, getLocalBounds().reduced (7, 3).removeFromBottom (12),
                juce::Justification::centredLeft);

    g.setColour (juce::Colour (0xff3a7f85).withAlpha (0.8f));
    g.drawRoundedRectangle (bounds.reduced (0.75f), 3.0f, 1.0f);
}

} // namespace triad::ui
