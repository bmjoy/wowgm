#pragma once

#include "UpdateField.hpp"
#include "ObjectGuid.hpp"
#include "C3Vector.hpp"

#include <type_traits>
#include <cstdint>

namespace wowgm::game::entities
{
    using namespace wowgm::game::structures;

    struct CGObjectData
    {
        CGObjectData() { }
        virtual ~CGObjectData() { }

        Descriptor<ObjectGuid> GUID;
        Descriptor<ObjectGuid> Data;
        Descriptor<std::uint16_t, 2> Type;
        Descriptor<std::uint32_t> Entry;
        Descriptor<float> Scale;
        Descriptor<std::uint32_t> _; // Padding
    };

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
        explicit CGObject(TypeMask typeMask);
        virtual ~CGObject();

        CGObjectData const& GetObjectData() const;

        virtual CGUnit* ToUnit();
        virtual CGUnit const* ToUnit() const;

        // virtual CGPlayer* ToPlayer();
        // virtual CGPlayer const* ToPlayer() const;

        // virtual CGGameObject* ToGameObject();
        // virtual CGGameObject const* ToGameObject() const;

        virtual CGItem* ToItem();
        virtual CGItem const* ToItem() const;

        virtual CGContainer* ToContainer();
        virtual CGContainer const* ToContainer() const;

        C3Vector* GetPosition();
        C3Vector const* GetPosition() const;

    private:
        TypeMask _typeMask;
    };
}