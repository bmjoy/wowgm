#include <graphics/vulkan/Image.hpp>
#include <graphics/vulkan/CommandBuffer.hpp>
#include <graphics/vulkan/Helpers.hpp>
#include <graphics/vulkan/Device.hpp>
#include <graphics/vulkan/Instance.hpp>

namespace gfx::vk
{
    Image* Image::CreateFromHandle(Device* device, const ImageCreateInfo* pImageCreateInfo, VkImageLayout imageLayout, VkImage image, VmaAllocation allocation)
    {
        Image* imageObject = new Image;
        imageObject->_device = device;
        imageObject->_handle = image;
        imageObject->_allocation = allocation;
        imageObject->_imageLayout = imageLayout;
        memcpy(&imageObject->_createInfo, pImageCreateInfo, sizeof(ImageCreateInfo));

#if _DEBUG
        if (pImageCreateInfo->pImageName != nullptr)
            device->GetInstance()->SetObjectName(device, imageObject, pImageCreateInfo->pImageName);
#endif

        return imageObject;
    }

    Instance* Image::GetInstance() const
    {
        return _device->GetInstance();
    }

    Device* Image::GetDevice() const
    {
        return _device;
    }

    VkResult Image::TransitionToLayout(VkImageLayout targetLayout, VkPipelineStageFlagBits startStage, VkPipelineStageFlagBits endStage, VkDependencyFlags dependencyFlag)
    {
        CommandBuffer* currentCommandBuffer = _device->GetCurrentCommandBuffer();
        if (currentCommandBuffer == nullptr)
            currentCommandBuffer = _device->GetOneTimeSubmitCommandBuffer();

        VkImageMemoryBarrier memoryBarrier{};
        memoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        memoryBarrier.srcAccessMask = 0;
        memoryBarrier.dstAccessMask = 0;
        memoryBarrier.oldLayout = GetLayout();
        memoryBarrier.newLayout = targetLayout;
        memoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        memoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        memoryBarrier.image = GetHandle();

        // All mip levels and array layers will be transitioned.
        const auto& imageCreateInfo = GetCreateInfo();
        memoryBarrier.subresourceRange.aspectMask = GetImageAspectFlags();
        memoryBarrier.subresourceRange.baseMipLevel = 0;
        memoryBarrier.subresourceRange.levelCount = imageCreateInfo.mipLevels;
        memoryBarrier.subresourceRange.baseArrayLayer = 0;
        memoryBarrier.subresourceRange.layerCount = imageCreateInfo.arrayLayers;

        currentCommandBuffer->PipelineBarrier(startStage, endStage, dependencyFlag, 0, nullptr, 0, nullptr, 1, &memoryBarrier);
        return VK_SUCCESS;
    }

    VkImageAspectFlags Image::GetImageAspectFlags() const
    {
        switch (GetCreateInfo().format)
        {
            case VK_FORMAT_D16_UNORM:
            case VK_FORMAT_D32_SFLOAT:
                return VK_IMAGE_ASPECT_DEPTH_BIT;

            case VK_FORMAT_D16_UNORM_S8_UINT:
            case VK_FORMAT_D24_UNORM_S8_UINT:
            case VK_FORMAT_D32_SFLOAT_S8_UINT:
                return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;

            default:
                return VK_IMAGE_ASPECT_COLOR_BIT;
        }
    }
}
