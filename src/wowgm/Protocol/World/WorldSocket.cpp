#include "WorldSocket.hpp"
#include "Packet.hpp"
#include "Opcodes.hpp"
#include "PacketLogger.hpp"
#include "MiscPackets.hpp"

// Shut up.
#define BOOST_ASIO_ENABLE_BUFFER_DEBUGGING

#include <boost/asio/buffer.hpp>
#include <boost/core/demangle.hpp>
#include <typeinfo>
#include <iomanip>
#include <shared/log/log.hpp>

namespace wowgm::protocol::world
{
    using namespace wowgm::utilities;

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
        _decompressionStream = new z_stream();
        _decompressionStream->zalloc = (alloc_func)nullptr;
        _decompressionStream->zfree = (free_func)nullptr;
        _decompressionStream->opaque = (voidpf)nullptr;
        _decompressionStream->avail_in = 0;
        _decompressionStream->next_in = nullptr;
        int32_t z_res = inflateInit(_decompressionStream);
        BOOST_ASSERT_MSG_FMT(z_res == Z_OK, "Can't initialize packet decompression. Error code: %i (%s)", z_res, zError(z_res));
    }

    void WorldSocket::OnConnect()
    {
        AsyncRead();
    }

    void WorldSocket::ReadHandler()
    {
        if (!IsOpen())
            return;

        MessageBuffer& buffer = GetReadBuffer();

        while (buffer.GetActiveSize() > 0)
        {
            if (!_headerBuffer.Read(this, _authCrypt, _isInitialized))
            {
                BOOST_ASSERT_MSG(buffer.GetActiveSize() == 0, "Failed to read packet header from network, but there is some data left in the pipe!");
                break;
            }
            else if (_requirePacketBufferResize)
            {
                _packetBuffer.Resize(_headerBuffer.Size - (!_isInitialized ? 0 : ServerPacketHeader::opcode_size));
                _requirePacketBufferResize = false;
            }

            // Load data payload
            if (_packetBuffer.GetRemainingSpace() > 0)
            {
                std::size_t readDataSize = std::min(buffer.GetActiveSize(), _packetBuffer.GetRemainingSpace());
                _packetBuffer.Write(buffer.GetReadPointer(), readDataSize);
                buffer.ReadCompleted(readDataSize);

                if (_packetBuffer.GetRemainingSpace() > 0)
                {
                    // Couldn't receive the whole data this time.
                    BOOST_ASSERT_MSG(buffer.GetActiveSize() == 0, "Error while reading incoming packet payload");
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
            _requirePacketBufferResize = true;
        }

        AsyncRead();
    }


    void WorldSocket::SetNoDelay(bool disableNagle)
    {
        BaseSocket::SetNoDelay(disableNagle);

        if (!disableNagle)
        {
            packets::UserRouterClientEnableNagle enableServerNagle(Opcode::CMSG_ENABLE_NAGLE);
            SendPacket(enableServerNagle);
        }
    }

    bool WorldSocket::ReadDataHandler()
    {
        if (_isInitialized)
        {
            WorldPacket worldPacket(_headerBuffer.Command, std::move(_packetBuffer));
            bool isCompressed = worldPacket.IsCompressed();
            if (isCompressed)
                worldPacket.Decompress(GetDecompressionStream());

            LOG_INFO("[S->C] {0} (0x{0:X4}, {1} bytes)", GetOpcodeNameForLogging(worldPacket.GetOpcode()), uint32_t(worldPacket.GetOpcode()), worldPacket.size());

            PacketLogger::WriteServerPacket(&worldPacket);

            if (!sOpcodeHandler->HasHandler(worldPacket.GetOpcode()))
                return true;

            return (*sOpcodeHandler)[worldPacket.GetOpcode()]->Call(this, worldPacket);
        }
        else
        {
            std::string serverInitializer(reinterpret_cast<char const*>(_packetBuffer.GetReadPointer()), std::min(_packetBuffer.GetActiveSize(), ServerConnectionInitialize.length()));
            if (serverInitializer != ServerConnectionInitialize)
                return false;

            LOG_INFO("Connection initialized.");

            _isInitialized = true;

            MessageBuffer clientInitializer(50);
            ClientPacketHeader clientHeader(48, Opcode::NULL_OPCODE);
            clientInitializer.Write(clientHeader.Data, ClientPacketHeader::size_size);
            clientInitializer.Write(&ClientConnectionInitialize[0], 48);
            QueuePacket(std::move(clientInitializer));

            _packetBuffer.Reset();

            return true;
        }
    }

    void WorldSocket::SendPacket(packets::ClientPacket& clientPacket)
    {
        BOOST_ASSERT_MSG_FMT(clientPacket.IsWritten(), "Erorr while sending a structured packet. Did you forget to call %s::Write() ?", boost::core::demangle(typeid(clientPacket).name()).c_str());

        SendPacket(clientPacket.GetPacket());
    }

    void WorldSocket::SendPacket(WorldPacket const* worldPacket)
    {
        PacketLogger::WriteClientPacket(const_cast<WorldPacket*>(worldPacket));

        EncryptablePacket* packet = new EncryptablePacket(*worldPacket, _authCrypt.IsInitialized());
        _bufferQueue.Enqueue(packet);
    }

    bool WorldSocket::Update()
    {
        EncryptablePacket* queued;
        MessageBuffer buffer(_sendBufferSize);
        while (_bufferQueue.Dequeue(queued))
        {
            uint32_t packetSize = queued->size();
            ClientPacketHeader packetHeader(uint16_t(queued->size() + ClientPacketHeader::opcode_size), queued->GetOpcode());

            LOG_INFO("[C->S] {0} (0x{1}, {2} bytes)", GetOpcodeNameForLogging(packetHeader.Command), uint32_t(packetHeader.Command), queued->size());

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

        return Socket<WorldSocket>::Update();
    }

    void WorldSocket::OnClose()
    {

    }
}