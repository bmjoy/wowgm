#pragma once

#include "MessageBuffer.hpp"
#include <cstdint>

namespace wowgm::protocol::world
{
    class WorldSocket;
    class WorldPacketCrypt;

    struct ServerPacketHeader
    {
        std::uint16_t Size;
        std::uint32_t Opcode;

        ServerPacketHeader();

        bool Read(WorldSocket* socket, WorldPacketCrypt& authCrypt, bool initialized);
        void Reset();

    private:
        MessageBuffer _headerBuffer;
        bool _isLargePacket;
        bool _receivedFirstByte;
    };

    #pragma pack(push, 1)
    struct ClientPacketHeader
    {
        ClientPacketHeader(std::uint16_t size, std::uint32_t opcode) : Size(size), Opcode(opcode)
        {
            std::swap(Data[0], Data[1]);
        }

        union
        {
            std::uint8_t Data[6];
            struct {
                std::uint16_t Size;
                std::uint32_t Opcode;
            };
        };

        enum { data_size = 6, size_size = 2, opcode_size = 4 };

        bool IsValidSize() const { return Size >= 4 && Size < 10240; }
    };
    #pragma pack(pop)
}