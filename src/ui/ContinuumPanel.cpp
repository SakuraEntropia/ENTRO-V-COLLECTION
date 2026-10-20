#include "ui/ContinuumPanel.h"
#include "ui/LookAndFeel.h"
#include "engines/Continuum.h"

#include <cmath>

namespace triad::ui
{

namespace
{
juce::Colour continuumAccent() { return juce::Colour (0xff2fb6a8); }

juce::String formulaAbbreviation (int formula)
{
    switch (formula)
    {
        case 1:  return "DIR";
        case 2:  return "X";
        case 3:  return "Y";
        case 4:  return "Z";
        case 5:  return "XY";
        case 6:  return "XZ";
        case 7:  return "YZ";
        case 8:  return "XYZ";
        case 9:  return "-1";
        case 10: return "SFT";
        case 11: return "GATE";
        default: return {};
    }
}
} // namespace

// ---------------------------------------------------------------------------
ContinuumSurface::ContinuumSurface (EditorHost& host) : host_ (host)
{
    setOpaque (true);
    setMouseCursor (juce::MouseCursor::CrosshairCursor);
    juce::Desktop::getInstance().addGlobalMouseListener (this);
    startTimerHz (24);
}

ContinuumSurface::~ContinuumSurface()
{
    if (active_)
        endNote();

    juce::Desktop::getInstance().removeGlobalMouseListener (this);
}

juce::Rectangle<float> ContinuumSurface::playingArea() const
{
    return getLocalBounds().toFloat().reduced (6.0f, 0.0f)
        .withTrimmedTop (24.0f).withTrimmedBottom (24.0f);
}

float ContinuumSurface::pitchForX (float x) const
{
    const auto area = playingArea();
    const auto normalised = juce::jlimit (0.0f, 0.9999f,
                                          (x - area.getX()) / juce::jmax (1.0f, area.getWidth()));
    return 36.0f + normalised * 48.0f;
}

juce::Point<float> ContinuumSurface::pointFor (int note, float expr, float timbre) const
{
    const auto area = playingArea();
    const auto pitch = (float) note + expr;
    return { area.getX() + (pitch - 36.0f) / 48.0f * area.getWidth(),
             area.getY() + (1.0f - timbre) * area.getHeight() };
}

void ContinuumSurface::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    const auto area = playingArea();

    g.fillAll (juce::Colour (0xff0b0b0c));

    // The red field is inset between black header/footer rails, matching the
    // hardware where the pitch labels are not printed on the playing fabric.
    g.setColour (juce::Colour (0xff11110f));
    g.fillRect (bounds.withHeight (24.0f));
    g.fillRect (bounds.withTrimmedTop (bounds.getBottom() - 24.0f));

    const auto stripeCount = 32;
    const auto stripeWidth = area.getWidth() / (float) stripeCount;

    for (int i = 0; i < stripeCount; ++i)
    {
        const auto x = area.getX() + (float) i * stripeWidth;
        g.setColour (juce::Colour (0xffd8232a));
        g.fillRect (x, area.getY(), stripeWidth * 0.78f, area.getHeight());
    }

    juce::ColourGradient shade (juce::Colour (0xff050505).withAlpha (0.0f), 0.0f, area.getY(),
                                juce::Colour (0xff050505).withAlpha (0.28f), 0.0f,
                                area.getBottom(), false);
    g.setGradientFill (shade);
    g.fillRect (area);

    g.setColour (juce::Colour (0xff1b1a16));
    g.drawRect (area, 2.0f);
    g.setColour (juce::Colour (0xfff4e8d2).withAlpha (0.22f));
    g.drawRect (area, 0.6f);

    // Pitch grid: one vertical line per octave.
    for (int note = 36; note <= 84; note += 12)
    {
        const auto x = area.getX() + (float) (note - 36) / 48.0f * area.getWidth();
        g.setColour (juce::Colour (0xfff4e8d2).withAlpha (0.34f));
        g.drawVerticalLine ((int) x, area.getY(), area.getBottom());

        g.setColour (juce::Colour (0xfff4e8d2).withAlpha (0.9f));
        g.setFont (juce::Font (juce::FontOptions (8.0f)));
        g.drawText (juce::MidiMessage::getMidiNoteName (note, true, true, 3),
                    (int) x + 4, (int) bounds.getBottom() - 20, 28, 12,
                    juce::Justification::centredLeft);
    }

    // Timbre lines.
    for (int i = 1; i < 4; ++i)
    {
        const auto y = area.getY() + area.getHeight() * (float) i / 4.0f;
        g.setColour (juce::Colour (0xff111111).withAlpha (0.34f));
        g.drawHorizontalLine ((int) y, area.getX(), area.getRight());
    }

