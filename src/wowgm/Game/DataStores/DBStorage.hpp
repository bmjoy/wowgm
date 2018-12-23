#pragma once

#include <cstdint>
#include <string>
#include <type_traits>

#include "FileSystem.hpp"
#include <shared/assert/assert.hpp>
#include "DBTraits.hpp"

namespace wowgm::game::datastores
{
    using namespace shared::filesystem;

    struct DBCHeader
    {
        uint32_t Magic;
        uint32_t RecordCount;
        uint32_t FieldCount;
        uint32_t RecordSize;
        uint32_t StringBlockSize;
    };

    struct DB2Header
    {
        uint32_t Magic;
        uint32_t RecordCount;
        uint32_t FieldCount;
        uint32_t RecordSize;
        uint32_t StringBlockSize;
        uint32_t TableHash;
        uint32_t Build;
        uint32_t TimestampLastWritten;
        uint32_t MinIndex;
        uint32_t MaxIndex;
        uint32_t Locale;
        uint32_t CopyTableSize;
    };

    template <typename T>
    struct Storage
    {
        using meta_t = typename meta_type<T>::type;
        static_assert(!std::is_same<meta_t, std::nullptr_t>::value, "");

        using header_type = typename std::conditional<meta_t::sparse_storage, DB2Header, DBCHeader>::type;
        using record_type = T;

        static void Initialize();

        static void LoadRecords(uint8_t const* data);

        static void CopyToMemory(uint32_t index, uint8_t const* data);

        static T* GetRecord(uint32_t index);

    private:
        static header_type& get_header();
        static std::unordered_map<uint32_t, T>& get_storage();

        static std::vector<uint8_t>& get_string_table();
    };

}
