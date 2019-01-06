#include <graphics/vulkan/Sampler.h>

namespace gfx::vk
{
    Sampler::Sampler(Device* device, VkSampler handle) : _device(device), _handle(handle)
    {

    }
}
