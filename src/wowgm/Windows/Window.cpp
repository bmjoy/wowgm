#include "Window.hpp"

#include <graphics/vulkan/Instance.hpp>
#include <graphics/vulkan/PhysicalDevice.hpp>
#include <graphics/vulkan/Swapchain.hpp>
#include <graphics/vulkan/Pipeline.hpp>
#include <graphics/vulkan/RenderPass.hpp>
#include <graphics/vulkan/Shader.hpp>
#include <graphics/vulkan/CommandBuffer.hpp>
#include <graphics/vulkan/Framebuffer.hpp>
#include <graphics/vulkan/CommandPool.hpp>
#include <graphics/vulkan/Device.hpp>
#include <graphics/vulkan/Queue.hpp>
#include <graphics/vulkan/ResourceTracker.hpp>

#include <graphics/vulkan/Helpers.hpp>

#include <shared/assert/assert.hpp>
#include <shared/log/log.hpp>

#include <sstream>

#include "InterfaceRenderer.hpp"
#include "MetricsRenderer.hpp"

// Fucking hell, windows.
#if PLATFORM == PLATFORM_WINDOWS
#undef CreateSemaphore
#undef max
#undef min
#endif

namespace wowgm
{
    VkBool32 VKAPI_PTR debugUtilsCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT           messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT                  messageType,
        const VkDebugUtilsMessengerCallbackDataEXT*      pCallbackData,
        void*                                            pUserData)
    {
        // I am horribly lazy so this comes straight from LunarG's 23 pages PDF explaining how to move to
        // VK_EXT_debug_utils.

        if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
            return VK_FALSE;

        std::stringstream str;

        if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
            str << "INFO : ";
        else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
            str << "ERROR : ";
        else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
            str << "WARNING : ";

        if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT)
            str << "GENERAL";
        else if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT)
            str << "VALIDATION";
        else if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT)
            str << "PERFORMANCE";

        str << " - Message ({}) {}";

        LOG_GRAPHICS(str.str().c_str(), pCallbackData->messageIdNumber, pCallbackData->pMessageIdName == nullptr ? "" : pCallbackData->pMessageIdName);
        LOG_GRAPHICS("{}", pCallbackData->pMessage);

        if (pCallbackData->objectCount > 0) {
            LOG_GRAPHICS("\tObjects - {}", pCallbackData->objectCount);

            for (uint32_t object = 0; object < pCallbackData->objectCount; ++object) {
                if (nullptr != pCallbackData->pObjects[object].pObjectName
                    && strlen(pCallbackData->pObjects[object].pObjectName) > 0)
                {
                    LOG_GRAPHICS("\t\tObject[{}] - {}, Handle {}, Name \"{}\"",
                        object,
                        gfx::vk::ObjectTypeToString(pCallbackData->pObjects[object].objectType),
                        (void*)(pCallbackData->pObjects[object].objectHandle),
                        pCallbackData->pObjects[object].pObjectName);
                }
                else {
                    LOG_GRAPHICS("\t\tObject[{}] - {}, Handle {}",
                        object,
                        gfx::vk::ObjectTypeToString(pCallbackData->pObjects[object].objectType),
                        (void*)(pCallbackData->pObjects[object].objectHandle));
                }
            }
        }

        if (pCallbackData->cmdBufLabelCount > 0) {

            LOG_GRAPHICS("\tCommand Buffer Labels - {}", pCallbackData->cmdBufLabelCount);

            for (uint32_t cmd_buf_label = 0; cmd_buf_label < pCallbackData->cmdBufLabelCount; ++cmd_buf_label) {

                LOG_GRAPHICS("\t\tLabel[{}] - {} [ {}, {}, {}, {} ]\n", cmd_buf_label,
                    pCallbackData->pCmdBufLabels[cmd_buf_label].pLabelName, pCallbackData->pCmdBufLabels[cmd_buf_label].color[0],
                    pCallbackData->pCmdBufLabels[cmd_buf_label].color[1], pCallbackData->pCmdBufLabels[cmd_buf_label].color[2],
                    pCallbackData->pCmdBufLabels[cmd_buf_label].color[3]);
            }
        }

        return VK_FALSE;
    }

    VkBool32 VKAPI_PTR debugReportCallback(
        VkDebugReportFlagsEXT                       flags,
        VkDebugReportObjectTypeEXT                  objectType,
        uint64_t                                    object,
        size_t                                      location,
        int32_t                                     messageCode,
        const char*                                 pLayerPrefix,
        const char*                                 pMessage,
        void*                                       pUserData)
    {
        Window* window = reinterpret_cast<Window*>(pUserData);

        return VK_FALSE;
    }


    Window::Window(int32_t width, int32_t height, std::string const& title)
        : gfx::Window(width, height, title)
    {
        using namespace gfx::vk;

        ApplicationInfo appInfo;
        appInfo.pApplicationName = "WowGM";
        appInfo.pEngineName = "WowGM";
        appInfo.apiVersion = VK_API_VERSION_1_1;
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);

        InstanceCreateInfo instanceCreateInfo;
        instanceCreateInfo.pApplicationInfo = &appInfo;
        instanceCreateInfo.enabledExtensionNames = getRequiredInstanceExtensions();

        // Specifying a debug callback automatically causes Instance::Create to add VK_EXT_debug_utils to the layers.
        instanceCreateInfo.debugUtils.messengerCallback = &debugUtilsCallback;
        instanceCreateInfo.debugUtils.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        instanceCreateInfo.debugUtils.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        instanceCreateInfo.debugUtils.pUserData = this;

        // Also try to set up for the older debug_report extension
        instanceCreateInfo.debugReport.callback = &debugReportCallback;
        instanceCreateInfo.debugReport.pUserData = this;

        _instance = new Instance(&instanceCreateInfo);

        // Select a physical device.
        PhysicalDevice* physicalDevice = nullptr;
        for (PhysicalDevice* itr : _instance->GetPhysicalDevices())
        {
            if (itr->GetProperties().deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
                physicalDevice = itr;

            if (physicalDevice != nullptr)
                break;
        }

        if (physicalDevice == nullptr)
            physicalDevice = _instance->GetPhysicalDevices().front();

        // We now create a logical device
        DeviceCreateInfo deviceCreateInfo;
        deviceCreateInfo.physicalDevice = physicalDevice;
        deviceCreateInfo.enabledExtensionNames.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
        _device = new Device(_instance, &deviceCreateInfo);

        _surface = createSurface(_instance, nullptr);

        // And now a swapchain
        SwapchainCreateInfo swapchainCreateInfo;
        swapchainCreateInfo.surface = _surface;
        swapchainCreateInfo.tripleBuffer = true;
        swapchainCreateInfo.preferredFormat.format = VK_FORMAT_B8G8R8A8_UNORM;
        swapchainCreateInfo.preferredFormat.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

        _swapchain = new Swapchain(GetDevice(), &swapchainCreateInfo);

        // Prepare the render pass
        _renderPass = new RenderPass(GetDevice());

        // Register every renderer here
        _renderers.push_back(new MetricsRenderer<InterfaceRenderer>(_swapchain));

        // Give a chance for every renderer to add a subpass
        for (auto&& itr : _renderers)
            itr->initializeRenderPass(_renderPass);

        // Then let them create their pipelines
        for (auto&& itr : _renderers)
            itr->initializePipeline(this);

        // Create the data structures of our swapchain images (pretty much frames)
        _frames.resize(GetSwapchain()->GetImageCount());
        uint32_t frameIndex = 0;
        for (auto& frame : _frames)
        {
            frame.acquireSemaphore = GetDevice()->CreateSemaphore();
            BOOST_ASSERT_MSG(frame.acquireSemaphore != VK_NULL_HANDLE, "Failed to create a semaphore");

            frame.presentSemaphore = GetDevice()->CreateSemaphore();
            BOOST_ASSERT_MSG(frame.presentSemaphore != VK_NULL_HANDLE, "Failed to create a semaphore");

            frame.frameBuffer = GetSwapchain()->CreateFrameBuffer(frameIndex++, _renderPass);
            BOOST_ASSERT_MSG(frame.frameBuffer != nullptr, "Failed to create a framebuffer");

            frame.inflightFence = GetDevice()->CreateFence(VK_FENCE_CREATE_SIGNALED_BIT);
            BOOST_ASSERT_MSG(frame.inflightFence != VK_NULL_HANDLE, "Failed to create a fence");

            Queue* graphicsQueue = GetDevice()->GetQueueByFlags(VK_QUEUE_GRAPHICS_BIT, 0);

            VkResult result = GetDevice()->AllocateCommandBuffers(graphicsQueue, nullptr, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1, &frame.commandBuffer);
            BOOST_ASSERT_MSG(result == VK_SUCCESS, "Failed to allocate a primary command buffer");
            frame.commandBuffer->SetName("Primary command buffer");
        }

        _currentFrame = extstd::containers::make_ring_iterator(_frames.begin(), _frames.end());
    }

    Window::~Window()
    {
        // swapchain framebuffers
        // pipeline
        // pipelineLayout
        // renderpass
        // swapchain image views
        // swapchain
        // device
        // validation layers
        // surface
        // instance

        GetDevice()->WaitIdle();

        for (auto&& itr : _frames)
        {
            GetDevice()->DestroySemaphore(itr.acquireSemaphore);
            GetDevice()->DestroySemaphore(itr.presentSemaphore);

            delete itr.frameBuffer;
            delete itr.commandBuffer;

            GetDevice()->DestroyFence(itr.inflightFence);
        }

        for (auto&& itr : _renderers)
            delete itr;

        _renderers.clear();

        delete _renderPass;

        // swapchain imageViews, swapchain
        delete _swapchain;

        // device, descriptor set layout cache, pipeline cache, staging buffer
        delete _device;

        vkDestroySurfaceKHR(_instance->GetHandle(), _surface, nullptr);

        delete _instance;

#if _DEBUG && PLATFORM == PLATFORM_WINDOWS
        DebugBreak();
#endif
    }

    void Window::onFrame()
    {
        VkResult result;

        gfx::vk::Queue* graphicsQueue = GetDevice()->GetQueueByFlags(VK_QUEUE_GRAPHICS_BIT, 0);

        graphicsQueue->WaitForFences(1, &_currentFrame->inflightFence, std::numeric_limits<uint64_t>::max());
        graphicsQueue->ResetFences(1, &_currentFrame->inflightFence);

        uint32_t imageIndex; // The index of the image that has become available
        result = graphicsQueue->AcquireNextImage(GetSwapchain(), imageIndex, &_currentFrame->acquireSemaphore, nullptr);
        BOOST_ASSERT_MSG(result == VK_SUCCESS, "Failed to acquire the next image");
        BOOST_ASSERT_MSG(_currentFrame->commandBuffer != nullptr, "The command buffer of the current frame must be initialized");

        // Recording commands now!
        result = _currentFrame->commandBuffer->Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, nullptr);
        BOOST_ASSERT_MSG(result == VK_SUCCESS, "Failed to start recording");

        for (auto&& itr : _renderers)
            itr->beforeRenderQuery(_currentFrame->commandBuffer);

        // Begin a render pass
        gfx::vk::RenderPassBeginInfo renderPassBeginInfo{};
        renderPassBeginInfo.pRenderPass = _renderPass;
        renderPassBeginInfo.clearValues.push_back({ 0.0f, 0.0f, 0.0f, 1.0f });
        renderPassBeginInfo.renderArea.extent = GetSwapchain()->GetExtent();
        renderPassBeginInfo.renderArea.offset = { 0, 0 };
        renderPassBeginInfo.pFramebuffer = _currentFrame->frameBuffer;
        _currentFrame->commandBuffer->BeginRenderPass(&renderPassBeginInfo);

        for (auto&& itr : _renderers)
            itr->onRenderQuery(_currentFrame->commandBuffer);

        // Finish this render pass.
        _currentFrame->commandBuffer->EndRenderPass();

        std::vector<std::pair<VkSemaphore, VkPipelineStageFlags>> waitSemaphores;
        for (auto&& itr : _renderers)
            itr->afterRenderQuery(_currentFrame->commandBuffer, waitSemaphores);

        // Done recording
        _currentFrame->commandBuffer->End();

        GetDevice()->GetResourceManager()->Collect(_currentFrame->inflightFence);

        // Submit the command buffer.
        gfx::vk::SubmitInfo submitInfo{};
        submitInfo.commandBuffers.emplace_back(_currentFrame->commandBuffer);
        submitInfo.waitSemaphores.push_back({
            _currentFrame->acquireSemaphore,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
        });
        // Add all the wait semaphores that renderers asked for
        submitInfo.waitSemaphores.insert(submitInfo.waitSemaphores.end(), waitSemaphores.begin(), waitSemaphores.end());

        submitInfo.signalSemaphores.push_back(_currentFrame->presentSemaphore);

        result = graphicsQueue->Submit(1, &submitInfo, &_currentFrame->inflightFence);
        BOOST_ASSERT_MSG(result == VK_SUCCESS, "Failed to submit to the queue");

        // And present the available image.
        gfx::vk::PresentInfo presentInfo;
        presentInfo.swapchains.push_back({
            GetSwapchain(),
            GetSwapchain()->GetImage(imageIndex),
            VK_SUCCESS
        });
        presentInfo.waitSemaphores.push_back(_currentFrame->presentSemaphore);

        result = graphicsQueue->Present(&presentInfo);

        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            // Recreate the swapchain
            gfx::vk::SwapchainCreateInfo swapchainCreateInfo;
            swapchainCreateInfo.surface = _surface;
            swapchainCreateInfo.tripleBuffer = true;
            swapchainCreateInfo.preferredFormat.format = VK_FORMAT_B8G8R8A8_UNORM;
            swapchainCreateInfo.preferredFormat.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

            delete _swapchain;
            _swapchain = new gfx::vk::Swapchain(GetDevice(), &swapchainCreateInfo);
            result = VK_SUCCESS; // Faking the following assert

            uint32_t index = 0;
            for (auto&& itr : _frames)
            {
                delete itr.frameBuffer;

                itr.frameBuffer = GetSwapchain()->CreateFrameBuffer(index++, _renderPass);
            }

            // Update cached swapchains
            for (auto&& itr : _renderers)
                itr->SetSwapchain(_swapchain);
        }

        BOOST_ASSERT_MSG(result == VK_SUCCESS, "Failed to present to screen");

        // Move to next frame
        ++_currentFrame;

        GetDevice()->GetResourceManager()->Clear();
    }

    void Window::onWindowResized(const glm::uvec2& newSize)
    {

    }

    void Window::onWindowClosed()
    {

    }

    void Window::onKeyPressed(int key, int mods)
    {
        LOG_INFO("Key pressed: {}", key);
    }

    void Window::onKeyReleased(int key, int mods)
    {

    }

    void Window::onMousePressed(int button, int mods)
    {

    }

    void Window::onMouseReleased(int button, int mods)
    {

    }

    void Window::onMouseMoved(const glm::vec2& newPos)
    {

    }

    void Window::onMouseScrolled(float delta)
    {

    }
}