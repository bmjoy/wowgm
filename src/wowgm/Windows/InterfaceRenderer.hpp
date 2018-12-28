#pragma once

#include "Renderer.hpp"

namespace gfx::vk {
    class Swapchain;
    class Pipeline;
    class RenderPass;
}

namespace wowgm
{
    class InterfaceRenderer final : public Renderer
    {
    public:
        InterfaceRenderer(gfx::vk::Swapchain* device);
        ~InterfaceRenderer();

        void Render() override;

        gfx::vk::Pipeline* GetPipeline() const override;

    private:
        gfx::vk::Pipeline* _pipeline;
        gfx::vk::RenderPass* _renderPass;
    };
}
