/*
* UI overlay class using ImGui
*
* Copyright (C) 2017 by Sascha Willems - www.saschawillems.de
*
* This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/

#pragma once

#include "Context.hpp"
#include <string_view>
#include <array>
#include <boost/optional.hpp>

struct ImFont;

namespace vkx { namespace ui {

    struct GUICreateInfo
    {
        vk::Queue copyQueue;
        vk::RenderPass renderPass;
        std::vector<vk::Framebuffer> framebuffers;
        vk::Format colorformat;
        vk::Format depthformat;
        vk::Extent2D size;
        std::vector<vk::PipelineShaderStageCreateInfo> shaders;
        vk::SampleCountFlagBits rasterizationSamples{ vk::SampleCountFlagBits::e1 };
        uint32_t subpassCount{ 1 };
        std::vector<vk::ClearValue> clearValues = {};
        uint32_t attachmentCount = 1;
    };

    class GUI
    {
        private:
            struct _InterfaceElementCreateInfo {
                float PreviousLabelSize[2] = { 0.0f, 0.0f };
                std::string_view Label;

                boost::optional<std::array<float, 4>> TextColor;

                bool Finalized = false;

                std::function<void(_InterfaceElementCreateInfo const&)> Callback;
            } _interfaceElementCreateInfo;

            GUICreateInfo _createInfo;
            const vks::Context& _context;
            vks::Buffer _vertexBuffer;
            vks::Buffer _indexBuffer;
            int32_t _vertexCount = 0;
            int32_t _indexCount = 0;

            vk::DescriptorPool _descriptorPool;
            vk::DescriptorSetLayout _descriptorSetLayout;
            vk::DescriptorSet _descriptorSet;
            vk::PipelineLayout _pipelineLayout;
            vk::Pipeline _pipeline;
            vk::RenderPass _renderPass;
            vk::CommandPool _commandPool;
            vk::Fence _fence;

            vks::Image _font;

            struct PushConstBlock {
                glm::vec2 scale;
                glm::vec2 translate;
            } _pushConstBlock;

            void PrepareResources();
            void PreparePipeline();
            void PrepareRenderPass();
            void UpdateCommandBuffers();

        public:
            std::vector<vk::CommandBuffer> cmdBuffers;

            GUI(const vks::Context& context);
            ~GUI();

            bool IsVisible() const;
            void ToggleVisibility();

        public: // ImGui

            ImFont* GetFont(std::uint32_t index);
            void SelectFont(std::uint32_t index);

            void PrepareTextElement(std::string_view characterSequence);
            void SetTextColor(std::array<float, 4>&& values);
            void FinalizeTextElement();

        public: // Vulkan

            void Create(const GUICreateInfo& createInfo);
            void destroy();

            void update();
            void resize(const vk::Extent2D& newSize, const std::vector<vk::Framebuffer>& framebuffers);

            void submit(const vk::Queue& queue, uint32_t bufferindex, vk::SubmitInfo submitInfo) const;

        private:

            void _FinalizeCurrentInterfaceElement();

            bool _visible = true;
            float _scale = 1.0f;

    };
}}  // namespace vkx::ui
