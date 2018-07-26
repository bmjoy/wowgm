#pragma once

#include "AuthCommand.hpp"
#include "AuthResult.hpp"
#include "BigNumber.hpp"

#include <string>
#include <cstdint>
#include <memory>
#include <vector>

#include <boost/optional.hpp>

namespace wowgm {
    namespace threading {
        class SocketManager;
    }
}

namespace wowgm::protocol
{
    class BaseSocket;

    namespace authentification
    {
        struct AuthRealmInfo;
    }

    using namespace wowgm::threading;
    using namespace wowgm::cryptography;
    using namespace authentification;

    class ClientServices
    {
        ClientServices();

    public:
        static ClientServices* instance();

        void AsyncConnect(std::string username, std::string password);
        void AsyncConnect(std::string username, std::string password, const std::string& realmAddress, std::int32_t port = 3724);

        bool IsConnected();


    public: /* Realms */
        void UpdateIdentificationStatus(AuthCommand authCommand, AuthResult result);
        AuthResult GetAuthentificationResult();

        void SetRealmInfo(std::vector<AuthRealmInfo> realmInfo);

        AuthRealmInfo* GetRealmInfo(std::uint32_t index);
        std::uint32_t GetAvailableRealmCount();

        void ConnectToRealm(AuthRealmInfo const& realmInfo);

        void SetSessionKey(BigNumber const& K);
        BigNumber const& GetSessionKey();

        void SetUsername(const std::string& username);
        std::string const& GetUsername();

        void SetPassword(const std::string& password);
        std::string const& GetPassword();

        BigNumber& GetPasswordHash();

    public: /* World */

        bool IsInWorld();

    private:
        std::shared_ptr<SocketManager> _socketUpdater;
        std::shared_ptr<BaseSocket> _socket;

        AuthResult _authResult = LOGIN_OK;

        boost::optional<std::vector<AuthRealmInfo>> _realmInfos;

        std::string _username;
        std::string _password;

        BigNumber _sessionKey;
        boost::optional<BigNumber> _passwordHash;

        bool _isConnected;
    };

}

#define sClientServices wowgm::protocol::ClientServices::instance()
