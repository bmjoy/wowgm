#pragma once

#include <unordered_map>
#include <cstdint>

namespace wowgm::game::structures
{
    struct JamCliValuesUpdate
    {
        std::unordered_map<uint32_t, uint32_t> Descriptors;
    };
}