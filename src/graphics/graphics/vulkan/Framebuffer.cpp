#include <graphics/vulkan/Framebuffer.hpp>
#include <graphics/vulkan/ImageView.hpp>
#include <graphics/vulkan/RenderPass.hpp>
#include <graphics/vulkan/Device.hpp>

#include <boost/iterator/transform_iterator.hpp>

namespace gfx::vk
{
    VkResult Framebuffer::Create(Device* pDevice, const FramebufferCreateInfo* pCreateInfo, Framebuffer** ppFramebuffer)
    {
        std::vector<VkImageView> attachments(pCreateInfo->attachments.size());
        for (uint32_t i = 0; i < pCreateInfo->attachments.size(); ++i)
            attachments[i] = pCreateInfo->attachments[i]->GetHandle();

        // Create a Framebuffer class instance.
        Framebuffer* framebuffer = new Framebuffer;
        framebuffer->_device = pDevice;
        framebuffer->_width = pCreateInfo->width;
        framebuffer->_height = pCreateInfo->height;
        framebuffer->_layers = pCreateInfo->layers;
        framebuffer->_attachments = std::move(pCreateInfo->attachments);

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
        BOOST_ASSERT_MSG(result == VK_SUCCESS, "Error creating a framebuffer");

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
