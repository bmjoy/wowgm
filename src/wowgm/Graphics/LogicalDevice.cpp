#include "LogicalDevice.hpp"
#include "PhysicalDevice.hpp"

#include <vector>

#include "SharedGraphicsDefines.hpp"

namespace wowgm::graphics
{
    LogicalDevice* LogicalDevice::Create(PhysicalDevice& physicalDevice)
    {
        QueueFamilyIndices& queueFamilies = physicalDevice.GetQueues();

        // TODO: Implement multiple queues.

        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamilies.Graphics;
        queueCreateInfo.queueCount = 1;

        float queuePriority = 1.0f;
        queueCreateInfo.pQueuePriorities = &queuePriority;

        // Set of device features that we'll be using.
        // Related to the features we need on the physical device
        VkPhysicalDeviceFeatures deviceFeatures = { };

        VkDeviceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

        createInfo.pQueueCreateInfos = &queueCreateInfo;
        createInfo.queueCreateInfoCount = 1;
        createInfo.pEnabledFeatures = &deviceFeatures;

        createInfo.enabledExtensionCount = 0;

#ifdef ENABLE_VALIDATION_LAYERS
        createInfo.enabledLayerCount = static_cast<uint32_t>(details::validationLayers.size());
        createInfo.ppEnabledLayerNames = details::validationLayers.data();
#else
        createInfo.enabledLayerCount = 0;
#endif

        VkDevice device;
        if (vkCreateDevice(physicalDevice.GetDevice(), &createInfo, nullptr, &device) != VK_SUCCESS)
            throw std::runtime_error("failed to create logical device!");

        return new LogicalDevice(device);
    }

    LogicalDevice::LogicalDevice(VkDevice device, QueueFamilyIndices& indices) : _device(device)
    {
        vkGetDeviceQueue(device, indices.Graphics, 0, &_graphicsQueue);
    }

    LogicalDevice::~LogicalDevice()
    {
        vkDestroyDevice(_device, nullptr);
    }
}
