#include "DBStorage.hpp"
#include "DBCMeta.hpp"
#include "DBCStructures.hpp"

#include "Profiler.hpp"

// Fucking windows.
#ifdef max
#undef max
#endif

namespace wowgm::game::datastores
{
    template <typename T, typename Meta>
    void Storage<T, Meta>::Initialize()
    {
        std::string completeFilePath = "DBFilesClient\\";
        completeFilePath += Meta::name();

        auto fileHandle = MpqFileSystem::Instance()->OpenFile(completeFilePath, LoadStrategy::Memory);
        if (fileHandle == nullptr)
            return;

        PROFILE;

        _storage.clear();
        memcpy(&_header, fileHandle->GetData(), sizeof(header_type));
        _storage.resize(_header.RecordCount);

        if (!Meta::sparse_storage)
            BOOST_ASSERT_MSG_FMT(_header.Magic == 'CBDW', "File %s is WDBC but meta marks it as sparse. Re-generate file metadata.", Meta::name());
        else
            BOOST_ASSERT_MSG_FMT(_header.Magic == '2BDW', "File %s is WDB2 but meta marks it as non-sparse. Re-generate file metadata.", Meta::name());

        std::uint8_t const* recordData = fileHandle->GetData() + sizeof(header_type);
        if constexpr (Meta::sparse_storage)
            recordData += (4 + 2) * (_header.MaxIndex - _header.MinIndex + 1);
        std::uint8_t const* stringTableData = recordData + _header.RecordCount * _header.RecordSize;

        _stringTable.assign(stringTableData, stringTableData + _header.StringBlockSize);

        if constexpr (!Meta::sparse_storage)
            LoadRecords(recordData, reinterpret_cast<std::uintptr_t>(_stringTable.data()));
        else
            LoadSparseRecords(data, 0);
    }

    template <typename T, typename Meta>
    T& Storage<T, Meta>::operator [] (int index)
    {
        return _storage[index];
    }

    template <typename T, typename Meta>
    T const& Storage<T, Meta>::operator [] (int index) const
    {
        return _storage[index];
    }

    template <typename T, typename Meta>
    auto Storage<T, Meta>::begin() noexcept -> iterator
    {
        return _storage.begin();
    }

    template <typename T, typename Meta>
    auto Storage<T, Meta>::begin() const noexcept -> const_iterator
    {
        return _storage.begin();
    }

    template <typename T, typename Meta>
    void Storage<T, Meta>::LoadRecords(std::uint8_t const* data, std::uintptr_t stringPool)
    {
        for (std::uint32_t i = 0; i < _header.RecordCount; ++i)
            CopyToMemory(i, data + i * Meta::record_size, stringPool);
    }

    template <typename T, typename Meta>
    void Storage<T, Meta>::CopyToMemory(std::uint32_t index, std::uint8_t const* data, std::uintptr_t stringPool)
    {
        static_assert(alignof(T) == 1, "Structures passed to Storage<T, ...> must be aligned to 1 byte. Use #pragma pack(push, 1)!");

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
                for (std::uint32_t k = 0; k < Meta::array_sizes[j]; ++k)
                {
                    // Advance file offset by 4 * k, since string offsets are stored on 4 bytes.
                    auto stringTableOffset = *reinterpret_cast<std::uint32_t const*>(data + Meta::field_offsets[j] + 4u * k);
                    const char* stringValue = reinterpret_cast<const char*>(&_stringTable[stringTableOffset]);

                    *reinterpret_cast<std::uintptr_t*>(memberTarget) = reinterpret_cast<std::uintptr_t>(&_stringTable[stringTableOffset]);

                    memoryOffset += sizeof(std::uintptr_t); // // Advance to the next member ...
                    memberTarget += sizeof(std::uintptr_t); // ... But also advance our own pointer in case we are an array.
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
    }

    template struct Storage<MapEntry, MapMeta>;
}