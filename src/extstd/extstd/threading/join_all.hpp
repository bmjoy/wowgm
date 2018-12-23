#pragma once

#include <thread>
#include <type_traits>
#include <algorithm>

namespace extstd::threading
{
    template <template <typename...> typename C, typename... Args>
    inline void join_all(C<std::thread, Args...>& threads)
    {
        std::for_each(std::begin(threads), std::end(threads), [](std::thread& thr) { thr.join(); });
    }
}
