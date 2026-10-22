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
        case 12: return "USER";
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
    auto pitch = 36.0f + normalised * 48.0f;

    if (auto* quantize = host_.apvts().getRawParameterValue ("con.surface.quantize"))
    {
        const auto amount = juce::jlimit (0.0f, 1.0f, quantize->load());
        pitch = pitch + (std::round (pitch) - pitch) * amount;
    }

    return pitch;
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

    // Four octaves = 48 continuous semitone cells.  Red cells are the
    // white-key pitch classes, black cells the black-key pitch classes.
    // One cell = exactly one semitone, so the visual blocks and the linear
    // X-pitch mapping share the same boundaries.
    const auto semitoneWidth = area.getWidth() / 48.0f;
    static constexpr bool blackKey[12] = { false, true, false, true, false, false,
                                           true, false, true, false, true, false };

    for (int semitone = 0; semitone < 48; ++semitone)
    {
        const auto x = area.getX() + (float) semitone * semitoneWidth;
        const auto isBlack = blackKey[semitone % 12];

        g.setColour (isBlack ? juce::Colour (0xff08090a) : juce::Colour (0xffd8232a));
        g.fillRect (x, area.getY(), semitoneWidth + 0.5f, area.getHeight());

        g.setColour (isBlack ? juce::Colour (0xff4b4b4d).withAlpha (0.72f)
                             : juce::Colour (0xff6f1013).withAlpha (0.85f));
        g.drawVerticalLine ((int) x, area.getY(), area.getBottom());

        if (isBlack)
        {
            g.setColour (juce::Colour (0xff3c3c40).withAlpha (0.8f));
            g.drawVerticalLine ((int) (x + semitoneWidth - 1.0f),
                                area.getY(), area.getBottom());
        }
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
        g.drawText ("TOUCH THE SURFACE  /  DRAG FOR PITCH AND TIMBRE  /  WHEEL FOR PRESSURE",
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
    const juce::String readout = "PITCH " + juce::String (60.0f + exprPitch_, 2)
                                 + "  Y " + juce::String (timbre_, 2)
                                 + "  Z " + juce::String (pressure_, 2);
    g.drawText (readout, (int) point.x + 18, (int) point.y - 8, 280, 16,
                juce::Justification::centredLeft);
}

ContinuumSurface::State ContinuumSurface::state() const
{
    return { active_, 60.0f + exprPitch_, timbre_, pressure_, timbre_ };
}

void ContinuumSurface::mouseDown (const juce::MouseEvent& event)
{
    if (active_)
        endNote();

    const auto pressure = event.isPressureValid()
                              ? juce::jlimit (0.02f, 1.0f, event.pressure)
                              : pressure_;
    beginNote (event.position, pressure);
}

void ContinuumSurface::beginNote (juce::Point<float> position, float pressure)
{
    const auto pitch = pitchForX (position.x);
    // One fixed voice + a continuous ±24-semitone expression offset.  Changing
    // MIDI notes at every semitone boundary retriggered the envelope and made
    // the surface sound stepped; the hardware surface is continuous.
    note_ = 60;
    exprPitch_ = pitch - 60.0f;
    timbre_ = juce::jlimit (0.02f, 0.98f, 1.0f - (position.y - playingArea().getY()) / juce::jmax (1.0f, playingArea().getHeight()));
    pressure_ = pressure;
    active_ = true;
    lastPoint_ = pointFor (note_, exprPitch_, timbre_);
    host_.continuumNoteOn (note_, exprPitch_, timbre_, pressure_);
    repaint();
}

void ContinuumSurface::updateFromPosition (juce::Point<float> position, bool allowNoteChange,
                                           float pressure)
{
    juce::ignoreUnused (allowNoteChange);

    const auto pitch = pitchForX (position.x);
    exprPitch_ = pitch - 60.0f;
    timbre_ = juce::jlimit (0.02f, 0.98f, 1.0f - (position.y - playingArea().getY()) / juce::jmax (1.0f, playingArea().getHeight()));
    pressure_ = juce::jlimit (0.02f, 1.0f, pressure);
    lastPoint_ = pointFor (note_, exprPitch_, timbre_);
    host_.continuumSetExpression (note_, exprPitch_, timbre_, pressure_);
    repaint();
}

void ContinuumSurface::mouseDrag (const juce::MouseEvent& event)
{
    if (! active_)
        return;

    if (event.isPressureValid())
        updateFromPosition (event.position, true, event.pressure);
    else
        updateFromPosition (event.position, true, pressure_);
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

    formulaEditor_.setFont (juce::Font (juce::FontOptions (14.0f)));
    formulaEditor_.setColour (juce::TextEditor::backgroundColourId, juce::Colour (0xfff0f2eb));
    formulaEditor_.setColour (juce::TextEditor::textColourId, juce::Colour (0xff30355d));
    formulaEditor_.setColour (juce::TextEditor::outlineColourId, juce::Colour (0xff2fb6a8).withAlpha (0.45f));
    formulaEditor_.setText ("sin(pi*x)*y", false);
    formulaEditor_.setTooltip ("User formula: s, x, y, z; sin/cos/tan/abs/sqrt/exp/log; + - * / ^");
    addAndMakeVisible (formulaEditor_);

    applyFormula_.setButtonText ("APPLY");
    applyFormula_.setTooltip ("Compile the user formula for cells set to user formula");
    applyFormula_.onClick = [this]
    {
        host_.engine().setUserFormula (formulaEditor_.getText().toStdString());
    };
    addAndMakeVisible (applyFormula_);

    startTimerHz (10);
}

juce::Rectangle<float> ContinuumMatrix::matrixArea() const
{
    return getLocalBounds().toFloat().reduced (6.0f).withTrimmedBottom (54.0f);
}

void ContinuumMatrix::resized()
{
    auto footer = getLocalBounds().removeFromBottom (46).reduced (8, 6);
    applyFormula_.setBounds (footer.removeFromRight (62));
    formulaEditor_.setBounds (footer.reduced (0, 0));
}

juce::Rectangle<float> ContinuumMatrix::cellBounds (int source, int destination) const
{
    const auto area = matrixArea();
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
    const auto area = matrixArea();
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
    g.fillAll (juce::Colour (0xffd9dfd6));

    g.setColour (juce::Colour (0xff2fb6a8).withAlpha (0.28f));
    g.drawRect (getLocalBounds(), 1);

    g.setColour (juce::Colour (0xff303d37));
    g.setFont (juce::Font (juce::FontOptions (14.0f, juce::Font::bold)).withExtraKerningFactor (0.18f));
    g.drawText ("EIGEN MATRIX", 14, 8, 300, 18, juce::Justification::centredLeft);

    g.setColour (juce::Colour (0xff56685e));
    g.setFont (juce::Font (juce::FontOptions (12.0f)).withExtraKerningFactor (0.14f));
    g.drawText ("SOURCES / DESTINATIONS   /   CLICK = FORMULA   /   DRAG = AMOUNT",
                14, 27, 600, 12, juce::Justification::centredLeft);

    auto& engine = host_.engine();
    const auto area = matrixArea();
    const auto left = 104.0f;
    const auto top = 100.0f;
    const auto cellW = (area.getWidth() - left) / (float) ContinuumEngine::kDestinations;
    const auto cellH = (area.getHeight() - top) / (float) ContinuumEngine::kSources;

    for (int d = 0; d < ContinuumEngine::kDestinations; ++d)
    {
        const auto name = juce::String (engine.matrixDestinationName (d).c_str());
        g.setColour (juce::Colour (0xff35483d).withAlpha (0.85f));
        g.setFont (juce::Font (juce::FontOptions (12.0f)));
        g.drawFittedText (name, (int) (area.getX() + left + (float) d * cellW) + 1,
                          (int) area.getY() + 40, (int) cellW - 2, 54,
                          juce::Justification::centred, 2);
    }

    for (int s = 0; s < ContinuumEngine::kSources; ++s)
    {
        const auto name = juce::String (engine.matrixSourceName (s).c_str());
        g.setColour (juce::Colour (0xff35483d).withAlpha (0.9f));
        g.setFont (juce::Font (juce::FontOptions (12.0f)));
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

            g.setColour (juce::Colour (0xffe9ede5));
            g.fillRect (bounds);

            if (formula != 0)
            {
                g.setColour (juce::Colour (0xff2fb6a8).withAlpha (0.12f + amount * 0.38f));
                g.fillRect (bounds);
                g.setColour (juce::Colour (0xffa7514c).withAlpha (0.8f));
                g.fillRect (bounds.getX(), bounds.getBottom() - 3.0f,
                            bounds.getWidth() * amount, 2.0f);

                g.setColour (juce::Colour (0xff383962));
                g.setFont (juce::Font (juce::FontOptions (12.0f)));
                g.drawText (formulaAbbreviation (formula) + "  " + juce::String (amount, 2), bounds, juce::Justification::centred);
            }

            g.setColour (juce::Colour (0xffa7b5aa));
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
// ---------------------------------------------------------------------------
class ContinuumMonitor final : public juce::Component, private juce::Timer
{
public:
    explicit ContinuumMonitor (ContinuumSurface& surface) : surface_ (surface)
    {
        setInterceptsMouseClicks (false, false);
        startTimerHz (30);
    }

    ~ContinuumMonitor() override { stopTimer(); }

    void paint (juce::Graphics& g) override
    {
        const auto state = surface_.state();
        auto bounds = getLocalBounds().toFloat();

        g.setColour (juce::Colour (0xff071012));
        g.fillRoundedRectangle (bounds.reduced (0.5f), 3.0f);
        g.setColour (juce::Colour (0xff2fb6a8).withAlpha (0.55f));
        g.drawRoundedRectangle (bounds.reduced (0.5f), 3.0f, 1.0f);

        g.setColour (juce::Colour (0xff8ff3ea));
        g.setFont (juce::Font (juce::FontOptions (10.0f, juce::Font::bold)));
        g.drawText ("CONTINUUM 3D DISPLAY", 14, 9, 230, 14, juce::Justification::centredLeft);

        auto stage = bounds.reduced (12.0f, 8.0f).withTrimmedTop (14.0f);
        const auto infoWidth = 430.0f;
        auto field = stage.withTrimmedRight (infoWidth);
        g.setColour (juce::Colour (0xff0a1b1e));
        g.fillRoundedRectangle (field, 2.0f);

        for (int i = 1; i < 8; ++i)
        {
            g.setColour (juce::Colour (0xff5fd6d0).withAlpha (0.10f));
            g.drawVerticalLine ((int) (field.getX() + field.getWidth() * (float) i / 8.0f),
                                field.getY(), field.getBottom());
        }

        const auto pitchNorm = juce::jlimit (0.0f, 1.0f, (state.pitch - 36.0f) / 48.0f);
        const auto timbreNorm = juce::jlimit (0.0f, 1.0f, state.timbre);
        const auto point = juce::Point<float> (field.getX() + pitchNorm * field.getWidth(),
                                                field.getY() + (1.0f - timbreNorm) * field.getHeight());
        const auto radius = 4.0f + state.pressure * 15.0f;

        g.setColour (juce::Colour (0xff5fd6d0).withAlpha (0.18f));
        g.fillEllipse (point.x - radius, point.y - radius, radius * 2.0f, radius * 2.0f);
        g.setColour (state.active ? juce::Colour (0xffdffbf6) : juce::Colour (0xff50706f));
        g.drawEllipse (point.x - radius, point.y - radius, radius * 2.0f, radius * 2.0f, 1.2f);

        // Character: a short line inside the circle, direction from timbre.
        const auto angle = (state.character * 2.0f - 1.0f) * juce::MathConstants<float>::halfPi;
        const auto lineLength = radius * 0.75f;
        g.setColour (juce::Colour (0xff2fb6a8));
        g.drawLine (point.x, point.y,
                    point.x + std::sin (angle) * lineLength,
                    point.y - std::cos (angle) * lineLength, 1.5f);

        g.setColour (juce::Colour (0xff8ff3ea));
        g.setFont (juce::Font (juce::FontOptions (8.0f)));
        g.drawText ("X  PITCH   /   Y  TIMBRE   /   Z  PRESSURE",
                    field.getX() + 5, field.getBottom() - 13, 420, 10,
                    juce::Justification::centredLeft);

        auto info = stage.removeFromRight (infoWidth).reduced (12.0f, 0.0f);
        g.setColour (juce::Colour (0xffbdfaf3));
        g.setFont (juce::Font (juce::FontOptions (9.0f)));
        g.drawText ("PITCH  " + juce::String (state.pitch, 2), info.removeFromTop (14),
                    juce::Justification::centredLeft);
        g.drawText ("PRESSURE RADIUS  " + juce::String (state.pressure, 2),
                    info.removeFromTop (14), juce::Justification::centredLeft);
        g.drawText ("CHARACTER  " + juce::String (state.character, 2),
                    info.removeFromTop (14), juce::Justification::centredLeft);
        g.setColour (juce::Colour (0xff5fd6d0).withAlpha (0.75f));
        g.setFont (juce::Font (juce::FontOptions (7.0f)));
        g.drawText ("KEYS  A W S E D F T G Y H U J K",
                    info.removeFromTop (12), juce::Justification::centredLeft);
    }

private:
    void timerCallback() override { repaint(); }
    ContinuumSurface& surface_;
};

std::unique_ptr<juce::Component> createContinuumMonitor (ContinuumSurface& surface)
{
    return std::make_unique<ContinuumMonitor> (surface);
}

class RecirculatorModeStrip final : public juce::Component, private juce::Timer
{
public:
    explicit RecirculatorModeStrip (EditorHost& host) : host_ (host)
    {
        setInterceptsMouseClicks (true, false);
        startTimerHz (12);
    }

    ~RecirculatorModeStrip() override { stopTimer(); }

    void paint (juce::Graphics& g) override
    {
        static const char* names[6] = { "SHORT", "LONG", "MOD", "ANALOG", "SWEPT", "DIGITAL" };
        static const char* sub[6] = { "REVERB", "REVERB", "DELAY", "ECHO", "ECHO", "ECHO LPF" };
        const auto selected = currentMode();
        auto area = getLocalBounds().toFloat().reduced (2.0f, 2.0f);
        const auto width = area.getWidth() / 6.0f;

        for (int i = 0; i < 6; ++i)
        {
            const auto x = area.getX() + width * (float) i;
            const auto gap = 3.0f;
            juce::Path trapezoid;
            // Inverted trapezoid: wide top, narrow bottom.
            trapezoid.startNewSubPath (x + gap, area.getY());
            trapezoid.lineTo (x + width - gap, area.getY());
            trapezoid.lineTo (x + width - gap - 10.0f, area.getBottom());
            trapezoid.lineTo (x + gap + 10.0f, area.getBottom());
            trapezoid.closeSubPath();

            g.setColour (i == selected ? juce::Colour (0xff2fb6a8).withAlpha (0.78f)
                                       : juce::Colour (0xff0b1b1d));
            g.fillPath (trapezoid);
            g.setColour (i == selected ? juce::Colour (0xffdffbf6)
                                       : juce::Colour (0xff5fd6d0).withAlpha (0.45f));
            g.strokePath (trapezoid, juce::PathStrokeType (1.0f));

            g.setColour (i == selected ? juce::Colour (0xff071012) : juce::Colour (0xff9fe7df));
            g.setFont (juce::Font (juce::FontOptions (7.5f, juce::Font::bold)));
            g.drawText (names[i], (int) (x + 4), (int) area.getY() + 4,
                        (int) width - 8, 12, juce::Justification::centred);
            g.setFont (juce::Font (juce::FontOptions (6.5f)));
            g.drawText (sub[i], (int) (x + 4), (int) area.getY() + 19,
                        (int) width - 8, 12, juce::Justification::centred);
        }
    }

    void mouseDown (const juce::MouseEvent& event) override
    {
        const auto width = getWidth() / 6.0f;
        const auto index = juce::jlimit (0, 5, (int) (event.position.x / width));

        if (auto* parameter = host_.apvts().getParameter ("con.fx.mode"))
            parameter->setValueNotifyingHost (parameter->convertTo0to1 ((float) index));
    }

private:
    int currentMode() const
    {
        if (auto* value = host_.apvts().getRawParameterValue ("con.fx.mode"))
            return juce::jlimit (0, 5, (int) std::lround (value->load()));

        return 0;
    }

    void timerCallback() override { repaint(); }
    EditorHost& host_;
};

class DelayLpfFaders final : public juce::Component, private juce::Timer
{
public:
    explicit DelayLpfFaders (EditorHost& host) : host_ (host)
    {
        setName ("Delay-LPF R1-R6");
        startTimerHz (20);
    }
    ~DelayLpfFaders() override
    {
        stopTimer();
        if (dragParameter_ != nullptr) dragParameter_->endChangeGesture();
    }

    void paint (juce::Graphics& g) override
    {
        static const char* labels[] = { "LPF", "OFFSET", "PINGPONG", "FEEDBACK", "FULL", "TIME" };
        const auto width = (float) getWidth() / 6.0f;
        for (int i = 0; i < 6; ++i)
        {
            const auto c = column (i);
            juce::Path trapezoid;
            trapezoid.startNewSubPath (c.getX(), c.getY());
            trapezoid.lineTo (c.getRight(), c.getY());
            trapezoid.lineTo (c.getRight() - c.getWidth() * 0.28f, c.getBottom());
            trapezoid.lineTo (c.getX() + c.getWidth() * 0.28f, c.getBottom());
            trapezoid.closeSubPath();
            g.setColour (juce::Colour (0xffeceee7)); g.fillPath (trapezoid);
            g.setColour (juce::Colour (0xff89988c)); g.strokePath (trapezoid, juce::PathStrokeType (1.0f));
            auto* parameter = host_.apvts().getParameter (ids[i]);
            const auto value = parameter != nullptr ? parameter->getValue() : 0.0f;
            const auto y = c.getBottom() - value * c.getHeight();
            g.setColour (juce::Colour (0xff484477).withAlpha (0.48f));
            g.fillRect (c.getCentreX() - 2.0f, c.getY() + 4.0f, 4.0f, c.getHeight() - 8.0f);
            g.setColour (juce::Colour (0xffa24744));
            g.drawLine (c.getX(), y, c.getRight(), y, 1.5f);
            g.setFont (juce::Font (juce::FontOptions (13.0f)));
            g.setColour (juce::Colour (0xff3b3a6b));
            g.drawText (parameter != nullptr ? (parameter->getAllValueStrings().isEmpty()
                ? juce::String (parameter->convertFrom0to1 (value), 3) : parameter->getCurrentValueAsText()) : "", (int) (i * width), 0,
                        (int) width, 20, juce::Justification::centred);
            g.setColour (juce::Colour (0xff34443b));
            g.setFont (juce::Font (juce::FontOptions (11.5f)));
            g.drawText (labels[i], (int) (i * width), getHeight() - 22, (int) width, 18, juce::Justification::centred);
        }
    }
    void mouseDown (const juce::MouseEvent& event) override
    {
        const auto i = juce::jlimit (0, 5, (int) (event.position.x * 6.0f / getWidth()));
        dragParameter_ = host_.apvts().getParameter (ids[i]);
        dragIndex_ = i;
        if (dragParameter_ != nullptr) dragParameter_->beginChangeGesture();
        update (event.position.y);
    }
    void mouseDrag (const juce::MouseEvent& event) override { update (event.position.y); }
    void mouseUp (const juce::MouseEvent&) override
    {
        if (dragParameter_ != nullptr) dragParameter_->endChangeGesture();
        dragParameter_ = nullptr;
    }
private:
    static constexpr const char* ids[] = { "con.fx.delay.lpf", "con.fx.delay.offset", "con.fx.delay.pingpong",
                                           "con.fx.delay.fb", "con.fx.delay.full", "con.fx.delay.time" };
    juce::Rectangle<float> column (int i) const
    {
        const auto width = (float) getWidth() / 6.0f;
        return { i * width + 28.0f, 25.0f, width - 56.0f, (float) getHeight() - 54.0f };
    }
    void update (float y)
    {
        const auto c = column (dragIndex_);
        if (dragParameter_ != nullptr)
            dragParameter_->setValueNotifyingHost (juce::jlimit (0.0f, 1.0f, (c.getBottom() - y) / c.getHeight()));
    }
    void timerCallback() override { repaint(); }
    EditorHost& host_;
    juce::RangedAudioParameter* dragParameter_ = nullptr;
    int dragIndex_ = 0;
};

ContinuumPanel::ContinuumPanel (EditorHost& host)
    : PanelBase (host, "CONTINUUM V", 2800, 1340, continuumAccent())
{
    setBranding ("CONTINUUM V", "EIGEN MATRIX  /  CONTINUOUS SURFACE");
    addModule ("CONTROL PANEL 1 / MASTER & EXPRESSION", { 8, 8, 2784, 68 });
    const char* masterIds[] = { "con.master.volume", "con.master.glide", "con.master.yDepth", "con.master.zDepth",
        "con.master.bendRange", "con.master.reverbMix", "con.master.delayMix", "con.master.noise",
        "con.cvc.1", "con.cvc.2", "con.cvc.3", "con.cvc.4", "con.surface.quantize" };
    const char* masterNames[] = { "VOLUME", "GLIDE", "Y DEPTH", "Z DEPTH", "MPE BEND", "REVERB", "DELAY", "NOISE",
        "CVC 1 / CC20", "CVC 2 / CC21", "CVC 3 / CC22", "CVC 4 / CC23", "PITCH SNAP" };
    for (int i = 0; i < 13; ++i) addNumber (masterIds[i], masterNames[i], { 22 + i * 213, 32, 190, 40 });

    addModule ("CONTROL PANEL 2 / RECIRCULATOR", { 8, 84, 684, 208 });
    fxModes_ = std::make_unique<RecirculatorModeStrip> (host);
    fxModes_->setBounds (20, 111, 660, 40); addAndMakeVisible (*fxModes_);
    const char* fxIds[] = { "con.fx.reverb.damping", "con.fx.chorus.rate", "con.fx.chorus.depth", "con.fx.chorus.mix",
        "con.fx.phaser.rate", "con.fx.phaser.depth", "con.fx.phaser.feedback", "con.fx.phaser.mix" };
    const char* fxNames[] = { "REVERB DAMP", "CHORUS RATE", "CHORUS DEPTH", "CHORUS MIX",
        "PHASER RATE", "PHASER DEPTH", "PHASER FB", "PHASER MIX" };
    for (int i = 0; i < 8; ++i) addNumber (fxIds[i], fxNames[i], { 24 + i % 4 * 166, 170 + i / 4 * 62, 146, 42 });

    addModule ("DELAY / LPF - R1-R6", { 700, 84, 736, 208 });
    delayFaders_ = std::make_unique<DelayLpfFaders> (host);
    delayFaders_->setBounds (712, 110, 712, 174); addAndMakeVisible (*delayFaders_);

    addModule ("MODULATED ECHO / CONVOLUTION", { 1444, 84, 536, 208 });
    addNumber ("con.fx.delay.mod", "MODULATION", { 1462, 128, 234, 48 });
    addNumber ("con.fx.delay.rate", "RATE", { 1714, 128, 248, 48 });
    addNumber ("con.fx.conv1.mix", "PRE CONV / MIX", { 1462, 212, 234, 48 });
    addNumber ("con.fx.conv2.mix", "POST CONV / MIX", { 1714, 212, 248, 48 });

    addModule ("NOISE / SPACE", { 1988, 84, 804, 208 });
    const char* spaceIds[] = { "con.noise.level", "con.noise.color", "con.master.reverbSize", "con.master.delayTime", "con.master.delayFeedback" };
    const char* spaceNames[] = { "NOISE LEVEL", "NOISE COLOR", "REVERB SIZE", "DELAY TIME", "DELAY FEEDBACK" };
    for (int i = 0; i < 5; ++i) addNumber (spaceIds[i], spaceNames[i], { 2008 + i % 3 * 258, 128 + i / 3 * 84, 232, 48 });

    matrix_ = std::make_unique<ContinuumMatrix> (host);
    matrix_->setBounds (8, 302, 2784, 510); addAndMakeVisible (*matrix_);

    // Numeric model blocks below the matrix, as in the editor's parameter tables.
    for (int i = 0; i < 5; ++i)
    {
        const auto x = 8 + i * 444;
        const auto osc = "con.osc" + juce::String (i + 1) + ".";
        const auto flt = "con.flt" + juce::String (i + 1) + ".";
        const auto sg = "con.sg" + juce::String (i + 1) + ".";
        addModule ("OSC " + juce::String (i + 1) + " / FILTER " + juce::String (i + 1), { x, 822, 436, 366 });
        const char* ids[] = { "ratio", "fine", "phase", "shape", "level" };
        const char* names[] = { "RATIO", "FINE", "PHASE", "SHAPE", "LEVEL" };
        for (int n = 0; n < 5; ++n)
            addNumber (osc + ids[n], names[n], { x + 12 + n % 3 * 140, 853 + n / 3 * 54, 126, 42 });
        auto& wave = addChoice (osc + "wave", "WAVE", { x + 292, 907, 126, 42 });
        wave.setColour (juce::ComboBox::textColourId, juce::Colour (0xff34355f));
        auto& type = addChoice (flt + "type", "FILTER TYPE", { x + 12, 974, 126, 42 });
        type.setColour (juce::ComboBox::textColourId, juce::Colour (0xff34355f));
        addNumber (flt + "cutoff", "CUTOFF", { x + 152, 974, 126, 42 });
        addNumber (flt + "res", "RESONANCE", { x + 292, 974, 126, 42 });
        addNumber (flt + "env", "ENV 2", { x + 12, 1028, 126, 42 });
        addCaption ("SHAPE GENERATOR " + juce::String (i + 1), { x + 14, 1090, 404, 16 },
                    juce::Justification::centredLeft, 11.0f, juce::Colour (0xff455448));
        addNumber (sg + "rate", "RATE", { x + 12, 1124, 126, 42 });
        auto& shape = addChoice (sg + "shape", "SHAPE", { x + 152, 1124, 126, 42 });
        shape.setColour (juce::ComboBox::textColourId, juce::Colour (0xff34355f));
        addNumber (sg + "level", "LEVEL", { x + 292, 1124, 126, 42 });
    }
    for (int i = 0; i < 2; ++i)
    {
        const auto x = 2228 + i * 284;
        const auto stem = "con.bank" + juce::String (i + 1) + ".";
        addModule ("BIQUAD BANK " + juce::String (i + 1), { x, 822, 276, 208 });
        const char* ids[] = { "freq", "spread", "bandwidth", "mix", "level" };
        const char* names[] = { "FREQUENCY", "SPREAD", "BANDWIDTH", "MIX", "LEVEL" };
        for (int n = 0; n < 5; ++n) addNumber (stem + ids[n], names[n], { x + 12 + n % 2 * 132, 853 + n / 2 * 54, 120, 42 });
        const auto d = "con.delay" + juce::String (i + 1) + ".";
        addModule (i == 0 ? "SUMMED DELAY" : "MICRO DELAY", { x, 1038, 276, 150 });
        addNumber (d + "time", "TIME", { x + 12, 1070, 120, 42 });
        addNumber (d + "feedback", "FEEDBACK", { x + 144, 1070, 120, 42 });
        addNumber (d + "mix", "MIX", { x + 12, 1124, 120, 42 });
    }
    for (int env = 0; env < 2; ++env)
    {
        const auto x = 8 + env * 1396;
        addModule ("ENVELOPE " + juce::String (env + 1), { x, 1198, 1388, 76 });
        const char* ids[] = { "attack", "decay", "sustain", "release" };
        const char* names[] = { "ATTACK", "DECAY", "SUSTAIN", "RELEASE" };
        for (int n = 0; n < 4; ++n)
            addNumber ("con.env" + juce::String (env + 1) + "." + ids[n], names[n],
                       { x + 16 + n * 342, 1226, 320, 42 });
    }
}
ContinuumPanel::~ContinuumPanel() = default;
} // namespace triad::ui
