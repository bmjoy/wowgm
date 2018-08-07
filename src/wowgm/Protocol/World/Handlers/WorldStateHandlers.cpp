#include "WorldSocket.hpp"
#include "WorldStatePackets.hpp"
#include "WorldStateMgr.hpp"

namespace wowgm::protocol::world
{
    using namespace packets;

    bool WorldSocket::HandleClientInitWorldStates(ClientInitWorldStates& packet)
    {
        sWorldStateMgr->InitWorldState(packet.MapID, packet.AreaID, packet.ZoneID);

        for (auto&& itr : packet.Worldstates)
            sWorldStateMgr->UpdateWorldState(itr.VariableID, itr.Value);

        return true;
    }

    bool WorldSocket::HandleClientUpdateWorldState(ClientUpdateWorldState& packet)
    {
        sWorldStateMgr->UpdateWorldState(packet.VariableID, packet.Value);
        return true;
    }
}