    g.setColour (juce::Colour (0xfff4e8d2).withAlpha (0.8f));
    g.setFont (juce::Font (juce::FontOptions (8.0f)).withExtraKerningFactor (0.24f));
    g.drawText ("X  PITCH      Y  TIMBRE      Z  PRESSURE  (WHEEL)",
                14, 7, 500, 12, juce::Justification::centredLeft);

    if (! active_)
    {
        g.setColour (juce::Colour (0xffd8f0ec).withAlpha (0.65f));
        g.setFont (juce::Font (juce::FontOptions (13.0f)));
        g.drawText ("TOUCH THE SURFACE  ·  DRAG FOR PITCH AND TIMBRE  ·  WHEEL FOR PRESSURE",
                    bounds.toNearestInt().reduced (12), juce::Justification::centred);
        return;
    }

    const auto point = pointFor (note_, exprPitch_, timbre_);
    const auto radius = 10.0f + pressure_ * 26.0f;

    g.setColour (juce::Colour (0xfff4e8d2).withAlpha (0.12f));
    g.fillEllipse (point.x - radius, point.y - radius, radius * 2.0f, radius * 2.0f);
    g.setColour (juce::Colour (0xfff4e8d2).withAlpha (0.9f));
    g.drawEllipse (point.x - radius, point.y - radius, radius * 2.0f, radius * 2.0f, 1.5f);
    g.setColour (juce::Colour (0xff2fb6a8));
    g.fillEllipse (point.x - 5.0f, point.y - 5.0f, 10.0f, 10.0f);

    if (lastPoint_.getDistanceFrom (point) > 1.0f)
    {
        g.setColour (juce::Colour (0xfff4e8d2).withAlpha (0.45f));
        g.drawLine (juce::Line<float> (lastPoint_, point), 1.0f);
    }

    g.setColour (juce::Colour (0xffe9f7f3));
    g.setFont (juce::Font (juce::FontOptions (10.0f)));
    const juce::String readout = "NOTE " + juce::String (note_)
                                 + "  X " + juce::String (exprPitch_, 2)
                                 + "  Y " + juce::String (timbre_, 2)
                                 + "  Z " + juce::String (pressure_, 2);
    g.drawText (readout, (int) point.x + 18, (int) point.y - 8, 280, 16,
                juce::Justification::centredLeft);
}

void ContinuumSurface::mouseDown (const juce::MouseEvent& event)
{
    if (active_)
        endNote();

    beginNote (event.position);
}

void ContinuumSurface::beginNote (juce::Point<float> position)
{
    const auto pitch = pitchForX (position.x);
    note_ = juce::jlimit (0, 127, (int) std::floor (pitch));
    exprPitch_ = pitch - (float) note_;
    timbre_ = juce::jlimit (0.02f, 0.98f, 1.0f - (position.y - playingArea().getY()) / juce::jmax (1.0f, playingArea().getHeight()));
    pressure_ = 0.8f;
    active_ = true;
    lastPoint_ = pointFor (note_, exprPitch_, timbre_);
    host_.continuumNoteOn (note_, exprPitch_, timbre_, pressure_);
    repaint();
}

void ContinuumSurface::updateFromPosition (juce::Point<float> position, bool allowNoteChange)
{
    const auto pitch = pitchForX (position.x);
    const auto newNote = juce::jlimit (0, 127, (int) std::floor (pitch));
    const auto newExpr = pitch - (float) newNote;

    if (allowNoteChange && newNote != note_)
    {
        host_.continuumNoteOff (note_);
        note_ = newNote;
        host_.continuumNoteOn (note_, newExpr, timbre_, pressure_);
    }

    exprPitch_ = newExpr;
    timbre_ = juce::jlimit (0.02f, 0.98f, 1.0f - (position.y - playingArea().getY()) / juce::jmax (1.0f, playingArea().getHeight()));
    lastPoint_ = pointFor (note_, exprPitch_, timbre_);
    host_.continuumSetExpression (note_, exprPitch_, timbre_, pressure_);
    repaint();
}

void ContinuumSurface::mouseDrag (const juce::MouseEvent& event)
{
    if (active_)
        updateFromPosition (event.position, true);
}

void ContinuumSurface::mouseUp (const juce::MouseEvent&)
{
    endNote();
}

void ContinuumSurface::endNote()
{
    if (! active_)
        return;

    host_.continuumNoteOff (note_);
    active_ = false;
    repaint();
}

