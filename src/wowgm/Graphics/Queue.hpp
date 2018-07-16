#pragma once

#include <vulkan/vulkan.h>

#include <cstdint>

namespace wowgm::graphics
{
    class LogicalDevice;

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
    public:
        Queue(LogicalDevice* device, VkQueue queue, std::uint32_t indice);
        ~Queue();

        Queue(Queue const&) = delete;

        operator VkQueue() const { return _queue; }

        std::uint32_t GetFamilyIndice();
        LogicalDevice* GetDevice();

    private:
        LogicalDevice* _device;

        VkQueue _queue;
        std::uint32_t _indice;
    };
}
