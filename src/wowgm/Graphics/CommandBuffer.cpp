#include "CommandBuffer.hpp"
#include "Command.hpp"
#include "RenderPass.hpp"
#include "FrameBuffer.hpp"
#include "Assert.hpp"

#include <stdexcept>

namespace wowgm::graphics
{
    CommandBuffer::CommandBuffer(VkCommandBuffer buffer) : _commandBuffer(buffer)
    {

    }

    CommandBuffer::~CommandBuffer()
    {

    }

    void CommandBuffer::BeginRecording(VkCommandBufferUsageFlagBits usageFlags)
    {
        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = usageFlags;

        VkResult result = vkBeginCommandBuffer(_commandBuffer, &beginInfo);
        if (result != VK_SUCCESS)
            wowgm::exceptions::throw_with_trace(std::runtime_error("Unable to begin recording!"));
    }

    void CommandBuffer::FinishRecording()
    {
        VkResult result = vkEndCommandBuffer(_commandBuffer);
        if (result != VK_SUCCESS)
            wowgm::exceptions::throw_with_trace(std::runtime_error("Unable to end recording!"));
    }
}