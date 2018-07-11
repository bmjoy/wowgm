#pragma once

#include <vulkan/vulkan.h>

namespace wowgm::graphics
{
    class Queue
    {
    public:
        Queue(VkQueue queue);

        VkQueue GetVkQueue();

    private:
        VkQueue _queue;
    };
}
