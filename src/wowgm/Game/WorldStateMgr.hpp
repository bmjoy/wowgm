#pragma once

#include <cstdint>
#include <unordered_map>

namespace wowgm::game
{
    class WorldStateMgr
    {
    public:
        static WorldStateMgr* instance();

    private:
        WorldStateMgr();
        ~WorldStateMgr();

    public:
        void InitWorldState(uint32_t mapID, uint32_t areaID, uint32_t zoneID);

        void UpdateWorldState(uint32_t worldStateID, uint32_t value);

    private:
        uint32_t _mapID;
        uint32_t _areaID;
        uint32_t _zoneID;

        std::unordered_map<uint32_t, uint32_t> _worldStates;
    };
}

#define sWorldStateMgr wowgm::game::WorldStateMgr::instance()
