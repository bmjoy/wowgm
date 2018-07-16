#pragma once

#include <memory>

namespace wowgm::graphics
{
    class CommandBuffer;
    class RenderPass;
    class FrameBuffer;

    class Command
    {
    public:
        virtual void Enqueue(CommandBuffer* buffer) = 0;
    };

    class BeginRenderPassCommand : public Command
    {
    public:
        BeginRenderPassCommand(RenderPass* renderPass, FrameBuffer* frameBuffer);

        virtual void Enqueue(CommandBuffer* buffer) override final;
    };
}