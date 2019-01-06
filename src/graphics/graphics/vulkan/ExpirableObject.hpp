#pragma once

#include <graphics/vulkan/ResourceTracker.hpp>
#include <graphics/vulkan/Device.hpp>

namespace gfx::vk
{
    template <typename T>
    class ExpirableObject
    {
    public:
        void ScheduleRelease()
        {
            T* self = static_cast<T*>(this);

            Device* device = self->GetDevice();

            device->GetResourceManager()->ScheduleForRelease(self);
        }
    };
}