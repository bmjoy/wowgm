#pragma once

#include "VEZ.hpp"
#include <vector>

namespace vez
{
    class Image;

    struct SwapchainSupport
    {
        VkSurfaceCapabilitiesKHR capabilities = {};
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    class Swapchain
    {
    public:
        static VkResult Create(Device* pDevice, const SwapchainCreateInfo* pCreateInfo, Swapchain** ppSwapchain);

        VkSwapchainKHR GetHandle() { return _handle; }

        const SwapchainCreateInfo& GetCreateInfo() { return _createInfo; }

        VkSurfaceFormatKHR GetFormat() { return _format; }

        VkSurfaceKHR GetSurface() const { return _surface; }

        VkResult AcquireNextImage(uint32_t* pImageIndex, VkSemaphore* pImageAcquiredSemaphore);

        Image* GetImage(uint32_t index);

    private:
        VkResult Allocate();

        VkSwapchainKHR _handle = VK_NULL_HANDLE;
        VkSurfaceFormatKHR _format{};
        VkSurfaceKHR _surface = VK_NULL_HANDLE;

        SwapchainSupport _swapchainSupport;
        SwapchainCreateInfo _createInfo;

        Device* _device = nullptr;

        std::vector<Image*> _images;
    };
}