#include "CommandPool.hpp"
#include "CommandBuffer.hpp"
#include "Queue.hpp"
#include "LogicalDevice.hpp"
#include "Assert.hpp"

#include <stdexcept>

namespace wowgm::graphics
{
    CommandPool::CommandPool(Queue* assignedQueue, VkCommandPoolCreateFlags createFlags) : _assignedQueue(assignedQueue)
    {
        VkCommandPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.queueFamilyIndex = assignedQueue->GetFamilyIndice();
        poolInfo.flags = createFlags; // Optional

        if (vkCreateCommandPool(*assignedQueue->GetDevice(), &poolInfo, nullptr, &_commandPool) != VK_SUCCESS)
            wowgm::exceptions::throw_with_trace(std::runtime_error("Failed to create a command pool!"));
    }

    CommandPool::~CommandPool()
    {
        vkDestroyCommandPool(*_assignedQueue->GetDevice(), _commandPool, nullptr);
        _assignedQueue = nullptr;
    }

    ///! TODO: performance probably scales with the amount of buffers allocated at once
    CommandBuffer* CommandPool::_AllocateBuffer(VkCommandBufferLevel level)
    {
        VkCommandBuffer buffer;

        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = _commandPool;
        allocInfo.level = level;
        allocInfo.commandBufferCount = 1;

        if (vkAllocateCommandBuffers(*_assignedQueue->GetDevice(), &allocInfo, &buffer) != VK_SUCCESS)
            wowgm::exceptions::throw_with_trace(std::runtime_error("Failed to allocate a command buffer!"));

        return new CommandBuffer(buffer);
    }

    CommandBuffer* CommandPool::AllocatePrimaryBuffer()
    {
        return _AllocateBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY);
    }

    CommandBuffer* CommandPool::AllocateSecondaryBuffer()
    {
        return _AllocateBuffer(VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    }
}