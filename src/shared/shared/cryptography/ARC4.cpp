#include "ARC4.hpp"

namespace shared::crypto
{
    ARC4::ARC4(uint32_t len) : m_ctx(EVP_CIPHER_CTX_new())
    {
        EVP_CIPHER_CTX_init(m_ctx);
        EVP_EncryptInit_ex(m_ctx, EVP_rc4(), nullptr, nullptr, nullptr);
        EVP_CIPHER_CTX_set_key_length(m_ctx, len);
    }

    ARC4::ARC4(uint8_t* seed, uint32_t len) : m_ctx(EVP_CIPHER_CTX_new())
    {
        EVP_CIPHER_CTX_init(m_ctx);
        EVP_EncryptInit_ex(m_ctx, EVP_rc4(), nullptr, nullptr, nullptr);
        EVP_CIPHER_CTX_set_key_length(m_ctx, len);
        EVP_EncryptInit_ex(m_ctx, nullptr, nullptr, seed, nullptr);
    }

    ARC4::~ARC4()
    {
        EVP_CIPHER_CTX_free(m_ctx);
    }

    void ARC4::Init(uint8_t* seed)
    {
        EVP_EncryptInit_ex(m_ctx, nullptr, nullptr, seed, nullptr);
    }

    void ARC4::UpdateData(uint32_t len, uint8_t* data)
    {
        int outlen = 0;
        EVP_EncryptUpdate(m_ctx, data, &outlen, data, len);
        EVP_EncryptFinal_ex(m_ctx, data, &outlen);
    }
}
