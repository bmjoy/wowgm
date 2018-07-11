#include "Surface.hpp"
#include "Instance.hpp"

#include <vulkan/vulkan.h>

namespace wowgm::graphics
{
    Surface::Surface(Instance* instance, VkSurfaceKHR surface) : _instance(instance), _surface(surface)
    {

    }

    Surface::~Surface()
    {
        vkDestroySurfaceKHR(_instance->GetVkInstance(), _surface, nullptr);
    }

    VkSurfaceKHR Surface::GetVkSurface()
    {
        return _surface;
    }

    Instance* Surface::GetInstance()
    {
        return _instance;
    }
}
