#include "GameWindow.hpp"

#include <imgui.h>
#include <numeric>

#include <GLFW/glfw3.h>
#if PLATFORM == PLATFORM_WINDOWS
# define GLFW_INCLUDE_NONE
# define GLFW_EXPOSE_NATIVE_WIN32
# include <GLFW/glfw3native.h>
#endif

#include "Assert.hpp"
#include "FileSystem.hpp"
#include "Logger.hpp"

#include <vector>
#include <algorithm>
#include <iostream>
#include <memory>

#ifdef max
#undef max
#endif

#include "Instance.hpp"
#include "PhysicalDevice.hpp"
#include "Swapchain.hpp"
#include "Shader.hpp"
#include "Pipeline.hpp"
#include "CommandBuffer.hpp"
#include "Image.hpp"
#include "ImageView.hpp"
#include "Queue.hpp"
#include "Buffer.hpp"

namespace debug
{
#if _DEBUG
    const char* ObjectTypeToString(const VkObjectType object_type)
    {
        switch (object_type)
        {
        case VK_OBJECT_TYPE_INSTANCE: return "VkInstance";
        case VK_OBJECT_TYPE_PHYSICAL_DEVICE: return "VkPhysicalDevice";
        case VK_OBJECT_TYPE_DEVICE: return "VkDevice";
        case VK_OBJECT_TYPE_QUEUE: return "VkQueue";
        case VK_OBJECT_TYPE_SEMAPHORE: return "VkSemaphore";
        case VK_OBJECT_TYPE_COMMAND_BUFFER: return "VkCommandBuffer";
        case VK_OBJECT_TYPE_FENCE: return "VkFence";
        case VK_OBJECT_TYPE_DEVICE_MEMORY: return "VkDeviceMemory";
        case VK_OBJECT_TYPE_BUFFER: return "VkBuffer";
        case VK_OBJECT_TYPE_IMAGE: return "VkImage";
        case VK_OBJECT_TYPE_EVENT: return "VkEvent";
        case VK_OBJECT_TYPE_QUERY_POOL: return "VkQueryPool";
        case VK_OBJECT_TYPE_BUFFER_VIEW: return "VkBufferView";
        case VK_OBJECT_TYPE_IMAGE_VIEW: return "VkImageView";
        case VK_OBJECT_TYPE_SHADER_MODULE: return "VkShaderModule";
        case VK_OBJECT_TYPE_PIPELINE_CACHE: return "VkPipelineCache";
        case VK_OBJECT_TYPE_PIPELINE_LAYOUT: return "VkPipelineLayout";
        case VK_OBJECT_TYPE_RENDER_PASS: return "VkRenderPass";
        case VK_OBJECT_TYPE_PIPELINE: return "VkPipeline";
        case VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT: return "VkDescriptorSetLayout";
        case VK_OBJECT_TYPE_SAMPLER: return "VkSampler";
        case VK_OBJECT_TYPE_DESCRIPTOR_POOL: return "VkDescriptorPool";
        case VK_OBJECT_TYPE_DESCRIPTOR_SET: return "VkDescriptorSet";
        case VK_OBJECT_TYPE_FRAMEBUFFER: return "VkFramebuffer";
        case VK_OBJECT_TYPE_COMMAND_POOL: return "VkCommandPool";
        case VK_OBJECT_TYPE_SURFACE_KHR: return "VkSurfaceKHR";
        case VK_OBJECT_TYPE_SWAPCHAIN_KHR: return "VkSwapchainKHR";
        case VK_OBJECT_TYPE_DEBUG_REPORT_CALLBACK_EXT: return "VkDebugReportCallbackEXT";
        case VK_OBJECT_TYPE_DISPLAY_KHR: return "VkDisplayKHR";
        case VK_OBJECT_TYPE_DISPLAY_MODE_KHR: return "VkDisplayModeKHR";
        case VK_OBJECT_TYPE_OBJECT_TABLE_NVX: return "VkObjectTableNVX";
        case VK_OBJECT_TYPE_INDIRECT_COMMANDS_LAYOUT_NVX: return "VkIndirectCommandsLayoutNVX";
        case VK_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE_KHR: return "VkDescriptorUpdateTemplateKHR";
        default:
            return "Unknown Type";
        }
    }
#endif

    static VKAPI_ATTR VkBool32 VKAPI_CALL oldDebugCallback(VkDebugReportFlagsEXT messageSeverity, VkDebugReportObjectTypeEXT objType,
        uint64_t obj, size_t location, int32_t code, const char* layerPrefix, const char* msg, void* userData)
    {
#if _DEBUG
        std::stringstream ss;
        if (messageSeverity & VK_DEBUG_REPORT_INFORMATION_BIT_EXT)
            ss << "INFO : ";
        else if (messageSeverity & VK_DEBUG_REPORT_WARNING_BIT_EXT)
            ss << "WARNING : ";
        else if (messageSeverity & VK_DEBUG_REPORT_ERROR_BIT_EXT)
            ss << "ERROR : ";
        else if (messageSeverity & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
            ss << "PERFORMANCE : ";
        else if (messageSeverity & VK_DEBUG_REPORT_DEBUG_BIT_EXT)
            ss << "DEBUG : ";

        constexpr static const char indent[] = "    ";

        ss << '\n' << indent << msg << '\n';
        std::cerr << ss.str() << std::endl;
#endif

        return VK_FALSE;
    }

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* callbackData, void* userData)
    {
#if _DEBUG
        std::stringstream ss;
        if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
            ss << "VERBOSE : ";
        else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
            ss << "INFO : ";
        else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
            ss << "WARNING : ";
        else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
            ss << "ERROR : ";

        if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT)
            ss << "GENERAL ";
        else
        {
            if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT)
                ss << "SPEC";

