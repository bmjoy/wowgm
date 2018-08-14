#pragma once

#include "ObjectGuid.hpp"
#include "CGObject.hpp"

#include <type_traits>
#include <vector>

namespace wowgm::game::entities
{
    using namespace wowgm::utilities;

    class ObjectMgr final
    {
        template <TypeID TypeId>
        struct typeid_trait {
        };

        template <> struct typeid_trait<TYPEID_OBJECT>        { using type = CGObject; };
        template <> struct typeid_trait<TYPEID_ITEM>          { using type = CGItem; };
        template <> struct typeid_trait<TYPEID_CONTAINER>     { using type = CGContainer; };
        template <> struct typeid_trait<TYPEID_UNIT>          { using type = CGUnit; };
        template <> struct typeid_trait<TYPEID_PLAYER>        { using type = CGPlayer; };
        template <> struct typeid_trait<TYPEID_GAMEOBJECT>    { using type = CGGameObject; };
        template <> struct typeid_trait<TYPEID_DYNAMICOBJECT> { using type = CGDynamicObject; };
        template <> struct typeid_trait<TYPEID_CORPSE>        { using type = CGCorpse; };
        template <> struct typeid_trait<TYPEID_AREATRIGGER>   { using type = CGAreaTrigger; };

    public:

        static ObjectMgr* Instance();

        template <typename T, typename std::enable_if<std::is_base_of<CGObject, T>::value, int>::type = 0>
        inline T* GetEntity(ObjectGuid const& guid) const {
            for (CGObject* itr : _objects)
                if (itr->GetGUID() == guid)
                    return static_cast<T*>(itr);
            return nullptr;
        }

        template <TypeID Type>
        inline auto GetEntity(ObjectGuid const& guid) const -> typename typeid_trait<Type>::type* {
            return GetEntity<typename typeid_trait<Type>::type>(guid);
        }

        CGPlayer* GetLocalPlayer() { return _localPlayer; }

    private:
        std::vector<CGObject*> _objects;
        CGPlayer* _localPlayer;
    };
}
