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

        UserRouterClientAuthSession authSession;

        return true;
    }
}
