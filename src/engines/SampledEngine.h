#pragma once
#include "core/Engine.h"
namespace triad {
/** Adds explicit recorded-instrument programs; original synthesis remains program zero. */
std::unique_ptr<SynthEngine> withSamplePrograms(std::unique_ptr<SynthEngine>);
}
