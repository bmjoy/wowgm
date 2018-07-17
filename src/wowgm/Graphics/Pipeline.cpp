#include "Pipeline.hpp"
#include "Shader.hpp"
#include "SwapChain.hpp"
#include "LogicalDevice.hpp"
#include "SharedGraphicsDefines.hpp"
#include "RenderPass.hpp"
#include "Assert.hpp"

#include <boost/iterator/transform_iterator.hpp>

namespace wowgm::graphics
{
    template <typename T, typename std::enable_if<details::optional_bit<T>::value, int>::type = 0>
    void TryAddDynamicStateBit(std::vector<VkDynamicState>& states, const boost::optional<T>& memberPtr)
    {
        if (memberPtr)
            return;

        states.push_back(details::optional_bit<T>::bit_value);
        return;
    }

    Pipeline::Pipeline(SwapChain* swapChain, RenderPass* renderPass) : _swapchain(swapChain), _renderPass(renderPass)
    {
        _useDynamicState = false;

        _inputAssembly = { };
        _inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;

        _viewportCreateInfo = { };
        _viewportCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        _viewportCreateInfo.viewportCount = 1;
        _viewportCreateInfo.scissorCount = 1;
        _viewportCreateInfo.pViewports = &_viewport;
        _viewportCreateInfo.pScissors = &_scissors;

        _viewport.x = 0.0f;
        _viewport.y = 0.0f;
        _viewport.maxDepth = 1.0f;
        _viewport.minDepth = 0.0f;
        _viewport.width = static_cast<float>(swapChain->GetExtent().width);
        _viewport.height = static_cast<float>(swapChain->GetExtent().height);

        _scissors = { };
        _scissors.offset = { 0, 0 };
        _scissors.extent = swapChain->GetExtent();

        _rasterizationState = { };
        _rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        _rasterizationState.depthClampEnable = VK_FALSE;
        // If rasterizerDiscardEnable is set to VK_TRUE, then geometry never passes through
        // the rasterizer stage. This basically disables any output to the framebuffer.
        _rasterizationState.rasterizerDiscardEnable = VK_FALSE;
        _rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
        // Any value larger than 1 requires `wideLine` feature on the logical device.
        _rasterizationState.lineWidth = 1.0f;
        _rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;
        _rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        // This is usually used for shadow mapping. We don't use that by default.
        _rasterizationState.depthBiasEnable = VK_FALSE;
        _rasterizationState.depthBiasConstantFactor = 0.0f; // Optional
        _rasterizationState.depthBiasClamp = 0.0f; // Optional
        _rasterizationState.depthBiasSlopeFactor = 0.0f; // Optional

        _multisamplingState = { };
        _multisamplingState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        //! TOOD: https://www.khronos.org/registry/vulkan/specs/1.1-extensions/man/html/VkSampleCountFlagBits.html
        // The number of samples generated per pixel.
        _multisamplingState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        _multisamplingState.minSampleShading = 1.0f; // Optional
        _multisamplingState.pSampleMask = nullptr; // Optional
        _multisamplingState.alphaToCoverageEnable = VK_FALSE; // Optional
        _multisamplingState.alphaToOneEnable = VK_FALSE; // Optional

        // This requires a depth/stencil test resource to be dynamically added, so, uh, wait a bit.
        // For now, disable depth testing.
        _depthStencilState = { };
        _depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        _depthStencilState.depthTestEnable = VK_TRUE;
        _depthStencilState.stencilTestEnable = VK_FALSE;

        //! TODO: Blending
        //! TODO: Dynamic state
    }

    Pipeline::~Pipeline()
    {

    }


    RenderPass* Pipeline::GetRenderPass()
    {
        return _renderPass;
    }

    void Pipeline::AddShader(Shader* shader)
    {
        _shaders.push_back(shader);
    }

