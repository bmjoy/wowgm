#pragma once
#include <string>
#include <cstdint>
#include <memory>
#include <vector>
#include <boost/optional.hpp>

#include "AuthCommand.hpp"
#include "AuthResult.hpp"
#include "BigNumber.hpp"
#include "Socket.hpp"
#include "CharacterPackets.hpp"

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
        BigNumber const& GetSessionKey() const;

        void SetUsername(const std::string& username);
        std::string const& GetUsername() const;

        void SetPassword(const std::string& password);
        std::string const& GetPassword() const;

        void SetHostname(const std::string& hostname);
        std::string const& GetHostname() const;

        std::uint32_t* GetHostPort();

        BigNumber& GetPasswordHash();

        template <typename T>
        typename std::enable_if<std::is_base_of<Socket<T>, T>::value, T*>::type GetSocket()
        {
            return reinterpret_cast<T*>(_socket.get());
        }

    public: /* World */

        std::uint32_t GetCharacterCount() const;
        std::vector<world::packets::CharacterInfo> const& GetCharacters() const;
        void SetCharacters(std::vector<world::packets::CharacterInfo> const& characters);

        void EnterWorld(world::packets::CharacterInfo const& characterInfo);

        bool IsInWorld() const;

    private:
        std::shared_ptr<SocketManager> _socketUpdater;
        std::shared_ptr<BaseSocket> _socket;

        AuthResult _authResult = LOGIN_OK;

        boost::optional<std::vector<AuthRealmInfo>> _realmInfos;

        std::vector<world::packets::CharacterInfo> _characters;

        std::string _username;
        std::string _password;
        std::string _hostname;
        std::uint32_t _hostPort = 3724;

        BigNumber _sessionKey;
        boost::optional<BigNumber> _passwordHash;

        bool _isConnected;
    };

}

#define sClientServices wowgm::protocol::ClientServices::instance()
