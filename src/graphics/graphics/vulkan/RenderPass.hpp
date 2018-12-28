#pragma once

#include <vulkan/vulkan.h>

#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <cstdint>
#include <list>

namespace gfx::vk
{
    enum class AttachmentType
    {
        Color,
        Input,
        // TODO: NYI below
        Resolve,
        DepthStencil,
        Preserve
    };

    class Device;

    /**
     * Usage (because by the end of the week it won't be obvious to me)
     *
     * Device* device = ...;
     * RenderPass* renderPass = new RenderPass(device);
     * uint32_t attachmentIndex = renderPass->AddAttachment(AttachmentType::Color, VkAttachmentDescription { ... });
     *
     * ...
     *
     * renderPass->BeginSubpass();
     * renderPass->AddAttachmentReference(AttachmentType::Color, attachmentIndex, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
     * renderPass->FinalizeSubpass();
     * renderPass->Finalize();
     *
     */
    class RenderPass
    {
    public:
        RenderPass(Device* device);
        ~RenderPass();

        uint32_t GetSubpassCount() const { return _subpasses.size(); }

        VkRenderPass GetHandle();

        /**
        * Add an attachment to the render pass..
        *
        * @returns the index of the newly created attachment.
        */
        uint32_t AddAttachment(AttachmentType type, VkAttachmentDescription attachmentDescription);

        /**
         * Add a reference to the given attachment for the current subpass.
         *
         * @param attachmentIndex The index of the attachment as returned by AddAttachment.
         * @param layout Specifies which layout we would like the attachment to have during a subpass that uses this reference.
         *               Vulkan will automatically transition the attachment to this layout when the subpass is started.
         */
        void AddAttachmentReference(AttachmentType type, uint32_t attachmentIndex, VkImageLayout layout);

        void Finalize();

        void BeginSubpass();
        void FinalizeSubpass();

    private:

        Device* _device = nullptr;

        VkRenderPass _handle = VK_NULL_HANDLE;

        using attachment_reference_map = std::unordered_map<AttachmentType, std::vector<VkAttachmentReference>>;
        using attachment_map = std::unordered_map<AttachmentType, std::vector<VkAttachmentDescription>>;

        using SubpassInfo = std::pair<VkSubpassDescription, attachment_reference_map>;

        SubpassInfo* _currentSubpass = nullptr;

        attachment_map _attachments;
        std::vector<SubpassInfo> _subpasses;

    };
}