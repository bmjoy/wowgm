#include "WorldPacket.hpp"
#include "Packet.hpp"
#include "Assert.hpp"

namespace wowgm::protocol::world::packets
{
    Packet::Packet(WorldPacket&& worldPacket) : _worldPacket(std::move(worldPacket))
    {
    }

    WorldPacket const* Packet::GetPacket() const
    {
        return &_worldPacket;
    }

    size_t Packet::GetSize() const
    {
        return _worldPacket.size();
    }

    std::uint32_t Packet::GetConnectionId() const
    {
        return _worldPacket.GetConnection();
    }

    void ServerPacket::Clear()
    {
        _worldPacket.clear();
    }

    WorldPacket&& ServerPacket::Move()
    {
        return std::move(_worldPacket);
    }

    Opcode Packet::GetOpcode() const
    {
        return _worldPacket.GetOpcode();
    }

    ServerPacket::ServerPacket(WorldPacket&& packet) : Packet(std::move(packet))
    {

    }

    ServerPacket::~ServerPacket()
    {

    }

    ClientPacket::ClientPacket(Opcode opcode, size_t initialSize, std::uint32_t connectionId)
        : Packet(WorldPacket(opcode, initialSize, connectionId))
    {

    }

    ClientPacket::~ClientPacket()
    {

    }

    bool ClientPacket::IsWritten() const
    {
        return _worldPacket.HasData();
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