#pragma once

#include <graphics/vulkan/Traits.hpp>

namespace gfx::vk
{
    /**
     * Convert a VEZ object to its underlying handle.
     */
    template <typename T>
    inline auto AsObjectHandle(T* source) -> typename traits<T>::handle_type {
        return source->GetHandle();
    }

    /**
     * Converts a container of VEZ wrappers to a container of the underlying vulkan handles.
     */
    template <typename Container>
    inline auto AsObjectHandles(Container const& container) -> std::vector<
        typename traits<
            typename Container::value_type
        >::handle_type>
    {
        using value_t = typename Container::value_type;
        using handle_t = typename traits<value_t>::handle_type;

        using decayed_value_t = typename std::remove_pointer<typename std::decay<value_t>::type>::type;

        auto fn = &AsObjectHandle<decayed_value_t>;

        auto itr = boost::make_transform_iterator(std::begin(container), fn);
        auto end = boost::make_transform_iterator(std::end(container), fn);

        return std::vector<handle_t>(itr, end);
    }
}
