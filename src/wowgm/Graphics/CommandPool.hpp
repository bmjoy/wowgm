#pragma once

#include <vulkan/vulkan.h>

namespace wowgm::graphics
{
    class Queue;
    class CommandBuffer;

    class CommandPool
    {
    public:
        CommandPool(Queue* assignedQueue, VkCommandPoolCreateFlags createFlags = 0);
        ~CommandPool();

        CommandPool(CommandPool&&) = delete;
        CommandPool(const CommandPool&) = delete;

        operator VkCommandPool() const { return _commandPool; }

        CommandBuffer* AllocatePrimaryBuffer();
        CommandBuffer* AllocateSecondaryBuffer();

    private:
        CommandBuffer* _AllocateBuffer(VkCommandBufferLevel level);

    private:
        VkCommandPool _commandPool;

        Queue* _assignedQueue;
    };
}