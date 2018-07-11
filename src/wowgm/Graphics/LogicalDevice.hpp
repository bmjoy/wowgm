#pragma once

#include <vulkan/vulkan.h>

namespace wowgm::graphics
{
    class PhysicalDevice;

    class LogicalDevice
    {
        LogicalDevice(VkDevice device, QueueFamilyIndices& indices);

    public:
        static LogicalDevice* Create(PhysicalDevice& physicalDevice);

        ~LogicalDevice();

    private:
        VkDevice _device;
        VkQueue _graphicsQueue;
    };
}
