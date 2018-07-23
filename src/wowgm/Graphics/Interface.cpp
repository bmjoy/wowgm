#include "Interface.hpp"
#include "LogicalDevice.hpp"
#include "PhysicalDevice.hpp"
#include "Instance.hpp"
#include "SwapChain.hpp"
#include "CommandPool.hpp"
#include "CommandBuffer.hpp"
#include "Queue.hpp"
#include "Command.hpp"
#include "Pipeline.hpp"
#include "RenderPass.hpp"
#include "Shader.hpp"
#include "FrameBuffer.hpp"
#include "Assert.hpp"
#include "Image.hpp"
#include "ImageView.hpp"
#include "Surface.hpp"
#include "SynchronizationPrimitive.hpp"
#include "ClientServices.hpp"

#include <stdexcept>
#include <chrono>

#define IMGUI_UNLIMITED_FRAME_RATE

#undef CreateSemaphore

namespace wowgm::graphics
{
    // TODO: Clean this mess

    Interface::Interface(std::unique_ptr<Instance>& instance, Window* window) : _window(window)
    {
        _surface = instance->CreateSurface(_window);
        _device = instance->CreateLogicalDevice();
        _swapChain = instance->CreateSwapChain();

        // Create Descriptor Pool
        {
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
            vkCreateDescriptorPool(*_device, &pool_info, nullptr, &g_DescriptorPool);
        }

        { // ImGui
            _interfaceWindowData.Surface = *_surface;
            _interfaceWindowData.ClearEnable = false;
            _interfaceWindowData.Swapchain = *_swapChain;
            _interfaceWindowData.Width = window->GetWidth();
            _interfaceWindowData.Height = window->GetHeight();

            const VkFormat requestSurfaceImageFormat[] = { VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_B8G8R8_UNORM, VK_FORMAT_R8G8B8_UNORM };
            const VkColorSpaceKHR requestSurfaceColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
            _interfaceWindowData.SurfaceFormat = ImGui_ImplVulkanH_SelectSurfaceFormat(*_swapChain->GetPhysicalDevice(), *_surface, requestSurfaceImageFormat, 1, requestSurfaceColorSpace);

            _interfaceWindowData.PresentMode = _swapChain->GetPresentMode();

            ImGui_ImplVulkanH_CreateWindowDataCommandBuffers(*_swapChain->GetPhysicalDevice(), *_device, _swapChain->GetPhysicalDevice()->GetQueues().Graphics, &_interfaceWindowData, nullptr);
            _interfaceWindowData.BackBufferCount = _swapChain->GetImageCount();
            vkGetSwapchainImagesKHR(*_device, *_swapChain, &_interfaceWindowData.BackBufferCount, _interfaceWindowData.BackBuffer);
            ImGui_ImplVulkanH_CreateFrameBuffer(*_swapChain->GetPhysicalDevice(), *_device, &_interfaceWindowData, nullptr);

            IMGUI_CHECKVERSION();
            ImGui::CreateContext();

            ImGui_ImplGlfw_InitForVulkan(window->GetHandle(), true);
            // Setup Vulkan binding
            ImGui_ImplVulkan_InitInfo init_info = {};
            init_info.Instance = *instance;
            init_info.PhysicalDevice = *_swapChain->GetPhysicalDevice();
            init_info.Device = *_device;
            init_info.QueueFamily = _swapChain->GetPhysicalDevice()->GetQueues().Present;
            init_info.Queue = *_device->GetPresentQueue();
            init_info.PipelineCache = g_PipelineCache;
            init_info.DescriptorPool = g_DescriptorPool;
            init_info.Allocator = nullptr;
            init_info.CheckVkResultFn = nullptr;
            ImGui_ImplVulkan_Init(&init_info, _interfaceWindowData.RenderPass);
            ImGui::StyleColorsDark();

            { // Upload font
                // Use any command queue
                VkCommandPool command_pool = _interfaceWindowData.Frames[_interfaceWindowData.FrameIndex].CommandPool;
                VkCommandBuffer command_buffer = _interfaceWindowData.Frames[_interfaceWindowData.FrameIndex].CommandBuffer;

                vkResetCommandPool(*_device, command_pool, 0);
                VkCommandBufferBeginInfo begin_info = {};
                begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
                begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
                vkBeginCommandBuffer(command_buffer, &begin_info);

                ImGui_ImplVulkan_CreateFontsTexture(command_buffer);

                VkSubmitInfo end_info = {};
                end_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
                end_info.commandBufferCount = 1;
                end_info.pCommandBuffers = &command_buffer;
                vkEndCommandBuffer(command_buffer);
                vkQueueSubmit(init_info.Queue, 1, &end_info, VK_NULL_HANDLE);

                vkDeviceWaitIdle(init_info.Device);
                ImGui_ImplVulkan_InvalidateFontUploadObjects();
            }
        }

        CreatePipeline();
    }

