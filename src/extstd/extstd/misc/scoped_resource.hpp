#pragma once

#include <extstd/traits/function_traits.hpp>

#include <memory>
#include <utility>

namespace extstd::misc
{
    template <typename D>
    struct scope_manager
    {
        scope_manager(D fn) {
            mgr = fn;
        }

        void operator()(T t) {
            mgr(t);
        }

        scope_manager(scope_manager&& other) {
            mgr = std::move(other.mgr);
        }

        scope_manager(scope_manager const& other) {
            mgr = other.mgr;
        }

        using pointer = typename extstd::traits::function_traits<D>::arg<0>::type;

    private:
        D mgr;
    };

    template <typename C, typename... Args>
    struct scope_manager : public scope_manager<void(C::*)(Args...)> {
        scope_manager(void(C::*)(Args...) fn, Args&&... args) : scope_manager([&](pointer ptr) -> void {
            fn(ptr, std::forward<Args>(args)...);
        }) {

        }
    };

    template <typename T, typename D>
    using scoped_resource = std::unique_ptr<T, scope_manager<T>>;

    template <typename T, typename D>
    auto make_scoped_resource(T resource, D&& deleter) -> scoped_resource<T, scope_manager<D>> {
        return scoped_resource<T, scope_manager<D>>(resource, scope_manager<D>(std::forward<D>(deleter)));
    }
}
