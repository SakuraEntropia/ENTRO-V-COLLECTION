#include "ui/PanelBase.h"

#include <algorithm>
#include <cmath>

namespace triad::ui
{

PanelBase::PanelBase (EditorHost& host, juce::String title, int designWidth, int designHeight,
                      juce::Colour accent)
    : host_ (host), title_ (std::move (title)), designWidth_ (designWidth),
      designHeight_ (designHeight), accent_ (accent)
{
    setSize (designWidth_, designHeight_);
    setOpaque (true);
    fallbackParameter_ = std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "panel.fallback", 1 }, "Fallback",
        juce::NormalisableRange<float> (0.0f, 3.0f, 1.0f), 0.0f);
    refreshPatchCables();
}

PanelBase::~PanelBase() = default;

void PanelBase::setBranding (juce::String mainText, juce::String subText)
{
    brandMain_ = std::move (mainText);
    brandSub_ = std::move (subText);
}

juce::RangedAudioParameter* PanelBase::parameterFor (const juce::String& paramId) const
{
    return host_.apvts().getParameter (paramId);
}

void PanelBase::addModule (const juce::String& moduleTitle, juce::Rectangle<int> area)
{
    addModule (moduleTitle, {}, area);
}

void PanelBase::addModule (const juce::String& moduleTitle, const juce::String& subtitle,
                           juce::Rectangle<int> area)
{
    modules_.push_back ({ moduleTitle, subtitle, area });
}

void PanelBase::addLabel (const juce::String& text, juce::Rectangle<int> area, float fontSize,
                          juce::Colour colour, bool bold)
{
    auto label = std::make_unique<juce::Label>();
    label->setText (text, juce::dontSendNotification);
    label->setJustificationType (juce::Justification::centred);
    label->setInterceptsMouseClicks (false, false);
    label->setColour (juce::Label::textColourId, colour);
    label->setFont (juce::Font (juce::FontOptions (fontSize, bold ? juce::Font::bold : juce::Font::plain)));
    label->setMinimumHorizontalScale (0.55f);
    label->setBounds (area);
    addAndMakeVisible (*label);
    labels_.push_back (std::move (label));
}

juce::Label& PanelBase::addCaption (const juce::String& text, juce::Rectangle<int> area,
                                    juce::Justification justification, float fontSize,
                                    juce::Colour colour, bool bold)
{
    auto label = std::make_unique<juce::Label>();
    label->setText (text, juce::dontSendNotification);
    label->setJustificationType (justification);
    label->setInterceptsMouseClicks (false, false);
    label->setColour (juce::Label::textColourId, colour);
    label->setFont (juce::Font (juce::FontOptions (fontSize, bold ? juce::Font::bold : juce::Font::plain)));
    label->setMinimumHorizontalScale (0.55f);
    label->setBounds (area);
    addAndMakeVisible (*label);
    auto& ref = *label;
    labels_.push_back (std::move (label));
    return ref;
}

juce::Slider& PanelBase::addKnob (const juce::String& paramId, const juce::String& labelText,
                                  juce::Rectangle<int> area)
{
    auto spec = definition().find (paramId.toStdString());
    auto* parameter = parameterFor (paramId);

    auto slider = std::make_unique<juce::Slider>();
    slider->setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slider->setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    slider->setScrollWheelEnabled (true);
    slider->setMouseDragSensitivity (110);
    slider->setRotaryParameters (juce::MathConstants<float>::pi * 1.2f,
                                 juce::MathConstants<float>::pi * 2.8f, true);
    slider->setTooltip (spec != nullptr ? juce::String (spec->name) + "  [" + paramId + "]" : paramId);

    const auto labelHeight = 12;
    const auto size = juce::jmax (18, juce::jmin (area.getWidth(), area.getHeight() - labelHeight));
    slider->setBounds (area.getX() + (area.getWidth() - size) / 2, area.getY(), size, size);

    if (spec != nullptr)
        slider->setDoubleClickReturnValue (true, juce::jlimit (spec->min, spec->max, spec->def));

    addAndMakeVisible (*slider);

    if (parameter != nullptr)
        sliderAttachments_.push_back (
            std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (host_.apvts(), paramId, *slider));

    auto& ref = *slider;
    sliders_.push_back (std::move (slider));

    addLabel (labelText, { area.getX(), area.getBottom() - labelHeight, area.getWidth(), labelHeight },
              10.2f, juce::Colour (0xffd3cfc6), false);

    return ref;
}

