#pragma once

#include <vulkan/vulkan.h>

namespace wowgm::graphics
{
    class CommandBuffer
    {
    public:

        operator VkCommandBuffer() const { return _commandBuffer; }

    private:
        VkCommandBuffer _commandBuffer;
    };
}