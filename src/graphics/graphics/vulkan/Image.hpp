#pragma once

#include <graphics/vulkan/NamedObject.hpp>
#include <graphics/vulkan/VK.hpp>

namespace gfx::vk
{
    class Image final : public NamedObject<Image>
    {
    public:
        static Image* CreateFromHandle(Device* device, const ImageCreateInfo* pImageCreateInfo, VkImageLayout imageLayout, VkImage image, VmaAllocation allocation);

        VkImage GetHandle() const { return _handle; }
        Instance* GetInstance() const;
        Device* GetDevice() const;

        VmaAllocation GetAllocation() const { return _allocation; }

        const VkFormat& GetCreateFormat() const { return _createFormat; }

        VkImageLayout GetLayout() { return _imageLayout; }

        VkImageAspectFlags GetImageAspectFlags() const;

    private:
        VkImage _handle = VK_NULL_HANDLE;
        Device* _device = nullptr;
        VmaAllocation _allocation;
        VkImageLayout _imageLayout = VK_IMAGE_LAYOUT_GENERAL;

        VkFormat _createFormat;
    };
}