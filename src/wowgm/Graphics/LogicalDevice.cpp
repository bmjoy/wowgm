#include "LogicalDevice.hpp"
#include "PhysicalDevice.hpp"

#include <vector>

#include "SharedGraphicsDefines.hpp"

namespace wowgm::graphics
{
    LogicalDevice::LogicalDevice(VkDevice device, QueueFamilyIndices& indices) : _device(device)
    {
        for (std::uint32_t i = 0; i < indices.GetQueueCount(); ++i)
        {
            VkQueue deviceQueue;
            vkGetDeviceQueue(device, indices.EnumerateFamilies()[i], 0, &deviceQueue);
            reinterpret_cast<Queue*>(&_graphicsQueue)[i] = Queue(deviceQueue);
        }
    }

    LogicalDevice::~LogicalDevice()
    {
        vkDestroyDevice(_device, nullptr);
    }
}
