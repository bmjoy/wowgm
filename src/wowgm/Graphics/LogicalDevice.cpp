#include "LogicalDevice.hpp"
#include "PhysicalDevice.hpp"
#include "Queue.hpp"

#include <vector>

#include "SharedGraphicsDefines.hpp"

namespace wowgm::graphics
{
    LogicalDevice::LogicalDevice(VkDevice device, QueueFamilyIndices& indices) : _device(device), _graphicsQueue(nullptr), _presentQueue(nullptr)
    {
        for (std::uint32_t i = 0; i < indices.GetQueueCount(); ++i)
        {
            auto queueIndice = indices.EnumerateFamilies()[i];

            VkQueue deviceQueue;
            vkGetDeviceQueue(device, queueIndice, 0, &deviceQueue);
            reinterpret_cast<Queue**>(&_graphicsQueue)[i] = new Queue(this, deviceQueue, queueIndice);
        }
    }

    LogicalDevice::~LogicalDevice()
    {
        delete _graphicsQueue;
        _graphicsQueue = nullptr;
        delete _presentQueue;
        _presentQueue = nullptr;

        vkDestroyDevice(_device, nullptr);
        _device = VK_NULL_HANDLE;
    }

    Queue* LogicalDevice::GetGraphicsQueue()
    {
        return _graphicsQueue;
    }

    Queue* LogicalDevice::GetPresentQueue()
    {
        return _presentQueue;
    }
}
