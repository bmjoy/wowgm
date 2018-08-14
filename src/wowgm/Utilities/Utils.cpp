#include "Utils.hpp"
#include "Defines.hpp"

#include <sstream>

#if PLATFORM == PLATFORM_WINDOWS
struct tm* localtime_r(const time_t* time, struct tm *result)
{
    localtime_s(result, time);
    return result;
}
#endif

namespace wowgm::utilities
{
    std::string ByteArrayToHexStr(std::uint8_t const* bytes, std::uint32_t arrayLen, bool reverse)
    {
        std::int32_t init = 0;
        std::int32_t end = arrayLen;
        std::int8_t op = 1;

        if (reverse)
        {
            init = arrayLen - 1;
            end = -1;
            op = -1;
        }

        std::ostringstream ss;
        for (std::int32_t i = init; i != end; i += op)
        {
            char buffer[4];
            sprintf(buffer, "%02X", bytes[i]);
            ss << buffer;
        }

        return ss.str();
    }
}
