#pragma once

#include <vulkan/vulkan.h>

#include <vector>

namespace wowgm::graphics
{
    class SwapChain;
    class Subpass;

    class RenderPass
    {
    public:
        RenderPass(SwapChain* swapChain);
        ~RenderPass();

        RenderPass(RenderPass&&) = delete;
        RenderPass(RenderPass const&) = delete;

        void Finalize();

        void AddSubpassDependency(VkSubpassDependency dependency);

        void AddSubpass(Subpass* subpass);

        operator VkRenderPass() const { return _renderPass; }

    private:
        void _CreateDefaultSubpass();

        VkRenderPass _renderPass;

        SwapChain* _swapchain;

        std::vector<VkSubpassDependency> _subpassDependencies;
        std::vector<Subpass*> _subpasses;

        std::vector<VkAttachmentDescription> _attachmentDescriptions;
    };
}
