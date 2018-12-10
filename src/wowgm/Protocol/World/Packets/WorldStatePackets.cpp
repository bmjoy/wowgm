#include "WorldStatePackets.hpp"

namespace wowgm::protocol::world::packets
{
    ClientInitWorldStates::ClientInitWorldStates(WorldPacket&& packet) : ServerPacket(std::move(packet))
    {

    }

    void ClientInitWorldStates::Read()
    {
        _worldPacket >> MapID;
        _worldPacket >> ZoneID;
        _worldPacket >> AreaID;

        uint16_t updateCount;
        _worldPacket >> updateCount;

        Worldstates.resize(updateCount);
        for (auto&& itr : Worldstates)
        {
            _worldPacket >> itr.VariableID;
            _worldPacket >> itr.Value;
        }
    }

    ClientUpdateWorldState::ClientUpdateWorldState(WorldPacket&& packet) : ServerPacket(std::move(packet))
    {

    }

    void ClientUpdateWorldState::Read()
    {
        _worldPacket >> VariableID;
        _worldPacket >> Value;
        _worldPacket >> Hidden;
    }
}
