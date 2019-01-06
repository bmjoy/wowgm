#pragma once

#include <graphics/vulkan/VK.hpp>

namespace gfx::vk
{
    class Device;

    class ImageView
    {
    public:
        ~ImageView();

        Device* GetDevice() const { return _device; }
        VkImageView GetHandle() const { return _handle; }

    private:
        ImageView() { }

        friend class Device;

        VkImageView _handle = VK_NULL_HANDLE;
        Device* _device = nullptr;
        Image* _image = nullptr;

        VkImageViewType _viewType;
        VkFormat _format;

        VkComponentMapping _components;
        VkImageSubresourceRange _subresourceRange;
    };
}
