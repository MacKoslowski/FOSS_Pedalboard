#include "PluginProcessor.h"

#include "PluginEditor.h"
#include "core/ParameterIds.h"
#include "core/PedalRegistry.h"
#include "pedals/DelayPedal.h"
#include "pedals/ReverbPedal.h"

namespace fpb
{

FossPedalboardProcessor::FossPedalboardProcessor()
    : juce::AudioProcessor (BusesProperties()
                                .withInput ("Input", juce::AudioChannelSet::stereo(), true)
                                .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      pedals_ (PedalRegistry::createAll()),
      apvts_ (*this, nullptr, "FOSSPedalboard", createParameterLayout (pedals_)),
      chain_ (pedals_)
{
    for (auto& pedal : pedals_)
        pedal->attachParameters (apvts_);

    inputGainDb_ = apvts_.getRawParameterValue (ParamIds::inputGain);
    outputGainDb_ = apvts_.getRawParameterValue (ParamIds::outputGain);
    jassert (inputGainDb_ != nullptr && outputGainDb_ != nullptr);

    // A fresh board starts with something to hear.
    chain_.setPedal (0, chain_.indexOfTypeId (DelayPedal::kTypeId));
    chain_.setPedal (1, chain_.indexOfTypeId (ReverbPedal::kTypeId));
}

FossPedalboardProcessor::~FossPedalboardProcessor() = default;

juce::AudioProcessorValueTreeState::ParameterLayout
    FossPedalboardProcessor::createParameterLayout (std::vector<std::unique_ptr<Pedal>>& pedals)
{
    using namespace juce;
    AudioProcessorValueTreeState::ParameterLayout layout;

    const NormalisableRange<float> gainRange (-24.0f, 24.0f, 0.1f);

    layout.add (std::make_unique<AudioParameterFloat> (
        ParameterID { ParamIds::inputGain, 1 }, "Input Gain", gainRange, 0.0f,
        AudioParameterFloatAttributes().withLabel ("dB")));

    layout.add (std::make_unique<AudioParameterFloat> (
        ParameterID { ParamIds::outputGain, 1 }, "Output Gain", gainRange, 0.0f,
        AudioParameterFloatAttributes().withLabel ("dB")));

    for (auto& pedal : pedals)
        pedal->addParameters (layout);

    return layout;
}

// ---------------------------------------------------------------------------
// Audio
// ---------------------------------------------------------------------------

void FossPedalboardProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = (juce::uint32) juce::jmax (1, samplesPerBlock);
    spec.numChannels = (juce::uint32) juce::jmax (1, getTotalNumOutputChannels());

    chain_.prepare (spec);

    inputGain_.reset (sampleRate, 0.02);
    outputGain_.reset (sampleRate, 0.02);
    inputGain_.setCurrentAndTargetValue (juce::Decibels::decibelsToGain (inputGainDb_->load()));
    outputGain_.setCurrentAndTargetValue (juce::Decibels::decibelsToGain (outputGainDb_->load()));
}

void FossPedalboardProcessor::releaseResources()
{
    chain_.reset();
}

bool FossPedalboardProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto in = layouts.getMainInputChannelSet();
    const auto out = layouts.getMainOutputChannelSet();

    const auto isMonoOrStereo = [] (const juce::AudioChannelSet& set)
    {
        return set == juce::AudioChannelSet::mono() || set == juce::AudioChannelSet::stereo();
    };

    // Mono or stereo, and never more inputs than outputs (mono in / stereo out is fine).
    return isMonoOrStereo (in) && isMonoOrStereo (out) && in.size() <= out.size();
}

void FossPedalboardProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const int numSamples = buffer.getNumSamples();
    const int numInputs = getTotalNumInputChannels();
    const int numOutputs = getTotalNumOutputChannels();

    // Mono in, stereo out: feed both sides the same signal.
    for (int ch = numInputs; ch < numOutputs; ++ch)
    {
        if (numInputs > 0)
            buffer.copyFrom (ch, 0, buffer, 0, 0, numSamples);
        else
            buffer.clear (ch, 0, numSamples);
    }

    updateTransport();

    inputGain_.setTargetValue (juce::Decibels::decibelsToGain (inputGainDb_->load()));
    inputGain_.applyGain (buffer, numSamples);

    juce::dsp::AudioBlock<float> block (buffer);
    chain_.process (block);

    outputGain_.setTargetValue (juce::Decibels::decibelsToGain (outputGainDb_->load()));
    outputGain_.applyGain (buffer, numSamples);
}

void FossPedalboardProcessor::updateTransport()
{
    TransportInfo transport;

    if (auto* head = getPlayHead())
    {
        if (const auto position = head->getPosition())
        {
            if (const auto bpm = position->getBpm())
                transport.bpm = *bpm;

            transport.playing = position->getIsPlaying();
        }
    }

    chain_.setTransport (transport);
}

// ---------------------------------------------------------------------------
// State
// ---------------------------------------------------------------------------

void FossPedalboardProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts_.copyState();
    chain_.writeTo (state);

    if (const auto xml = state.createXml())
        copyXmlToBinary (*xml, destData);
}

void FossPedalboardProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    const auto xml = getXmlFromBinary (data, sizeInBytes);

    if (xml == nullptr || ! xml->hasTagName (apvts_.state.getType()))
        return;

    const auto tree = juce::ValueTree::fromXml (*xml);
    apvts_.replaceState (tree);
    chain_.readFrom (tree);
}

float FossPedalboardProcessor::getUiScale() const
{
    return (float) (double) apvts_.state.getProperty (StateIds::uiScale, 1.0);
}

void FossPedalboardProcessor::setUiScale (float scale)
{
    apvts_.state.setProperty (StateIds::uiScale, (double) scale, nullptr);
}

// ---------------------------------------------------------------------------
// Editor
// ---------------------------------------------------------------------------

juce::AudioProcessorEditor* FossPedalboardProcessor::createEditor()
{
    return new FossPedalboardEditor (*this);
}

} // namespace fpb

// This creates new instances of the plugin.
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new fpb::FossPedalboardProcessor();
}
