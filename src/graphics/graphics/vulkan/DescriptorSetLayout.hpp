#pragma once

#include <vulkan/vulkan.h>
#include <unordered_map>

namespace gfx::vk
{
    class Device;
    class DescriptorPool;
    class Sampler;
    class ImageView;

    struct PipelineResource;

    class DescriptorSetLayout
    {
    public:
        static VkResult Create(Device* device, const std::vector<PipelineResource>& setResources, DescriptorSetLayout** pLayout);

        ~DescriptorSetLayout();

        Device* GetDevice() { return _device; }
        VkDescriptorSetLayout GetHandle() const { return _handle; }
        std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> const& GetBindings() const { return _bindings; }

        VkDescriptorSetLayoutBinding const& GetBinding(uint32_t bindingIndex);

        VkDescriptorSet AllocateDescriptorSet();

        VkResult FreeDescriptorSet(VkDescriptorSet descriptorSet);

        VkDescriptorSet const& GetDescriptorSet(uint32_t bindingIndex);

        void UpdateBinding(uint32_t bindingIndex, Sampler* sampler, ImageView* imageView, VkImageLayout layout);

    private:
        Device * _device = nullptr;
        DescriptorPool* _descriptorPool = nullptr;

        VkDescriptorSetLayout _handle = VK_NULL_HANDLE;
        std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> _bindings;
        std::unordered_map<uint32_t, VkDescriptorSet> _descriptorSets;
    };
}
