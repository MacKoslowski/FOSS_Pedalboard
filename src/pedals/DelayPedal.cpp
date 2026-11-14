#include "DelayPedal.h"

#include "ui/ParameterPanel.h"

#include <cmath>

namespace fpb
{

namespace
{
    // Note divisions offered by the "division" parameter, as fractions of a quarter note.
    constexpr std::array<float, 5> kDivisionBeats { 1.0f, 0.5f, 0.75f, 1.0f / 3.0f, 0.25f };
    const juce::StringArray kDivisionNames { "1/4", "1/8", "1/8 dotted", "1/8 triplet", "1/16" };
} // namespace

juce::String DelayPedal::getAlgorithmCredit() const
{
    return "Feedback delay with a one-pole low-pass tone filter in the loop.\n"
           "Built from juce::dsp::DelayLine and juce::dsp::FirstOrderTPTFilter "
           "(JUCE framework, AGPLv3). Textbook topology, see Zolzer, DAFX (2011), ch. 2.";
}

void DelayPedal::addParameters (juce::AudioProcessorValueTreeState::ParameterLayout& layout)
{
    using namespace juce;

    layout.add (std::make_unique<AudioParameterFloat> (
        ParameterID { paramId (pTime), 1 }, "Delay Time",
        NormalisableRange<float> (1.0f, kMaxDelayMs, 0.01f, 0.4f), 350.0f,
        AudioParameterFloatAttributes().withLabel ("ms")));

    layout.add (std::make_unique<AudioParameterFloat> (
        ParameterID { paramId (pFeedback), 1 }, "Delay Feedback",
        NormalisableRange<float> (0.0f, 0.95f, 0.001f), 0.4f));

    layout.add (std::make_unique<AudioParameterFloat> (
        ParameterID { paramId (pTone), 1 }, "Delay Tone",
        NormalisableRange<float> (500.0f, 12000.0f, 1.0f, 0.35f), 4000.0f,
        AudioParameterFloatAttributes().withLabel ("Hz")));

    layout.add (std::make_unique<AudioParameterFloat> (
        ParameterID { paramId (pMix), 1 }, "Delay Mix",
        NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.35f));

    layout.add (std::make_unique<AudioParameterBool> (
        ParameterID { paramId (pSync), 1 }, "Delay Sync", false));

    layout.add (std::make_unique<AudioParameterChoice> (
        ParameterID { paramId (pDivision), 1 }, "Delay Division", kDivisionNames, 1));
}

void DelayPedal::attachParameters (juce::AudioProcessorValueTreeState& apvts)
{
    time_ = apvts.getRawParameterValue (paramId (pTime));
    feedback_ = apvts.getRawParameterValue (paramId (pFeedback));
    tone_ = apvts.getRawParameterValue (paramId (pTone));
    mix_ = apvts.getRawParameterValue (paramId (pMix));
    sync_ = apvts.getRawParameterValue (paramId (pSync));
    division_ = apvts.getRawParameterValue (paramId (pDivision));

    jassert (time_ != nullptr && feedback_ != nullptr && tone_ != nullptr
             && mix_ != nullptr && sync_ != nullptr && division_ != nullptr);
}

void DelayPedal::prepare (const juce::dsp::ProcessSpec& spec)
{
    sampleRate_ = spec.sampleRate;
    maxDelaySamples_ = (int) std::ceil (spec.sampleRate * kMaxDelayMs / 1000.0) + 1;

    delay_.setMaximumDelayInSamples (maxDelaySamples_);
    delay_.prepare (spec);

    toneFilter_.prepare (spec);
    toneFilter_.setType (juce::dsp::FirstOrderTPTFilterType::lowpass);

    delaySamples_.reset (spec.sampleRate, 0.1);
    feedbackSmoothed_.reset (spec.sampleRate, 0.02);
    mixSmoothed_.reset (spec.sampleRate, 0.02);

    reset();
}

void DelayPedal::reset()
{
    delay_.reset();
    toneFilter_.reset();

    delaySamples_.setCurrentAndTargetValue (msToSamples (computeDelayMs()));
    feedbackSmoothed_.setCurrentAndTargetValue (feedback_ != nullptr ? feedback_->load() : 0.0f);
    mixSmoothed_.setCurrentAndTargetValue (mix_ != nullptr ? mix_->load() : 0.0f);
}

float DelayPedal::computeDelayMs() const
{
    if (time_ == nullptr)
        return 350.0f;

    const bool synced = sync_ != nullptr && sync_->load() > 0.5f;

    if (synced && division_ != nullptr)
    {
        const auto index = juce::jlimit (0, (int) kDivisionBeats.size() - 1, (int) std::lround (division_->load()));
        const auto quarterMs = (float) (60000.0 / juce::jmax (20.0, transport_.bpm));
        return juce::jlimit (1.0f, kMaxDelayMs, quarterMs * kDivisionBeats[(size_t) index]);
    }

    return time_->load();
}

float DelayPedal::msToSamples (float ms) const
{
    const auto samples = (float) (ms * sampleRate_ / 1000.0);
    return juce::jlimit (1.0f, (float) (maxDelaySamples_ - 1), samples);
}

void DelayPedal::process (juce::dsp::AudioBlock<float>& block)
{
    const auto numChannels = (int) block.getNumChannels();
    const auto numSamples = (int) block.getNumSamples();

    toneFilter_.setCutoffFrequency (tone_->load());
    delaySamples_.setTargetValue (msToSamples (computeDelayMs()));
    feedbackSmoothed_.setTargetValue (feedback_->load());
    mixSmoothed_.setTargetValue (mix_->load());

    for (int i = 0; i < numSamples; ++i)
    {
        delay_.setDelay (delaySamples_.getNextValue());
        const float feedback = feedbackSmoothed_.getNextValue();
        const float mix = mixSmoothed_.getNextValue();

        for (int ch = 0; ch < numChannels; ++ch)
        {
            auto* data = block.getChannelPointer ((size_t) ch);
            const float in = data[i];
            const float wet = delay_.popSample (ch);
            delay_.pushSample (ch, in + toneFilter_.processSample (ch, wet) * feedback);
            data[i] = in + (wet - in) * mix;
        }
    }

    toneFilter_.snapToZero();
}

std::unique_ptr<juce::Component> DelayPedal::createEditor (juce::AudioProcessorValueTreeState& apvts)
{
    return std::make_unique<ParameterPanel> (apvts, std::vector<ParameterPanel::Item> {
        { paramId (pTime), "TIME" },
        { paramId (pFeedback), "FEEDBACK" },
        { paramId (pTone), "TONE" },
        { paramId (pMix), "MIX" },
        { paramId (pSync), "SYNC" },
        { paramId (pDivision), "NOTE" },
    });
}

} // namespace fpb
