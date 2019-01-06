#include <graphics/vulkan/Pipeline.hpp>
#include <graphics/vulkan/PipelineCache.hpp>
#include <graphics/vulkan/Shader.hpp>
#include <graphics/vulkan/Device.hpp>
#include <graphics/vulkan/DescriptorSetLayoutCache.hpp>
#include <graphics/vulkan/DescriptorSetLayout.hpp>
#include <graphics/vulkan/RenderPass.hpp>

#include <graphics/vulkan/VK.hpp>

#include <extstd/containers/extract.hpp>

#include <shared/assert/assert.hpp>

#include <boost/iterator/transform_iterator.hpp>

namespace gfx::vk
{
    Pipeline::Pipeline()
    {

    }

    Pipeline::~Pipeline()
    {
        vkDestroyPipeline(_device->GetHandle(), GetHandle(), nullptr);
        vkDestroyPipelineLayout(_device->GetHandle(), GetLayout(), nullptr);
    }

    void MergeShaderResources(std::unordered_map<std::string, PipelineResource>& pipelineResources, const Shader* const* pShaders, uint32_t stageCount)
    {
        for (uint32_t i = 0; i < stageCount; ++i)
        {
            for (auto&& resource : pShaders[i]->GetResources())
            {
                // The key used for each resource is its name, except in the case of outputs, since its legal to
                // have separate outputs with the same name across shader stages.
                auto key = std::string(resource.name);
                if (resource.resourceType == PIPELINE_RESOURCE_TYPE_OUTPUT || resource.resourceType == PIPELINE_RESOURCE_TYPE_INPUT)
                    key = std::to_string(resource.stages) + ":" + key;

                // If resource already exists in pipeline resource map, add current stage's bit.
                // Else create a new entry in the pipeline resource map.
                auto it = pipelineResources.find(key);
                if (it != pipelineResources.end())
                    it->second.stages |= resource.stages;
                else
                    pipelineResources.emplace(key, resource);
            }
        }
    }

    VkFormat getResourceFormat(PipelineResource const& resource)
    {
        VkFormat resourceFormat;

        if (resource.baseType == PIPELINE_RESOURCE_BASE_TYPE_FLOAT)
        {
            switch (resource.vecSize)
            {
            case 1:
                resourceFormat = VK_FORMAT_R32_SFLOAT;
                break;
            case 2:
                resourceFormat = VK_FORMAT_R32G32_SFLOAT;
                break;
            case 3:
                resourceFormat = VK_FORMAT_R32G32B32_SFLOAT;
                break;
            case 4:
                resourceFormat = VK_FORMAT_R32G32B32A32_SFLOAT;
                break;
            }
        }
        else if (resource.baseType == PIPELINE_RESOURCE_BASE_TYPE_DOUBLE)
        {
            switch (resource.vecSize)
            {
            case 1:
                resourceFormat = VK_FORMAT_R64_SFLOAT;
                break;
            case 2:
                resourceFormat = VK_FORMAT_R64G64_SFLOAT;
                break;
            case 3:
                resourceFormat = VK_FORMAT_R64G64B64_SFLOAT;
                break;
            case 4:
                resourceFormat = VK_FORMAT_R64G64B64A64_SFLOAT;
                break;
            }
        }
        else if (resource.baseType == PIPELINE_RESOURCE_BASE_TYPE_INT)
        {
            switch (resource.vecSize)
            {
            case 1:
                resourceFormat = VK_FORMAT_R32_SINT;
                break;
            case 2:
                resourceFormat = VK_FORMAT_R32G32_SINT;
                break;
            case 3:
                resourceFormat = VK_FORMAT_R32G32B32_SINT;
                break;
            case 4:
                resourceFormat = VK_FORMAT_R32G32B32A32_SINT;
                break;
            }
        }
        else if (resource.baseType == PIPELINE_RESOURCE_BASE_TYPE_UINT)
        {
            switch (resource.vecSize)
            {
            case 1:
                resourceFormat = VK_FORMAT_R32_UINT;
                break;
            case 2:
                resourceFormat = VK_FORMAT_R32G32_UINT;
                break;
            case 3:
                resourceFormat = VK_FORMAT_R32G32B32_UINT;
                break;
            case 4:
                resourceFormat = VK_FORMAT_R32G32B32A32_UINT;
                break;
            }
        }

        return resourceFormat;
    }

