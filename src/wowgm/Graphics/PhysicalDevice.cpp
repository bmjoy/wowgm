#include "PhysicalDevice.hpp"
#include "Surface.hpp"
#include "Instance.hpp"

#include "SharedGraphicsDefines.hpp"

#include <set>
#include <string>

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

    PhysicalDevice::PhysicalDevice(VkPhysicalDevice device, Surface* surface) : _device(device), _deviceScore(0), _surface(surface)
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

        _CreateSwapChainSupportDetails();

        bool extensionsSupported = CheckDeviceExtensionSupport();
        bool swapChainAdequate = false;
        if (extensionsSupported)
            swapChainAdequate = !_swapChainSupportDetails.Formats.empty() && !_swapChainSupportDetails.PresentModes.empty();

        // Ignore devices without a graphics queue, without the required expansion support, or without swap chain support
        if (_queueFamilyIndices.Graphics == -1 || !extensionsSupported || !swapChainAdequate)
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

    VkPhysicalDevice PhysicalDevice::GetVkDevice()
    {
        return _device;
    }

    VkPhysicalDeviceFeatures& PhysicalDevice::GetVkFeatures()
    {
        return _deviceFeatures;
    }

    VkPhysicalDeviceProperties& PhysicalDevice::GetVkProperties()
    {
        return _deviceProperties;
    }

    bool PhysicalDevice::CheckDeviceExtensionSupport()
    {
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(_device, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(_device, nullptr, &extensionCount, availableExtensions.data());

        std::set<std::string> requiredExtensions(details::requiredDeviceExtensions.begin(), details::requiredDeviceExtensions.end());

        for (const auto& extension : availableExtensions)
            requiredExtensions.erase(extension.extensionName);

        return requiredExtensions.empty();
    }

    SwapChainSupportDetails& PhysicalDevice::GetSwapChainSupportDetails()
    {
        return _swapChainSupportDetails;
    }

    void PhysicalDevice::_CreateSwapChainSupportDetails()
    {
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_device, _surface->GetSurface(), &_swapChainSupportDetails.Capabilities);

        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(_device, _surface->GetSurface(), &formatCount, nullptr);

        if (formatCount != 0)
        {
            _swapChainSupportDetails.Formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(_device, _surface->GetSurface(), &formatCount, _swapChainSupportDetails.Formats.data());
        }

        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(_device, _surface->GetSurface(), &presentModeCount, nullptr);

        if (presentModeCount != 0)
        {
            _swapChainSupportDetails.PresentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(_device, _surface->GetSurface(), &presentModeCount, _swapChainSupportDetails.PresentModes.data());
        }

    }

    Surface* PhysicalDevice::GetSurface()
    {
        return _surface;
    }

    Instance* PhysicalDevice::GetInstance()
    {
        return _surface->GetInstance();
    }
}
