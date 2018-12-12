#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <unordered_map>

#include "Spinlock.hpp"

namespace vez
{
    class DescriptorSetLayout;

    class DescriptorPool
    {
    public:
        DescriptorPool(DescriptorSetLayout* layout);

        ~DescriptorPool();

        VkDescriptorSet AllocateDescriptorSet();

        VkResult FreeDescriptorSet(VkDescriptorSet descriptorSet);

    private:
        DescriptorSetLayout* _layout = nullptr;
        std::vector<VkDescriptorPoolSize> _poolSizes;
        std::vector<VkDescriptorPool> _pools;
        std::vector<uint32_t> _allocatedSets;
        uint32_t _currentAllocationPoolIndex = 0;
        uint32_t _maxSetsPerPool = 50;

        std::unordered_map<VkDescriptorSet, uint32_t> _allocatedDescriptorSets;
        Spinlock _spinLock;
    };
}