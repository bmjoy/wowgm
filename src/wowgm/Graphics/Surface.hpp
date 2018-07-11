#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <cstdint>

namespace wowgm::graphics
{
    class Instance;

    class Surface
    {
    public:
        Surface(Instance* instance, VkSurfaceKHR surface, std::uint32_t _width, std::uint32_t height);
        ~Surface();

        VkSurfaceKHR GetVkSurface();
        Instance* GetInstance();

        std::uint32_t GetWidth();
        std::uint32_t GetHeight();

        operator VkSurfaceKHR() const { return _surface; }
    private:
        Instance* _instance;
        VkSurfaceKHR _surface;

        std::uint32_t _width;
        std::uint32_t _height;
    };
}