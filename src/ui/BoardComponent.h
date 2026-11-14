#pragma once

#include "SlotComponent.h"
#include "core/PedalChain.h"

#include <functional>
#include <memory>
#include <vector>

namespace fpb
{

/** The whole pedalboard: title, global input/output gain, UI scaling box and
    the row of six slots. Listens to the chain and refreshes the slots. */
class BoardComponent : public juce::Component,
                       private juce::ChangeListener
{
public:
    BoardComponent (PedalChain& chain, juce::AudioProcessorValueTreeState& apvts);
    ~BoardComponent() override;

    /** Called when the user picks a new UI scale. */
    std::function<void (float)> onScaleChanged;

    /** Updates the scale box without firing onScaleChanged. */
    void setScaleSelection (float scale);

    void paint (juce::Graphics& g) override;
    void resized() override;

    static constexpr int kWidth = 1100;
    static constexpr int kHeight = 520;

private:
    void changeListenerCallback (juce::ChangeBroadcaster* source) override;
    void refreshSlots();

    PedalChain& chain_;
    juce::AudioProcessorValueTreeState& apvts_;

    std::vector<std::unique_ptr<SlotComponent>> slots_;

    juce::Slider inputGain_ { juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::NoTextBox };
    juce::Slider outputGain_ { juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::NoTextBox };
    juce::Label inputLabel_, outputLabel_, scaleLabel_;
    juce::ComboBox scaleBox_;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> inputAttachment_, outputAttachment_;

    juce::Rectangle<int> titleArea_, slotsArea_;
};

} // namespace fpb
