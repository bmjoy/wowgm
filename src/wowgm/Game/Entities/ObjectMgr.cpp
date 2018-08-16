#include "ObjectMgr.hpp"

#include "CGObject.hpp"
#include "CGUnit.hpp"
#include "CGItem.hpp"
#include "CGContainer.hpp"

namespace wowgm::game::entities
{
    template <typename T>
    void ObjectHolder<T>::Insert(T* object)
    {
        std::unique_lock<std::shared_mutex> lock(*GetMutex());

        GetContainer()[object->GUID] = object;
    }

    template <typename T>
    void ObjectHolder<T>::Remove(T* object)
    {
        std::unique_lock<std::shared_mutex> lock(*GetMutex());

        GetContainer().erase(object->GUID);
    }

    template <typename T>
    void ObjectHolder<T>::Remove(ObjectGuid const& guid)
    {
        std::unique_lock<std::shared_mutex> lock(*GetMutex());

        GetContainer().erase(guid);
    }

    template <typename T>
    T* ObjectHolder<T>::Find(ObjectGuid guid)
    {
        std::shared_lock<std::shared_mutex> lock(*GetMutex());

        typename ContainerType::iterator itr = GetContainer().find(guid);
        return (itr != GetContainer().end()) ? itr->second : nullptr;
    }

    template <typename T>
    std::shared_mutex* ObjectHolder<T>::GetMutex()
    {
        static std::shared_mutex _lock;
        return &_lock;
    }

    template <typename T>
    auto ObjectHolder<T>::GetContainer() -> ContainerType&
    {
        static ContainerType _objectMap;
        return _objectMap;
    }

    template class ObjectHolder<CGUnit>;
    template class ObjectHolder<CGItem>;
    template class ObjectHolder<CGContainer>;

    namespace ObjectAccessor
    {
        template <typename T>
        T* GetObject(ObjectGuid const& guid)
        {
            static_assert(std::is_base_of<CGObject, T>::value);

            // Specialized just below.
            static_assert(!std::is_same<CGObject, T>::value);

            return ObjectHolder<T>::Find(guid);
        }

        template <>
        CGObject* GetObject<CGObject>(ObjectGuid const& guid)
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

        void Destroy(ObjectGuid const& objectGuid)
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

        void Destroy(CGObject* object)
        {
            if (object != nullptr)
                Destroy(object->GUID);
        }

    }
}
