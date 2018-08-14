#pragma once

#include "UpdateField.hpp"
#include "ObjectGuid.hpp"

#include "CGObject.hpp"
#include "CGItem.hpp"

#include <cstdint>

namespace wowgm::game::entities
{
    using namespace wowgm::utilities;

    struct CGContainerData
    {
        CGContainerData() { }
        virtual ~CGContainerData() { }

        Descriptor<std::uint32_t> NumSlots;
        Descriptor<std::uint8_t, 4> _;
        Descriptor<ObjectGuid, 36> Contents;
    };

    class CGContainer : public CGContainerData, public CGItem
    {
    public:
        CGContainer(TypeMask typeMask);
        virtual ~CGContainer();

        CGContainerData const& GetContainerData() const;

        CGItem* ToItem() override;
        CGItem const* ToItem() const override;
    };
}
