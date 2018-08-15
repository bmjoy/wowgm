#pragma once

#include "Packet.hpp"
#include "WorldPacket.hpp"
#include "ObjectGuid.hpp"
#include "CMovementStatus.hpp"
#include "JamCliValuesUpdate.hpp"

#include <cstdint>
#include <vector>

namespace wowgm::protocol::world::packets
{
    using namespace wowgm::game::structures;

    enum class UpdateType : std::uint8_t
    {
        Values         = 0,
        CreateObject1  = 1,
        CreateObject2  = 2,
        DestroyObjects = 3
    };

    struct CClientObjCreate
    {
        CClientObjCreate() : GUID(), Movement(), Values() { }

        UpdateType UpdateType;
        ObjectGuid GUID;
        CMovementStatus Movement;
        JamCliValuesUpdate Values;
    };

    struct ClientUpdateObject final : public ServerPacket
    {
        ClientUpdateObject(WorldPacket&& packet);

        void Read() override;

        std::uint16_t MapID;
        std::vector<CClientObjCreate> Updates;
        std::vector<ObjectGuid> DestroyObjects;
    };
}
