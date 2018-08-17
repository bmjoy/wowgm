#pragma once

#include <cstdint>

namespace wowgm::game::datastores
{
#pragma pack(push, 1)
    struct MapEntry
    {
        std::uint32_t ID;
        const char* Directory;
        std::uint32_t InstanceType;
        std::uint32_t Type;
        std::uint32_t Flags;
        std::uint32_t IsPvpMap;
        const char* Name;
        std::uint32_t LinkedZoneID;
        const char* Introductions[2];
        std::uint32_t LoadingScreenID;
        float BattlefieldMapIconScale;
        struct {
            std::uint32_t MapID;
            float X;
            float Y;
        } GhostInfo;
        std::uint32_t TimeOfDayOverride;
        std::uint32_t ExpansionID;
        std::uint32_t ExpireTime;
        std::uint32_t MaxPlayers;
        std::int32_t RootPhaseMapID;
    };
#pragma pack(pop)
}
