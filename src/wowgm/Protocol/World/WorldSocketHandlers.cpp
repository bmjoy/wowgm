#include "WorldSocket.hpp"
#include "WorldPacket.hpp"

#include "AuthentificationPackets.hpp"
#include "CharacterPackets.hpp"
#include "Packet.hpp"

#include "BigNumber.hpp"
#include "SHA1.hpp"
#include "ClientServices.hpp"
#include "ResponseCodes.hpp"

namespace wowgm::protocol::world
{
    using namespace packets;
    using namespace wowgm::cryptography;

    bool WorldSocket::HandleAuthResponse(ClientConnectionAuthResponse& packet)
    {
        if (packet.AuthResult != ResponseCodes::AUTH_OK)
            return false;

        EmptyClientPacket charEnumerate(Opcode::CMSG_CHAR_ENUM);
        SendPacket(charEnumerate);
        return true;
    }

    bool WorldSocket::HandleAuthChallenge(ClientConnectionAuthChallenge& packet)
    {
        BigNumber clientSeed;
        clientSeed.SetRand(8 * 4);

        std::array<std::uint8_t, 4> zero{ { 0, 0, 0, 0} };
        BigNumber serverSeed(packet.AuthSeed);

        std::string username = sClientServices->GetUsername();

        SHA1 context;
        context.UpdateData(username);
        context.UpdateData(zero);
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
        authSession.LoginServerType = 0;
        authSession.RealmID = 0; // FIXME
        authSession.RegionID = 0; // FIXME
        authSession.ServerID = 0; // FIXME
        authSession.UseIPv6 = false;
        authSession.AccountName = sClientServices->GetUsername();
        authSession.Write();
        SendPacket(authSession);

        _authCrypt.Init(sClientServices->GetSessionKey());

        return true;
    }

    bool WorldSocket::HandleEnumerateCharacterResult(ClientEnumCharactersResult& packet)
    {
        sClientServices->SetCharacters(packet.Characters);
        return true;
    }
}