void ContinuumSurface::mouseWheelMove (const juce::MouseEvent&,
                                       const juce::MouseWheelDetails& wheel)
{
    pressure_ = juce::jlimit (0.02f, 1.0f, pressure_ + wheel.deltaY * 0.5f);
    if (active_)
        host_.continuumSetExpression (note_, exprPitch_, timbre_, pressure_);
    repaint();
}

void ContinuumSurface::timerCallback()
{
    // Mouse-up can be lost when the window loses focus or the pointer is
    // released outside the component.  Never leave a note latched.
    const auto modifiers = juce::ModifierKeys::getCurrentModifiersRealtime();
    if (active_ && ! modifiers.isAnyMouseButtonDown())
        endNote();

    if (active_)
        repaint();
}

void ContinuumSurface::visibilityChanged()
{
    if (! isVisible() && active_)
        endNote();
}

void ContinuumSurface::focusLost (juce::Component::FocusChangeType)
{
    if (active_ && ! juce::ModifierKeys::getCurrentModifiersRealtime().isAnyMouseButtonDown())
        endNote();
}

// ---------------------------------------------------------------------------
ContinuumMatrix::ContinuumMatrix (EditorHost& host) : host_ (host)
{
    setOpaque (true);
    startTimerHz (10);
}

juce::Rectangle<float> ContinuumMatrix::cellBounds (int source, int destination) const
{
    const auto area = getLocalBounds().toFloat().reduced (6.0f);
    const auto left = 104.0f;
    const auto top = 100.0f;
    const auto cellW = (area.getWidth() - left) / (float) ContinuumEngine::kDestinations;
    const auto cellH = (area.getHeight() - top) / (float) ContinuumEngine::kSources;
    return { area.getX() + left + (float) destination * cellW,
             area.getY() + top + (float) source * cellH,
             cellW - 1.0f, cellH - 1.0f };
}

juce::Point<int> ContinuumMatrix::cellAt (juce::Point<float> position) const
{
    const auto area = getLocalBounds().toFloat().reduced (6.0f);
    const auto left = 104.0f;
    const auto top = 100.0f;

    if (position.x < area.getX() + left || position.y < area.getY() + top)
        return { -1, -1 };

    const auto cellW = (area.getWidth() - left) / (float) ContinuumEngine::kDestinations;
    const auto cellH = (area.getHeight() - top) / (float) ContinuumEngine::kSources;
    const auto column = (int) ((position.x - area.getX() - left) / cellW);
    const auto row = (int) ((position.y - area.getY() - top) / cellH);

    if (column < 0 || column >= ContinuumEngine::kDestinations
        || row < 0 || row >= ContinuumEngine::kSources)
        return { -1, -1 };

    return { row, column };
}

void ContinuumMatrix::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff0d1112));

    g.setColour (juce::Colour (0xff2fb6a8).withAlpha (0.28f));
    g.drawRect (getLocalBounds(), 1);

    g.setColour (juce::Colour (0xffe7f5f1));
    g.setFont (juce::Font (juce::FontOptions (14.0f, juce::Font::bold)).withExtraKerningFactor (0.18f));
    g.drawText ("EIGEN MATRIX", 14, 8, 300, 18, juce::Justification::centredLeft);

    g.setColour (juce::Colour (0xff83a6a2));
    g.setFont (juce::Font (juce::FontOptions (8.0f)).withExtraKerningFactor (0.14f));
    g.drawText ("SOURCES / DESTINATIONS   ·   CLICK = FORMULA   ·   DRAG = AMOUNT",
                14, 27, 600, 12, juce::Justification::centredLeft);

    auto& engine = host_.engine();
    const auto area = getLocalBounds().toFloat().reduced (6.0f);
    const auto left = 104.0f;
    const auto top = 100.0f;
    const auto cellW = (area.getWidth() - left) / (float) ContinuumEngine::kDestinations;
    const auto cellH = (area.getHeight() - top) / (float) ContinuumEngine::kSources;

    for (int d = 0; d < ContinuumEngine::kDestinations; ++d)
    {
        const auto name = juce::String (engine.matrixDestinationName (d).c_str());
        g.setColour (juce::Colour (0xff8fb6b1).withAlpha (0.85f));
        g.setFont (juce::Font (juce::FontOptions (7.0f)));
        g.drawFittedText (name, (int) (area.getX() + left + (float) d * cellW) + 1,
                          (int) area.getY() + 40, (int) cellW - 2, 54,
                          juce::Justification::centred, 2);
    }

    for (int s = 0; s < ContinuumEngine::kSources; ++s)
    {
        const auto name = juce::String (engine.matrixSourceName (s).c_str());
        g.setColour (juce::Colour (0xff8fb6b1).withAlpha (0.9f));
        g.setFont (juce::Font (juce::FontOptions (8.0f)));
        g.drawText (name, (int) area.getX() + 2,
                    (int) (area.getY() + top + (float) s * cellH),
                    (int) left - 8, (int) cellH, juce::Justification::centredRight);
    }

    for (int s = 0; s < ContinuumEngine::kSources; ++s)
    {
        for (int d = 0; d < ContinuumEngine::kDestinations; ++d)
        {
            const auto bounds = cellBounds (s, d);
            const auto formula = engine.matrixCellFormula (s, d);
            const auto amount = engine.matrixCellAmount (s, d);

            g.setColour (juce::Colour (0xff141c1d));
            g.fillRect (bounds);

            if (formula != 0)
            {
                g.setColour (juce::Colour (0xff2fb6a8).withAlpha (0.12f + amount * 0.38f));
                g.fillRect (bounds);
                g.setColour (juce::Colour (0xff9fe7df).withAlpha (0.8f));
                g.fillRect (bounds.getX(), bounds.getBottom() - 3.0f,
                            bounds.getWidth() * amount, 2.0f);

                g.setColour (juce::Colour (0xffe9f7f3));
                g.setFont (juce::Font (juce::FontOptions (7.0f)));
                g.drawText (formulaAbbreviation (formula), bounds, juce::Justification::centred);
            }

            g.setColour (juce::Colour (0xff2c3a3b));
            g.drawRect (bounds, 0.5f);
        }
    }
}

