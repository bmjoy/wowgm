#include "CGItem.hpp"
#include "ObjectMgr.hpp"

namespace wowgm::game::entities
{
    using namespace wowgm::game::structures;

    CGItem::CGItem(CClientObjCreate const& objCreate) : CGItemData(), CGObject(objCreate)
    {

    }

    CGItem::~CGItem()
    {

    }

    CGItemData const& CGItem::GetItemData() const
    {
        return static_cast<CGItemData const&>(*this);
    }

    CGItemData& CGItem::GetItemData()
    {
        return static_cast<CGItemData&>(*this);
    }

    CGPlayer* CGItem::GetOwner() const
    {
        // return ObjectMgr::Instance()->GetEntity<CGPlayer>(Owner);
        return nullptr;
    }

    CGPlayer* CGItem::GetCreator() const
    {
        // return ObjectMgr::Instance()->GetEntity<CGPlayer>(Creator);
        return nullptr;
    }

    CGItem* CGItem::ToItem()
    {
        return this;
    }

    CGItem const* CGItem::ToItem() const
    {
        return this;
    }

    void CGItem::UpdateDescriptors(JamCliValuesUpdate const& valuesUpdate)
    {
        CGObject::UpdateDescriptors(valuesUpdate);

        std::uint8_t* itemDataBase = reinterpret_cast<std::uint8_t*>(&GetItemData());
        for (auto&& itr : valuesUpdate.Descriptors)
        {
            auto offset = itr.first * 4;

            if (offset <= sizeof(CGObjectData))
                continue;

            offset -= sizeof(CGObjectData);
            if (offset > sizeof(CGItemData))
                continue;

            *reinterpret_cast<std::uint32_t*>(itemDataBase + offset) = itr.second;
        }
    }
}
