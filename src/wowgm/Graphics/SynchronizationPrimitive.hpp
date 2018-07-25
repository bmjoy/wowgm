#pragma once
#include "Assert.hpp"

#include <vulkan/vulkan.h>

namespace wowgm::graphics
{
    class LogicalDevice;

    template <typename T>
    using PrimitiveDestroyer = void(*)(VkDevice, T, VkAllocationCallbacks const*);

    template <typename T>
    class SynchronizationPrimitive
    {
        static_assert(std::is_same<T, VkSemaphore>::value || std::is_same<T, VkFence>::value,
            "SynchronizationPrimitive<...> is limited to VkFence and VkSemaphore!");

    protected:
        SynchronizationPrimitive(LogicalDevice* device, PrimitiveDestroyer<T> destroyer)
            : _device(device), _destroyer(destroyer)
        {
            _primitive = VK_NULL_HANDLE;
        }

        virtual ~SynchronizationPrimitive()
        {
            _destroyer(*_device, _primitive, nullptr);

            _primitive = VK_NULL_HANDLE;
            _device = nullptr;
        }

        T _primitive;

    public:
        inline operator T() const { return _primitive; }

    private:
        LogicalDevice* _device;
        PrimitiveDestroyer<T> _destroyer;
    };

    class Semaphore : public SynchronizationPrimitive<VkSemaphore>
    {
    public:
        explicit Semaphore(LogicalDevice* device) : SynchronizationPrimitive(device, &vkDestroySemaphore)
        {
            VkSemaphoreCreateInfo createInfo = {};
            createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

            if (vkCreateSemaphore(*device, &createInfo, nullptr, &_primitive) != VK_SUCCESS)
                wowgm::exceptions::throw_with_trace<std::runtime_error>("Unable to create a semaphore");
        }
    };

    class Fence : public SynchronizationPrimitive<VkFence>
    {
    public:
        explicit Fence(LogicalDevice* device) : SynchronizationPrimitive(device, &vkDestroyFence)
        {
            VkFenceCreateInfo createInfo = { };
            createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
            createInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

            if (vkCreateFence(*device, &createInfo, nullptr, &_primitive) != VK_SUCCESS)
                wowgm::exceptions::throw_with_trace<std::runtime_error>("Unable to create a fence");
        }
    };
}
