#pragma once

#include <type_traits>
#include <VEZ.h>

struct VmaAllocation_T;
typedef struct VmaAllocation_T* VmaAllocation;

typedef struct VezMappedBufferCreateInfo : public VezBufferCreateInfo
{
    bool alwaysMapped = false;
} VezMappedBufferCreateInfo;

namespace vez
{
    class Buffer;

    namespace extensions
    {
        class MappedBuffer;

        VkResult createMappedBuffer(VkDevice device, VezMemoryFlags memFlags, const VezMappedBufferCreateInfo* pCreateInfo, MappedBuffer** mappedBuffer);
        VkResult destroyMappedBuffer(MappedBuffer* buffer);

        class MappedBuffer
        {
        public:
            // Implicit cast operator
            operator Buffer() const { return *_buffer; }

            MappedBuffer(Buffer* buffer);
            ~MappedBuffer();

            template <typename T, size_t N>
            VkResult UploadData(VkDeviceSize offset, T (&data)[N])
            {
                static_assert(std::is_standard_layout<T>::value && std::is_trivial<T>::value);

                VkDeviceSize requestedSize = sizeof(T) * N;
                return UploadData<T>(data, offset, requestedSize);
            }

            template <typename T>
            inline VkResult UploadData(T* data, VkDeviceSize offset, VkDeviceSize size)
            {
                // If we are outside of the default-mapped range (which only ever happens if alwaysMapped
                // was set to false when creating this object), create a new map
                if (_mappedData != nullptr && (offset < _mappedDataOffset || offset + size > _mappedDataOffset + _mappedDataSize))
                {
                    void* mappableData;
                    VkResult result = MapMemory(offset, size, &mappableData);
                    if (result != VK_SUCCESS)
                        return result;

                    memcpy(mappableData, data, size);
                    UnmapMemory();
                    return VK_SUCCESS;
                }

                if (_mappedData == nullptr)
                {
                    VkResult result = MapMemory(offset, size, &_mappedData);
                    if (result != VK_SUCCESS)
                        return result;

                    memcpy(_mappedData, data, size);

                    UnmapMemory();
                    return VK_SUCCESS;
                }

                if (offset >= _mappedDataOffset)
                {
                    std::uint8_t* mappedData = reinterpret_cast<std::uint8_t*>(_mappedData);
                    mappedData += offset - _mappedDataOffset;

                    if (offset + size > _mappedDataOffset + _mappedDataSize)
                        size = _mappedDataSize - offset;

                    memcpy(mappedData, data, size);

                    return VK_SUCCESS;
                }
            }

            VkBuffer GetHandle() { return _buffer->GetHandle(); }

        private:

            MappedBuffer(const MappedBuffer&) = delete;
            MappedBuffer(MappedBuffer&&) = delete;

            VkResult MapMemory(VkDeviceSize offset = 0, VkDeviceSize size = 0, void** mapTarget);
            void UnmapMemory();

        private:
            Buffer* _buffer;
            void* _mappedData;
            VkDeviceSize _mappedDataOffset;
            VkDeviceSize _mappedDataSize;
        };
    }
}
