#include "WorldSocket.hpp"
#include "WorldPacket.hpp"

#include "UpdatePackets.hpp"
#include "Packet.hpp"
#include "ObjectMgr.hpp"

#include "CGObject.hpp"
#include "CGUnit.hpp"
#include "CGItem.hpp"
#include "CGContainer.hpp"

namespace wowgm::protocol::world
{
    using namespace wowgm::game::entities;
    using namespace wowgm::game::structures;

    using namespace packets;
    using namespace wowgm::cryptography;

    bool WorldSocket::HandleObjectUpdate(ClientUpdateObject& packet)
    {
        for (ObjectGuid const& itr : packet.DestroyObjects)
            ObjectAccessor::Destroy(itr);

        for (CClientObjCreate const& itr : packet.Updates)
        {
            if (itr.UpdateType == UpdateType::Values)
            {
                switch (itr.GUID.GetTypeId())
                {
                    case TYPEID_UNIT:
                        if (CGUnit* unit = ObjectAccessor::GetObject<CGUnit>(itr.GUID))
                            unit->UpdateDescriptors(itr.Values);
                        break;
                    case TYPEID_ITEM:
                        if (CGItem* item = ObjectAccessor::GetObject<CGItem>(itr.GUID))
                            item->UpdateDescriptors(itr.Values);
                        break;
                    case TYPEID_CONTAINER:
                        if (CGContainer* container = ObjectAccessor::GetObject<CGContainer>(itr.GUID))
                            container->UpdateDescriptors(itr.Values);
                        break;
                }
            }
            else
            {
                switch (itr.GUID.GetTypeId())
                {
                    case TYPEID_UNIT:
                        if (CGUnit* unit = ObjectHolder<CGUnit>::Emplace(itr))
                            unit->UpdateDescriptors(itr.Values);
                        break;
                    case TYPEID_ITEM:
                        if (CGItem* item = ObjectHolder<CGItem>::Emplace(itr))
                            item->UpdateDescriptors(itr.Values);
                        break;
                    case TYPEID_CONTAINER:
                        if (CGContainer* container = ObjectHolder<CGContainer>::Emplace(itr))
                            container->UpdateDescriptors(itr.Values);
                        break;
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
