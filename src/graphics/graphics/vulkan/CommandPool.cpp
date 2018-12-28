#include <graphics/vulkan/CommandPool.hpp>
#include <graphics/vulkan/Device.hpp>
#include <graphics/vulkan/CommandBuffer.hpp>

#include <boost/iterator/transform_iterator.hpp>

namespace gfx::vk
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

    VkResult CommandPool::AllocateCommandBuffers(VkCommandBufferLevel level, uint32_t commandBufferCount, CommandBuffer** pCommandBuffers, const void* pNext)
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

        if (result == VK_SUCCESS)
        {
            for (uint32_t i = 0; i < commandBufferCount; ++i)
                pCommandBuffers[i] = new CommandBuffer(this, commandBuffers[i], level);
        }

        // Return result.
        return result;
    }

    void CommandPool::FreeCommandBuffer(CommandBuffer* commandBuffer)
    {
        _spinLock.Lock();

        auto handle = commandBuffer->GetHandle();

        vkFreeCommandBuffers(_device->GetHandle(), _handle, 1, &handle);

        _spinLock.Unlock();
    }

    void CommandPool::FreeCommandBuffers(uint32_t commandBufferCount, CommandBuffer** pCommandBuffers)
    {
        // Safe guard access to internal resources across threads.
        _spinLock.Lock();

        std::vector<VkCommandBuffer> cmdBuffers(commandBufferCount);
        for (uint32_t i = 0; i < commandBufferCount; ++i)
        {
            cmdBuffers[i] = pCommandBuffers[i]->GetHandle();

            pCommandBuffers[i]->MarkAsDeleted();
        }

        vkFreeCommandBuffers(_device->GetHandle(), _handle, commandBufferCount, cmdBuffers.data());

        _spinLock.Unlock();
    }
}
