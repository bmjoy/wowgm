#pragma once

#include <vulkan/vulkan.h>
#include <cstdint>

#include "VK.hpp"


namespace gfx::vk
{
    class Buffer
    {
        friend class Device;
        Buffer() { }

    public:

        Device* GetDevice() const { return _device; }
        VkBuffer GetHandle() const { return _handle; }
        VmaAllocation GetAllocation() const { return _allocation; }

        Instance* GetInstance() const;

        VkResult Map(void** ppData);
        void Unmap();

        template <typename T>
        inline VkResult WriteMemory(VkDeviceSize offset, const T* data, VkDeviceSize dataSize) {
            return WriteBytes(offset, reinterpret_cast<const uint8_t*>(data), dataSize * sizeof(T));
        }

        VkResult WriteBytes(VkDeviceSize offset, const uint8_t* data, VkDeviceSize dataSize);

    private:
        VkBuffer _handle = VK_NULL_HANDLE;
        Device* _device = nullptr;
        VmaAllocation _allocation = VK_NULL_HANDLE;
        VkDeviceSize _size = 0;
    };
}