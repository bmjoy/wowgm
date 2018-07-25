#pragma once

#include "WorldPacket.hpp"
#include <cstdint>

namespace wowgm::protocol::world::packets
{
    class Packet
    {
        Packet(Packet&&) = delete;
        Packet(const Packet&&) = delete;

    public:
        Packet(WorldPacket&& packet);
        virtual ~Packet() = default;

        virtual WorldPacket const* Write() = 0;
        virtual void Read() = 0;

        WorldPacket const* GetPacket() const { return &_worldPacket; }
        size_t GetSize() const { return _worldPacket.size(); }
        std::uint32_t GetConnectionId() const { return _worldPacket.GetConnection(); }

    protected:
        WorldPacket _worldPacket;
    };

    class ServerPacket : public Packet
    {
        public:
            ServerPacket(WorldPacket&& packet);
            ~ServerPacket();

            WorldPacket const* Write() override final;

            void Clear() { _worldPacket.clear(); }
            WorldPacket&& Move() { return std::move(_worldPacket); }

            std::uint32_t GetOpcode() { return _worldPacket.GetOpcode(); }
    };

    class ClientPacket : public Packet
    {
        public:
            ClientPacket(std::uint32_t opcode, size_t initialSize = 200, std::uint32_t connectionId = 0);
            ~ClientPacket();

            void Read() override final;

            std::uint32_t GetOpcode() { return _worldPacket.GetOpcode(); }
    };
}