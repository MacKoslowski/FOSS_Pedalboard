#pragma once

#include "InfoIcon.h"
#include "core/PedalChain.h"

#include <juce_audio_processors/juce_audio_processors.h>

#include <memory>

namespace fpb
{

/** One pedal enclosure on the board: title, "(i)" icon, the pedal's own
    control panel, and a stomp switch bound to the slot's bypass flag. */
class PedalComponent : public juce::Component
{
public:
    PedalComponent (PedalChain& chain, int slotIndex, int pedalIndex, juce::AudioProcessorValueTreeState& apvts);

    /** Syncs the footswitch with the chain's bypass state. */
    void refresh();

    [[nodiscard]] int getPedalIndex() const { return pedalIndex_; }

    void paint (juce::Graphics& g) override;
    void resized() override;

    static constexpr int kTitleHeight = 34;
    static constexpr int kFootswitchHeight = 56;

private:
    PedalChain& chain_;
    const int slotIndex_;
    const int pedalIndex_;

    juce::Label title_;
    InfoIcon info_;
    std::unique_ptr<juce::Component> body_;
    juce::ToggleButton footswitch_;
};

} // namespace fpb
