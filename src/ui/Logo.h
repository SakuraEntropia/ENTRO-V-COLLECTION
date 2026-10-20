#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace triad::ui
{

enum class LogoKind
{
    Matriarch,
    PolyBrute,
    One
};

LogoKind logoKindForInstrument (const juce::String& instrumentId);

/** Draws the ENTRO V instrument mark in a square area.  Coordinates are the
    same 128-unit grid used by the SVG source in assets/logos. */
void drawInstrumentMark (juce::Graphics& g, juce::Rectangle<float> area, LogoKind kind,
                         juce::Colour accent,
                         juce::Colour foreground = juce::Colour (0xffede7dc));

} // namespace triad::ui
