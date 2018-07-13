#pragma once

#include <vulkan/vulkan.h>
#include <boost/optional/optional.hpp>

#include <vector>

namespace wowgm::graphics
{
    class Shader;
    class SwapChain;
    class RenderPass;
    class CommandBuffer;

    class Pipeline
    {
    public:
        Pipeline(SwapChain* swapChain);
        ~Pipeline();

        void Finalize();
        bool IsReady();

        Pipeline(Pipeline&&) = delete;
        Pipeline(Pipeline const&) = delete;

        void EnableDynamicStates() { _useDynamicState = true; }
        void Bind(CommandBuffer* buffer); // vkCmdBindCommandBuffer

    public: /* Rasterization*/
        void SetWireframe(bool wireframe);
        void SetFragmentClampState(bool clampFragments);
        void SetCulling(VkCullModeFlagBits cullMode);
        void SetFrontFaceOrientation(bool clockWise);

    public: /* DepthStencil */
        void SetDepthTest(bool enable);
        void SetDepthTest(VkCompareOp op);
        void SetStencilTest(bool enable);

    public: /* InputAssembly */
        /*
         * In case of drawing commands that use vertex indices, `restartEnable` specifies wether
         * a special index value should restart a primitive. Defaults to false.
         */
        void SetPrimitiveType(VkPrimitiveTopology topology, bool restartEnable = false);

    public: /* Tessellation */
        void SetTessellationControlPoints(std::uint32_t controlPoints);

    public: /* Viewport and scissors */
        void SetViewport(std::uint32_t width, std::uint32_t height);
        void SetViewportDepth(float minDepth, float maxDepth);
        void SetScissors(std::uint32_t width, std::uint32_t height);


    public: /* Attributes and Bindings */

        void AddBinding(VkVertexInputBindingDescription binding);

        template <typename T>
        void AddBinding(std::uint32_t index, VkVertexInputRate inputRate) {
            AddBinding({ index, sizeof(T), inputRate });
        }

        // TODO: This was needed in OpenGL; is it still needed in Vulkan?
        // template <>
        // void AddBinding<glm::mat4>(std::uint32_t index, VkVertexInputRate inputRate)
        // {
        //     AddBinding({ index, sizeof(glm::vec4), inputRate });
        //     AddBinding({ index + 1, sizeof(glm::vec4), inputRate });
        //     AddBinding({ index + 2, sizeof(glm::vec4), inputRate });
        //     AddBinding({ index + 3, sizeof(glm::vec4), inputRate });
        // }

        void AddAttribute(VkVertexInputAttributeDescription attrDescription);

    private:
        SwapChain* _swapchain;
        RenderPass* _renderPass;

        std::vector<VkVertexInputBindingDescription> _vertexBindingDescriptions;
        std::vector<VkVertexInputAttributeDescription> _vertexAttributeDescriptions;

        VkPipelineLayout _pipelineLayout;
        std::vector<VkDescriptorSetLayout> _descriptorSets;
        std::vector<VkPushConstantRange> _pushConstantRanges;

        std::vector<Shader*> _shaders;

        VkPipelineVertexInputStateCreateInfo                    _vertexInputState;
        VkPipelineInputAssemblyStateCreateInfo                  _inputAssembly;
        boost::optional<VkPipelineTessellationStateCreateInfo>  _tessellationState;
        VkPipelineViewportStateCreateInfo                       _viewportCreateInfo;
        // Rasterization is needed when creating the pipeline
        VkPipelineRasterizationStateCreateInfo                  _rasterizationState;
        // Multisampling is optional IFF rasterization is off.
        VkPipelineMultisampleStateCreateInfo                    _multisamplingState;
        VkPipelineDepthStencilStateCreateInfo                   _depthStencilState;
        boost::optional<VkPipelineColorBlendStateCreateInfo>    _colorBlendState;

        bool _useDynamicState;
        VkPipelineDynamicStateCreateInfo                        _dynamicState;
        std::vector<VkDynamicState> _dynamicStates;

        VkGraphicsPipelineCreateInfo _graphicsPipelineCreateInfo;

        VkViewport _viewport;
        VkRect2D   _scissors;
    };
}
