#pragma once

#include "Packet.hpp"
#include "WorldPacket.hpp"
#include "ObjectGuid.hpp"
#include "CMovementStatus.hpp"

#include <cstdint>
#include <boost/optional.hpp>

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

    struct ClientUpdateObject final : public ServerPacket
    {
        ClientUpdateObject(WorldPacket&& packet);

        void Read() override;

        ObjectGuid GUID;
        ObjectGuid TargetGUID;
        std::vector<std::uint32_t> StopFrames;

        bool InitializeActivePlayerComponent;

        CMovementStatus Movement;
    };
}
