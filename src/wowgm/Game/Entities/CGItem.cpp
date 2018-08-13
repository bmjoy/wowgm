#include "CGItem.hpp"

namespace wowgm::game::entities
{
    using namespace wowgm::utilities;

    CGItem::~CGItem()
    {

    }

    CGItemData const& CGItem::GetItemData() const
    {
        return *this;
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
