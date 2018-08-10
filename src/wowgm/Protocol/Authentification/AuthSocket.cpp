#include "AuthSocket.hpp"
#include "AuthCommand.hpp"
#include "LogonChallenge.hpp"
#include "AuthProof.hpp"
#include "AuthPacket.hpp"
#include "AuthResult.hpp"
#include "RealmList.hpp"
#include "Utils.hpp"
#include "SHA1.hpp"
#include "Logger.hpp"
#include "ClientServices.hpp"

#include <iostream>

#include <boost/algorithm/string.hpp>

namespace wowgm::protocol::authentification
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

    void AuthSocket::ReadHandler()
    {
        while (GetReadBuffer().GetActiveSize())
        {
            AuthCommand command = AuthCommand(GetReadBuffer().GetReadPointer()[0]);

            auto itr = _packetHandlers.find(command);
            if (itr == _packetHandlers.end())
            {
                BOOST_ASSERT_MSG(false, "Unable to find an authpacket handler");
                break;
            }

            // check if available size is enough
            if (GetReadBuffer().GetActiveSize() < itr->second.size)
                break;

            if (!(*this.*itr->second.handler)())
            {
                CloseSocket();
                return;
            }
        }

        AsyncRead();
    }

    void AuthSocket::OnConnect()
    {
        SendAuthChallenge();

        AsyncRead();
    }

    void AuthSocket::OnClose()
    {

    }

    void AuthSocket::SendAuthChallenge(std::string&& platform, std::string&& operatingSystem, std::string&& countryCode, std::string&& gameCode)
    {
        AuthPacket<LogonChallenge> command(this->shared_from_this(), AUTH_LOGON_CHALLENGE);
        command.GetData()->Error = 6;
        command.GetData()->Size = std::uint16_t(30 + sClientServices->GetUsername().length());
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
        command.GetData()->Name.Length = std::uint8_t(sClientServices->GetUsername().length());
        command += sClientServices->GetUsername();

        // Sent when out of scope
    }

    bool AuthSocket::HandleAuthChallenge()
    {
        BigNumber B, g, N, salt;

        { // Scoping the pointers so they get properly deallocated
            AuthPacket<AuthLogonChallenge> command(GetReadBuffer());
            AuthLogonChallenge* challenge = command.GetData();
            if (challenge->Command != AUTH_LOGON_CHALLENGE)
                return false;

            if (challenge->Error != LOGIN_OK)
            {
                sClientServices->UpdateIdentificationStatus(AUTH_LOGON_CHALLENGE, AuthResult(challenge->Error));
                return false;
            }

            B.SetBinary(challenge->B, sizeof(AuthLogonChallenge::B));
            g.SetBinary(challenge->g, challenge->g_length);
            N.SetBinary(challenge->N, challenge->n_length);
            salt.SetBinary(challenge->Salt, sizeof(AuthLogonChallenge::Salt));
        }

        BigNumber A, a, u, x, S, M1, K;

        LOG_INFO << "[S->C] AUTH_LOGON_CHALLENGE.";

        BigNumber& passwordHash = sClientServices->GetPasswordHash();

        SHA1 context;
        context.UpdateBigNumbers(salt);
        context.UpdateBigNumbers(passwordHash);
        context.Finalize();
        x.SetBinary(context);

        // LOG_DEBUG << "x = " << x.AsHexStr();

        do {
            a.SetRand(19 * 8);
            A = g.ModExp(a, N);
        } while (A.ModExp(1, N).IsZero());

        // Compute the session key
        context.Initialize();
        context.UpdateBigNumbers(A);
        context.UpdateBigNumbers(B);
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
        sClientServices->SetSessionKey(K);

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
        context.UpdateData(sClientServices->GetUsername());
        context.Finalize();
        memcpy(userHash, context.GetDigest(), SHA_DIGEST_LENGTH);

        context.Initialize();
        context.UpdateBigNumbers(t3);
        context.UpdateData(userHash, 20);
        context.UpdateBigNumbers(salt);
        context.UpdateBigNumbers(A);
        context.UpdateBigNumbers(B);
        context.UpdateBigNumbers(K);
        context.Finalize();
        M1.SetBinary(context);

        context.Initialize();
        context.UpdateBigNumbers(A, M1, S);
        context.Finalize();
        M2.SetBinary(context);

        LOG_INFO << "[C->S] AUTH_LOGON_PROOF.";
        // LOG_DEBUG << "s = " << salt.AsHexStr();
        // LOG_DEBUG << "N = " << N.AsHexStr();
        // LOG_DEBUG << "A = " << A.AsHexStr();
        // LOG_DEBUG << "u = " << u.AsHexStr();
        // LOG_DEBUG << "S = " << S.AsHexStr();
        // LOG_DEBUG << "K = " << K.AsHexStr();
        // LOG_DEBUG << "M1= " << M1.AsHexStr();
        // LOG_DEBUG << "M2= " << M2.AsHexStr();
        // LOG_DEBUG << "Us= " << wowgm::utilities::ByteArrayToHexStr(userHash, 20);
        // LOG_DEBUG << "T3= " << t3.AsHexStr();

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
        AuthPacket<AuthLogonProof> command(GetReadBuffer());
        AuthLogonProof* proof = command.GetData();

        LOG_INFO << "[S->C] AUTH_LOGON_PROOF.";

        if (proof->Error != LOGIN_OK)
        {
            sClientServices->UpdateIdentificationStatus(AUTH_LOGON_PROOF, AuthResult(proof->Error));
            return false;
        }

        if (memcmp(proof->M2, M2.AsByteArray().get(), sizeof(proof->M2)) == 0)
        {
            sClientServices->UpdateIdentificationStatus(AUTH_LOGON_PROOF, LOGIN_INVALID_SRP6);
            return false;
        }

        // Request realm list
        LOG_INFO << "[C->S] REALM_LIST.";
        AuthPacket<RealmList> realmList(this->shared_from_this(), REALM_LIST);
        realmList.GetData()->Data = 0;
        return true;
    }

    bool AuthSocket::HandleRealmList()
    {
        LOG_INFO << "[S->C] REALM_LIST.";

        AuthPacket<AuthRealmList> command(GetReadBuffer());
        if (command.GetData()->Count == 0 || command.GetData()->Size == 0)
            return false;

        std::vector<AuthRealmInfo> realmList;
        realmList.resize(command.GetData()->Count);

        for (std::uint8_t i = 0; i < realmList.size(); ++i)
        {
            realmList[i].Type = GetReadBuffer().GetReadPointer()[0];
            realmList[i].Locked = GetReadBuffer().GetReadPointer()[1];
            realmList[i].Flags = GetReadBuffer().GetReadPointer()[2];

            GetReadBuffer().ReadCompleted(3);

            // These call ReadCompleted!
            GetReadBuffer().ReadCString(realmList[i].Name, 0x100);
            GetReadBuffer().ReadCString(realmList[i].Address, 0x40);

            realmList[i].Population = *reinterpret_cast<float*>(GetReadBuffer().GetReadPointer());
            realmList[i].Load = GetReadBuffer().GetReadPointer()[4];
            realmList[i].Timezone = GetReadBuffer().GetReadPointer()[5];
            realmList[i].ID = GetReadBuffer().GetReadPointer()[6];
            GetReadBuffer().ReadCompleted(4 + 1 + 1 + 1);

            if ((realmList[i].Flags & 0x04) != 0)
            {
                memcpy(realmList[i].Version, GetReadBuffer().GetReadPointer(), 3);
                realmList[i].Build = *reinterpret_cast<std::uint16_t*>(GetReadBuffer().GetReadPointer() + 3);

                GetReadBuffer().ReadCompleted(3 + 2);
            }
        }

        sClientServices->SetRealmInfo(realmList);

        return true;
    }

} // wowgm::protocol::authentification
