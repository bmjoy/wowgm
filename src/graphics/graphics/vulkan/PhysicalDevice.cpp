#include "PhysicalDevice.hpp"
#include "Instance.hpp"

#include "VK.hpp"

namespace gfx::vk
{
    PhysicalDevice::PhysicalDevice(Instance* instance, VkPhysicalDevice handle) : _instance(instance), _handle(handle)
    {
        vkGetPhysicalDeviceProperties(_handle, &_properties);
    }
}