            if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT)
            {
                if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT)
                    ss << "|";
                ss << "PERF";
            }
            ss << ' ';
        }

        constexpr static const char indent[] = "    ";

        ss << "Message ID #" << callbackData->messageIdNumber;
        if (callbackData->pMessageIdName != nullptr)
            ss << " (" << callbackData->pMessageIdName << ")";
        ss << '\n' << indent << callbackData->pMessage << '\n';

        if (callbackData->objectCount > 0)
        {
            ss << '\n' << indent << "Objects - " << callbackData->objectCount << " total\n";
            for (uint32_t i = 0; i < callbackData->objectCount; ++i)
            {
                ss << indent << indent << "Object #" << i
                    << ": Type " << ObjectTypeToString(callbackData->pObjects[i].objectType)
                    << ", Value " << std::hex << std::setfill('0') << std::setw(16) << callbackData->pObjects[i].objectHandle;
                if (callbackData->pObjects[i].pObjectName != nullptr)
                    ss << ", Name \"" << callbackData->pObjects[i].pObjectName << "\"";
                ss << '\n';
            }
        }

        if (callbackData->cmdBufLabelCount > 0)
        {
            ss << '\n' << indent << "Command buffer labels - " << callbackData->cmdBufLabelCount << " total\n";
            for (uint32_t i = 0; i < callbackData->cmdBufLabelCount; ++i)
                ss << indent << indent << "Command buffer label #" << i << ": Name \"" << callbackData->pCmdBufLabels[i].pLabelName << "\"\n";
        }

        if (callbackData->queueLabelCount > 0)
        {
            ss << '\n' << indent << "Queue labels - " << callbackData->queueLabelCount << " total\n";
            for (uint32_t i = 0; i < callbackData->queueLabelCount; ++i)
                ss << indent << indent << "Queue label #" << i << ": Name \"" << callbackData->pQueueLabels[i].pLabelName << "\"\n";
        }

        std::cerr << ss.str() << std::endl;
#endif

        return VK_FALSE;
    }
}

GameWindow::GameWindow(std::string_view title, uint32_t width, uint32_t height) :
    _mpqSystemInitialized(false), _frameDrawTimes(1000, 0.0f)
{
    _title = std::move(title);

    std::call_once(glfwInitializer, []() -> void {
        glfwInit();
    });
}

GameWindow::~GameWindow()
{
    // Empty all destructible data buckets
    _ExecuteDelayedDestructionRequests();

    delete _swapchain;
    _swapchain = nullptr;

    // Release interface font texture data
    _device->DestroyImage(_interfaceFontTexture);
    delete _interfaceFontTextureView;
    _device->DestroySampler(_interfaceFontSampler);

    _interfaceFontTexture = nullptr;
    _interfaceFontTextureView = nullptr;
    _interfaceFontSampler = nullptr;

    // Release interface pipeline informations
    if (_interfacePipeline.Handle != nullptr)
    {
        delete _interfacePipeline.Handle;
        _interfacePipeline.Handle = nullptr;

        delete _interfacePipeline.Shaders[0];
        delete _interfacePipeline.Shaders[1];
        _interfacePipeline.Shaders[0] = nullptr;
        _interfacePipeline.Shaders[1] = nullptr;
    }

    // Release command buffers
    _device->FreeCommandBuffers(_commandBuffers.Interface.size(), _commandBuffers.Interface.data());

    // Release geometry pipeline infos
    if (_geometryPipeline.Handle != nullptr)
    {
        delete _geometryPipeline.Handle;
        _geometryPipeline.Handle = nullptr;
    }

    // Destroy framebuffer
    if (_frameBuffer.Handle != nullptr)
    {
        _device->DestroyFramebuffer(_frameBuffer.Handle);
        _frameBuffer.Handle = nullptr;

        delete _frameBuffer.ColorImageView;
        delete _frameBuffer.DepthStencilImageView;

        _device->DestroyImage(_frameBuffer.ColorImage);
        _device->DestroyImage(_frameBuffer.DepthStencilImage);

        _frameBuffer.ColorImageView = nullptr;
        _frameBuffer.DepthStencilImageView = nullptr;

        _frameBuffer.ColorImage = nullptr;
        _frameBuffer.DepthStencilImage = nullptr;
    }

    if (_interfaceVertexBuffer != nullptr)
        _device->DestroyBuffer(_interfaceVertexBuffer);

    if (_interfaceIndexBuffer != nullptr)
        _device->DestroyBuffer(_interfaceIndexBuffer);

    delete _device;
    _device = nullptr;

    vkDestroySurfaceKHR(_instance->GetHandle(), _surface, nullptr);
    _surface = VK_NULL_HANDLE;

    delete _instance;
    _instance = nullptr;

    _physicalDevice = nullptr;

    glfwDestroyWindow(_window);
    glfwTerminate();

    _window = nullptr;

    ImGui::DestroyContext();
}

