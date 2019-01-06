#pragma once

#include "Renderer.hpp"

#include <vector>
#include <vulkan/vulkan.h>

#include <extstd/containers/iterators/ring_iterator.hpp>

#include <imgui/imgui.h>

namespace gfx::vk {
    class Swapchain;
    class Pipeline;
    class RenderPass;
    class CommandBuffer;
    class Buffer;
    class Image;
    class ImageView;
}

namespace wowgm
{
    class Window;

    class InterfaceRenderer : public Renderer
    {
    public:
        InterfaceRenderer(gfx::vk::Swapchain* device);
        ~InterfaceRenderer();

        void initializePipeline(wowgm::Window* window) override;
        void initializeRenderPass(gfx::vk::RenderPass* renderPass) override;

        void onRenderQuery(gfx::vk::CommandBuffer* commandBuffer) override;

        void beforeRenderQuery(gfx::vk::CommandBuffer* buffer) override;
        void afterRenderQuery(gfx::vk::CommandBuffer* buffer, std::vector<std::pair<VkSemaphore, VkPipelineStageFlags>>& waitSemaphores) override;

        gfx::vk::Pipeline* GetPipeline() const override;

    private:
        void _RenderInterface();
        void UploadData(gfx::vk::CommandBuffer* frameCommandBuffer, std::vector<ImDrawVert> const& vertices, std::vector<ImDrawIdx> const& indices);

        gfx::vk::Pipeline* _pipeline = nullptr;
        gfx::vk::RenderPass* _renderPass = nullptr;

        gfx::vk::CommandBuffer* _transferCommandBuffer = nullptr;

        gfx::vk::Buffer* _vertexBuffer = nullptr;
        gfx::vk::Buffer* _indexBuffer = nullptr;
        gfx::vk::Buffer* _stagingBuffer = nullptr;

        gfx::vk::Image* _fontTexture = nullptr;
        gfx::vk::ImageView* _fontTextureView = nullptr;

        // This boolean controls if we need to wait for the transfer semaphore
        // at the top of the pipe. This is needed because we would be waiting
        // forever should the UI not get dirty, so the logical condition on
        // using the semaphore goes as follow:
        // - Do we need to transfer data from the staging buffer into the vertices
        //   or indices buffer?
        // and
        // - We only do it if the transfer and the graphics queue are different.
        bool _waitNeeded = false;
        VkSemaphore _transferSemaphore = VK_NULL_HANDLE;

        uint32_t _vertexCount = 0;
        uint32_t _indexCount = 0;

        std::unordered_map<std::string, ImFont*> _fontStore;
    };
}
