#include "AuthSocket.hpp"
#include "AuthCommand.hpp"
#include "LogonChallenge.hpp"
#include "AuthPacket.hpp"
#include "AuthResult.hpp"

#include <iostream>

AuthSocket::AuthSocket(asio::io_context& io_context) : Socket(io_context)
{
    InitializeHandlers();
}

void AuthSocket::InitializeHandlers()
{
    _packetHandlers[AUTH_LOGON_CHALLENGE] = { sizeof(AuthLogonChallenge), &AuthSocket::HandleAuthChallenge };
}

void AuthSocket::ReadHandler()
{
    while (_readBuffer.GetActiveSize())
    {
        AuthCommand command = (AuthCommand)_readBuffer.GetReadPointer()[0];

        auto itr = _packetHandlers.find(command);
        if (itr == _packetHandlers.end())
        {
            _readBuffer.Reset();
            break;
        }

        // check if available size is enough
        if (_readBuffer.GetActiveSize() < itr->second.size)
            break;

        if (!(*this.*itr->second.handler)())
        {
            CloseSocket();
            return;
        }

        _readBuffer.ReadCompleted(itr->second.size);
    }
}


void AuthSocket::SendAuthChallenge(std::string&& username, std::string&& password, std::string&& platform, std::string&& operatingSystem, std::string&& countryCode, std::string&& gameCode)
{
    _username = std::move(username);
    _password = std::move(password);

    AuthPacket<LogonChallenge> command(this->shared_from_this(), AUTH_LOGON_CHALLENGE);
    command.GetData()->Error = 6;
    command.GetData()->Size = std::uint16_t(30 + _username.length());
    command.GetData()->Game = gameCode;
    command.GetData()->Version[0] = 4;
    command.GetData()->Version[1] = 3;
    command.GetData()->Version[2] = 4;
    command.GetData()->Build = 15595;
    command.GetData()->Platform = platform;
    command.GetData()->OS = operatingSystem;
    command.GetData()->CountryCode = countryCode;
    command.GetData()->timeZoneBias = 0x3C;
    command.GetData()->IP = *reinterpret_cast<std::uint32_t*>(GetLocalEndpoint().address().to_v4().to_bytes().data());
    command.GetData()->Name.Length = std::uint8_t(_username.length());
    command += _username;

    // Sent when out of scope
}

bool AuthSocket::HandleAuthChallenge()
{
    AuthPacket<AuthLogonChallenge> command(_readBuffer);
    AuthLogonChallenge* challenge = command.GetData();
    if (challenge->Command != AUTH_LOGON_CHALLENGE)
        return false;

    if (challenge->Error != LOGIN_OK) {
        std::cout << "[HandleAuthChallenge] Error code " << std::uint32_t(challenge->Error) << '\n';
        return false;
    }

    return true;
}
