#include "WorldSocket.hpp"
#include "WorldPacket.hpp"

#include "WorldPackets.hpp"
#include "Packet.hpp"

#include "WorldRenderer.hpp"
#include "Utils.hpp"

namespace wowgm::protocol::world
{
    using namespace wowgm::utilities;
    using namespace wowgm::game::structures;
    using namespace wowgm::game::geometry;
    using namespace packets;

    bool WorldSocket::HandleNewWorld(ClientNewWorld& packet)
    {
        // WorldRenderer::UnloadCurrentGeometry();
        WorldRenderer::SetCoordinates(packet.Position);
        WorldRenderer::SetMapID(packet.MapID);
        WorldRenderer::LoadGeometry(GeometryLoadFlags(GeometryLoadFlags::Terrain | GeometryLoadFlags::Mmaps | GeometryLoadFlags::Vmaps));

        return true;
    }
}
