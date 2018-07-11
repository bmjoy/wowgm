#pragma once

#include <vulkan/vulkan.h>

namespace wowgm::graphics
{
    class PhysicalDevice;
    class Surface;

    class SwapChain
    {
    public:
        SwapChain(PhysicalDevice* device);
        ~SwapChain();

    private:
        void _SelectFormat();
        void _SelectPresentMode();
        void _SelectExtent();

    private:
        PhysicalDevice* _device;

        VkSwapchainKHR _swapChain;

        VkSurfaceFormatKHR _surfaceFormat;
        VkPresentModeKHR _presentMode;
        VkExtent2D _extent;
    };
}