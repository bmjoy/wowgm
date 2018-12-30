#pragma once

#include <mutex>

namespace gfx::vk
{
    class Instance;
    class Device;
    class Swapchain;
    class Pipeline;
    class RenderPass;
    class Framebuffer;
    class CommandBuffer;
}

namespace wowgm
{
    class Renderer
    {
    public:
        Renderer(gfx::vk::Swapchain* swapchain);
        virtual ~Renderer();

        /// Implement this method if you need to add a subpass to the provided renderpass. This is typically always the case.
        virtual void initializeRenderPass(gfx::vk::RenderPass* renderPass) = 0;

        /// Implement this method when you need to create your rendering pipeline.
        virtual void initializePipeline() = 0;

        /// This method is called before the render pass begins.
        virtual void beforeRenderQuery(gfx::vk::CommandBuffer* buffer) = 0;

        /// This method is called when the render pass has started.
        virtual void onRenderQuery(gfx::vk::CommandBuffer* buffer) = 0;

        /// This method is called after the render pass has stopped recording render queries.
        virtual void afterRenderQuery(gfx::vk::CommandBuffer* buffer) = 0;

        Renderer(Renderer&&) = delete;
        Renderer(Renderer const&) = delete;

        Renderer& operator = (Renderer const&) = delete;
        Renderer& operator = (Renderer&&) = delete;

    protected:
        gfx::vk::Instance*  GetInstance() const;
        gfx::vk::Device*    GetDevice() const;
        gfx::vk::Swapchain* GetSwapchain() const;

        virtual gfx::vk::Pipeline*  GetPipeline() const = 0;
    private:
        gfx::vk::Swapchain* _swapchain;
    };
}