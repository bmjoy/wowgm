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

    uint32_t CGItem::UpdateDescriptors(JamCliValuesUpdate& valuesUpdate)
    {
        uint32_t startOffset = CGObject::UpdateDescriptors(valuesUpdate);

        uint8_t* itemDataBase = reinterpret_cast<uint8_t*>(&GetItemData());

        auto itr = valuesUpdate.Descriptors.begin();
        while (itr != valuesUpdate.Descriptors.end())
        {
            uint32_t calculatedOffset = itr->first * 4 - startOffset;
            if (calculatedOffset > sizeof(CGItemData))
            {
                ++itr;
                continue;
            }

            *reinterpret_cast<uint32_t*>(itemDataBase + calculatedOffset) = itr->second;
            itr = valuesUpdate.Descriptors.erase(itr);
        }

        return startOffset + sizeof(CGItemData);
    }
}
