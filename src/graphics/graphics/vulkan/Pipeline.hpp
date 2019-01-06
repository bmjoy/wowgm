#pragma once

#include <vulkan/vulkan.hpp>
#include <vector>
#include <unordered_map>

#include <graphics/vulkan/VK.hpp>

namespace gfx::vk
{
    class Device;
    class DescriptorSetLayout;
    class RenderPass;
    class PipelineCache;

    struct GraphicsPipelineCreateInfo;
    struct ComputePipelineCreateInfo;
    struct PipelineResource;

    class Pipeline
    {
        friend class PipelineCache;

        Pipeline();

    public:

        ~Pipeline();

    public:
        static VkResult Create(Device* pDevice, const GraphicsPipelineCreateInfo* pCreateInfo, Pipeline** ppPipeline);

        // static VkResult Create(Device* pDevice, const ComputePipelineCreateInfo* pCreateInfo, Pipeline** ppPipeline);

        VkPipelineBindPoint GetBindPoint() const { return _bindPoint; }

        VkPipeline GetHandle() const { return _handle; }

        Device* GetDevice() const { return _device; }

        VkPipelineLayout GetLayout() const { return _pipelineLayout; }

        PipelineDynamicState const& GetDynamicStates() const { return _dynamicState; }

        VkShaderStageFlags GetPushConstantsRangeStages(uint32_t offset, uint32_t size);

        DescriptorSetLayout* GetDescriptorSetLayout(uint32_t setIndex);

    private:
        void CreateSetBindings(std::unordered_map<std::string, PipelineResource> const& resources);
        VkResult CreateDescriptorSetLayouts();

    private:
        VkPipeline _handle = VK_NULL_HANDLE;
        VkPipelineLayout _pipelineLayout = VK_NULL_HANDLE;

        Device* _device = nullptr;

        VkPipelineBindPoint _bindPoint;
        PipelineDynamicState _dynamicState;

        std::vector<VkPushConstantRange> _pushConstants;

        std::unordered_map<uint32_t, std::vector<PipelineResource>> _bindings;
        std::unordered_map<uint32_t, DescriptorSetLayout*> _descriptorSetLayouts;
        std::unordered_map<uint64_t, VkAccessFlags> _bindingAccessFlags;
    };
}
