#include "PacketHeaders.hpp"
#include "WorldSocket.hpp"
#include "WorldPacketCrypt.hpp"

#include <algorithm>

namespace wowgm::protocol::world
{
    ServerPacketHeader::ServerPacketHeader() : _headerBuffer(5), _isLargePacket(false), _receivedFirstByte(false), Size(0u), Opcode(0u)
    {

    }

    bool ServerPacketHeader::Read(WorldSocket* socket, WorldPacketCrypt& authCrypt, bool initialized)
    {
        // Size different from zero means we successfully parsed (since size should include opcode size)
        if (Size != 0)
            return true;

        MessageBuffer& packet = socket->GetReadBuffer();

        if (packet.GetActiveSize() == 0)
            return false;

        if (!initialized)
        {
            auto initializerSize = std::min(packet.GetActiveSize(), size_t(2));
            _headerBuffer.Write(packet.GetReadPointer(), initializerSize);
            packet.ReadCompleted(initializerSize);

            // This is cheating, but i don't care. The size of the handshake string fits 1 byte.
            if (_headerBuffer.GetActiveSize() == 2)
                Size = _headerBuffer.GetReadPointer()[1] + 2; // This size does include the opcode, but there isn't any, and we expect there to be one in the code immediately after.

            return initializerSize == 2;
        }

        // Single out the first byte of the header
        if (!_receivedFirstByte)
        {
            _receivedFirstByte = true;

            _headerBuffer.Write(packet.GetReadPointer(), 1);

            authCrypt.DecryptRecv(_headerBuffer.GetReadPointer(), 1);
            _isLargePacket = _headerBuffer.GetReadPointer()[0] & 0x80;

            if (_isLargePacket)
            {
                _headerBuffer.GetReadPointer()[0] &= 0x7F;
            }

            packet.ReadCompleted(1);

            // Needed for cases where we get bytes on the wire 1 by 1
            _headerBuffer.ReadCompleted(1);
        }

        auto remainderHeaderSize = std::min(packet.GetActiveSize(), size_t(_isLargePacket ? 4 : 3));
        _headerBuffer.Write(packet.GetReadPointer(), remainderHeaderSize);
        authCrypt.DecryptRecv(_headerBuffer.GetReadPointer(), remainderHeaderSize);
        _headerBuffer.ReadCompleted(remainderHeaderSize);
        packet.ReadCompleted(remainderHeaderSize);

        if (_headerBuffer.GetActiveSize() == (!_isLargePacket ? 0 : 1))
        {
            _headerBuffer.Reset();
            std::uint8_t* headerData = _headerBuffer.GetReadPointer();
            if (!_isLargePacket)
            {
                Size = ((headerData[0] << 8) | headerData[1]);
                Opcode = (headerData[3] << 8) | headerData[2];
            }
            else
            {
                Size = ((headerData[0] << 16) | (headerData[1] << 8) | headerData[2]);
                Opcode = (headerData[4] << 8) | headerData[3];
            }
            return true;
        }

        return false;
    }

    void ServerPacketHeader::Reset()
    {
        _receivedFirstByte = false;
        _isLargePacket = false;

        _headerBuffer.Reset();

        Size = 0;
    }
}