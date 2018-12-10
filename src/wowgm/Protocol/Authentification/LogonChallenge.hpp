#pragma once

#include <cstdint>

#include "FourCC.hpp"

namespace wowgm::protocol::authentification
{
    #pragma pack(push, 1)
    struct LogonChallenge
    {
        LogonChallenge() { }

        uint8_t Error;
        uint16_t Size;
        wowgm::datatypes::FourCC Game;
        uint8_t Version[3];
        uint16_t Build;
        wowgm::datatypes::FourCC Platform;
        wowgm::datatypes::FourCC OS;
        wowgm::datatypes::FourCC CountryCode;
        uint32_t timeZoneBias;
        uint32_t IP;
        struct {
            uint8_t Length;
        } Name;
    };
    #pragma pack(pop)

    #pragma pack(push, 1)
    struct AuthLogonChallenge
    {
        uint8_t Command;
        uint8_t Unk;
        uint8_t Error;
        uint8_t B[32];
        uint8_t g_length;
        uint8_t g[1];
        uint8_t n_length;
        uint8_t N[32];
        uint8_t Salt[32];
        uint8_t unk2[16];
        uint8_t SecurityFlags;
    };
    #pragma pack(pop)

} // namespace wowgm::protocol::authentification
