#pragma once

#include "UpdateField.hpp"
#include "ObjectGuid.hpp"
#include "CClientObjCreate.hpp"
#include "CGObject.hpp"
#include "CGItem.hpp"

#include <cstdint>

namespace wowgm::game::entities
{
    using namespace wowgm::game::structures;

#pragma pack(push, 1)
    struct CGContainerData
    {
        Descriptor<std::uint32_t> NumSlots;
        Descriptor<std::uint8_t, 4> _;
        Descriptor<ObjectGuid, 36> Contents;
    };
#pragma pack(pop)

    static_assert(sizeof(CGContainerData) == sizeof(std::uint32_t) * 74);

    class CGContainer : public CGContainerData, public CGItem
    {
    public:
        explicit CGContainer(CClientObjCreate const& objCreate);
        virtual ~CGContainer();

        CGContainerData const& GetContainerData() const;

        CGItem* ToItem() override;
        CGItem const* ToItem() const override;
    };
}
