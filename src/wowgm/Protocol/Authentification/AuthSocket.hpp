#pragma once

#include "Socket.hpp"
#include "BigNumber.hpp"
#include "SHA1.hpp"

#include "RealmList.hpp"

#include <boost/asio/io_context.hpp>

#include <unordered_map>

namespace boost {
    namespace system {
        class error_code;
    }
}

namespace asio = boost::asio;
using tcp = asio::ip::tcp;

namespace wowgm::protocol::authentification
{
    using namespace wowgm::cryptography;

    class AuthSocket : public Socket<AuthSocket>
    {
        typedef Socket<AuthSocket> Base;

        AuthSocket() = delete;
        AuthSocket(const AuthSocket&) = delete;
        AuthSocket(AuthSocket&&) = delete;

    public:
        AuthSocket(asio::io_context& context);

        inline void SendAuthChallenge() {
            SendAuthChallenge(std::string("x86"), std::string("Win"), std::string("enUS"), std::string("WoW"));
        }

        void SendAuthChallenge(std::string&& platform, std::string&& operatingSystem, std::string&& countryCode, std::string&& gameCode);

        bool HandleAuthChallenge();
        bool HandleAuthProof();
        bool HandleRealmList();

        void ReadHandler() override;
        void OnConnect() override;
        void OnClose() override;

    private:
        void InitializeHandlers();

        struct AuthHandler {
            std::size_t size;
            bool (AuthSocket::*handler)();
        };

        std::unordered_map<uint8_t, AuthHandler> _packetHandlers;

        BigNumber M2;
    };
} // namespace wowgm
