#include <graphics/vulkan/Buffer.hpp>
#include <graphics/vulkan/Device.hpp>
#include <graphics/vulkan/Instance.hpp>

namespace gfx::vk
{
    VkResult Buffer::Map(void** ppData)
    {
        return _device->MapBuffer(this, ppData);
    }

    void Buffer::Unmap()
    {
        _device->UnmapBuffer(this);
    }

    Instance* Buffer::GetInstance() const
    {
        return _device->GetInstance();
    }

    VkResult Buffer::WriteBytes(VkDeviceSize offset, const uint8_t* data, VkDeviceSize dataSize)
    {
        uint8_t* mappedData = nullptr;
        VkResult mapResult = Map((void**)&mappedData);
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
