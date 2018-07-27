#include "WorldSocket.hpp"
#include "WorldPacket.hpp"
#include "AuthentificationPackets.hpp"
#include "BigNumber.hpp"
#include "SHA1.hpp"
#include "ClientServices.hpp"

namespace wowgm::protocol::world
{
    using namespace packets;
    using namespace wowgm::cryptography;

    bool WorldSocket::HandleAuthChallenge(ClientConnectionAuthChallenge& packet)
    {
        BigNumber clientSeed;
        clientSeed.SetRand(8 * 4);

        BigNumber zero(0u);
        BigNumber serverSeed(packet.AuthSeed);

        std::string username = sClientServices->GetUsername();

        SHA1 context;
        context.Initialize();
        context.UpdateData(username);
        context.UpdateBigNumbers(zero, clientSeed, serverSeed);
        context.UpdateBigNumbers(sClientServices->GetSessionKey());
        context.Finalize();

        UserRouterClientAuthSession authSession;
        authSession.AccountName = sClientServices->GetUsername();
        authSession.BattlegroupID = 0;
        authSession.Build = 15595;
        authSession.ClientSeed = clientSeed.AsDword();
        memcpy(authSession.Digest.data(), context.GetDigest(), context.GetLength());
        authSession.LoginServerType = 1;
        authSession.RealmID = 0; // FIXME
        authSession.RegionID = 0; // FIXME
        authSession.ServerID = 0; // FIXME
        authSession.UseIPv6 = false;
        authSession.Write();
        SendPacket(authSession);

        return true;
    }
}
