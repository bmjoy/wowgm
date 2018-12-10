#pragma once

#include <cstdint>
#include <type_traits>

namespace wowgm
{
    namespace utils
    {
        namespace endianness
        {
            namespace detail
            {
                template<typename UINT>
                UINT byteswap(UINT const a);

                template<>
                constexpr uint16_t byteswap(uint16_t const a) {
                    constexpr uint8_t b[] = { 8 * (2 - 1) };

                    auto const x =
                        (0x00ffULL & a) << b[0] |
                        (0xff00ULL & a) >> b[0]
                        ;

                    return static_cast<uint16_t>(x);
                }

                template<>
                constexpr uint32_t byteswap(uint32_t const a) {
                    constexpr uint8_t b[] = {
                        8 * (2 - 1),
                        8 * (4 - 1)
                    };

                    auto const x =
                        (0x000000ffULL & a) << b[1] |
                        (0x0000ff00ULL & a) << b[0] |
                        (0x00ff0000ULL & a) >> b[0] |
                        (0xff000000ULL & a) >> b[1]
                        ;

                    return static_cast<uint32_t>(x);
                }

                template<>
                constexpr uint64_t byteswap(uint64_t const a) {
                    constexpr uint8_t b[] = {
                        8 * (2 - 1),
                        8 * (4 - 1),
                        8 * (6 - 1),
                        8 * (8 - 1)
                    };

                    auto const x =
                        (0x00000000000000ffULL & a) << b[3] |
                        (0x000000000000ff00ULL & a) << b[2] |
                        (0x0000000000ff0000ULL & a) << b[1] |
                        (0x00000000ff000000ULL & a) << b[0] |
                        (0x000000ff00000000ULL & a) >> b[0] |
                        (0x0000ff0000000000ULL & a) >> b[1] |
                        (0x00ff000000000000ULL & a) >> b[2] |
                        (0xff00000000000000ULL & a) >> b[3]
                        ;

                    return static_cast<uint64_t>(x);
                }

                // ==============================

                class is_big_endian {
                    constexpr static uint32_t u4 = 1;
                    constexpr static uint8_t  u1 = (const uint8_t &)u4;
                public:
                    constexpr static bool value = u1 == 0;
                };

                // ==============================

                // ==============================

                template<typename UINT>
                constexpr UINT htobe(UINT const a, std::bool_constant<true>) {
                    return a;
                }

                template<typename UINT>
                constexpr UINT htobe(UINT const a, std::bool_constant<false>) {
                    return byteswap(a);
                }

            } // namespace myendian_impl_

            /// Swaps only if platform needs it
            constexpr uint16_t platform_swap(uint16_t a) {
                return detail::htobe(a, std::bool_constant<detail::is_big_endian::value>{});
            }

            constexpr uint32_t platform_swap(uint32_t a) {
                return detail::htobe(a, std::bool_constant<detail::is_big_endian::value>{});
            }

            constexpr uint64_t platform_swap(uint64_t a) {
                return detail::htobe(a, std::bool_constant<detail::is_big_endian::value>{});
            }

            constexpr int16_t swap(int16_t a) {
                return (int16_t)detail::htobe(uint16_t(a), std::false_type{});
            }

            constexpr int32_t swap(int32_t a) {
                return (int32_t)detail::htobe(uint32_t(a), std::false_type{});
            }

            constexpr int64_t swap(int64_t a) {
                return (int64_t)detail::htobe(uint64_t(a), std::false_type{});
            }
        }
    }
}
