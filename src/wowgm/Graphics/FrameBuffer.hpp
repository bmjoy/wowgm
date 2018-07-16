#pragma once

#include <vulkan/vulkan.h>

#include <vector>

namespace wowgm::graphics
{
    class RenderPass;
    class ImageView;
    class SwapChain;

    class FrameBuffer
    {
    public:

        ~FrameBuffer();

        void Finalize();

        operator VkFramebuffer() const { return _frameBuffer; }

    private:
        VkFramebuffer _frameBuffer;

        SwapChain* _swapChain;
        RenderPass* _renderPass;
        std::vector<ImageView*> _attachments;

    };
}
