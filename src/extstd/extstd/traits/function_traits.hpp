#pragma once

#include <tuple>

namespace extstd::traits
{
    template <typename T>
    struct function_traits : public function_traits<decltype(&T::operator())>;

    template <typename R, typename... Args>
    struct function_traits<R(*)(Args...)> {
        using return_type = R;

        enum { arity = sizeof...(Args) };

        template <size_t N>
        struct arg {
            using type = typename std::tuple_element<N, std::tuple<Args...>>::type;
        };
    };

    template <typename C, typename R, typename... Args>
    struct function_traits<R(C::*)(Args...)> : public function_traits<R(*)(Args...)> {
        typedef C& owner_type;
    };

    template <typename T> struct function_traits<T&> : public function_traits<T> {};
    template <typename T> struct function_traits<T const&> : public function_traits<T> {};
    template <typename T> struct function_traits<T volatile&> : public function_traits<T> {};
    template <typename T> struct function_traits<T const volatile&> : public function_traits<T> {};

    template <typename T> struct function_traits<T&&> : public function_traits<T> {};
    template <typename T> struct function_traits<T const&&> : public function_traits<T> {};
    template <typename T> struct function_traits<T volatile&&> : public function_traits<T> {};
    template <typename T> struct function_traits<T const volatile&&> : public function_traits<T> {};

    static_assert(std::is_same<typename function_traits<void(*)(int)>::arg<0>::type, int>::value, "yikes");
}
