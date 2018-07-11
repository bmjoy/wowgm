#include "PhysicalDevice.hpp"
#include "Surface.hpp"

#include <vector>

namespace wowgm::graphics
{
    bool QueueFamilyIndices::IsComplete()
    {
        return Graphics >= 0 && Present >= 0;
    }

    std::uint32_t QueueFamilyIndices::GetQueueCount()
    {
        return sizeof(QueueFamilyIndices) / sizeof(std::int32_t);
    }

    std::int32_t* QueueFamilyIndices::EnumerateFamilies()
    {
        return reinterpret_cast<std::int32_t*>(this);
    }

    PhysicalDevice::PhysicalDevice() : _device(VK_NULL_HANDLE), _deviceScore(0)
    {

    }

    PhysicalDevice::PhysicalDevice(VkPhysicalDevice device, Surface* surface) : _device(device), _deviceScore(0)
    {
        vkGetPhysicalDeviceProperties(device, &_deviceProperties);
        vkGetPhysicalDeviceFeatures(device, &_deviceFeatures);

        if (_deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) // Favor dedicated graphics card
            _deviceScore += 1000;

        // Favor devices with larger texture sizes
        _deviceScore += _deviceProperties.limits.maxImageDimension2D;

        {
            uint32_t queueFamilyCount = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

            std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
            vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

            std::int32_t i = 0;
            for (const auto& queueFamily : queueFamilies)
            {
                VkBool32 presentSupport = false;
                vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface->GetSurface(), &presentSupport);

                if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
                    _queueFamilyIndices.Graphics = i;

                if (presentSupport && queueFamily.queueCount > 0)
                    _queueFamilyIndices.Present = 1;

                if (_queueFamilyIndices.IsComplete())
                    break;

                i++;
            }

        }

        // Ignore devices without a graphics queue
        if (_queueFamilyIndices.Graphics == -1)
            _deviceScore = 0;
    }

    std::uint32_t PhysicalDevice::GetScore()
    {
        return _deviceScore;
    }

    QueueFamilyIndices& PhysicalDevice::GetQueues()
    {
        return _queueFamilyIndices;
    }

    VkPhysicalDevice PhysicalDevice::GetDevice()
    {
        return _device;
    }

    VkPhysicalDeviceFeatures& PhysicalDevice::GetFeatures()
    {
        return _deviceFeatures;
    }

    VkPhysicalDeviceProperties& PhysicalDevice::GetProperties()
    {
        return _deviceProperties;
    }
}
