#pragma once

#include <vulkan/vulkan.h>
#include <cstdint>

#include <graphics/vulkan/Buffer.hpp>
#include <graphics/vulkan/NamedObject.hpp>

#include <graphics/vulkan/VK.hpp>

namespace gfx::vk
{
    /**
     * A thin wrapper around a VkBuffer handle.
     * Lifetime management of this object should be handled through Device::DestroyBuffer.
     */
    class Buffer final : public NamedObject<Buffer>
    {
        friend class Device;

        // Mapping needs to be handled by Device::MapBuffer and Device::UnmapBuffer,
        // but we have to be made aware that we are mapped.
        void SetMapped(bool mapped);

    public:

        Device* GetDevice() const { return _device; }
        VkBuffer GetHandle() const { return _handle; }
        Instance* GetInstance() const;

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
        bool _mapped = false;
    };
}