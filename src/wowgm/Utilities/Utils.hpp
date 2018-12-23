#pragma once

#include <ctime>
#include <string>
#include <cstdint>
#include <type_traits>

struct tm* localtime_r(const time_t* time, struct tm *result);

namespace wowgm::utilities
{
    std::string ByteArrayToHexStr(uint8_t const* bytes, uint32_t arrayLen, bool reverse = false);
}