void GameWindow::InitializeWindow(bool fullScreen)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
    io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;

    io.KeyMap[ImGuiKey_Tab] = GLFW_KEY_TAB;
    io.KeyMap[ImGuiKey_LeftArrow] = GLFW_KEY_LEFT;
    io.KeyMap[ImGuiKey_RightArrow] = GLFW_KEY_RIGHT;
    io.KeyMap[ImGuiKey_UpArrow] = GLFW_KEY_UP;
    io.KeyMap[ImGuiKey_DownArrow] = GLFW_KEY_DOWN;
    io.KeyMap[ImGuiKey_PageUp] = GLFW_KEY_PAGE_UP;
    io.KeyMap[ImGuiKey_PageDown] = GLFW_KEY_PAGE_DOWN;
    io.KeyMap[ImGuiKey_Home] = GLFW_KEY_HOME;
    io.KeyMap[ImGuiKey_End] = GLFW_KEY_END;
    io.KeyMap[ImGuiKey_Insert] = GLFW_KEY_INSERT;
    io.KeyMap[ImGuiKey_Delete] = GLFW_KEY_DELETE;
    io.KeyMap[ImGuiKey_Backspace] = GLFW_KEY_BACKSPACE;
    io.KeyMap[ImGuiKey_Space] = GLFW_KEY_SPACE;
    io.KeyMap[ImGuiKey_Enter] = GLFW_KEY_ENTER;
    io.KeyMap[ImGuiKey_Escape] = GLFW_KEY_ESCAPE;
    io.KeyMap[ImGuiKey_A] = GLFW_KEY_A;
    io.KeyMap[ImGuiKey_C] = GLFW_KEY_C;
    io.KeyMap[ImGuiKey_V] = GLFW_KEY_V;
    io.KeyMap[ImGuiKey_X] = GLFW_KEY_X;
    io.KeyMap[ImGuiKey_Y] = GLFW_KEY_Y;
    io.KeyMap[ImGuiKey_Z] = GLFW_KEY_Z;

    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* videoMode = glfwGetVideoMode(monitor);
    if (fullScreen)
    {
        _width = videoMode->width;
        _height = videoMode->height;
    }
    else
    {
        monitor = nullptr;

        _width = videoMode->width * 0.75f;
        _height = videoMode->height * 0.75f;

        _previousWindowData[2] = _width;
        _previousWindowData[3] = _height;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    _window = glfwCreateWindow(_width, _height, _title.data(), monitor, nullptr);
    if (_window == nullptr)
        wowgm::exceptions::throw_with_trace<std::runtime_error>("Unable to create a window with GLFW.");

#if PLATFORM == PLATFORM_WINDOWS
    io.ImeWindowHandle = static_cast<void*>(glfwGetWin32Window(_window));
#endif

    glfwGetWindowPos(_window, &_previousWindowData[0], &_previousWindowData[1]);
    glfwSetWindowUserPointer(_window, this);

    glfwSetKeyCallback(_window, [](GLFWwindow* window, int key, int scancode, int action, int mods) -> void {
        ImGuiIO& io = ImGui::GetIO();
        if (action == GLFW_PRESS)
            io.KeysDown[key] = true;
        if (action == GLFW_RELEASE)
            io.KeysDown[key] = false;

        io.KeyCtrl = io.KeysDown[GLFW_KEY_LEFT_CONTROL] || io.KeysDown[GLFW_KEY_RIGHT_CONTROL];
        io.KeyShift = io.KeysDown[GLFW_KEY_LEFT_SHIFT] || io.KeysDown[GLFW_KEY_RIGHT_SHIFT];
        io.KeyAlt = io.KeysDown[GLFW_KEY_LEFT_ALT] || io.KeysDown[GLFW_KEY_RIGHT_ALT];
        io.KeySuper = io.KeysDown[GLFW_KEY_LEFT_SUPER] || io.KeysDown[GLFW_KEY_RIGHT_SUPER];

        GameWindow* gameWindow = static_cast<GameWindow*>(glfwGetWindowUserPointer(window));
        if (action == GLFW_PRESS)
            gameWindow->OnKeyDown(key, mods);
        else if (action == GLFW_RELEASE)
            gameWindow->OnKeyUp(key, mods);
    });

    glfwSetMouseButtonCallback(_window, [](GLFWwindow* window, int button, int action, int mods) -> void {
        GameWindow* gameWindow = static_cast<GameWindow*>(glfwGetWindowUserPointer(window));
        double x, y;
        glfwGetCursorPos(window, &x, &y);

        if (action == GLFW_PRESS)
            gameWindow->OnMouseDown(button, static_cast<float>(x), static_cast<float>(y));
        else if (action == GLFW_RELEASE)
            gameWindow->OnMouseUp(button, static_cast<float>(x), static_cast<float>(y));
    });

    glfwSetScrollCallback(_window, [](GLFWwindow* window, double x, double y) -> void {
        GameWindow* gameWindow = static_cast<GameWindow*>(glfwGetWindowUserPointer(window));
        gameWindow->OnMouseScroll(static_cast<float>(x), static_cast<float>(y));
    });

    glfwSetCharCallback(_window, [](GLFWwindow* window, unsigned int character) -> void {
        ImGuiIO& io = ImGui::GetIO();
        if (character > 0 && character < 0x10000)
            io.AddInputCharacter(static_cast<unsigned short>(character));
    });

    glfwSetFramebufferSizeCallback(_window, [](GLFWwindow* window, int width, int height) -> void {
        GameWindow* gameWindow = static_cast<GameWindow*>(glfwGetWindowUserPointer(window));
        gameWindow->OnResize(width, height);
    });

    glfwSetWindowCloseCallback(_window, [](GLFWwindow* window) -> void {
        GameWindow* gameWindow = static_cast<GameWindow*>(glfwGetWindowUserPointer(window));
        gameWindow->PrepareForRelease();
        glfwSetWindowShouldClose(window, 1);
    });

    if (glfwVulkanSupported() == GLFW_FALSE)
        wowgm::exceptions::throw_with_trace<std::runtime_error>("Vulkan is not supported on your system.");
}

void GameWindow::InitializeVulkan()
{
    if (_InitializeVulkanInstance() != VK_SUCCESS)
        wowgm::exceptions::throw_with_trace<std::runtime_error>("Unable to initialize a Vulkan instance.");

    if (_SelectVulkanPhysicalDevice() != VK_SUCCESS)
        wowgm::exceptions::throw_with_trace<std::runtime_error>("Unable to select a physical device.");

    // Create a surface
    VkResult result = glfwCreateWindowSurface(_instance->GetHandle(), _window, nullptr, &_surface);
    if (result != VK_SUCCESS)
        wowgm::exceptions::throw_with_trace<std::runtime_error>("Unable to create a window surface.");

    if (_InitializeVulkanDevice() != VK_SUCCESS)
        wowgm::exceptions::throw_with_trace<std::runtime_error>("Unable to create a logical Vulkan device.");

    if (_InitializeVulkanSwapchain() != VK_SUCCESS)
        wowgm::exceptions::throw_with_trace<std::runtime_error>("Unable to create a Vulkan swapchain.");

    // Get the present queue
    _graphicsQueue = _device->GetQueueByFlags(VK_QUEUE_GRAPHICS_BIT, 0);

    if (_InitializeVulkanFramebuffer() != VK_SUCCESS)
        wowgm::exceptions::throw_with_trace<std::runtime_error>("Unable to create a framebuffer.");

    if (_InitializeInterfacePipeline() != VK_SUCCESS)
        wowgm::exceptions::throw_with_trace<std::runtime_error>("Unable to create a pipeline for the interface.");
}

