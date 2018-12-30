#include <graphics/vulkan/Device.hpp>
#include <graphics/vulkan/PipelineCache.hpp>
#include <graphics/vulkan/PhysicalDevice.hpp>
#include <graphics/vulkan/Queue.hpp>
#include <graphics/vulkan/Buffer.hpp>
#include <graphics/vulkan/Image.hpp>
#include <graphics/vulkan/CommandPool.hpp>
#include <graphics/vulkan/CommandBuffer.hpp>
#include <graphics/vulkan/Spinlock.hpp>
#include <graphics/vulkan/DescriptorSetLayoutCache.hpp>
#include <graphics/vulkan/Framebuffer.hpp>
#include <graphics/vulkan/Helpers.hpp>

#include <extstd/literals/memory.hpp>

#include <vector>

namespace gfx::vk
{
    VkResult Device::Create(Instance* instance, const DeviceCreateInfo* pCreateInfo, Device** ppDevice)
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

        VkDevice handle = VK_NULL_HANDLE;
        VkResult result = vkCreateDevice(pPhysicalDevice->GetHandle(), &deviceCreateInfo, nullptr, &handle);
        if (result != VK_SUCCESS)
            return result;

        Device* device = new Device();
        device->_physicalDevice = pPhysicalDevice;
        device->_handle = handle;
        device->_instance = instance;
        device->_descriptorSetLayoutCache = new DescriptorSetLayoutCache(device);

        // Create a pipeline cache
        result = PipelineCache::Create(device, &device->_pipelineCache);
        if (result != VK_SUCCESS)
        {
            delete device;
            return result;
        }

        // Acquire all the device's queue families for layer use.
        device->_queues.resize(queueFamilyCount);
        for (uint32_t i = 0; i < queueFamilyCount; ++i)
        {
            QueueFamily qf;
            for (uint32_t j = 0U; j < queueCreateInfos[i].queueCount; ++j)
            {
                VkQueue queue = VK_NULL_HANDLE;
                vkGetDeviceQueue(handle, i, j, &queue);
                if (queue)
                    device->_queues[i].push_back(new Queue(device, queue, i, j, queueFamilyProperties[i]));
            }
        }

        // Create our allocator
        VmaAllocatorCreateInfo allocatorCreateInfo{};
        allocatorCreateInfo.physicalDevice = pPhysicalDevice->GetHandle();
        allocatorCreateInfo.device = handle;
        result = vmaCreateAllocator(&allocatorCreateInfo, &device->_allocator);
        if (result != VK_SUCCESS)
        {
            delete device;
            return result;
        }

        // Create a permanently pinned 8 MB data buffer for buffer copies with the GPU on targets that are not CPU visible
        BufferCreateInfo bufferCreateInfo = {};
        bufferCreateInfo.size = 8_MB;
        bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        bufferCreateInfo.pBufferName = "Staging buffer";
        result = device->CreateBuffer(VMA_MEMORY_USAGE_CPU_ONLY, VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT, &bufferCreateInfo, &device->_stagingBuffer);
        if (result != VK_SUCCESS)
        {
            delete device;
            return result;
        }

        // Pin it.
        result = device->MapBuffer(device->_stagingBuffer, &device->_stagingBufferPinnedDataPtr);
        if (result != VK_SUCCESS)
        {
            delete device;
            return result;
        }

