#pragma once

#include <memory>
#include <cstdint>

#include <vulkan/vulkan.h>

namespace vez
{
    class Device;

    class PipelineCache
    {
    public:
        static VkResult Create(Device* pDevice, PipelineCache** ppPipelineCache);
        static VkResult Create(Device* pDevice, std::unique_ptr<uint8_t[]> dataBlob, size_t dataSize, PipelineCache** ppPipelineCache);

        ~PipelineCache();

        std::unique_ptr<uint8_t[]> GetBlob(size_t& size);

        VkPipelineCache GetHandle() const { return _handle; }

    private:
        Device* _device = nullptr;
        VkPipelineCache _handle = VK_NULL_HANDLE;
    };
}
