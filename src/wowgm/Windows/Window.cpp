#include "Window.hpp"
#include "Logger.hpp"

#include <stdexcept>
#include <iostream>
#include <cstring>

namespace wowgm::windows
{
    const std::vector<const char*> validationLayers = {
        "VK_LAYER_LUNARG_standard_validation"
    };

    VkResult CreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback);
    void DestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator);

    static VKAPI_ATTR VkBool32 VKAPI_CALL globalDebugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType,
        uint64_t obj, size_t location, int32_t code, const char* layerPrefix, const char* msg, void* userData)
    {
        LOG_GRAPHICS << msg << std::endl;
        return VK_FALSE;
    }

    window::window(const char* title) : window(800u, 600u, title)
    {

    }

    window::window(std::uint32_t width, std::uint32_t height, const char* title) : _width(width), _height(height), _title(title)
    {
    }

    void window::InitializeWindow()
    {
        glfwInit();

        // No API (Originally designed for OpenGL contexts)
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        // Disable resize for now.
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        _window = glfwCreateWindow(_width, _height, _title, nullptr, nullptr);
    }

    void window::Run()
    {
        InitializeWindow();
        InitializeVulkan();
        Execute();
        Cleanup();
    }

    void window::CreateVulkanInstance()
    {
        if (!CheckValidationLayerSupport())
            throw std::runtime_error("Validation layers are not supported on your system!");

        // Application informations
        VkApplicationInfo appInfo = {};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "WowGM";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        auto requiredExtensions = GetRequiredExtensions();
        createInfo.enabledExtensionCount = static_cast<std::uint32_t>(requiredExtensions.size());
        createInfo.ppEnabledExtensionNames = requiredExtensions.data();

        // Define validation layers (debug only)
#ifdef ENABLE_VALIDATION_LAYERS
        createInfo.enabledLayerCount = static_cast<std::uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
#else
        createInfo.enabledLayerCount = 0;
#endif

        // Create the instance.
        // For allocators, see https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#memory-allocation.
        // TLDR: We let the driver allocate memory itself.
        VkResult result = vkCreateInstance(&createInfo, nullptr, &_instance);
        if (result != VK_SUCCESS)
            throw std::runtime_error("Unable to initialize Vulkan!");
    }

    void window::InitializeVulkan()
    {
        CreateVulkanInstance();
        SetupDebugCallback();
    }

    void window::SetupDebugCallback()
    {
#ifdef ENABLE_VALIDATION_LAYERS
        VkDebugReportCallbackCreateInfoEXT createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
        createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
        createInfo.pfnCallback = globalDebugCallback;

        if (CreateDebugReportCallbackEXT(_instance, &createInfo, nullptr, &_debugReportCallback) != VK_SUCCESS)
            throw std::runtime_error("Failed to set up debug callback!");
#endif
    }

    void window::Execute()
    {
        // Also plug ourselves in here.
        while (!glfwWindowShouldClose(_window))
            glfwPollEvents();
    }

    void window::Cleanup()
    {
#ifdef ENABLE_VALIDATION_LAYERS
        DestroyDebugReportCallbackEXT(_instance, _debugReportCallback, nullptr);
#endif

        vkDestroyInstance(_instance, nullptr);

        glfwDestroyWindow(_window);
        glfwTerminate();
    }

    std::vector<VkExtensionProperties> window::GetAvailableExtensions(const char* layerName)
    {
        // First, we obtain the number of extensions across all layers...
        uint32_t extensionCount = 0;
        if (vkEnumerateInstanceExtensionProperties(layerName, &extensionCount, nullptr) != VK_SUCCESS)
            throw std::runtime_error("Unable to enumerate Vulkan extensions!");

        // And then the actual details.
        std::vector<VkExtensionProperties> extensions(extensionCount);
        if (vkEnumerateInstanceExtensionProperties(layerName, &extensionCount, extensions.data()) != VK_SUCCESS)
            throw std::runtime_error("Unable to enumerate Vulkan extensions!");

        return extensions;
    }

    bool window::CheckValidationLayerSupport()
    {
#ifdef ENABLE_VALIDATION_LAYERS
        std::uint32_t layerCount;
        if (vkEnumerateInstanceLayerProperties(&layerCount, nullptr) != VK_SUCCESS)
            throw std::runtime_error("Unable to enumerate Vulkan layers!");

        std::vector<VkLayerProperties> availableLayers(layerCount);
        if (vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data()) != VK_SUCCESS)
            throw std::runtime_error("Unable to enumerate Vulkan layers!");

        for (const char* layerName : validationLayers)
        {
            bool layerFound = false;
            for (const auto& layerProperties : availableLayers)
            {
                if (strcmp(layerName, layerProperties.layerName) == 0)
                {
                    layerFound = true;
                    break;
                }
            }

            if (!layerFound)
                return false;
        }
#endif

        return true;
    }

    std::vector<const char*> window::GetRequiredExtensions() {
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

#ifdef ENABLE_VALIDATION_LAYERS
        extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
#endif

        return extensions;
    }

    VkResult CreateDebugReportCallbackEXT(VkInstance instance,
        const VkDebugReportCallbackCreateInfoEXT* pCreateInfo,
        const VkAllocationCallbacks* pAllocator,
        VkDebugReportCallbackEXT* pCallback)
    {
        auto func = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
        if (func != nullptr)
            return func(instance, pCreateInfo, pAllocator, pCallback);
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }

    void DestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator)
    {
        auto func = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
        if (func != nullptr)
            func(instance, callback, pAllocator);
    }
}

