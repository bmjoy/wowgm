#pragma once

#include "RenderPass.hpp"

#include <vector>
#include <vulkan/vulkan.h>

namespace wowgm::graphics
{
    class ImageView;
    class SwapChain;

    class FrameBuffer
    {
        friend FrameBuffer* RenderPass::CreateFrameBuffer(SwapChain* swapChain);
        FrameBuffer(RenderPass* renderPass, SwapChain* swapChain);

        FrameBuffer(FrameBuffer&&) = delete;
        FrameBuffer(const FrameBuffer&) = delete;

    public:
        ~FrameBuffer();

        void Finalize();

        operator VkFramebuffer() const { return _frameBuffer; }

        void AttachImageView(ImageView* imageView);
        std::uint32_t GetAttachmentCount();

        RenderPass* GetRenderPass();

    private:
        VkFramebuffer _frameBuffer;

        SwapChain* _swapChain;
        RenderPass* _renderPass;
        std::vector<ImageView*> _attachments;

    };
}
