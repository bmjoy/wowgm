#pragma once

#include <shared/cryptography/BigNumber.hpp>

#include <shared/cryptography/ARC4.hpp>

#include <cstdint>

namespace wowgm::protocol
{
    using namespace shared::crypto;

    class PacketCrypt
    {
    public:
        PacketCrypt(uint32_t digestLength);
        virtual ~PacketCrypt() { }

        virtual void Init(const BigNumber& K) = 0;
        void DecryptRecv(uint8_t* data, size_t length);
        void EncryptSend(uint8_t* data, size_t length);

        bool IsInitialized() const { return _initialized; }

    protected:
        ARC4 _serverDecrypt;
        ARC4 _clientEncrypt;
        bool _initialized;
    };
}
