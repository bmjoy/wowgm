#include <graphics/vulkan/DescriptorSetLayoutCache.hpp>
#include <graphics/vulkan/DescriptorSetLayout.hpp>
#include <graphics/vulkan/VK.hpp>


namespace gfx::vk
{
    //! TODO: Proper caching, hashing, and external synchronization

    DescriptorSetLayoutCache::DescriptorSetLayoutCache(Device* device) : _device(device)
    {

    }

    DescriptorSetLayoutCache::~DescriptorSetLayoutCache()
    {
        for (auto it : _layouts)
            delete it;
    }

    VkResult DescriptorSetLayoutCache::CreateLayout(uint32_t setIndex, const std::vector<PipelineResource>& setResources, DescriptorSetLayout** pLayout)
    {
        // Find or create a DescriptorSetLayout instance for the given descriptor set resouces.  Make thread-safe.
        DescriptorSetLayout* descriptorSetLayout = nullptr;
        VkResult result = VK_SUCCESS;

        // Acquire access to the cache.
        // _spinLock.Lock();

        result = DescriptorSetLayout::Create(_device, setResources, &descriptorSetLayout);
        // if (result == VK_SUCCESS)
        //     _layouts.push_back(descriptorSetLayout);

        // Release access to the cache.
        // _spinLock.Unlock();

        // Return result.
        if (result == VK_SUCCESS)
            *pLayout = descriptorSetLayout;

        return result;
    }

    void DescriptorSetLayoutCache::DestroyLayout(DescriptorSetLayout* layout)
    {
    }
}