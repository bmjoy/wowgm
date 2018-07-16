#include "Queue.hpp"

namespace wowgm::graphics
{
    Queue::Queue(VkQueue queue) : _queue(queue)
    {

    }

    Queue::~Queue()
    {
        _queue = VK_NULL_HANDLE;
    }
}