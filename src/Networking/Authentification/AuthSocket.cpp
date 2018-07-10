#include "AuthSocket.hpp"
#include "AuthCommand.hpp"
#include "LogonChallenge.hpp"
#include "AuthProof.hpp"
#include "AuthPacket.hpp"
#include "AuthResult.hpp"
#include "RealmList.hpp"
#include "Utilities.hpp"
#include "SHA1.hpp"

#include "Profiler.hpp"

#include <iostream>

#include <boost/algorithm/string.hpp>

namespace wowgm::networking::authentification
{
    using namespace wowgm::cryptography;

    AuthSocket::AuthSocket(asio::io_context& io_context) : Socket(io_context)
    {
        InitializeHandlers();
    }

    void AuthSocket::InitializeHandlers()
    {
        _packetHandlers[AUTH_LOGON_CHALLENGE] = { sizeof(AuthLogonChallenge), &AuthSocket::HandleAuthChallenge };
        _packetHandlers[AUTH_LOGON_PROOF] = { sizeof(AuthLogonProof), &AuthSocket::HandleAuthProof };
        _packetHandlers[REALM_LIST] = { sizeof(AuthRealmList), &AuthSocket::HandleRealmList };
    }

    void AuthSocket::OnClose()
    {

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

            std::uint8_t* readPos = _readBuffer.GetReadPointer();
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

        boost::to_upper(_username);
        boost::to_upper(_password);

        std::cout << "[C->S] AUTH_LOGON_CHALLENGE." << std::endl;

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
        PROFILE;
        BigNumber N, A, B, a, u, x, S, salt, g, M1;

        { // Scoping the pointers so they get properly deallocated
            AuthPacket<AuthLogonChallenge> command(_readBuffer);
            AuthLogonChallenge* challenge = command.GetData();
            if (challenge->Command != AUTH_LOGON_CHALLENGE)
                return false;

            if (challenge->Error != LOGIN_OK) {
                std::cout << "[HandleAuthChallenge] Error code " << std::uint32_t(challenge->Error) << '\n';
                return false;
            }

            B.SetBinary(challenge->B, sizeof(AuthLogonChallenge::B));
            g.SetBinary(challenge->g, challenge->g_length);
            N.SetBinary(challenge->N, challenge->n_length);
            salt.SetBinary(challenge->Salt, sizeof(AuthLogonChallenge::Salt));
        }

        std::cout << "[S->C] AUTH_LOGON_CHALLENGE." << std::endl;

        // Hash the password now
        BigNumber passwordHash(CalculateSHA1(_username + ":" + _password));
        SHA1 context;
        context.UpdateBigNumbers(salt, passwordHash);
        context.Finalize();
        x.SetBinary(context);

        do {
            a.SetRand(19 * 8);
            A = g.ModExp(a, N);
        } while (A.ModExp(1, N).IsZero());

        // Compute the session key
        context.Initialize();
        context.UpdateBigNumbers(A, B);
        context.Finalize();
        u.SetBinary(context);

        BigNumber k(3);

        S = ((B + k * (N - g.ModExp(x, N))) % N).ModExp(a + (u * x), N);

        std::uint8_t sData[32];
        memcpy(sData, S.AsByteArray(32).get(), 32);
        std::uint8_t keyData[40];
        std::uint8_t temp[16];

        for (int i = 0; i < 16; ++i)
            temp[i] = sData[i * 2];

        context.Initialize();
        context.UpdateData(temp, 16);
        context.Finalize();

        for (int i = 0; i < 20; ++i)
            keyData[i * 2] = context.GetDigest()[i];

        for (int i = 0; i < 16; ++i)
            temp[i] = sData[i * 2 + 1];

        context.Initialize();
        context.UpdateData(temp, 16);
        context.Finalize();

        for (int i = 0; i < 20; ++i)
            keyData[i * 2 + 1] = context.GetDigest()[i];

        K.SetBinary(keyData, 40);

        std::uint8_t gNHash[20];
        context.Initialize();
        context.UpdateBigNumbers(N);
        context.Finalize();
        memcpy(gNHash, context.GetDigest(), context.GetLength());

        context.Initialize();
        context.UpdateBigNumbers(g);
        context.Finalize();

        for (int i = 0; i < 20; ++i)
            gNHash[i] ^= context.GetDigest()[i];

        BigNumber t3;
        t3.SetBinary(gNHash, 20);

        std::uint8_t userHash[20];
        context.Initialize();
        context.UpdateData(_username);
        context.Finalize();
        memcpy(userHash, context.GetDigest(), SHA_DIGEST_LENGTH);

        context.Initialize();
        context.UpdateBigNumbers(t3);
        context.UpdateData(userHash, 20);
        context.UpdateBigNumbers(salt, A, B, K);
        context.Finalize();
        M1.SetBinary(context);

        context.Initialize();
        context.UpdateBigNumbers(A, M1, S);
        context.Finalize();
        M2.SetBinary(context);

        std::cout << "[C->S] AUTH_LOGON_PROOF." << std::endl;
        AuthPacket<LogonProof> logonChallenge(this->shared_from_this(), AUTH_LOGON_PROOF);
        memcpy(logonChallenge.GetData()->A, A.AsByteArray(32).get(), 32);
        memcpy(logonChallenge.GetData()->M1, M1.AsByteArray(20).get(), 20);
        memset(logonChallenge.GetData()->CRC, 0, 20);
        logonChallenge.GetData()->NumberOfKeys = 0;
        logonChallenge.GetData()->SecurityFlags = 0;

        return true;
    }

