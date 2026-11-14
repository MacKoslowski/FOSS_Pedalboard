#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

#include <memory>
#include <vector>

namespace fpb
{

/** A generic grid of controls bound to APVTS parameters.

    Picks the widget from the parameter type: a rotary knob for floats, a
    toggle for bools and a combo box for choices. Pedals use this as their
    default editor so a new pedal gets a working UI for free; a pedal can still
    return a custom component from createEditor() when it wants more.
*/
class ParameterPanel : public juce::Component
{
public:
    struct Item
    {
        juce::String paramId;
        juce::String label;
    };

    ParameterPanel (juce::AudioProcessorValueTreeState& apvts, std::vector<Item> items);
    ~ParameterPanel() override;

    void resized() override;

    static constexpr int kColumns = 2;
    static constexpr int kCellWidth = 70;
    static constexpr int kControlHeight = 52;
    static constexpr int kLabelHeight = 16;
    static constexpr int kCellHeight = kControlHeight + kLabelHeight;

    /** Height needed to show the given number of controls. */
    [[nodiscard]] static int heightForItems (int numItems);

private:
    struct Control
    {
        std::unique_ptr<juce::Component> widget;
        std::unique_ptr<juce::Label> label;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sliderAttachment;
        std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> buttonAttachment;
        std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> comboAttachment;
    };

    void addControl (juce::AudioProcessorValueTreeState& apvts, const Item& item);

    std::vector<Control> controls_;
};

} // namespace fpb
