#pragma once

#include <memory>
#include <vector>

#include <vulkan/vulkan.h>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_vulkan.h"
#include "imgui/imgui_impl_glfw.h"

namespace wowgm::graphics
{
    class Instance;
    class Surface;
    class Window;
    class LogicalDevice;
    class SwapChain;
    class RenderPass;
    class Pipeline;
    class FrameBuffer;
    class CommandBuffer;
    class ImageView;
    class Semaphore;

    class Interface
    {
    public:
        explicit Interface(std::unique_ptr<Instance>& instance, Window* window);
        ~Interface();

        void Draw();

        void PrepareCommandBuffers();

        void RenderImGui(VkSemaphore image_acquired_semaphore, VkSemaphore signalSemaphore);

        void CreatePipeline();

    private:
        Surface* _surface;
        Window* _window;
        LogicalDevice* _device;
        SwapChain* _swapChain;

        struct RenderStage
        {
            RenderPass* RenderPass;
            Pipeline* Pipeline;
            std::vector<FrameBuffer*> FrameBuffers;
            std::vector<CommandBuffer*> CommandBuffers;
        };

        std::vector<Semaphore*> _interfaceSemaphores;

        RenderStage _geometryRenderStage;

        ImGui_ImplVulkanH_WindowData _interfaceWindowData;
        VkPipelineCache g_PipelineCache = VK_NULL_HANDLE;
        VkDescriptorPool g_DescriptorPool = VK_NULL_HANDLE;
    };
}
