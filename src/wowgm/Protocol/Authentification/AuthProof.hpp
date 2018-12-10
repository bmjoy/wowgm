#pragma once

#include <cstdint>
namespace wowgm::protocol::authentification
{
    #pragma pack(push, 1)
    struct LogonProof
    {
        uint8_t A[32];
        uint8_t M1[20];
        uint8_t CRC[20];
        uint8_t NumberOfKeys;
        uint8_t SecurityFlags;
    };
    #pragma pack(pop)

    #pragma pack(push, 1)
    struct AuthLogonProof
    {
        uint8_t Command;
        uint8_t Error;
        union {
            uint8_t M2[20];
            //uint16_t ErrorReason;
        };
        uint32_t Unk1;
        uint32_t Unk2;
        uint16_t Unk3;
    };
    #pragma pack(pop)

} // namespace wowgm::protocol::authentification
