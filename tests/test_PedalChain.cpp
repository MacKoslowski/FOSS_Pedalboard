#include "core/PedalChain.h"
#include "core/PedalRegistry.h"
#include "PluginProcessor.h"

#include <catch2/catch_test_macros.hpp>

#include <cmath>

using namespace fpb;

TEST_CASE ("PedalChain starts empty and enforces one instance per type", "[chain]")
{
    auto pedals = PedalRegistry::createAll();
    PedalChain chain (pedals);

    REQUIRE (chain.getNumPedalTypes() >= 2);

    for (int s = 0; s < PedalChain::kMaxSlots; ++s)
        REQUIRE (chain.getSlot (s).pedalIndex == PedalChain::kNoPedal);

    REQUIRE (chain.setPedal (0, 0));
    REQUIRE (chain.findSlotOf (0) == 0);

    SECTION ("the same pedal cannot be placed in a second slot")
    {
        REQUIRE_FALSE (chain.setPedal (3, 0));
        REQUIRE (chain.getSlot (3).pedalIndex == PedalChain::kNoPedal);
    }

    SECTION ("re-setting the same pedal in its own slot is allowed")
    {
        REQUIRE (chain.setPedal (0, 0));
    }

    SECTION ("out-of-range pedal indices are rejected")
    {
        REQUIRE_FALSE (chain.setPedal (1, chain.getNumPedalTypes()));
        REQUIRE_FALSE (chain.setPedal (1, -7));
    }
}

TEST_CASE ("PedalChain swap, bypass and clear", "[chain]")
{
    auto pedals = PedalRegistry::createAll();
    PedalChain chain (pedals);

    REQUIRE (chain.setPedal (0, 0));
    REQUIRE (chain.setPedal (1, 1));

    chain.setBypassed (1, true);
    REQUIRE (chain.getSlot (1).bypassed);

    chain.swapSlots (0, 1);
    REQUIRE (chain.getSlot (0).pedalIndex == 1);
    REQUIRE (chain.getSlot (0).bypassed);
    REQUIRE (chain.getSlot (1).pedalIndex == 0);
    REQUIRE_FALSE (chain.getSlot (1).bypassed);

    chain.clearSlot (0);
    REQUIRE (chain.getSlot (0).pedalIndex == PedalChain::kNoPedal);
    REQUIRE_FALSE (chain.getSlot (0).bypassed);

    // Bypassing an empty slot is a no-op.
    chain.setBypassed (0, true);
    REQUIRE_FALSE (chain.getSlot (0).bypassed);
}

TEST_CASE ("PedalChain setSlots drops duplicates and bad indices", "[chain]")
{
    auto pedals = PedalRegistry::createAll();
    PedalChain chain (pedals);

    PedalChain::Slots slots;
    slots[0] = { 1, false };
    slots[1] = { 1, true };     // duplicate of slot 0
    slots[2] = { 99, false };   // bad index
    slots[3] = { 0, true };
    chain.setSlots (slots);

    REQUIRE (chain.getSlot (0).pedalIndex == 1);
    REQUIRE (chain.getSlot (1).pedalIndex == PedalChain::kNoPedal);
    REQUIRE (chain.getSlot (2).pedalIndex == PedalChain::kNoPedal);
    REQUIRE (chain.getSlot (3).pedalIndex == 0);
    REQUIRE (chain.getSlot (3).bypassed);
}

TEST_CASE ("PedalChain round-trips through a ValueTree", "[chain][state]")
{
    auto pedals = PedalRegistry::createAll();
    PedalChain chain (pedals);

    REQUIRE (chain.setPedal (2, 1));
    REQUIRE (chain.setPedal (5, 0));
    chain.setBypassed (5, true);

    juce::ValueTree tree ("ROOT");
    chain.writeTo (tree);

    auto otherPedals = PedalRegistry::createAll();
    PedalChain restored (otherPedals);
    restored.readFrom (tree);

    REQUIRE (restored.getSlots() == chain.getSlots());
}

TEST_CASE ("PedalChain processes silence to silence with no NaNs", "[chain][audio]")
{
    FossPedalboardProcessor processor;
    auto& chain = processor.getChain();

    REQUIRE (chain.setPedal (0, 0));
    REQUIRE (chain.setPedal (1, 1));
    chain.setBypassed (1, true);

    constexpr double sampleRate = 48000.0;
    constexpr int blockSize = 256;
    processor.prepareToPlay (sampleRate, blockSize);

    juce::AudioBuffer<float> buffer (2, blockSize);
    juce::MidiBuffer midi;

    for (int block = 0; block < 50; ++block)
    {
        buffer.clear();

        // Toggle bypass mid-way so the crossfade path runs too.
        if (block == 20)
            chain.setBypassed (0, true);

        processor.processBlock (buffer, midi);

        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            const auto* data = buffer.getReadPointer (ch);

            for (int i = 0; i < blockSize; ++i)
            {
                REQUIRE_FALSE (std::isnan (data[i]));
                REQUIRE (std::abs (data[i]) < 1.0e-6f);
            }
        }
    }
}
