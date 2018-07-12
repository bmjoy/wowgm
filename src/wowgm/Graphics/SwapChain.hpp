#pragma once
#include "Image.hpp"

#include <vector>

#include <vulkan/vulkan.h>

namespace wowgm::graphics
{
    class PhysicalDevice;
    class LogicalDevice;
    class Surface;

    class SwapChain
    {
    public:
        SwapChain(PhysicalDevice* device);
        ~SwapChain();

        SwapChain(SwapChain&&) = delete;
        SwapChain(SwapChain const&) = delete;
        operator VkSwapchainKHR() const { return _swapChain; }

        VkSurfaceFormatKHR GetSurfaceFormat();
        VkPresentModeKHR GetPresentMode();
        VkExtent2D GetExtent();

        PhysicalDevice* GetPhysicalDevice();
        LogicalDevice* GetLogicalDevice();

    private:
        void _SelectFormat();
        void _SelectPresentMode();
        void _SelectExtent();


    private:
        PhysicalDevice* _physicalDevice;

        VkSwapchainKHR _swapChain;

        VkSurfaceFormatKHR _surfaceFormat;
        VkPresentModeKHR _presentMode;
        VkExtent2D _extent;

        std::vector<Image*> _swapChainImages;
    };
}