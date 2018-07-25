#pragma once

#include <cstdint>
namespace wowgm::protocol::authentification
{
    #pragma pack(push, 1)
    struct LogonProof
    {
        std::uint8_t A[32];
        std::uint8_t M1[20];
        std::uint8_t CRC[20];
        std::uint8_t NumberOfKeys;
        std::uint8_t SecurityFlags;
    };
    #pragma pack(pop)

    #pragma pack(push, 1)
    struct AuthLogonProof
    {
        std::uint8_t Command;
        std::uint8_t Error;
        union {
            std::uint8_t M2[20];
            //std::uint16_t ErrorReason;
        };
        std::uint32_t Unk1;
        std::uint32_t Unk2;
        std::uint16_t Unk3;
    };
    #pragma pack(pop)

} // namespace wowgm::protocol::authentification
