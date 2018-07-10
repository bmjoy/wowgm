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

namespace wowgm::networking::authentification
{
    using namespace wowgm::cryptography;

    class AuthSocket : public Socket<AuthSocket>
    {
        typedef Socket<AuthSocket> BaseSocket;

    public:
        AuthSocket(asio::io_context& service);

        AuthSocket(AuthSocket&&) = delete;
        AuthSocket(AuthSocket const&) = delete;

        AuthSocket& operator = (AuthSocket const&) = delete;

        inline void SendAuthChallenge(std::string& username, std::string& password) {
            SendAuthChallenge(std::forward<std::string>(username), std::forward<std::string>(password), std::string("x86"), std::string("Win"), std::string("enUS"), std::string("WoW"));
        }

        void SendAuthChallenge(std::string&& username, std::string&& password, std::string&& platform, std::string&& operatingSystem, std::string&& countryCode, std::string&& gameCode);

        bool HandleAuthChallenge();
        bool HandleAuthProof();
        bool HandleRealmList();

        AuthRealmInfo& GetRealm(std::uint32_t index) {
            return _realms[index];
        }

    protected:
        void ReadHandler() override;

        void OnClose() override;

    private:
        void InitializeHandlers();

        struct AuthHandler {
            std::size_t size;
            bool (AuthSocket::*handler)();
        };

        std::unordered_map<std::uint8_t, AuthHandler> _packetHandlers;
        std::string _username;
        std::string _password;

        BigNumber K;
        BigNumber M2;

        std::vector<AuthRealmInfo> _realms;
    };

} // namespace wowgm
