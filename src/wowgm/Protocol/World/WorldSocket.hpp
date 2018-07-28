#pragma once

#include "Socket.hpp"
#include "BigNumber.hpp"
#include "SHA1.hpp"
#include "MPSCQueue.hpp"
#include "WorldPacket.hpp"
#include "WorldPacketCrypt.hpp"
#include "MessageBuffer.hpp"
#include "PacketHeaders.hpp"

#include <boost/asio/detail/type_traits.hpp>
#include <boost/asio/io_context.hpp>
#include <string>
#include <cstdint>
#include <unordered_map>

namespace boost {
    namespace system {
        class error_code;
    }
}

namespace asio = boost::asio;
using tcp = asio::ip::tcp;

namespace wowgm::protocol::world
{
    namespace packets
    {
        class ClientPacket;

        struct ClientConnectionAuthChallenge;
        struct ClientConnectionAuthResponse;
    }

    using namespace wowgm::protocol::world::packets;
    using namespace wowgm::cryptography;
    using namespace wowgm::utilities;


    class EncryptablePacket;

    class WorldSocket : public Socket<WorldSocket>
    {
        WorldSocket(WorldSocket&&) = delete;
        WorldSocket(WorldSocket const&) = delete;

        typedef Socket<WorldSocket> BaseSocket;

        bool ReadDataHandler();

    public: /* Handlers */

        bool HandleAuthResponse(ClientConnectionAuthResponse& packet);
        bool HandleAuthChallenge(ClientConnectionAuthChallenge& packet);

    public:

        WorldSocket(asio::io_context& service);

        void SendPacket(packets::ClientPacket& packet);
        void SendPacket(WorldPacket const* worldPacket);

        void Update() override final;
    protected:
        void ReadHandler() override;

        void OnClose() override;

    private:

        std::string _username;
        std::string _password;

        BigNumber M2;

        BigNumber _encryptionSeed;
        BigNumber _decryptionSeed;

        bool _isInitialized = false;

        WorldPacketCrypt _authCrypt;

        ServerPacketHeader _headerBuffer;
        MessageBuffer _packetBuffer;
        MPSCQueue<EncryptablePacket> _bufferQueue;
        std::size_t _sendBufferSize;
    };

} // namespace wowgm
