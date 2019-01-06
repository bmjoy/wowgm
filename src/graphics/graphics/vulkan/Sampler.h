#pragma once

#include <graphics/vulkan/NamedObject.hpp>

namespace gfx::vk
{
    class Device;

    class Sampler final : NamedObject<Sampler>
    {
        friend class Device;

        Sampler(Device* device, VkSampler handle);

    public:

        VkSampler GetHandle() const { return _handle; }
        Device* GetDevice() const { return _device; }

    private:
        VkSampler _handle;
        Device* _device = nullptr;
    };
}