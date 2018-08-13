#pragma once

#include "UpdateField.hpp"
#include "ObjectGuid.hpp"

#include <type_traits>

namespace wowgm::game::entities
{
    using namespace wowgm::utilities;

    struct CGObjectData
    {
        CGObjectData() { }
        virtual ~CGObjectData() { }

        Descriptor<ObjectGuid> GUID;
        Descriptor<ObjectGuid> Data;
        Descriptor<std::uint16_t, 2> Type;
        Descriptor<std::uint32_t> Entry;
        Descriptor<float> Scale;
        Descriptor<std::uint32_t> Padding; // Actual name in the client, ???
    };

    class CGUnit;
    class CGGameObject;
    class CGPlayer;
    class CGItem;

    class CGObject : public CGObjectData
    {
    public:
        explicit CGObject(TypeMask typeMask);
        virtual ~CGObject();

        CGObjectData const& GetObjectData() const;

        //virtual CGUnit* ToUnit();
        //virtual CGUnit const* ToUnit() const;

        //virtual CGPlayer* ToPlayer();
        //virtual CGPlayer const* ToPlayer() const;

        //virtual CGGameObject* ToGameObject();
        //virtual CGGameObject const* ToGameObject() const;

        virtual CGItem* ToItem();
        virtual CGItem const* ToItem() const;

    private:
        TypeMask _typeMask;
    };
}