    Interface::~Interface()
    {
        _device->WaitIdle();

        delete _geometryRenderStage.Pipeline;
        delete _swapChain;

    }

    void Interface::CreatePipeline()
    {
        _geometryRenderStage.RenderPass = _device->CreateRenderPass();

        VkAttachmentDescription geometryColorAttachment = { };
        geometryColorAttachment.format = _swapChain->GetSurfaceFormat().format;
        geometryColorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        geometryColorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        geometryColorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        geometryColorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        geometryColorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        geometryColorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        geometryColorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        _geometryRenderStage.RenderPass->AddAttachment(geometryColorAttachment);
        _geometryRenderStage.RenderPass->Finalize();

        _geometryRenderStage.Pipeline = new Pipeline(_swapChain, _geometryRenderStage.RenderPass);

        { // Geometry shaders
            Shader* vertexShader = Shader::CreateVertexShader(_device, "main", "C:\\Users\\Vincent Piquet\\source\\repos\\WowGM\\src\\wowgm\\Shaders\\geometry\\vert.spv");
            Shader* fragmentShader = Shader::CreateFragmentShader(_device, "main", "C:\\Users\\Vincent Piquet\\source\\repos\\WowGM\\src\\wowgm\\Shaders\\geometry\\frag.spv");

            _geometryRenderStage.Pipeline->SetDepthTest(false); // Fixme
            _geometryRenderStage.Pipeline->SetStencilTest(false); // Fixme
            _geometryRenderStage.Pipeline->SetWireframe(false);
            _geometryRenderStage.Pipeline->SetPrimitiveType(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
            _geometryRenderStage.Pipeline->SetCulling(VK_CULL_MODE_NONE);
            _geometryRenderStage.Pipeline->AddShader(vertexShader);
            _geometryRenderStage.Pipeline->AddShader(fragmentShader);
            _geometryRenderStage.Pipeline->Finalize();
        }

        { // Create rendering data for the 3D geometry
            _geometryRenderStage.FrameBuffers.resize(_swapChain->GetImageCount());
            _geometryRenderStage.CommandBuffers.resize(_swapChain->GetImageCount());

            _interfaceSemaphores.resize(_swapChain->GetImageCount());

            for (std::uint32_t frameIndex = 0; frameIndex < _swapChain->GetImageCount(); ++frameIndex)
            {
                _geometryRenderStage.FrameBuffers[frameIndex] = _geometryRenderStage.RenderPass->CreateFrameBuffer(_swapChain);
                _geometryRenderStage.FrameBuffers[frameIndex]->AttachImageView(_swapChain->GetImageView(frameIndex));
                _geometryRenderStage.FrameBuffers[frameIndex]->Finalize();

                PrepareCommandBuffers(frameIndex);

                _interfaceSemaphores[frameIndex] = _device->CreateSemaphore();
            }
        }
    }

    void Interface::PrepareCommandBuffers(std::uint32_t frameIndex)
    {
        VkClearValue clearValue = { 0.0f, 0.0f, 0.0f, 1.0f };

        _geometryRenderStage.CommandBuffers[frameIndex] = _device->GetGraphicsQueue()->GetCommandPool()->AllocatePrimaryBuffer();
        _geometryRenderStage.CommandBuffers[frameIndex]->BeginRecording(VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);
        _geometryRenderStage.CommandBuffers[frameIndex]->Record<BeginRenderPass>(_geometryRenderStage.FrameBuffers[frameIndex], _swapChain->GetExtent(), clearValue);
        _geometryRenderStage.CommandBuffers[frameIndex]->Record<BindPipeline>(VK_PIPELINE_BIND_POINT_GRAPHICS, _geometryRenderStage.Pipeline);
        _geometryRenderStage.CommandBuffers[frameIndex]->Draw(3);
        _geometryRenderStage.CommandBuffers[frameIndex]->Record<EndRenderPass>();
        _geometryRenderStage.CommandBuffers[frameIndex]->FinishRecording();

        _device->AddCommandBuffer(frameIndex, _geometryRenderStage.CommandBuffers[frameIndex]);
    }

    void Interface::Draw()
    {
        auto drawStart = std::chrono::high_resolution_clock::now();

        // This semaphore signals when an image is acquired
        Semaphore* imageAvailableSemaphore = _device->GetImageAvailableSemaphore();
        // This semaphore signals when the geometry has been rendered.
        Semaphore* geometryRenderedSemaphore = _device->GetSignalSemaphore();

        // Acquires the next image, waiting on the provided fence and immediately resetting it for reuse.
        _device->AddWaitFence(_device->GetFlightFence()); // Wait on geometry
        _device->AddWaitFence(_interfaceWindowData.Frames[_interfaceWindowData.FrameIndex].Fence); // Wait on GUI
        std::uint32_t image = _device->AcquireNextImage(_swapChain);

        Semaphore* interfaceRenderedSemaphore = _interfaceSemaphores[image];

        PrepareGUI();
        _device->Submit(image, _swapChain, _device->GetFlightFence());

        RenderImGui(*geometryRenderedSemaphore, *interfaceRenderedSemaphore);

        _device->Present(image, _swapChain, interfaceRenderedSemaphore);

        _frameDrawTime = float(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - drawStart).count()) / 1000.0f;
    }

