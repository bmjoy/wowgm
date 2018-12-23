#pragma once

#include <graphics/vulkan/VK.hpp>

#include <vector>
#include <unordered_map>
#include <thread>

namespace gfx::vk
{
    typedef std::vector<Queue*> QueueFamily;
    typedef std::unordered_map<Queue*, CommandPool*> QueueCommandPools;

    struct BufferCreateInfo;

    class Device
    {
    public:
        static VkResult Create(Instance* instance, const DeviceCreateInfo* pCreateInfo, Device** ppDevice);

        ~Device();

    public: /* Buffer */

        VkResult CreateBuffer(VmaMemoryUsage memoryUsage, VmaAllocationCreateFlagBits allocationFlags, const BufferCreateInfo* pCreateInfo, Buffer** ppBuffer);

        void DestroyBuffer(Buffer* pBuffer);

        VkResult MapBuffer(Buffer* pBuffer, void** ppData);

        void UnmapBuffer(Buffer* pBuffer);

    public: /* Images */

        VkResult CreateImage(VmaMemoryUsage memoryUsage, VmaAllocationCreateFlagBits allocationFlags, const ImageCreateInfo* pCreateInfo, Image** ppImage);

        void DestroyImage(Image* pImage);

        VkResult ImageSubData(Image* pImage, const ImageSubDataInfo* pSubdataInfo, void* pData);

    public:

    public: /* Sampler */

        VkResult CreateSampler(const SamplerCreateInfo* pCreateInfo, Sampler** pSampler);
        VkResult DestroySampler(Sampler* pSampler);

    public: /* Command Pool */

        CommandPool* GetCommandPool(Queue* queue);

    public: /* Command Buffers */

        VkResult AllocateCommandBuffers(Queue* pQueue, const void* pNext, VkCommandBufferLevel level, uint32_t commandBufferCount, CommandBuffer** ppCommandBuffers);
        void FreeCommandBuffers(uint32_t commandBufferCount, CommandBuffer** ppCommandBuffers);

        CommandBuffer* GetCurrentCommandBuffer() { return nullptr; } //! FIXME

        CommandBuffer* GetOneTimeSubmitCommandBuffer() { return nullptr; } //! FIXME

    public: /* Queues */

        Queue * GetQueueByFlags(VkQueueFlags queueFlags, uint32_t queueIndex);

    public: /* Device */

        void WaitIdle();

        PhysicalDevice* GetPhysicalDevice() const { return _physicalDevice; }
        DescriptorSetLayoutCache* GetDescriptorSetLayoutCache() const { return _descriptorSetLayoutCache; }
        PipelineCache* GetPipelineCache() const { return _pipelineCache; }
        VkDevice GetHandle() const { return _handle; }
        Instance* GetInstance() const { return _instance; }

    public: /* Framebuffer */

        VkResult CreateFramebuffer(const FramebufferCreateInfo* pCreateInfo, Framebuffer** ppFramebuffer);
        VkResult DestroyFramebuffer(Framebuffer* pFramebuffer);

    public: /* Fence */

        void DestroyFence(VkFence fence);

        VkResult GetFenceStatus(VkFence fence);

    private:

        VkResult UncompressedImageSubData(Image* pImage, const ImageSubDataInfo* pSubdataInfo, void* pData);
        VkResult CompressedImageSubData(Image* pImage, const ImageSubDataInfo* pSubDataInfo, const void* pData);

    private:

        PhysicalDevice* _physicalDevice = nullptr;
        VkDevice _handle = VK_NULL_HANDLE;
        VmaAllocator _allocator = VK_NULL_HANDLE;

        std::vector<QueueFamily> _queues;

        std::unordered_map<std::thread::id, QueueCommandPools> _commandPools;

        DescriptorSetLayoutCache* _descriptorSetLayoutCache = nullptr;
        PipelineCache* _pipelineCache = nullptr;

        Instance* _instance = nullptr;

        Buffer* _stagingBuffer = nullptr;
        void* _stagingBufferPinnedDataPtr = nullptr;
    };
}