#include "WorldSocket.hpp"

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

    std::string const WorldSocket::ServerConnectionInitialize("WORLD OF WARCRAFT CONNECTION - SERVER TO CLIENT\0");
    std::string const WorldSocket::ClientConnectionInitialize("WORLD OF WARCRAFT CONNECTION - CLIENT TO SERVER\0");

    WorldSocket::WorldSocket(asio::io_context& io_context) : Socket(io_context), _sendBufferSize(0x1000)
    {
    }


    bool WorldSocket::Connect(asio::ip::tcp::endpoint const& endpoint)
    {
        BaseSocket::Connect(endpoint);
        if (!IsOpen())
            return false;

        auto buffer = boost::asio::buffer(&ClientConnectionInitialize[0], ClientConnectionInitialize.length());
        return SendRawPacket(buffer);
    }

    void WorldSocket::ReadHandler()
    {
        if (!IsOpen())
            return;

        MessageBuffer& packet = GetReadBuffer();
        while (packet.GetActiveSize() > 0)
        {
            if (!_headerBuffer.Read(this, _authCrypt))
            {
                BOOST_ASSERT(packet.GetActiveSize() == 0);
                return;
            }

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

    bool WorldSocket::ReadHeaderHandler()
    {
        _packetBuffer.Resize(_headerBuffer.Size - 2);
        return true;
    }

    bool WorldSocket::ReadDataHandler()
    {
        return true;
    }

    void WorldSocket::OnClose()
    {

    }
}