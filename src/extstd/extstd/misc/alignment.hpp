#pragma once

namespace extstd::misc
{
    /**
     * Aligns the provided value upwards, to the boundary defined by {@param alignment}.
     * For example, align_up(26, 4) should return 7 (because 7 * 4 = 28).
     *
     * Stolen from RenderDoc.
     */
    template <typename T>
    constexpr static T align_up(T from, T alignment) {
        return (from + alignment - 1) & (~(alignment - 1));
    }

    /**
     * Aligns the provided value downwards, to the boundary defined by {@param alignment}.
     * For example, align_down(26, 4) should return 6 (because 6 * 4 = 24).
     */
    template <typename T>
    constexpr static T align_down(T from, T alignment) {
        return align_up(from, alignment) - alignment;
    }

    static_assert(align_up(26u, 4u) == 28); // 7 * 4 = 28
    static_assert(align_down(26u, 4u) == 24); // 6 * 4 = 24
}