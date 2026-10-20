#pragma once

#include "core/Engine.h"
#include "engines/Matriarch.h"
#include "engines/One.h"
#include "engines/PolyBrute.h"

#include <memory>

namespace triad
{

std::unique_ptr<SynthEngine> makeEngine (InstrumentKind kind);

} // namespace triad
