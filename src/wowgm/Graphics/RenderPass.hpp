#pragma once

#include <vulkan/vulkan.h>

#include <vector>

namespace wowgm::graphics
{
    class LogicalDevice;
    class Subpass;

    class RenderPass
    {
        RenderPass(RenderPass&&) = delete;
        RenderPass(RenderPass const&) = delete;

    public:

        RenderPass(LogicalDevice* device);
        ~RenderPass();

        void Finalize();

        void AddSubpassDependency(VkSubpassDependency dependency);

        void AddSubpass(Subpass* subpass);

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
