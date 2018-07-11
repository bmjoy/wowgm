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

        VkImageView GetVkImageView();
        operator VkImageView() const { return _imageView; }

    private:
        VkImageView _imageView;
    };
}
