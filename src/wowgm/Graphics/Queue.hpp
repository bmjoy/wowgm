#pragma once

#include <vulkan/vulkan.h>

namespace wowgm::graphics
{
    class Queue
    {
    public:
        Queue(VkQueue queue);

        VkQueue GetVkQueue();

        operator VkQueue() const { return _queue; }

    private:
        VkQueue _queue;
    };
}
