#pragma once
#include "Assert.hpp"

#include <vulkan/vulkan.h>

namespace wowgm::graphics
{
    class LogicalDevice;

    template <typename T, typename C>
    using PrimitiveAllocator = VkResult(*)(VkDevice, C const*, VkAllocationCallbacks const*, T*);

    template <typename T>
    using PrimitiveDestroyer = void(*)(VkDevice, T, VkAllocationCallbacks const*);

    template <typename T, typename C, VkStructureType Type>
    class SynchronizationPrimitive
    {
        static_assert(std::is_same<T, VkSemaphore>::value || std::is_same<T, VkFence>::value,
            "SynchronizationPrimitive<...> is limited to VkFence and VkSemaphore!");
        static_assert(std::is_same<C, VkSemaphoreCreateInfo>::value || std::is_same<C, VkFenceCreateInfo>::value,
            "SynchronizationPrimitive<...> is limited to VkFence and VkSemaphore!");

    protected:
        SynchronizationPrimitive(LogicalDevice* device, PrimitiveAllocator<T, C> allocator, PrimitiveDestroyer<T> destroyer)
            : _device(device), _destroyer(destroyer)
        {
            C createInfo = {};
            // This works because VkStructureType is the first member - we need something more robust if that changes
            *reinterpret_cast<VkStructureType*>(&createInfo) = Type;

            if (allocator(*device, &createInfo, nullptr, &_primitive) != VK_SUCCESS)
                wowgm::exceptions::throw_with_trace(std::runtime_error("Unable to create a synchronization primitive"));
        }

        virtual ~SynchronizationPrimitive()
        {
            _destroyer(*_device, _primitive, nullptr);

            _primitive = VK_NULL_HANDLE;
            _device = nullptr;
        }

    public:
        inline operator T() const { return _primitive; }

    private:
        T _primitive;
        LogicalDevice* _device;
        PrimitiveDestroyer<T> _destroyer;
    };

    class Semaphore : public SynchronizationPrimitive<VkSemaphore, VkSemaphoreCreateInfo, VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO>
    {
    public:
        Semaphore(LogicalDevice* device) : SynchronizationPrimitive(device, &vkCreateSemaphore, &vkDestroySemaphore)
        {

        }
    };

    class Fence : public SynchronizationPrimitive<VkFence, VkFenceCreateInfo, VK_STRUCTURE_TYPE_FENCE_CREATE_INFO>
    {
    public:
        Fence(LogicalDevice* device) : SynchronizationPrimitive(device, &vkCreateFence, &vkDestroyFence)
        {

        }
    };
}
