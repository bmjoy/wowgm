#include "PhysicalDevice.hpp"
#include "Instance.hpp"

#include "VEZ.hpp"

namespace vez
{
    PhysicalDevice::PhysicalDevice(Instance* instance, VkPhysicalDevice handle) : _instance(instance), _handle(handle)
    {
        vkGetPhysicalDeviceProperties(_handle, &_properties);
    }
}
