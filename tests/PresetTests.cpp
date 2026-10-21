#include "engines/Engines.h"
#include "plugin/Presets.h"

#include <cmath>
#include <cstdio>
#include <set>
#include <vector>

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
                             triad::InstrumentKind::MoogOne,
                             triad::InstrumentKind::Continuum })
    {
        const auto engine = triad::makeEngine (kind);
        const auto& definition = engine->definition();
        const auto presets = triad::factoryPresetsFor (definition.id);

        check (! presets.empty(), "no factory presets", definition.id);

        std::set<juce::String> names;
        int lastCategoryIndex = -1;
        int bassCount = 0;
        bool hasReese = false;
        bool hasKevinEharp = false;
        bool hasKevinKoto = false;

        const juce::StringArray categoryOrder { "Bass", "Lead", "Brass", "Pad", "Strings",
                                                "Winds", "Vocal", "Keyboard", "Classic", "Other",
                                                "Percussion", "Tuned Percussion", "Processor",
                                                "Drone", "Midi", "CVC", "User", "Surface", "Texture", "Pluck",
                                                "Keys", "Organ", "Arp", "Sequence", "Split",
                                                "Utility", "FX" };

        for (const auto& preset : presets)
        {
            check (names.insert (preset.name).second, "duplicate preset name", preset.name);
            check (! preset.values.empty(), "empty preset", preset.name);
            check (preset.category.isNotEmpty(), "empty preset category", preset.name);

            const auto categoryIndex = categoryOrder.indexOf (preset.category);
            check (categoryIndex >= 0, "unknown preset category", preset.category);
            check (categoryIndex >= lastCategoryIndex, "preset categories not grouped", preset.name);
            lastCategoryIndex = juce::jmax (lastCategoryIndex, categoryIndex);

            if (preset.category == "Bass")
                ++bassCount;

            if (preset.name.containsIgnoreCase ("Reese"))
                hasReese = true;

            if (preset.name == "Kevin Penkin Style Eharp")
                hasKevinEharp = true;

            if (preset.name == "Kevin Penkin Style E Koto")
                hasKevinKoto = true;

            for (const auto& cell : preset.matrix)
            {
                check (cell.source >= 0 && cell.source < engine->matrixRows(),
                       "preset matrix source out of range", preset.name);
                check (cell.destination >= 0 && cell.destination < engine->matrixColumns(),
                       "preset matrix destination out of range", preset.name);
                check (cell.formula >= 0 && cell.formula < engine->matrixFormulaCount(),
                       "preset matrix formula out of range", preset.name);
                check (cell.amount >= 0.0f && cell.amount <= 1.0f,
                       "preset matrix amount out of range", preset.name);
            }

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

        check (presets.size() >= 12, "too few factory presets", definition.id);
        check (bassCount >= 2, "bass category too small", definition.id);
        check (hasReese, "missing Reese bass preset", definition.id);

        if (kind == triad::InstrumentKind::Continuum)
        {
            check (hasKevinEharp, "missing Kevin Penkin Style Eharp preset", definition.id);
            check (hasKevinKoto, "missing Kevin Penkin Style E Koto preset", definition.id);
        }

        // Render each preset to make sure it is audible, finite and bounded.
        auto renderEngine = triad::makeEngine (kind);
        renderEngine->prepare (48000.0, 256);

        int renderCounter = 0;
        for (const auto& preset : presets)
        {
            // The restored Haken library has 600+ entries; render a
            // representative subset so the contract test stays fast while
            // every preset's parameter data is still validated above.
            if (presets.size() > 100 && (renderCounter++ % 12) != 0)
                continue;

            std::vector<float> values (definition.params.size());

            for (size_t i = 0; i < values.size(); ++i)
                values[i] = definition.params[i].def;

            for (const auto& entry : preset.values)
            {
                const auto index = definition.indexOf (entry.first.toStdString());
                if (index >= 0)
                    values[(size_t) index] = entry.second;
            }

            renderEngine->reset();
            renderEngine->resetMatrix();
            renderEngine->setParameters (values.data(), (int) values.size());

            for (const auto& cell : preset.matrix)
                renderEngine->setMatrixCell (cell.source, cell.destination,
                                             cell.formula, cell.amount);

            triad::MidiEvent note;
            note.type = triad::MidiEventType::NoteOn;
            note.note = 60;
            note.value = 0.8f;
            renderEngine->processMidi (note);

            float left[256];
            float right[256];
            double energy = 0.0;
            float peak = 0.0f;
            bool finite = true;

            for (int block = 0; block < (int) (0.75 * 48000.0 / 256.0); ++block)
            {
                renderEngine->render (left, right, 256);

                for (int i = 0; i < 256; ++i)
                {
                    finite = finite && std::isfinite (left[i]) && std::isfinite (right[i]);
                    energy += (double) left[i] * left[i] + (double) right[i] * right[i];
                    peak = std::max (peak, std::max (std::abs (left[i]), std::abs (right[i])));
                }
            }

            check (finite, "preset produced non-finite audio", preset.name);
            check (energy > 1.0e-5, "preset is silent", preset.name);
            check (peak < 1.05f, "preset clips", preset.name);
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
