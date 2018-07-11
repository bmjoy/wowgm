#pragma once

#include <vulkan/vulkan.h>

namespace wowgm::graphics
{
    class Queue
    {
    public:
        Queue(VkQueue queue);

    private:
        VkQueue _queue;
    };
}
