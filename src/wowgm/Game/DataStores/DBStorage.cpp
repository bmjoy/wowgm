#include "DBStorage.hpp"
#include "DBCMeta.hpp"
#include "DBCStructures.hpp"

#include "Profiler.hpp"

#include <shared/filesystem/mpq_file_system.hpp>

// Fucking windows.
#ifdef max
#undef max
#endif

namespace wowgm::game::datastores
{
    template <typename T>
    void Storage<T>::Initialize()
    {
        std::string completeFilePath = "DBFilesClient\\";
        completeFilePath += meta_t::name();

        auto fileHandle = mpq_file_system::Instance()->OpenFile(completeFilePath);
        if (fileHandle == nullptr)
            return;

        PROFILE;

        get_storage().clear();
        memcpy(&get_header(), fileHandle->GetData(), sizeof(header_type));

        if (!meta_t::sparse_storage)
            BOOST_ASSERT_MSG_FMT(get_header().Magic == 'CBDW', "File %s is WDBC but meta marks it as sparse. Re-generate file metadata.", meta_t::name());
        else
            BOOST_ASSERT_MSG_FMT(get_header().Magic == '2BDW', "File %s is WDB2 but meta marks it as non-sparse. Re-generate file metadata.", meta_t::name());

        uint8_t const* recordData = fileHandle->GetData() + sizeof(header_type);
        if constexpr (meta_t::sparse_storage)
            recordData += (4 + 2) * (get_header().MaxIndex - get_header().MinIndex + 1);
        uint8_t const* stringTableData = recordData + get_header().RecordCount * get_header().RecordSize;

        get_string_table().assign(stringTableData, stringTableData + get_header().StringBlockSize);

        // sparse tables can be loaded just like non-sparse if they don't have strings
        LoadRecords(recordData);
    }

    template <typename T>
    T* Storage<T>::GetRecord(uint32_t index)
    {
        return &get_storage()[index];
    }

    template <typename T>
    void Storage<T>::LoadRecords(uint8_t const* data)
    {
        for (uint32_t i = 0; i < get_header().RecordCount; ++i)
            CopyToMemory(i, data + i * meta_t::record_size);
    }

    template <typename T>
    void Storage<T>::CopyToMemory(uint32_t index, uint8_t const* data)
    {
        static_assert(alignof(T) == 1, "Structures passed to Storage<T, ...> must be aligned to 1 byte. Use #pragma pack(push, 1)!");

        uint32_t memoryOffset = 0;
        uint8_t* structure_ptr = reinterpret_cast<uint8_t*>(&get_storage()[*reinterpret_cast<uint32_t const*>(data)]); // fixme find indexof n
        for (uint32_t j = 0; j < meta_t::field_count; ++j)
        {
            uint8_t* memberTarget = structure_ptr + memoryOffset;
            BOOST_ASSERT(reinterpret_cast<uintptr_t>(memberTarget) < reinterpret_cast<uintptr_t>(structure_ptr + sizeof(T)));

            auto array_size = meta_t::field_sizes[j];
            auto memory_size_item = 4u;
            switch (meta_t::field_types[j])
            {
                case 's':
                case 'i':
                case 'n':
                case 'u':
                case 'f':
                    break;
                case 'l':
                    memory_size_item = 8u;
                    break;
                case 'b':
                    memory_size_item = 1u;
                    break;
            }

            array_size /= memory_size_item;

            if (meta_t::field_types[j] == 's')
            {
                for (uint32_t i = 0; i < array_size; ++i)
                {
                    if constexpr (true) // (!meta_t::sparse_storage) // WDB2 still has a string table
                    {
                        auto stringTableOffset = *reinterpret_cast<uint32_t const*>(data + meta_t::field_offsets[j] + 4u * i);
                        const char* stringValue = reinterpret_cast<const char*>(&get_string_table()[stringTableOffset]);
                        *reinterpret_cast<uintptr_t*>(memberTarget) = reinterpret_cast<uintptr_t>(stringValue);
                    }
                    else
                    {
                        uint8_t const* recordMember = data + meta_t::field_offsets[j] + 4u * i;
                        size_t stringLength = strlen(reinterpret_cast<const char*>(recordMember));
                        auto itr = &*get_string_table().insert(get_string_table().end(), recordMember, recordMember + stringLength);
                        *reinterpret_cast<uintptr_t*>(memberTarget) = *reinterpret_cast<uintptr_t*>(*itr);
                    }

                    memoryOffset += sizeof(uintptr_t); // // Advance to the next member ...
                    memberTarget += sizeof(uintptr_t); // ... But also advance our own pointer in case we are an array.
                }
            }
            else
            {
                if (memory_size_item > meta_t::field_sizes[j])
                {
                    for (uint32_t i = 0; i < array_size; ++i)
                    {
                        memcpy(memberTarget, data + meta_t::field_offsets[j] + i * memory_size_item, memory_size_item);
                        constexpr static const uint32_t masks[] = { 0x000000FFu, 0x0000FFFFu, 0x00FFFFFFu, 0xFFFFFFFFu };
                        *reinterpret_cast<uint32_t*>(memberTarget) &= masks[meta_t::field_sizes[j] - 1];

                        memoryOffset += memory_size_item;
                        memberTarget += memory_size_item;
                    }
                }
                else
                {
                    memcpy(memberTarget, data + meta_t::field_offsets[j], meta_t::field_sizes[j]);
                    memoryOffset += meta_t::field_sizes[j];
                }
            }
        }
    }

    template <typename T>
    auto Storage<T>::get_header() -> header_type&
    {
        static header_type header;
        return header;
    }

    template <typename T>
    auto Storage<T>::get_storage() -> std::unordered_map<uint32_t, T>&
    {
        static std::unordered_map<uint32_t, T> _storage;
        return _storage;
    }

    template <typename T>
    auto Storage<T>::get_string_table() -> std::vector<uint8_t>&
    {
        static std::vector<uint8_t> _stringTable;
        return _stringTable;
    }

    template struct Storage<MapEntry>;
    template struct Storage<ItemSparseEntry>;
    template struct Storage<SpellEntry>;
    template struct Storage<ChrClassesEntry>;
    template struct Storage<ChrRacesEntry>;
}
