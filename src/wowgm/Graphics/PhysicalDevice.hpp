#pragma once

#include <vulkan/vulkan.h>
#include <cstdint>
#include <vector>

namespace wowgm::graphics
{
    class Surface;
    class Instance;

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

    struct SwapChainSupportDetails
    {
        VkSurfaceCapabilitiesKHR Capabilities;
        std::vector<VkSurfaceFormatKHR> Formats;
        std::vector<VkPresentModeKHR> PresentModes;
    };

    class PhysicalDevice
    {
    public:
        PhysicalDevice(VkPhysicalDevice device, Surface* surface);
        PhysicalDevice();

        std::uint32_t GetScore();
        QueueFamilyIndices& GetQueues();

        VkPhysicalDevice GetVkDevice();
        VkPhysicalDeviceFeatures& GetVkFeatures();
        VkPhysicalDeviceProperties& GetVkProperties();

        bool CheckDeviceExtensionSupport();

        SwapChainSupportDetails& GetSwapChainSupportDetails();

        Surface* GetSurface();
        Instance* GetInstance();

    private:
        void _CreateSwapChainSupportDetails();

    private:
        Surface* _surface;

        VkPhysicalDevice _device;
        VkPhysicalDeviceProperties _deviceProperties;
        VkPhysicalDeviceFeatures _deviceFeatures;

        QueueFamilyIndices _queueFamilyIndices;
        SwapChainSupportDetails _swapChainSupportDetails;

        std::uint32_t _deviceScore;
    };
}
