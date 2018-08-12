#pragma once

#include <cstdint>
#include <string>
#include <type_traits>

#include "Storage.hpp"
#include "Assert.hpp"

namespace wowgm::game::datastores
{
    using namespace vks::storage;

    struct DBCHeader
    {
        std::uint32_t Magic;
        std::uint32_t RecordCount;
        std::uint32_t FieldCount;
        std::uint32_t RecordSize;
        std::uint32_t StringBlockSize;
    };

    struct DB2Header
    {
        std::uint32_t Magic;
        std::uint32_t RecordCount;
        std::uint32_t FieldCount;
        std::uint32_t RecordSize;
        std::uint32_t StringTableSize;
        std::uint32_t TableHash;
        std::uint32_t Build;
        std::uint32_t TimestampLastWritten;
        std::uint32_t MinIndex;
        std::uint32_t MaxIndex;
        std::uint32_t Locale;
        std::uint32_t CopyTableSize;
    };

    template <typename T, typename Meta>
    struct Storage
    {
        using header_type = typename std::conditional<Meta::sparse_storage, DB2Header, DBCHeader>::type;
        using record_type = T;

        using iterator = std::vector<T>::iterator;
        using const_iterator = std::vector<T>::const_iterator;

        constexpr Storage()
        {
            StoragePointer file(Storage::readFile(Meta::name()));
            memcpy(&_header, file->data(), sizeof(header_type));

            BOOST_ASSERT_MSG_FMT(_header.Magic == 'CBDW' && !Meta::sparse_storage, "File %s is WDBC but meta marks it as sparse. Re-generate file metadata.", Meta::name());
            BOOST_ASSERT_MSG_FMT(_header.Magic == '2BDW' && Meta::sparse_storage, "File %s is WDB2 but meta marks it as non-sparse. Re-generate file metadata.", Meta::name());

            std::uint8_t* recordData = file->data() + sizeof(header_type);
            if constexpr (Meta::sparse_storage)
                recordData += (4 + 2) * (_header.MaxIndex - _header.MinIndex + 1);
            std::uint8_t* stringTableData = recordData + _header.RecordCount * _header.RecordSize;

            _stringTable.assign(stringTableData, stringTableData + _header.StringTableSize);
            LoadRecords(recordData, reinterpret_cast<std::uint32_t>(&_stringTable.data()));
        }

        void LoadRecords(std::uint8_t* data, std::uint32_t stringPool);
        void LoadSparseRecords(std::uint8_t* data, std::uint32_t stringPool);

        void CopyToMemory(std::uint32_t index, std::uint8_t* data, std::uint32_t stringPool)
        {
#if _WIN64
            // 64-bits arch requires some specific fuckery, because string offsets are stored as 4-bytes, which obviously doesn't match anymore.
            // We thus maintain two cursors, one within the data record, and one within the file.
            // The data record cursor is maintained using sizeof(T).
            // The file cursor is maintained using Meta::field_offsets.

            std::uint32_t memoryOffset = 0;
            for (std::uint32_t j = 0; j < Meta::field_count; ++j)
            {
                std::uint8_t* memberTarget = reinterpret_cast<std::uint8_t*>(&_storage[index]) + memoryOffset;
                switch (Meta::field_types[j])
                {
                    case 's':
                        for (std::uint32_t k = 0; k < Meta::array_sizes[j]; ++j)
                        {
                            // Advance file offset by 4 * k, since string offsets are stored on 4 bytes.
                            *memberTarget = *reinterpret_cast<std::uint32_t*>(data + Meta::field_offsets[j] + 4u * k);

                            // Advance to the next member
                            memoryOffset += sizeof(std::uintptr_t);

                            // But also advance our own pointer in case we are an array.
                            memberTarget += sizeof(std::uintptr_t);
                        }
                        break;
                    default: // Any other type (u64, u32, u16, u8, f32) is the same size in file and arch.

                        // memory offset and good old data are the same layout, because we don't host any pointer to a string

                        // Client metadata exposes the size of every field but that's not dumped.
                        // Instead, infer it from (ofs_next_member - ofs_current_member) / arity_current_member.
                        // Of course, ofs_next_member becomes record_size if we are the last field of the record.
                        std::uint32_t fieldSize = (j + 1 == Meta::field_count ? Meta::record_size : Meta::field_offsets[j + 1]) - Meta::field_offsets[j];

                        memcpy(memberTarget, data + Meta::field_offsets[j], fieldSize);
                        memoryOffset += fieldSize;
                        break;
                }
            }
#elif _WIN32
            // x86 is just good old memcpy.
            memcpy(&_storage[index], data, _header.RecordSize);
#else
#error "Unknown Architecture."
#endif
            DBMeta<T>::AdjustStringOffsets(&_storage[i], stringPool);
        }

        T& operator [] (int index) {
            return _storage[index];
        }

        T const& operator [] (int index) const {
            return _storage[index];
        }

        iterator begin() noexcept { return _storage.begin(); }
        const_iterator begin() const noexcept { return _storage.begin(); }

    private:
        header_type header;

    protected:
        std::vector<T> _storage;
        std::vector<std::uint8_t> _stringTable;
    };
}
