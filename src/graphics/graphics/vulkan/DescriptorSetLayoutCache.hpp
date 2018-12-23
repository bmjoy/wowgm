#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <unordered_map>
#include <graphics/vulkan/Spinlock.hpp>

namespace gfx::vk
{
    class Device;
    class DescriptorSetLayout;

    struct PipelineResource;

    class DescriptorSetLayoutCache
    {
    public:
        DescriptorSetLayoutCache(Device* device);

        ~DescriptorSetLayoutCache();

        VkResult CreateLayout(uint32_t setIndex, const std::vector<PipelineResource>& setResources, DescriptorSetLayout** pLayout);

        void DestroyLayout(DescriptorSetLayout* layout);

    private:
        Device* _device = nullptr;
        std::vector<DescriptorSetLayout*> _layouts;
        Spinlock _spinLock;
    };
}