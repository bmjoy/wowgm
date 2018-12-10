#include "WorldStateMgr.hpp"

namespace wowgm::game
{
    WorldStateMgr* WorldStateMgr::instance()
    {
        static WorldStateMgr _instance;
        return &_instance;
    }

    WorldStateMgr::WorldStateMgr()
    {

    }

    WorldStateMgr::~WorldStateMgr()
    {

    }

    void WorldStateMgr::UpdateWorldState(uint32_t worldStateID, uint32_t value)
    {
        _worldStates[worldStateID] = value;
    }

    void WorldStateMgr::InitWorldState(uint32_t mapID, uint32_t areaID, uint32_t zoneID)
    {
        _mapID = mapID;
        _areaID = areaID;
        _zoneID = zoneID;
    }
}
