#pragma once
#include <string>
#include <string_view>
#include <cstdint>
#include <memory>
#include <vector>
#include <boost/optional.hpp>

#include "RealmList.hpp"
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

    using namespace wowgm::threading;
    using namespace wowgm::cryptography;
    using namespace authentification;

    class ClientServices
    {
        ClientServices();

    public:
        static ClientServices* instance();

        void AsyncConnect(std::string_view username, std::string_view password, std::string_view realmAddress, std::int32_t port = 3724);

        bool IsConnected();

    public: /* Realms */
        void UpdateIdentificationStatus(AuthCommand authCommand, AuthResult result);
        AuthResult GetAuthentificationResult();
        AuthCommand GetAuthentificationStatus();

        void SetRealmInfo(std::vector<AuthRealmInfo> realmInfo);

        AuthRealmInfo* GetRealmInfo(std::uint32_t index);
        AuthRealmInfo const& GetSelectedRealmInfo();
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

        AuthResult _authResult = LOGIN_NOT_STARTED;
        AuthCommand _authCommand = AuthCommand(-1);

        boost::optional<std::vector<AuthRealmInfo>> _realmInfos;
        AuthRealmInfo _selectedRealm;

        std::vector<world::packets::CharacterInfo> _characters;

        std::string _username;
        std::string _password;
        std::string _hostname;

        BigNumber _sessionKey;
        boost::optional<BigNumber> _passwordHash;

        bool _isConnected;
    };

}

#define sClientServices wowgm::protocol::ClientServices::instance()
