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

        bool Read(WorldSocket* socket, WorldPacketCrypt& authCrypt);
        void Reset();

    private:
        MessageBuffer _headerBuffer;
        bool _isLargePacket;
        bool _receivedFirstByte;
    };

    #pragma pack(push, 1)
    struct ClientPacketHeader
    {
        std::uint16_t Size;
        std::uint32_t Opcode;

        bool IsValidSize() const { return Size >= 4 && Size < 10240; }
    };
    #pragma pack(pop)
}