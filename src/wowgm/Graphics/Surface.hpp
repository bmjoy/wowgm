#pragma once
#include "Instance.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <cstdint>

namespace wowgm::graphics
{
    class Window;

    /*
     * A surface abstracts the native platform's (Windows, Linux, Android, and more) windowing/surface mechanisms.
     *
     * This implementation uses GLFW's surface/window create methods, for cross-compatibility.
     */
    class Surface
    {
        friend Surface* Instance::CreateSurface(Window* window);

        Surface(Instance* instance, VkSurfaceKHR surface, std::uint32_t _width, std::uint32_t height);

    public:
        ~Surface();

        Surface(Surface const&) = delete;

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