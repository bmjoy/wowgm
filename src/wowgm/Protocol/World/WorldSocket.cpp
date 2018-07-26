#include "WorldSocket.hpp"
#include "Logger.hpp"
#include "Opcodes.hpp"

#include <boost/asio/buffer.hpp>

namespace wowgm::protocol::world
{
    class EncryptablePacket : public WorldPacket
    {
        public:
            EncryptablePacket(WorldPacket const& packet, bool encrypt) : WorldPacket(packet), _encrypt(encrypt) { }

            bool NeedsEncryption() const { return _encrypt; }

        private:
            bool _encrypt;
    };

    std::string const ServerConnectionInitialize("WORLD OF WARCRAFT CONNECTION - SERVER TO CLIENT");
    constexpr static const char ClientConnectionInitialize[] = "WORLD OF WARCRAFT CONNECTION - CLIENT TO SERVER\0";

    WorldSocket::WorldSocket(asio::io_context& io_context) : Socket(io_context), _sendBufferSize(0x1000)
    {
    }

    void WorldSocket::ReadHandler()
    {
        if (!IsOpen())
            return;

        MessageBuffer& packet = GetReadBuffer();
        if (_isInitialized)
        {
            while (packet.GetActiveSize() > 0)
            {
                if (!_headerBuffer.Read(this, _authCrypt))
                {
                    BOOST_ASSERT(packet.GetActiveSize() == 0);
                    return;
                }
                else
                    _packetBuffer.Resize(_headerBuffer.Size - 2);

                // Load data payload
                if (_packetBuffer.GetRemainingSpace() > 0)
                {
                    std::size_t readDataSize = std::min(packet.GetActiveSize(), _packetBuffer.GetRemainingSpace());
                    _packetBuffer.Write(packet.GetReadPointer(), readDataSize);
                    packet.ReadCompleted(readDataSize);

                    if (_packetBuffer.GetRemainingSpace() > 0)
                    {
                        // Couldn't receive the whole data this time.
                        BOOST_ASSERT_MSG(packet.GetActiveSize() == 0, "Error while reading incoming packet payload");
                        break;
                    }
                }

                bool successfulRead = ReadDataHandler();
                if (!successfulRead)
                {
                    CloseSocket();
                    return;
                }

                _headerBuffer.Reset();
            }
        }
        else
        {
            if (packet.GetActiveSize() < 2)
                return;

            packet.ReadCompleted(2);
            if (packet.GetActiveSize() == 0)
                return;

            std::string serverInitializer(reinterpret_cast<char const*>(packet.GetReadPointer()), std::min(packet.GetActiveSize(), ServerConnectionInitialize.length()));
            if (serverInitializer != ServerConnectionInitialize)
                return;

            LOG_INFO << serverInitializer << " received. Sending " << ClientConnectionInitialize;

            _isInitialized = true;

            MessageBuffer clientInitializer(50);
            ClientPacketHeader clientHeader(48, 0);
            std::swap(clientHeader.Data[0], clientHeader.Data[1]);
            clientInitializer.Write(clientHeader.Data, 2);
            clientInitializer.Write(&ClientConnectionInitialize[0], 48);
            QueuePacket(std::move(clientInitializer));
        }
    }

    bool WorldSocket::ReadHeaderHandler()
    {
        _packetBuffer.Resize(_headerBuffer.Size - 2);
        return true;
    }

    bool WorldSocket::ReadDataHandler()
    {
        Opcode opcode = Opcode(_headerBuffer.Opcode);
        LOG_INFO << "[S->C] " << opcode;

        WorldPacket worldPacket(opcode, std::move(_packetBuffer));

        return (*sOpcodeHandler)[Opcode(_headerBuffer.Opcode)]->Call(this, worldPacket);
    }

    void WorldSocket::OnClose()
    {

    }
}