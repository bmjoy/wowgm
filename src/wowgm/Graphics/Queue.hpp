#pragma once

#include <cstdint>
#include <unordered_map>
#include <vulkan/vulkan.h>

namespace wowgm::graphics
{
    class LogicalDevice;
    class CommandPool;

    /*
     * In Vulkan, when we want to perform operations on hardware, we submit them to queues. The operations within a
     * single queue are processed one after another, in the same order they were submitted--that's why it's called
     * a queue. However, operations submitted to different queues are processed independently (if we need, we can
     * synchronize them).
     *
     * Different queues may represent different parts of the hardware, and thus may support different kinds of operations.
     * Not all operations may be performed on all queues.
     *
     * Queues with the same capabilities are grouped into families. A device may expose any number of queue families,
     * and each family may contain one or more queues. To check what operations can be performed on the given hardware,
     * we need to query the properties of all queue families.
     */
    class Queue
    {
        Queue(Queue&&) = delete;
        Queue(const Queue&) = delete;

    public:

        Queue(LogicalDevice* device, VkQueue queue, std::uint32_t indice);

        ~Queue();

        std::uint32_t GetFamilyIndice();

        LogicalDevice* GetDevice();

        CommandPool* GetCommandPool(VkCommandPoolCreateFlags createFlags = 0);

        operator VkQueue() const { return _queue; }

    private:
        LogicalDevice* _device;
        std::unordered_map<VkCommandPoolCreateFlags, CommandPool*> _commandPool;

        VkQueue _queue;
        std::uint32_t _indice;
    };
}
