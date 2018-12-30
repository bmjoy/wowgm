#pragma once

#include <graphics/vulkan/Traits.hpp>
#include <graphics/vulkan/VK.hpp>

#include <string>

namespace gfx::vk
{
    class Device;
    class Instance;

    template <typename T>
    class NamedObject
    {
        //static_assert(std::is_same<decltype(std::declval<T>().GetDevice()), Device*>::value, "Type of NamedObject<T> must have Device* T::GetDevice().");
        //static_assert(std::is_same<decltype(std::declval<T>().GetInstance()), Instance*>::value, "Type of NamedObject<T> must have Instance* T::GetInstance().");

        // Mark ctor as private and make derived friend so that only X : NamedObject<X> compiles
        NamedObject() {

        }

        friend T;

        T& underlying() {
            return static_cast<T&>(*this);
        }

        T const& underlying() const {
            return static_cast<T const&>(*this);
        }

    public:

        inline void SetName(std::string const& name) {
            _name = std::move(name);

            T& derived = underlying();

            derived.GetInstance()->SetObjectName<T>(derived.GetDevice(), uint64_t(derived.GetHandle()), std::string_view(_name.data()));
        }

        std::string const& GetName() const {
            return _name;
        }

    private:
        std::string _name;
    };
}