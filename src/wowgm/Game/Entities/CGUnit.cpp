#include "CGUnit.hpp"
#include "ObjectMgr.hpp"

namespace wowgm::game::entities
{
    using namespace wowgm::game::structures;

    CGUnit::CGUnit(CClientObjCreate const& createBlock) : CGUnitData(), CGObject(createBlock)
    {
        //! TODO: This will need to be reworked later on since we don't have CGPlayer yet.
        UpdateDescriptors(createBlock.Values);
    }

    CGUnit::~CGUnit()
    {

    }

    CGUnitData const& CGUnit::GetUnitData() const
    {
        return static_cast<CGUnitData const&>(*this);
    }

    CGUnitData& CGUnit::GetUnitData()
    {
        return static_cast<CGUnitData&>(*this);
    }

    CGUnit* CGUnit::ToUnit()
    {
        return this;
    }

    CGUnit const* CGUnit::ToUnit() const
    {
        return this;
    }

    void CGUnit::UpdateDescriptors(JamCliValuesUpdate const& valuesUpdate)
    {
        CGObject::UpdateDescriptors(valuesUpdate);

        std::uint8_t* unitDataBase = reinterpret_cast<std::uint8_t*>(&GetUnitData());
        for (auto&& itr : valuesUpdate.Descriptors)
        {
            auto offset = itr.first * 4;
            if (offset <= sizeof(CGObjectData))
                continue;

            offset -= sizeof(CGObjectData);
            if (offset > sizeof(CGUnitData))
                continue;

            *reinterpret_cast<std::uint32_t*>(unitDataBase + offset) = itr.second;
        }
    }
}
