#pragma once

#include "ByteBuffer.hpp"
#include "Opcodes.hpp"

#include <cstdint>         // for uint32_t
#include <type_traits>     // for move
#include <vector>          // for vector

#include <zlib.h>

namespace wowgm::protocol::world
{
    class WorldPacket : public ByteBuffer
    {
    public:
        // just container for later use
        WorldPacket() : ByteBuffer(0), _opcode(Opcode::NULL_OPCODE), _connection(0)
        {
        }

        WorldPacket(Opcode opcode, size_t res = 200, uint32_t connection = 0) : ByteBuffer(res),
            _opcode(opcode), _connection(connection) { }

        WorldPacket(WorldPacket&& packet) : ByteBuffer(std::move(packet)),
            _opcode(packet._opcode), _connection(packet._connection)
        {
        }

        WorldPacket(WorldPacket const& right) : ByteBuffer(right),
            _opcode(right._opcode), _connection(right._connection)
        {
        }

        WorldPacket& operator=(WorldPacket const& right)
        {
            if (this != &right)
            {
                _opcode = right._opcode;
                _connection = right._connection;
                ByteBuffer::operator =(right);
            }

            return *this;
        }

        WorldPacket& operator=(WorldPacket&& right)
        {
            if (this != &right)
            {
                _opcode = right._opcode;
                _connection = right._connection;
                ByteBuffer::operator=(std::move(right));
            }

            return *this;
        }

        WorldPacket(Opcode opcode, MessageBuffer&& buffer, uint32_t connection = 0) : ByteBuffer(std::move(buffer)),
            _opcode(opcode), _connection(connection) { }

        void Initialize(Opcode opcode, size_t newres = 200, uint32_t connection = 0)
        {
            clear();

            _storage.reserve(newres);
            _opcode = opcode;
            _connection = connection;
        }

        Opcode GetOpcode() const { return _opcode; }
        void SetOpcode(Opcode opcode) { _opcode = opcode; }

        uint32_t GetConnection() const { return _connection; }


        bool IsCompressed() const { return (uint32_t(GetOpcode()) & 0x8000) != 0; }

        void Decompress(z_stream* decompressionStream);
        void Decompress(uint8_t* dst, uint32_t* dst_size, uint8_t* src, uint32_t src_size);

    private:
        z_stream * _decompressionStream;

    protected:

        Opcode _opcode;
        uint32_t _connection;
    };

} // wowgm::protocol
