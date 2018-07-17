#pragma once

#include <vulkan/vulkan.h>

namespace wowgm::graphics
{
    class Queue;
    class CommandBuffer;

    class CommandPool
    {
        CommandPool(CommandPool&&) = delete;
        CommandPool(const CommandPool&) = delete;

    public:
        CommandPool(Queue* assignedQueue, VkCommandPoolCreateFlags createFlags = 0);
        ~CommandPool();

        operator VkCommandPool() const { return _commandPool; }

        CommandBuffer* AllocatePrimaryBuffer();
        CommandBuffer* AllocateSecondaryBuffer();

        VkCommandPoolCreateFlags GetCreateFlags();

    private:
        CommandBuffer* _AllocateBuffer(VkCommandBufferLevel level);

    private:
        VkCommandPool _commandPool;
        VkCommandPoolCreateFlags _createFlags;


        Queue* _assignedQueue;
    };
}