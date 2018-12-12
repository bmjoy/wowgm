#include "ImageView.hpp"
#include "Image.hpp"
#include "Device.hpp"

namespace vez
{
    VkResult ImageView::Create(Device* pDevice, const ImageViewCreateInfo* pCreateInfo, ImageView** ppImageView)
    {
        VkImageViewCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.pNext = pCreateInfo->pNext;
        createInfo.image = pCreateInfo->image->GetHandle();
        createInfo.viewType = pCreateInfo->viewType;
        createInfo.format = pCreateInfo->format;
        memcpy(&createInfo.components, &pCreateInfo->components, sizeof(VkComponentMapping));
        createInfo.subresourceRange.aspectMask = pCreateInfo->image->GetImageAspectFlags();
        createInfo.subresourceRange.baseMipLevel = pCreateInfo->subresourceRange.baseMipLevel;
        createInfo.subresourceRange.levelCount = pCreateInfo->subresourceRange.levelCount;
        createInfo.subresourceRange.baseArrayLayer = pCreateInfo->subresourceRange.baseArrayLayer;
        createInfo.subresourceRange.layerCount = pCreateInfo->subresourceRange.layerCount;

        VkImageView handle = VK_NULL_HANDLE;
        VkResult result = vkCreateImageView(pDevice->GetHandle(), &createInfo, nullptr, &handle);
        if (result != VK_SUCCESS)
            return result;

        ImageView* pImageView = new ImageView;
        pImageView->_handle = handle;
        pImageView->_device = pDevice;
        pImageView->_image = pCreateInfo->image;
        pImageView->_viewType = pCreateInfo->viewType;
        pImageView->_format = pCreateInfo->format;

        memcpy(&pImageView->_components, &pCreateInfo->components, sizeof(VkComponentMapping));
        memcpy(&pImageView->_subresourceRange, &pCreateInfo->subresourceRange, sizeof(VkImageSubresourceRange));

        *ppImageView = pImageView;
        return VK_SUCCESS;
    }

    ImageView::~ImageView()
    {
        if (_handle != VK_NULL_HANDLE)
            vkDestroyImageView(_device->GetHandle(), _handle, nullptr);

        _handle = VK_NULL_HANDLE;
        _device = nullptr;
        _image = nullptr;
    }
}
