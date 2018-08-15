#include "ObjectMgr.hpp"

#include "CGObject.hpp"
#include "CGUnit.hpp"
#include "CGItem.hpp"
#include "CGContainer.hpp"

namespace wowgm::game::entities
{
    template <typename T>
    void ObjectHolder<T>::Insert(T* o)
    {
        std::unique_lock<std::shared_mutex> lock(*GetMutex());

        GetContainer()[o->GetGUID()] = o;
    }

    template< typename T>
    void ObjectHolder<T>::Remove(T* o)
    {
        std::unique_lock<std::shared_mutex> lock(*GetMutex());

        GetContainer().erase(o->GetGUID());
    }

    template< typename T>
    void ObjectHolder<T>::Remove(ObjectGuid guid)
    {
        std::unique_lock<std::shared_mutex> lock(*GetMutex());

        GetContainer().erase(guid);
    }

    template <typename T>
    T* ObjectHolder<T>::Find(ObjectGuid guid)
    {
        std::shared_lock<std::shared_mutex> lock(*GetMutex());

        typename Container::iterator itr = GetContainer().find(guid);
        return (itr != GetContainer().end()) ? itr->second : nullptr;
    }

    template <typename T>
    auto ObjectHolder<T>::GetContainer() -> ContainerType&
    {
        static ContainerType _objectMap;
        return _objectMap;
    }

    template <typename T>
    std::shared_mutex* ObjectHolder<T>::GetMutex()
    {
        static std::shared_mutex _lock;
        return &_lock;
    }

    template <> class ObjectHolder<CGUnit>;
    template <> class ObjectHolder<CGItem>;
    template <> class ObjectHolder<CGContainer>;
}