        *ppDevice = device;
        return VK_SUCCESS;
    }

    Device::~Device()
    {
        vkDeviceWaitIdle(_handle);

        DestroyBuffer(_stagingBuffer);

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
    }

    VkResult Device::CreateBuffer(VmaMemoryUsage memoryUsage, VmaAllocationCreateFlagBits allocationFlags, const BufferCreateInfo* pCreateInfo, Buffer** ppBuffer)
    {
        VkBufferCreateInfo bufferCreateInfo{};
        bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferCreateInfo.pNext = pCreateInfo->pNext;
        bufferCreateInfo.usage = pCreateInfo->usage;
        bufferCreateInfo.size = pCreateInfo->size;

        // If no queue family indices are passed in with pCreateInfo, then assume Buffer will be used
        // by all available queues and hence require VK_SHARING_MODE_CONCURRENT.
        std::vector<uint32_t> queueFamilyIndices;
        if (pCreateInfo->queueFamilyIndexCount == 0)
        {
            // Generate a vector of the queue family indices.
            queueFamilyIndices.resize(_queues.size());
            for (auto i = 0U; i < queueFamilyIndices.size(); ++i)
                queueFamilyIndices[i] = i;

            // Set sharing mode to concurrent since Buffer is accessible across all queues.
            bufferCreateInfo.sharingMode = (queueFamilyIndices.size() > 1) ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE;
            bufferCreateInfo.queueFamilyIndexCount = static_cast<uint32_t>(queueFamilyIndices.size());
            bufferCreateInfo.pQueueFamilyIndices = queueFamilyIndices.data();
        }
        else
        {
            bufferCreateInfo.sharingMode = (pCreateInfo->queueFamilyIndexCount == 1) ? VK_SHARING_MODE_EXCLUSIVE : VK_SHARING_MODE_CONCURRENT;
            bufferCreateInfo.queueFamilyIndexCount = pCreateInfo->queueFamilyIndexCount;
            bufferCreateInfo.pQueueFamilyIndices = pCreateInfo->pQueueFamilyIndices;
        }

        // Allocate memory from the Vulkan Memory Allocator.
        VmaAllocation allocation = VK_NULL_HANDLE;
        VkBuffer buffer = VK_NULL_HANDLE;
        VkResult result = VK_SUCCESS;

        if (allocationFlags != 0)
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
            return result;

        *ppBuffer = new Buffer();
        (*ppBuffer)->_handle = buffer;
        (*ppBuffer)->_device = this;
        (*ppBuffer)->_allocation = allocation;
        (*ppBuffer)->_size = pCreateInfo->size;

#if _DEBUG
        if (pCreateInfo->pBufferName != nullptr)
            (*ppBuffer)->SetName(pCreateInfo->pBufferName);
#endif

        return VK_SUCCESS;
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

    VkResult Device::CreateImage(VmaMemoryUsage memoryUsage, VmaAllocationCreateFlagBits allocationFlags, const ImageCreateInfo* pCreateInfo, Image** ppImage)
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
        imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        // Try to guess the initial layout based on image usage
        auto usage = pCreateInfo->usage;
        VkImageLayout initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        if (usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
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
            initialLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

        imageCreateInfo.initialLayout = initialLayout;

        // If no queue family indices are passed in with pCreateInfo, then assume Image will be used
        // by all available queues and hence require VK_SHARING_MODE_CONCURRENT.
        std::vector<uint32_t> queueFamilyIndices;
        if (pCreateInfo->queueFamilyIndexCount == 0)
        {
            // Generate a vector of the queue family indices.
            queueFamilyIndices.resize(_queues.size());
            for (auto i = 0U; i < queueFamilyIndices.size(); ++i)
                queueFamilyIndices[i] = i;

            // Set sharing mode to concurrent since Image is accessible across all queues.
            imageCreateInfo.sharingMode = (queueFamilyIndices.size() == 1) ? VK_SHARING_MODE_EXCLUSIVE : VK_SHARING_MODE_CONCURRENT;
            imageCreateInfo.queueFamilyIndexCount = static_cast<uint32_t>(queueFamilyIndices.size());
            imageCreateInfo.pQueueFamilyIndices = queueFamilyIndices.data();
        }
        else
        {
            // If only a single queue can access the Image then sharing is exclusive.
            // Else if more than one queue will access the Image then sharing is concurrent.
            imageCreateInfo.sharingMode = (pCreateInfo->queueFamilyIndexCount == 1) ? VK_SHARING_MODE_EXCLUSIVE : VK_SHARING_MODE_CONCURRENT;
            imageCreateInfo.queueFamilyIndexCount = pCreateInfo->queueFamilyIndexCount;
            imageCreateInfo.pQueueFamilyIndices = pCreateInfo->pQueueFamilyIndices;
        }

        // Allocate memory using the Vulkan Memory Allocator (unless memFlags has the NO_ALLOCATION bit set).
        VmaAllocation allocation = VK_NULL_HANDLE;
        VkImage handle = VK_NULL_HANDLE;

        VkResult result = VK_SUCCESS;
        if (allocationFlags != 0)
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

        // Create an Image class instance from handle.
        *ppImage = Image::CreateFromHandle(this, pCreateInfo, initialLayout, handle, allocation);

        // Return success.
        return VK_SUCCESS;
    }

    void Device::DestroyImage(Image* pImage)
    {
        if (pImage->GetAllocation() != VK_NULL_HANDLE)
            vmaDestroyImage(_allocator, pImage->GetHandle(), pImage->GetAllocation());
        else
            vkDestroyImage(_handle, pImage->GetHandle(), nullptr);

        delete pImage;
    }


    VkResult Device::ImageSubData(Image* pImage, const ImageSubDataInfo* pSubdataInfo, void* pData)
    {
        if (!IsCompressedImageFormat(pImage->GetCreateInfo().format))
            return UncompressedImageSubData(pImage, pSubdataInfo, pData);
        else
            return CompressedImageSubData(pImage, pSubdataInfo, pData);
    }

    VkResult Device::UncompressedImageSubData(Image* pImage, const ImageSubDataInfo* pSubDataInfo, void* pData)
    {
        CommandBuffer* cmdBuffer = GetCurrentCommandBuffer();
        if (cmdBuffer == nullptr)
            cmdBuffer = GetOneTimeSubmitCommandBuffer();

        Queue* queue = _queues[cmdBuffer->GetPool()->GetQueueFamilyIndex()][0];
        auto alignedFlushSize = _physicalDevice->GetPhysicalDeviceProperties().limits.nonCoherentAtomSize;

        auto pixelSize = GetUncompressedImageFormatSize(pImage->GetCreateInfo().format);

        uint32_t pinnedMemoryBufferSize = 8 * 1024 * 1024u; // _stagingBuffer->GetSize();
        uint32_t numSlices = std::min(pSubDataInfo->imageExtent.depth, pinnedMemoryBufferSize / (pixelSize * pSubDataInfo->imageExtent.width * pSubDataInfo->imageExtent.height));
        uint32_t numRows = std::min(pSubDataInfo->imageExtent.height, pinnedMemoryBufferSize / (pixelSize * pSubDataInfo->imageExtent.width));
        uint32_t numCols = std::min(pSubDataInfo->imageExtent.width, pinnedMemoryBufferSize / pixelSize);
        VkExtent3D maxExtent { numCols, std::max(1U, numRows), std::max(1U, numSlices) };

        const uint8_t* srcPtr = reinterpret_cast<const uint8_t*>(pData);

        VkOffset3D curOffset = pSubDataInfo->imageOffset;
        for (uint32_t arrayLayer = pSubDataInfo->imageSubresource.baseArrayLayer; arrayLayer < pSubDataInfo->imageSubresource.layerCount; ++arrayLayer)
        {
            // Each depth layer in a 3D texture is copied separately.
            for (auto z = 0U; z < pSubDataInfo->imageExtent.depth; z += maxExtent.depth)
            {
                // Set current destination image z offset.
                curOffset.z = z;

                // Handle end of image depth case.
                auto slicesToCopy = std::min(maxExtent.depth, pSubDataInfo->imageExtent.depth - z);

                // Copy pixel data with scrolling extents window moving across 2D image layer.
                for (auto row = 0U; row < pSubDataInfo->imageExtent.height; row += maxExtent.height)
                {
                    // Handle end of image height case.
                    auto rowsToCopy = std::min(maxExtent.height, pSubDataInfo->imageExtent.height - row);

                    for (auto col = 0U; col < pSubDataInfo->imageExtent.width; col += maxExtent.width)
                    {
                        // Handle end of image width case.
                        auto colsToCopy = std::min(maxExtent.width, pSubDataInfo->imageExtent.width - col);
                        auto bytesCopied = 0;

                        // Copy the host memory to the pinned memory buffer.
                        if (pSubDataInfo->dataRowLength == 0)
                        {
                            auto bytesToCopy = colsToCopy * rowsToCopy * slicesToCopy * pixelSize;
                            memcpy(_stagingBufferPinnedDataPtr, srcPtr, bytesToCopy);
                            srcPtr += bytesToCopy;
                            bytesCopied = bytesToCopy;
                        }
                        else
                        {
                            for (auto y = 0U; y < rowsToCopy; ++y)
                            {
                                memcpy(&reinterpret_cast<uint8_t*>(_stagingBufferPinnedDataPtr)[y * maxExtent.width * pixelSize], srcPtr, colsToCopy * pixelSize);
                                srcPtr += pSubDataInfo->dataRowLength * pixelSize;
                                bytesCopied = colsToCopy * pixelSize;
                            }
                        }

                        // Flush must be aligned according to physical device's limits.
                        auto alignedBytesToCopy = static_cast<VkDeviceSize>(std::ceil(bytesCopied / static_cast<float>(alignedFlushSize))) * alignedFlushSize;

                        // Flush the memory write.
                        VmaAllocationInfo allocInfo = {};
                        vmaGetAllocationInfo(_allocator, _stagingBuffer->_allocation, &allocInfo);

                        VkMappedMemoryRange memoryRange = {};
                        memoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
                        memoryRange.memory = allocInfo.deviceMemory;
                        memoryRange.offset = 0;
                        memoryRange.size = alignedBytesToCopy;
                        vkFlushMappedMemoryRanges(_handle, 1, &memoryRange);

                        // Copy the pinned memory buffer to the destination image.
                        BufferImageCopy region {};
                        // region.bufferRowLength = std::max(0U, std::min(pSubDataInfo->dataRowLength, pSubDataInfo->dataRowLength - col * pixelSize));
                        // region.bufferImageHeight = std::max(0U, std::min(pSubDataInfo->dataImageHeight, pSubDataInfo->dataImageHeight - row));
                        region.imageSubresource.baseArrayLayer = arrayLayer;
                        region.imageSubresource.mipLevel = pSubDataInfo->imageSubresource.mipLevel;
                        region.imageSubresource.layerCount = 1;
                        region.imageOffset = curOffset;
                        region.imageExtent = VkExtent3D{ colsToCopy, rowsToCopy, slicesToCopy };
                        cmdBuffer->Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
                        cmdBuffer->CopyBufferToImage(_stagingBuffer, pImage, 1, &region);
                        cmdBuffer->End();

                        // Submit to the queue.
                        {
                            SubmitInfo submitInfo = {};
                            submitInfo.commandBuffers.emplace_back(cmdBuffer);
                            VkResult result = queue->Submit(1, &submitInfo, nullptr);
                            if (result != VK_SUCCESS)
                                return result;
                        }

                        // Wait for all operations to complete.
                        queue->WaitIdle();

                        // Update current destination image x offset.
                        curOffset.x += colsToCopy;
                        if (curOffset.x >= int32_t(pSubDataInfo->imageExtent.width))
                            curOffset.x = int32_t(pSubDataInfo->imageOffset.x);
                    }

                    // Update current destination image y offset.
                    curOffset.y += rowsToCopy;
                    if (curOffset.y >= int32_t(pSubDataInfo->imageExtent.height))
                        curOffset.y = int32_t(pSubDataInfo->imageOffset.y);
                }

                // Update destination image z offset.
                curOffset.z += slicesToCopy;
                if (curOffset.z >= int32_t(pSubDataInfo->imageExtent.depth))
                    curOffset.z = int32_t(pSubDataInfo->imageOffset.z);
            }

            // Increment source pointer address.
            if (pSubDataInfo->dataImageHeight != 0)
            {
                if (pSubDataInfo->dataRowLength != 0)
                    srcPtr += pSubDataInfo->dataRowLength * pixelSize * (pSubDataInfo->dataImageHeight - pSubDataInfo->imageExtent.height);
                else
                    srcPtr += pSubDataInfo->imageExtent.width * pixelSize * (pSubDataInfo->dataImageHeight - pSubDataInfo->imageExtent.height);
            }
        }

        return VK_SUCCESS;
    }

    VkResult Device::CompressedImageSubData(Image* pImage, const ImageSubDataInfo* pSubDataInfo, const void* pData)
    {
        CommandBuffer* cmdBuffer = GetCurrentCommandBuffer();
        if (cmdBuffer == nullptr)
            cmdBuffer = GetOneTimeSubmitCommandBuffer();

        // Get the queue family index the command buffer was created for.
        auto queue = _queues[cmdBuffer->GetPool()->GetQueueFamilyIndex()][0];

        // Get required alignment flush size for selected physical device.
        VkPhysicalDeviceProperties properties = {};
        vkGetPhysicalDeviceProperties(_physicalDevice->GetHandle(), &properties);
        auto alignedFlushSize = properties.limits.nonCoherentAtomSize;

        // Determine the size of each compressed block.
        uint32_t blockSize = 0U, blockWidth = 0U, blockHeight = 0U;
        GetCompressedImageFormatInfo(pImage->GetCreateInfo().format, blockSize, blockWidth, blockHeight);

        // Compute number of compressed blocks making up image data.
        uint32_t numBlocksX = pSubDataInfo->imageExtent.width / blockWidth;
        uint32_t numBlocksY = pSubDataInfo->imageExtent.height / blockHeight;

        // Determine total number of compressed blocks that can be transferred at once.
        uint32_t pinnedMemoryBufferSize = 8 * 1024 * 1024; // _stagingBuffer->GetCreateInfo().size;
        uint32_t maxTransferBlocksY = std::min(numBlocksY, uint32_t(pinnedMemoryBufferSize / (blockSize * numBlocksX)));
        uint32_t maxTransferBlocksX = std::min(numBlocksX, uint32_t(pinnedMemoryBufferSize / blockSize));
        VkExtent2D maxExtent{ maxTransferBlocksX, std::max(1U, maxTransferBlocksX) };

        // Get 8-bit starting address of pixel data.
        const uint8_t* srcPtr = reinterpret_cast<const uint8_t*>(pData);

        // Moving offset and extents during copy.
        auto curOffset = pSubDataInfo->imageOffset;

        // Each layer's data is copied separately.
        for (auto layer = pSubDataInfo->imageSubresource.baseArrayLayer; layer < pSubDataInfo->imageSubresource.layerCount; ++layer)
        {
            // Each depth layer in a 3D texture is copied separately.
            for (auto z = 0U; z < pSubDataInfo->imageExtent.depth; ++z)
            {
                // Set current destination image z offset.
                curOffset.z = z;

                // Copy compressed block data with scrolling extents window moving across 2D image layer.
                for (auto row = 0U; row < numBlocksY; row += maxExtent.height)
                {
                    // Handle end of image height case.
                    auto rowsToCopy = std::min(maxExtent.height, numBlocksY - row);

                    for (auto col = 0U; col < numBlocksX; col += maxExtent.width)
                    {
                        // Handle end of image width case.
                        auto colsToCopy = std::min(maxExtent.width, numBlocksX - col);
                        auto bytesCopied = 0;

                        // Copy the host memory to the pinned memory buffer.
                        if (pSubDataInfo->dataRowLength == 0)
                        {
                            auto bytesToCopy = colsToCopy * rowsToCopy * blockSize;
                            memcpy(_stagingBufferPinnedDataPtr, srcPtr, bytesToCopy);
                            srcPtr += bytesToCopy;
                            bytesCopied = bytesToCopy;
                        }
                        else
                        {
                            for (auto y = 0U; y < rowsToCopy; ++y)
                            {
                                memcpy(&reinterpret_cast<uint8_t*>(_stagingBufferPinnedDataPtr)[y * maxExtent.width * blockSize], srcPtr, colsToCopy * blockSize);
                                srcPtr += pSubDataInfo->dataRowLength * blockSize;
                                bytesCopied = colsToCopy * blockSize;
                            }
                        }

                        // Flush must be aligned according to physical device's limits.
                        auto alignedBytesToCopy = static_cast<VkDeviceSize>(std::ceil(bytesCopied / static_cast<float>(alignedFlushSize))) * alignedFlushSize;

                        // Flush the memory write.
                        VmaAllocationInfo allocInfo = {};
                        vmaGetAllocationInfo(_allocator, _stagingBuffer->_allocation, &allocInfo);

                        VkMappedMemoryRange memoryRange = {};
                        memoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
                        memoryRange.memory = allocInfo.deviceMemory;
                        memoryRange.offset = 0;
                        memoryRange.size = alignedBytesToCopy;
                        vkFlushMappedMemoryRanges(_handle, 1, &memoryRange);

                        // Copy the pinned memory buffer to the destination image.
                        BufferImageCopy region = {};
                        //region.bufferRowLength = std::max(0U, std::min(pSubDataInfo->dataRowLength, pSubDataInfo->dataRowLength - col * pixelSize));
                        //region.bufferImageHeight = std::max(0U, std::min(pSubDataInfo->dataImageHeight, pSubDataInfo->dataImageHeight - row));
                        region.imageSubresource.baseArrayLayer = layer;
                        region.imageSubresource.mipLevel = pSubDataInfo->imageSubresource.mipLevel;
                        region.imageSubresource.layerCount = 1;
                        region.imageOffset = curOffset;
                        region.imageExtent = VkExtent3D{ colsToCopy * blockWidth, rowsToCopy * blockHeight, 1U };
                        cmdBuffer->Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
                        cmdBuffer->CopyBufferToImage(_stagingBuffer, pImage, 1, &region);
                        cmdBuffer->End();

                        // Submit to the queue.

                        {
                            SubmitInfo submitInfo = {};
                            submitInfo.commandBuffers.emplace_back(cmdBuffer);
                            VkResult result = queue->Submit(1, &submitInfo, nullptr);
                            if (result != VK_SUCCESS)
                                return result;
                        }

                        // Wait for all operations to complete.
                        queue->WaitIdle();

                        // Update current destination image x offset.
                        curOffset.x += colsToCopy * blockWidth;
                        if (curOffset.x >= int32_t(pSubDataInfo->imageExtent.width))
                            curOffset.x = int32_t(pSubDataInfo->imageOffset.x);
                    }

                    // Update current destination image y offset.
                    curOffset.y += rowsToCopy * blockHeight;
                    if (curOffset.y >= int32_t(pSubDataInfo->imageExtent.height))
                        curOffset.y = int32_t(pSubDataInfo->imageOffset.y);
                }
            }

            // Increment source pointer address.
            if (pSubDataInfo->dataImageHeight != 0)
            {
                if (pSubDataInfo->dataRowLength != 0)
                    srcPtr += pSubDataInfo->dataRowLength * blockSize * ((pSubDataInfo->dataImageHeight - pSubDataInfo->imageExtent.height) / blockHeight);
                else
                    srcPtr += (pSubDataInfo->imageExtent.width / blockWidth) * blockSize * ((pSubDataInfo->dataImageHeight - pSubDataInfo->imageExtent.height) / blockHeight);
            }
        }

        // Return success.
        return VK_SUCCESS;
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

    VkResult Device::CreateFence(VkFence* pFence, VkFenceCreateFlagBits createFlags)
    {
        VkFenceCreateInfo fenceInfo = {};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = createFlags;

        return vkCreateFence(_handle, &fenceInfo, nullptr, pFence);
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

    VkResult Device::CreateSemaphore(VkSemaphore* semaphore)
    {
        VkSemaphoreCreateInfo semaphoreCreateInfo{};
        semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        return vkCreateSemaphore(_handle, &semaphoreCreateInfo, nullptr, semaphore);
    }

    void Device::DestroySemaphore(VkSemaphore semaphore)
    {
        vkDestroySemaphore(_handle, semaphore, nullptr);
    }

}