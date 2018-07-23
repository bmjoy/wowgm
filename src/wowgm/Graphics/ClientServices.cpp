#include "ClientServices.hpp"
#include "Updater.hpp"
#include "SocketManager.hpp"
#include "AuthSocket.hpp"

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ip/address.hpp>

namespace ip = boost::asio::ip;

using namespace wowgm::networking;
using namespace wowgm::networking::authentification;
using namespace wowgm::threading;

namespace wowgm::networking
{
    ClientServices* ClientServices::instance()
    {
        static ClientServices instance;
        return &instance;
    }

    ClientServices::ClientServices() : _socketUpdater(sUpdater->CreateUpdatable<SocketManager>())
    {
    }

    void ClientServices::Connect(std::string username, std::string password)
    {
        Connect(username, password, "127.0.0.1", 3724);
    }

    void ClientServices::Connect(std::string username, std::string password, const std::string& realmAddress, std::int32_t port /* = 3724 */)
    {
        ip::tcp::endpoint authEndpoint(ip::make_address(realmAddress), port);

        std::shared_ptr<AuthSocket> loginSocket = _socketUpdater->Create<AuthSocket>(_socketUpdater->GetContext());
        loginSocket->Connect(authEndpoint);
        loginSocket->SendAuthChallenge(username, password);
    }
}
