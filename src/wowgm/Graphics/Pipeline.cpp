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

        _InitializeDefaultViewPort();
        _InitializeResterizationState();
        _InitializeMultiSamplingState();

        // This requires a depth/stencil test resource to be dynamically added, so, uh, wait a bit.
        // For now, disable depth testing.
        _depthStencilState = { };
        _depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        _depthStencilState.depthTestEnable = VK_TRUE;
        _depthStencilState.stencilTestEnable = VK_FALSE;

        //! TODO: Blending
        _colorBlendAttachmentState = {};
        _colorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        _colorBlendAttachmentState.blendEnable = VK_FALSE;

        _colorBlendState = {};
        _colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        _colorBlendState.logicOpEnable = VK_FALSE;
        _colorBlendState.logicOp = VK_LOGIC_OP_COPY;
        _colorBlendState.attachmentCount = 1;
        _colorBlendState.pAttachments = &_colorBlendAttachmentState;
        _colorBlendState.blendConstants[0] = 0.0f;
        _colorBlendState.blendConstants[1] = 0.0f;
        _colorBlendState.blendConstants[2] = 0.0f;
        _colorBlendState.blendConstants[3] = 0.0f;

        //! TODO: Dynamic state
    }

    void Pipeline::_InitializeDefaultViewPort()
    {
        ViewportInfo viewport;
        viewport.Viewport.x = 0.0f;
        viewport.Viewport.y = 0.0f;
        viewport.Viewport.maxDepth = 1.0f;
        viewport.Viewport.minDepth = 0.0f;
        viewport.Viewport.width = static_cast<float>(_swapchain->GetExtent().width);
        viewport.Viewport.height = static_cast<float>(_swapchain->GetExtent().height);

        viewport.Scissors.offset = { 0, 0 };
        viewport.Scissors.extent = _swapchain->GetExtent();

        _viewports.push_back(viewport);
    }

    void Pipeline::_InitializeResterizationState()
    {
        _rasterizationState = {};
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
        _rasterizationState.depthBiasEnable = VK_FALSE;
        _rasterizationState.depthBiasConstantFactor = 0.0f; // Optional
        _rasterizationState.depthBiasClamp = 0.0f; // Optional
        _rasterizationState.depthBiasSlopeFactor = 0.0f; // Optional
    }

    void Pipeline::_InitializeMultiSamplingState()
    {
        _multisamplingState = {};
        _multisamplingState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;

        //! TOOD: https://www.khronos.org/registry/vulkan/specs/1.1-extensions/man/html/VkSampleCountFlagBits.html
        // The number of samples generated per pixel.

        _multisamplingState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        _multisamplingState.minSampleShading = 1.0f; // Optional
        _multisamplingState.pSampleMask = nullptr; // Optional
        _multisamplingState.alphaToCoverageEnable = VK_FALSE; // Optional
        _multisamplingState.alphaToOneEnable = VK_FALSE; // Optional
    }

    Pipeline::~Pipeline()
    {
        vkDestroyPipeline(*_swapchain->GetLogicalDevice(), _pipeline, nullptr);
        vkDestroyPipelineLayout(*_swapchain->GetLogicalDevice(), _pipelineLayout, nullptr);

        delete _renderPass;

        _swapchain = nullptr;
        _renderPass = nullptr;

        for (auto&& itr : _shaders)
            delete itr;
    }


    RenderPass* Pipeline::GetRenderPass()
    {
        return _renderPass;
    }

    void Pipeline::AddShader(Shader* shader)
    {
        _shaders.push_back(shader);
    }

    void Pipeline::_GenerateVertexInputState()
    {
        _vertexInputState = {};
        _vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        _vertexInputState.vertexBindingDescriptionCount = _vertexBindingDescriptions.size();
        _vertexInputState.pVertexBindingDescriptions = _vertexBindingDescriptions.data();
        _vertexInputState.vertexAttributeDescriptionCount = _vertexAttributeDescriptions.size();
        _vertexInputState.pVertexAttributeDescriptions = _vertexAttributeDescriptions.data();
    }

    void Pipeline::Finalize()
    {
        _viewportCreateInfo = {};
        _viewportCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        _viewportCreateInfo.viewportCount = _viewports.size();
        _viewportCreateInfo.scissorCount = _viewports.size();

        auto viewportMutator = [](ViewportInfo info) { return info.Viewport; };
        auto scissorsMutator = [](ViewportInfo info) { return info.Scissors; };

        auto viewportItr = boost::make_transform_iterator(_viewports.begin(), viewportMutator);
        auto viewportEnd = boost::make_transform_iterator(_viewports.end(), viewportMutator);
        std::vector<VkViewport> vkViewports(viewportItr, viewportEnd);

        auto scissorsItr = boost::make_transform_iterator(_viewports.begin(), scissorsMutator);
        auto scissorsEnd = boost::make_transform_iterator(_viewports.end(), scissorsMutator);
        std::vector<VkRect2D> vkScissors(scissorsItr, scissorsEnd);

        _viewportCreateInfo.pViewports = vkViewports.data();
        _viewportCreateInfo.pScissors = vkScissors.data();

        _GenerateVertexInputState();

        if (_useDynamicState)
        {
            // Cookbook, page 791/1166
            //! COLOR BLENDING IS NEEDED IF THERE IS NO RASTERIZER DISCARD
            // TryAddDynamicStateBit(_dynamicStates, _colorBlendState);
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
            wowgm::exceptions::throw_with_trace<std::runtime_error>("Unable to create a pipeline layout!");

        _graphicsPipelineCreateInfo = { };
        _graphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

        auto shaderMutator = [](Shader* s)-> VkPipelineShaderStageCreateInfo { return *s; };
        auto shaderItr = boost::iterators::make_transform_iterator(_shaders.begin(), shaderMutator);
        auto shaderEnd = boost::iterators::make_transform_iterator(_shaders.end(), shaderMutator);
        std::vector<VkPipelineShaderStageCreateInfo> shaderStages(shaderItr, shaderEnd);

        _graphicsPipelineCreateInfo.stageCount = shaderStages.size();
        _graphicsPipelineCreateInfo.pStages = shaderStages.data();

        _graphicsPipelineCreateInfo.pVertexInputState = &_vertexInputState;
        _graphicsPipelineCreateInfo.pInputAssemblyState = &_inputAssembly;
        _graphicsPipelineCreateInfo.pTessellationState = _tessellationState.get_ptr();
        _graphicsPipelineCreateInfo.pViewportState = &_viewportCreateInfo;
        _graphicsPipelineCreateInfo.pRasterizationState = &_rasterizationState;
        _graphicsPipelineCreateInfo.pMultisampleState = &_multisamplingState;
        _graphicsPipelineCreateInfo.pDepthStencilState = &_depthStencilState;
        _graphicsPipelineCreateInfo.pColorBlendState = &_colorBlendState;

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
            wowgm::exceptions::throw_with_trace<std::runtime_error>("Unable to create pipeline");
    }


    VkPipelineRasterizationStateCreateInfo& Pipeline::GetRasterizationStateInfo()
    {
        return _rasterizationState;
    }

    VkPipelineDepthStencilStateCreateInfo& Pipeline::GetDepthStencilStateInfo()
    {
        return _depthStencilState;
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

    void Pipeline::CreateViewport(std::uint32_t x, std::uint32_t y, std::uint32_t w, std::uint32_t h, float minDepth, float maxDepth)
    {
        ViewportInfo viewport = { };
        viewport.Viewport.x = x;
        viewport.Viewport.y = y;
        viewport.Viewport.width = w;
        viewport.Viewport.height = h;
        viewport.Viewport.minDepth = minDepth;
        viewport.Viewport.maxDepth = maxDepth;

        viewport.Scissors.extent.width = w - x;
        viewport.Scissors.extent.height = h - y;
        viewport.Scissors.offset.x = 0;
        viewport.Scissors.offset.y = 0;
    }

    void Pipeline::CreateViewport(std::uint32_t w, std::uint32_t h, float minDepth, float maxDepth)
    {
        CreateViewport(0u, 0u, w, h, minDepth, maxDepth);
    }

    bool Pipeline::IsReady()
    {
        return _pipelineLayout != VK_NULL_HANDLE;
    }

    void Pipeline::AddVertexBinding(VkVertexInputBindingDescription binding)
    {
        _vertexBindingDescriptions.push_back(binding);
    }

    void Pipeline::AddVertexAttribute(VkVertexInputAttributeDescription attrDescription)
    {
        _vertexAttributeDescriptions.push_back(attrDescription);
    }

    void Pipeline::AddPushConstant(VkPushConstantRange constantRange)
    {
        _pushConstantRanges.push_back(constantRange);
    }

    void Pipeline::AddDescriptorSetLayout(VkDescriptorSetLayout descriptor)
    {
        _descriptorSets.push_back(descriptor);
    }

    void Pipeline::SetColorBlendState(VkPipelineColorBlendStateCreateInfo blendState)
    {
        memcpy(&_colorBlendState, &blendState, sizeof(VkPipelineColorBlendStateCreateInfo));
    }
}
