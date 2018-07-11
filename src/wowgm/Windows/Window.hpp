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

    struct QueueFamilyIndices
    {
        std::int32_t graphicsFamily = -1;
    };

    QueueFamilyIndices GetQueueFamilies(VkPhysicalDevice device)
    {
        QueueFamilyIndices indices;

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        int i = 0;
        for (const auto& queueFamily : queueFamilies) {
            if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
                indices.graphicsFamily = i;

            i++;
        }

        return indices;
    }

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

        void SelectPhysicalDevice();

        void InitializeLogicalDevice();
    };
}

