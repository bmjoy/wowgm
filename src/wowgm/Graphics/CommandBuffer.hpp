#pragma once

#include <type_traits>
#include <vulkan/vulkan.h>
#include <cstdint>

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
        VkCommandBuffer& AsCommandBuffer() { return _commandBuffer; }

    public: /* Recording */
        void BeginRecording(VkCommandBufferUsageFlagBits usageFlags);
        void FinishRecording();

        template <typename T, typename... Args, typename std::enable_if<std::is_base_of<Command, T>::value, int>::type = 0>
        void Record(Args&&... args)
        {
            T command(std::forward<Args>(args)...);
            command.Enqueue(this);
        }

        void Draw(std::uint32_t vertexCount);
        void Draw(std::uint32_t vertexCount, std::uint32_t instanceCount);

    private:
        VkCommandBuffer _commandBuffer;
    };
}