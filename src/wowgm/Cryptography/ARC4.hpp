#pragma once

#include <openssl/evp.h>
#include <cstdint>

namespace wowgm::cryptography
{
    class ARC4
    {
        public:
            ARC4(std::uint32_t len);
            ARC4(std::uint8_t* seed, std::uint32_t length);
            ~ARC4();

            void Init(std::uint8_t* seed);
            void UpdateData(std::uint32_t len, std::uint8_t* data);

        private:
            EVP_CIPHER_CTX * m_ctx;
    };

}
