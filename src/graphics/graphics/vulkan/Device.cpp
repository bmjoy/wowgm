#include <graphics/vulkan/Device.hpp>
#include <graphics/vulkan/PipelineCache.hpp>
#include <graphics/vulkan/PhysicalDevice.hpp>
#include <graphics/vulkan/Queue.hpp>
#include <graphics/vulkan/Buffer.hpp>
#include <graphics/vulkan/Image.hpp>
#include <graphics/vulkan/ImageView.hpp>
#include <graphics/vulkan/CommandPool.hpp>
#include <graphics/vulkan/CommandBuffer.hpp>
#include <graphics/vulkan/Spinlock.hpp>
#include <graphics/vulkan/DescriptorSetLayoutCache.hpp>
#include <graphics/vulkan/Framebuffer.hpp>
#include <graphics/vulkan/ResourceTracker.hpp>
#include <graphics/vulkan/Helpers.hpp>
#include <graphics/vulkan/Sampler.h>

#include <extstd/literals/memory.hpp>

#include <vector>

namespace gfx::vk
{
    Device::Device(Instance* instance, const DeviceCreateInfo* pCreateInfo)
    {
        PhysicalDevice* pPhysicalDevice = pCreateInfo->physicalDevice;

        // Enumerate queue families
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(pPhysicalDevice->GetHandle(), &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(pPhysicalDevice->GetHandle(), &queueFamilyCount, queueFamilyProperties.data());

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos(queueFamilyCount);
        std::vector<std::vector<float>> priorities(queueFamilyCount);
        for (uint32_t i = 0; i < queueFamilyCount; ++i)
        {
            const float defaultPriority = 1.0f;
            priorities[i].resize(queueFamilyProperties[i].queueCount, defaultPriority);
            queueCreateInfos[i].pNext = nullptr;
            queueCreateInfos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfos[i].flags = 0;

            queueCreateInfos[i].queueFamilyIndex = i;
            queueCreateInfos[i].queueCount = queueFamilyProperties[i].queueCount;
            queueCreateInfos[i].pQueuePriorities = priorities[i].data();
        }

        VkPhysicalDeviceFeatures enabledFeatures{};
        vkGetPhysicalDeviceFeatures(pPhysicalDevice->GetHandle(), &enabledFeatures);

        VkDeviceCreateInfo deviceCreateInfo{};
        deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceCreateInfo.pNext = pCreateInfo->pNext;
        deviceCreateInfo.ppEnabledExtensionNames = pCreateInfo->enabledExtensionNames.data();
        deviceCreateInfo.ppEnabledLayerNames = pCreateInfo->enabledLayerNames.data();
        deviceCreateInfo.enabledExtensionCount = pCreateInfo->enabledExtensionNames.size();
        deviceCreateInfo.enabledLayerCount = pCreateInfo->enabledLayerNames.size();

        deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
        deviceCreateInfo.queueCreateInfoCount = uint32_t(queueCreateInfos.size());
        deviceCreateInfo.pEnabledFeatures = &enabledFeatures;

        VkResult result = vkCreateDevice(pPhysicalDevice->GetHandle(), &deviceCreateInfo, nullptr, &_handle);
        BOOST_ASSERT_MSG(result == VK_SUCCESS, "Failed to create a VkDevice");

        _physicalDevice = pPhysicalDevice;
        _instance = instance;
        _descriptorSetLayoutCache = new DescriptorSetLayoutCache(this);

        // Create a pipeline cache
        result = PipelineCache::Create(this, &_pipelineCache);
        BOOST_ASSERT_MSG(result == VK_SUCCESS, "Failed to create a pipeline cache");

        // Acquire all the device's queue families for layer use.
        _queues.resize(queueFamilyCount);
        for (uint32_t i = 0; i < queueFamilyCount; ++i)
        {
            QueueFamily qf;
            for (uint32_t j = 0U; j < queueCreateInfos[i].queueCount; ++j)
            {
                VkQueue queue = VK_NULL_HANDLE;
                vkGetDeviceQueue(_handle, i, j, &queue);
                if (queue)
                    _queues[i].push_back(new Queue(this, queue, i, j, queueFamilyProperties[i]));
            }
        }

        // Create our allocator
        VmaAllocatorCreateInfo allocatorCreateInfo{};
        allocatorCreateInfo.physicalDevice = pPhysicalDevice->GetHandle();
        allocatorCreateInfo.device = _handle;
        result = vmaCreateAllocator(&allocatorCreateInfo, &_allocator);
        BOOST_ASSERT_MSG(result == VK_SUCCESS, "Failed to create a vma allocator for the device");

        _tracker = new ResourceTracker(this);
    }

    Device::~Device()
    {
        vkDeviceWaitIdle(_handle);

        for (auto&& itr : _commandPools)
            for (auto&& kv : itr.second)
                delete kv.second;

        _commandPools.clear();

        delete _descriptorSetLayoutCache;
        delete _pipelineCache;

        vkDestroyDevice(_handle, nullptr);
    }

    VkResult Device::MapBuffer(Buffer* pBuffer, void** ppData)
    {
        VkResult result = vmaMapMemory(_allocator, pBuffer->_allocation, ppData);
        pBuffer->SetMapped(result == VK_SUCCESS);
        return result;
    }

    void Device::UnmapBuffer(Buffer* pBuffer)
    {
        if (!pBuffer->_mapped)
            return;

        vmaUnmapMemory(_allocator, pBuffer->_allocation);
        pBuffer->SetMapped(false);
    }

    Buffer* Device::CreateBuffer(VmaMemoryUsage memoryUsage, VmaAllocationCreateFlags allocationFlags, const BufferCreateInfo* pCreateInfo)
    {
        VkBufferCreateInfo bufferCreateInfo{};
        bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferCreateInfo.pNext = pCreateInfo->pNext;
        bufferCreateInfo.usage = pCreateInfo->usage;
        bufferCreateInfo.size = pCreateInfo->size;

        // If no queue family indices are passed in with pCreateInfo, then assume Buffer will be used
        // by all available queues and hence require VK_SHARING_MODE_CONCURRENT.
        if (pCreateInfo->queueFamilyIndices.size() > 0)
        {
            // Set sharing mode to concurrent since Buffer is accessible across all queues.
            bufferCreateInfo.sharingMode = (pCreateInfo->queueFamilyIndices.size() > 1) ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE;
            bufferCreateInfo.queueFamilyIndexCount = pCreateInfo->queueFamilyIndices.size();
            bufferCreateInfo.pQueueFamilyIndices = pCreateInfo->queueFamilyIndices.data();
        }
        else
        {
            bufferCreateInfo.sharingMode = (pCreateInfo->queueFamilyIndices.size() == 1) ? VK_SHARING_MODE_EXCLUSIVE : VK_SHARING_MODE_CONCURRENT;
            bufferCreateInfo.queueFamilyIndexCount = pCreateInfo->queueFamilyIndices.size();
            bufferCreateInfo.pQueueFamilyIndices = pCreateInfo->queueFamilyIndices.data();
        }

        // Allocate memory from the Vulkan Memory Allocator.
        VmaAllocation allocation = VK_NULL_HANDLE;
        VkBuffer buffer = VK_NULL_HANDLE;
        VkResult result = VK_SUCCESS;

        if (allocationFlags != 0 || memoryUsage != 0)
        {
            // Determine appropriate memory usage flags.
            VmaAllocationCreateInfo allocCreateInfo = {};
            allocCreateInfo.usage = memoryUsage;
            allocCreateInfo.flags = allocationFlags;

            result = vmaCreateBuffer(_allocator, &bufferCreateInfo, &allocCreateInfo, &buffer, &allocation, nullptr);
        }
        else
        {
            result = vkCreateBuffer(_handle, &bufferCreateInfo, nullptr, &buffer);
        }

        if (result != VK_SUCCESS)
            return nullptr;

        Buffer* pBuffer = new Buffer();
        pBuffer->_handle = buffer;
        pBuffer->_device = this;
        pBuffer->_allocation = allocation;
        pBuffer->_size = pCreateInfo->size;

#if _DEBUG
        if (pCreateInfo->pBufferName != nullptr)
            pBuffer->SetName(pCreateInfo->pBufferName);
#endif

        return pBuffer;
    }

    void Device::DestroyBuffer(Buffer* pBuffer)
    {
        UnmapBuffer(pBuffer);

        if (pBuffer->_allocation != VK_NULL_HANDLE)
            vmaDestroyBuffer(_allocator, pBuffer->GetHandle(), pBuffer->_allocation);
        else
            vkDestroyBuffer(_handle, pBuffer->GetHandle(), nullptr);

        delete pBuffer;
    }

    ImageView* Device::CreateImageView(const ImageViewCreateInfo* pCreateInfo)
    {
        VkImageViewCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.pNext = pCreateInfo->pNext;
        createInfo.image = pCreateInfo->image->GetHandle();
        createInfo.viewType = pCreateInfo->viewType;
        createInfo.format = pCreateInfo->image->GetCreateFormat();
        memcpy(&createInfo.components, &pCreateInfo->components, sizeof(VkComponentMapping));
        createInfo.subresourceRange.aspectMask = pCreateInfo->image->GetImageAspectFlags();
        createInfo.subresourceRange.baseMipLevel = pCreateInfo->subresourceRange.baseMipLevel;
        createInfo.subresourceRange.levelCount = pCreateInfo->subresourceRange.levelCount;
        createInfo.subresourceRange.baseArrayLayer = pCreateInfo->subresourceRange.baseArrayLayer;
        createInfo.subresourceRange.layerCount = pCreateInfo->subresourceRange.layerCount;


        VkImageView handle = VK_NULL_HANDLE;
        VkResult result = vkCreateImageView(GetHandle(), &createInfo, nullptr, &handle);
        if (result != VK_SUCCESS)
            return nullptr;

        ImageView* pImageView = new ImageView();
        pImageView->_handle = handle;
        pImageView->_device = this;
        pImageView->_image = pCreateInfo->image;
        pImageView->_viewType = pCreateInfo->viewType;
        pImageView->_format = pCreateInfo->image->GetCreateFormat();

        memcpy(&pImageView->_components, &pCreateInfo->components, sizeof(VkComponentMapping));
        memcpy(&pImageView->_subresourceRange, &pCreateInfo->subresourceRange, sizeof(VkImageSubresourceRange));

        return pImageView;
    }

    Image* Device::CreateImage(VmaMemoryUsage memoryUsage, VmaAllocationCreateFlags allocationFlags, const ImageCreateInfo* pCreateInfo)
    {
        // Create Vulkan image handle.
        VkImageCreateInfo imageCreateInfo = {};
        imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageCreateInfo.pNext = pCreateInfo->pNext;
        imageCreateInfo.imageType = pCreateInfo->imageType;
        imageCreateInfo.format = pCreateInfo->format;
        memcpy(&imageCreateInfo.extent, &pCreateInfo->extent, sizeof(VkExtent3D));
        imageCreateInfo.mipLevels = pCreateInfo->mipLevels;
        imageCreateInfo.arrayLayers = pCreateInfo->arrayLayers;
        imageCreateInfo.samples = pCreateInfo->samples;
        imageCreateInfo.tiling = pCreateInfo->tiling;
        imageCreateInfo.usage = pCreateInfo->usage;

        // Try to guess the initial layout based on image usage
        auto usage = pCreateInfo->usage;
        VkImageLayout initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        /*if (usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
            initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        else if (usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
            initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        else if (usage & VK_IMAGE_USAGE_STORAGE_BIT)
            initialLayout = VK_IMAGE_LAYOUT_GENERAL;
        else if (usage & VK_IMAGE_USAGE_SAMPLED_BIT)
            initialLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        else if (usage & VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT)
            initialLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        else if (usage & VK_IMAGE_USAGE_TRANSFER_DST_BIT)
            initialLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        else if (usage & VK_IMAGE_USAGE_TRANSFER_SRC_BIT)
            initialLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;*/

        imageCreateInfo.initialLayout = initialLayout;


        // If only a single queue can access the Image then sharing is exclusive.
        // Else if more than one queue will access the Image then sharing is concurrent.
        imageCreateInfo.sharingMode = (pCreateInfo->queueFamilyIndices.size() == 1) ? VK_SHARING_MODE_EXCLUSIVE : VK_SHARING_MODE_CONCURRENT;
        imageCreateInfo.queueFamilyIndexCount = pCreateInfo->queueFamilyIndices.size();
        imageCreateInfo.pQueueFamilyIndices = pCreateInfo->queueFamilyIndices.data();

        // Allocate memory using the Vulkan Memory Allocator (unless memFlags has the NO_ALLOCATION bit set).
        VmaAllocation allocation = VK_NULL_HANDLE;
        VkImage handle = VK_NULL_HANDLE;

        VkResult result = VK_SUCCESS;
        if (allocationFlags != 0 || memoryUsage != 0)
        {
            // Determine appropriate memory usage flags.
            VmaAllocationCreateInfo allocCreateInfo = {};
            allocCreateInfo.usage = memoryUsage;
            allocCreateInfo.flags = allocationFlags;

            result = vmaCreateImage(_allocator, &imageCreateInfo, &allocCreateInfo, &handle, &allocation, nullptr);
        }
        else
        {
            result = vkCreateImage(_handle, &imageCreateInfo, nullptr, &handle);
        }

        if (result != VK_SUCCESS)
            return nullptr;

        // Create an Image class instance from handle.
        Image* vkImage = Image::CreateFromHandle(this, pCreateInfo, initialLayout, handle, allocation);

        // Return success.
        return vkImage;
    }

    void Device::DestroyImage(Image* pImage)
    {
        if (pImage->GetAllocation() != VK_NULL_HANDLE)
            vmaDestroyImage(_allocator, pImage->GetHandle(), pImage->GetAllocation());
        else
            vkDestroyImage(_handle, pImage->GetHandle(), nullptr);

        delete pImage;
    }

    void Device::DestroyImageView(ImageView* pImageView)
    {
        delete pImageView;
    }

    void Device::FlushBuffer(Buffer* pBuffer, uint32_t offset, uint32_t size /* = -1 */)
    {
        // NYI (needed? VMA doesnt return a VkDeviceMemory afaik)
    }

    VkResult Device::AllocateCommandBuffers(Queue* pQueue, const void* pNext, VkCommandBufferLevel level, uint32_t commandBufferCount, CommandBuffer** ppCommandBuffers)
    {
        CommandPool* pool = GetCommandPool(pQueue);

        VkResult result = pool->AllocateCommandBuffers(level, commandBufferCount, ppCommandBuffers, pNext);
        if (result != VK_SUCCESS)
            return result;

        return VK_SUCCESS;
    }

    void Device::FreeCommandBuffers(uint32_t commandBufferCount, CommandBuffer** ppCommandBuffers)
    {
        for (uint32_t i = 0; i < commandBufferCount; ++i)
            delete ppCommandBuffers[i];
    }

    CommandPool* Device::GetCommandPool(Queue* queue)
    {
        CommandPool* pool = nullptr;

        // Make thread-safe.
        static Spinlock lock;
        lock.Lock();

        // Find QueueCommandPools for current thread.
        auto itr = _commandPools.find(std::this_thread::get_id());
        if (itr != _commandPools.end())
        {
            // Find CommandPool for specified queue.
            auto& queueCommandPools = itr->second;
            auto itr2 = queueCommandPools.find(queue);
            if (itr2 != queueCommandPools.end())
                pool = itr2->second;
            else if (CommandPool::Create(this, queue->GetFamilyIndex(), &pool) == VK_SUCCESS)
                queueCommandPools.emplace(queue, pool);
        }
        else
        {
            // Create a new QueueCommandPools entry.
            QueueCommandPools queueCommandPools;

            // Create a new CommandPool for the specified queue.
            if (CommandPool::Create(this, queue->GetFamilyIndex(), &pool) == VK_SUCCESS)
            {
                // Add new CommandPool to the new QueueCommandPools instance.
                queueCommandPools.emplace(queue, pool);

                // Add the QueueCommandPools instance to the device level map keyed by the threadID.
                _commandPools.emplace(std::this_thread::get_id(), std::move(queueCommandPools));
            }
        }

        // End thread-safe block.
        lock.Unlock();

        // Return result.
        return pool;
    }

    Queue* Device::GetQueueByFlags(VkQueueFlags queueFlags, uint32_t queueIndex)
    {
        // Enumerate all available queues.
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(_physicalDevice->GetHandle(), &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(_physicalDevice->GetHandle(), &queueFamilyCount, queueFamilyProperties.data());

        // Iterate over queues in order to find one matching requested flags.
        // Favor queue families matching only what's specified in queueFlags over families having other bits set as well.
        VkQueueFlags minFlags = ~0;
        Queue* bestQueue = nullptr;
        for (auto queueFamilyIndex = 0U; queueFamilyIndex < queueFamilyCount; ++queueFamilyIndex)
        {
            if (((queueFamilyProperties[queueFamilyIndex].queueFlags & queueFlags) == queueFlags) && queueIndex < queueFamilyProperties[queueFamilyIndex].queueCount)
            {
                if (queueFamilyProperties[queueFamilyIndex].queueFlags < minFlags)
                {
                    minFlags = queueFamilyProperties[queueFamilyIndex].queueFlags;
                    bestQueue = _queues[queueFamilyIndex][queueIndex];
                }
            }
        }

        // Return the queue for the given flags.
        return bestQueue;
    }

    void Device::WaitIdle()
    {
        vkDeviceWaitIdle(_handle);
    }

    VkFence Device::CreateFence(VkFenceCreateFlags createFlags)
    {
        VkFenceCreateInfo fenceInfo = {};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = createFlags;

        VkFence pFence;
        VkResult result = vkCreateFence(_handle, &fenceInfo, nullptr, &pFence);
        if (result != VK_SUCCESS)
            return VK_NULL_HANDLE;

        return pFence;
    }

    void Device::DestroyFence(VkFence fence)
    {
        vkDestroyFence(_handle, fence, nullptr);
    }

    VkResult Device::GetFenceStatus(VkFence fence)
    {
        return vkGetFenceStatus(_handle, fence);
    }

    VkResult Device::CreateFramebuffer(const FramebufferCreateInfo* pCreateInfo, Framebuffer** ppFramebuffer)
    {
        return Framebuffer::Create(this, pCreateInfo, ppFramebuffer);
    }

    VkResult Device::DestroyFramebuffer(Framebuffer* pFramebuffer)
    {
        delete pFramebuffer;
        return VK_SUCCESS;
    }

    VkSemaphore Device::CreateSemaphore()
    {
        VkSemaphoreCreateInfo semaphoreCreateInfo{};
        semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkSemaphore semaphore;
        VkResult result = vkCreateSemaphore(_handle, &semaphoreCreateInfo, nullptr, &semaphore);
        if (result != VK_SUCCESS)
            return VK_NULL_HANDLE;

        return semaphore;
    }

    void Device::DestroySemaphore(VkSemaphore semaphore)
    {
        vkDestroySemaphore(_handle, semaphore, nullptr);
    }

    Sampler* Device::CreateSampler(const SamplerCreateInfo* pCreateInfo)
    {
        VkSamplerCreateInfo samplerCreateInfo;
        samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerCreateInfo.pNext = nullptr;
        samplerCreateInfo.flags = 0;
        samplerCreateInfo.magFilter = pCreateInfo->magFilter;
        samplerCreateInfo.minFilter = pCreateInfo->minFilter;
        samplerCreateInfo.mipmapMode = pCreateInfo->mipmapMode;
        samplerCreateInfo.addressModeU = pCreateInfo->addressModeU;
        samplerCreateInfo.addressModeV = pCreateInfo->addressModeV;
        samplerCreateInfo.addressModeW = pCreateInfo->addressModeW;
        samplerCreateInfo.mipLodBias = pCreateInfo->mipLodBias;
        samplerCreateInfo.anisotropyEnable = pCreateInfo->anisotropyEnable;
        samplerCreateInfo.maxAnisotropy = pCreateInfo->maxAnisotropy;
        samplerCreateInfo.compareEnable = pCreateInfo->compareEnable;
        samplerCreateInfo.compareOp = pCreateInfo->compareOp;
        samplerCreateInfo.minLod = pCreateInfo->minLod;
        samplerCreateInfo.maxLod = pCreateInfo->maxLod;
        samplerCreateInfo.borderColor = pCreateInfo->borderColor;
        samplerCreateInfo.unnormalizedCoordinates = pCreateInfo->unnormalizedCoordinates;

        VkSampler samplerHandle;
        VkResult result = vkCreateSampler(_handle, &samplerCreateInfo, nullptr, &samplerHandle);
        if (result != VK_SUCCESS)
            return nullptr;

        return new Sampler(this, samplerHandle);
    }
}