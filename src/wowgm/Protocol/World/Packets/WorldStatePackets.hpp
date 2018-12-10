#pragma once

#include "WorldPacket.hpp"
#include "Packet.hpp"

namespace wowgm::protocol::world::packets
{
    struct ClientInitWorldStates final : public ServerPacket
    {
        struct WorldStateInfo
        {
            uint32_t VariableID;
            int32_t Value;
        };

        ClientInitWorldStates(WorldPacket&& packet);

        void Read() override;

        uint32_t ZoneID = 0; ///< ZoneId
        uint32_t AreaID = 0; ///< AreaId
        uint32_t MapID = 0; ///< MapId

        std::vector<WorldStateInfo> Worldstates;
    };

    struct ClientUpdateWorldState final : public ServerPacket
    {
        ClientUpdateWorldState(WorldPacket&& packet);

        void Read() override;

        int32_t Value = 0;
        bool Hidden = false; ///< @todo: research
        uint32_t VariableID = 0;
    };
}
