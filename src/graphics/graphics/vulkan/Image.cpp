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
        imageObject->_createFormat = pImageCreateInfo->format;

#if _DEBUG
        if (pImageCreateInfo->pImageName != nullptr)
            imageObject->SetName(pImageCreateInfo->pImageName);
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

    VkImageAspectFlags Image::GetImageAspectFlags() const
    {
        switch (GetCreateFormat())
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
