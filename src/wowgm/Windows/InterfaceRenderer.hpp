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
    class InterfaceRenderer : public Renderer
    {
    public:
        InterfaceRenderer(gfx::vk::Swapchain* device);
        ~InterfaceRenderer();

        void initializePipeline() override;
        void initializeRenderPass(gfx::vk::RenderPass* renderPass) override;

        void onRenderQuery(gfx::vk::CommandBuffer* commandBuffer) override;

        void beforeRenderQuery(gfx::vk::CommandBuffer* buffer) { }
        void afterRenderQuery(gfx::vk::CommandBuffer* buffer) { }

        gfx::vk::Pipeline* GetPipeline() const override;

    private:
        gfx::vk::Pipeline* _pipeline;
        gfx::vk::RenderPass* _renderPass;
    };
}
