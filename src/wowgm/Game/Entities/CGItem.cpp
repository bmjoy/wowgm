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
}
