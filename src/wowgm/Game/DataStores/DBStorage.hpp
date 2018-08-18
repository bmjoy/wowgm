#pragma once

#include <cstdint>
#include <string>
#include <type_traits>

#include "FileSystem.hpp"
#include "Assert.hpp"
#include "DBTraits.hpp"

namespace wowgm::game::datastores
{
    using namespace wowgm::filesystem;

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
        std::uint32_t StringBlockSize;
        std::uint32_t TableHash;
        std::uint32_t Build;
        std::uint32_t TimestampLastWritten;
        std::uint32_t MinIndex;
        std::uint32_t MaxIndex;
        std::uint32_t Locale;
        std::uint32_t CopyTableSize;
    };

    template <typename T>
    struct Storage
    {
        using meta_t = typename meta_type<T>::type;
        static_assert(!std::is_same<meta_t, std::nullptr_t>::value, "");

        using header_type = typename std::conditional<meta_t::sparse_storage, DB2Header, DBCHeader>::type;
        using record_type = T;

        static void Initialize();

        static void LoadRecords(std::uint8_t const* data);

        static void LoadSparseRecords(std::uint8_t const* data, std::uintptr_t stringPool)
        {
            BOOST_ASSERT_MSG(false, "Not implemented");
        }

        static void CopyToMemory(std::uint32_t index, std::uint8_t const* data);

        static T* GetRecord(std::uint32_t index);

    private:
        static header_type& get_header();
        static std::unordered_map<std::uint32_t, T>& get_storage();

        static std::vector<std::uint8_t>& get_string_table();
    };

}
