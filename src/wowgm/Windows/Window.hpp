#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vulkan/vulkan.h>

#include <cstdint>
#include <vector>

namespace wowgm::windows
{
#ifdef _DEBUG
# define ENABLE_VALIDATION_LAYERS
#endif

    class Window
    {
    public:
        Window(std::uint32_t width, std::uint32_t height, const char* title);
        Window(const char* title);

        void Run();

        void InitializeWindow();

    private:
        GLFWwindow* _window;

        void InitializeVulkan();
        void Execute();
        void Cleanup();

        std::uint32_t _width, _height;
        const char* _title;

    private: /* Vulkan initializer */

        void SelectPhysicalDevice();

        void InitializeLogicalDevice();
    };
}

