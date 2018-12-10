#pragma once
#include "Opcodes.hpp"
#include <cstdint>

#include "WorldPacket.hpp"

namespace wowgm::protocol::world::packets
{
    using WorldPacket = wowgm::protocol::world::WorldPacket;

    class Packet
    {
            Packet(Packet&&) = delete;
            Packet(const Packet&&) = delete;

        public:
            Packet(WorldPacket&& packet);
            virtual ~Packet() = default;

            virtual WorldPacket const* Write() = 0;
            virtual void Read() = 0;

            WorldPacket const* GetPacket() const;
            size_t GetSize() const;
            uint32_t GetConnectionId() const;
            Opcode GetOpcode() const;

        protected:
            WorldPacket _worldPacket;
    };

    class ServerPacket : public Packet
    {
        public:
            ServerPacket(WorldPacket&& packet);
            ~ServerPacket();

            WorldPacket const* Write() override final;

            void Clear();
            WorldPacket&& Move();
    };

    class ClientPacket : public Packet
    {
        public:
            ClientPacket(Opcode opcode, size_t initialSize = 200, uint32_t connectionId = 0);
            ~ClientPacket();

            void Read() override final;
            bool IsWritten() const;
    };

    class EmptyClientPacket : public ClientPacket
    {
        public:
            EmptyClientPacket(Opcode opcode) : ClientPacket(opcode, 0)
            {

            }

            WorldPacket const* Write() override final { return &_worldPacket; };
            bool IsWritten() const { return true; }
    };
}