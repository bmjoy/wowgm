#include "Queue.hpp"

namespace wowgm::graphics
{
    Queue::Queue(VkQueue queue) : _queue(queue)
    {

    }


    VkQueue Queue::GetVkQueue()
    {
        return _queue;
    }
}