#pragma once

#include "Packet.hpp"

namespace wowgm::protocol::world::packets
{
    struct ClientInitWorldStates final : public ServerPacket
    {
        struct WorldStateInfo
        {
            std::uint32_t VariableID;
            std::int32_t Value;
        };

        ClientInitWorldStates(WorldPacket&& packet);

        void Read() override;

        std::uint32_t ZoneID = 0; ///< ZoneId
        std::uint32_t AreaID = 0; ///< AreaId
        std::uint32_t MapID = 0; ///< MapId

        std::vector<WorldStateInfo> Worldstates;
    };

    struct ClientUpdateWorldState final : public ServerPacket
    {
        ClientUpdateWorldState(WorldPacket&& packet);

        void Read() override;

        std::int32_t Value = 0;
        bool Hidden = false; ///< @todo: research
        std::uint32_t VariableID = 0;
    };
}
