#include "GUI.hpp"
#include "Window.hpp"
#include "Assert.hpp"
#include "Surface.hpp"
#include "Instance.hpp"
#include "PhysicalDevice.hpp"
#include "LogicalDevice.hpp"
#include "SwapChain.hpp"
#include "RenderPass.hpp"
#include "ImageView.hpp"
#include "Image.hpp"
#include "FrameBuffer.hpp"

#include <iostream>
#include <stdexcept>
#include <limits>

#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#ifdef _DEBUG
#define IMGUI_VULKAN_DEBU_REPORT
#endif

#undef min
#undef max

namespace wowgm::graphics
{
    GUI::GUI(Window* window, Instance* instance, Surface* surface, SwapChain* swapChain) : _window(window), _instance(instance), _surface(surface), _swapChain(swapChain)
    {
        if (!glfwVulkanSupported())
            wowgm::exceptions::throw_with_trace(std::runtime_error("Vulkan is not supported on your system!"));

        RenderPass* renderPass = instance->GetLogicalDevice()->CreateRenderPass();

        _windowData.Surface = *surface;
        _physicalDevice = *instance->GetPhysicalDevice();
        _queueFamily = instance->GetPhysicalDevice()->GetQueues().Graphics;
        _windowData.Swapchain = *swapChain;
        _windowData.RenderPass = *renderPass;
        _device = *instance->GetLogicalDevice();
        _queue = *instance->GetLogicalDevice()->GetGraphicsQueue();

        _windowData.ClearEnable = true;
        _windowData.ClearValue = { 0.45f, 0.55f, 0.60f, 1.00f };

        int index = 0;
        for (auto&& itr : swapChain->GetImageViews())
        {
            _windowData.BackBuffer[index] = *itr->GetImage();
            _windowData.BackBufferView[index] = *itr;
            _windowData.Framebuffer[index] = *renderPass->GetFrameBuffer(index);

            ++index;
        }

        _windowData.BackBufferCount = index;

       /* glfwSetFramebufferSizeCallback(window->GetHandle(), [this](GLFWwindow* window, int width, int height) -> void {
            this->ResizeFrameBuffer(width, height);
        });*/

        VkBool32 physicalSurfaceSupport;
        vkGetPhysicalDeviceSurfaceSupportKHR(_physicalDevice, _queueFamily, *surface, &physicalSurfaceSupport);

        const VkFormat requestSurfaceImageFormat[] = { VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_B8G8R8_UNORM, VK_FORMAT_R8G8B8_UNORM };
        const VkColorSpaceKHR requestSurfaceColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;

        _windowData.SurfaceFormat = ImGui_ImplVulkanH_SelectSurfaceFormat(_physicalDevice, _windowData.Surface, requestSurfaceImageFormat, IM_ARRAYSIZE(requestSurfaceImageFormat), requestSurfaceColorSpace);

        VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;
        _windowData.PresentMode = ImGui_ImplVulkanH_SelectPresentMode(_physicalDevice, _windowData.Surface, &present_mode, 1);

        CreateWindowDataCommandBuffers();

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        ImGui_ImplGlfw_InitForVulkan(window->GetHandle(), true);

        ImGui_ImplVulkan_InitInfo initInfo = {};
        initInfo.Instance = *_instance;
        initInfo.PhysicalDevice = _physicalDevice;
        initInfo.Device = _device;
        initInfo.QueueFamily = _queueFamily;
        initInfo.Queue = _queue;
        initInfo.PipelineCache = _pipelineCache;
        initInfo.DescriptorPool = _descriptorPool;
        initInfo.Allocator = _allocator;
        initInfo.CheckVkResultFn = &GUI::CheckVulkanResult;
        ImGui_ImplVulkan_Init(&initInfo, _windowData.RenderPass);

        ImGui::StyleColorsDark();

        { // Upload Fonts
            // Use any command queue
            VkCommandPool command_pool = _windowData.Frames[_windowData.FrameIndex].CommandPool;
            VkCommandBuffer command_buffer = _windowData.Frames[_windowData.FrameIndex].CommandBuffer;

            CheckVulkanResult(vkResetCommandPool(_device, command_pool, 0));
            VkCommandBufferBeginInfo begin_info = {};
            begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
            CheckVulkanResult(vkBeginCommandBuffer(command_buffer, &begin_info));

            ImGui_ImplVulkan_CreateFontsTexture(command_buffer);

            VkSubmitInfo end_info = {};
            end_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            end_info.commandBufferCount = 1;
            end_info.pCommandBuffers = &command_buffer;
            CheckVulkanResult(vkEndCommandBuffer(command_buffer));
            CheckVulkanResult(vkQueueSubmit(_queue, 1, &end_info, VK_NULL_HANDLE));

            CheckVulkanResult(vkDeviceWaitIdle(_device));
            ImGui_ImplVulkan_InvalidateFontUploadObjects();
        }
    }

    GUI::~GUI()
    {
        vkDeviceWaitIdle(_device);
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        vkDestroyDescriptorPool(_device, _descriptorPool, _allocator);
    }

