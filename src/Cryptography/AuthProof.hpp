#pragma once

#include <cstdint>

#pragma pack(push, 1)
struct AuthProof
{
    std::uint8_t A[32];
    std::uint8_t M1[20];
    std::uint8_t CRC[20];
    std::uint8_t NumberOfKeys;
    std::uint8_t SecurityFlags;
};

#pragma pack(pop)
