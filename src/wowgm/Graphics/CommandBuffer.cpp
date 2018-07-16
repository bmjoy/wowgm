#include "CommandBuffer.hpp"
#include "Command.hpp"
#include "RenderPass.hpp"
#include "FrameBuffer.hpp"

namespace wowgm::graphics
{
    CommandBuffer::CommandBuffer(VkCommandBuffer buffer) : _commandBuffer(buffer)
    {

    }

    CommandBuffer::~CommandBuffer()
    {

    }

    void CommandBuffer::EnqueueCommand(Command* command)
    {
        command->Enqueue(this);
    }

    void CommandBuffer::BeginRenderPass(RenderPass* renderPass, FrameBuffer* buffer)
    {
        Enqueue<BeginRenderPassCommand>(renderPass, buffer);
    }
}