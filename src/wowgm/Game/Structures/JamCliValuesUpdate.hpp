#pragma once

#include <unordered_map>
#include <cstdint>

namespace wowgm::game::structures
{
    struct JamCliValuesUpdate
    {
        std::unordered_map<std::uint32_t, std::uint32_t> Descriptors;
    };
}