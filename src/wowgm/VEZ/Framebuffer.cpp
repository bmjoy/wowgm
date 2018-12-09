#include "Framebuffer.hpp"
#include "ImageView.hpp"
#include "Device.hpp"

#include <boost/iterator/transform_iterator.hpp>

namespace vez
{
    VkResult Framebuffer::Create(Device* pDevice, const FramebufferCreateInfo* pCreateInfo, Framebuffer** ppFramebuffer)
    {
        std::vector<ImageView*> attachmentsModels(pCreateInfo->ppAttachments[0], pCreateInfo->ppAttachments[pCreateInfo->attachmentCount]);

        auto mutator = [](ImageView* imageView) -> VkImageView { return imageView->GetHandle(); };
        auto begin = boost::make_transform_iterator(attachmentsModels.begin(), mutator);
        auto end = boost::make_transform_iterator(attachmentsModels.end(), mutator);

        std::vector<VkImageView> attachments(begin, end);

        // Create a Framebuffer class instance.
        Framebuffer* framebuffer = new Framebuffer;
        framebuffer->_device = pDevice;
        framebuffer->_width = pCreateInfo->width;
        framebuffer->_height = pCreateInfo->height;
        framebuffer->_layers = pCreateInfo->layers;
        framebuffer->_attachments = std::move(attachmentsModels);

        VkFramebufferCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        createInfo.pNext = pCreateInfo->pNext;
        createInfo.renderPass = pCreateInfo->pRenderPass->GetHandle();
        createInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        createInfo.pAttachments = attachments.data();
        createInfo.width = framebuffer->_width;
        createInfo.height = framebuffer->_height;
        createInfo.layers = framebuffer->_layers;

        VkResult result = vkCreateFramebuffer(pDevice->GetHandle(), &createInfo, nullptr, &framebuffer->_handle);
        if (result != VK_SUCCESS)
        {
            delete framebuffer;
            return result;
        }

        *ppFramebuffer = framebuffer;
        return VK_SUCCESS;
    }

    Framebuffer::~Framebuffer()
    {
        vkDestroyFramebuffer(_device->GetHandle(), _handle, nullptr);
    }

    VkExtent2D Framebuffer::GetExtents() const
    {
        return { _width, _height };
    }

    ImageView* Framebuffer::GetAttachment(uint32_t attachmentIndex) const
    {
        if (attachmentIndex < _attachments.size())
            return _attachments[attachmentIndex];
        return nullptr;
    }
}
