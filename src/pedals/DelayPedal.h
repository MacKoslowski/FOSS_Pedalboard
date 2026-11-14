#pragma once

/*  DelayPedal
    ----------
    Algorithm: digital delay line with feedback and a one-pole low-pass "tone"
    filter in the feedback path, optionally synced to host tempo.

    Source: juce::dsp::DelayLine (linear interpolation) and
            juce::dsp::FirstOrderTPTFilter, both from the JUCE framework
            (https://github.com/juce-framework/JUCE), used under the AGPLv3.
            The topology is the textbook feedback delay found in e.g. Zölzer,
            "DAFX: Digital Audio Effects", 2nd ed., Wiley 2011, ch. 2.
*/

#include "core/Pedal.h"

#include <atomic>

namespace fpb
{

class DelayPedal final : public Pedal
{
public:
    static constexpr const char* kTypeId = "delay";
    static constexpr const char* kDisplayName = "DELAY";

    [[nodiscard]] juce::String getTypeId() const override { return kTypeId; }
    [[nodiscard]] juce::String getDisplayName() const override { return kDisplayName; }
    [[nodiscard]] juce::String getAlgorithmCredit() const override;

    void addParameters (juce::AudioProcessorValueTreeState::ParameterLayout& layout) override;
    void attachParameters (juce::AudioProcessorValueTreeState& apvts) override;

    void prepare (const juce::dsp::ProcessSpec& spec) override;
    void reset() override;
    void setTransport (const TransportInfo& transport) override { transport_ = transport; }
    void process (juce::dsp::AudioBlock<float>& block) override;

    [[nodiscard]] std::unique_ptr<juce::Component> createEditor (juce::AudioProcessorValueTreeState& apvts) override;

    // Parameter names (combined with the type id by Pedal::paramId)
    static constexpr const char* pTime = "time";
    static constexpr const char* pFeedback = "feedback";
    static constexpr const char* pTone = "tone";
    static constexpr const char* pMix = "mix";
    static constexpr const char* pSync = "sync";
    static constexpr const char* pDivision = "division";

    static constexpr float kMaxDelayMs = 2000.0f;

private:
    [[nodiscard]] float computeDelayMs() const;
    [[nodiscard]] float msToSamples (float ms) const;

    std::atomic<float>* time_ = nullptr;
    std::atomic<float>* feedback_ = nullptr;
    std::atomic<float>* tone_ = nullptr;
    std::atomic<float>* mix_ = nullptr;
    std::atomic<float>* sync_ = nullptr;
    std::atomic<float>* division_ = nullptr;

    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> delay_;
    juce::dsp::FirstOrderTPTFilter<float> toneFilter_;

    juce::SmoothedValue<float> delaySamples_;
    juce::SmoothedValue<float> feedbackSmoothed_;
    juce::SmoothedValue<float> mixSmoothed_;

    double sampleRate_ = 44100.0;
    int maxDelaySamples_ = 1;
    TransportInfo transport_;
};

} // namespace fpb
