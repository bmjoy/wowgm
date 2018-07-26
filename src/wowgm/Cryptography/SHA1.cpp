#include "SHA1.hpp"

#include <cstring>
#include <stdarg.h>

namespace wowgm::cryptography
{
    SHA1::SHA1()
    {
        SHA1_Init(&mC);
        memset(mDigest, 0, SHA_DIGEST_LENGTH * sizeof(std::uint8_t));
    }

    SHA1::~SHA1()
    {
        SHA1_Init(&mC);
    }

    void SHA1::UpdateData(const std::uint8_t *dta, int len)
    {
        SHA1_Update(&mC, dta, len);
    }

    void SHA1::UpdateData(const std::string &str)
    {
        UpdateData((std::uint8_t const*)str.c_str(), str.length());
    }

    void SHA1::UpdateData(char c)
    {
        UpdateData((std::uint8_t const*)&c, 1);
    }

    void SHA1::Initialize()
    {
        SHA1_Init(&mC);
        memset(mDigest, 0, SHA_DIGEST_LENGTH * sizeof(std::uint8_t));
    }

    void SHA1::Finalize()
    {
        SHA1_Final(mDigest, &mC);
    }

} // namespace wowgm::cryptography