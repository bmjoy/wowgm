#include "WorldPackets.hpp"

namespace wowgm::protocol::world::packets
{
    ClientNewWorld::ClientNewWorld(WorldPacket&& packet) : ServerPacket(std::move(packet))
    {

    }

    void ClientNewWorld::Read()
    {
        _worldPacket >> Position.X;
        _worldPacket >> Position.O;
        _worldPacket >> Position.Z;
        _worldPacket >> MapID;
        _worldPacket >> Position.Y;
    }
}