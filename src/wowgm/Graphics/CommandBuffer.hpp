#pragma once

#include <type_traits>
#include <vulkan/vulkan.h>

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
        CommandBuffer(CommandBuffer&&) = delete;
        CommandBuffer(const CommandBuffer&) = delete;

    public:
        ~CommandBuffer();

        operator VkCommandBuffer() const { return _commandBuffer; }

    public: /* Recording */
        void BeginRecording(VkCommandBufferUsageFlagBits usageFlags);
        void FinishRecording();

        template <typename T, typename... Args, typename std::enable_if<std::is_base_of<Command, T>::value, int>::type = 0>
        void Record(Args&&... args)
        {
            T command(std::forward<Args>(args)...);
            command.Enqueue(this);
        }

    private:
        VkCommandBuffer _commandBuffer;
    };
}