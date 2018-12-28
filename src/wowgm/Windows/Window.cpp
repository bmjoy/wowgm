#include "Window.hpp"

#include <graphics/vulkan/Instance.hpp>
#include <graphics/vulkan/PhysicalDevice.hpp>
#include <graphics/vulkan/Swapchain.hpp>
#include <graphics/vulkan/Pipeline.hpp>
#include <graphics/vulkan/RenderPass.hpp>
#include <graphics/vulkan/Shader.hpp>
#include <graphics/vulkan/CommandBuffer.hpp>
#include <graphics/vulkan/CommandPool.hpp>

#include <graphics/vulkan/Helpers.hpp>

#include <shared/assert/assert.hpp>
#include <shared/log/log.hpp>

#include <sstream>

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
            char tmp_message[500];

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

        VkResult result = Instance::Create(&instanceCreateInfo, &_instance);
        BOOST_ASSERT_MSG(result == VK_SUCCESS, "Unable to create an instance of Vulkan!");

        // Select a physical device.
        PhysicalDevice* physicalDevice = nullptr;
        for (PhysicalDevice* itr : _instance->GetPhysicalDevices())
        {
            if (itr->GetPhysicalDeviceProperties().deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
                physicalDevice = itr;

            if (physicalDevice != nullptr)
                break;
        }

        if (physicalDevice == nullptr)
            physicalDevice = *_instance->GetPhysicalDevices().begin();

        // We now create a logical device
        DeviceCreateInfo deviceCreateInfo;
        deviceCreateInfo.physicalDevice = physicalDevice;
        deviceCreateInfo.enabledExtensionNames.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
        result = Device::Create(_instance, &deviceCreateInfo, &_device);
        if (result != VK_SUCCESS)
        {
            delete _instance;
            shared::assert::throw_with_trace("Unable to create a logical device!");
        }

        _surface = createSurface(_instance, nullptr);

        // And now a swapchain
        SwapchainCreateInfo swapchainCreateInfo;
        swapchainCreateInfo.surface = _surface;
        swapchainCreateInfo.tripleBuffer = true;
        swapchainCreateInfo.preferredFormat.format = VK_FORMAT_B8G8R8A8_UNORM;
        swapchainCreateInfo.preferredFormat.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

        result = Swapchain::Create(_device, &swapchainCreateInfo, &_swapchain);
        BOOST_ASSERT_MSG(result == VK_SUCCESS, "Unable to create a swapchain");

        _interfaceRenderer = new InterfaceRenderer(_swapchain);
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

        // pipeline, pipelineLayout, renderPass
        delete _interfaceRenderer;

        // swapchain imageViews, swapchain
        delete _swapchain;

        // device, descriptor set layout cache, pipeline cache
        delete _device;

        vkDestroySurfaceKHR(_instance->GetHandle(), _surface, nullptr);

        delete _instance;

#if _DEBUG && PLATFORM == PLATFORM_WINDOWS
        DebugBreak();
#endif
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