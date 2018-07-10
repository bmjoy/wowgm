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

    class window
    {
    public:
        window(std::uint32_t width, std::uint32_t height, const char* title);
        window(const char* title);

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

        // Creates an instance of Vulkan.
        void CreateVulkanInstance();

        // Sets up a debug callback. This is a no-op in release builds.
        void SetupDebugCallback();
    public: /* Vulkan information getters */

        // Returns informations about all available extensions on the provided layer.
        // If nullptr is passed, information about all layers is returned
        std::vector<VkExtensionProperties> GetAvailableExtensions(const char* layerName = nullptr);

        // Checks if the system supports validation layers. This is a no-op in release builds and always returns true.
        bool CheckValidationLayerSupport();

        // Returns a list of required extensions on the system.
        std::vector<const char*> GetRequiredExtensions();

    private: /* Vulkan data */
        VkInstance _instance;
        VkDebugReportCallbackEXT _debugReportCallback;
    };
}

