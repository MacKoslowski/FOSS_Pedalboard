#include "PluginProcessor.h"
#include "pedals/DelayPedal.h"
#include "pedals/ReverbPedal.h"

#include <catch2/catch_test_macros.hpp>

using namespace fpb;

TEST_CASE ("A fresh processor has delay and reverb on the board", "[processor]")
{
    FossPedalboardProcessor processor;
    auto& chain = processor.getChain();

    REQUIRE (chain.getSlot (0).pedalIndex == chain.indexOfTypeId (DelayPedal::kTypeId));
    REQUIRE (chain.getSlot (1).pedalIndex == chain.indexOfTypeId (ReverbPedal::kTypeId));
    REQUIRE (chain.getSlot (2).pedalIndex == PedalChain::kNoPedal);
}

TEST_CASE ("Plugin state round-trips board layout, bypass, parameters and UI scale", "[processor][state]")
{
    juce::MemoryBlock state;
    PedalChain::Slots savedSlots;

    {
        FossPedalboardProcessor source;
        auto& chain = source.getChain();

        chain.swapSlots (0, 1);          // reverb first, delay second
        chain.swapSlots (1, 4);          // delay to slot 5
        chain.setBypassed (4, true);
        savedSlots = chain.getSlots();

        auto* feedback = source.getValueTreeState().getParameter (chain.getPedal (chain.indexOfTypeId (DelayPedal::kTypeId)).paramId (DelayPedal::pFeedback));
        REQUIRE (feedback != nullptr);
        feedback->setValueNotifyingHost (feedback->convertTo0to1 (0.77f));

        source.setUiScale (1.5f);
        source.getStateInformation (state);
    }

    FossPedalboardProcessor target;
    target.setStateInformation (state.getData(), (int) state.getSize());

    auto& chain = target.getChain();
    REQUIRE (chain.getSlots() == savedSlots);
    REQUIRE (chain.getSlot (4).bypassed);

    const auto feedbackId = chain.getPedal (chain.indexOfTypeId (DelayPedal::kTypeId)).paramId (DelayPedal::pFeedback);
    const auto* raw = target.getValueTreeState().getRawParameterValue (feedbackId);
    REQUIRE (raw != nullptr);
    REQUIRE (std::abs (raw->load() - 0.77f) < 0.01f);

    REQUIRE (juce::approximatelyEqual (target.getUiScale(), 1.5f));
}

TEST_CASE ("Garbage state is ignored", "[processor][state]")
{
    FossPedalboardProcessor processor;
    const auto before = processor.getChain().getSlots();

    const char junk[] = "definitely not a plugin state";
    processor.setStateInformation (junk, (int) sizeof (junk));

    REQUIRE (processor.getChain().getSlots() == before);
}

TEST_CASE ("Bus layouts: mono/stereo accepted, more inputs than outputs rejected", "[processor][buses]")
{
    FossPedalboardProcessor processor;

    auto layout = processor.getBusesLayout();

    layout.getMainInputChannelSet() = juce::AudioChannelSet::mono();
    layout.getMainOutputChannelSet() = juce::AudioChannelSet::stereo();
    REQUIRE (processor.checkBusesLayoutSupported (layout));

    layout.getMainInputChannelSet() = juce::AudioChannelSet::stereo();
    layout.getMainOutputChannelSet() = juce::AudioChannelSet::mono();
    REQUIRE_FALSE (processor.checkBusesLayoutSupported (layout));

    layout.getMainInputChannelSet() = juce::AudioChannelSet::create5point1();
    layout.getMainOutputChannelSet() = juce::AudioChannelSet::create5point1();
    REQUIRE_FALSE (processor.checkBusesLayoutSupported (layout));
}
