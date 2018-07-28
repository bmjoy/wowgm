#include "WorldSocket.hpp"
#include "Logger.hpp"
#include "Packet.hpp"
#include "Opcodes.hpp"

#include <boost/asio/buffer.hpp>
#include <boost/core/demangle.hpp>
#include <typeinfo>

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
    constexpr static const char ClientConnectionInitialize[] = "WORLD OF WARCRAFT CONNECTION - CLIENT TO SERVER";

    WorldSocket::WorldSocket(asio::io_context& io_context) : Socket(io_context), _sendBufferSize(0x1000)
    {
    }

    void WorldSocket::ReadHandler()
    {
        if (!IsOpen())
            return;

        MessageBuffer& packet = GetReadBuffer();

        while (packet.GetActiveSize() > 0)
        {
            if (!_headerBuffer.Read(this, _authCrypt, _isInitialized))
            {
                BOOST_ASSERT_MSG(packet.GetActiveSize() == 0, "Failed to read packet header from network, but there is some data left in the pipe!");
                break;
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

    bool WorldSocket::ReadDataHandler()
    {
        if (_isInitialized)
        {
            Opcode opcode = Opcode(_headerBuffer.Opcode);
            LOG_INFO << "[S->C] " << opcode;

            WorldPacket worldPacket(opcode, std::move(_packetBuffer));

            return (*sOpcodeHandler)[Opcode(_headerBuffer.Opcode)]->Call(this, worldPacket);
        }
        else
        {
            std::string serverInitializer(reinterpret_cast<char const*>(_packetBuffer.GetReadPointer()), std::min(_packetBuffer.GetActiveSize(), ServerConnectionInitialize.length()));
            if (serverInitializer != ServerConnectionInitialize)
                return false;

            LOG_INFO << "Connection initialized.";

            _isInitialized = true;

            MessageBuffer clientInitializer(50);
            ClientPacketHeader clientHeader(48, 0);
            clientInitializer.Write(clientHeader.Data, ClientPacketHeader::size_size);
            clientInitializer.Write(&ClientConnectionInitialize[0], 48);
            QueuePacket(std::move(clientInitializer));

            _packetBuffer.Reset();
            return true;
        }
    }

    void WorldSocket::SendPacket(packets::ClientPacket& clientPacket)
    {
        BOOST_ASSERT_MSG_FMT(clientPacket.IsWritten(), "Erorr while sending a structured packet. Did you forget to call %s::Write() ?", boost::core::demangle(typeid(clientPacket).name()));

        SendPacket(clientPacket.GetPacket());
    }

    void WorldSocket::SendPacket(WorldPacket const* worldPacket)
    {
        EncryptablePacket* packet = new EncryptablePacket(*worldPacket, _authCrypt.IsInitialized());
        _bufferQueue.Enqueue(packet);
    }

    void WorldSocket::Update()
    {
        EncryptablePacket* queued;
        MessageBuffer buffer(_sendBufferSize);
        while (_bufferQueue.Dequeue(queued))
        {
            std::uint32_t packetSize = queued->size();
            // if (!queued->IsCompressed())
            //     queued->Compress(GetCompressionStream());

            ClientPacketHeader packetHeader(std::uint16_t(queued->size() + ClientPacketHeader::opcode_size), std::uint32_t(queued->GetOpcode()));

            if (_isInitialized)
                std::cout << "[C->S] " << Opcode(packetHeader.Opcode);

            if (queued->NeedsEncryption())
                _authCrypt.EncryptSend(packetHeader.Data, ClientPacketHeader::data_size);

            if (buffer.GetRemainingSpace() < queued->size() + ClientPacketHeader::data_size)
            {
                QueuePacket(std::move(buffer));
                buffer.Resize(_sendBufferSize);
            }

            if (buffer.GetRemainingSpace() >= queued->size() + ClientPacketHeader::data_size)
            {
                buffer.Write(packetHeader.Data, ClientPacketHeader::data_size);
                if (!queued->empty())
                    buffer.Write(queued->contents(), queued->size());
            }
            else // Packet is larger than _sendBufferSize, in-place buffer construction
            {
                MessageBuffer packetBuffer(queued->size() + ClientPacketHeader::data_size);
                packetBuffer.Write(packetHeader.Data, ClientPacketHeader::data_size);
                if (!queued->empty())
                    packetBuffer.Write(queued->contents(), queued->size());

                QueuePacket(std::move(packetBuffer));
            }

            delete queued;
        }

        if (buffer.GetActiveSize() > 0)
            QueuePacket(std::move(buffer));

        Socket<WorldSocket>::Update();
    }

    void WorldSocket::OnClose()
    {

    }
}