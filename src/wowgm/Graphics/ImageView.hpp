#pragma once

#include <vulkan/vulkan.h>

namespace wowgm::graphics
{
    class Image;
    class SwapChain;

    class ImageView
    {
    public:
        ImageView(SwapChain* swapChain, Image* image);
        ~ImageView();

        VkImageView GetVkImageView();
        operator VkImageView() const { return _imageView; }

        SwapChain* GetSwapChain();

    private:
        VkImageView _imageView;

        SwapChain* _swapChain;
    };
}
