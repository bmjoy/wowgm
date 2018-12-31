#pragma once

#include <graphics/vulkan/VK.hpp>
#include <graphics/vulkan/Instance.hpp>

namespace gfx::vk
{
    class PhysicalDevice
    {
        friend class Instance;

        PhysicalDevice(Instance* instance, ::VkPhysicalDevice handle);

        PhysicalDevice(const PhysicalDevice&) = delete;
        PhysicalDevice(PhysicalDevice&&) = delete;

    public:

        Instance* GetInstance() const { return _instance; }
        VkPhysicalDevice GetHandle() const { return _handle; }
        VkPhysicalDeviceProperties const& GetProperties() const { return _properties; }

    private:
        VkPhysicalDevice _handle = VK_NULL_HANDLE;
        VkPhysicalDeviceProperties _properties;
        Instance* _instance = nullptr;
    };
}
