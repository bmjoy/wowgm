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
        SelectPhysicalDevice();
        InitializeLogicalDevice();
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

    void window::SelectPhysicalDevice()
    {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(_instance, &deviceCount, nullptr);

        if (deviceCount == 0)
            throw std::runtime_error("Your system does not support Vulkan!");

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(_instance, &deviceCount, devices.data());

        std::uint32_t currentBestScore = 0;
        for (const auto& device : devices)
        {
            std::uint32_t deviceScore = IsDeviceSuitable(&device);
            if (deviceScore > currentBestScore)
            {
                currentBestScore = deviceScore;
                _physicalDevice = device;
            }
        }

        if (_physicalDevice == VK_NULL_HANDLE)
            throw std::runtime_error("Failed to find a suitable Vulkan GPU");
    }

    void window::InitializeLogicalDevice()
    {

    }

}

