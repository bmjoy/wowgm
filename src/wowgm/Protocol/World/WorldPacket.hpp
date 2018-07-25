#pragma once

#include "ByteBuffer.hpp"
#include <cstdint>

namespace wowgm::protocol::world
{
    class WorldPacket;

    class WorldPacket : public ByteBuffer
    {
        public:
            WorldPacket() : ByteBuffer(0), _opcode(0), _connection(0)
            {

            }

            WorldPacket(WorldPacket&& packet)
                : ByteBuffer(std::move(packet)), _opcode(packet._opcode), _connection(packet._connection)
            {
                packet._opcode = 0;
                packet._connection = 0;
            }

            WorldPacket(const WorldPacket& packet)
                : ByteBuffer(packet), _opcode(packet._opcode), _connection(packet._connection)
            {

            }

            WorldPacket(std::uint32_t opcode, MessageBuffer&& buffer, std::uint32_t connection)
                : ByteBuffer(std::move(buffer)), _opcode(opcode), _connection(connection)
            {

            }

            std::uint32_t GetOpcode() const { return _opcode; }
            std::uint32_t GetConnectionType() const { return _connection; }

        private:
            std::uint32_t _opcode;
            std::uint32_t _connection;
    };
}