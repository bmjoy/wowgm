#pragma once

#include "ByteBuffer.hpp"
#include "Packet.hpp"

#include <cstdint>

namespace wowgm::protocol
{

    class WorldPacket : public ByteBuffer
    {
    public:
        // just container for later use
        WorldPacket() : ByteBuffer(0), m_opcode(0), _connection(0)
        {
        }

        WorldPacket(std::uint32_t opcode, size_t res = 200, std::uint32_t connection = 0) : ByteBuffer(res),
            m_opcode(opcode), _connection(connection) { }

        WorldPacket(WorldPacket&& packet) : ByteBuffer(std::move(packet)), m_opcode(packet.m_opcode), _connection(packet._connection)
        {
        }

        WorldPacket(WorldPacket const& right) : ByteBuffer(right), m_opcode(right.m_opcode), _connection(right._connection)
        {
        }

        WorldPacket& operator=(WorldPacket const& right)
        {
            if (this != &right)
            {
                m_opcode = right.m_opcode;
                _connection = right._connection;
                ByteBuffer::operator =(right);
            }

            return *this;
        }

        WorldPacket& operator=(WorldPacket&& right)
        {
            if (this != &right)
            {
                m_opcode = right.m_opcode;
                _connection = right._connection;
                ByteBuffer::operator=(std::move(right));
            }

            return *this;
        }

        WorldPacket(std::uint32_t opcode, MessageBuffer&& buffer, std::uint32_t connection) : ByteBuffer(std::move(buffer)), m_opcode(opcode), _connection(connection) { }

        void Initialize(std::uint32_t opcode, size_t newres = 200, std::uint32_t connection = 0)
        {
            clear();
            _storage.reserve(newres);
            m_opcode = opcode;
            _connection = connection;
        }

        std::uint32_t GetOpcode() const { return m_opcode; }
        void SetOpcode(std::uint32_t opcode) { m_opcode = opcode; }

        std::uint32_t GetConnection() const { return _connection; }

    protected:
        std::uint32_t m_opcode;
        std::uint32_t _connection;
    };

} // wowgm::protocol
