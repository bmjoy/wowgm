#pragma once

#include "imgui.h"
#include "imgui_impl_vulkan.h"

#include <vulkan/vulkan.h>

namespace wowgm::graphics
{
    class Window;
    class Instance;
    class Surface;

    class GUI
    {
    public:

        explicit GUI(Window* window, Instance* instance, Surface* surface);
        ~GUI();

    private: /* ImGUI */

        Window* _window;
        Instance* _instance;
        Surface* _surface;

        void ResizeFrameBuffer(int width, int height);

        void Draw();

        static void CheckVulkanResult(VkResult result);

    private: /* Vulkan */

        VkAllocationCallbacks*       _allocator = nullptr;
        VkPhysicalDevice             _physicalDevice = VK_NULL_HANDLE;
        VkDevice                     _device = VK_NULL_HANDLE;
        uint32_t                     _queueFamily = (uint32_t)-1;
        VkQueue                      _queue = VK_NULL_HANDLE;
        VkPipelineCache              _pipelineCache = VK_NULL_HANDLE;
        VkDescriptorPool             _descriptorPool = VK_NULL_HANDLE;

        ImGui_ImplVulkanH_WindowData _windowData;
        bool                         _resizeWanted = false;
        int _resizeWidth = 0, _resizeHeight = 0;
    };
}