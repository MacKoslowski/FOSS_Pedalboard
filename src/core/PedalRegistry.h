#pragma once

#include "Pedal.h"

#include <functional>
#include <memory>
#include <vector>

namespace fpb
{

/** Describes one pedal type available to the board. */
struct PedalDescriptor
{
    const char* typeId;
    const char* displayName;
    std::function<std::unique_ptr<Pedal>()> create;
};

/** The single list of pedal types. Adding a pedal means adding one line to
    PedalRegistry.cpp. Order here is the order shown in the "add pedal" menu. */
namespace PedalRegistry
{
    [[nodiscard]] const std::vector<PedalDescriptor>& all();

    /** Creates one instance of every registered pedal, in registry order. */
    [[nodiscard]] std::vector<std::unique_ptr<Pedal>> createAll();
} // namespace PedalRegistry

} // namespace fpb
