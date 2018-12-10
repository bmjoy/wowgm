#include "PacketHeaders.hpp"
#include "WorldSocket.hpp"
#include "WorldPacketCrypt.hpp"

#include <algorithm>

namespace wowgm::protocol::world
{
    ServerPacketHeader::ServerPacketHeader() : _headerBuffer(5), _isLargePacket(false), _receivedFirstByte(false), Size(0u), Command(Opcode::NULL_OPCODE)
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
                Size = _headerBuffer.GetReadPointer()[1];

            return initializerSize == 2;
        }

        // Single out the first byte of the header
        if (!_receivedFirstByte)
        {
            _receivedFirstByte = true;

            _headerBuffer.Write(packet.GetReadPointer(), 1);
            authCrypt.DecryptRecv(_headerBuffer.GetReadPointer(), 1);

            _isLargePacket = (_headerBuffer.GetReadPointer()[0] & 0x80) != 0;

            packet.ReadCompleted(1);
            _headerBuffer.ReadCompleted(1);
        }

        auto remainderHeaderSize = std::min(packet.GetActiveSize(), size_t(_isLargePacket ? 4 : 3));
        _headerBuffer.Write(packet.GetReadPointer(), remainderHeaderSize);
        authCrypt.DecryptRecv(_headerBuffer.GetReadPointer(), remainderHeaderSize);

        _headerBuffer.ReadCompleted(remainderHeaderSize);
        packet.ReadCompleted(remainderHeaderSize);

        if (_isLargePacket)
        {
            BOOST_ASSERT(_headerBuffer.GetActiveSize() == 0);

            _headerBuffer.Reset();
            uint8_t* headerData = _headerBuffer.GetReadPointer();

            BOOST_ASSERT(_headerBuffer.GetActiveSize() == 0);
            Size = ((headerData[0] & 0x7F) << 16) | (headerData[1] << 8) | headerData[2];
            Command = Opcode((headerData[4] << 8) | headerData[3]);

            return true;
        }
        else
        {
            BOOST_ASSERT(_headerBuffer.GetActiveSize() == 0);

            _headerBuffer.Reset();
            uint8_t* headerData = _headerBuffer.GetReadPointer();

            Size = (headerData[0] << 8) | headerData[1];
            Command = Opcode((headerData[3] << 8) | headerData[2]);

            return true;
        }

        return false;
    }

    void ServerPacketHeader::Reset()
    {
        _receivedFirstByte = false;
        _isLargePacket = false;

        _headerBuffer.Reset();
        memset(_headerBuffer.GetReadPointer(), 0, _headerBuffer.GetBufferSize());

        Size = 0;
        Command = Opcode::NULL_OPCODE;
    }
}