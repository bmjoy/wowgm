#pragma once

#include "BigNumber.hpp"
#include "ARC4.hpp"

#include <cstdint>

namespace wowgm::protocol
{
    using namespace wowgm::cryptography;

    class PacketCrypt
    {
    public:
        PacketCrypt(std::uint32_t digestLength);
        virtual ~PacketCrypt() { }

        virtual void Init(BigNumber* K) = 0;
        void DecryptRecv(std::uint8_t* data, size_t length);
        void EncryptSend(std::uint8_t* data, size_t length);

        bool IsInitialized() const { return _initialized; }

    protected:
        ARC4 _clientDecrypt;
        ARC4 _serverEncrypt;
        bool _initialized;
    };
}