void GameWindow::InitializeInterface()
{
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowPadding = ImVec2(15, 15);
    style.WindowRounding = 5.0f;
    style.FramePadding = ImVec2(5, 5);
    style.FrameRounding = 4.0f;
    style.ItemSpacing = ImVec2(12, 8);
    style.ItemInnerSpacing = ImVec2(8, 6);
    style.IndentSpacing = 25.0f;
    style.ScrollbarSize = 15.0f;
    style.ScrollbarRounding = 9.0f;
    style.GrabMinSize = 5.0f;
    style.WindowBorderSize = 1.0f;
    style.GrabRounding = 3.0f;

    style.Colors[ImGuiCol_Text] = ImVec4(0.80f, 0.80f, 0.83f, 1.00f);
    style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.05f, 0.07f, 0.90f);
    style.Colors[ImGuiCol_ChildWindowBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
    style.Colors[ImGuiCol_PopupBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
    style.Colors[ImGuiCol_Border] = ImVec4(0.06f, 0.05f, 0.07f, 0.88f);
    style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.92f, 0.91f, 0.88f, 0.00f);
    style.Colors[ImGuiCol_FrameBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
    style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
    style.Colors[ImGuiCol_TitleBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
    style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.00f, 0.98f, 0.95f, 0.75f);
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
    style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
    style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
    style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
    style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
    style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
    // style.Colors[ImGuiCol_ComboBg] = ImVec4(0.19f, 0.18f, 0.21f, 1.00f); // Merged with PopupBg
    style.Colors[ImGuiCol_CheckMark] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
    style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
    style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
    style.Colors[ImGuiCol_Button] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
    style.Colors[ImGuiCol_Header] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
    style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
    style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
    style.Colors[ImGuiCol_Column] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
    style.Colors[ImGuiCol_ColumnHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
    style.Colors[ImGuiCol_ColumnActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
    style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
    style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
    // style->Colors[ImGuiCol_CloseButton] = ImVec4(0.40f, 0.39f, 0.38f, 0.16f); // Now uses regular button colors
    // style->Colors[ImGuiCol_CloseButtonHovered] = ImVec4(0.40f, 0.39f, 0.38f, 0.39f);
    // style->Colors[ImGuiCol_CloseButtonActive] = ImVec4(0.40f, 0.39f, 0.38f, 1.00f);
    style.Colors[ImGuiCol_PlotLines] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
    style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
    style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
    style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
    style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.25f, 1.00f, 0.00f, 0.43f);
    style.Colors[ImGuiCol_ModalWindowDarkening] = ImVec4(1.00f, 0.98f, 0.95f, 0.73f);

    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->AddFontFromFileTTF("./resources/fonts/Ruda-Bold.ttf", 15.6f);
    io.Fonts->AddFontFromFileTTF("./resources/fonts/Ruda-Bold.ttf", 13.0f);
    io.Fonts->AddFontFromFileTTF("./resources/fonts/Ruda-Bold.ttf", 18.2f);
    io.Fonts->AddFontFromFileTTF("./resources/fonts/Ruda-Bold.ttf", 23.4f);
    io.Fonts->AddFontFromFileTTF("./resources/fonts/Ubuntu-Bold.ttf", 110.0f);
    io.Fonts->AddFontFromFileTTF("./resources/fonts/04B_03__.ttf", 8.0f);

    io.DisplaySize = ImVec2(float(_width), float(_height));
    io.FontGlobalScale = 1.0f;

    // Create font texture
    std::vector<uint8_t> fontData;
    int texWidth, texHeight;
    {
        unsigned char* fontBuffer;
        io.Fonts->GetTexDataAsRGBA32(&fontBuffer, &texWidth, &texHeight);
        VkDeviceSize uploadSize = texWidth * texHeight * 4 * sizeof(char);

        // Faster than assign (at least on MSVC)
        fontData.resize(uploadSize);
        memcpy(fontData.data(), fontBuffer, uploadSize);
    }

    vez::ImageCreateInfo imageCreateInfo{ };
    imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    imageCreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    imageCreateInfo.extent.width = texWidth;
    imageCreateInfo.extent.height = texHeight;
    imageCreateInfo.extent.depth = 1;
    imageCreateInfo.mipLevels = 1;
    imageCreateInfo.arrayLayers = 1;
    imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageCreateInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    VkResult result = _device->CreateImage(VMA_MEMORY_USAGE_GPU_ONLY, VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT, &imageCreateInfo, &_interfaceFontTexture);
    if (result != VK_SUCCESS)
        wowgm::exceptions::throw_with_trace<std::runtime_error>("Unable to create a font texture.");

    // Create the texture's associated view
    vez::ImageViewCreateInfo viewInfo{};
    viewInfo.image = _interfaceFontTexture;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = imageCreateInfo.format;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.layerCount = 1;
    result = vez::ImageView::Create(_device, &viewInfo, &_interfaceFontTextureView);
    if (result != VK_SUCCESS)
        wowgm::exceptions::throw_with_trace<std::runtime_error>("Unable to create a font texture.");

    // Upload the font to the GPU
    vez::ImageSubDataInfo subDataInfo = {};
    subDataInfo.imageSubresource.mipLevel = 0;
    subDataInfo.imageSubresource.baseArrayLayer = 0;
    subDataInfo.imageSubresource.layerCount = 1;
    subDataInfo.imageOffset = { 0, 0, 0 };
    subDataInfo.imageExtent = { imageCreateInfo.extent.width, imageCreateInfo.extent.height, 1 };
    result = _device->ImageSubData(_interfaceFontTexture, &subDataInfo, fontData.data());
    if (result != VK_SUCCESS)
        wowgm::exceptions::throw_with_trace<std::runtime_error>("Unable to upload font texture data to the GPU.");

    // Create a font sampler
    vez::SamplerCreateInfo samplerInfo{ };
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    result = _device->CreateSampler(&samplerInfo, &_interfaceFontSampler);
    if (result != VK_SUCCESS)
        wowgm::exceptions::throw_with_trace<std::runtime_error>("Unable to create a texture sampler.");
}

VkResult GameWindow::_InitializeVulkanInstance()
{
    vez::ApplicationInfo appInfo{};
    appInfo.pApplicationName = _title.data();
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = _engineName.data();
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);

    uint32_t instanceExtensionCount = 0;
    const char** instanceExtensions = glfwGetRequiredInstanceExtensions(&instanceExtensionCount);

    vez::InstanceCreateInfo instanceCreateInfo{};
    instanceCreateInfo.pApplicationInfo = &appInfo;
#if _DEBUG
    std::vector<const char*> extensions(instanceExtensions, instanceExtensions + instanceExtensionCount);

    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);

    instanceCreateInfo.ppEnabledExtensionNames = extensions.data();
    instanceCreateInfo.enabledExtensionCount = instanceExtensionCount + 2;

    constexpr static const char* instanceLayers[] = {
        "VK_LAYER_LUNARG_standard_validation",
    };

    instanceCreateInfo.ppEnabledLayerNames = instanceLayers;
    instanceCreateInfo.enabledLayerCount = sizeof(instanceLayers) / sizeof(const char*);

    instanceCreateInfo.debugUtils.messengerCallback = debug::debugCallback;
    instanceCreateInfo.debugUtils.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_FLAG_BITS_MAX_ENUM_EXT;
    instanceCreateInfo.debugUtils.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_FLAG_BITS_MAX_ENUM_EXT;

    instanceCreateInfo.debugReport.callback = debug::oldDebugCallback;
    instanceCreateInfo.debugReport.flags = VK_DEBUG_REPORT_FLAG_BITS_MAX_ENUM_EXT;
#else
    instanceCreateInfo.ppEnabledExtensionNames = instanceExtensions;
    instanceCreateInfo.enabledExtensionCount = instanceExtensionCount;
#endif

    VkResult result = vez::Instance::Create(&instanceCreateInfo, &_instance);
    if (result != VK_SUCCESS)
        return result;

    return result;
}

