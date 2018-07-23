#pragma once

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

#include <vulkan/vulkan.h>

namespace wowgm::graphics
{
    class Window;
    class Instance;
    class Surface;
    class SwapChain;
    class RenderPass;

    class GUI
    {
        GUI(GUI&&) = delete;
        GUI(const GUI&) = delete;

    public:

        explicit GUI(Window* window, Instance* instance, Surface* surface, SwapChain* swapChain);
        ~GUI();

        void Draw();

    private: /* ImGUI */

        Window* _window;
        Instance* _instance;
        Surface* _surface;
        SwapChain* _swapChain;

        void ResizeFrameBuffer(int width, int height);

        void CreateWindowDataCommandBuffers();


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