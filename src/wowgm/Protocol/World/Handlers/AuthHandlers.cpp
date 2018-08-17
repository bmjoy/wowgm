#include "WorldSocket.hpp"
#include "WorldPacket.hpp"

#include "AuthentificationPackets.hpp"
#include "CharacterPackets.hpp"
#include "Packet.hpp"
#include "RealmList.hpp"

#include "BigNumber.hpp"
#include "SHA1.hpp"
#include "ClientServices.hpp"
#include "ResponseCodes.hpp"
#include "Logger.hpp"

#include <array>

namespace wowgm::protocol::world
{
    using namespace packets;
    using namespace wowgm::cryptography;
    using namespace wowgm::protocol::authentification;

    bool WorldSocket::HandleAuthResponse(ClientConnectionAuthResponse& packet)
    {
        if (packet.AuthResult != ResponseCodes::AUTH_OK)
            return false;

        EmptyClientPacket readyForAccountDataTimes(Opcode::CMSG_READY_FOR_ACCOUNT_DATA_TIMES);
        SendPacket(readyForAccountDataTimes);

        EmptyClientPacket charEnumerate(Opcode::CMSG_CHAR_ENUM);
        SendPacket(charEnumerate);
        return true;
    }

    bool WorldSocket::HandleAuthChallenge(ClientConnectionAuthChallenge& packet)
    {
        BigNumber clientSeed;
        clientSeed.SetRand(8 * 4);

        std::string username = sClientServices->GetUsername();

        SHA1 context;
        context.UpdateData(username);
        for (std::uint32_t i = 0; i < packet.Seeds.size(); ++i)
            context.UpdateData(reinterpret_cast<std::uint8_t*>(&packet.Seeds[i]), 4);

        auto checkInt64 = [](SHA1 const& sourceContext, std::uint32_t maxByte, std::uint64_t* output) -> bool {
            std::uint64_t zero = 0;

            while (true)
            {
                SHA1 context(sourceContext);
                context.UpdateData(reinterpret_cast<std::uint8_t*>(&zero), 8);
                context.Finalize();

                // Find the first non-zero bit in the digest
                std::uint32_t bitIndex = 0;
                std::uint32_t digestItr = 0;
                while (!context.GetDigest()[digestItr])
                {
                    ++digestItr;
                    bitIndex += 8;
                    if (digestItr >= SHA_DIGEST_LENGTH)
                        break;
                }
                if (digestItr < SHA_DIGEST_LENGTH)
                {
                    for (std::uint8_t currentDigestByte = context.GetDigest()[digestItr]; !(currentDigestByte & 1); ++bitIndex)
                        currentDigestByte >>= 1;
                }
                if (bitIndex >= maxByte)
                    break;

                zero = (zero + 1);
                if (!zero)
                    return false;
            }

            *output = zero;
            return true;
        };

        std::uint64_t dosResponse = 0;
        if (!checkInt64(context, packet.UnkByte, &dosResponse))
        {
            LOG_DEBUG << "Failed to validate dos response, sending zero instead.";
            dosResponse = 0;
        }
        else
            LOG_DEBUG << "Dos response calculated as " << dosResponse;

        context.Initialize();
        context.UpdateData(username);
        std::uint32_t zero = 0u;
        context.UpdateData(reinterpret_cast<std::uint8_t*>(&zero), 4);
        context.UpdateBigNumbers(clientSeed);
        context.UpdateData(reinterpret_cast<std::uint8_t*>(&packet.AuthSeed), 4);
        context.UpdateBigNumbers(sClientServices->GetSessionKey());
        context.Finalize();

        BigNumber bn;
        bn.SetBinary(context);

        UserRouterClientAuthSession authSession;
        authSession.BattlegroupID = 0;
        authSession.Build = 15595;
        authSession.ClientSeed = clientSeed.AsDword();
        memcpy(authSession.Digest.data(), context.GetDigest(), context.GetLength());
        authSession.LoginServerType = 0; // 1 Bnet, 0 grunt
        authSession.RealmID = sClientServices->GetSelectedRealmInfo().ID;
        authSession.RegionID = 0; // Used by Bnet only
        authSession.ServerID = 0; // Used by Bnet only
        authSession.UseIPv6 = false;
        authSession.DosResponse = dosResponse;
        authSession.AccountName = sClientServices->GetUsername();
        authSession.Write();
        SendPacket(authSession);

        _authCrypt.Init(sClientServices->GetSessionKey());

        SetNoDelay(true);

        return true;
    }

    bool WorldSocket::HandleEnumerateCharacterResult(ClientEnumCharactersResult& packet)
    {
        sClientServices->SetCharacters(packet.Characters);
        return true;
    }
}
