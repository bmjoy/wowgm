#pragma once

#include "UpdateField.hpp"
#include "ObjectGuid.hpp"

namespace wowgm::game::entities
{
    using namespace wowgm::utilities;

    struct CGObjectData
    {
        CGObjectData() { }

        Descriptor<ObjectGuid> GUID;
        Descriptor<ObjectGuid> Data;
        Descriptor<std::uint16_t, 2> Type;
        Descriptor<std::uint32_t> Entry;
        Descriptor<float> Scale;
    };

    class Unit;
    class GameObject;
    class Player;
    class Item;

    class Object : public CGObjectData
    {
    public:
        Object();
        virtual ~Object();

        ObjectGuid GetGUID() const;
        float GetScale() const;

        CGObjectData const& GetData() const;

        Unit* ToUnit();
        Unit const* ToUnit() const;

        Player* ToPlayer();
        Player const* ToPlayer() const;

        GameObject* ToGameObject();
        GameObject const* ToGameObject() const;

        Item* ToItem();
        Item const* ToItem() const;
    };
}