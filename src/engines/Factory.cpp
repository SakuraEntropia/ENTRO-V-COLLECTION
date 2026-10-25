#include "engines/Engines.h"
#include "engines/SampledEngine.h"

namespace triad
{

std::unique_ptr<SynthEngine> makeEngine (InstrumentKind kind)
{
    switch (kind)
    {
        case InstrumentKind::Matriarch: return withSamplePrograms(makeMatriarchEngine());
        case InstrumentKind::PolyBrute: return withSamplePrograms(makePolyBruteEngine());
        case InstrumentKind::MoogOne:   return withSamplePrograms(makeOneEngine());
        case InstrumentKind::Continuum: return withSamplePrograms(makeContinuumEngine());
    }

    return withSamplePrograms(makeMatriarchEngine());
}

} // namespace triad
