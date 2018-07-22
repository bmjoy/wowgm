#include "GUI.hpp"
#include "Window.hpp"
#include "Assert.hpp"
#include "Surface.hpp"
#include "Instance.hpp"
#include "PhysicalDevice.hpp"
#include "LogicalDevice.hpp"

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

#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

namespace wowgm::graphics
{
    GUI::GUI(Window* window, Instance* instance, Surface* surface) : _window(window), _instance(instance), _surface(surface)
    {
        if (!glfwVulkanSupported())
            wowgm::exceptions::throw_with_trace(std::runtime_error("Vulkan is not supported on your system!"));

        _windowData.Surface = *surface;
        _physicalDevice = *instance->GetPhysicalDevice();
        _queueFamily = instance->GetPhysicalDevice()->GetQueues().Graphics;
        _device = *instance->CreateLogicalDevice();

        { // Create descriptor pool
            VkDescriptorPoolSize pool_sizes[] =
            {
                { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
                { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
                { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
                { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
                { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
                { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
                { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
                { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
                { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
                { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
                { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
            };
            VkDescriptorPoolCreateInfo pool_info = {};
            pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
            pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
            pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
            pool_info.pPoolSizes = pool_sizes;
            VkResult err = vkCreateDescriptorPool(_device, &pool_info, _allocator, &_descriptorPool);
            if (err != VK_SUCCESS)
                wowgm::exceptions::throw_with_trace(std::runtime_error("Unable to allocate descriptor pools"));
        }

        glfwSetFramebufferSizeCallback(window->GetHandle(), [this](GLFWwindow* window, int width, int height) -> void {
            this->ResizeFrameBuffer(width, height);
        });

        VkBool32 physicalSurfaceSupport;
        vkGetPhysicalDeviceSurfaceSupportKHR(_physicalDevice, _queueFamily, *surface, &physicalSurfaceSupport);

        const VkFormat requestSurfaceImageFormat[] = { VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_B8G8R8_UNORM, VK_FORMAT_R8G8B8_UNORM };
        const VkColorSpaceKHR requestSurfaceColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;

        _windowData.SurfaceFormat = ImGui_ImplVulkanH_SelectSurfaceFormat(_physicalDevice, _windowData.Surface, requestSurfaceImageFormat, IM_ARRAYSIZE(requestSurfaceImageFormat), requestSurfaceColorSpace);

        VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;
        _windowData.PresentMode = ImGui_ImplVulkanH_SelectPresentMode(_physicalDevice, _windowData.Surface, &present_mode, 1);

        ImGui_ImplVulkanH_CreateWindowDataCommandBuffers(_physicalDevice, _device, _queueFamily, &_windowData, _allocator);
        ImGui_ImplVulkanH_CreateWindowDataSwapChainAndFramebuffer(_physicalDevice, _device, &_windowData, _allocator, window->GetWidth(), window->GetHeight());

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        ImGui_ImplGlfw_InitForVulkan(window->GetHandle(), true);

        ImGui_ImplVulkan_InitInfo init_info = {};
        init_info.Instance = *_instance;
        init_info.PhysicalDevice = _physicalDevice;
        init_info.Device = _device;
        init_info.QueueFamily = _queueFamily;
        init_info.Queue = _queue;
        init_info.PipelineCache = _pipelineCache;
        init_info.DescriptorPool = _descriptorPool;
        init_info.Allocator = _allocator;
        init_info.CheckVkResultFn = &GUI::CheckVulkanResult;
        ImGui_ImplVulkan_Init(&init_info, _windowData.RenderPass);

        ImGui::StyleColorsDark();

        { // Upload Fonts
            // Use any command queue
            VkCommandPool command_pool = _windowData.Frames[wd->FrameIndex].CommandPool;
            VkCommandBuffer command_buffer = _windowData.Frames[wd->FrameIndex].CommandBuffer;

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

        ImGui_ImplVulkanH_DestroyWindowData(*_instance, _device, &_windowData, _allocator);
        vkDestroyDescriptorPool(_device, _descriptorPool, _allocator);

        vkDestroyDevice(_device, _allocator);
    }

    void GUI::Draw()
    {
        if (_resizeWanted)
        {
            ImGui_ImplVulkanH_CreateWindowDataSwapChainAndFramebuffer(_physicalDevice, _device, &_windowData, _allocator, _resizeWidth, _resizeHeight);
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
            vkCmdBeginRenderPass(&fd->CommandBuffer, &info, VK_SUBPASS_CONTENTS_INLINE);
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
        ImGui_ImplVulkanH_FrameData* fd = &_windowData.Frames[_windowData.FrameIndex];
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