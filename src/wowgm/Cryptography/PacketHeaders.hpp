#pragma once

#include "MessageBuffer.hpp"
#include "Opcodes.hpp"
#include <cstdint>

namespace wowgm::protocol::world
{
    class WorldSocket;
    class WorldPacketCrypt;

    struct ServerPacketHeader
    {
        uint16_t Size;
        Opcode Command;

        ServerPacketHeader();

        bool Read(WorldSocket* socket, WorldPacketCrypt& authCrypt, bool initialized);
        void Reset();

        enum { data_size = 5, size_size = 3, opcode_size = 2 };

    private:
        MessageBuffer _headerBuffer;
        bool _isLargePacket;
        bool _receivedFirstByte;
    };

    #pragma pack(push, 1)
    struct ClientPacketHeader
    {
        ClientPacketHeader(uint16_t size, Opcode opcode) : Size(size), Command(opcode)
        {
            std::swap(Data[0], Data[1]);
        }

        union
        {
            uint8_t Data[6];
            struct {
                uint16_t Size;
                Opcode Command;
            };
        };

        enum { data_size = 6, size_size = 2, opcode_size = 4 };

        bool IsValidSize() const { return Size >= 4 && Size < 10240; }
    };
    #pragma pack(pop)
}