VkResult GameWindow::_SelectVulkanPhysicalDevice()
{
    // Select a physical device.
    for (auto&& physicalDevice : _instance->GetPhysicalDevices())
    {
        if (physicalDevice->GetPhysicalDeviceProperties().deviceType == VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        {
            _physicalDevice = physicalDevice;
            break;
        }
    }

    // None found, pick the first available one
    if (_physicalDevice == nullptr)
        _physicalDevice = _instance->GetPhysicalDevices()[0];

    return VK_SUCCESS;
}

VkResult GameWindow::_InitializeVulkanDevice()
{
    // Create a Vulkan device.
    constexpr static const char* deviceExtensions[] = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    };

#if _DEBUG
    constexpr static const char* instanceLayers[] = {
        "VK_LAYER_LUNARG_standard_validation",
    };
#endif

    vez::DeviceCreateInfo deviceCreateInfo{};
    deviceCreateInfo.enabledExtensionCount = sizeof(deviceExtensions) / sizeof(const char*);
    deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions;
#if _DEBUG
    deviceCreateInfo.enabledLayerCount = sizeof(instanceLayers) / sizeof(const char*);
    deviceCreateInfo.ppEnabledLayerNames = instanceLayers;
#endif
    deviceCreateInfo.physicalDevice = _physicalDevice;

    VkResult result = vez::Device::Create(_instance, &deviceCreateInfo, &_device);
    return result;
}

VkResult GameWindow::_InitializeVulkanSwapchain()
{
    vez::SwapchainCreateInfo swapchainCreateInfo{};
    swapchainCreateInfo.format = { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
    swapchainCreateInfo.surface = _surface;
    swapchainCreateInfo.tripleBuffer = true;
    VkResult result = vez::Swapchain::Create(_device, &swapchainCreateInfo, &_swapchain);
    return result;
}

VkResult GameWindow::_InitializeVulkanFramebuffer()
{
    if (_frameBuffer.Handle != nullptr)
    {
        DelayedReleaseResource<decltype(_frameBuffer)>(_frameBuffer, [&](decltype(_frameBuffer) frameBuffer) -> void {
            _device->DestroyFramebuffer(frameBuffer.Handle);
            delete frameBuffer.ColorImageView;
            delete frameBuffer.DepthStencilImageView;
            _device->DestroyImage(frameBuffer.ColorImage);
            _device->DestroyImage(frameBuffer.DepthStencilImage);
        });

        _frameBuffer.Handle = nullptr;
        _frameBuffer.ColorImageView = nullptr;
        _frameBuffer.DepthStencilImageView = nullptr;
        _frameBuffer.ColorImage = nullptr;
        _frameBuffer.DepthStencilImage = nullptr;
    }

    VkSurfaceFormatKHR swapchainFormat = _swapchain->GetFormat();

    vez::ImageCreateInfo imageCreateInfo{};
    imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    imageCreateInfo.format = swapchainFormat.format;
    imageCreateInfo.extent = { _width, _height, 1 };
    imageCreateInfo.mipLevels = 1;
    imageCreateInfo.arrayLayers = 1;
    imageCreateInfo.samples = _frameBuffer.Parameters.Sampling;
    imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    // Check the allocation bits
    VkResult result = _device->CreateImage(VMA_MEMORY_USAGE_GPU_ONLY, VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT, &imageCreateInfo, &_frameBuffer.ColorImage);
    // VkResult result = vezCreateImage(_device, VEZ_MEMORY_GPU_ONLY, &imageCreateInfo, &_frameBuffer.ColorImage);
    if (result != VK_SUCCESS)
        return result;

    vez::ImageViewCreateInfo imageViewCreateInfo{};
    imageViewCreateInfo.image = _frameBuffer.ColorImage;
    imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imageViewCreateInfo.format = imageCreateInfo.format;
    imageViewCreateInfo.subresourceRange.layerCount = 1;
    imageViewCreateInfo.subresourceRange.levelCount = 1;

    result = vez::ImageView::Create(_device, &imageViewCreateInfo, &_frameBuffer.ColorImageView);
    if (result != VK_SUCCESS)
        return result;

    // Create the depth texture
    imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    imageCreateInfo.format = VK_FORMAT_D32_SFLOAT;
    imageCreateInfo.extent = { _width, _height, 1 };
    imageCreateInfo.mipLevels = 1;
    imageCreateInfo.arrayLayers = 1;
    imageCreateInfo.samples = _frameBuffer.Parameters.Sampling;
    imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

    result = _device->CreateImage(VMA_MEMORY_USAGE_GPU_ONLY, VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT, &imageCreateInfo, &_frameBuffer.DepthStencilImage);
    // result = vezCreateImage(_device, VEZ_MEMORY_GPU_ONLY, &imageCreateInfo, &_frameBuffer.DepthStencilImage);
    if (result != VK_SUCCESS)
        return result;

    imageViewCreateInfo.image = _frameBuffer.DepthStencilImage;
    imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imageViewCreateInfo.format = imageCreateInfo.format;
    imageViewCreateInfo.subresourceRange.layerCount = 1;
    imageViewCreateInfo.subresourceRange.levelCount = 1;

    result = vez::ImageView::Create(_device, &imageViewCreateInfo, &_frameBuffer.DepthStencilImageView);
    if (result != VK_SUCCESS)
        return result;

    // And now create the framebuffer
    std::array<vez::ImageView*, 2> attachments{ _frameBuffer.ColorImageView, _frameBuffer.DepthStencilImageView };
    vez::FramebufferCreateInfo framebufferCreateInfo = {};
    framebufferCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    framebufferCreateInfo.ppAttachments = const_cast<const vez::ImageView**>(attachments.data());
    framebufferCreateInfo.width = _width;
    framebufferCreateInfo.height = _height;
    framebufferCreateInfo.layers = 1;
    result = _device->CreateFramebuffer(&framebufferCreateInfo, &_frameBuffer.Handle);
    return result;
}

VkResult GameWindow::_InitializeInterfacePipeline()
{
    using namespace wowgm::filesystem;

    auto vertexShaderFileHandle = DiskFileSystem::Instance()->OpenFile("resources/shaders/imgui/vert.spv", LoadStrategy::Mapped);
    auto fragmentShaderFileHandle = DiskFileSystem::Instance()->OpenFile("resources/shaders/imgui/frag.spv", LoadStrategy::Mapped);

    vez::ShaderModuleCreateInfo shaderModuleCreateInfo{};
    shaderModuleCreateInfo.pEntryPoint = "main";
    shaderModuleCreateInfo.codeSize = vertexShaderFileHandle->GetFileSize();
    shaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(vertexShaderFileHandle->GetData()); // Not sure this works still now (because mem-mapped)
    shaderModuleCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    VkResult result = vez::Shader::Create(_device, &shaderModuleCreateInfo, &_interfacePipeline.Shaders[0]);
    if (result != VK_SUCCESS)
        return result;

    _interfacePipeline.Shaders[0]->SetName("Shaders/Interface.vtx");

    shaderModuleCreateInfo.codeSize = fragmentShaderFileHandle->GetFileSize();
    shaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(fragmentShaderFileHandle->GetData()); // Not sure this works still now (because mem-mapped)
    shaderModuleCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    result = vez::Shader::Create(_device, &shaderModuleCreateInfo, &_interfacePipeline.Shaders[1]);
    if (result != VK_SUCCESS)
        return result;

    _interfacePipeline.Shaders[1]->SetName("Shader/Interface.fgt");

    std::array<vez::PipelineShaderStageCreateInfo, 2> pipelineShaderStages;
    pipelineShaderStages[0].pNext = nullptr;
    pipelineShaderStages[0].pSpecializationInfo = nullptr;
    pipelineShaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    pipelineShaderStages[0].shader = _interfacePipeline.Shaders[0];

    pipelineShaderStages[1].pNext = nullptr;
    pipelineShaderStages[1].pSpecializationInfo = nullptr;
    pipelineShaderStages[0].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    pipelineShaderStages[0].shader = _interfacePipeline.Shaders[1];

    vez::GraphicsPipelineCreateInfo pipelineCreateInfo{ };
    pipelineCreateInfo.stageCount = pipelineShaderStages.size();
    pipelineCreateInfo.pStages = pipelineShaderStages.data();

    // Create input format
    VkVertexInputBindingDescription vertexInputBindingDescriptions[] = {
        { 0, sizeof(ImDrawVert), VK_VERTEX_INPUT_RATE_VERTEX }
    };

    VkVertexInputAttributeDescription vertexInputAttributeDescriptions[] = {
        { 0, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(ImDrawVert, pos) },
        { 1, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(ImDrawVert, uv) },
        { 2, 0, VK_FORMAT_R8G8B8A8_UNORM, offsetof(ImDrawVert, col) }
    };
    pipelineCreateInfo.vertexInputState.pVertexBindingDescriptions = vertexInputBindingDescriptions;
    pipelineCreateInfo.vertexInputState.vertexBindingDescriptionCount = sizeof(vertexInputBindingDescriptions) / sizeof(VkVertexInputBindingDescription);
    pipelineCreateInfo.vertexInputState.pVertexAttributeDescriptions = vertexInputAttributeDescriptions;
    pipelineCreateInfo.vertexInputState.vertexAttributeDescriptionCount = sizeof(vertexInputAttributeDescriptions) / sizeof(VkVertexInputAttributeDescription);

    VkPipelineColorBlendAttachmentState colorBlendAttachmentState{};
    colorBlendAttachmentState.blendEnable = true;
    colorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT;

    pipelineCreateInfo.colorBlendState.pAttachments = &colorBlendAttachmentState;
    pipelineCreateInfo.colorBlendState.attachmentCount = 1;

    // This is the interface pipeline, we always render on top.
    pipelineCreateInfo.depthStencilState.depthTestEnable = false;

    pipelineCreateInfo.dynamicState.scissors = true;
    pipelineCreateInfo.dynamicState.viewport = true;

    // More states to set here, FIXME

    result = vez::Pipeline::Create(_device, &pipelineCreateInfo, &_interfacePipeline.Handle);
    if (result != VK_SUCCESS)
        return result;

    return result;
}

void GameWindow::Run()
{
    _interfacePipeline.PushConstants.Scale.x = 2.0f / _width;
    _interfacePipeline.PushConstants.Scale.y = 2.0f / _height;
    _interfacePipeline.PushConstants.Translate = glm::vec2(-1.0f);

    auto last_time = std::chrono::high_resolution_clock::now();
    std::chrono::microseconds last_frame_time{ };

    while (!glfwWindowShouldClose(_window))
    {
        glfwPollEvents();

        auto current_time = std::chrono::high_resolution_clock::now();

        Update(last_frame_time);
        UpdateGeometryBuffers();
        UpdateCommandBuffers();
        Draw();

        last_frame_time = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - current_time);
        _frameDrawTimes.push_back(float(last_frame_time.count()) / 1000.0f);

        // _interfaceCommandsChanged = true; // hack
        // _interfaceBuffersChanged = true; // hack
    }
}

