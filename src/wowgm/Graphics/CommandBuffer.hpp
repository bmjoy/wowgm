#pragma once

#include <vulkan/vulkan.h>
#include <type_traits>

namespace wowgm::graphics
{
    class Command;
    class RenderPass;
    class FrameBuffer;
    class CommandPool;

    class CommandBuffer
    {
        friend class CommandPool;

        CommandBuffer(VkCommandBuffer buffer);

    public:
        ~CommandBuffer();

        operator VkCommandBuffer() const { return _commandBuffer; }

    public: /* Recording */
        void EnqueueCommand(Command* command);
        void BeginRenderPass(RenderPass* renderPass, FrameBuffer* buffer);

        template <typename T, typename... Args, typename std::enable_if<std::is_base_of<Command, T>::value, int>::type = 0>
        void Enqueue(Args&&... args)
        {
            EnqueueCommand(new T(std::forward<Args>(args)...));
        }

    private:
        VkCommandBuffer _commandBuffer;
    };
}