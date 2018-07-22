#pragma once
#include "Instance.hpp"

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

        Surface(Surface&&) = delete;
        Surface(const Surface&) = delete;

    public:
        ~Surface();

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