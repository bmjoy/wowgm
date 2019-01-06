#include <graphics/vulkan/Buffer.hpp>
#include <graphics/vulkan/Device.hpp>
#include <graphics/vulkan/Instance.hpp>

#include <vk_mem_alloc.h>

namespace gfx::vk
{
    void Buffer::SetMapped(bool mapped)
    {
        _mapped = mapped;
    }

    Instance* Buffer::GetInstance() const
    {
        return _device->GetInstance();
    }

    uint32_t Buffer::GetSize() const
    {
        if (_allocation == VK_NULL_HANDLE)
            return 0;

        VmaAllocationInfo allocInfo;
        vmaGetAllocationInfo(GetDevice()->GetAllocator(), _allocation, &allocInfo);
        return allocInfo.size;
    }

    VkResult Buffer::WriteBytes(VkDeviceSize offset, const uint8_t* data, VkDeviceSize dataSize)
    {
        uint8_t* mappedData = nullptr;
        VkResult mapResult = _device->MapBuffer(this, (void**)&mappedData);
        if (mapResult == VK_SUCCESS)
        {
            if (offset + dataSize > _size)
                return VK_INCOMPLETE;

            memcpy(mappedData, data, dataSize);
            return VK_SUCCESS;
        }
        else
        {
            // Memory is not visible to the host.
            // A) If the device has an open and recording command buffer,
            //    insert our buffer commands into it.
            // B) Otherwise, acquire a one-time-submit command buffer,
            //    do our upload, and submit the command buffer.
        }

        return VK_INCOMPLETE;
    }
}
