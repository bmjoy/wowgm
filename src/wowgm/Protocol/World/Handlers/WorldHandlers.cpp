#include "WorldSocket.hpp"
#include "WorldPacket.hpp"

#include "WorldPackets.hpp"
#include "Packet.hpp"

namespace wowgm::protocol::world
{
    using namespace wowgm::game::structures;

    using namespace packets;

    bool WorldSocket::HandleNewWorld(ClientNewWorld& packet)
    {
        // TerrainViewer::Initialize()->SetMapID(packet.MapID);
        // TerrainViewer::Initialize()->SetCoordinates(packet.Position);

        return true;
    }
}
