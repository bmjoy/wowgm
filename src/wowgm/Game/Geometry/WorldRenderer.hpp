#pragma once

#include "C3Vector.hpp"

#include <cstdint>

namespace wowgm::game::geometry
{
    class ADT;

    using namespace wowgm::game::structures;

    enum GeometryLoadFlags : uint32_t
    {
        Terrain = 0x01,
        Vmaps = 0x02,
        Mmaps = 0x04
    };

    class WorldRenderer
    {
    public:
        static void SetCoordinates(C3Vector const& position);
        static void SetMapID(uint32_t mapID);

        static void LoadGeometry(GeometryLoadFlags flags);

        static void Render();

    private:
        static WorldRenderer* Instance();
        void _LoadGeometry(GeometryLoadFlags flags);

        void _Render();

    private:
        C3Vector _worldPosition;
        uint32_t _mapID = -1;

        ADT* _adt = nullptr;

        float _farclip = 1000.0f;

    };
}
