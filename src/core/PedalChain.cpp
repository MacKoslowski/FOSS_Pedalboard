#include "PedalChain.h"

#include "ParameterIds.h"

namespace fpb
{

PedalChain::PedalChain (std::vector<std::unique_ptr<Pedal>>& pedals)
    : pedals_ (pedals)
{
    // 7 bits per slot for the pedal index (+1 so that 0 means empty) and 1 bit for bypass.
    jassert (pedals_.size() < 127);
}

// ---------------------------------------------------------------------------
// Packing
// ---------------------------------------------------------------------------

PedalChain::Packed PedalChain::pack (const Slots& slots)
{
    Packed packed = 0;

    for (int s = 0; s < kMaxSlots; ++s)
    {
        const auto& slot = slots[(size_t) s];
        const auto index = (std::uint8_t) (slot.pedalIndex + 1);
        const auto byte = (std::uint8_t) ((slot.bypassed ? 0x80u : 0u) | (index & 0x7fu));
        packed |= (Packed) byte << (8 * s);
    }

    return packed;
}

PedalChain::Slots PedalChain::unpack (Packed packed)
{
    Slots slots;

    for (int s = 0; s < kMaxSlots; ++s)
    {
        const auto byte = (std::uint8_t) ((packed >> (8 * s)) & 0xffu);
        slots[(size_t) s].pedalIndex = (int) (byte & 0x7fu) - 1;
        slots[(size_t) s].bypassed = (byte & 0x80u) != 0;
    }

    return slots;
}

void PedalChain::publish (const Slots& slots)
{
    packed_.store (pack (slots), std::memory_order_release);
    sendChangeMessage();
}

// ---------------------------------------------------------------------------
// Audio thread
// ---------------------------------------------------------------------------

void PedalChain::prepare (const juce::dsp::ProcessSpec& spec)
{
    dryBuffer_.setSize ((int) spec.numChannels, (int) spec.maximumBlockSize);
    dryBuffer_.clear();

    for (auto& pedal : pedals_)
        pedal->prepare (spec);

    for (auto& rt : runtime_)
    {
        rt.mix.reset (spec.sampleRate, kBypassRampSeconds);
        rt.lastPedalIndex = kNoPedal;
    }
}

void PedalChain::reset()
{
    for (auto& pedal : pedals_)
        pedal->reset();

    for (auto& rt : runtime_)
    {
        rt.mix.setCurrentAndTargetValue (0.0f);
        rt.lastPedalIndex = kNoPedal;
    }
}

void PedalChain::setTransport (const TransportInfo& transport)
{
    for (auto& pedal : pedals_)
        pedal->setTransport (transport);
}

void PedalChain::process (juce::dsp::AudioBlock<float>& block)
{
    const auto slots = unpack (packed_.load (std::memory_order_acquire));
    const auto numChannels = block.getNumChannels();
    const auto numSamples = block.getNumSamples();

    for (int s = 0; s < kMaxSlots; ++s)
    {
        const auto& slot = slots[(size_t) s];
        auto& rt = runtime_[(size_t) s];
        const bool active = slot.pedalIndex != kNoPedal && ! slot.bypassed;
        const float target = active ? 1.0f : 0.0f;

        if (slot.pedalIndex != rt.lastPedalIndex)
        {
            // A new pedal landed in this slot: start it from silence, no crossfade.
            if (slot.pedalIndex != kNoPedal)
                pedals_[(size_t) slot.pedalIndex]->reset();

            rt.lastPedalIndex = slot.pedalIndex;
            rt.mix.setCurrentAndTargetValue (target);
        }

        rt.mix.setTargetValue (target);

        if (slot.pedalIndex == kNoPedal)
            continue;

        auto& pedal = *pedals_[(size_t) slot.pedalIndex];

        if (! rt.mix.isSmoothing())
        {
            if (target > 0.5f)
                pedal.process (block);

            continue;
        }

        // Mid-crossfade: keep a dry copy, process, then blend per sample.
        auto dry = juce::dsp::AudioBlock<float> (dryBuffer_)
                       .getSubsetChannelBlock (0, numChannels)
                       .getSubBlock (0, numSamples);
        dry.copyFrom (block);
        pedal.process (block);

        for (size_t i = 0; i < numSamples; ++i)
        {
            const float g = rt.mix.getNextValue();

            for (size_t ch = 0; ch < numChannels; ++ch)
            {
                auto* wet = block.getChannelPointer (ch);
                const auto* dryData = dry.getChannelPointer (ch);
                wet[i] = dryData[i] + (wet[i] - dryData[i]) * g;
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Message thread
// ---------------------------------------------------------------------------

PedalChain::Slots PedalChain::getSlots() const
{
    return unpack (packed_.load (std::memory_order_acquire));
}

PedalChain::SlotState PedalChain::getSlot (int slot) const
{
    jassert (juce::isPositiveAndBelow (slot, kMaxSlots));
    return getSlots()[(size_t) slot];
}

void PedalChain::setSlots (const Slots& requested)
{
    // Enforce "one instance per pedal type": later duplicates are dropped.
    Slots slots = requested;

    for (int s = 0; s < kMaxSlots; ++s)
    {
        auto& slot = slots[(size_t) s];

        if (slot.pedalIndex != kNoPedal && ! juce::isPositiveAndBelow (slot.pedalIndex, getNumPedalTypes()))
            slot.pedalIndex = kNoPedal;

        for (int earlier = 0; earlier < s; ++earlier)
            if (slot.pedalIndex != kNoPedal && slots[(size_t) earlier].pedalIndex == slot.pedalIndex)
                slot.pedalIndex = kNoPedal;

        if (slot.pedalIndex == kNoPedal)
            slot.bypassed = false;
    }

    publish (slots);
}

bool PedalChain::setPedal (int slot, int pedalIndex)
{
    jassert (juce::isPositiveAndBelow (slot, kMaxSlots));

    if (pedalIndex != kNoPedal)
    {
        if (! juce::isPositiveAndBelow (pedalIndex, getNumPedalTypes()))
            return false;

        const int existing = findSlotOf (pedalIndex);

        if (existing != kNoPedal && existing != slot)
            return false;
    }

    auto slots = getSlots();
    slots[(size_t) slot] = { pedalIndex, false };
    publish (slots);
    return true;
}

void PedalChain::clearSlot (int slot)
{
    setPedal (slot, kNoPedal);
}

void PedalChain::swapSlots (int slotA, int slotB)
{
    jassert (juce::isPositiveAndBelow (slotA, kMaxSlots));
    jassert (juce::isPositiveAndBelow (slotB, kMaxSlots));

    if (slotA == slotB)
        return;

    auto slots = getSlots();
    std::swap (slots[(size_t) slotA], slots[(size_t) slotB]);
    publish (slots);
}

void PedalChain::setBypassed (int slot, bool shouldBeBypassed)
{
    jassert (juce::isPositiveAndBelow (slot, kMaxSlots));

    auto slots = getSlots();
    auto& state = slots[(size_t) slot];

    if (state.pedalIndex == kNoPedal || state.bypassed == shouldBeBypassed)
        return;

    state.bypassed = shouldBeBypassed;
    publish (slots);
}

int PedalChain::findSlotOf (int pedalIndex) const
{
    if (pedalIndex == kNoPedal)
        return kNoPedal;

    const auto slots = getSlots();

    for (int s = 0; s < kMaxSlots; ++s)
        if (slots[(size_t) s].pedalIndex == pedalIndex)
            return s;

    return kNoPedal;
}

int PedalChain::indexOfTypeId (const juce::String& typeId) const
{
    for (int i = 0; i < getNumPedalTypes(); ++i)
        if (pedals_[(size_t) i]->getTypeId() == typeId)
            return i;

    return kNoPedal;
}

// ---------------------------------------------------------------------------
// Persistence
// ---------------------------------------------------------------------------

void PedalChain::writeTo (juce::ValueTree& parent) const
{
    parent.removeChild (parent.getChildWithName (StateIds::board), nullptr);

    juce::ValueTree board (StateIds::board);
    const auto slots = getSlots();

    for (const auto& slot : slots)
    {
        juce::ValueTree node (StateIds::slot);
        node.setProperty (StateIds::pedal,
                          slot.pedalIndex == kNoPedal ? juce::String() : getPedal (slot.pedalIndex).getTypeId(),
                          nullptr);
        node.setProperty (StateIds::bypass, slot.bypassed, nullptr);
        board.appendChild (node, nullptr);
    }

    parent.appendChild (board, nullptr);
}

void PedalChain::readFrom (const juce::ValueTree& parent)
{
    const auto board = parent.getChildWithName (StateIds::board);

    if (! board.isValid())
        return;

    Slots slots;

    for (int s = 0; s < kMaxSlots && s < board.getNumChildren(); ++s)
    {
        const auto node = board.getChild (s);
        const auto typeId = node.getProperty (StateIds::pedal).toString();
        slots[(size_t) s].pedalIndex = typeId.isEmpty() ? kNoPedal : indexOfTypeId (typeId);
        slots[(size_t) s].bypassed = (bool) node.getProperty (StateIds::bypass, false);
    }

    setSlots (slots);
}

} // namespace fpb
