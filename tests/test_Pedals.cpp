#include "PluginProcessor.h"
#include "pedals/DelayPedal.h"
#include "pedals/ReverbPedal.h"

#include <catch2/catch_test_macros.hpp>

#include <cmath>

using namespace fpb;

namespace
{
    constexpr double kSampleRate = 48000.0;
    constexpr int kBlockSize = 512;

    void setParam (FossPedalboardProcessor& processor, const juce::String& id, float plainValue)
    {
        auto* param = processor.getValueTreeState().getParameter (id);
        REQUIRE (param != nullptr);
        param->setValueNotifyingHost (param->convertTo0to1 (plainValue));
    }

    bool allFinite (const juce::AudioBuffer<float>& buffer)
    {
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
            for (int i = 0; i < buffer.getNumSamples(); ++i)
                if (! std::isfinite (buffer.getSample (ch, i)))
                    return false;

        return true;
    }
} // namespace

TEST_CASE ("Every registered pedal reports an algorithm credit", "[pedals][credits]")
{
    FossPedalboardProcessor processor;
    auto& chain = processor.getChain();

    for (int i = 0; i < chain.getNumPedalTypes(); ++i)
    {
        const auto& pedal = chain.getPedal (i);
        INFO ("pedal " << pedal.getTypeId());
        REQUIRE (pedal.getTypeId().isNotEmpty());
        REQUIRE (pedal.getDisplayName().isNotEmpty());
        REQUIRE (pedal.getAlgorithmCredit().length() > 20);
    }
}

TEST_CASE ("Delay echoes an impulse after the set time", "[pedals][delay]")
{
    FossPedalboardProcessor processor;
    auto& chain = processor.getChain();
    const int delayIndex = chain.indexOfTypeId (DelayPedal::kTypeId);
    REQUIRE (delayIndex != PedalChain::kNoPedal);

    // Only the delay on the board.
    PedalChain::Slots slots;
    slots[0] = { delayIndex, false };
    chain.setSlots (slots);

    const auto& delay = chain.getPedal (delayIndex);
    constexpr float delayMs = 100.0f;
    setParam (processor, delay.paramId (DelayPedal::pTime), delayMs);
    setParam (processor, delay.paramId (DelayPedal::pFeedback), 0.0f);
    setParam (processor, delay.paramId (DelayPedal::pMix), 1.0f);
    setParam (processor, delay.paramId (DelayPedal::pSync), 0.0f);

    processor.prepareToPlay (kSampleRate, kBlockSize);

    const int expectedSample = (int) std::lround (delayMs * kSampleRate / 1000.0);
    const int totalSamples = expectedSample + kBlockSize * 2;

    juce::AudioBuffer<float> output (2, totalSamples);
    output.clear();

    juce::AudioBuffer<float> block (2, kBlockSize);
    juce::MidiBuffer midi;

    for (int start = 0; start < totalSamples; start += kBlockSize)
    {
        block.clear();

        if (start == 0)
            for (int ch = 0; ch < 2; ++ch)
                block.setSample (ch, 0, 1.0f);

        processor.processBlock (block, midi);
        REQUIRE (allFinite (block));

        const int n = juce::jmin (kBlockSize, totalSamples - start);

        for (int ch = 0; ch < 2; ++ch)
            output.copyFrom (ch, start, block, ch, 0, n);
    }

    // Find the loudest sample: it should be the echo, close to the expected position.
    int peakIndex = 0;
    float peak = 0.0f;

    for (int i = 1; i < totalSamples; ++i) // skip the (fully wet, so silent) direct sample
    {
        const float v = std::abs (output.getSample (0, i));

        if (v > peak)
        {
            peak = v;
            peakIndex = i;
        }
    }

    INFO ("peak " << peak << " at " << peakIndex << ", expected near " << expectedSample);
    REQUIRE (peak > 0.5f);
    REQUIRE (std::abs (peakIndex - expectedSample) <= 2);
}

TEST_CASE ("Reverb produces a decaying tail and stays finite", "[pedals][reverb]")
{
    FossPedalboardProcessor processor;
    auto& chain = processor.getChain();
    const int reverbIndex = chain.indexOfTypeId (ReverbPedal::kTypeId);
    REQUIRE (reverbIndex != PedalChain::kNoPedal);

    PedalChain::Slots slots;
    slots[0] = { reverbIndex, false };
    chain.setSlots (slots);

    const auto& reverb = chain.getPedal (reverbIndex);
    setParam (processor, reverb.paramId (ReverbPedal::pMix), 1.0f);
    setParam (processor, reverb.paramId (ReverbPedal::pSize), 0.8f);

    processor.prepareToPlay (kSampleRate, kBlockSize);

    juce::AudioBuffer<float> block (2, kBlockSize);
    juce::MidiBuffer midi;

    float earlyEnergy = 0.0f;
    float lateEnergy = 0.0f;

    for (int b = 0; b < 400; ++b) // ~4.3 s
    {
        block.clear();

        if (b == 0)
            block.setSample (0, 0, 1.0f), block.setSample (1, 0, 1.0f);

        processor.processBlock (block, midi);
        REQUIRE (allFinite (block));

        const float rms = block.getRMSLevel (0, 0, kBlockSize);

        if (b >= 2 && b < 20)
            earlyEnergy += rms;
        else if (b >= 380)
            lateEnergy += rms;
    }

    REQUIRE (earlyEnergy > 0.0f);
    REQUIRE (lateEnergy < earlyEnergy * 0.05f);
}
