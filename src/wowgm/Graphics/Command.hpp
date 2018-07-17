#pragma once

#include <memory>
#include <vulkan/vulkan.h>

namespace wowgm::graphics
{
    class CommandBuffer;
    class RenderPass;
    class FrameBuffer;
    class Pipeline;

    class Command
    {
        Command(Command&&) = delete;
        Command(Command const&) = delete;

    protected:
        Command() { }

    public:
        virtual void Enqueue(CommandBuffer* buffer) = 0;
    };

    class BeginRenderPassCommand : public Command
    {
    public:
        BeginRenderPassCommand(RenderPass* renderPass, FrameBuffer* frameBuffer, VkExtent2D extent);

        virtual void Enqueue(CommandBuffer* buffer) override final;

    private:
        VkRenderPassBeginInfo _command;
    };

    class BindPipelineCommand : public Command
    {
    public:
        BindPipelineCommand(VkPipelineBindPoint bindPoint, Pipeline* pipeline);

        virtual void Enqueue(CommandBuffer* buffer) override final;

    private:
        VkPipelineBindPoint _bindPoint;
        Pipeline* _pipeline;
    };

    class EndRenderPassCommand : public Command
    {
    public:
        EndRenderPassCommand();

        virtual void Enqueue(CommandBuffer* buffer) override final;
    };
}