    VkResult Pipeline::Create(Device* pDevice, const GraphicsPipelineCreateInfo* pCreateInfo, Pipeline** ppPipeline)
    {
        std::vector<VkPushConstantRange> pushConstants;

        Pipeline* pPipeline = new Pipeline;
        pPipeline->_device = pDevice;
        pPipeline->_bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

        VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
        pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineCreateInfo.flags = pCreateInfo->flags;
        pipelineCreateInfo.stageCount = pCreateInfo->shaders.size();

        if (pipelineCreateInfo.stageCount == 0)
            shared::assert::throw_with_trace("No shader stages provided. Did you forget to add shaders, you spoon?");

        Shader* vertexShader = nullptr;
        std::vector<VkPipelineShaderStageCreateInfo> shaderStages(pCreateInfo->shaders.size());
        for (uint32_t i = 0; i < pCreateInfo->shaders.size(); ++i)
        {
            VkPipelineShaderStageCreateInfo& stageInfo = shaderStages[i];
            stageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            stageInfo.pNext = nullptr;
            stageInfo.flags = 0; // TODO: Flags
            stageInfo.stage = pCreateInfo->shaders[i]->GetStage();
            stageInfo.module = pCreateInfo->shaders[i]->GetHandle();
            stageInfo.pName = pCreateInfo->shaders[i]->GetEntryPoint().data();
            stageInfo.pSpecializationInfo = nullptr; // TODO: Specialization constants

            if (stageInfo.stage == VK_SHADER_STAGE_VERTEX_BIT)
                vertexShader = pCreateInfo->shaders[i];
        }

        pipelineCreateInfo.pStages = shaderStages.data();

        VkPipelineVertexInputStateCreateInfo vertexInputState {};
        pipelineCreateInfo.pVertexInputState = &vertexInputState;

        vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputState.pNext = pCreateInfo->vertexInputState.pNext;
        vertexInputState.flags = pCreateInfo->vertexInputState.flags;

        // Create attribute descriptions from reflection if there was none provided
        if (pCreateInfo->vertexInputState.vertexAttributeDescriptions.size() == 0 && vertexShader != nullptr)
        {
            GraphicsPipelineCreateInfo* mutableCreateInfo = const_cast<GraphicsPipelineCreateInfo*>(pCreateInfo);

            auto resources = vertexShader->GetResources();

            for (auto&& resource : resources)
            {
                if (resource.resourceType != PIPELINE_RESOURCE_TYPE_INPUT)
                    continue;

                VkVertexInputAttributeDescription attrDescription;
                attrDescription.binding = resource.binding;
                attrDescription.location = resource.location;
                attrDescription.offset = resource.offset;
                attrDescription.format = getResourceFormat(resource);

                mutableCreateInfo->vertexInputState.vertexAttributeDescriptions.push_back(attrDescription);
            }
        }

        vertexInputState.vertexAttributeDescriptionCount = pCreateInfo->vertexInputState.vertexAttributeDescriptions.size();
        vertexInputState.pVertexAttributeDescriptions = pCreateInfo->vertexInputState.vertexAttributeDescriptions.data();

        if (pCreateInfo->vertexInputState.vertexBindingDescriptions.size() == 0 && vertexShader != nullptr)
        {
            BOOST_ASSERT_MSG(false, "Reflection on vertex bindings is not implemented. Please fill vertexBindingDescriptions in your pipeline.");
        }

        vertexInputState.vertexBindingDescriptionCount = pCreateInfo->vertexInputState.vertexBindingDescriptions.size();
        vertexInputState.pVertexBindingDescriptions = pCreateInfo->vertexInputState.vertexBindingDescriptions.data();

        VkPipelineInputAssemblyStateCreateInfo inputAssemblyState{};
        pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
        inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssemblyState.pNext = pCreateInfo->inputAssemblyState.pNext;
        inputAssemblyState.flags = pCreateInfo->inputAssemblyState.flags;
        inputAssemblyState.topology = pCreateInfo->inputAssemblyState.topology;
        inputAssemblyState.primitiveRestartEnable = pCreateInfo->inputAssemblyState.primitiveRestartEnable;

        VkPipelineTessellationStateCreateInfo tessellationState{};
        pipelineCreateInfo.pTessellationState = &tessellationState;
        tessellationState.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
        tessellationState.pNext = pCreateInfo->tessellationState.pNext;
        tessellationState.flags = pCreateInfo->tessellationState.flags;
        tessellationState.patchControlPoints = pCreateInfo->tessellationState.patchControlPoints;

        VkPipelineViewportStateCreateInfo viewportState{};
        pipelineCreateInfo.pViewportState = &viewportState;
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.pNext = pCreateInfo->viewportState.pNext;
        viewportState.flags = pCreateInfo->viewportState.flags;
        viewportState.pScissors = pCreateInfo->viewportState.scissors.data();
        viewportState.pViewports = pCreateInfo->viewportState.viewports.data();
        viewportState.scissorCount = pCreateInfo->viewportState.scissors.size();
        viewportState.viewportCount = pCreateInfo->viewportState.viewports.size();

        VkPipelineRasterizationStateCreateInfo rasterizationState{};
        pipelineCreateInfo.pRasterizationState = &rasterizationState;
        rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizationState.pNext = pCreateInfo->rasterizationState.pNext;
        rasterizationState.flags = pCreateInfo->rasterizationState.flags;
        rasterizationState.cullMode = pCreateInfo->rasterizationState.cullMode;
        rasterizationState.depthBiasClamp = pCreateInfo->rasterizationState.depthBiasClamp;
        rasterizationState.depthBiasConstantFactor = pCreateInfo->rasterizationState.depthBiasConstantFactor;
        rasterizationState.depthBiasEnable = pCreateInfo->rasterizationState.depthBiasEnable;
        rasterizationState.depthBiasSlopeFactor = pCreateInfo->rasterizationState.depthBiasSlopeFactor;
        rasterizationState.depthClampEnable = pCreateInfo->rasterizationState.depthClampEnable;
        rasterizationState.frontFace = pCreateInfo->rasterizationState.frontFace;
        rasterizationState.lineWidth = pCreateInfo->rasterizationState.lineWidth;
        rasterizationState.polygonMode = pCreateInfo->rasterizationState.polygonMode;
        rasterizationState.rasterizerDiscardEnable = pCreateInfo->rasterizationState.rasterizerDiscardEnable;

        VkPipelineMultisampleStateCreateInfo multisampleState{};
        pipelineCreateInfo.pMultisampleState = &multisampleState;
        multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampleState.flags = pCreateInfo->multisampleState.flags;
        multisampleState.pNext = pCreateInfo->multisampleState.pNext;
        multisampleState.alphaToCoverageEnable = pCreateInfo->multisampleState.alphaToCoverageEnable;
        multisampleState.alphaToOneEnable = pCreateInfo->multisampleState.alphaToOneEnable;
        multisampleState.minSampleShading = pCreateInfo->multisampleState.minSampleShading;
        multisampleState.pSampleMask = pCreateInfo->multisampleState.pSampleMask;
        multisampleState.rasterizationSamples = pCreateInfo->multisampleState.rasterizationSamples;
        multisampleState.sampleShadingEnable = pCreateInfo->multisampleState.sampleShadingEnable;

        VkPipelineDepthStencilStateCreateInfo depthStencilState{};
        pipelineCreateInfo.pDepthStencilState = &depthStencilState;
        depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencilState.flags = pCreateInfo->depthStencilState.flags;
        depthStencilState.pNext = pCreateInfo->depthStencilState.pNext;
        depthStencilState.back = pCreateInfo->depthStencilState.back;
        depthStencilState.depthBoundsTestEnable = pCreateInfo->depthStencilState.depthBoundsTestEnable;
        depthStencilState.depthCompareOp = pCreateInfo->depthStencilState.depthCompareOp;
        depthStencilState.depthTestEnable = pCreateInfo->depthStencilState.depthTestEnable;
        depthStencilState.depthWriteEnable = pCreateInfo->depthStencilState.depthWriteEnable;
        depthStencilState.front = pCreateInfo->depthStencilState.front;
        depthStencilState.maxDepthBounds = pCreateInfo->depthStencilState.maxDepthBounds;
        depthStencilState.minDepthBounds = pCreateInfo->depthStencilState.minDepthBounds;
        depthStencilState.stencilTestEnable = pCreateInfo->depthStencilState.stencilTestEnable;

        VkPipelineColorBlendStateCreateInfo colorBlendState{};
        pipelineCreateInfo.pColorBlendState = &colorBlendState;
        colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlendState.flags = pCreateInfo->colorBlendState.flags;
        colorBlendState.pNext = pCreateInfo->colorBlendState.pNext;
        colorBlendState.attachmentCount = pCreateInfo->colorBlendState.attachments.size();
        memcpy(colorBlendState.blendConstants, pCreateInfo->colorBlendState.blendConstants, sizeof(float) * 4);
        colorBlendState.logicOp = pCreateInfo->colorBlendState.logicOp;
        colorBlendState.logicOpEnable = pCreateInfo->colorBlendState.logicOpEnable;
        colorBlendState.pAttachments = pCreateInfo->colorBlendState.attachments.data();

        // Scoped here so that it lives longer than the if branch
        std::vector<VkDynamicState> dynamicStates;
        if (pCreateInfo->dynamicState)
        {
            VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo{};
            pipelineCreateInfo.pDynamicState = &dynamicStateCreateInfo;
            dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;

            if (pCreateInfo->dynamicState.viewport)
                dynamicStates.push_back(VK_DYNAMIC_STATE_VIEWPORT);
            if (pCreateInfo->dynamicState.scissors)
                dynamicStates.push_back(VK_DYNAMIC_STATE_SCISSOR);
            if (pCreateInfo->dynamicState.lineWidth)
                dynamicStates.push_back(VK_DYNAMIC_STATE_LINE_WIDTH);
            if (pCreateInfo->dynamicState.depthBias)
                dynamicStates.push_back(VK_DYNAMIC_STATE_DEPTH_BIAS);
            if (pCreateInfo->dynamicState.blendConstants)
                dynamicStates.push_back(VK_DYNAMIC_STATE_BLEND_CONSTANTS);
            if (pCreateInfo->dynamicState.depthBounds)
                dynamicStates.push_back(VK_DYNAMIC_STATE_DEPTH_BOUNDS);
            if (pCreateInfo->dynamicState.stencilCompareMask)
                dynamicStates.push_back(VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK);
            if (pCreateInfo->dynamicState.stencilWriteMask)
                dynamicStates.push_back(VK_DYNAMIC_STATE_STENCIL_WRITE_MASK);
            if (pCreateInfo->dynamicState.stencilReference)
                dynamicStates.push_back(VK_DYNAMIC_STATE_STENCIL_REFERENCE);

            dynamicStateCreateInfo.pDynamicStates = dynamicStates.data();
            dynamicStateCreateInfo.dynamicStateCount = dynamicStates.size();
        }
        else
            pipelineCreateInfo.pDynamicState = nullptr;

        pipelineCreateInfo.renderPass = pCreateInfo->renderPass->GetHandle();
        pipelineCreateInfo.subpass = pCreateInfo->subpass;
        pipelineCreateInfo.basePipelineHandle = pCreateInfo->basePipelineHandle;
        pipelineCreateInfo.basePipelineIndex = pCreateInfo->basePipelineIndex;

        if (pCreateInfo->pLayout != nullptr)
        {
            pPipeline->_pipelineLayout = *pCreateInfo->pLayout;
            pipelineCreateInfo.layout = *pCreateInfo->pLayout;
        }
        else
        {
            std::unordered_map<std::string, PipelineResource> pipelineResources;
            MergeShaderResources(pipelineResources, pCreateInfo->shaders.data(), pCreateInfo->shaders.size());

            pPipeline->CreateSetBindings(pipelineResources);
            VkResult result = pPipeline->CreateDescriptorSetLayouts();
            if (result != VK_SUCCESS)
            {
                delete pPipeline;
                return result;
            }

            //! FIXME TODO: Create the VkPipelineLayout handle
            VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
            pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

            for (auto&& kv : pipelineResources)
            {
                auto& resource = kv.second;
                switch (resource.resourceType)
                {
                    case PIPELINE_RESOURCE_TYPE_PUSH_CONSTANT_BUFFER:
                    {
                        VkPushConstantRange pushConstant{};
                        pushConstant.offset = resource.offset;
                        pushConstant.size = resource.size;
                        pushConstant.stageFlags = resource.stages;
                        pushConstants.push_back(pushConstant);
                        break;
                    }
                }
            }

            pipelineLayoutCreateInfo.pPushConstantRanges = pushConstants.data();
            pipelineLayoutCreateInfo.pushConstantRangeCount = pushConstants.size();

            auto descriptorSetLayouts = extstd::values(pPipeline->_descriptorSetLayouts);
            std::vector<VkDescriptorSetLayout> descriptors(descriptorSetLayouts.size());
            for (uint32_t i = 0; i < descriptorSetLayouts.size(); ++i)
                descriptors[i] = descriptorSetLayouts[i]->GetHandle();

            pipelineLayoutCreateInfo.pSetLayouts = descriptors.data();
            pipelineLayoutCreateInfo.setLayoutCount = descriptors.size();

            result = vkCreatePipelineLayout(pDevice->GetHandle(), &pipelineLayoutCreateInfo, nullptr, &pPipeline->_pipelineLayout);
            if (result != VK_SUCCESS)
                shared::assert::throw_with_trace("Unable to create a pipeline layout from reflection!");

            pipelineCreateInfo.layout = pPipeline->_pipelineLayout;
        }

        VkPipelineCache pipelineCache = VK_NULL_HANDLE;
        if (pDevice->GetPipelineCache() != nullptr)
            pipelineCache = pDevice->GetPipelineCache()->GetHandle();

        VkResult result = vkCreateGraphicsPipelines(pDevice->GetHandle(), pipelineCache, 1, &pipelineCreateInfo, nullptr, &pPipeline->_handle);
        if (result != VK_SUCCESS)
        {
            delete pPipeline;
            return result;
        }

#if _DEBUG
        memcpy(&pPipeline->_dynamicState, &pCreateInfo->dynamicState, sizeof(PipelineDynamicState));
#endif

        *ppPipeline = pPipeline;
        pushConstants.swap((*ppPipeline)->_pushConstants);

        return VK_SUCCESS;
    }

