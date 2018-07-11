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
        vkDestroySurfaceKHR(_instance->GetInstance(), _surface, nullptr);
    }

    VkSurfaceKHR Surface::GetSurface()
    {
        return _surface;
    }
}
