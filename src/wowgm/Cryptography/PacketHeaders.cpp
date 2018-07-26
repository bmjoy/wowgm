#include "PacketHeaders.hpp"
#include "WorldSocket.hpp"
#include "WorldPacketCrypt.hpp"

#include <algorithm>

namespace wowgm::protocol::world
{
    ServerPacketHeader::ServerPacketHeader() : _headerBuffer(5), _isLargePacket(false), _receivedFirstByte(false), Size(0u), Opcode(0u)
    {

    }

    bool ServerPacketHeader::Read(WorldSocket* socket, WorldPacketCrypt& authCrypt)
    {
        // Size different from zero means we successfully parsed (since size should include opcode size)
        if (Size != 0)
            return true;

        MessageBuffer& packet = socket->GetReadBuffer();

        if (packet.GetActiveSize() == 0)
            return false;

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

        Size = 0;
    }
}