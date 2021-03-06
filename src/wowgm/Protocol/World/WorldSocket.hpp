#pragma once

#include "Socket.hpp"
#include "WorldPacket.hpp"
#include "WorldPacketCrypt.hpp"
#include "MessageBuffer.hpp"
#include "PacketHeaders.hpp"

#include <shared/cryptography/BigNumber.hpp>
#include <shared/cryptography/SHA1.hpp>
#include <shared/threading/MPSCQueue.hpp>

#include <boost/asio/detail/type_traits.hpp>
#include <boost/asio/io_context.hpp>
#include <string>
#include <cstdint>
#include <unordered_map>

#include <zlib.h>

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
        struct ClientEnumCharactersResult;
        struct ClientInitWorldStates;
        struct ClientUpdateWorldState;
        struct ClientUpdateObject;
        struct ClientDestroyObject;
        struct ClientNewWorld;
    }

    using namespace packets;
    using namespace shared::crypto;
    using namespace wowgm::utilities;

    class EncryptablePacket;

    class WorldSocket final : public Socket<WorldSocket>
    {
        WorldSocket(WorldSocket&&) = delete;
        WorldSocket(WorldSocket const&) = delete;

        typedef Socket<WorldSocket> BaseSocket;

        bool ReadDataHandler();

    public: /* Handlers */

        bool HandleAuthResponse(ClientConnectionAuthResponse& packet);
        bool HandleAuthChallenge(ClientConnectionAuthChallenge& packet);
        bool HandleEnumerateCharacterResult(ClientEnumCharactersResult& packet);

        bool HandleClientInitWorldStates(ClientInitWorldStates& packet);
        bool HandleClientUpdateWorldState(ClientUpdateWorldState& packet);

        bool HandleObjectUpdate(ClientUpdateObject& packet);
        bool HandleDestroyObject(ClientDestroyObject& packet);

        bool HandleNewWorld(ClientNewWorld& packet);

    public:

        WorldSocket(asio::io_context& service);

        void SendPacket(packets::ClientPacket& packet);
        void SendPacket(WorldPacket const* worldPacket);

        bool Update() override final;

        z_stream_s* GetDecompressionStream() { return _decompressionStream; }

        void SetNoDelay(bool disableNagle) override;

    public:

        void ReadHandler() override;
        void OnConnect() override;
        void OnClose() override;

    private:
        z_stream_s * _decompressionStream;

        BigNumber M2;

        BigNumber _encryptionSeed;
        BigNumber _decryptionSeed;

        bool _isInitialized = false;

        WorldPacketCrypt _authCrypt;

        bool _requirePacketBufferResize = true;
        ServerPacketHeader _headerBuffer;
        MessageBuffer _packetBuffer;
        MPSCQueue<EncryptablePacket> _bufferQueue;
        std::size_t _sendBufferSize;
    };

} // namespace wowgm
