#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <sstream>
#include <vulkan/vulkan.h>

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

    class PhysicalDevice
    {
    public:
        PhysicalDevice(VkPhysicalDevice device, Surface* surface);
        ~PhysicalDevice();

        PhysicalDevice() = delete;
        PhysicalDevice(PhysicalDevice const&) = delete;
        PhysicalDevice(PhysicalDevice&&) = delete;

        std::uint32_t GetScore();
        QueueFamilyIndices& GetQueues();
        VkPhysicalDeviceFeatures& GetPhysicalDeviceFeatures();
        VkPhysicalDeviceProperties& GetPhysicalDeviceProperties();

        bool CheckDeviceExtensionSupport();

        VkSurfaceCapabilitiesKHR& GetCapabilities();
        std::vector<VkSurfaceFormatKHR>& GetFormats();
        std::vector<VkPresentModeKHR> GetPresentModes();
        VkPhysicalDeviceMemoryProperties& GetMemoryProperties();

        Surface* GetSurface();
        Instance* GetInstance();

        operator VkPhysicalDevice() const { return _device; }

        std::string ToString();
        void ToString(std::stringstream& ss);

    private:
        void _CreateSwapChainSupportDetails();

    private:
        Surface* _surface;

        VkPhysicalDevice _device;
        VkPhysicalDeviceProperties _deviceProperties;
        VkPhysicalDeviceFeatures _deviceFeatures;
        VkPhysicalDeviceMemoryProperties _deviceMemoryProperties;

        QueueFamilyIndices _queueFamilyIndices;

        VkSurfaceCapabilitiesKHR _surfaceCapabilities;
        std::vector<VkSurfaceFormatKHR> _surfaceFormats;
        std::vector<VkPresentModeKHR> _surfacePresentModes;


        std::uint32_t _deviceScore;
    };
}
