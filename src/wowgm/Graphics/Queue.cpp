#include "Queue.hpp"
#include "LogicalDevice.hpp"
#include "CommandPool.hpp"

namespace wowgm::graphics
{
    Queue::Queue(LogicalDevice* device, VkQueue queue, std::uint32_t indice) : _device(device), _queue(queue), _indice(indice)
    {

    }

    Queue::~Queue()
    {
        _queue = VK_NULL_HANDLE;
    }

    std::uint32_t Queue::GetFamilyIndice()
    {
        return _indice;
    }

    LogicalDevice* Queue::GetDevice()
    {
        return _device;
    }

    CommandPool* Queue::GetCommandPool(VkCommandPoolCreateFlags createFlags)
    {
        auto itr = _commandPool.at(createFlags);
        if (itr != nullptr)
            return itr;

        auto newPool = new CommandPool(this, createFlags);
        _commandPool[createFlags] = newPool;
        return newPool;
    }
}