void GameWindow::Update(std::chrono::microseconds previousFrameTime)
{
    // Draw game geometry if any

    (this->*_interfaceDrawCallback)();
}

void GameWindow::Draw()
{
    vez::SubmitInfo submitInfo{ };
    submitInfo.commandBuffers.push_back(_commandBuffers.Interface[_commandBuffers.FrameIndex]);

    VkFence submissionFence = VK_NULL_HANDLE;

    VkSemaphore semaphore = VK_NULL_HANDLE;
    submitInfo.signalSemaphores.push_back(semaphore);

    if (_graphicsQueue->Submit(1, &submitInfo, &submissionFence) != VK_SUCCESS)
        wowgm::exceptions::throw_with_trace<std::runtime_error>("Unable to submit a queue.");

    _PrepareDelayedDestruction(submissionFence);

    // Present to screen
    vez::PresentInfo presentInfo{ };
    presentInfo.waitSemaphores.push_back(semaphore);
    presentInfo.swapchains.push_back(vez::PresentInfo::PresentChain {
        _swapchain, _frameBuffer.ColorImage, VK_NOT_READY
    });

    if (_graphicsQueue->Present(&presentInfo) != VK_SUCCESS)
        wowgm::exceptions::throw_with_trace<std::runtime_error>("Unable to present the framebuffer to the target surface.");

    _ExecuteDelayedDestructionRequests();

    _commandBuffers.FrameIndex++;
    if (_commandBuffers.FrameIndex >= _commandBuffers.Interface.size())
        _commandBuffers.FrameIndex = 0;
}

void GameWindow::_PrepareDelayedDestruction(VkFence fence)
{
    decltype(_dumpster) otherDumpster;
    otherDumpster.swap(_dumpster);

    _recycler.push({ fence, [otherDumpster]() {
        for (auto&& itr : otherDumpster)
            itr();
    } });

    _dumpster.clear();
}