juce::Slider& PanelBase::addKnob (const juce::String& paramId, const juce::String& labelText,
                                  juce::Point<int> centre, int diameter)
{
    return addKnob (paramId, labelText,
                    { centre.x - diameter / 2, centre.y - diameter / 2,
                      diameter, diameter + 12 });
}

juce::Slider& PanelBase::addFader (const juce::String& paramId, const juce::String& labelText,
                                  juce::Rectangle<int> area)
{
    auto spec = definition().find (paramId.toStdString());
    auto* parameter = parameterFor (paramId);

    auto slider = std::make_unique<juce::Slider>();
    slider->setSliderStyle (juce::Slider::LinearVertical);
    slider->setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    slider->setTooltip (spec != nullptr ? juce::String (spec->name) + "  [" + paramId + "]" : paramId);

    const auto labelHeight = 11;
    const auto faderHeight = juce::jmax (24, area.getHeight() - labelHeight);
    slider->setBounds (area.getX() + 4, area.getY(), area.getWidth() - 8, faderHeight);

    if (spec != nullptr)
        slider->setDoubleClickReturnValue (true, juce::jlimit (spec->min, spec->max, spec->def));

    addAndMakeVisible (*slider);

    if (parameter != nullptr)
        sliderAttachments_.push_back (
            std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (host_.apvts(), paramId, *slider));

    auto& ref = *slider;
    sliders_.push_back (std::move (slider));

    addLabel (labelText, { area.getX(), area.getBottom() - labelHeight, area.getWidth(), labelHeight },
              9.6f, juce::Colour (0xffc9c5bb), false);

    return ref;
}

juce::ComboBox& PanelBase::addChoice (const juce::String& paramId, const juce::String& labelText,
                                      juce::Rectangle<int> area)
{
    auto spec = definition().find (paramId.toStdString());
    auto* parameter = parameterFor (paramId);

    auto combo = std::make_unique<juce::ComboBox>();
    juce::StringArray items;

    if (parameter != nullptr)
        items = parameter->getAllValueStrings();

    if (items.isEmpty() && spec != nullptr)
        for (const auto& choice : spec->choices)
            items.add (choice);

    combo->addItemList (items, 1);
    combo->setJustificationType (juce::Justification::centredLeft);
    combo->setTooltip (spec != nullptr ? juce::String (spec->name) + "  [" + paramId + "]" : paramId);

    const auto labelHeight = 11;
    const auto comboHeight = juce::jlimit (18, 24, area.getHeight() - labelHeight);
    combo->setBounds (area.getX(), area.getY(), area.getWidth(), comboHeight);

    if (parameter != nullptr)
        comboAttachments_.push_back (
            std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (host_.apvts(), paramId, *combo));

    addAndMakeVisible (*combo);
    auto& ref = *combo;
    combos_.push_back (std::move (combo));

    if (labelText.isNotEmpty())
        addLabel (labelText, { area.getX(), area.getBottom() - labelHeight, area.getWidth(), labelHeight },
                  9.6f, juce::Colour (0xffc9c5bb), false);

    return ref;
}

HardwareSwitch& PanelBase::addSwitch (const juce::String& paramId, const juce::String& labelText,
                                      juce::Rectangle<int> area, bool vertical)
{
    auto* parameter = parameterFor (paramId);
    auto spec = definition().find (paramId.toStdString());

    if (parameter == nullptr)
    {
        jassertfalse;
        parameter = fallbackParameter_.get();
    }

    auto component = std::make_unique<HardwareSwitch> (
        *parameter, vertical ? HardwareSwitch::Orientation::Vertical : HardwareSwitch::Orientation::Horizontal);
    component->setTooltip (spec != nullptr ? juce::String (spec->name) + "  [" + paramId + "]" : paramId);

    const auto labelHeight = 11;
    const auto switchHeight = juce::jmax (18, area.getHeight() - labelHeight);
    component->setBounds (area.getX(), area.getY(), area.getWidth(), switchHeight);

    addAndMakeVisible (*component);
    auto& ref = *component;
    switches_.push_back (std::move (component));

    if (labelText.isNotEmpty())
        addLabel (labelText, { area.getX(), area.getBottom() - labelHeight, area.getWidth(), labelHeight },
                  9.6f, juce::Colour (0xffc9c5bb), false);

    return ref;
}

