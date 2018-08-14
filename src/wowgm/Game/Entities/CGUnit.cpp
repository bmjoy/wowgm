#include "CGUnit.hpp"
#include "ObjectMgr.hpp"

namespace wowgm::game::entities
{
    using namespace wowgm::utilities;

    CGUnit::CGUnit(TypeMask typeMask) : CGUnitData(), CGObject(typeMask)
    {

    }

    CGUnit::~CGUnit()
    {

    }

    CGUnitData const& CGUnit::GetUnitData() const
    {
        return static_cast<CGUnitData const&>(*this);
    }

    CGUnit* CGUnit::ToUnit()
    {
        return this;
    }

    CGUnit const* CGUnit::ToUnit() const
    {
        return this;
    }
}
