#pragma once

#include "Packet.hpp"
#include "WorldPacket.hpp"
#include "ObjectGuid.hpp"
#include "C4Vector.hpp"

#include <cstdint>
#include <vector>

namespace wowgm::protocol::world::packets
{
    using namespace wowgm::game::structures;

    struct ClientNewWorld final : public ServerPacket
    {
        ClientNewWorld(WorldPacket&& packet);

        void Read() override;

        std::uint32_t MapID = 0;
        C4Vector Position;
    };
}
