#include "engines/Engines.h"
#include "plugin/Presets.h"

#include <cmath>
#include <cstdio>
#include <set>

namespace
{
int failures = 0;

void check (bool condition, const char* message, const juce::String& detail = {})
{
    if (! condition)
    {
        std::printf ("FAIL: %s %s\n", message, detail.toRawUTF8());
        ++failures;
    }
}
} // namespace

int main()
{
    for (const auto kind : { triad::InstrumentKind::Matriarch,
                             triad::InstrumentKind::PolyBrute,
                             triad::InstrumentKind::MoogOne })
    {
        const auto engine = triad::makeEngine (kind);
        const auto& definition = engine->definition();
        const auto presets = triad::factoryPresetsFor (definition.id);

        check (! presets.empty(), "no factory presets", definition.id);

        std::set<juce::String> names;

        for (const auto& preset : presets)
        {
            check (names.insert (preset.name).second, "duplicate preset name", preset.name);
            check (! preset.values.empty(), "empty preset", preset.name);

            for (const auto& entry : preset.values)
            {
                const auto* spec = definition.find (entry.first.toStdString());

                if (spec == nullptr)
                {
                    check (false, "unknown parameter in preset", preset.name + " / " + entry.first);
                    continue;
                }

                if (! std::isfinite (entry.second))
                {
                    check (false, "non-finite preset value", preset.name + " / " + entry.first);
                    continue;
                }

                if (entry.second < spec->min || entry.second > spec->max)
                {
                    check (false, "preset value outside range",
                           preset.name + " / " + entry.first + " = " + juce::String (entry.second));
                    continue;
                }

                if (spec->curve == triad::ParamCurve::Discrete)
                {
                    const auto index = (int) std::lround (entry.second);
                    if (index < 0 || index >= (int) spec->choices.size())
                        check (false, "preset choice index out of range",
                               preset.name + " / " + entry.first);
                }
            }
        }

        std::printf ("%s: %d presets, %zu parameters\n", definition.id.c_str(),
                     (int) presets.size(), definition.params.size());
    }

    if (failures == 0)
    {
        std::printf ("All factory preset checks passed.\n");
        return 0;
    }

    return 1;
}
