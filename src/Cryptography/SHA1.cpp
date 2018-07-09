#include "SHA1.hpp"
#include "Utilities.hpp"

#include <cstring>
#include <stdarg.h>

namespace crypto
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

    void SHA1::Initialize()
    {
        SHA1_Init(&mC);
    }

    void SHA1::Finalize()
    {
        SHA1_Final(mDigest, &mC);
    }

    BigNumber CalculateSHA1(std::string const& content)
    {
        std::uint8_t digest[SHA_DIGEST_LENGTH];
        ::SHA1((unsigned char*)content.c_str(), content.length(), (unsigned char*)&digest);

        BigNumber bigNumber;
        bigNumber.SetBinary(digest, SHA_DIGEST_LENGTH);
        return bigNumber;
    }

} // namespace crypto