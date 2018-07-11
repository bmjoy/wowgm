#pragma once

#include <vulkan/vulkan.h>
#include <cstdint>

namespace wowgm::graphics
{
    struct QueueFamilyIndices
    {
        std::int32_t Graphics = -1;

        bool IsComplete();
    };

    class PhysicalDevice
    {
    public:
        PhysicalDevice(VkPhysicalDevice device);
        PhysicalDevice();

        std::uint32_t GetScore();

    private:
        VkPhysicalDevice _device;
        VkPhysicalDeviceProperties _deviceProperties;
        VkPhysicalDeviceFeatures _deviceFeatures;

        QueueFamilyIndices _queueFamilyIndices;

        std::uint32_t _deviceScore;
    };
}