    VkShaderStageFlags Pipeline::GetPushConstantsRangeStages(uint32_t offset, uint32_t size)
    {
        VkShaderStageFlags flags = VK_SHADER_STAGE_ALL;

        for (auto&& itr : _pushConstants)
        {
            if (itr.offset <= offset && offset + size <= itr.offset + itr.size)
                return itr.stageFlags;
        }

        return flags;
    }

    void Pipeline::CreateSetBindings(std::unordered_map<std::string, PipelineResource> const& resources)
    {
        for (auto&& itr : resources)
        {
            auto resource = itr.second;
            auto bindingItr = _bindings.find(resource.set);
            if (bindingItr != _bindings.end())
                bindingItr->second.push_back(resource);
            else
                _bindings.emplace(resource.set, std::vector<PipelineResource> { resource });

            switch (resource.resourceType)
            {
                case PIPELINE_RESOURCE_TYPE_INPUT:
                case PIPELINE_RESOURCE_TYPE_OUTPUT:
                case PIPELINE_RESOURCE_TYPE_PUSH_CONSTANT_BUFFER:
                    break;
                default:
                {
                    uint64_t resourceKey = static_cast<uint64_t>(resource.set) << 32uLL | resource.binding;
                    _bindingAccessFlags[resourceKey] = resource.access;
                    break;
                }
            }
        }
    }

    DescriptorSetLayout* Pipeline::GetDescriptorSetLayout(uint32_t setIndex)
    {
        return _descriptorSetLayouts[setIndex];
    }

    VkResult Pipeline::CreateDescriptorSetLayouts()
    {
        for (auto&& itr : _bindings)
        {
            DescriptorSetLayout* descriptorSetLayout = nullptr;
            auto result = _device->GetDescriptorSetLayoutCache()->CreateLayout(itr.first, itr.second, &descriptorSetLayout);
            if (result != VK_SUCCESS)
                return result;

            _descriptorSetLayouts.emplace(itr.first, descriptorSetLayout);
        }

        return VK_SUCCESS;
    }
}