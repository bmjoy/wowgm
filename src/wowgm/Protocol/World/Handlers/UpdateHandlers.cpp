#include "WorldSocket.hpp"
#include "WorldPacket.hpp"

#include "UpdatePackets.hpp"
#include "Packet.hpp"
#include "ObjectMgr.hpp"

#include "CGObject.hpp"
#include "CGUnit.hpp"
#include "CGItem.hpp"
#include "CGContainer.hpp"
#include "CGPlayer.hpp"

#include "WorldRenderer.hpp"

namespace wowgm::protocol::world
{
    using namespace wowgm::game::geometry;
    using namespace wowgm::game::entities;
    using namespace wowgm::game::structures;

    using namespace packets;

    bool WorldSocket::HandleObjectUpdate(ClientUpdateObject& packet)
    {
        for (ObjectGuid const& itr : packet.DestroyObjects)
            ObjectAccessor::Destroy(itr);

        for (CClientObjCreate const& itr : packet.Updates)
        {
            if (itr.UpdateType == UpdateType::Values)
            {
                if (CGObject* currentUnit = ObjectAccessor::GetObject<CGObject>(itr.GUID))
                    currentUnit->UpdateDescriptors(itr.Values);
            }
            else
            {
                switch (itr.GUID.GetTypeId())
                {
                    case TYPEID_UNIT:
                    {
                        CGUnit* unit = new CGUnit(itr);
                        ObjectHolder<CGUnit>::Insert(unit);
                        unit->UpdateDescriptors(itr.Values);
                        break;
                    }
                    case TYPEID_ITEM:
                    {
                        CGItem* item = new CGItem(itr);
                        ObjectHolder<CGItem>::Insert(item);
                        item->UpdateDescriptors(itr.Values);
                        break;
                    }
                    case TYPEID_CONTAINER:
                    {
                        CGContainer* container = new CGContainer(itr);
                        ObjectHolder<CGContainer>::Insert(container);
                        container->UpdateDescriptors(itr.Values);
                        break;
                    }
                    case TYPEID_PLAYER:
                    {
                        CGPlayer* player = new CGPlayer(itr);
                        ObjectHolder<CGPlayer>::Insert(player);
                        player->UpdateDescriptors(itr.Values);

                        if (itr.Movement.ThisIsYou)
                        {
                            WorldRenderer::SetCoordinates(itr.Movement.Position);
                            WorldRenderer::SetMapID(packet.MapID);
                            WorldRenderer::LoadGeometry(GeometryLoadFlags::Terrain | GeometryLoadFlags::Mmaps | GeometryLoadFlags::Vmaps);

                            ObjectAccessor::s_localPlayer = itr.GUID;
                        }
                        break;
                    }
                }
            }
        }

        return true;
    }

    bool WorldSocket::HandleDestroyObject(ClientDestroyObject& packet)
    {
        CGObject* object = ObjectAccessor::GetObject<CGObject>(packet.GUID);
        if (object != nullptr)
            return true;

        // if (packet.OnDeath)
        //     object->OnDeath();

        ObjectAccessor::Destroy(object);

        return true;
    }
}
