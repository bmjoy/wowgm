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

    CGItem* CGContainer::ToItem()
    {
        return static_cast<CGItem*>(this);
    }

    CGItem const* CGContainer::ToItem() const
    {
        return static_cast<CGItem const*>(this);
    }
}
