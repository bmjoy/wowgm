#pragma once

#include <string>
#include <vulkan/vulkan.hpp>

#include "VulkanBase.hpp"

namespace wowgm
{
    class Window : public vkx::BaseWindow
    {
    public:
        vks::Buffer vertices;
        vks::Buffer indices;
        vks::Buffer uniformDataVS;
        uint32_t indexCount { 0 };

        // As before
        vk::DescriptorSet descriptorSet;
        vk::DescriptorSetLayout descriptorSetLayout;
        vk::Pipeline pipeline;
        vk::PipelineLayout pipelineLayout;

        struct Vertex {
            float pos[3];
            float col[3];
        };

        // As before
        struct UboVS {
            glm::mat4 projectionMatrix;
            glm::mat4 modelMatrix;
            glm::mat4 viewMatrix;
        } uniformBufferObject;

        Window(const std::string& title);

        ~Window();

        void Prepare() override;

        // In our previous example, we created a function buildCommandBuffers that did two jobs.  First, it allocated a
        // command buffer for each swapChain image, and then it populated those command buffers with the commands required
        // to render our triangle.
        //
        // Some of this is now done by the base class, which calls this method to populate the actual commands for each
        // swapChain image specific CommandBuffer
        //
        // Note that this method only works if we have a single renderpass, since the parent class calls beginRenderPass
        // and endRenderPass around this method.  If we have multiple render passes then we'd need to override the
        // parent class buildCommandBuffers to do the appropriate work
        //
        // For now, that is left for us to do is to set viewport & scissor regions, bind pipelines, and draw geometry.
        void UpdateDrawCommandBuffer(const vk::CommandBuffer& cmdBuffer) override;

        void PrepareVertices();

        void PrepareUniformBuffers();

        void SetupDescriptorSetLayout();

        void PreparePipelines();

        void SetupDescriptorPool();

        void SetupDescriptorSet();

        void OnUpdateOverlay() override;

    private:
        char _accountName[100] = { 0 };
        char _accountPassword[100] = { 0 };
        char _realmAddress[100] = { 0 };

        char _gameDataLocation[256] = { 0 };
        bool _showOptionsWindow = false;
    };

}