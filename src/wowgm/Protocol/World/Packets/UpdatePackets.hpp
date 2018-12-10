#pragma once

#include "Packet.hpp"
#include "WorldPacket.hpp"
#include "ObjectGuid.hpp"
#include "CMovementStatus.hpp"
#include "JamCliValuesUpdate.hpp"
#include "CClientObjCreate.hpp"

#include <cstdint>
#include <vector>

namespace wowgm::protocol::world::packets
{
    using namespace wowgm::game::structures;

    struct ClientUpdateObject final : public ServerPacket
    {
        ClientUpdateObject(WorldPacket&& packet);

        void Read() override;

        uint16_t MapID = 0xFFFF;
        std::vector<CClientObjCreate> Updates;
        std::vector<ObjectGuid> DestroyObjects;
    };

    struct ClientDestroyObject final : public ServerPacket
    {
        ClientDestroyObject(WorldPacket&& packet);

        void Read() override;

        ObjectGuid GUID = ObjectGuid::Empty;
        bool OnDeath = false;
    };
}
