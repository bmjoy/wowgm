#include "Framebuffer.hpp"
#include "ImageView.hpp"
#include "RenderPass.hpp"
#include "Device.hpp"

#include <boost/iterator/transform_iterator.hpp>

namespace gfx::vk
{
    VkResult Framebuffer::Create(Device* pDevice, const FramebufferCreateInfo* pCreateInfo, Framebuffer** ppFramebuffer)
    {
        std::vector<ImageView*> attachmentObjects(pCreateInfo->attachmentCount);
        std::vector<VkImageView> attachments(pCreateInfo->attachmentCount);
        for (uint32_t i = 0; i < pCreateInfo->attachmentCount; ++i)
        {
            attachments.push_back(pCreateInfo->ppAttachments[i]->GetHandle());
            attachmentObjects.push_back(const_cast<ImageView*>(pCreateInfo->ppAttachments[i]));
        }
        // Create a Framebuffer class instance.
        Framebuffer* framebuffer = new Framebuffer;
        framebuffer->_device = pDevice;
        framebuffer->_width = pCreateInfo->width;
        framebuffer->_height = pCreateInfo->height;
        framebuffer->_layers = pCreateInfo->layers;
        framebuffer->_attachments = std::move(attachmentObjects);

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
