#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace wowgm::graphics
{
    class Instance;

    class Surface
    {
    public:
        Surface(Instance* instance, VkSurfaceKHR surface);
        ~Surface();

        VkSurfaceKHR GetSurface();

    private:
        Instance * _instance;
        VkSurfaceKHR _surface;
    };
}