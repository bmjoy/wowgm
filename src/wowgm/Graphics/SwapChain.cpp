#include "SwapChain.hpp"
#include "PhysicalDevice.hpp"
#include "Surface.hpp"
#include "Instance.hpp"
#include "LogicalDevice.hpp"
#include "Image.hpp"
#include "ImageView.hpp"
#include "Assert.hpp"
#include "FrameBuffer.hpp"
#include "Pipeline.hpp"

#undef min
#undef max

#include <algorithm>

namespace wowgm::graphics
{
    SwapChain::SwapChain(PhysicalDevice* device) : _physicalDevice(device)
    {
        _SelectFormat();
        _SelectPresentMode();
        _SelectExtent();

        auto& capabilities = _physicalDevice->GetCapabilities();

        // Enable triple buffering
        uint32_t imageCount = capabilities.minImageCount + 1;
        if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount)
            imageCount = capabilities.maxImageCount;

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
        QueueFamilyIndices& indices = _physicalDevice->GetQueues();
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

        createInfo.preTransform = capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = _presentMode;
        createInfo.clipped = VK_TRUE;
        createInfo.oldSwapchain = VK_NULL_HANDLE;

        if (vkCreateSwapchainKHR(*_physicalDevice->GetInstance()->GetLogicalDevice(), &createInfo, nullptr, &_swapChain) != VK_SUCCESS)
            wowgm::exceptions::throw_with_trace(std::runtime_error("Failed to create swap chain!"));

        vkGetSwapchainImagesKHR(*_physicalDevice->GetInstance()->GetLogicalDevice(), _swapChain, &imageCount, nullptr);

        std::vector<VkImage> swapChainImages(imageCount);
        vkGetSwapchainImagesKHR(*_physicalDevice->GetInstance()->GetLogicalDevice(), _swapChain, &imageCount, swapChainImages.data());

        _swapChainImages.resize(imageCount);
        _imageViews.resize(imageCount);
        for (std::uint32_t i = 0; i < imageCount; ++i)
        {
            _swapChainImages[i] = new Image(swapChainImages[i]);
            _imageViews[i] = new ImageView(this, _swapChainImages[i]);
        }
    }

    SwapChain::~SwapChain()
    {
        vkDestroySwapchainKHR(*_physicalDevice->GetInstance()->GetLogicalDevice(), _swapChain, nullptr);
        _swapChain = VK_NULL_HANDLE;

        for (std::uint32_t i = 0; i < _swapChainImages.size(); ++i)
        {
            delete _swapChainImages[i];
            delete _imageViews[i];
            _swapChainImages[i] = nullptr;
            _imageViews[i] = nullptr;
        }

        _swapChainImages.clear();
        _imageViews.clear();
    }

    void SwapChain::_SelectFormat()
    {
        auto& supportDetails = _physicalDevice->GetFormats();
        if (supportDetails.size() == 1 && supportDetails[0].format == VK_FORMAT_UNDEFINED)
        {
            _surfaceFormat.format = VK_FORMAT_B8G8R8A8_UNORM;
            _surfaceFormat.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
            return;
        }

        for (const auto& availableFormat : supportDetails)
        {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            {
                _surfaceFormat = availableFormat;
                return;
            }
        }

        _surfaceFormat = supportDetails[0];
    }

    void SwapChain::_SelectPresentMode()
    {
        // Some drivers currently don't properly support VK_PRESENT_MODE_FIFO_KHR, so we should prefer
        // VK_PRESENT_MODE_IMMEDIATE_KHR if VK_PRESENT_MODE_MAILBOX_KHR is not available.

        VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR;

        for (const auto& availablePresentMode : _physicalDevice->GetPresentModes())
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
        auto& capabilities = _physicalDevice->GetCapabilities();

        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
        {
            _extent = capabilities.currentExtent;
        }
        else
        {
            // Match window resolution
            VkExtent2D actualExtent = { _physicalDevice->GetSurface()->GetWidth() , _physicalDevice->GetSurface()->GetHeight() };

            actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
            actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

            _extent = actualExtent;
        }
    }

    VkSurfaceFormatKHR SwapChain::GetSurfaceFormat()
    {
        return _surfaceFormat;
    }

    VkPresentModeKHR SwapChain::GetPresentMode()
    {
        return _presentMode;
    }

    VkExtent2D SwapChain::GetExtent()
    {
        return _extent;
    }

    PhysicalDevice* SwapChain::GetPhysicalDevice()
    {
        return _physicalDevice;
    }

    LogicalDevice* SwapChain::GetLogicalDevice()
    {
        return _physicalDevice->GetInstance()->GetLogicalDevice();
    }

    std::uint32_t SwapChain::GetImageCount()
    {
        return _swapChainImages.size();
    }

    Image* SwapChain::GetImage(std::uint32_t index)
    {
        return _swapChainImages[index];
    }

    ImageView* SwapChain::GetImageView(std::uint32_t index)
    {
        return _imageViews[index];
    }

    std::vector<ImageView*>& SwapChain::GetImageViews()
    {
        return _imageViews;
    }
}
