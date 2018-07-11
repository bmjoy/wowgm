#include "Window.hpp"
#include "Logger.hpp"

#include <stdexcept>
#include <iostream>
#include <cstring>

namespace wowgm::windows
{
    window::window(const char* title) : window(800u, 600u, title)
    {

    }

    window::window(std::uint32_t width, std::uint32_t height, const char* title) : _width(width), _height(height), _title(title)
    {
    }

    void window::InitializeWindow()
    {
        glfwInit();

        // No API (Originally designed for OpenGL contexts)
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        // Disable resize for now.
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        _window = glfwCreateWindow(_width, _height, _title, nullptr, nullptr);
    }

    void window::Run()
    {
        InitializeWindow();
        InitializeVulkan();
        Execute();
        Cleanup();
    }

    void window::InitializeVulkan()
    {

    }


    void window::Execute()
    {
        // Also plug ourselves in here.
        while (!glfwWindowShouldClose(_window))
            glfwPollEvents();
    }

    void window::Cleanup()
    {
        glfwDestroyWindow(_window);
        glfwTerminate();
    }
}

