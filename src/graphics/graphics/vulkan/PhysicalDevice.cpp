#include <graphics/vulkan/PhysicalDevice.hpp>
#include <graphics/vulkan/Instance.hpp>

#include <graphics/vulkan/VK.hpp>

namespace gfx::vk
{
    PhysicalDevice::PhysicalDevice(Instance* instance, VkPhysicalDevice handle) : _instance(instance), _handle(handle)
    {
        vkGetPhysicalDeviceProperties(_handle, &_properties);
    }
}
