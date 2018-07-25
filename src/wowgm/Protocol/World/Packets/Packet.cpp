#include "Packet.hpp"

#include "Assert.hpp"

namespace wowgm::protocol::world::packets
{
    Packet::Packet(WorldPacket&& worldPacket) : _worldPacket(std::move(worldPacket))
    {
    }

    ServerPacket::ServerPacket(WorldPacket&& packet) : Packet(std::move(packet))
    {

    }

    ServerPacket::~ServerPacket()
    {

    }

    ClientPacket::ClientPacket(std::uint32_t opcode, size_t initialSize, std::uint32_t connectionId)
        : Packet(WorldPacket(opcode, initialSize, connectionId))
    {

    }

    ClientPacket::~ClientPacket()
    {

    }

    WorldPacket const* ServerPacket::Write()
    {
        BOOST_ASSERT_MSG(false, "Write not allowed for server packets.");
        return nullptr;
    }

    void ClientPacket::Read()
    {
        BOOST_ASSERT_MSG(false, "Read not allowed for client packets.");
    }
}