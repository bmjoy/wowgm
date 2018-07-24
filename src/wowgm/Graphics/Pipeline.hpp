#pragma once

#include <boost/optional/optional.hpp>
#include <vector>
#include <vulkan/vulkan.h>

namespace wowgm::graphics
{
    class Shader;
    class SwapChain;
    class RenderPass;
    class CommandBuffer;

    class Pipeline
    {
        Pipeline(Pipeline&&) = delete;
        Pipeline(Pipeline const&) = delete;

    public:
        Pipeline(SwapChain* swapChain, RenderPass* renderPass);
        ~Pipeline();

        void Finalize();

        bool IsReady();

        void EnableDynamicStates() { _useDynamicState = true; }

        void AddShader(Shader* shader);

        RenderPass* GetRenderPass();

        operator VkPipeline() const { return _pipeline; }

    public: /* Rasterization*/

        VkPipelineRasterizationStateCreateInfo& GetRasterizationStateInfo();

    public: /* DepthStencil */

        VkPipelineDepthStencilStateCreateInfo& GetDepthStencilStateInfo();

    public: /* InputAssembly */

        /*
         * In case of drawing commands that use vertex indices, `restartEnable` specifies wether
         * a special index value should restart a primitive. Defaults to false.
         */
        void SetPrimitiveType(VkPrimitiveTopology topology, bool restartEnable = false);

    public: /* Tessellation */

        void SetTessellationControlPoints(std::uint32_t controlPoints);

    public: /* Viewport and scissors */

        void CreateViewport(std::uint32_t x, std::uint32_t y, std::uint32_t w, std::uint32_t h, float minDepth = 0.0f, float maxDepth = 1.0f);
        void CreateViewport(std::uint32_t w, std::uint32_t h, float minDepth = 0.0f, float maxDepth = 1.0f);

    public: /* Attributes and Bindings */

        void AddVertexBinding(VkVertexInputBindingDescription binding);

        template <typename T>
        void AddVertexBinding(std::uint32_t index, VkVertexInputRate inputRate) {
            AddVertexBinding({ index, sizeof(T), inputRate });
        }

        // TODO: This was needed in OpenGL; is it still needed in Vulkan?
        // template <>
        // void AddVertexBinding<glm::mat4>(std::uint32_t index, VkVertexInputRate inputRate)
        // {
        //     AddVertexBinding({ index, sizeof(glm::vec4), inputRate });
        //     AddVertexBinding({ index + 1, sizeof(glm::vec4), inputRate });
        //     AddVertexBinding({ index + 2, sizeof(glm::vec4), inputRate });
        //     AddVertexBinding({ index + 3, sizeof(glm::vec4), inputRate });
        // }

        void AddVertexAttribute(VkVertexInputAttributeDescription attrDescription);

        void AddPushConstant(VkPushConstantRange constantRange);
        void AddDescriptorSetLayout(VkDescriptorSetLayout descriptor);

        void SetColorBlendState(VkPipelineColorBlendStateCreateInfo blendState);

    private:
        void _GenerateVertexInputState();
        void _InitializeResterizationState();
        void _InitializeMultiSamplingState();
        void _InitializeDefaultViewPort();

        VkPipelineLayout _pipelineLayout;
        VkPipeline _pipeline;

        SwapChain* _swapchain;
        RenderPass* _renderPass;

        std::vector<Shader*> _shaders;

        std::vector<VkVertexInputBindingDescription>            _vertexBindingDescriptions;
        std::vector<VkVertexInputAttributeDescription>          _vertexAttributeDescriptions;
        std::vector<VkDescriptorSetLayout>                      _descriptorSets;
        std::vector<VkPushConstantRange>                        _pushConstantRanges;
        VkPipelineVertexInputStateCreateInfo                    _vertexInputState;
        VkPipelineInputAssemblyStateCreateInfo                  _inputAssembly;
        boost::optional<VkPipelineTessellationStateCreateInfo>  _tessellationState;
        VkPipelineViewportStateCreateInfo                       _viewportCreateInfo;
        VkPipelineRasterizationStateCreateInfo                  _rasterizationState;
        VkPipelineMultisampleStateCreateInfo                    _multisamplingState; // Multisampling is optional IFF rasterization is off.
        VkPipelineDepthStencilStateCreateInfo                   _depthStencilState;
        VkPipelineColorBlendStateCreateInfo                     _colorBlendState;
        VkPipelineColorBlendAttachmentState                     _colorBlendAttachmentState;

        bool                                                    _useDynamicState;
        VkPipelineDynamicStateCreateInfo                        _dynamicState;
        std::vector<VkDynamicState>                             _dynamicStates;

        VkGraphicsPipelineCreateInfo _graphicsPipelineCreateInfo;

        struct ViewportInfo
        {
            VkViewport Viewport;
            VkRect2D Scissors;
        };

        std::vector<ViewportInfo> _viewports;
    };
}
