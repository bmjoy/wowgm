#include "Buffer.hpp"
#include "Device.hpp"

namespace vez
{
    Buffer* Buffer::CreateFromDevice(Device* device, const BufferCreateInfo* pCreateInfo, VkBuffer bufferHandle, VmaAllocation allocation)
    {
        Buffer* buffer = new Buffer;
        buffer->_handle = bufferHandle;
        buffer->_device = device;
        buffer->_allocation = allocation;
        buffer->_size = pCreateInfo->size;

#if _DEBUG
        if (pCreateInfo->pBufferName != nullptr)
            buffer->SetName(pCreateInfo->pBufferName);
#endif

        return buffer;
    }

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
