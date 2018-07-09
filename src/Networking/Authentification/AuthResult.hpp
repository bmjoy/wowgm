#pragma once

#include <cstdint>

enum AuthResult : std::uint8_t
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
    LOGIN_DISCONNECTED        = 255
};
