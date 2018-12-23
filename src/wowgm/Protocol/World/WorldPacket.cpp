#include "WorldPacket.hpp"

#include <shared/assert/assert.hpp>

namespace wowgm::protocol::world
{
    void WorldPacket::Decompress(z_stream* decompressionStream)
    {
        uint32_t uncompressedOpcode = uint32_t(GetOpcode());
        if (!(uncompressedOpcode & 0x8000))
            return;

        Opcode opcode = Opcode(uncompressedOpcode & ~0x8000);

        uint32_t decompressedSize = *reinterpret_cast<uint32_t*>(contents());
        uint8_t* compressedData = contents();
        uint32_t compressedDataSize = size() - 4;

        std::vector<uint8_t> decompressedStorage(decompressedSize);

        _decompressionStream = decompressionStream;
        Decompress(decompressedStorage.data(), &decompressedSize, compressedData, compressedDataSize);
        if (decompressedSize == 0)
            return;

        clear();
        // Remove the excess data
        decompressedStorage.resize(decompressedSize);
        _storage = std::move(decompressedStorage);

        SetOpcode(opcode);
    }

    void WorldPacket::Decompress(uint8_t* dst, uint32_t* dst_size, uint8_t* src, uint32_t src_size)
    {
        // The client does +6 because it considers the opcode as part of the byte array. We don't.

        BOOST_ASSERT(src_size >= 4);
        uint32_t avail_in = *reinterpret_cast<uint32_t*>(src); // Client +2s here, byte-based
        BOOST_ASSERT(avail_in <= 0x7FFFFF);

        _decompressionStream->next_out = static_cast<Bytef*>(dst);
        _decompressionStream->avail_out = *dst_size;
        _decompressionStream->next_in = static_cast<Bytef*>(src + 4);
        _decompressionStream->avail_in = src_size;

        int32_t z_res = inflate(_decompressionStream, Z_SYNC_FLUSH);
        BOOST_ASSERT_MSG_FMT(z_res == Z_OK, "Error when decompressing: %u (%s)", z_res, zError(z_res));

        if (_decompressionStream->avail_in != 0)
        {
            *dst_size = 0;
            return;
        }

        *dst_size -= _decompressionStream->avail_out;
    }
}