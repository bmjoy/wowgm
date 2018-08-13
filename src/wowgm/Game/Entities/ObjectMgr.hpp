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
            static_assert(false, "Type not mapped.");
        };

        template<> struct typeid_trait<TYPEID_OBJECT> { using type = CGObject; };
        template<> struct typeid_trait<TYPEID_UNIT>   { using type = CGUnit; };
        template<> struct typeid_trait<TYPEID_PLAYER> { using type = CGPlayer; };
        template<> struct typeid_trait<TYPEID_ITEM>   { using type = CGItem; };

    public:

        static std::unique_ptr<ObjectMgr> Instance();

        template <typename T, typename std::enable_if<std::is_base_of<CGObject, T>::value, int>::type = 0>
        inline T* GetEntity(ObjectGuid guid) const {
            for (CGObject* itr : _objects)
                if (itr->GetGUID() == guid)
                    return static_cast<T*>(itr);
        }

        template <TypeID Type>
        inline auto GetEntity(ObjectGuid guid) const -> typeid_trait<Type>::type* {
            return GetEntity<typeid_trait<Type>::type>(guid);
        }

        CGPlayer* GetLocalPlayer() { return _localPlayer; }

    private:
        std::vector<CGObject*> _objects;
        CGPlayer* _localPlayer;
    };
}

#define sObjectMgr wowgm::game::entities::ObjectMgr::Instance();