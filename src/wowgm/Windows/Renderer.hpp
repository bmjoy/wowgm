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

        virtual void Render() = 0;

        virtual void onFrame(gfx::vk::CommandBuffer* buffer) = 0;

        Renderer(Renderer&&) = delete;
        Renderer(Renderer const&) = delete;
    protected:
        gfx::vk::Instance*  GetInstance() const;
        gfx::vk::Device*    GetDevice() const;
        gfx::vk::Swapchain* GetSwapchain() const;

        virtual gfx::vk::Pipeline*  GetPipeline() const = 0;
    private:
        gfx::vk::Swapchain* _swapchain;
    };
}