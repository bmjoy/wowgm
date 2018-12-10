#include "WorldRenderer.hpp"
#include "Utils.hpp"
#include "ADT.hpp"

#include "DBStorage.hpp"
#include "DBC.hpp"
#include "DBCStructures.hpp"
#include "ObjectMgr.hpp"
#include "VolumeIntersections.hpp"
#include "CGPlayer.hpp"
#include "CAaBox.hpp"

namespace wowgm::game::geometry
{
    using namespace wowgm::game::entities;
    using namespace wowgm::game::datastores;
    using namespace wowgm::game::structures;
    using namespace wowgm::utilities;

    WorldRenderer* WorldRenderer::Instance()
    {
        static WorldRenderer instance;
        return &instance;
    }

    void WorldRenderer::SetCoordinates(C3Vector const& position)
    {
        Instance()->_worldPosition = position;
    }

    void WorldRenderer::SetMapID(uint32_t mapID)
    {
        Instance()->_mapID = mapID;
    }

    void WorldRenderer::LoadGeometry(GeometryLoadFlags flags)
    {
        Instance()->_LoadGeometry(flags);
    }

    void WorldRenderer::_LoadGeometry(GeometryLoadFlags flags)
    {
        MapEntry const* mapEntry = Storage<MapEntry>::GetRecord(_mapID);
        if (mapEntry == nullptr)
            return;

        if (flags & GeometryLoadFlags::Terrain)
        {
            _adt = new ADT(mapEntry->Directory);
        }
    }

    void WorldRenderer::Render()
    {
        Instance()->_Render();
    }

    void WorldRenderer::_Render()
    {
        CGPlayer* localPlayer = ObjectAccessor::GetLocalPlayer();
        if (localPlayer == nullptr)
            return;

        ADT::const_iterator end;
        for (ADT::iterator itr = _adt->begin(); itr != end; ++itr)
        {
            if (!(*itr)->HasGeometry())
                continue;

            if (!VolumeIntersections::boxIntersectsSphere((*itr)->GetBoundingBox(), localPlayer->GetPosition(), _farclip))
                continue;

            (*itr)->Render();
        }
    }
}