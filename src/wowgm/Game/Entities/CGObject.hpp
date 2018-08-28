#pragma once

#include "ObjectGuid.hpp"
#include "C3Vector.hpp"
#include "JamCliValuesUpdate.hpp"
#include "CClientObjCreate.hpp"

#include <type_traits>
#include <cstdint>
#include <array>


namespace wowgm::game::entities
{
    using namespace wowgm::game::structures;

#pragma pack(push, 1)
    struct CGObjectData
    {
        ObjectGuid GUID;
        ObjectGuid Data;
        std::array<std::uint16_t, 2> Type;
        std::uint32_t Entry;
        float Scale;
        std::uint32_t _; // Padding
    };
#pragma pack(pop)

    static_assert(sizeof(CGObjectData) == sizeof(std::uint32_t) * 8);

    class CGUnit;
    class CGGameObject;
    class CGPlayer;
    class CGItem;
    class CGContainer;
    class CGDynamicObject;
    class CGCorpse;
    class CGAreaTrigger;

    class CGObject : public CGObjectData, private C3Vector
    {
    public:
        explicit CGObject(CClientObjCreate const& objCreate);
        virtual ~CGObject();

        CGObjectData const& GetObjectData() const;
        CGObjectData& GetObjectData();

        virtual void UpdateDescriptors(JamCliValuesUpdate const& valuesUpdate);

        virtual CGUnit* ToUnit();
        virtual CGUnit const* ToUnit() const;

        virtual CGPlayer* ToPlayer();
        virtual CGPlayer const* ToPlayer() const;

        // virtual CGGameObject* ToGameObject();
        // virtual CGGameObject const* ToGameObject() const;

        virtual CGItem* ToItem();
        virtual CGItem const* ToItem() const;

        virtual CGContainer* ToContainer();
        virtual CGContainer const* ToContainer() const;

        C3Vector& GetPosition();
        C3Vector const& GetPosition() const;

        TypeMask GetTypeMask() const;
    };
}