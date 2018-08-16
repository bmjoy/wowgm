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

    template <typename T, typename std::enable_if<std::is_base_of<CGObject, T>::value, int>::type = 0>
    class ObjectHolder final
    {
        ObjectHolder() { }

    public:
        using ContainerType = std::unordered_map<ObjectGuid, T*>;

        static ContainerType& GetContainer()
        {
            static ContainerType _objectMap;
            return _objectMap;
        }

        static void Insert(T* object)
        {
            std::unique_lock<std::shared_mutex> lock(*GetMutex());

            GetContainer()[object->GUID] = object;
        }

        static void Remove(T* object)
        {
            std::unique_lock<std::shared_mutex> lock(*GetMutex());

            GetContainer().erase(object->GUID);
        }

        static void Remove(ObjectGuid const& guid)
        {
            std::unique_lock<std::shared_mutex> lock(*GetMutex());

            GetContainer().erase(guid);
        }

        template <typename... Args>
        inline static T* Emplace(Args&&... args)
        {
            T* instance = new T(std::forward<Args>(args)...);
            Insert(instance);
            return instance;
        }

        static T* Find(ObjectGuid guid)
        {
            std::shared_lock<std::shared_mutex> lock(*GetMutex());

            typename ContainerType::iterator itr = GetContainer().find(guid);
            return (itr != GetContainer().end()) ? itr->second : nullptr;
        }

        static std::shared_mutex* GetMutex()
        {
            static std::shared_mutex _lock;
            return &_lock;
        }
    };

    namespace ObjectAccessor
    {
        template <typename T>
        static inline T* GetObject(ObjectGuid const& guid)
        {
            return ObjectHolder<T>::Find(guid);
        }

        template <TypeID Type>
        static inline auto GetObject(ObjectGuid const& guid) -> typename details::typeid_trait<Type>::type
        {
            return ObjectHolder<typename details::typeid_trait<Type>::type>(guid);
        }

        template <>
        static inline CGObject* GetObject(ObjectGuid const& guid)
        {
            switch (guid.GetTypeId())
            {
                case TYPEID_UNIT:
                    return GetObject<CGUnit>(guid);
                case TYPEID_ITEM:
                    return GetObject<CGItem>(guid);
                case TYPEID_CONTAINER:
                    return GetObject<CGContainer>(guid);
            }

            return nullptr;
        }

        static inline void Destroy(CGObject* object)
        {
            if (object != nullptr)
                Destroy(object->GUID);
        }

        static inline void Destroy(ObjectGuid const& objectGuid)
        {
            switch (objectGuid.GetTypeId())
            {
            case TYPEID_UNIT:
                ObjectHolder<CGUnit>::Remove(objectGuid);
                break;
            case TYPEID_ITEM:
                ObjectHolder<CGItem>::Remove(objectGuid);
                break;
            case TYPEID_CONTAINER:
                ObjectHolder<CGContainer>::Remove(objectGuid);
                break;
            }
        }
    }
}
