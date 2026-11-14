#pragma once

#include "core/Pedal.h"
#include "core/PedalChain.h"

#include <juce_audio_processors/juce_audio_processors.h>

#include <atomic>
#include <memory>
#include <vector>

namespace fpb
{

/** The plugin. Owns one instance of every pedal type, the parameter tree and
    the PedalChain that routes audio through the occupied slots. */
class FossPedalboardProcessor : public juce::AudioProcessor
{
public:
    FossPedalboardProcessor();
    ~FossPedalboardProcessor() override;

    // AudioProcessor ------------------------------------------------------
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "FOSS Pedalboard"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 4.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return "Default"; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // Project API ---------------------------------------------------------
    [[nodiscard]] juce::AudioProcessorValueTreeState& getValueTreeState() { return apvts_; }
    [[nodiscard]] PedalChain& getChain() { return chain_; }

    [[nodiscard]] float getUiScale() const;
    void setUiScale (float scale);

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout
        createParameterLayout (std::vector<std::unique_ptr<Pedal>>& pedals);

    void updateTransport();

    // Declaration order matters: pedals build the layout, the APVTS feeds the chain.
    std::vector<std::unique_ptr<Pedal>> pedals_;
    juce::AudioProcessorValueTreeState apvts_;
    PedalChain chain_;

    std::atomic<float>* inputGainDb_ = nullptr;
    std::atomic<float>* outputGainDb_ = nullptr;
    juce::LinearSmoothedValue<float> inputGain_ { 1.0f };
    juce::LinearSmoothedValue<float> outputGain_ { 1.0f };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FossPedalboardProcessor)
};

} // namespace fpb
