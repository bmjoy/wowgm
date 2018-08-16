#pragma once

#include "ObjectGuid.hpp"
#include "CGObject.hpp"

#include <shared_mutex>
#include <type_traits>

// WTF, stop this shit please
#ifdef GetObject
#undef GetObject
#endif

namespace wowgm::game::entities
{
    using namespace wowgm::game::structures;

    namespace details
    {
        template <TypeID TypeId>
        struct typeid_trait { };

        template <> struct typeid_trait<TYPEID_OBJECT> { using type = CGObject; };
        template <> struct typeid_trait<TYPEID_ITEM> { using type = CGItem; };
        template <> struct typeid_trait<TYPEID_CONTAINER> { using type = CGContainer; };
        template <> struct typeid_trait<TYPEID_UNIT> { using type = CGUnit; };
        template <> struct typeid_trait<TYPEID_PLAYER> { using type = CGPlayer; };
        template <> struct typeid_trait<TYPEID_GAMEOBJECT> { using type = CGGameObject; };
        template <> struct typeid_trait<TYPEID_DYNAMICOBJECT> { using type = CGDynamicObject; };
        template <> struct typeid_trait<TYPEID_CORPSE> { using type = CGCorpse; };
        template <> struct typeid_trait<TYPEID_AREATRIGGER> { using type = CGAreaTrigger; };
    }

    template <typename T>
    class ObjectHolder final
    {
        static_assert(std::is_base_of<CGObject, T>::value);

        ObjectHolder() { }

    public:
        using ContainerType = std::unordered_map<ObjectGuid, T*>;

        static ContainerType& GetContainer();

        static void Insert(T* object);

        static void Remove(T* object);

        static void Remove(ObjectGuid const& guid);

        static T* Find(ObjectGuid guid);

        static std::shared_mutex* GetMutex();
    };

    namespace ObjectAccessor
    {
        template <typename T>
        T* GetObject(ObjectGuid const& guid);

        template <TypeID Type>
        inline auto GetObject(ObjectGuid const& guid) -> typename details::typeid_trait<Type>::type
        {
            return ObjectHolder<typename details::typeid_trait<Type>::type>(guid);
        }

        void Destroy(ObjectGuid const& objectGuid);

        void Destroy(CGObject* object);
    }
}
