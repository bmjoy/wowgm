#include "MappedBuffer.hpp"
#include "Core/Buffer.h"
#include "Core/Device.h"
#include "Utility/ObjectLookup.h"

namespace vez
{
    namespace extensions
    {
        MappedBuffer::MappedBuffer(Buffer* buffer) : _buffer(buffer)
        {
            const VezMappedBufferCreateInfo& createInfo = static_cast<const VezMappedBufferCreateInfo&>(_buffer->GetCreateInfo());
            if (createInfo.alwaysMapped)
                MapMemory(0, createInfo.size, &_mappedData);
        }

        MappedBuffer::~MappedBuffer()
        {
            UnmapMemory();

            _buffer->GetDevice()->DestroyBuffer(_buffer);
        }

        VkResult MappedBuffer::MapMemory(VkDeviceSize offset /* = 0 */, VkDeviceSize size /* = 0 */, void** mapTarget)
        {
            if (offset != 0 && offset >= _buffer->GetCreateInfo().size)
                return;

            if (size == 0)
                size = _buffer->GetCreateInfo().size - offset;
            else if (size + offset >= _buffer->GetCreateInfo().size)
                size = _buffer->GetCreateInfo().size - offset;

            _mappedDataOffset = offset;
            _mappedDataSize = size;
            return _buffer->GetDevice()->MapBuffer(_buffer, offset, size, mapTarget);
        }

        void MappedBuffer::UnmapMemory()
        {
            if (_mappedData != nullptr)
                _buffer->GetDevice()->UnmapBuffer(_buffer);

            _mappedData = nullptr;
            _mappedDataOffset = 0;
            _mappedDataSize = 0;
        }

        VkResult createMappedBuffer(VkDevice device, VezMemoryFlags memFlags, const VezMappedBufferCreateInfo* pCreateInfo, MappedBuffer** mappedBuffer)
        {
            auto deviceImpl = vez::ObjectLookup::GetObjectImpl(device);
            if (!deviceImpl)
                return VK_INCOMPLETE;

            // Create the buffer.
            vez::Buffer* bufferImpl = nullptr;
            auto result = deviceImpl->CreateBuffer(memFlags, pCreateInfo, &bufferImpl);
            if (result != VK_SUCCESS)
                return result;

            *mappedBuffer = new vez::extensions::MappedBuffer(bufferImpl);
            return result;
        }

        VkResult destroyMappedBuffer(MappedBuffer* buffer)
        {
            if (buffer == nullptr)
                return VK_INCOMPLETE;

            if (buffer->GetHandle() == VK_NULL_HANDLE)
                return VK_INCOMPLETE;

            delete buffer;
            return VK_SUCCESS;
        }
    }
}