    bool AuthSocket::HandleAuthProof()
    {
        AuthPacket<AuthLogonProof> command(_readBuffer);
        AuthLogonProof* proof = command.GetData();

        std::cout << "[S->C] AUTH_LOGON_PROOF." << std::endl;

        if (proof->Error == LOGIN_UNKNOWN_ACCOUNT)
        {
            std::cout << "       Unknown account or invalid password!" << std::endl;
            return false;
        }

        BigNumber serverM2;
        serverM2.SetBinary(proof->M2, sizeof(AuthLogonProof::M2));

        if (memcmp(proof->M2, M2.AsByteArray().get(), sizeof(M2)) == 0)
        {
            std::cout << "       Error during SRP6 handshake!" << std::endl;
            return false;
        }

        // Request realm list
        std::cout << "[C->S] REALM_LIST." << std::endl;
        AuthPacket<RealmList> realmList(this->shared_from_this(), REALM_LIST);
        realmList.GetData()->Data = 0;
        return true;
    }

    bool AuthSocket::HandleRealmList()
    {
        std::cout << "[S->C] REALM_LIST." << std::endl;

        {
            AuthPacket<AuthRealmList> command(_readBuffer);
            if (command.GetData()->Count == 0 || command.GetData()->Size == 0)
                return false;

            _realms.resize(command.GetData()->Count);

            _readBuffer.ReadCompleted(sizeof(AuthRealmList));

            for (std::uint8_t i = 0; i < _realms.size(); ++i)
            {
                _realms[i].Type = _readBuffer.GetReadPointer()[0];
                _realms[i].Locked = _readBuffer.GetReadPointer()[1];
                _realms[i].Flags = _readBuffer.GetReadPointer()[2];

                _readBuffer.ReadCompleted(3);

                // These call ReadCompleted!
                _readBuffer >> _realms[i].Name;
                _readBuffer >> _realms[i].Address;

                _realms[i].Population = *reinterpret_cast<float*>(_readBuffer.GetReadPointer());
                _realms[i].Load = _readBuffer.GetReadPointer()[4];
                _realms[i].Timezone = _readBuffer.GetReadPointer()[5];
                _readBuffer.ReadCompleted(4 + 1 + 1 + 4);

                if ((_realms[i].Flags & 0x04) != 0)
                {
                    memcpy(_realms[i].Version, _readBuffer.GetReadPointer(), 3);
                    _realms[i].Build = *reinterpret_cast<std::uint16_t*>(_readBuffer.GetReadPointer() + 3);

                    _readBuffer.ReadCompleted(3 + 2);
                }

                _readBuffer.ReadCompleted(2); // two trailing bytes, we don't even really bother

                std::cout << "Realm Name: '" << _realms[i].Name << "'" << std::endl;
                std::cout << "  Address: " << _realms[i].GetEndpoint() << std::endl;
                std::cout << "  Timezone: " << std::uint32_t(_realms[i].Timezone) << std::endl;
                if ((_realms[i].Flags & 0x04) != 0)
                    std::cout << "  Version: " << std::uint32_t(_realms[i].Version[0]) << "." << std::uint32_t(_realms[i].Version[1]) << "." << std::uint32_t(_realms[i].Version[2]) << "." << _realms[i].Build << std::endl;

            }

            // ReadHandlerInternal does it for us.
            _readBuffer.ReadCompleted(-sizeof(AuthRealmList));
        }

        return true;
    }

} // wowgm::networking::authentification
