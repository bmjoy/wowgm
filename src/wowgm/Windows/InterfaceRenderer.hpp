#pragma once

#include "Renderer.hpp"

#include <vector>
#include <vulkan/vulkan.h>

#include <extstd/containers/iterators/ring_iterator.hpp>

namespace gfx::vk {
    class Swapchain;
    class Pipeline;
    class RenderPass;
    class CommandBuffer;
}

namespace wowgm
{
    class InterfaceRenderer final : public Renderer
    {
    public:
        InterfaceRenderer(gfx::vk::Swapchain* device);
        ~InterfaceRenderer();

        void Render() override;

        void Initialize();
        void InitializeRenderPass(gfx::vk::RenderPass* renderPass);

        void onFrame(gfx::vk::CommandBuffer* commandBuffer) override;

        gfx::vk::Pipeline* GetPipeline() const override;

    private:
        gfx::vk::Pipeline* _pipeline;
        gfx::vk::RenderPass* _renderPass;
    };
}
