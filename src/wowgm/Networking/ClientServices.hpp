#pragma once

#include "AuthCommand.hpp"
#include "AuthResult.hpp"

#include <string>
#include <cstdint>
#include <memory>
#include <vector>

#include <boost/optional.hpp>

namespace wowgm::threading
{
    class SocketManager;
}

namespace wowgm::networking
{
    namespace authentification
    {
        class AuthSocket;
        struct AuthRealmInfo;
    }

    using namespace wowgm::threading;
    using namespace authentification;

    class ClientServices
    {
        ClientServices();

    public:
        static ClientServices* instance();

        void AsyncConnect(std::string username, std::string password);
        void AsyncConnect(std::string username, std::string password, const std::string& realmAddress, std::int32_t port = 3724);

        bool IsConnected();

        void UpdateIdentificationStatus(AuthCommand authCommand, AuthResult result);

    public: /* Realms */
        void SetRealmInfo(std::vector<AuthRealmInfo> realmInfo);

        AuthRealmInfo* GetRealmInfo(std::uint32_t index);
        std::uint32_t GetAvailableRealmCount();

    public: /* World */

        bool IsInWorld();

    private:
        std::shared_ptr<SocketManager> _socketUpdater;
        std::shared_ptr<AuthSocket> _authSocket;
        bool _isConnected;

        boost::optional<std::vector<AuthRealmInfo>> _realmInfos;
    };

}

#define sClientServices wowgm::networking::ClientServices::instance()