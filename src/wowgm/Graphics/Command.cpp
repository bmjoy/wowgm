#include "Command.hpp"
#include "RenderPass.hpp"
#include "FrameBuffer.hpp"
#include "CommandBuffer.hpp"
#include "Pipeline.hpp"

namespace wowgm::graphics
{
    BeginRenderPass::BeginRenderPass(RenderPass* renderPass, FrameBuffer* frameBuffer, VkExtent2D extent) : Command()
    {
        _command.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        _command.pNext = nullptr;
        _command.renderPass = *renderPass;
        _command.framebuffer = *frameBuffer;
        _command.renderArea.offset = { 0, 0 };
        _command.renderArea.extent = extent;

        VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
        _command.clearValueCount = 1;
        _command.pClearValues = &clearColor;
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
