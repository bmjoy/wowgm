#include "ClientServices.hpp"
#include "RealmList.hpp"
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
#include "AuthResult.hpp"


#include <stdexcept>
#include <chrono>

#define IMGUI_UNLIMITED_FRAME_RATE

#undef CreateSemaphore

namespace wowgm::graphics
{
    // TODO: Clean this mess

    Interface::Interface(std::unique_ptr<Instance>& instance, Window* window) : _window(window)
    {
        memset(_username, 0, 16);
        memset(_password, 0, 16);

        memset(_realmAddress, 0, 100);
        strcpy(_realmAddress, "127.0.0.1");

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

            ImGui_ImplVulkanH_CreateWindowDataCommandBuffers(*_device, _swapChain->GetPhysicalDevice()->GetQueues().Graphics, &_interfaceWindowData, nullptr);
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

            { // Style
                ImGuiStyle * style = &ImGui::GetStyle();

                style->WindowPadding = ImVec2(15, 15);
                style->WindowRounding = 5.0f;
                style->FramePadding = ImVec2(5, 5);
                style->FrameRounding = 4.0f;
                style->ItemSpacing = ImVec2(12, 8);
                style->ItemInnerSpacing = ImVec2(8, 6);
                style->IndentSpacing = 25.0f;
                style->ScrollbarSize = 15.0f;
                style->ScrollbarRounding = 9.0f;
                style->GrabMinSize = 5.0f;
                style->GrabRounding = 3.0f;

                style->Colors[ImGuiCol_Text] = ImVec4(0.80f, 0.80f, 0.83f, 1.00f);
                style->Colors[ImGuiCol_TextDisabled] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
                style->Colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
                style->Colors[ImGuiCol_ChildWindowBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
                style->Colors[ImGuiCol_PopupBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
                style->Colors[ImGuiCol_Border] = ImVec4(0.80f, 0.80f, 0.83f, 0.88f);
                style->Colors[ImGuiCol_BorderShadow] = ImVec4(0.92f, 0.91f, 0.88f, 0.00f);
                style->Colors[ImGuiCol_FrameBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
                style->Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
                style->Colors[ImGuiCol_FrameBgActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
                style->Colors[ImGuiCol_TitleBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
                style->Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.00f, 0.98f, 0.95f, 0.75f);
                style->Colors[ImGuiCol_TitleBgActive] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
                style->Colors[ImGuiCol_MenuBarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
                style->Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
                style->Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
                style->Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
                style->Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
                // style->Colors[ImGuiCol_ComboBg] = ImVec4(0.19f, 0.18f, 0.21f, 1.00f); // Merged with PopupBg
                style->Colors[ImGuiCol_CheckMark] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
                style->Colors[ImGuiCol_SliderGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
                style->Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
                style->Colors[ImGuiCol_Button] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
                style->Colors[ImGuiCol_ButtonHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
                style->Colors[ImGuiCol_ButtonActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
                style->Colors[ImGuiCol_Header] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
                style->Colors[ImGuiCol_HeaderHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
                style->Colors[ImGuiCol_HeaderActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
                style->Colors[ImGuiCol_Column] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
                style->Colors[ImGuiCol_ColumnHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
                style->Colors[ImGuiCol_ColumnActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
                style->Colors[ImGuiCol_ResizeGrip] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
                style->Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
                style->Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
                // style->Colors[ImGuiCol_CloseButton] = ImVec4(0.40f, 0.39f, 0.38f, 0.16f); // Now uses regular button colors
                // style->Colors[ImGuiCol_CloseButtonHovered] = ImVec4(0.40f, 0.39f, 0.38f, 0.39f);
                // style->Colors[ImGuiCol_CloseButtonActive] = ImVec4(0.40f, 0.39f, 0.38f, 1.00f);
                style->Colors[ImGuiCol_PlotLines] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
                style->Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
                style->Colors[ImGuiCol_PlotHistogram] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
                style->Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
                style->Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.25f, 1.00f, 0.00f, 0.43f);
                style->Colors[ImGuiCol_ModalWindowDarkening] = ImVec4(1.00f, 0.98f, 0.95f, 0.73f);

                ImGuiIO& io = ImGui::GetIO();
                io.Fonts->AddFontFromFileTTF("./fonts/Ruda-Bold.ttf", 12);
                io.Fonts->AddFontFromFileTTF("./fonts/Ruda-Bold.ttf", 10);
                io.Fonts->AddFontFromFileTTF("./fonts/Ruda-Bold.ttf", 14);
                io.Fonts->AddFontFromFileTTF("./fonts/Ruda-Bold.ttf", 18);
            }

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
            Shader* vertexShader = Shader::CreateVertexShader(_device, "main", "./shaders/geometry/vert.spv");
            Shader* fragmentShader = Shader::CreateFragmentShader(_device, "main", "./shaders/geometry/frag.spv");

            _geometryRenderStage.Pipeline->GetDepthStencilStateInfo().depthTestEnable = false;
            _geometryRenderStage.Pipeline->GetDepthStencilStateInfo().stencilTestEnable = false;
            _geometryRenderStage.Pipeline->GetRasterizationStateInfo().polygonMode = VK_POLYGON_MODE_FILL;
            _geometryRenderStage.Pipeline->SetPrimitiveType(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
            _geometryRenderStage.Pipeline->GetRasterizationStateInfo().cullMode = VK_CULL_MODE_NONE;
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

        // This semaphore signals when the geometry has been rendered.
        Semaphore* geometryRenderedSemaphore = _device->GetSignalSemaphore();

        // Acquires the next image, waiting on the provided fence and immediately resetting it for reuse.
        _device->AddWaitFence(_device->GetFlightFence()); // Wait on geometry
        _device->AddWaitFence(_interfaceWindowData.Frames[_interfaceWindowData.FrameIndex].Fence); // Wait on GUI
        std::uint32_t image = _device->AcquireNextImage(_swapChain);

        Semaphore* interfaceRenderedSemaphore = _interfaceSemaphores[image];

        PrepareGUI();
        _device->Submit(image, _device->GetFlightFence());

        RenderImGui(*geometryRenderedSemaphore, *interfaceRenderedSemaphore);

        _device->Present(image, _swapChain, interfaceRenderedSemaphore);

        _frameDrawTime = float(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - drawStart).count()) / 1000.0f;
    }

    void Interface::PrepareGUI()
    {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGuiIO& io = ImGui::GetIO();
        ImGui::PushFont(io.Fonts->Fonts[2]);

        if (ImGui::BeginMainMenuBar())
        {
            std::stringstream ss;
            ss << "Frame draw time: " << std::fixed << std::setprecision(3) << _frameDrawTime << " FPS: " << (1000.0f / _frameDrawTime);
            ImGui::Text(ss.str().c_str());
            ImGui::EndMainMenuBar();
        }

        if (sClientServices->GetAvailableRealmCount() == 0)
        {
            ImVec2 size = { 300.0f, 165.0f };

            if (sClientServices->GetAuthentificationResult() != wowgm::protocol::authentification::LOGIN_OK)
                size.y += 30.0f;

            ImGui::SetNextWindowSizeConstraints(size, size);
            ImGui::SetNextWindowPos({ 5.0f, 28.0f });

            ImGui::Begin("Connection");
            ImGui::InputText("Realm", _realmAddress, 100);
            ImGui::InputText("Username", _username, 16);
            ImGui::InputText("Password", _password, 16, ImGuiInputTextFlags_Password);
            if (ImGui::Button("Log in"))
                sClientServices->AsyncConnect(_username, _password);

            if (sClientServices->GetAuthentificationResult() != wowgm::protocol::authentification::LOGIN_OK)
            {
                switch (sClientServices->GetAuthentificationResult())
                {
                    case wowgm::protocol::authentification::LOGIN_BANNED:
                        ImGui::TextColored({ 1.0f, 0.0f, 0.0f, 1.0f }, "This account has been banned.");
                        break;
                    case wowgm::protocol::authentification::LOGIN_UNKNOWN_ACCOUNT:
                        ImGui::TextColored({ 1.0f, 0.0f, 0.0f, 1.0f }, "This account does not exist.");
                        break;
                    case wowgm::protocol::authentification::LOGIN_ALREADY_ONLINE:
                        ImGui::TextColored({ 1.0f, 0.0f, 0.0f, 1.0f }, "This account is already online.");
                        break;
                    case wowgm::protocol::authentification::LOGIN_NO_TIME: // CN
                        ImGui::TextColored({ 1.0f, 0.0f, 0.0f, 1.0f }, "This account has exceeded its online play time.");
                        break;
                    case wowgm::protocol::authentification::LOGIN_BUSY:
                        ImGui::TextColored({ 1.0f, 0.0f, 0.0f, 1.0f }, "Login server is busy.");
                        break;
                    case wowgm::protocol::authentification::LOGIN_BAD_VERSION:
                        ImGui::TextColored({ 1.0f, 0.0f, 0.0f, 1.0f }, "The server is not accepting connection from your version.");
                        break;
                    case wowgm::protocol::authentification::LOGIN_DOWNLOAD_FILE:
                        ImGui::TextColored({ 1.0f, 0.0f, 0.0f, 1.0f }, "Unsupported action requested by the server.");
                        break;
                    case wowgm::protocol::authentification::LOGIN_SUSPENDED:
                        ImGui::TextColored({ 1.0f, 0.0f, 0.0f, 1.0f }, "The login operation was suspended.");
                        break;
                    case wowgm::protocol::authentification::LOGIN_PARENTAL_CONTROL:
                        ImGui::TextColored({ 1.0f, 0.0f, 0.0f, 1.0f }, "Your account is locked due to parental control.");
                        break;
                    case wowgm::protocol::authentification::LOGIN_LOCKED_ENFORCED:
                        ImGui::TextColored({ 1.0f, 0.0f, 0.0f, 1.0f }, "This account is locked.");
                        break;
                    case wowgm::protocol::authentification::LOGIN_CONVERSION_REQUIRED:
                        ImGui::TextColored({ 1.0f, 0.0f, 0.0f, 1.0f }, "This account needs to be converted to Battle.NET.");
                        break;
                    case wowgm::protocol::authentification::LOGIN_DISCONNECTED:
                        ImGui::TextColored({ 1.0f, 0.0f, 0.0f, 1.0f }, "You've been disconnected.");
                        break;
                    case wowgm::protocol::authentification::LOGIN_INVALID_SRP6:
                    default:
                        ImGui::TextColored({ 1.0f, 0.0f, 0.0f, 1.0f }, "You've been disconnected.");
                        break;
                }
            }

            ImGui::End();
        }

        if (sClientServices->GetAvailableRealmCount() > 1)
        {
            ImVec2 size = { 180.0f, 60.0f * sClientServices->GetAvailableRealmCount() + 25.0f };
            ImGui::SetNextWindowSizeConstraints(size, size);
            ImGui::SetNextWindowPos({ 5.0f, 28.0f });

            ImGui::Begin("Realm selection");

            using namespace wowgm::protocol::authentification;

            for (std::uint32_t i = 0; i < sClientServices->GetAvailableRealmCount(); ++i)
            {
                AuthRealmInfo* realmInfo = sClientServices->GetRealmInfo(i);

                ImGui::PushFont(io.Fonts->Fonts[3]);
                ImGui::TextColored({ 1.0f,1.0f,0.0f,1.0f }, realmInfo->Name.c_str());
                ImGui::PopFont();
                ImGui::SameLine();

                std::stringstream ss;
                ss << realmInfo->GetEndpoint();

                if (ImGui::Button(ss.str().c_str()))
                    sClientServices->ConnectToRealm(*realmInfo);
            }

            ImGui::End();
        }

        ImGui::PopFont();
        ImGui::Render();
    }

    void Interface::RenderImGui(VkSemaphore canStartRenderingSemaphore, VkSemaphore signalSemaphore)
    {
        VkResult err;

        ImGui_ImplVulkanH_FrameData* fd = &_interfaceWindowData.Frames[_interfaceWindowData.FrameIndex];

        err = vkResetCommandPool(*_device, fd->CommandPool, 0);
        VkCommandBufferBeginInfo commandBufferBeginInfo = {};
        commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        commandBufferBeginInfo.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        err = vkBeginCommandBuffer(fd->CommandBuffer, &commandBufferBeginInfo);

        VkRenderPassBeginInfo renderPassBeginInfo = {};
        renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassBeginInfo.renderPass = _interfaceWindowData.RenderPass;
        renderPassBeginInfo.framebuffer = _interfaceWindowData.Framebuffer[_interfaceWindowData.FrameIndex];
        renderPassBeginInfo.renderArea.extent.width = _interfaceWindowData.Width;
        renderPassBeginInfo.renderArea.extent.height = _interfaceWindowData.Height;
        renderPassBeginInfo.clearValueCount = 1;
        renderPassBeginInfo.pClearValues = &_interfaceWindowData.ClearValue;
        vkCmdBeginRenderPass(fd->CommandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        // Record Imgui Draw Data and draw funcs into command buffer
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), fd->CommandBuffer);

        // Submit command buffer
        vkCmdEndRenderPass(fd->CommandBuffer);

        VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = &canStartRenderingSemaphore;
        submitInfo.pWaitDstStageMask = &wait_stage;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &fd->CommandBuffer;
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = &signalSemaphore;

        err = vkEndCommandBuffer(fd->CommandBuffer);
        err = vkQueueSubmit(*_device->GetPresentQueue(), 1, &submitInfo, fd->Fence);

        _interfaceWindowData.FrameIndex = (_interfaceWindowData.FrameIndex + 1) % IMGUI_VK_QUEUED_FRAMES;
    }
}