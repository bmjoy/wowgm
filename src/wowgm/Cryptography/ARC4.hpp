#pragma once

#include <openssl/evp.h>
#include <cstdint>

namespace wowgm::cryptography
{
    class ARC4
    {
        public:
            ARC4(uint32_t len);
            ARC4(uint8_t* seed, uint32_t length);
            ~ARC4();

            void Init(uint8_t* seed);
            void UpdateData(uint32_t len, uint8_t* data);

        private:
            EVP_CIPHER_CTX * m_ctx;
    };

}
