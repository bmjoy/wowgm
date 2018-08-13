#include "CGObject.hpp"
#include "CGItem.hpp"

namespace wowgm::game::entities
{
    using namespace wowgm::utilities;

    CGObject::CGObject(TypeMask typeMask)
    {
        _typeMask = typeMask;
    }

    CGObject::~CGObject()
    {

    }

    CGObjectData const& CGObject::GetObjectData() const
    {
        return *this;
    }

    CGItem* CGObject::ToItem()
    {
        if (_typeMask & TYPEMASK_ITEM)
            return static_cast<CGItem*>(this);
        return nullptr;
    }

    CGItem const* CGObject::ToItem() const
    {
        if (_typeMask & TYPEMASK_ITEM)
            return static_cast<CGItem const*>(this);
        return nullptr;
    }
}