    void Interface::PrepareGUI()
    {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (ImGui::BeginMainMenuBar())
        {
            std::stringstream ss;
            ss << "Frame draw time: " << std::fixed << std::setprecision(3) << _frameDrawTime << " FPS: " << (1000.0f / _frameDrawTime);
            ImGui::MenuItem(ss.str().c_str());
            ImGui::EndMainMenuBar();
        }

        if (!_isLoggedIn)
        {
            ImGui::Begin("Connect");
            ImGui::InputText("Username", _username, 16);
            ImGui::InputText("Password", _password, 16, ImGuiInputTextFlags_Password);
            if (ImGui::Button("Log in"))
            {
                sClientServices->Connect(_username, _password);
            }

            ImGui::End();
        }

        ImGui::Render();
    }

    void Interface::RenderImGui(VkSemaphore canStartRenderingSemaphore, VkSemaphore signalSemaphore)
    {
        VkResult err;

        ImGui_ImplVulkanH_FrameData* fd = &_interfaceWindowData.Frames[_interfaceWindowData.FrameIndex];

        {
            // err = vkResetCommandPool(*_device, fd->CommandPool, 0);
            VkCommandBufferBeginInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
            err = vkBeginCommandBuffer(fd->CommandBuffer, &info);
        }

        {
            VkRenderPassBeginInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            info.renderPass = _interfaceWindowData.RenderPass;
            info.framebuffer = _interfaceWindowData.Framebuffer[_interfaceWindowData.FrameIndex];
            info.renderArea.extent.width = _interfaceWindowData.Width;
            info.renderArea.extent.height = _interfaceWindowData.Height;
            info.clearValueCount = 1;
            info.pClearValues = &_interfaceWindowData.ClearValue;
            vkCmdBeginRenderPass(fd->CommandBuffer, &info, VK_SUBPASS_CONTENTS_INLINE);
        }

        // Record Imgui Draw Data and draw funcs into command buffer
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), fd->CommandBuffer);

        // Submit command buffer
        vkCmdEndRenderPass(fd->CommandBuffer);
        {
            VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            VkSubmitInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            info.waitSemaphoreCount = 1;
            info.pWaitSemaphores = &canStartRenderingSemaphore;
            info.pWaitDstStageMask = &wait_stage;
            info.commandBufferCount = 1;
            info.pCommandBuffers = &fd->CommandBuffer;
            info.signalSemaphoreCount = 1;
            info.pSignalSemaphores = &signalSemaphore;

            err = vkEndCommandBuffer(fd->CommandBuffer);
            err = vkQueueSubmit(*_device->GetPresentQueue(), 1, &info, fd->Fence);
        }

        _interfaceWindowData.FrameIndex = (_interfaceWindowData.FrameIndex + 1) % IMGUI_VK_QUEUED_FRAMES;
    }
}