    void GUI::CreateWindowDataCommandBuffers()
    {
        for (int i = 0; i < IMGUI_VK_QUEUED_FRAMES; i++)
        {
            ImGui_ImplVulkanH_FrameData* fd = &_windowData.Frames[i];
            {
                VkCommandPoolCreateInfo info = { };
                info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
                info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
                info.queueFamilyIndex = _queueFamily;

                if (vkCreateCommandPool(_device, &info, _allocator, &fd->CommandPool) != VK_SUCCESS)
                    wowgm::exceptions::throw_with_trace(std::runtime_error("Unable to create command pool in ImGui"));
            }

            {
                VkCommandBufferAllocateInfo info = { };
                info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
                info.commandPool = fd->CommandPool;
                info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
                info.commandBufferCount = 1;

                if (vkAllocateCommandBuffers(_device, &info, &fd->CommandBuffer) != VK_SUCCESS)
                    wowgm::exceptions::throw_with_trace(std::runtime_error("Unable to allocate command buffers in ImGui"));
            }

            {
                VkFenceCreateInfo info = {};
                info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
                info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
                if (vkCreateFence(_device, &info, _allocator, &fd->Fence) != VK_SUCCESS)
                    wowgm::exceptions::throw_with_trace(std::runtime_error("Unable to create fence in ImGui"));
            }
            {
                VkSemaphoreCreateInfo info = {};
                info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
                if (vkCreateSemaphore(_device, &info, _allocator, &fd->ImageAcquiredSemaphore) != VK_SUCCESS)
                    wowgm::exceptions::throw_with_trace(std::runtime_error("Unable to create semaphore in ImGui"));
                if (vkCreateSemaphore(_device, &info, _allocator, &fd->RenderCompleteSemaphore) != VK_SUCCESS)
                    wowgm::exceptions::throw_with_trace(std::runtime_error("Unable to create semaphore in ImGui"));
            }
        }
    }

    void GUI::Draw()
    {
        if (_resizeWanted)
        {
            // ImGui_ImplVulkanH_CreateWindowDataSwapChainAndFramebuffer(_physicalDevice, _device, &_windowData, _allocator, _resizeWidth, _resizeHeight);
            _resizeWanted = false;
        }

        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Hello, world");
        ImGui::Text("Account name");
        // ImGui::SameLine();

        bool connectRequested = ImGui::Button("Connect");

        ImGui::End();

        // Done setting up
        ImGui::Render();

        // Render
        VkSemaphore& image_acquired_semaphore = _windowData.Frames[_windowData.FrameIndex].ImageAcquiredSemaphore;
        CheckVulkanResult(vkAcquireNextImageKHR(_device, _windowData.Swapchain, UINT64_MAX, image_acquired_semaphore, VK_NULL_HANDLE, &_windowData.FrameIndex));

        ImGui_ImplVulkanH_FrameData* fd = &_windowData.Frames[_windowData.FrameIndex];

        vkWaitForFences(_device, 1, &fd->Fence, VK_TRUE, std::numeric_limits<std::uint64_t>::max());
        vkResetFences(_device, 1, &fd->Fence);

        {
            vkResetCommandPool(_device, fd->CommandPool, 0);
            VkCommandBufferBeginInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
            vkBeginCommandBuffer(fd->CommandBuffer, &info);
        }

        {
            VkRenderPassBeginInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            info.renderPass = _windowData.RenderPass;
            info.framebuffer = _windowData.Framebuffer[_windowData.FrameIndex];
            info.renderArea.extent.width = _windowData.Width;
            info.renderArea.extent.height = _windowData.Height;
            info.clearValueCount = 1;
            info.pClearValues = &_windowData.ClearValue;
            vkCmdBeginRenderPass(fd->CommandBuffer, &info, VK_SUBPASS_CONTENTS_INLINE);
        }

        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), fd->CommandBuffer);
        vkCmdEndRenderPass(fd->CommandBuffer);

        {
            VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            VkSubmitInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            info.waitSemaphoreCount = 1;
            info.pWaitSemaphores = &image_acquired_semaphore;
            info.pWaitDstStageMask = &wait_stage;
            info.commandBufferCount = 1;
            info.pCommandBuffers = &fd->CommandBuffer;
            info.signalSemaphoreCount = 1;
            info.pSignalSemaphores = &fd->RenderCompleteSemaphore;

            vkEndCommandBuffer(fd->CommandBuffer);
            vkQueueSubmit(_queue, 1, &info, fd->Fence);
        }

        // Present
        VkPresentInfoKHR info = {};
        info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        info.waitSemaphoreCount = 1;
        info.pWaitSemaphores = &fd->RenderCompleteSemaphore;
        info.swapchainCount = 1;
        info.pSwapchains = &_windowData.Swapchain;
        info.pImageIndices = &_windowData.FrameIndex;
        VkResult err = vkQueuePresentKHR(_queue, &info);
    }

#ifdef IMGUI_VULKAN_DEBU_REPORT
    static VKAPI_ATTR VkBool32 VKAPI_CALL debu_report(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage, void* pUserData)
    {
        std::cerr << "[GUI Error - Vulkan] ObjectType: " << objectType << " Message: " << pMessage << std::endl;
        return VK_FALSE;
    }
#endif // IMGUI_VULKAN_DEBU_REPORT


    void GUI::CheckVulkanResult(VkResult result)
    {
        if (result != VK_SUCCESS)
            wowgm::exceptions::throw_with_trace(std::runtime_error("Invalid result for VK"));
    }

    void GUI::ResizeFrameBuffer(int width, int height)
    {
        _resizeWanted = true;
        _resizeWidth = width;
        _resizeHeight = height;
    }
}