LedButton& PanelBase::addLed (const juce::String& paramId, const juce::String& labelText,
                              juce::Rectangle<int> area)
{
    auto* parameter = parameterFor (paramId);
    auto spec = definition().find (paramId.toStdString());

    if (parameter == nullptr)
    {
        jassertfalse;
        parameter = fallbackParameter_.get();
    }

    auto component = std::make_unique<LedButton> (*parameter, labelText);
    component->setTooltip (spec != nullptr ? juce::String (spec->name) + "  [" + paramId + "]" : paramId);
    component->setBounds (area);
    addAndMakeVisible (*component);
    auto& ref = *component;
    leds_.push_back (std::move (component));
    return ref;
}

Jack& PanelBase::addJack (const juce::String& patchId, juce::Point<int> centre, int diameter)
{
    return addJack (patchId, { centre.x - diameter / 2, centre.y - diameter / 2, diameter, diameter });
}

Jack& PanelBase::addJack (const juce::String& patchId, juce::Rectangle<int> area)
{
    const auto index = definition().patchIndexOf (patchId.toStdString());
    const auto* spec = index >= 0 ? &definition().patchPoints[(size_t) index] : nullptr;

    auto jack = std::make_unique<Jack> (index, spec != nullptr && spec->isOutput,
                                        spec != nullptr ? juce::String (spec->name) : patchId,
                                        accent_);
    jack->setBounds (area);
    jack->onJackClicked = [this] (Jack& j) { handleJackClicked (j); };
    jack->onJackRightClicked = [this] (Jack& j) { handleJackRightClicked (j); };
    addAndMakeVisible (*jack);
    auto& ref = *jack;
    jacks_.push_back (std::move (jack));

    if (spec == nullptr)
    {
        ref.setTooltip ("Missing patch point: " + patchId);
    }
    else
    {
        const auto labelHeight = 8;
        addLabel (juce::String (spec->name),
                  { area.getX() - 22, area.getBottom(), area.getWidth() + 44, labelHeight },
                  7.6f, juce::Colour (0xffb6b2a9), false);
    }

    return ref;
}

juce::TextButton& PanelBase::addButton (const juce::String& text, juce::Rectangle<int> area,
                                       std::function<void()> onClick)
{
    auto button = std::make_unique<juce::TextButton> (text);
    button->setBounds (area);
    button->setTooltip (text);
    if (onClick != nullptr)
        button->onClick = std::move (onClick);

    addAndMakeVisible (*button);
    auto& ref = *button;
    buttons_.push_back (std::move (button));
    return ref;
}

SourceMatrix& PanelBase::addSourceMatrix (std::vector<juce::String> sourceParameterIds,
                                         int rows, juce::Rectangle<int> area)
{
    auto matrix = std::make_unique<SourceMatrix> (host_.apvts(), std::move (sourceParameterIds),
                                                   rows, 8);
    matrix->setBounds (area);
    addAndMakeVisible (*matrix);
    auto& ref = *matrix;
    matrices_.push_back (std::move (matrix));
    return ref;
}

LcdDisplay& PanelBase::addLcd (juce::Rectangle<int> area)
{
    auto lcd = std::make_unique<LcdDisplay>();
    lcd->setBounds (area);
    addAndMakeVisible (*lcd);
    auto& ref = *lcd;
    lcds_.push_back (std::move (lcd));
    return ref;
}

juce::Slider& PanelBase::knobFor (const juce::String& paramId)
{
    for (size_t i = 0; i < sliders_.size(); ++i)
        if (sliders_[i]->getTooltip().contains (paramId))
            return *sliders_[i];

    jassertfalse;
    return *sliders_.front();
}

