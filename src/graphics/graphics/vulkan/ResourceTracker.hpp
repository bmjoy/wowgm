#pragma once

#include <vulkan/vulkan.h>
#include <functional>
#include <vector>
#include <list>
#include <utility>
#include <algorithm>
#include <queue>

namespace gfx::vk
{
    class Device;
    class Buffer;

    class ResourceTracker
    {
    public:
        ResourceTracker(Device* device);
        ~ResourceTracker();

        template <typename T>
        inline void ScheduleForRelease(T value)
        {
            static_assert(std::is_pointer<T>::value, "T must be a pointer. Try passing a lambda.");

            ScheduleForRelease(value, [](T elem) -> void { delete elem; });
        }

        template <template <typename...> class C, typename T>
        inline void ScheduleForRelease(C<T> container)
        {
            for (auto itr = std::begin(container); itr != std::end(container); ++itr)
                ScheduleForRelease(*itr);
        }

        void ScheduleForRelease(Buffer* buffer);
        void ScheduleForRelease(VkFence fence);
        void ScheduleForRelease(VkSemaphore semapore);

        template <typename T>
        inline void ScheduleForRelease(T value, std::function<void(T)> releaser)
        {
            if (!value)
                return;

            _dumpster.push_back([=]() {
                releaser(value);
            });
        }

        void Collect(VkFence fence);

        void Clear();

        Device* GetDevice() const { return _device; }

    private:
        Device* _device;
        VkFence _fence;

        using deleter_t = std::function<void()>;
        using deleter_list = std::list<deleter_t>;
        using fenced_lambda = std::pair<VkFence, deleter_t>;
        using fenced_lambda_queue = std::queue<fenced_lambda>;

        mutable deleter_list _dumpster;
        fenced_lambda_queue _recycler;

    };
}