void GameWindow::_ExecuteDelayedDestructionRequests()
{
    while (!_recycler.empty() && _device->GetFenceStatus(_recycler.front().first) == VK_SUCCESS)
    {
        VkFence fence;
        std::function<void()> destructor;
        std::tie(fence, destructor) = _recycler.front();

        _recycler.pop();

        destructor();
        if (_recycler.empty() || _recycler.front().first != fence)
            _device->DestroyFence(fence);
    }
}

void GameWindow::_UpdateInterfaceCommandBuffers()
{
    static const float COMMAND_BUFFER_COLOR[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

    //! TODO: Since we are ultimately going to have a static interface, we should only update the command buffer
    //! TODO: when the interface actually changes (teleporting, entering world, text input, etc)

    vez::CommandBuffer* currentCommandBuffer = _commandBuffers.Interface[_commandBuffers.FrameIndex];
    if (currentCommandBuffer == nullptr)
    {
        if (_device->AllocateCommandBuffers(_graphicsQueue, nullptr, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1, &currentCommandBuffer) != VK_SUCCESS)
            wowgm::exceptions::throw_with_trace<std::runtime_error>("Unable to create a command buffer");

        currentCommandBuffer->SetName("CmdBuf/Interface");
        _commandBuffers.Interface[_commandBuffers.FrameIndex] = currentCommandBuffer;
    }
    else
    {
        VkResult result = currentCommandBuffer->Reset();
        if (result != VK_SUCCESS)
            return;
    }

    ImDrawData* drawData = ImGui::GetDrawData();
    if (drawData == nullptr || drawData->CmdListsCount == 0)
    {
        currentCommandBuffer->Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
        currentCommandBuffer->InsertLabel("Interface/Rendering", COMMAND_BUFFER_COLOR);
        currentCommandBuffer->End();

        return;
    }

    if (currentCommandBuffer->Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT) != VK_SUCCESS)
        return;

    currentCommandBuffer->BeginLabel("Interface/Rendering", COMMAND_BUFFER_COLOR);

    VkViewport viewport[] = {
        { 0.0f, 0.0f, float(_width), float(_height), 0.0f, 1.0f }
    };
    VkRect2D scissors[] = {
        { { 0, 0 }, { _width, _height } }
    };
    currentCommandBuffer->SetScissor(0, sizeof(viewport) / sizeof(VkViewport), scissors);
    currentCommandBuffer->SetViewport(0, sizeof(scissors) / sizeof(VkRect2D), viewport);

    // Begin the render pass
    {
        std::array<vez::AttachmentReference, 2> attachmentReferences { };
        attachmentReferences[0].clearValue.color = { 0.1f, 0.1f, 0.1f, 0.0f };
        attachmentReferences[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; //! TODO: Fixme, shouldn't clear since it's all previous geometry
        attachmentReferences[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachmentReferences[1].clearValue.depthStencil.depth = 1.0f;
        attachmentReferences[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; //! TODO: Fixme, shouldn't clear since it's all previous geometry
        attachmentReferences[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;

        vez::RenderPassBeginInfo renderPassBeginInfo{ };
        renderPassBeginInfo.pFramebuffer = _frameBuffer.Handle;
        renderPassBeginInfo.pAttachments = attachmentReferences.data();
        renderPassBeginInfo.attachmentCount = attachmentReferences.size();
        currentCommandBuffer->BeginRenderPass(&renderPassBeginInfo);
    }

    currentCommandBuffer->BindPipeline(_interfacePipeline.Handle);
    currentCommandBuffer->BindImageView(_interfaceFontTextureView, _interfaceFontSampler, 0, 0, 0);
    currentCommandBuffer->PushConstants(0, sizeof(_interfacePipeline.PushConstants), &_interfacePipeline.PushConstants);

    // Bind vertex buffer and index buffers
    VkDeviceSize offset = 0;
    currentCommandBuffer->BindVertexBuffers(0, 1, &_interfaceVertexBuffer, &offset);
    currentCommandBuffer->BindIndexBuffer(_interfaceIndexBuffer, offset, VK_INDEX_TYPE_UINT16);

    // Issue draw commands
    int32_t vertexOffset = 0;
    int32_t indexOffset = 0;
    for (int32_t commandList = 0; commandList < drawData->CmdListsCount; ++commandList)
    {
        const ImDrawList* drawList = drawData->CmdLists[commandList];
        for (int32_t commandIndex = 0; commandIndex < drawList->CmdBuffer.Size; ++commandIndex)
        {
            const ImDrawCmd* drawCommand = &drawList->CmdBuffer[commandIndex];
            VkRect2D drawCommandScissors{
                { static_cast<int32_t>(std::max(drawCommand->ClipRect.x, 0.0f)), static_cast<int32_t>(std::max(drawCommand->ClipRect.y, 0.0f)) },
                { uint32_t(drawCommand->ClipRect.z - drawCommand->ClipRect.x), uint32_t(drawCommand->ClipRect.w - drawCommand->ClipRect.y) }
            };
            currentCommandBuffer->SetScissor(0, 1, &drawCommandScissors);
            currentCommandBuffer->DrawIndexed(drawCommand->ElemCount, 1, indexOffset, vertexOffset, 0);

            indexOffset += drawCommand->ElemCount;
        }

        vertexOffset += drawList->VtxBuffer.Size;
    }

    // All done, finish the render pass
    currentCommandBuffer->EndRenderPass();
    currentCommandBuffer->EndLabel();

    if (currentCommandBuffer->End() != VK_SUCCESS)
        return;

}

void GameWindow::UpdateCommandBuffers()
{
    _UpdateInterfaceCommandBuffers();
}

void GameWindow::UpdateGeometryBuffers()
{
    _UpdateInterfaceGeometryBuffers();
}

void GameWindow::_UpdateInterfaceGeometryBuffers()
{
    // Abort if there is no geometry to render.
    ImDrawData* imDrawData = ImGui::GetDrawData();
    if (imDrawData == nullptr || imDrawData->TotalIdxCount == 0 || imDrawData->TotalVtxCount == 0)
        return;

    // Update buffers as needed
    bool updateVertexBuffer = false;
    bool updateIndexBuffer = false;
    if (imDrawData->TotalVtxCount != _interfaceVertexCount)
    {
        _interfaceVertexCount = imDrawData->TotalVtxCount;

        // Destroy the previously created vertex buffer
        if (_interfaceVertexBuffer != VK_NULL_HANDLE)
        {
            DelayedReleaseResource<vez::Buffer*>(_interfaceVertexBuffer, [&](vez::Buffer* buffer) -> void {
                _device->DestroyBuffer(buffer);
            });
            _interfaceVertexBuffer = VK_NULL_HANDLE;
        }

        vez::BufferCreateInfo bufferCreateInfo{};
        bufferCreateInfo.size = imDrawData->TotalVtxCount * sizeof(ImDrawVert);
        bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        VkResult result = _device->CreateBuffer(VMA_MEMORY_USAGE_CPU_TO_GPU, VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT, &bufferCreateInfo, &_interfaceVertexBuffer);
        if (result != VK_SUCCESS)
            return;

        _interfaceVertexBuffer->SetName("Buffer/Interface.Vertices");
        updateVertexBuffer = true;
    }

    if (imDrawData->TotalIdxCount != _interfaceIndexCount)
    {
        // Destroy previously created index buffer
        if (_interfaceIndexBuffer != VK_NULL_HANDLE)
        {
            DelayedReleaseResource<vez::Buffer*>(_interfaceIndexBuffer, [&](vez::Buffer* buffer) -> void {
                _device->DestroyBuffer(buffer);
            });
            _interfaceIndexBuffer = VK_NULL_HANDLE;
        }

        vez::BufferCreateInfo bufferCreateInfo{};
        bufferCreateInfo.size = imDrawData->TotalIdxCount * sizeof(ImDrawIdx);
        bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        VkResult result = _device->CreateBuffer(VMA_MEMORY_USAGE_CPU_TO_GPU, VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT, &bufferCreateInfo, &_interfaceIndexBuffer);
        if (result != VK_SUCCESS)
            return;

        _interfaceIndexBuffer->SetName("Buffer/Interface.Indices");
        updateIndexBuffer = true;
    }

    if (!updateIndexBuffer && !updateVertexBuffer)
        return;

    uint32_t vertexCount = 0;
    uint32_t indexCount = 0;

    for (int32_t i = 0; i < imDrawData->CmdListsCount; ++i)
    {
        indexCount += imDrawData->CmdLists[i]->IdxBuffer.Size;
        vertexCount += imDrawData->CmdLists[i]->VtxBuffer.Size;
    }

    std::vector<ImDrawVert> vertices(vertexCount);
    std::vector<ImDrawIdx> indices(indexCount);
    uint32_t vertexOffset = 0;
    uint32_t indexOffset = 0;

    for (int32_t i = 0; i < imDrawData->CmdListsCount; ++i)
    {
        const ImDrawList* drawList = imDrawData->CmdLists[i];

        if (updateVertexBuffer)
            memcpy(vertices.data() + vertexOffset, drawList->VtxBuffer.Data, drawList->VtxBuffer.Size * sizeof(ImDrawVert));

        if (updateIndexBuffer)
            memcpy(indices.data() + indexOffset, drawList->IdxBuffer.Data, drawList->IdxBuffer.Size * sizeof(ImDrawIdx));

        indexOffset += drawList->IdxBuffer.Size;
        vertexOffset += drawList->VtxBuffer.Size;
    }

    if (updateIndexBuffer)
        _interfaceIndexBuffer->WriteMemory(0, indices.data(), indices.size());

    if (updateVertexBuffer)
        _interfaceVertexBuffer->WriteMemory(0, vertices.data(), vertices.size());
}

void GameWindow::OnKeyUp(int key, int mods)
{
    if (key == GLFW_KEY_F12 && mods == GLFW_MOD_SHIFT)
        ToggleFullscreen();

    if (key == GLFW_KEY_F3 && mods == GLFW_MOD_ALT)
        _showFramesMetrics = !_showFramesMetrics;
}

void GameWindow::OnKeyDown(int key, int mods)
{

}

void GameWindow::OnMouseUp(int button, float x, float y)
{
    ImGuiIO& io = ImGui::GetIO();

    io.MousePos.x = x;
    io.MousePos.y = y;

    switch (button)
    {
        case GLFW_MOUSE_BUTTON_LEFT:
        case GLFW_MOUSE_BUTTON_RIGHT:
        case GLFW_MOUSE_BUTTON_MIDDLE:
            io.MouseDown[button] = false;
            break;
    }
}

void GameWindow::OnMouseDown(int button, float x, float y)
{
    ImGuiIO& io = ImGui::GetIO();

    io.MousePos.x = x;
    io.MousePos.y = y;

    switch (button)
    {
        case GLFW_MOUSE_BUTTON_LEFT:
        case GLFW_MOUSE_BUTTON_RIGHT:
        case GLFW_MOUSE_BUTTON_MIDDLE:
            io.MouseDown[button] = true;
            break;
    }
}

void GameWindow::OnMouseScroll(float x, float y)
{
    ImGuiIO& io = ImGui::GetIO();
    io.MouseWheelH += x;
    io.MouseWheel += y;
}

void GameWindow::PrepareForRelease()
{
    // Clean up and stop rendering
}

void GameWindow::OnResize(uint32_t width, uint32_t height)
{
    _width = width;
    _height = height;

    // Update values for the interface shaders
    _interfacePipeline.PushConstants.Scale.x = 2.0f / width;
    _interfacePipeline.PushConstants.Scale.y = 2.0f / height;
    _interfacePipeline.PushConstants.Translate = glm::vec2(-1.0f);

    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2(float(_width), float(_height));

    // Recreate the framebuffer
    _InitializeVulkanFramebuffer();
}

void GameWindow::ToggleVSync()
{
    _vsync ^= true;

    // _device->WaitIdle();
}

void GameWindow::ToggleFullscreen(int32_t selectedMonitor /* = -1 */)
{
    GLFWmonitor* monitor = glfwGetWindowMonitor(_window);
    if (monitor == nullptr)
    {
        int32_t monitorCount = 0;
        GLFWmonitor** monitors = glfwGetMonitors(&monitorCount);
        monitor = monitors[0]; // Default to the first monitor available.
        if (selectedMonitor >= 0 && selectedMonitor < monitorCount)
            monitor = monitors[selectedMonitor];

        const GLFWvidmode* videoMode = glfwGetVideoMode(monitor);
        _width = videoMode->width;
        _height = videoMode->height;

        glfwGetWindowPos(_window, &_previousWindowData[0], &_previousWindowData[1]);
        glfwGetWindowSize(_window, &_previousWindowData[2], &_previousWindowData[3]);

        glfwSetWindowMonitor(_window, monitor, 0, 0, _width, _height, GLFW_DONT_CARE);
    }
    else
    {
        glfwSetWindowMonitor(_window, nullptr, _previousWindowData[0], _previousWindowData[1], _previousWindowData[2], _previousWindowData[3], GLFW_DONT_CARE);
    }
}
