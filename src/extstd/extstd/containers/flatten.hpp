#pragma once

#include <tuple>

namespace extstd
{
    /*template <template <typename...> class... Containers, typename... Args>
    class flattening_iterator
    {
    private:
        using tuple_t = std::tuple<Containers<Args...>...>;
        tuple_t _tuple;

        using iterator_t = flattening_iterator<Containers, Args>;

    public:
        explicit flattening_iterator(Containers<Args...>&&... args) {
            _tuple = std::forward_as_tuple(args);
        }

        using iterator_category = std::forward_iterator_tag;
        using value_type = typename decltype(std::get<0>(std::tuple_t{}))::value_type;
        using difference_type = int;
        using pointer = typename std::add_pointer<value_type>::type;
        using reference = typename std::add_rvalue_reference<value_type>::type;

    };

    template <template <typename...> class Output, template <typename...> class... Container, typename... Args>
    inline auto flatten(Container<Args...>&&... containers) -> Output<typename std::tuple_element<0, std::tuple<Args...>>::type>
    {
        using output_t = Output<typename std::tuple_element<0, std::tuple<Args...>>::type>;

        output_t output;
        do_flatten(output, std::forward<Container<Args...>>(containers)...);
        return output;
    }*/
}