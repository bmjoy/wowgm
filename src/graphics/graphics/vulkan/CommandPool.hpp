#pragma once

#include <vulkan/vulkan.h>

#include <graphics/vulkan/VK.hpp>
#include <graphics/vulkan/Spinlock.hpp>

namespace gfx::vk
{
    class CommandPool
    {
    public:
        static VkResult Create(Device* device, uint32_t queueFamilyIndex, CommandPool** ppCommandPool);

        ~CommandPool();

        Device* GetDevice() const { return _device; }

        uint32_t GetQueueFamilyIndex() const { return _queueFamilyIndex; }

        VkCommandPool GetHandle() const { return _handle; }

        VkResult AllocateCommandBuffers(VkCommandBufferLevel level, uint32_t commandBufferCount, VkCommandBuffer* commandBuffers, const void* pNext);

        void FreeCommandBuffers(uint32_t commandBufferCount, const VkCommandBuffer* commandBuffers);

    private:
        Device* _device = nullptr;
        VkCommandPool _handle = VK_NULL_HANDLE;
        Spinlock _spinLock;
        uint32_t _queueFamilyIndex = 0;
    };
}
