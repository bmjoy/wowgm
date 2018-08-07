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

    void WorldStateMgr::UpdateWorldState(std::uint32_t worldStateID, std::uint32_t value)
    {
        _worldStates[worldStateID] = value;
    }

    void WorldStateMgr::InitWorldState(std::uint32_t mapID, std::uint32_t areaID, std::uint32_t zoneID)
    {
        _mapID = mapID;
        _areaID = areaID;
        _zoneID = zoneID;
    }
}
