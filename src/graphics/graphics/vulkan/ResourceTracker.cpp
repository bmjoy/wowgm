#include <graphics/vulkan/ResourceTracker.hpp>
#include <graphics/vulkan/Device.hpp>

namespace gfx::vk
{
    ResourceTracker::ResourceTracker(Device* device) : _device(device)
    {

    }

    ResourceTracker::~ResourceTracker()
    {
        Clear();
    }

    void ResourceTracker::Collect(VkFence fence)
    {
        deleter_list newDeleterList;
        newDeleterList.swap(_dumpster);
        _recycler.push(std::make_pair(fence, [newDeleterList]() -> void {
            for (const auto& f : newDeleterList)
                f();
        }));
    }

    void ResourceTracker::ScheduleForRelease(Buffer* buffer)
    {
        GetDevice()->DestroyBuffer(buffer);
    }

    void ResourceTracker::ScheduleForRelease(VkFence fence)
    {
        GetDevice()->DestroyFence(fence);
    }

    void ResourceTracker::ScheduleForRelease(VkSemaphore semapore)
    {
        GetDevice()->DestroySemaphore(semapore);
    }

    void ResourceTracker::Clear()
    {
        while (!_recycler.empty() && GetDevice()->GetFenceStatus(_recycler.front().first) == VK_SUCCESS)
        {
            VkFence fence = _recycler.front().first;
            deleter_t deleter = _recycler.front().second;

            _recycler.pop();

            deleter();

            //if (_recycler.empty() || fence != _recycler.front().first)
            //    GetDevice()->DestroyFence(fence);
        }
    }

}
