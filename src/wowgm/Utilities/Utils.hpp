#pragma once

#include <ctime>
#include <string>
#include <cstdint>

namespace wowgm::utilities
{
    struct tm* localtime_r(const time_t* time, struct tm *result);

    std::string ByteArrayToHexStr(std::uint8_t const* bytes, std::uint32_t arrayLen, bool reverse = false);
}