#include "Window.hpp"
#include "Logger.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stdexcept>
#include <iostream>
#include <cstring>

namespace wowgm::graphics
{
    Window::Window(const char* title) : Window(800u, 600u, title)
    {

    }

    Window::Window(std::uint32_t width, std::uint32_t height, const char* title) : _width(width), _height(height), _title(title)
    {
    }

    void Window::InitializeWindow()
    {
        glfwInit();

        // No API (Originally designed for OpenGL contexts)
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        // Disable resize for now.
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        _window = glfwCreateWindow(_width, _height, _title, nullptr, nullptr);
    }

    GLFWwindow* Window::GetHandle()
    {
        return _window;
    }

    void Window::Execute()
    {
        // Also plug ourselves in here.
        while (!glfwWindowShouldClose(_window))
            glfwPollEvents();
    }

    void Window::Cleanup()
    {
        glfwDestroyWindow(_window);
        glfwTerminate();
    }

    std::uint32_t Window::GetWidth()
    {
        return _width;
    }

    std::uint32_t Window::GetHeight()
    {
        return _height;
    }
}

