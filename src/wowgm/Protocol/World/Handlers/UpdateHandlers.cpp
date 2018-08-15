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
        {
            switch (itr.GetTypeId())
            {
                case TYPEID_UNIT:
                    ObjectHolder<CGUnit>::Remove(itr);
                    break;
                case TYPEID_ITEM:
                    ObjectHolder<CGItem>::Remove(itr);
                    break;
                case TYPEID_CONTAINER:
                    ObjectHolder<CGContainer>::Remove(itr);
                    break;
            }
        }

        for (CClientObjCreate const& itr : packet.Updates)
        {
            if (itr.UpdateType == UpdateType::Values)
            {
                switch (itr.GUID.GetTypeId())
                {
                    case TYPEID_UNIT:
                        if (CGUnit* unit = ObjectHolder<CGUnit>::Find(itr.GUID))
                            unit->UpdateDescriptors(itr.Values);
                        break;
                    case TYPEID_ITEM:
                        if (CGItem* item = ObjectHolder<CGItem>::Find(itr.GUID))
                            item->UpdateDescriptors(itr.Values);
                        break;
                    case TYPEID_CONTAINER:
                        if (CGContainer* container = ObjectHolder<CGContainer>::Find(itr.GUID))
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
}
