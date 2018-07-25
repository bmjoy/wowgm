#include "ClientServices.hpp"
#include "Updater.hpp"
#include "SocketManager.hpp"
#include "AuthSocket.hpp"
#include "Logger.hpp"

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ip/address.hpp>

namespace ip = boost::asio::ip;

using namespace wowgm::protocol;
using namespace wowgm::protocol::authentification;
using namespace wowgm::threading;

namespace wowgm::protocol
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

    AuthResult ClientServices::GetAuthentificationResult()
    {
        return _authResult;
    }

    void ClientServices::SetRealmInfo(std::vector<AuthRealmInfo> realmInfo)
    {
        if (realmInfo.size() == 0)
            return;

        if (realmInfo.size() > 1)
            _realmInfos = std::move(realmInfo);
        else
            ConnectToRealm(realmInfo[0]);
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

    void ClientServices::ConnectToRealm(AuthRealmInfo const& realmInfo)
    {
        LOG_INFO << "Disconnecting from authentification server.";

        // Close auth socket
        _authSocket->CloseSocket();
        _authSocket.reset();

        LOG_DEBUG << "Connecting to realm " << realmInfo.Name << " at " << realmInfo.GetEndpoint();

        // _socketUpdater->Create<WorldSocket>(_socketUpdater->GetContext());
    }

    bool ClientServices::IsInWorld()
    {
        return !!_authSocket && _authResult == LOGIN_OK;
    }

    void ClientServices::SetSessionKey(BigNumber const& K)
    {
        _sessionKey = K;
    }

    BigNumber const& ClientServices::GetSessionKey()
    {
        return _sessionKey;
    }
}
