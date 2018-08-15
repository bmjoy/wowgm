#include "CGObject.hpp"
#include "CGItem.hpp"
#include "CGContainer.hpp"
#include "CGUnit.hpp"

namespace wowgm::game::entities
{
    using namespace wowgm::game::structures;

    CGObject::CGObject(CClientObjCreate const& objCreate)
    {
    }

    CGObject::~CGObject()
    {

    }

    CGObjectData const& CGObject::GetObjectData() const
    {
        return static_cast<CGObjectData const&>(*this);
    }


    CGObjectData& CGObject::GetObjectData()
    {
        return static_cast<CGObjectData&>(*this);
    }

    CGUnit* CGObject::ToUnit()
    {
        if (GetTypeMask() & TYPEMASK_UNIT)
            return static_cast<CGUnit*>(this);
        return nullptr;
    }

    CGUnit const* CGObject::ToUnit() const
    {
        if (GetTypeMask() & TYPEMASK_UNIT)
            return static_cast<CGUnit const*>(this);
        return nullptr;
    }

    CGItem* CGObject::ToItem()
    {
        if (GetTypeMask() & TYPEMASK_ITEM)
            return static_cast<CGItem*>(this);
        return nullptr;
    }

    CGItem const* CGObject::ToItem() const
    {
        if (GetTypeMask() & TYPEMASK_ITEM)
            return static_cast<CGItem const*>(this);
        return nullptr;
    }

    CGContainer* CGObject::ToContainer()
    {
        if (GetTypeMask() & TYPEMASK_CONTAINER)
            return static_cast<CGContainer*>(this);
        return nullptr;
    }

    CGContainer const* CGObject::ToContainer() const
    {
        if (GetTypeMask() & TYPEMASK_CONTAINER)
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

    void CGObject::UpdateDescriptors(JamCliValuesUpdate const& valuesUpdate)
    {
        std::uint8_t* objectDataBase = reinterpret_cast<std::uint8_t*>(&GetObjectData());
        for (auto&& itr : valuesUpdate.Descriptors)
        {
            if (itr.first * 4 > sizeof(CGObjectData))
                continue;

            *reinterpret_cast<std::uint32_t*>(objectDataBase + itr.first * 4) = itr.second;
        }
    }
}