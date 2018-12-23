#pragma once

#include <graphics/vulkan/VK.hpp>

namespace gfx::vk
{
    class ImageView
    {
    public:
        static VkResult Create(Device* pDevice, const ImageViewCreateInfo* pCreateInfo, ImageView** ppImageView);

        ~ImageView();

        Device* GetDevice() const { return _device; }
        VkImageView GetHandle() const { return _handle; }

    private:
        VkImageView _handle = VK_NULL_HANDLE;
        Device* _device = nullptr;
        Image* _image = nullptr;

        VkImageViewType _viewType;
        VkFormat _format;

        VkComponentMapping _components;
        VkImageSubresourceRange _subresourceRange;
    };
}
