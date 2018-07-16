#pragma once

#include <vulkan/vulkan.h>

#include <vector>
#include <cstdint>

namespace wowgm::graphics
{
    class Subpass
    {
    public:
        Subpass(VkPipelineBindPoint bindPoint);
        ~Subpass();

        Subpass(Subpass&&) = delete;
        Subpass(Subpass const&) = delete;

        void Finalize();

        operator VkSubpassDescription() const { return _subpassDescription; }

        void AddInput(VkAttachmentReference ref);
        void AddColor(VkAttachmentReference color);
        void AddResolve(VkAttachmentReference resolve);

    private:
        VkSubpassDescription _subpassDescription;

        VkPipelineBindPoint _bindPoint;

        /// A collection of attachments from which we will read data during the subpass.
        std::vector<VkAttachmentReference> _inputAttachments;

        /// Attachments that will be used as color attachments (into which we will render during the subpass).
        std::vector<VkAttachmentReference> _colorAttachments;

        /// Which color attachments should be resolved (changed from a multisampled image to a
        /// non-multisampled/single sampled image) at the end of the subpass.
        std::vector<VkAttachmentReference> _resolveAttachments;

        /// Which attachment is used as a depth and/or stencil attachment during the subpass.
        VkAttachmentReference const* _depthStencilAttachment;

        /// A set of attachments that are not used in the subpass but whose contents must be preserved
        /// during the whole subpass.
        std::vector<std::uint32_t> _preserveAttachments;
    };
}