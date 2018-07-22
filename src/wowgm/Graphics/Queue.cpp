#include "Queue.hpp"
#include "LogicalDevice.hpp"
#include "CommandPool.hpp"

namespace wowgm::graphics
{
    Queue::Queue(LogicalDevice* device, VkQueueFlagBits type, VkQueue queue, std::uint32_t indice) : _device(device), _queue(queue), _indice(indice), _queueType(type)
    {

    }

    Queue::~Queue()
    {
        _queue = VK_NULL_HANDLE;

        _commandPool.clear();
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
        auto itr = _commandPool.find(createFlags);
        if (itr != _commandPool.end())
            return itr->second.get();

        auto newPool = std::make_unique<CommandPool>(this, createFlags);
        _commandPool[createFlags] = std::move(newPool);
        return _commandPool[createFlags].get();
    }

    bool Queue::IsQueueType(VkQueueFlagBits type)
    {
        return (_queueType & type) != 0;
    }
}