void ContinuumMatrix::mouseDown (const juce::MouseEvent& event)
{
    const auto cell = cellAt (event.position);
    if (cell.x < 0)
        return;

    dragSource_ = cell.x;
    dragDestination_ = cell.y;
    dragStartY_ = event.position.y;
    auto& engine = host_.engine();
    const auto currentFormula = engine.matrixCellFormula (dragSource_, dragDestination_);
    const auto currentAmount = engine.matrixCellAmount (dragSource_, dragDestination_);

    if (event.mods.isRightButtonDown())
    {
        engine.setMatrixCell (dragSource_, dragDestination_, 0, 0.0f);
        dragFormula_ = 0;
        dragStartAmount_ = 0.0f;
    }
    else
    {
        dragFormula_ = (currentFormula + 1) % ContinuumEngine::kFormulaCount;
        dragStartAmount_ = currentAmount > 0.01f ? currentAmount : 0.65f;
        engine.setMatrixCell (dragSource_, dragDestination_, dragFormula_, dragStartAmount_);
    }

    repaint();
}

void ContinuumMatrix::mouseDrag (const juce::MouseEvent& event)
{
    if (dragSource_ < 0 || dragFormula_ == 0)
        return;

    const auto amount = juce::jlimit (0.0f, 1.0f,
                                      dragStartAmount_ - (event.position.y - dragStartY_) / 100.0f);
    host_.engine().setMatrixCell (dragSource_, dragDestination_, dragFormula_, amount);
    repaint();
}

void ContinuumMatrix::timerCallback()
{
    repaint();
}

