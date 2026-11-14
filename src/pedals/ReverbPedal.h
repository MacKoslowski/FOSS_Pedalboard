#pragma once

/*  ReverbPedal
    -----------
    Algorithm: Freeverb, the Schroeder/Moorer style reverb (8 parallel comb
    filters into 4 series all-pass filters per channel) written by "Jezar at
    Dreampoint" around 2000 and released into the public domain.

    Source: juce::dsp::Reverb, the JUCE framework's Freeverb implementation
            (https://github.com/juce-framework/JUCE), used under the AGPLv3.
            Original Freeverb: https://ccrma.stanford.edu/~jos/pasp/Freeverb.html
*/

#include "core/Pedal.h"

#include <atomic>

namespace fpb
{

class ReverbPedal final : public Pedal
{
public:
    static constexpr const char* kTypeId = "reverb";
    static constexpr const char* kDisplayName = "REVERB";

    [[nodiscard]] juce::String getTypeId() const override { return kTypeId; }
    [[nodiscard]] juce::String getDisplayName() const override { return kDisplayName; }
    [[nodiscard]] juce::String getAlgorithmCredit() const override;

    void addParameters (juce::AudioProcessorValueTreeState::ParameterLayout& layout) override;
    void attachParameters (juce::AudioProcessorValueTreeState& apvts) override;

    void prepare (const juce::dsp::ProcessSpec& spec) override;
    void reset() override;
    void process (juce::dsp::AudioBlock<float>& block) override;

    [[nodiscard]] std::unique_ptr<juce::Component> createEditor (juce::AudioProcessorValueTreeState& apvts) override;

    static constexpr const char* pSize = "size";
    static constexpr const char* pDamping = "damping";
    static constexpr const char* pWidth = "width";
    static constexpr const char* pMix = "mix";

private:
    std::atomic<float>* size_ = nullptr;
    std::atomic<float>* damping_ = nullptr;
    std::atomic<float>* width_ = nullptr;
    std::atomic<float>* mix_ = nullptr;

    juce::dsp::Reverb reverb_;
};

} // namespace fpb
