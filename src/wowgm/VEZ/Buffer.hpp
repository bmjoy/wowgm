#pragma once

#include <vulkan/vulkan.h>
#include <cstdint>

#include "VEZ.hpp"
#include "NamedObject.hpp"

namespace vez
{
    class Buffer : public NamedObject<Buffer>
    {
    public:

        static Buffer* CreateFromDevice(Device* device, const BufferCreateInfo* pCreateInfo, VkBuffer bufferHandle, VmaAllocation allocation);

        Device* GetDevice() const override { return _device; }
        VkBuffer GetHandle() const { return _handle; }
        VmaAllocation GetAllocation() const { return _allocation; }

        Instance* GetInstance() const override;

        VkResult Map(void** ppData);
        void Unmap();

        template <typename T>
        inline VkResult WriteMemory(VkDeviceSize offset, const T* data, VkDeviceSize dataSize) {
            return WriteBytes(offset, reinterpret_cast<const std::uint8_t*>(data), dataSize * sizeof(T));
        }

        template <typename T, typename std::enable_if<std::is_data_container<T>::value, int>::type = 0>
        inline VkResult WriteMemory(VkDeviceSize offset, T const& container) {
            WriteMemory<typename T::element_type>(offset, container.data(), container.size() * sizeof(T));
        }

        VkResult WriteBytes(VkDeviceSize offset, const std::uint8_t* data, VkDeviceSize dataSize);

    private:
        VkBuffer _handle = VK_NULL_HANDLE;
        Device* _device = nullptr;
        VmaAllocation _allocation = VK_NULL_HANDLE;
        VkDeviceSize _size = 0;
    };
}