// ---------------------------------------------------------------------------
ContinuumPanel::ContinuumPanel (EditorHost& host)
    : PanelBase (host, "CONTINUUM V", 2000, 1300, continuumAccent())
{
    setBranding ("CONTINUUM V", "EIGEN MATRIX  /  CONTINUOUS SURFACE");

    const auto accent = continuumAccent();

    // Master and expression strip.
    addModule ("MASTER / EXPRESSION", { 8, 8, 1984, 56 });
    const char* masterIds[] = { "con.master.volume", "con.master.glide",
                                "con.master.yDepth", "con.master.zDepth",
                                "con.master.bendRange", "con.master.reverbMix",
                                "con.master.delayMix", "con.master.noise" };
    const char* masterLabels[] = { "VOLUME", "GLIDE", "Y DEPTH", "Z DEPTH",
                                   "MPE BEND", "REVERB", "DELAY", "NOISE" };
    for (int i = 0; i < 8; ++i)
        addKnob (masterIds[i], masterLabels[i], { 16 + i * 245, 14, 240, 42 });

    surface_ = std::make_unique<ContinuumSurface> (host);
    surface_->setBounds (8, 72, 1984, 356);
    addAndMakeVisible (*surface_);

    matrix_ = std::make_unique<ContinuumMatrix> (host);
    matrix_->setBounds (8, 436, 720, 760);
    addAndMakeVisible (*matrix_);

    for (int i = 0; i < 5; ++i)
    {
        const auto x = 736 + i * 248;
        const auto prefix = "con.osc" + juce::String (i + 1) + ".";
        const auto group = "OSC " + juce::String (i + 1);
        addModule (group, { x, 436, 244, 258 });
        auto inner = juce::Rectangle<int> (x + 4, 436 + 26, 236, 228);
        const auto cellW = inner.getWidth() / 2;
        const auto cellH = inner.getHeight() / 3;

        addKnob (prefix + "ratio", "RATIO", { inner.getX(), inner.getY(), cellW, cellH });
        addKnob (prefix + "fine", "FINE", { inner.getX() + cellW, inner.getY(), cellW, cellH });
        addKnob (prefix + "phase", "PHASE", { inner.getX(), inner.getY() + cellH, cellW, cellH });
        addKnob (prefix + "shape", "SHAPE", { inner.getX() + cellW, inner.getY() + cellH, cellW, cellH });
        addKnob (prefix + "level", "LEVEL", { inner.getX(), inner.getY() + cellH * 2, cellW, cellH });
        addChoice (prefix + "wave", "WAVE", { inner.getX() + cellW, inner.getY() + cellH * 2, cellW, cellH });
    }

    for (int i = 0; i < 5; ++i)
    {
        const auto x = 736 + i * 248;
        const auto prefix = "con.flt" + juce::String (i + 1) + ".";
        addModule ("FILTER " + juce::String (i + 1), { x, 702, 244, 190 });
        auto inner = juce::Rectangle<int> (x + 4, 702 + 26, 236, 158);
        const auto cellW = inner.getWidth() / 2;
        const auto cellH = inner.getHeight() / 2;

        addChoice (prefix + "type", "TYPE", { inner.getX(), inner.getY(), cellW, cellH });
        addKnob (prefix + "cutoff", "CUTOFF", { inner.getX() + cellW, inner.getY(), cellW, cellH });
        addKnob (prefix + "res", "RES", { inner.getX(), inner.getY() + cellH, cellW, cellH });
        addKnob (prefix + "env", "ENV 2", { inner.getX() + cellW, inner.getY() + cellH, cellW, cellH });
    }

    for (int i = 0; i < 2; ++i)
    {
        const auto x = 736 + i * 310;
        const auto prefix = "con.bank" + juce::String (i + 1) + ".";
        addModule ("BIQUAD BANK " + juce::String (i + 1), { x, 900, 300, 170 });
        auto inner = juce::Rectangle<int> (x + 4, 926, 292, 140);
        const auto cellW = inner.getWidth() / 2;
        const auto cellH = inner.getHeight() / 3;
        addKnob (prefix + "freq", "FREQ", { inner.getX(), inner.getY(), cellW, cellH });
        addKnob (prefix + "spread", "SPREAD", { inner.getX() + cellW, inner.getY(), cellW, cellH });
        addKnob (prefix + "bandwidth", "BANDWIDTH", { inner.getX(), inner.getY() + cellH, cellW, cellH });
        addKnob (prefix + "mix", "MIX", { inner.getX() + cellW, inner.getY() + cellH, cellW, cellH });
        addKnob (prefix + "level", "LEVEL", { inner.getX(), inner.getY() + cellH * 2, cellW, cellH });
    }

    for (int i = 0; i < 2; ++i)
    {
        const auto x = 1366 + i * 310;
        const auto prefix = "con.delay" + juce::String (i + 1) + ".";
        addModule ("DELAY " + juce::String (i + 1), { x, 900, 300, 170 });
        auto inner = juce::Rectangle<int> (x + 4, 926, 292, 140);
        const auto cellW = inner.getWidth() / 2;
        const auto cellH = inner.getHeight() / 3;
        addKnob (prefix + "time", "TIME", { inner.getX(), inner.getY(), cellW, cellH });
        addKnob (prefix + "feedback", "FEEDBACK", { inner.getX() + cellW, inner.getY(), cellW, cellH });
        addKnob (prefix + "mix", "MIX", { inner.getX(), inner.getY() + cellH, cellW, cellH });
    }

    addModule ("SHAPE GENERATORS", { 736, 1078, 1220, 160 });
    for (int i = 0; i < 5; ++i)
    {
        const auto x = 744 + i * 242;
        const auto prefix = "con.sg" + juce::String (i + 1) + ".";
        addCaption ("SG " + juce::String (i + 1), { x, 1104, 70, 14 }, juce::Justification::centredLeft, 9.0f, accent, true);
        addKnob (prefix + "rate", "RATE", { x, 1120, 72, 92 });
        addChoice (prefix + "shape", "SHAPE", { x + 76, 1140, 86, 42 });
        addKnob (prefix + "level", "LEVEL", { x + 168, 1120, 66, 92 });
    }
}

} // namespace triad::ui
