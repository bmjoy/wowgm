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
        void InitWorldState(std::uint32_t mapID, std::uint32_t areaID, std::uint32_t zoneID);

        void UpdateWorldState(std::uint32_t worldStateID, std::uint32_t value);

    private:
        std::uint32_t _mapID;
        std::uint32_t _areaID;
        std::uint32_t _zoneID;

        std::unordered_map<std::uint32_t, std::uint32_t> _worldStates;
    };
}

#define sWorldStateMgr wowgm::game::WorldStateMgr::instance()
