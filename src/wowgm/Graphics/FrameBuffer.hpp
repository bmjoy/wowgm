#pragma once

#include <vector>
#include <vulkan/vulkan.h>

namespace wowgm::graphics
{
    class RenderPass;
    class ImageView;
    class SwapChain;

    class FrameBuffer
    {
        FrameBuffer(FrameBuffer&&) = delete;
        FrameBuffer(const FrameBuffer&) = delete;

    public:
        FrameBuffer(RenderPass* renderPass, SwapChain* swapChain);
        ~FrameBuffer();

        void Finalize();

        operator VkFramebuffer() const { return _frameBuffer; }

        void AttachImageView(ImageView* imageView);

        RenderPass* GetRenderPass();

    private:
        VkFramebuffer _frameBuffer;

        SwapChain* _swapChain;
        RenderPass* _renderPass;
        std::vector<ImageView*> _attachments;

    };
}
