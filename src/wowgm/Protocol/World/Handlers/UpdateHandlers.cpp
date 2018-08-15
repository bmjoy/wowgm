#include "WorldSocket.hpp"
#include "WorldPacket.hpp"

#include "UpdatePackets.hpp"
#include "Packet.hpp"

namespace wowgm::protocol::world
{
    using namespace packets;
    using namespace wowgm::cryptography;

    bool WorldSocket::HandleObjectUpdate(ClientUpdateObject& packet)
    {
        // Here we fucking go.
        return true;
    }
}
