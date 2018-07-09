#pragma once

#include <cstdint>

#include "FourCC.hpp"

#pragma pack(push, 1)
struct LogonChallenge
{
    LogonChallenge() { }

    std::uint8_t Error;
    std::uint16_t Size;
    FourCC Game;
    std::uint8_t Version[3];
    std::uint16_t Build;
    FourCC Platform;
    FourCC OS;
    FourCC CountryCode;
    std::uint32_t timeZoneBias;
    std::uint32_t IP;
    struct
    {
        std::uint8_t Length;
    } Name;
};

struct AuthLogonChallenge
{
    std::uint8_t Command;
    std::uint8_t Unk;
    std::uint8_t Error;
    std::uint8_t B[32];
    std::uint8_t g_length;
    std::uint8_t g[1];
    std::uint8_t n_length;
    std::uint8_t N[32];
    std::uint8_t Salt[32];
    std::uint8_t unk2[16];
    std::uint8_t SecurityFlags;
};
#pragma pack(pop)