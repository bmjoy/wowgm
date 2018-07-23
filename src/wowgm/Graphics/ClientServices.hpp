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
    using namespace wowgm::threading;

    class ClientServices
    {
        ClientServices();

    public:
        static ClientServices* instance();

        void Connect(std::string username, std::string password);
        void Connect(std::string username, std::string password, const std::string& realmAddress, std::int32_t port = 3724);

    private:
        std::shared_ptr<SocketManager> _socketUpdater;
    };

}

#define sClientServices wowgm::networking::ClientServices::instance()