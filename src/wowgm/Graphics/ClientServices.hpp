#pragma once

#include <string>
#include <cstdint>
#include <memory>

namespace wowgm::threading
{
    class SocketManager;
}

namespace wowgm::networking
{
    namespace authentification
    {
        class AuthSocket;
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

    private:
        std::shared_ptr<SocketManager> _socketUpdater;
        std::shared_ptr<AuthSocket> _authSocket;
        bool _isConnected;
    };

}

#define sClientServices wowgm::networking::ClientServices::instance()