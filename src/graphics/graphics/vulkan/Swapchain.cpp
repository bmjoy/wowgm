#include <graphics/vulkan/Swapchain.hpp>
#include <graphics/vulkan/Device.hpp>
#include <graphics/vulkan/PhysicalDevice.hpp>
#include <graphics/vulkan/Image.hpp>

#include <limits>
#include <algorithm>

namespace gfx::vk
{
    static SwapchainSupport QuerySwapchainSupport(VkPhysicalDevice device, VkSurfaceKHR surface)
    {
        SwapchainSupport support;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &support.capabilities);

        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
        if (formatCount > 0)
        {
            support.formats.resize(formatCount);
            auto result = vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, support.formats.data());
        }

        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
        if (presentModeCount > 0)
        {
            support.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, support.presentModes.data());
        }

        return support;
    }

    static VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats, VkFormat format, VkColorSpaceKHR colorSpace)
    {
        if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED)
            return { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };

        for (const auto& availableFormat : availableFormats)
        {
            if (availableFormat.format == format && availableFormat.colorSpace == colorSpace)
                return availableFormat;
        }

        return availableFormats[0];
    }

    static VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, uint32_t clientWidth, uint32_t clientHeight)
    {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
            return capabilities.currentExtent;
        else
        {
            VkExtent2D actualExtent = { clientWidth, clientHeight };
            actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
            actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));
            return actualExtent;
        }
    }

    static VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes, bool vsyncEnabled)
    {
        // Try to match the correct present mode to the vsync state.
        std::vector<VkPresentModeKHR> desiredModes;
        if (vsyncEnabled)
            desiredModes = { VK_PRESENT_MODE_FIFO_KHR, VK_PRESENT_MODE_FIFO_RELAXED_KHR };
        else
            desiredModes = { VK_PRESENT_MODE_IMMEDIATE_KHR, VK_PRESENT_MODE_MAILBOX_KHR };

        // Iterate over all available present mdoes and match to one of the desired ones.
        for (const auto& availablePresentMode : availablePresentModes)
        {
            for (auto mode : desiredModes)
            {
                if (availablePresentMode == mode)
                    return availablePresentMode;
            }
        }

        // If no match was found, return the first present mode or default to FIFO.
        if (availablePresentModes.size() > 0)
            return availablePresentModes[0];
        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkResult Swapchain::Create(Device* pDevice, const SwapchainCreateInfo* pCreateInfo, Swapchain** ppSwapchain)
    {
        // Determine WSI support.
        VkBool32 supported = VK_FALSE;
        VkResult result = vkGetPhysicalDeviceSurfaceSupportKHR(pDevice->GetPhysicalDevice()->GetHandle(), 0, pCreateInfo->surface, &supported);
        if (result != VK_SUCCESS)
            return result;

        if (!supported)
            return VK_ERROR_INCOMPATIBLE_DISPLAY_KHR;

        // Query swapchain support on specified device and newly created surface.
        auto swapchainSupport = QuerySwapchainSupport(pDevice->GetPhysicalDevice()->GetHandle(), pCreateInfo->surface);
        if (swapchainSupport.formats.empty() || swapchainSupport.presentModes.empty())
            return VK_ERROR_INCOMPATIBLE_DISPLAY_KHR;

        // Initialize Swapchain class.
        auto swapchain = new Swapchain();
        swapchain->_device = pDevice;
        memcpy(&swapchain->_createInfo, pCreateInfo, sizeof(SwapchainCreateInfo));
        swapchain->_surface = pCreateInfo->surface;
        swapchain->_swapchainSupport = swapchainSupport;
        result = swapchain->Allocate();
        if (result != VK_SUCCESS)
        {
            delete swapchain;
            return result;
        }

        // Return success.
        *ppSwapchain = swapchain;
        return VK_SUCCESS;
    }

    VkResult Swapchain::Allocate()
    {
        // Request current surface properties.
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_device->GetPhysicalDevice()->GetHandle(), _surface, &_swapchainSupport.capabilities);

        // Select the best color format and present mode based on what's available.
        auto surfaceFormat = ChooseSwapSurfaceFormat(_swapchainSupport.formats, _createInfo.format.format, _createInfo.format.colorSpace);
        auto presentMode = ChooseSwapPresentMode(_swapchainSupport.presentModes, true /* m_vsyncEnabled */);

        // Determine the total number of images required.
        uint32_t imageCount = _createInfo.tripleBuffer ? 3 : _swapchainSupport.capabilities.minImageCount + 1;
        if (_swapchainSupport.capabilities.maxImageCount > 0 && imageCount > _swapchainSupport.capabilities.maxImageCount)
            imageCount = _swapchainSupport.capabilities.maxImageCount;

        // Get current dimensions of surface.
        auto extent = _swapchainSupport.capabilities.currentExtent;

        // Create Vulkan handle.
        VkSwapchainCreateInfoKHR swapchainCreateInfo = {};
        swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        swapchainCreateInfo.surface = _surface;
        swapchainCreateInfo.minImageCount = imageCount;
        swapchainCreateInfo.imageFormat = surfaceFormat.format;
        swapchainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
        swapchainCreateInfo.imageExtent = extent;
        swapchainCreateInfo.imageArrayLayers = 1;
        swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;

        //! TODO This should be conditional on queue being used
        swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapchainCreateInfo.queueFamilyIndexCount = 1;
        uint32_t queueFamilyIndex = 0;
        swapchainCreateInfo.pQueueFamilyIndices = &queueFamilyIndex;

        swapchainCreateInfo.preTransform = _swapchainSupport.capabilities.currentTransform;
        swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        swapchainCreateInfo.presentMode = presentMode;
        swapchainCreateInfo.clipped = VK_TRUE;
        swapchainCreateInfo.oldSwapchain = _handle;

        VkSwapchainKHR handle = VK_NULL_HANDLE;
        auto result = vkCreateSwapchainKHR(_device->GetHandle(), &swapchainCreateInfo, nullptr, &handle);
        if (result != VK_SUCCESS)
            return result;

        // Save final surface format.
        memcpy(&_format, &surfaceFormat, sizeof(VkSurfaceFormatKHR));

        // Free previous handle.
        if (_handle)
            vkDestroySwapchainKHR(_device->GetHandle(), _handle, nullptr);

        // Copy new swapchain handle.
        _handle = handle;

        // Clear old list of images.
        _images.clear();

        // Get the actual image count after swapchain is created since it could change.
        imageCount = 0U;
        vkGetSwapchainImagesKHR(_device->GetHandle(), _handle, &imageCount, nullptr);

        // Get the swapchain's image handles.
        std::vector<VkImage> images(imageCount);
        vkGetSwapchainImagesKHR(_device->GetHandle(), _handle, &imageCount, &images[0]);

        // Create an Image class instances to wrap swapchain image handles.
        for (auto handle : images)
        {
            ImageCreateInfo imageCreateInfo = {};
            imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
            imageCreateInfo.format = _createInfo.format.format;
            imageCreateInfo.extent = { extent.width, extent.height, 1 };
            imageCreateInfo.mipLevels = 1;
            imageCreateInfo.arrayLayers = 1;
            imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
            imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
            imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;

            auto image = Image::CreateFromHandle(_device, &imageCreateInfo, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, handle, nullptr);
            _images.push_back(image);

            // image->TransitionToLayout(VK_IMAGE_LAYOUT_PRESENT_SRC_KHR); // Is this needed?
        }

        // Return success.
        return VK_SUCCESS;
    }

    uint32_t Swapchain::GetImageIndex(Image* image)
    {
        uint32_t idx = 0;

        for (auto&& itr : _images)
        {
            if (itr->GetHandle() == image->GetHandle())
                return idx;

            ++idx;
        }
        return -1;
    }
}