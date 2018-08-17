#pragma once

#include <cstdint>
#include <string>
#include <type_traits>

#include "FileSystem.hpp"
#include "Assert.hpp"

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

    template <typename T, typename Meta>
    struct Storage
    {
        using header_type = typename std::conditional<Meta::sparse_storage, DB2Header, DBCHeader>::type;
        using record_type = T;

        using iterator = typename std::vector<T>::iterator;
        using const_iterator = typename std::vector<T>::const_iterator;

        constexpr Storage()
        {
        }

        void Initialize();

        void LoadRecords(std::uint8_t const* data, std::uintptr_t stringPool);

        void LoadSparseRecords(std::uint8_t const* data, std::uintptr_t stringPool)
        {
            BOOST_ASSERT_MSG(false, "Not implemented");
        }

        void CopyToMemory(std::uint32_t index, std::uint8_t const* data, std::uintptr_t stringPool);

        T& operator [] (int index);

        T const& operator [] (int index) const;

        iterator begin() noexcept;
        const_iterator begin() const noexcept;

    private:
        header_type _header;

        std::vector<T> _storage;
        std::vector<std::uint8_t> _stringTable;
    };

}