    void Pipeline::Finalize()
    {
        _vertexInputState = {};
        _vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        _vertexInputState.vertexBindingDescriptionCount = _vertexBindingDescriptions.size();
        _vertexInputState.pVertexBindingDescriptions = _vertexBindingDescriptions.data();
        _vertexInputState.vertexAttributeDescriptionCount = _vertexAttributeDescriptions.size();
        _vertexInputState.pVertexAttributeDescriptions = _vertexAttributeDescriptions.data();

        // Preparation done, create the pipeline

        if (_useDynamicState)
        {
            // Cookbook, page 791/1166
            TryAddDynamicStateBit(_dynamicStates, _colorBlendState);
            // TryAddDynamicStateBit(_dynamicStates, _viewportCreateInfo); // For now, force a viewport
        }

        VkPipelineLayoutCreateInfo pipelineLayoutInfo = { };
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.pPushConstantRanges = _pushConstantRanges.data();
        pipelineLayoutInfo.pushConstantRangeCount = _pushConstantRanges.size();
        pipelineLayoutInfo.pSetLayouts = _descriptorSets.data();
        pipelineLayoutInfo.setLayoutCount = _descriptorSets.size();

        VkResult result = vkCreatePipelineLayout(*_swapchain->GetLogicalDevice(), &pipelineLayoutInfo, nullptr, &_pipelineLayout);
        if (result != VK_SUCCESS)
            wowgm::exceptions::throw_with_trace(std::runtime_error("Unable to create a pipeline layout!"));

        // Cookbook, page 801/1166
        _graphicsPipelineCreateInfo = { };
        _graphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

        {
            auto shaderMutator = [](Shader* s) {
                return s->GetVkShaderStageInfo();
            };
            auto itr = boost::iterators::make_transform_iterator(_shaders.begin(), shaderMutator);
            auto end = boost::iterators::make_transform_iterator(_shaders.end(), shaderMutator);

            std::vector<VkPipelineShaderStageCreateInfo> shaderStages(itr, end);

            _graphicsPipelineCreateInfo.stageCount = shaderStages.size();
            _graphicsPipelineCreateInfo.pStages = shaderStages.data();
        }

        _graphicsPipelineCreateInfo.pVertexInputState = &_vertexInputState;
        _graphicsPipelineCreateInfo.pInputAssemblyState = &_inputAssembly;
        _graphicsPipelineCreateInfo.pTessellationState = _tessellationState.get_ptr();
        _graphicsPipelineCreateInfo.pViewportState = &_viewportCreateInfo;
        _graphicsPipelineCreateInfo.pRasterizationState = &_rasterizationState;
        _graphicsPipelineCreateInfo.pMultisampleState = &_multisamplingState;
        _graphicsPipelineCreateInfo.pDepthStencilState = &_depthStencilState;
        _graphicsPipelineCreateInfo.pColorBlendState = _colorBlendState.get_ptr();

        if (_useDynamicState)
        {
            _graphicsPipelineCreateInfo.pDynamicState = &_dynamicState;

            _dynamicState.dynamicStateCount = _dynamicStates.size();
            _dynamicState.pDynamicStates = _dynamicStates.data();
        }

        _graphicsPipelineCreateInfo.layout = _pipelineLayout;
        _graphicsPipelineCreateInfo.renderPass = *_renderPass;
        _graphicsPipelineCreateInfo.subpass = 0; // Index of the subpass where this pipeline will be used
        _graphicsPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;

        result = vkCreateGraphicsPipelines(*_swapchain->GetLogicalDevice(), VK_NULL_HANDLE, 1, &_graphicsPipelineCreateInfo, nullptr, &_pipeline);
        if (result != VK_SUCCESS)
            wowgm::exceptions::throw_with_trace(std::runtime_error("Unable to create pipeline"));
    }

    void Pipeline::SetWireframe(bool wireframe)
    {
        _rasterizationState.polygonMode = wireframe ? VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL;
    }

    void Pipeline::SetFragmentClampState(bool clampFragments)
    {
        _rasterizationState.depthClampEnable = clampFragments ? VK_TRUE : VK_FALSE;
    }

    void Pipeline::SetCulling(VkCullModeFlagBits cullMode)
    {
        _rasterizationState.cullMode = cullMode;
    }

    void Pipeline::SetFrontFaceOrientation(bool clockwise)
    {
        _rasterizationState.frontFace = clockwise ? VK_FRONT_FACE_CLOCKWISE : VK_FRONT_FACE_COUNTER_CLOCKWISE;
    }

    void Pipeline::SetDepthTest(bool enable)
    {
        _depthStencilState.depthTestEnable = enable ? VK_TRUE : VK_FALSE;
    }

    void Pipeline::SetDepthTest(VkCompareOp op)
    {
        SetDepthTest(true);
        _depthStencilState.depthCompareOp = op;
    }

    void Pipeline::SetStencilTest(bool enable)
    {
        _depthStencilState.stencilTestEnable = enable ? VK_TRUE : VK_FALSE;
    }

    void Pipeline::SetPrimitiveType(VkPrimitiveTopology topology, bool restartEnable)
    {
        _inputAssembly.topology = topology;
        _inputAssembly.primitiveRestartEnable = restartEnable ? VK_TRUE : VK_FALSE;
    }

    void Pipeline::SetTessellationControlPoints(std::uint32_t controlPoints)
    {
        if (!_tessellationState.is_initialized())
        {
            _tessellationState = VkPipelineTessellationStateCreateInfo { };
            _tessellationState->sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
        }

        _tessellationState.get_ptr()->patchControlPoints = controlPoints;
    }

    void Pipeline::SetViewport(std::uint32_t width, std::uint32_t height)
    {
        _viewport.width = static_cast<float>(width);
        _viewport.height = static_cast<float>(height);
    }

    void Pipeline::SetViewportDepth(float minDepth, float maxDepth)
    {
        _viewport.maxDepth = maxDepth;
        _viewport.minDepth = minDepth;
    }

    void Pipeline::SetScissors(std::uint32_t width, std::uint32_t height)
    {
        _scissors.extent.width = width;
        _scissors.extent.height = height;
    }

    bool Pipeline::IsReady()
    {
        return _pipelineLayout != VK_NULL_HANDLE;
    }

    void Pipeline::AddBinding(VkVertexInputBindingDescription binding)
    {
        _vertexBindingDescriptions.push_back(binding);
    }

    void Pipeline::AddAttribute(VkVertexInputAttributeDescription attrDescription)
    {
        _vertexAttributeDescriptions.push_back(attrDescription);
    }
}
