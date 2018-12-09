#include "CommandPool.hpp"
#include "Device.hpp"
#include "CommandBuffer.hpp"

#include <boost/iterator/transform_iterator.hpp>

namespace vez
{
    VkResult CommandPool::Create(Device* device, uint32_t queueFamilyIndex, CommandPool** ppCommandPool)
    {
        VkCommandPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.queueFamilyIndex = queueFamilyIndex;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

        VkCommandPool handle = VK_NULL_HANDLE;
        VkResult result = vkCreateCommandPool(device->GetHandle(), &poolInfo, nullptr, &handle);
        if (result != VK_SUCCESS)
            return result;

        CommandPool* commandPool = new CommandPool;
        commandPool->_device = device;
        commandPool->_handle = handle;
        commandPool->_queueFamilyIndex = queueFamilyIndex;
        *ppCommandPool = commandPool;
        return VK_SUCCESS;
    }

    CommandPool::~CommandPool()
    {
        if (_handle != VK_NULL_HANDLE)
            vkDestroyCommandPool(_device->GetHandle(), _handle, nullptr);
    }

    VkResult CommandPool::AllocateCommandBuffers(VkCommandBufferLevel level, uint32_t commandBufferCount, VkCommandBuffer* pCommandBuffers, const void* pNext)
    {
        // Safe guard access to internal resources across threads.
        _spinLock.Lock();

        // Allocate a new command buffer.
        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.pNext = pNext;
        allocInfo.commandPool = _handle;
        allocInfo.level = level;
        allocInfo.commandBufferCount = commandBufferCount;

        std::vector<VkCommandBuffer> commandBuffers(commandBufferCount);

        VkResult result = vkAllocateCommandBuffers(_device->GetHandle(), &allocInfo, commandBuffers.data());

        // Unlock access to internal resources.
        _spinLock.Unlock();

        // Return result.
        return result;
    }

    void CommandPool::FreeCommandBuffers(uint32_t commandBufferCount, const VkCommandBuffer* pCommandBuffers)
    {
        // Safe guard access to internal resources across threads.
        _spinLock.Lock();

        vkFreeCommandBuffers(_device->GetHandle(), _handle, commandBufferCount, pCommandBuffers);

        _spinLock.Unlock();
    }
}
