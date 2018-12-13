#pragma once

#include <memory>
#include <utility>
#include <functional>

#include <boost/bind/bind.hpp>

namespace extstd
{
    namespace impl
    {
        template <typename Container, typename T, typename Transform>
        inline void extract(Container const& source, std::vector<T>& output, Transform&& transform)
        {
            std::transform(std::begin(source), std::end(source), std::back_inserter(output), std::forward<Transform>(transform));
        }
    }

    template <typename Container>
    inline auto values(Container const& container) -> std::vector<typename std::remove_cv<typename Container::value_type::second_type>::type>
    {
        using vector_t = std::vector<typename std::remove_cv<typename Container::value_type::second_type>::type>;

        vector_t vector;
        impl::extract(container, vector, boost::bind(&Container::value_type::second, boost::placeholders::_1));
        return vector;
    }

    template <typename Container>
    inline auto keys(Container const& container) -> std::vector<typename std::remove_cv<typename Container::value_type::first_type>::type>
    {
        using vector_t = std::vector<typename std::remove_cv<typename Container::value_type::first_type>::type>;

        vector_t vector;
        impl::extract(container, vector, boost::bind(&Container::value_type::first, boost::placeholders::_1));
        return vector;
    }
}
