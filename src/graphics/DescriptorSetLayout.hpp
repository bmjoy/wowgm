#pragma once

#include <vulkan/vulkan.h>
#include <unordered_map>

namespace vez
{
    class Device;
    class DescriptorPool;

    struct PipelineResource;

    class DescriptorSetLayout
    {
    public:
        static VkResult Create(Device* device, const std::vector<PipelineResource>& setResources, DescriptorSetLayout** pLayout);

        ~DescriptorSetLayout();

        Device* GetDevice() { return _device; }
        VkDescriptorSetLayout GetHandle() const { return _handle; }
        std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> const& GetBindings() const { return _bindings; }

        bool GetLayoutBinding(uint32_t bindingIndex, VkDescriptorSetLayoutBinding** pBinding);

        VkDescriptorSet AllocateDescriptorSet();

        VkResult FreeDescriptorSet(VkDescriptorSet descriptorSet);

    private:
        Device * _device = nullptr;
        DescriptorPool* _descriptorPool = nullptr;

        VkDescriptorSetLayout _handle = VK_NULL_HANDLE;
        std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> _bindings;
    };
}
