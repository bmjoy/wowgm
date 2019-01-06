#pragma once

#include <graphics/vulkan/VK.hpp>

#include <vector>
#include <unordered_map>
#include <thread>

#undef CreateSemaphore // FUCKING HELL

namespace gfx::vk
{
    class ResourceTracker;

    typedef std::vector<Queue*> QueueFamily;
    typedef std::unordered_map<Queue*, CommandPool*> QueueCommandPools;

    struct BufferCreateInfo;

    class Device
    {
    public:
        Device(Instance* instance, const DeviceCreateInfo* pCreateInfo);

        ~Device();

        VmaAllocator const& GetAllocator() const { return _allocator; }

        ResourceTracker* GetResourceManager() const { return _tracker; }

    public: /* Buffer */

        Buffer* CreateBuffer(VmaMemoryUsage memoryUsage, VmaAllocationCreateFlags allocationFlags, const BufferCreateInfo* pCreateInfo);

        void DestroyBuffer(Buffer* pBuffer);

        VkResult MapBuffer(Buffer* pBuffer, void** ppData);

        void UnmapBuffer(Buffer* pBuffer);

        void FlushBuffer(Buffer* pBuffer, uint32_t offset = 0, uint32_t size = -1);

    public: /* Images */

        Image* CreateImage(VmaMemoryUsage memoryUsage, VmaAllocationCreateFlags allocationFlags, const ImageCreateInfo* pCreateInfo);

        void DestroyImage(Image* pImage);

    public: /* Image Views */

        ImageView* CreateImageView(const ImageViewCreateInfo* pCreateInfo);

        void DestroyImageView(ImageView* pImageView);

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
        VkFence CreateFence(VkFenceCreateFlags createFlags);
        VkResult GetFenceStatus(VkFence fence);
        void DestroyFence(VkFence fence);

    public: /* Semaphore */
        VkSemaphore CreateSemaphore();
        void DestroySemaphore(VkSemaphore semaphore);

    private:

        PhysicalDevice* _physicalDevice = nullptr;

        Instance* _instance = nullptr;

        DescriptorSetLayoutCache* _descriptorSetLayoutCache = nullptr;
        PipelineCache* _pipelineCache = nullptr;

        ResourceTracker* _tracker;

        VkDevice _handle = VK_NULL_HANDLE;
        VmaAllocator _allocator = VK_NULL_HANDLE;

        std::vector<QueueFamily> _queues;
        std::unordered_map<std::thread::id, QueueCommandPools> _commandPools;

    };
}