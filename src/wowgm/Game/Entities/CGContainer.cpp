#include "CGContainer.hpp"
#include "ObjectMgr.hpp"

namespace wowgm::game::entities
{
    using namespace wowgm::game::structures;

    CGContainer::CGContainer(CClientObjCreate const& objCreate) : CGContainerData(), CGItem(objCreate)
    {

    }

    CGContainer::~CGContainer()
    {

    }

    CGContainerData const& CGContainer::GetContainerData() const
    {
        return static_cast<CGContainerData const&>(*this);
    }

    CGContainerData& CGContainer::GetContainerData()
    {
        return static_cast<CGContainerData&>(*this);
    }

    CGItem* CGContainer::ToItem()
    {
        return static_cast<CGItem*>(this);
    }

    CGItem const* CGContainer::ToItem() const
    {
        return static_cast<CGItem const*>(this);
    }

    void CGContainer::UpdateDescriptors(JamCliValuesUpdate const& valuesUpdate)
    {
        CGItem::UpdateDescriptors(valuesUpdate);

        uint8_t* itemDataBase = reinterpret_cast<uint8_t*>(&GetContainerData());
        for (auto&& itr : valuesUpdate.Descriptors)
        {
            auto offset = itr.first * 4;

            if (offset <= sizeof(CGItemData) + sizeof(CGObjectData))
                continue;

            offset -= sizeof(CGObjectData) + sizeof(CGObjectData);
            if (offset > sizeof(CGItemData))
                continue;

            *reinterpret_cast<uint32_t*>(itemDataBase + offset) = itr.second;
        }
    }
}
