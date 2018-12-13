#pragma once

#include <memory>
#include <utility>

#include <set>
#include <vector>

namespace extstd
{
    namespace impl
    {
        template <typename T>
        void do_concat(T&) { }

        template <typename Out, typename Top, typename... Tail>
        inline void do_concat(Out& out, Top&& top, Tail&&... args)
        {
            std::copy(std::begin(top), std::end(top), std::end(out));
            do_concat(out, std::forward<Tail>(args)...);
        }
    }

    template <template <typename...> typename Out, typename... Containers>
    inline auto concatenate(Containers&&... args) -> Out<typename std::tuple_element<0, std::tuple<Containers...>>::type>
    {
        using top_t = typename std::tuple_element<0, std::tuple<Containers...>>::type>;
        using output_t = Out<typename top_t::value_type>;

        output_t out;
        impl::do_concat(out, std::forward<Containers>(args)...);
        return out;
    }

    int main()
    {
        std::vector<int> x, y;
        std::set<int> z;

        auto o = concatenate<std::vector>(x, y, z);
    }
}
