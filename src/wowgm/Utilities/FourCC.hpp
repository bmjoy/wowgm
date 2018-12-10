#pragma once

#include <cstdint>
#include <string_view>

#include "MessageBuffer.hpp"
#include "Endianness.hpp"

namespace wowgm
{
    namespace datatypes
    {
        using namespace wowgm::protocol;

        struct FourCC
        {
        public:
            FourCC(MessageBuffer& buffer)
            {
                Data.Integer = *reinterpret_cast<uint32_t*>(buffer.GetReadPointer());
            }

            FourCC() { Data.Integer = 0; }

            union {
                uint32_t Integer;
                char Bytes[4];
            } Data;

            std::string_view ToString() const
            {
                return std::string_view(Data.Bytes, std::size(Data.Bytes));
            }

            operator uint32_t() const { return Data.Integer; }

            FourCC& operator = (const char* data)
            {
                memcpy(Data.Bytes, data, std::min(std::size_t(4), strlen(data)));
                Data.Integer = utils::endianness::swap(int32_t(Data.Integer));
                return *this;
            }

            FourCC& operator = (uint32_t value)
            {
                Data.Integer = value;
                return *this;
            }

            FourCC& operator = (const std::string& value)
            {
                Data.Integer = 0;
                memcpy(Data.Bytes, value.c_str(), std::min(std::size_t(4), value.size()));
                Data.Integer = utils::endianness::swap(int32_t(Data.Integer));
                return *this;
            }
        };

        static_assert(sizeof(FourCC) == 4);

    } // namespace datatypes

} // namespace wowgm