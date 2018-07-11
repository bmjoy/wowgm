#pragma once

#include <vulkan/vulkan.h>
#include <cstdint>

namespace wowgm::graphics
{
    class LogicalDevice;
    class PhysicalDevice;
    class LogicalDeviceCache;
    class PhysicalDeviceCache;

    template <typename T>
    class DeviceCache
    {
    private:
        std::vector<T> _data;
    };

    class PhysicalDeviceCache : public DeviceCache<PhysicalDevice>
    {
    public:
        PhysicalDeviceCache(VkInstance instance)
        {
            uint32_t deviceCount = 0;
            vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
        }
    };

    class LogicalDevice
    {
    private:
        VkDevice _device;
    };

    class PhysicalDevice
    {
    private:
        VkPhysicalDevice _device;
    };
}