#include "CGUnit.hpp"
#include "ObjectMgr.hpp"

namespace wowgm::game::entities
{
    using namespace wowgm::game::structures;

    CGUnit::CGUnit(CClientObjCreate const& createBlock) : CGUnitData(), CGObject(createBlock)
    {
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

    uint32_t CGUnit::UpdateDescriptors(JamCliValuesUpdate& valuesUpdate)
    {
        uint32_t startOffset = CGObject::UpdateDescriptors(valuesUpdate);
        uint8_t* unitDatabase = reinterpret_cast<uint8_t*>(&GetUnitData());

        auto itr = valuesUpdate.Descriptors.begin();
        while (itr != valuesUpdate.Descriptors.end())
        {
            uint32_t calculatedOffset = itr->first * 4 - startOffset;
            if (calculatedOffset > sizeof(CGUnitData))
            {
                ++itr;
                continue;
            }

            *reinterpret_cast<uint32_t*>(unitDatabase + calculatedOffset) = itr->second;
            itr = valuesUpdate.Descriptors.erase(itr);
        }

        return startOffset + sizeof(CGUnitData);
    }
}
