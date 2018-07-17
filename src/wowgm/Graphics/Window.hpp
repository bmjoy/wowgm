#pragma once

#include <vulkan/vulkan.h>

#include <cstdint>
#include <vector>

struct GLFWwindow;

namespace wowgm::graphics
{
    class Window
    {
    public:
        Window(std::uint32_t width, std::uint32_t height, const char* title);
        Window(const char* title);

        Window(Window&&) = delete;
        Window(Window const&) = delete;

        void InitializeWindow();
        void Execute();
        void Cleanup();

        bool ShouldClose();

        GLFWwindow* GetHandle();

        std::uint32_t GetWidth();
        std::uint32_t GetHeight();

    private:
        GLFWwindow* _window;

        std::uint32_t _width, _height;
        const char* _title;
    };
}

