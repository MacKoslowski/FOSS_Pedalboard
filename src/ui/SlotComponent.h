#pragma once

#include "PedalComponent.h"
#include "core/PedalChain.h"

#include <memory>

namespace fpb
{

/** One of the six positions on the board. Shows either an "add pedal" button
    or a PedalComponent, plus the slot controls: move left, replace, remove,
    move right. */
class SlotComponent : public juce::Component
{
public:
    SlotComponent (PedalChain& chain, int slotIndex, juce::AudioProcessorValueTreeState& apvts);

    /** Rebuilds the pedal view if the chain changed. */
    void refresh();

    void paint (juce::Graphics& g) override;
    void resized() override;

    static constexpr int kButtonRowHeight = 26;

private:
    void showPedalMenu (juce::Component& anchor);

    PedalChain& chain_;
    const int slotIndex_;
    juce::AudioProcessorValueTreeState& apvts_;

    juce::TextButton moveLeft_ { "<" };
    juce::TextButton replace_ { "SWAP" };
    juce::TextButton remove_ { "X" };
    juce::TextButton moveRight_ { ">" };
    juce::TextButton add_ { "+ ADD PEDAL" };

    std::unique_ptr<PedalComponent> pedal_;
};

} // namespace fpb
