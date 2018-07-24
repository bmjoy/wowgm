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

    void ClientServices::AsyncConnect(std::string username, std::string password)
    {
        AsyncConnect(username, password, "127.0.0.1", 3724);
    }

    void ClientServices::AsyncConnect(std::string username, std::string password, const std::string& realmAddress, std::int32_t port /* = 3724 */)
    {
        ip::tcp::endpoint authEndpoint(ip::make_address(realmAddress), port);

        if (_authSocket)
        {
            _authSocket->CloseSocket();
            _authSocket.reset();
        }

        _authSocket = _socketUpdater->Create<AuthSocket>(_socketUpdater->GetContext());
        _authSocket->Connect(authEndpoint);
        _authSocket->SendAuthChallenge(username, password);
    }

    bool ClientServices::IsConnected()
    {
        return _authSocket->IsOpen();
    }

    void ClientServices::UpdateIdentificationStatus(AuthCommand /* authCommand */, AuthResult result)
    {
        _authResult = result;
    }

    void ClientServices::SetRealmInfo(std::vector<AuthRealmInfo> realmInfo)
    {
        _realmInfos = std::move(realmInfo);
    }

    AuthRealmInfo* ClientServices::GetRealmInfo(std::uint32_t index)
    {
        if (_realmInfos.is_initialized())
            return &(_realmInfos.get()[index]);

        return nullptr;
    }

    std::uint32_t ClientServices::GetAvailableRealmCount()
    {
        if (_realmInfos.is_initialized())
            return _realmInfos->size();

        return 0u;
    }


    bool ClientServices::IsInWorld()
    {
        return !!_authSocket && _authResult == LOGIN_OK;
    }
}
