#pragma once

#include <vulkan/vulkan.h>
#include <cstdint>

namespace wowgm::graphics
{
    class Surface;

    struct QueueFamilyIndices
    {
        /// Whenever you add/edit this block, make sure to fix the queues assignments in
        /// LogicalDevice.
        std::int32_t Graphics = -1;
        std::int32_t Present = -1;

        std::uint32_t GetQueueCount();
        std::int32_t* EnumerateFamilies();

        bool IsComplete();
    };

    class PhysicalDevice
    {
        PhysicalDevice(VkPhysicalDevice device, Surface* surface);
        PhysicalDevice();
    public:

        std::uint32_t GetScore();
        QueueFamilyIndices& GetQueues();

        VkPhysicalDevice GetDevice();
        VkPhysicalDeviceFeatures GetFeatures();

    private:
        VkPhysicalDevice _device;
        VkPhysicalDeviceProperties _deviceProperties;
        VkPhysicalDeviceFeatures _deviceFeatures;

        QueueFamilyIndices _queueFamilyIndices;

        std::uint32_t _deviceScore;
    };
}
