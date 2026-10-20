#include "engines/Engines.h"

namespace triad
{

std::unique_ptr<SynthEngine> makeEngine (InstrumentKind kind)
{
    switch (kind)
    {
        case InstrumentKind::Matriarch: return makeMatriarchEngine();
        case InstrumentKind::PolyBrute: return makePolyBruteEngine();
        case InstrumentKind::MoogOne:   return makeOneEngine();
        case InstrumentKind::Continuum: return makeContinuumEngine();
    }

    return makeMatriarchEngine();
}

} // namespace triad
