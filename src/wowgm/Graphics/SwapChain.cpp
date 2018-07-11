#include "SwapChain.hpp"
#include "PhysicalDevice.hpp"
#include "Surface.hpp"
#include "Instance.hpp"
#include "LogicalDevice.hpp"

#include <algorithm>

namespace wowgm::graphics
{
    SwapChain::SwapChain(PhysicalDevice* device) : _device(device)
    {
        _SelectFormat();
        _SelectPresentMode();
        _SelectExtent();

        auto& swapChainSupport = _device->GetSwapChainSupportDetails();

        // Enable triple buffering
        uint32_t imageCount = swapChainSupport.Capabilities.minImageCount + 1;
        if (swapChainSupport.Capabilities.maxImageCount > 0 && imageCount > swapChainSupport.Capabilities.maxImageCount)
            imageCount = swapChainSupport.Capabilities.maxImageCount;

        VkSwapchainCreateInfoKHR createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = *device->GetSurface();
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = _surfaceFormat.format;
        createInfo.imageColorSpace = _surfaceFormat.colorSpace;
        createInfo.imageExtent = _extent;
        createInfo.imageArrayLayers = 1; // Always 1 unless stereoscopic 3D.
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;


        // If indice and present are two different queues families, we need concurrency
        QueueFamilyIndices& indices = _device->GetQueues();
        uint32_t queueFamilyIndices[] = { (uint32_t)indices.Graphics, (uint32_t)indices.Present };

        if (indices.Graphics != indices.Present)
        {
            // Images can be used across multiple queue families without explicit ownership transfers.
            //! TODO: Implement ownership and remove this.

            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        }
        else
        {
            // An image is owned by one queue family at a time and ownership must be explicitly
            // transfered before using it in another queue family. This option offers the best performance.
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount = 0; // Optional
            createInfo.pQueueFamilyIndices = nullptr; // Optional
        }

        createInfo.preTransform = swapChainSupport.Capabilities.currentTransform;
        // Blending with other windows; no thanks.
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

        createInfo.presentMode = _presentMode;
        createInfo.clipped = VK_TRUE;

        // This is used for swap chain invalidation (typically resize)
        createInfo.oldSwapchain = VK_NULL_HANDLE;

        if (vkCreateSwapchainKHR(*_device->GetInstance()->GetLogicalDevice(), &createInfo, nullptr, &_swapChain) != VK_SUCCESS)
            throw std::runtime_error("Failed to create swap chain!");
    }

    SwapChain::~SwapChain()
    {
        delete _device;
    }

    void SwapChain::_SelectFormat()
    {
        auto& supportDetails = _device->GetSwapChainSupportDetails();
        if (supportDetails.Formats.size() == 1 && supportDetails.Formats[0].format == VK_FORMAT_UNDEFINED)
        {
            _surfaceFormat.format = VK_FORMAT_B8G8R8A8_UNORM;
            _surfaceFormat.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
            return;
        }

        for (const auto& availableFormat : supportDetails.Formats)
        {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            {
                _surfaceFormat = availableFormat;
                return;
            }
        }

        _surfaceFormat = supportDetails.Formats[0];
    }

    void SwapChain::_SelectPresentMode()
    {
        // Some drivers currently don't properly support VK_PRESENT_MODE_FIFO_KHR, so we should prefer
        // VK_PRESENT_MODE_IMMEDIATE_KHR if VK_PRESENT_MODE_MAILBOX_KHR is not available.

        VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR;

        for (const auto& availablePresentMode : _device->GetSwapChainSupportDetails().PresentModes)
        {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
            {
                _presentMode = availablePresentMode;
                return;
            }
            else if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR)
                bestMode = availablePresentMode;
        }

        _presentMode = bestMode;
    }

    void SwapChain::_SelectExtent()
    {
        auto& capabilities = _device->GetSwapChainSupportDetails().Capabilities;

        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            _extent = capabilities.currentExtent;
        }
        else
        {
            // Match window resolution
            VkExtent2D actualExtent = { _device->GetSurface()->GetWidth() , _device->GetSurface()->GetHeight() };

            actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
            actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

            _extent = actualExtent;
        }
    }
}
