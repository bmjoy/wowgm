#include "Command.hpp"
#include "RenderPass.hpp"
#include "FrameBuffer.hpp"
#include "CommandBuffer.hpp"
#include "Pipeline.hpp"

namespace wowgm::graphics
{
    BeginRenderPass::BeginRenderPass(FrameBuffer* frameBuffer, VkExtent2D extent, VkClearValue clearValue) : Command(), _clearValue(clearValue)
    {
        _command = { };
        _command.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        _command.pNext = nullptr;
        _command.renderPass = *frameBuffer->GetRenderPass();
        _command.framebuffer = *frameBuffer;
        _command.renderArea.offset = { 0, 0 };
        _command.renderArea.extent = extent;

        _command.clearValueCount = 1;
        _command.pClearValues = &_clearValue;
    }

    void BeginRenderPass::Enqueue(CommandBuffer* buffer)
    {
        vkCmdBeginRenderPass(*buffer, &_command, VK_SUBPASS_CONTENTS_INLINE);
    }

    BindPipeline::BindPipeline(VkPipelineBindPoint bindPoint, Pipeline* pipeline) : Command(), _bindPoint(bindPoint), _pipeline(pipeline)
    {
    }

    void BindPipeline::Enqueue(CommandBuffer* buffer)
    {
        vkCmdBindPipeline(*buffer, _bindPoint, *_pipeline);
    }

    EndRenderPass::EndRenderPass() : Command()
    {

    }

    void EndRenderPass::Enqueue(CommandBuffer* buffer)
    {
        vkCmdEndRenderPass(*buffer);
    }
}
