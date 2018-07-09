#include "AuthSocket.hpp"
#include "AuthCommand.hpp"
#include "LogonChallenge.hpp"
#include "AuthProof.hpp"
#include "AuthPacket.hpp"
#include "AuthResult.hpp"
#include "Utilities.hpp"
#include "SHA1.hpp"

#include <iostream>

#include <boost/algorithm/string.hpp>

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
    { // Scoping the pointers so they get properly deallocated
        AuthPacket<AuthLogonChallenge> command(_readBuffer);
        AuthLogonChallenge* challenge = command.GetData();
        if (challenge->Command != AUTH_LOGON_CHALLENGE)
            return false;

        if (challenge->Error != LOGIN_OK) {
            std::cout << "[HandleAuthChallenge] Error code " << std::uint32_t(challenge->Error) << '\n';
            return false;
        }

        _.B.SetBinary(challenge->B, sizeof(AuthLogonChallenge::B));
        _.g.SetBinary(challenge->g, challenge->g_length);
        _.N.SetBinary(challenge->N, challenge->n_length);
        _.salt.SetBinary(challenge->Salt, sizeof(AuthLogonChallenge::Salt));
    }

    std::cout << "[S->C] AUTH_LOGON_CHALLENGE." << std::endl;

    // Hash the password now
    crypto::BigNumber passwordHash(crypto::CalculateSHA1(_username + ":" + _password));
    crypto::SHA1 context;
    context.UpdateBigNumbers(_.salt, passwordHash);
    context.Finalize();
    _.x.SetBinary(context.GetDigest(), context.GetLength());

    std::cout << "x = " << _.x.AsHexStr() << std::endl;

    do {
        _.a.SetRand(19 * 8);
        _.A = _.g.ModExp(_.a, _.N);
    } while (_.A.ModExp(1, _.N).IsZero());

    std::cout << "s = " << _.salt.AsHexStr() << std::endl;
    std::cout << "N = " << _.N.AsHexStr() << std::endl;
    std::cout << "A = " << _.A.AsHexStr() << std::endl;

    // Compute the session key
    context.Initialize();
    context.UpdateBigNumbers(_.A, _.B);
    context.Finalize();
    _.u.SetBinary(context.GetDigest(), context.GetLength()); // OK

    crypto::BigNumber k(3);

    // S = ((B + k * (N - g.ModPow(x, N))) % N).ModPow(a + (u * x), N)
    _.S = ((_.B + k * (_.N - _.g.ModExp(_.x, _.N))) % _.N).ModExp(_.a + (_.u * _.x), _.N);

    std::uint8_t sData[32];
    memcpy(sData, _.S.AsByteArray(32).get(), 32);
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

    _.K.SetBinary(keyData, 40);

    std::cout << "u = " << _.u.AsHexStr() << std::endl;
    std::cout << "S = " << _.S.AsHexStr() << std::endl;
    std::cout << "K = " << _.K.AsHexStr() << std::endl;


    std::uint8_t gNHash[20];
    context.Initialize();
    context.UpdateBigNumbers(_.N);
    context.Finalize();
    memcpy(gNHash, context.GetDigest(), context.GetLength());

    std::cout << "T3= " << ByteArrayToHexStr(gNHash, 20) << std::endl;

    context.Initialize();
    context.UpdateBigNumbers(_.g);
    context.Finalize();

    for (int i = 0; i < 20; ++i)
        gNHash[i] ^= context.GetDigest()[i];

    std::cout << "T3= " << ByteArrayToHexStr(gNHash, 20) << std::endl;

    crypto::BigNumber t3;
    t3.SetBinary(gNHash, 20);

    std::cout << "T3= " << t3.AsHexStr() << std::endl;

    std::uint8_t userHash[20];
    context.Initialize();
    context.UpdateData(_username);
    context.Finalize();
    memcpy(userHash, context.GetDigest(), SHA_DIGEST_LENGTH);

    std::cout << "Us= " << ByteArrayToHexStr(userHash, 20) << std::endl;

    context.Initialize();
    context.UpdateBigNumbers(t3);
    context.UpdateData(userHash, 20);
    context.UpdateBigNumbers(_.salt, _.A, _.B, _.K);
    context.Finalize();
    _.M1.SetBinary(context.GetDigest(), context.GetLength());

    std::cout << "M1= " << _.M1.AsHexStr() << std::endl;

    _.M2.Initialize();
    _.M2.UpdateBigNumbers(_.A, _.M1,_.S);
    _.M2.Finalize();

    AuthPacket<AuthProof> logonChallenge(this->shared_from_this(), AUTH_LOGON_PROOF);
    memcpy(logonChallenge.GetData()->A, _.A.AsByteArray(32).get(), 32);
    memcpy(logonChallenge.GetData()->M1, _.M1.AsByteArray(20).get(), 20);
    memset(logonChallenge.GetData()->CRC, 0, 20);
    logonChallenge.GetData()->NumberOfKeys = 0;
    logonChallenge.GetData()->SecurityFlags = 0;
    return true;
}
