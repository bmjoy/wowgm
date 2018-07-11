#pragma once

#include <vulkan/vulkan.h>
#include <cstdint>

namespace wowgm::graphics
{
    struct QueueFamilyIndices
    {
        std::int32_t Graphics = -1;
        std::int32_t Present = -1;

        bool IsComplete();
    };

    class PhysicalDevice
    {
        PhysicalDevice(VkPhysicalDevice device, VkSurfaceKHR surface);
        PhysicalDevice();
    public:

        std::uint32_t GetScore();
        QueueFamilyIndices& GetQueues();

        VkPhysicalDevice GetDevice();

    private:
        VkPhysicalDevice _device;
        VkPhysicalDeviceProperties _deviceProperties;
        VkPhysicalDeviceFeatures _deviceFeatures;

        QueueFamilyIndices _queueFamilyIndices;

        std::uint32_t _deviceScore;
    };
}