void PanelBase::clearControls()
{
    sliderAttachments_.clear();
    comboAttachments_.clear();
    labels_.clear();
    sliders_.clear();
    combos_.clear();
    switches_.clear();
    leds_.clear();
    lcds_.clear();
    buttons_.clear();
    matrices_.clear();
    jacks_.clear();
    modules_.clear();
    selectedJack_ = -1;
}

void PanelBase::refreshPatchCables()
{
    cables_ = host_.patchCables();

    for (auto& jack : jacks_)
    {
        int count = 0;
        for (const auto& cable : cables_)
            if (cable.from == jack->patchIndex() || cable.to == jack->patchIndex())
                ++count;
        jack->setCableCount (count);
    }

    repaint();
}

Jack* PanelBase::findJack (int patchIndex) const
{
    for (const auto& jack : jacks_)
        if (jack->patchIndex() == patchIndex)
            return jack.get();

    return nullptr;
}

juce::Colour PanelBase::cableColour (int index) const
{
    static const juce::Colour colours[] = {
        juce::Colour (0xffd64b3a), juce::Colour (0xff3f8ed6), juce::Colour (0xffd8b03c),
        juce::Colour (0xff57b06a), juce::Colour (0xffb05fce), juce::Colour (0xffd07a3a)
    };

    return colours[(size_t) std::abs (index) % 6];
}

void PanelBase::handleJackClicked (Jack& jack)
{
    if (jack.patchIndex() < 0)
        return;

    if (selectedJack_ < 0)
    {
        selectedJack_ = jack.patchIndex();
        jack.setSelected (true);
        return;
    }

    if (selectedJack_ == jack.patchIndex())
    {
        jack.setSelected (false);
        selectedJack_ = -1;
        return;
    }

    auto* first = findJack (selectedJack_);
    if (first == nullptr)
    {
        selectedJack_ = jack.patchIndex();
        jack.setSelected (true);
        return;
    }

    int from = first->patchIndex();
    int to = jack.patchIndex();

    if (first->isOutput() && ! jack.isOutput())
    {
        from = first->patchIndex();
        to = jack.patchIndex();
    }
    else if (! first->isOutput() && jack.isOutput())
    {
        from = jack.patchIndex();
        to = first->patchIndex();
    }

    // Only one cable may drive a destination.
    for (const auto& cable : cables_)
        if (cable.to == to && ! (cable.from == from && cable.to == to))
            host_.setPatchCable (cable.from, cable.to, false);

    host_.setPatchCable (from, to, true);

    first->setSelected (false);
    selectedJack_ = -1;
    refreshPatchCables();

    if (onPatchChanged)
        onPatchChanged();
}

void PanelBase::handleJackRightClicked (Jack& jack)
{
    auto list = host_.patchCables();

    for (const auto& cable : list)
        if (cable.from == jack.patchIndex() || cable.to == jack.patchIndex())
            host_.setPatchCable (cable.from, cable.to, false);

    if (selectedJack_ == jack.patchIndex())
        selectedJack_ = -1;

    jack.setSelected (false);
    refreshPatchCables();

    if (onPatchChanged)
        onPatchChanged();
}

