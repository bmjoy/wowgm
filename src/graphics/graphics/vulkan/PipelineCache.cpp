#include "PipelineCache.hpp"
#include "Device.hpp"

#include <vector>

namespace gfx::vk
{
    VkResult PipelineCache::Create(Device* pDevice, PipelineCache** ppPipelineCache)
    {
        PipelineCache* pipelineCache = new PipelineCache;

        VkPipelineCacheCreateInfo pipelineCacheCreateInfo{};
        pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
        VkResult result = vkCreatePipelineCache(pDevice->GetHandle(), &pipelineCacheCreateInfo, nullptr, &pipelineCache->_handle);
        if (result != VK_SUCCESS)
        {
            delete pipelineCache;

            *ppPipelineCache = nullptr;
            return result;
        }

        *ppPipelineCache = pipelineCache;
        pipelineCache->_device = pDevice;
        return result;
    }

    VkResult PipelineCache::Create(Device* pDevice, std::unique_ptr<uint8_t[]> dataBlob, size_t dataSize, PipelineCache** ppPipelineCache)
    {
        PipelineCache* pipelineCache = new PipelineCache;

        VkPipelineCacheCreateInfo pipelineCacheCreateInfo{};
        pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
        pipelineCacheCreateInfo.pInitialData = dataBlob.get();
        pipelineCacheCreateInfo.initialDataSize = dataSize;

        VkResult result = vkCreatePipelineCache(pDevice->GetHandle(), &pipelineCacheCreateInfo, nullptr, &pipelineCache->_handle);
        if (result != VK_SUCCESS)
        {
            delete pipelineCache;

            *ppPipelineCache = nullptr;
            return result;
        }

        *ppPipelineCache = pipelineCache;
        pipelineCache->_device = pDevice;
        return result;
    }

    PipelineCache::~PipelineCache()
    {
        vkDestroyPipelineCache(_device->GetHandle(), _handle, nullptr);
    }

    std::unique_ptr<uint8_t[]> PipelineCache::GetBlob(size_t& size)
    {
        VkResult result = vkGetPipelineCacheData(_device->GetHandle(), _handle, &size, nullptr);
        if (result != VK_SUCCESS)
            return std::unique_ptr<uint8_t[]> {};

        uint8_t* data = new uint8_t[size];
        result = vkGetPipelineCacheData(_device->GetHandle(), _handle, &size, data);
        if (result != VK_SUCCESS)
            return std::unique_ptr<uint8_t[]> {};

        return std::unique_ptr<uint8_t[]>(data);
    }
}