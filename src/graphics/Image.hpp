#pragma once

#include "VEZ.hpp"
#include "NamedObject.hpp"

namespace vez
{
    class Device;

    class Image : public NamedObject<Image>
    {
    public:
        static Image* CreateFromHandle(Device* device, const ImageCreateInfo* pImageCreateInfo, VkImageLayout imageLayout, VkImage image, VmaAllocation allocation);

        VkImage GetHandle() const { return _handle; }

        VmaAllocation GetAllocation() const { return _allocation; }

        const ImageCreateInfo& GetCreateInfo() const { return _createInfo; }

        VkImageLayout GetLayout() { return _imageLayout; }

        Instance* GetInstance() const override;

        Device* GetDevice() const override;

        VkResult TransitionToLayout(VkImageLayout targetLayout, VkPipelineStageFlagBits startStage, VkPipelineStageFlagBits endStage, VkDependencyFlags dependencyFlag);

        VkImageAspectFlags GetImageAspectFlags() const;

    private:
        VkImage _handle = VK_NULL_HANDLE;
        Device* _device = nullptr;
        VmaAllocation _allocation;
        VkImageLayout _imageLayout = VK_IMAGE_LAYOUT_GENERAL;
        ImageCreateInfo _createInfo;
    };
}