#pragma once

#include <vulkan/vulkan.h>

#include <vector>

namespace wowgm::graphics
{
    class LogicalDevice;
    class Subpass;

    /*
     * A render pass contains subpasses and attachments. It describes the structure
     * of the drawing work to the driver, how data will flow between the various
     * attachments or what the ordering requirements are; and runtime behavior, such
     * as how these attachments will be treated at each load or wether it needs to be
     * clear or preserve information.
     */
    class RenderPass
    {
        RenderPass(RenderPass&&) = delete;
        RenderPass(RenderPass const&) = delete;

    public:

        RenderPass(LogicalDevice* device);
        ~RenderPass();

        void Finalize();

        void SetDependency(VkSubpassDependency dependency);

        std::uint32_t Insert(Subpass* subpass);

        void AddAttachment(VkAttachmentDescription attachment);

        operator VkRenderPass() const { return _renderPass; }

    private:
        void _CreateDefaultSubpass();

        VkRenderPass _renderPass;

        LogicalDevice* _device;

        std::vector<VkSubpassDependency> _subpassDependencies;
        std::vector<Subpass*> _subpasses;

        std::vector<VkAttachmentDescription> _attachmentDescriptions;
    };
}
