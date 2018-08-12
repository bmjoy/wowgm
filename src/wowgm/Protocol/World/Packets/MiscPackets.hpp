#pragma once

#include "Packet.hpp"

namespace wowgm::protocol::world::packets
{
    class EmptyClientPacket final : public ClientPacket
    {
        EmptyClientPacket() { }

        WorldPacket const* Write() override
        {
            return &_worldPacket;
        }
    };

    class EmptyServerPacket final : public ServerPacket
    {
        EmptyServerPacket(WorldPacket&& packet) : ServerPacket(std::move(packet)) { }

        void Read() override
        {
        }
    };

    using UserRouterClientEnableNagle = EmptyClientPacket;
}
