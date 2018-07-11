#pragma once

#include <vulkan/vulkan.h>

#include "Queue.hpp"

namespace wowgm::graphics
{
    class PhysicalDevice;
    struct QueueFamilyIndices;

    class LogicalDevice
    {
    public:
        LogicalDevice(VkDevice device, QueueFamilyIndices& indices);
        ~LogicalDevice();

        Queue* GetGraphicsQueue();
        Queue* GetPresentQueue();

        VkDevice GetVkDevice();

    private:
        VkDevice _device;

        // This *needs* to be in the same order as indices defined in QueueFamilyIndices.
        // (We are more or less memcpy-ing)
        Queue* _graphicsQueue;
        Queue* _presentQueue;
    };
}
