#include "Surface.hpp"
#include "Instance.hpp"

#include <vulkan/vulkan.h>

namespace wowgm::graphics
{
    Surface::Surface(Instance* instance, VkSurfaceKHR surface, std::uint32_t width, std::uint32_t height) : _instance(instance), _surface(surface), _width(width), _height(height)
    {

    }

    Surface::~Surface()
    {
        vkDestroySurfaceKHR(*_instance, _surface, nullptr);
        _surface = VK_NULL_HANDLE;
    }

    Instance* Surface::GetInstance()
    {
        return _instance;
    }

    std::uint32_t Surface::GetWidth()
    {
        return _width;
    }

    std::uint32_t Surface::GetHeight()
    {
        return _height;
    }
}
