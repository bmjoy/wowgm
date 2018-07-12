#include "ImageView.hpp"
#include "Image.hpp"
#include "LogicalDevice.hpp"
#include "SwapChain.hpp"

#include <stdexcept>

namespace wowgm::graphics
{
    ImageView::ImageView(SwapChain* swapChain, Image* image) : _swapChain(swapChain)
    {
        VkImageViewCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = *image;

        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = swapChain->GetSurfaceFormat().format;

        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(*swapChain->GetLogicalDevice(), &createInfo, nullptr, &_imageView) != VK_SUCCESS)
            throw std::runtime_error("Failed to create an image view!");
    }

    ImageView::~ImageView()
    {
        vkDestroyImageView(*_swapChain->GetLogicalDevice(), _imageView, nullptr);
        _imageView = VK_NULL_HANDLE;
    }

    SwapChain* ImageView::GetSwapChain()
    {
        return _swapChain;
    }

    VkImageView ImageView::GetVkImageView()
    {
        return _imageView;
    }
}
