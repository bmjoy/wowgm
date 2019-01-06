#include <graphics/vulkan/Swapchain.hpp>
#include <graphics/vulkan/Device.hpp>
#include <graphics/vulkan/PhysicalDevice.hpp>
#include <graphics/vulkan/Image.hpp>
#include <graphics/vulkan/ImageView.hpp>
#include <graphics/vulkan/Framebuffer.hpp>

#include <shared/assert/assert.hpp>

#include <limits>
#include <algorithm>

#ifdef max
#undef max
#undef min
#endif

namespace gfx::vk
{
    Swapchain::~Swapchain()
    {
        for (auto&& itr : _imageViews)
            vkDestroyImageView(_device->GetHandle(), itr->GetHandle(), nullptr);

        vkDestroySwapchainKHR(_device->GetHandle(), GetHandle(), nullptr);
    }

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

        // Iterate over all available present modes and match to one of the desired ones.
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

    Swapchain::Swapchain(Device* pDevice, const SwapchainCreateInfo* pCreateInfo)
    {
        // Determine WSI support.
        VkBool32 supported = VK_FALSE;
        VkResult result = vkGetPhysicalDeviceSurfaceSupportKHR(pDevice->GetPhysicalDevice()->GetHandle(), 0, pCreateInfo->surface, &supported);
        BOOST_ASSERT_MSG(result == VK_SUCCESS, "Failed to retrieve device surface support");
        BOOST_ASSERT_MSG(supported, "Display incompatible with KHR surface");

        // Query swapchain support on specified device and newly created surface.
        auto swapchainSupport = QuerySwapchainSupport(pDevice->GetPhysicalDevice()->GetHandle(), pCreateInfo->surface);
        if (swapchainSupport.formats.empty() || swapchainSupport.presentModes.empty())
            BOOST_ASSERT_MSG(false, "No format or present modes supported");

        _device = pDevice;
        memcpy(&_createInfo, pCreateInfo, sizeof(SwapchainCreateInfo));
        _surface = pCreateInfo->surface;
        _swapchainSupport = swapchainSupport;
        result = Allocate();
        BOOST_ASSERT_MSG(result == VK_SUCCESS, "Failed to allocate a swapchain");
    }

    VkExtent2D const& Swapchain::GetExtent() const
    {
        return _swapchainSupport.capabilities.currentExtent;
    }

    VkResult Swapchain::Allocate()
    {
        // Request current surface properties.
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_device->GetPhysicalDevice()->GetHandle(), _surface, &_swapchainSupport.capabilities);

        // Select the best color format and present mode based on what's available.
        auto surfaceFormat = ChooseSwapSurfaceFormat(_swapchainSupport.formats, _createInfo.preferredFormat.format, _createInfo.preferredFormat.colorSpace);
        auto presentMode = ChooseSwapPresentMode(_swapchainSupport.presentModes, false /* m_vsyncEnabled */);

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
        swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

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

        { // Clear previous image(view)s
            for (auto&& itr : _images)
                vkDestroyImage(_device->GetHandle(), itr->GetHandle(), nullptr);

            for (auto&& itr : _imageViews)
                vkDestroyImageView(_device->GetHandle(), itr->GetHandle(), nullptr);

            _images.clear();
            _imageViews.clear();
        }

        // Get the actual image count after swapchain is created since it could change.
        imageCount = 0U;
        vkGetSwapchainImagesKHR(_device->GetHandle(), _handle, &imageCount, nullptr);
        std::vector<VkImage> images(imageCount);
        vkGetSwapchainImagesKHR(_device->GetHandle(), _handle, &imageCount, &images[0]);

        // Create an Image class instances to wrap swapchain image handles.
        for (auto handle : images)
        {
            ImageCreateInfo imageCreateInfo = {};
            imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
            imageCreateInfo.format = surfaceFormat.format;
            imageCreateInfo.extent = { extent.width, extent.height, 1 };
            imageCreateInfo.mipLevels = 1;
            imageCreateInfo.arrayLayers = 1;
            imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
            imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
            imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;

            auto image = Image::CreateFromHandle(_device, &imageCreateInfo, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, handle, nullptr);

            ImageViewCreateInfo imageViewCreateInfo{};
            imageViewCreateInfo.image = image;
            imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
            imageViewCreateInfo.subresourceRange.levelCount = 1;
            imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
            imageViewCreateInfo.subresourceRange.layerCount = 1;

            ImageView* imageView = GetDevice()->CreateImageView(&imageViewCreateInfo);
            if (imageView == nullptr)
                shared::assert::throw_with_trace("Unable to create a view into one of the images of the swapchain");

            _images.push_back(image);
            _imageViews.push_back(imageView);
        }

        // Return success.
        return VK_SUCCESS;
    }

    Framebuffer* Swapchain::CreateFrameBuffer(uint32_t frameIndex, RenderPass* renderPass)
    {
        FramebufferCreateInfo framebufferCreateInfo{};
        framebufferCreateInfo.pRenderPass = renderPass;
        framebufferCreateInfo.layers = 1;
        framebufferCreateInfo.width = GetExtent().width;
        framebufferCreateInfo.height = GetExtent().height;
        framebufferCreateInfo.attachments.push_back(_imageViews[frameIndex]);

        Framebuffer* frameBuffer = nullptr;
        VkResult result = Framebuffer::Create(_device, &framebufferCreateInfo, &frameBuffer);
        if (result != VK_SUCCESS)
            shared::assert::throw_with_trace("Unable to create a framebuffer for frame {}", frameIndex);

        return frameBuffer;
    }

    Image* Swapchain::GetImage(uint32_t index)
    {
        return _images[index];
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

    ImageView* Swapchain::GetImageView(uint32_t index)
    {
        return _imageViews[index];
    }
}