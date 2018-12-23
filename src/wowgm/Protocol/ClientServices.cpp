#include "ClientServices.hpp"
#include "Updater.hpp"
#include "SocketManager.hpp"
#include "AuthSocket.hpp"
#include "WorldSocket.hpp"

#include <shared/log/log.hpp>
#include <shared/cryptography/SHA1.hpp>

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ip/address.hpp>
#include <boost/algorithm/string/case_conv.hpp>

namespace ip = boost::asio::ip;

using namespace wowgm::protocol;
using namespace wowgm::protocol::world;
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

    void ClientServices::AsyncConnect(std::string_view username, std::string_view password, std::string_view realmAddress, int32_t port /* = 3724 */)
    {
        if (_socket)
        {
            _socket->CloseSocket();
            _socket.reset();
        }

        SetUsername(std::string(username));
        SetPassword(std::string(password));

        auto authSocket = _socketUpdater->Create<AuthSocket>(_socketUpdater->GetContext());
        authSocket->Connect(realmAddress, port);
        _socket = authSocket;
    }

    bool ClientServices::IsConnected()
    {
        return _socket->IsOpen();
    }

    void ClientServices::UpdateIdentificationStatus(AuthCommand authCommand, AuthResult result)
    {
        _authResult = result;
        _authCommand = authCommand;
    }

    AuthResult ClientServices::GetAuthentificationResult()
    {
        return _authResult;
    }

    AuthCommand ClientServices::GetAuthentificationStatus()
    {
        return _authCommand;
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

    AuthRealmInfo* ClientServices::GetRealmInfo(uint32_t index)
    {
        if (_realmInfos.is_initialized())
            return &(_realmInfos.get()[index]);

        return nullptr;
    }

    uint32_t ClientServices::GetAvailableRealmCount()
    {
        if (_realmInfos.is_initialized())
            return _realmInfos->size();

        return 0u;
    }

    void ClientServices::ConnectToRealm(AuthRealmInfo const& realmInfo)
    {
        // Close auth socket
        _socket->CloseSocket();
        _socket.reset();

        LOG_INFO("Disconnecting from authentification server.");
        LOG_INFO("Connecting to realm {} at {}", realmInfo.Name, realmInfo.GetEndpoint());

        auto worldSocket = _socketUpdater->Create<WorldSocket>(_socketUpdater->GetContext());
        worldSocket->Connect(realmInfo.GetEndpoint());

        _selectedRealm = std::move(realmInfo);
        _realmInfos = boost::none;

        _socket = worldSocket;
    }

    AuthRealmInfo const& ClientServices::GetSelectedRealmInfo()
    {
        return _selectedRealm;
    }

    bool ClientServices::IsInWorld() const
    {
        return !!_socket && _authResult == LOGIN_OK;
    }

    void ClientServices::SetSessionKey(BigNumber const& K)
    {
        _sessionKey = K;
    }

    BigNumber const& ClientServices::GetSessionKey() const
    {
        return _sessionKey;
    }

    void ClientServices::SetUsername(const std::string& username)
    {
        _username = username;

        boost::to_upper(_username);

        _passwordHash = boost::none;
    }

    std::string const& ClientServices::GetUsername() const
    {
        return _username;
    }

    void ClientServices::SetPassword(const std::string& password)
    {
        _password = password;

        boost::to_upper(_password);

        _passwordHash = boost::none;
    }

    std::string const& ClientServices::GetPassword() const
    {
        return _password;
    }

    void ClientServices::SetHostname(const std::string& hostname)
    {
        _hostname = hostname;
    }

    std::string const& ClientServices::GetHostname() const
    {
        return _hostname;
    }

    BigNumber& ClientServices::GetPasswordHash()
    {
        if (_passwordHash.is_initialized())
            return _passwordHash.get();

        shared::crypto::SHA1 context;
        context.Initialize();
        context.UpdateData(_username);
        context.UpdateData(':');
        context.UpdateData(_password);
        context.Finalize();

        BigNumber bn;
        bn.SetBinary(context);
        _passwordHash = bn;

        return _passwordHash.get();
    }

    uint32_t ClientServices::GetCharacterCount() const
    {
        return _characters.size();
    }

    std::vector<world::packets::CharacterInfo> const& ClientServices::GetCharacters() const
    {
        return _characters;
    }

    void ClientServices::SetCharacters(std::vector<world::packets::CharacterInfo> const& characters)
    {
        _characters = std::move(characters);
    }

    void ClientServices::EnterWorld(world::packets::CharacterInfo const& characterInfo)
    {
        // Cleanup some state
        _characters.clear();
        _realmInfos = boost::none;

        if (WorldSocket* socket = GetSocket<WorldSocket>())
        {
            UserClientPlayerLogin playerLogin;
            playerLogin.GUID = characterInfo.GUID;

            socket->SendPacket(playerLogin.Write());
        }
    }
}
