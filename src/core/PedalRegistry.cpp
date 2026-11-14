#include "PedalRegistry.h"

#include "pedals/DelayPedal.h"
#include "pedals/ReverbPedal.h"

namespace fpb
{

const std::vector<PedalDescriptor>& PedalRegistry::all()
{
    static const std::vector<PedalDescriptor> registry {
        { DelayPedal::kTypeId,  DelayPedal::kDisplayName,  [] { return std::make_unique<DelayPedal>(); } },
        { ReverbPedal::kTypeId, ReverbPedal::kDisplayName, [] { return std::make_unique<ReverbPedal>(); } },
    };

    return registry;
}

std::vector<std::unique_ptr<Pedal>> PedalRegistry::createAll()
{
    std::vector<std::unique_ptr<Pedal>> pedals;
    pedals.reserve (all().size());

    for (const auto& descriptor : all())
        pedals.push_back (descriptor.create());

    return pedals;
}

} // namespace fpb
