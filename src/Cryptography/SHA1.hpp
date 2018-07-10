#pragma once

#include <string>
#include <type_traits>
#include <openssl/sha.h>

#include "BigNumber.hpp"

namespace crypto
{
    template <typename...>
    struct all_bignumber;

    template <>
    struct all_bignumber<> : std::true_type { };

    template <typename T, typename ...Rest>
    struct all_bignumber<T, Rest...> : std::bool_constant<std::is_same<T, BigNumber>::value && all_bignumber<Rest...>::value>
    { };

    class SHA1
    {
    public:
        typedef std::integral_constant<std::uint32_t, SHA_DIGEST_LENGTH> DigestLength;

        SHA1();
        ~SHA1();

        template <typename T, typename... Args, typename std::enable_if<std::is_same<T, BigNumber>::value, int>::type = 0>
        void UpdateBigNumbers(T& bn0, Args&&... args)
        {
            UpdateData(bn0.AsByteArray().get(), bn0.GetNumBytes());
            UpdateBigNumbers(std::forward<Args>(args)...);
        }

        template <typename T, typename std::enable_if<std::is_same<T, BigNumber>::value, int>::type = 0>
        void UpdateBigNumbers(T& bn0)
        {
            UpdateData(bn0.AsByteArray().get(), bn0.GetNumBytes());
        }

        void UpdateData(const std::uint8_t *dta, int len);
        void UpdateData(const std::string &str);

        void Initialize();
        void Finalize();

        std::uint8_t* GetDigest() const { return (std::uint8_t*)mDigest; }
        int GetLength() const { return SHA_DIGEST_LENGTH; }

    private:
        SHA_CTX mC;
        std::uint8_t mDigest[SHA_DIGEST_LENGTH];
    };

    /// Returns the SHA1 hash of the given content as hex string.
    BigNumber CalculateSHA1(std::string const& content);
}
