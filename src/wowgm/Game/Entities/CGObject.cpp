#include "CGObject.hpp"
#include "CGItem.hpp"
#include "CGContainer.hpp"
#include "CGUnit.hpp"

namespace wowgm::game::entities
{
    CGObject::CGObject(TypeMask typeMask)
    {
        _typeMask = typeMask;
    }

    CGObject::~CGObject()
    {

    }

    CGObjectData const& CGObject::GetObjectData() const
    {
        return static_cast<CGObjectData const&>(*this);
    }

    CGUnit* CGObject::ToUnit()
    {
        if (_typeMask & TYPEMASK_UNIT)
            return static_cast<CGUnit*>(this);
        return nullptr;
    }

    CGUnit const* CGObject::ToUnit() const
    {
        if (_typeMask & TYPEMASK_UNIT)
            return static_cast<CGUnit const*>(this);
        return nullptr;
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

    CGContainer* CGObject::ToContainer()
    {
        if (_typeMask & TYPEMASK_CONTAINER)
            return static_cast<CGContainer*>(this);
        return nullptr;
    }

    CGContainer const* CGObject::ToContainer() const
    {
        if (_typeMask & TYPEMASK_CONTAINER)
            return static_cast<CGContainer const*>(this);
        return nullptr;
    }

    C3Vector* CGObject::GetPosition()
    {
        return static_cast<C3Vector*>(this);
    }

    C3Vector const* CGObject::GetPosition() const
    {
        return static_cast<C3Vector const*>(this);
    }
}