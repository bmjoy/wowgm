#include "ClientServices.hpp"
#include "Updater.hpp"
#include "SocketManager.hpp"
#include "AuthSocket.hpp"
#include "WorldSocket.hpp"
#include "Logger.hpp"
#include "SHA1.hpp"

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ip/address.hpp>

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

    void ClientServices::AsyncConnect(std::string username, std::string password)
    {
        AsyncConnect(username, password, GetHostname(), *GetHostPort());
    }

    void ClientServices::AsyncConnect(std::string username, std::string password, const std::string& realmAddress, std::int32_t port /* = 3724 */)
    {
        ip::tcp::endpoint authEndpoint(ip::make_address(realmAddress), port);

        if (_socket)
        {
            _socket->CloseSocket();
            _socket.reset();
        }

        auto authSocket = _socketUpdater->Create<AuthSocket>(_socketUpdater->GetContext());

        _socket = authSocket;

        authSocket->Connect(authEndpoint);
        authSocket->SendAuthChallenge(username, password);
    }

    bool ClientServices::IsConnected()
    {
        return _socket->IsOpen();
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
        // Close auth socket
        _socket->CloseSocket();
        _socket.reset();

        LOG_INFO << "Disconnecting from authentification server.";
        LOG_INFO << "Connecting to realm " << realmInfo.Name << " at " << realmInfo.GetEndpoint();

        _socket = _socketUpdater->Create<WorldSocket>(_socketUpdater->GetContext());
        _socket->Connect(realmInfo.GetEndpoint());
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
        _passwordHash = boost::none;
    }

    std::string const& ClientServices::GetUsername() const
    {
        return _username;
    }

    void ClientServices::SetPassword(const std::string& password)
    {
        _password = password;
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

    std::uint32_t* ClientServices::GetHostPort()
    {
        return &_hostPort;
    }

    BigNumber& ClientServices::GetPasswordHash()
    {
        if (_passwordHash.is_initialized())
            return _passwordHash.get();

        SHA1 context;
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

    std::uint32_t ClientServices::GetCharacterCount() const
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
        _selectedCharacter = std::move(characterInfo);

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
