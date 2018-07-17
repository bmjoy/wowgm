#pragma once

#include <vulkan/vulkan.h>

namespace wowgm::graphics
{
    class Image;
    class SwapChain;

    class ImageView
    {
        ImageView(ImageView&&) = delete;
        ImageView(const ImageView&) = delete;

    public:
        ImageView(SwapChain* swapChain, Image* image);
        ~ImageView();

        operator VkImageView() const { return _imageView; }

        SwapChain* GetSwapChain();

    private:
        VkImageView _imageView;

        SwapChain* _swapChain;
    };
}