void PanelBase::paint (juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat();

    juce::ColourGradient background (juce::Colour (0xff232327), 0.0f, 0.0f,
                                     juce::Colour (0xff0f0f11), 0.0f, bounds.getBottom(), false);
    background.addColour (0.45, juce::Colour (0xff1a1a1d));
    g.setGradientFill (background);
    g.fillRect (bounds);

    // Brushed-metal texture.
    g.setColour (juce::Colours::white.withAlpha (0.018f));
    for (float y = 0.0f; y < bounds.getHeight(); y += 3.0f)
        g.drawHorizontalLine ((int) y, 0.0f, bounds.getWidth());

    // Panel border.
    g.setColour (juce::Colour (0xff000000).withAlpha (0.7f));
    g.drawRect (getLocalBounds(), 2);

    for (const auto& module : modules_)
    {
        const auto area = module.bounds.toFloat();
        g.setColour (juce::Colour (0xff151517));
        g.fillRoundedRectangle (area, 3.0f);

        g.setColour (accent_.withAlpha (0.22f));
        g.drawRoundedRectangle (area.reduced (0.5f), 3.0f, 1.0f);

        g.setColour (accent_.withAlpha (0.9f));
        g.setFont (juce::Font (juce::FontOptions (12.5f, juce::Font::bold)));
        g.drawText (module.title.toUpperCase(),
                    module.bounds.withHeight (15).reduced (7, 0),
                    juce::Justification::centredLeft);

        if (module.subtitle.isNotEmpty())
        {
            g.setColour (juce::Colour (0xff8f8c85));
            g.setFont (juce::Font (juce::FontOptions (8.0f)));
            g.drawText (module.subtitle,
                        module.bounds.withTrimmedTop (12).withHeight (11).reduced (7, 0),
                        juce::Justification::centredLeft);
        }

        g.setColour (juce::Colour (0xffffffff).withAlpha (0.07f));
        g.drawHorizontalLine (module.bounds.getY() + 24, area.getX() + 6.0f, area.getRight() - 6.0f);
    }

    if (brandMain_.isNotEmpty())
    {
        g.setColour (juce::Colour (0xffd9d5cc));
        g.setFont (juce::Font (juce::FontOptions (26.0f)));
        g.drawText (brandMain_, getLocalBounds().removeFromBottom (44).withTrimmedLeft (18),
                    juce::Justification::centredLeft);

        g.setColour (juce::Colour (0xff8f8c85));
        g.setFont (juce::Font (juce::FontOptions (9.0f)));
        g.drawText (brandSub_.toUpperCase(), getLocalBounds().removeFromBottom (44).withTrimmedLeft (220),
                    juce::Justification::centredLeft);
    }

    // Screws.
    g.setColour (juce::Colour (0xff4a4a50));
    for (const auto& point : { juce::Point<int> (8, 8), juce::Point<int> (getWidth() - 8, 8),
                               juce::Point<int> (8, getHeight() - 8),
                               juce::Point<int> (getWidth() - 8, getHeight() - 8) })
    {
        g.fillEllipse (juce::Rectangle<float> (5.0f, 5.0f).withCentre (point.toFloat()));
        g.setColour (juce::Colour (0xff0a0a0c));
        g.drawLine ((float) point.x - 1.5f, (float) point.y, (float) point.x + 1.5f, (float) point.y, 1.0f);
        g.setColour (juce::Colour (0xff4a4a50));
    }
}

void PanelBase::paintOverChildren (juce::Graphics& g)
{
    for (const auto& cable : cables_)
    {
        auto* from = findJack (cable.from);
        auto* to = findJack (cable.to);

        if (from == nullptr || to == nullptr)
            continue;

        const auto p1 = from->getBounds().getCentre().toFloat();
        const auto p2 = to->getBounds().getCentre().toFloat();

        const auto distance = p1.getDistanceFrom (p2);
        const auto sag = juce::jlimit (14.0f, 90.0f, distance * 0.18f + 12.0f);

        juce::Path path;
        path.startNewSubPath (p1);
        path.cubicTo ({ p1.x, p1.y + sag }, { p2.x, p2.y + sag }, p2);

        const auto colour = cableColour (cable.from);

        g.setColour (juce::Colours::black.withAlpha (0.45f));
        g.strokePath (path, juce::PathStrokeType (6.0f, juce::PathStrokeType::curved,
                                                  juce::PathStrokeType::rounded));

        g.setColour (colour.withAlpha (0.9f));
        g.strokePath (path, juce::PathStrokeType (3.0f, juce::PathStrokeType::curved,
                                                  juce::PathStrokeType::rounded));

        g.setColour (colour.brighter (0.3f).withAlpha (0.45f));
        g.strokePath (path, juce::PathStrokeType (1.0f, juce::PathStrokeType::curved,
                                                  juce::PathStrokeType::rounded));
    }

    if (host_.hasPatching() && ! cables_.empty())
    {
        g.setColour (juce::Colour (0xff8f8c85).withAlpha (0.8f));
        g.setFont (juce::Font (juce::FontOptions (8.5f)));
        g.drawText ("PATCH: click an output, then an input. Right-click a jack to clear.",
                    getLocalBounds().removeFromBottom (14).reduced (18, 0),
                    juce::Justification::centredRight);
    }
}

} // namespace triad::ui
