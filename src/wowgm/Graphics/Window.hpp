#pragma once

#include <vulkan/vulkan.h>

#include <cstdint>
#include <vector>

struct GLFWwindow;

namespace wowgm::graphics
{
#ifdef _DEBUG
# define ENABLE_VALIDATION_LAYERS
#endif

    class Window
    {
    public:
        Window(std::uint32_t width, std::uint32_t height, const char* title);
        Window(const char* title);

        void InitializeWindow();
        void Execute();
        void Cleanup();

        GLFWwindow* GetHandle();

        std::uint32_t GetWidth();
        std::uint32_t GetHeight();

    private:
        GLFWwindow* _window;

        std::uint32_t _width, _height;
        const char* _title;

    private: /* Vulkan initializer */

        void SelectPhysicalDevice();

        void InitializeLogicalDevice();
    };
}

