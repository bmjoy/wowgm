#pragma once

#include <memory>
#include <utility>
#include <functional>
#include <memory>

#include <boost/bind/bind.hpp>

namespace extstd
{
    namespace impl
    {
        template <typename Container, typename T, typename Transform>
        void extract(Container const& source, std::vector<T>& output, Transform&& transform)
        {
            std::transform(std::begin(source), std::end(source), std::back_inserter(output), transform);
        }

        template <typename T>
        void do_concat(T&) { }

        template <typename R, typename Top, typename... Tail>
        inline void do_concat(R& accum, Top top, Tail&&... args) {
            std::copy(std::begin(top), std::end(top), std::back_inserter(accum));
        }
    }

    template <typename Container>
    auto values(Container const& container) -> std::vector<typename std::remove_cv<typename Container::value_type::second_type>::type>
    {
        using vector_t = std::vector<typename std::remove_cv<typename Container::value_type::second_type>::type>;

        vector_t vector;
        impl::extract(container, vector, boost::bind(&Container::value_type::second, boost::placeholders::_1));
        return vector;
    }

    template <typename Container>
    auto keys(Container const& container) -> std::vector<typename std::remove_cv<typename Container::value_type::first_type>::type>
    {
        using vector_t = std::vector<typename std::remove_cv<typename Container::value_type::first_type>::type>;

        vector_t vector;
        impl::extract(container, vector, boost::bind(&Container::value_type::first, boost::placeholders::_1));
        return vector;
    }

    template <typename T, typename... Ts>
    inline std::vector<typename T::value_type> concatenate(T top, Ts&&... args)
    {
        std::vector<typename T::value_type> accumulator;
        impl::do_concat(accumulator, top, std::forward<Ts>(args)...);
    }
}
