#pragma once

#include <cstdint>

namespace wowgm::protocol::authentification
{
    enum AuthResult : uint8_t
    {
        LOGIN_OK                  = 0,
        LOGIN_BANNED              = 3,
        LOGIN_UNKNOWN_ACCOUNT     = 4, // Also 5
        LOGIN_ALREADY_ONLINE      = 6,
        LOGIN_NO_TIME             = 7, // Nerd
        LOGIN_BUSY                = 8,
        LOGIN_BAD_VERSION         = 9,
        LOGIN_DOWNLOAD_FILE       = 10,
        LOGIN_SUSPENDED           = 12,
        LOGIN_PARENTAL_CONTROL    = 15,
        LOGIN_LOCKED_ENFORCED     = 16,
        LOGIN_CONVERSION_REQUIRED = 32,

        // Custom values
        LOGIN_NOT_STARTED         = 253, // User hasn't started connecting
        LOGIN_INVALID_SRP6        = 254, // Custom value


        LOGIN_DISCONNECTED        = 255
    };

} // namespace wowgm
