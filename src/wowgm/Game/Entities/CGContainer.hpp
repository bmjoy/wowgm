#pragma once

#include "ObjectGuid.hpp"
#include "CClientObjCreate.hpp"
#include "JamCliValuesUpdate.hpp"
#include "CGObject.hpp"
#include "CGItem.hpp"

#include <cstdint>
#include <array>

namespace wowgm::game::entities
{
    using namespace wowgm::game::structures;

#pragma pack(push, 1)
    struct CGContainerData
    {
        uint32_t NumSlots;
        std::array<uint8_t, 4> _;
        std::array<ObjectGuid, 36> Contents;
    };
#pragma pack(pop)

    static_assert(sizeof(CGContainerData) == sizeof(uint32_t) * 74);

    class CGContainer : public CGContainerData, public CGItem
    {
    public:
        explicit CGContainer(CClientObjCreate const& objCreate);
        virtual ~CGContainer();

        CGContainerData const& GetContainerData() const;
        CGContainerData& GetContainerData();

        CGItem* ToItem() override;
        CGItem const* ToItem() const override;

        uint32_t UpdateDescriptors(JamCliValuesUpdate& valuesUpdate) override;
    };
}
