#pragma once

#include <memory>
#include <string>

struct bignum_st;

namespace crypto
{
    class SHA1;

    class BigNumber
    {
    public:
        BigNumber();
        BigNumber(BigNumber const& bn);
        BigNumber(BigNumber&& bn);
        BigNumber(std::uint32_t value);
        ~BigNumber();

        void SetDword(std::uint32_t value);
        void SetQword(std::uint64_t value);
        void SetBinary(std::uint8_t const* bytes, std::int32_t len);
        void SetBinary(SHA1 const& sha1);
        void SetHexStr(char const* str);

        void SetRand(std::int32_t numbits);

        BigNumber& operator=(BigNumber const& bn);

        BigNumber operator+=(BigNumber const& bn);
        BigNumber operator+(BigNumber const& bn)
        {
            BigNumber t(*this);
            return t += bn;
        }

        BigNumber operator-=(BigNumber const& bn);
        BigNumber operator-(BigNumber const& bn)
        {
            BigNumber t(*this);
            return t -= bn;
        }

        BigNumber operator*=(BigNumber const& bn);
        BigNumber operator*(BigNumber const& bn)
        {
            BigNumber t(*this);
            return t *= bn;
        }

        BigNumber operator/=(BigNumber const& bn);
        BigNumber operator/(BigNumber const& bn)
        {
            BigNumber t(*this);
            return t /= bn;
        }

        BigNumber operator%=(BigNumber const& bn);
        BigNumber operator%(BigNumber const& bn)
        {
            BigNumber t(*this);
            return t %= bn;
        }

        bool IsZero() const;
        bool IsNegative() const;

        BigNumber ModExp(BigNumber const& bn1, BigNumber const& bn2);
        BigNumber Exp(BigNumber const&);

        std::int32_t GetNumBytes(void);

        struct bignum_st *BN() { return _bn; }

        std::uint32_t AsDword();

        std::unique_ptr<std::uint8_t[]> AsByteArray(std::int32_t minSize = 0, bool littleEndian = true);

        std::string AsHexStr() const;
        std::string AsDecStr() const;

    private:
        struct bignum_st *_bn;

    };

} // namespace crypto