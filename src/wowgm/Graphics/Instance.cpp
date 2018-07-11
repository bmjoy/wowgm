#include "Instance.hpp"
#include "Logger.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace wowgm::graphics
{
    namespace details
    {
        bool CheckValidationLayerSupport();
        std::vector<const char*> GetRequiredExtensions();

        VkResult CreateDebugReportCallbackEXT(VkInstance instance,
            const VkDebugReportCallbackCreateInfoEXT* pCreateInfo,
            const VkAllocationCallbacks* pAllocator,
            VkDebugReportCallbackEXT* pCallback);
        void DestroyDebugReportCallbackEXT(VkInstance instance,
            VkDebugReportCallbackEXT callback,
            const VkAllocationCallbacks* pAllocator);

        static VKAPI_ATTR VkBool32 VKAPI_CALL globalDebugCallback(VkDebugReportFlagsEXT flags,
            VkDebugReportObjectTypeEXT objType,
            uint64_t obj, size_t location, int32_t code,
            const char* layerPrefix, const char* msg,
            void* userData)
        {
            // TOOD: LOG_GRAPHICS expands to a macro that collides with the namespace
            // ::LOG_GRAPHICS << msg << std::endl;
            return VK_FALSE;
        }

        const std::vector<const char*> validationLayers = {
            "VK_LAYER_LUNARG_standard_validation"
        };
    }

    Instance* Instance::Create(const char* applicationName, const char* engineName)
    {
        if (!details::CheckValidationLayerSupport())
            throw std::runtime_error("Validation layers are not supported on your system!");

        VkApplicationInfo appInfo = {};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = applicationName;
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = engineName;
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        auto requiredExtensions = details::GetRequiredExtensions();
        createInfo.enabledExtensionCount = static_cast<std::uint32_t>(requiredExtensions.size());
        createInfo.ppEnabledExtensionNames = requiredExtensions.data();

#ifdef ENABLE_VALIDATION_LAYERS
        createInfo.enabledLayerCount = static_cast<std::uint32_t>(details::validationLayers.size());
        createInfo.ppEnabledLayerNames = details::validationLayers.data();
#else
        createInfo.enabledLayerCount = 0;
#endif

        // Create the instance.
        // For allocators, see https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#memory-allocation.
        // TLDR: We let the driver allocate memory itself.
        VkInstance instance;
        VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
        if (result != VK_SUCCESS)
            throw std::runtime_error("Unable to initialize Vulkan!");

        Instance* vulkanInstance = new Instance(instance);
        return vulkanInstance;
    }

    Instance::Instance(VkInstance instance) : _instance(instance)
    {
        std::uint32_t physicalDeviceCount = 0;
        vkEnumeratePhysicalDevices(_instance, &physicalDeviceCount, nullptr);

        std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
        vkEnumeratePhysicalDevices(_instance, &physicalDeviceCount, physicalDevices.data());

        _physicalDevices.resize(physicalDeviceCount);
        for (std::uint32_t i = 0; i < physicalDeviceCount; ++i)
            _physicalDevices.emplace(_physicalDevices.begin() + i, physicalDevices[i]);

        SelectPhysicalDevice();
    }

    Instance::~Instance()
    {
#ifdef ENABLE_VALIDATION_LAYERS
        details::DestroyDebugReportCallbackEXT(_instance, _debugReportCallback, nullptr);
#endif

        vkDestroyInstance(_instance, nullptr);
    }

    void Instance::SelectPhysicalDevice(std::uint32_t deviceIndex)
    {
        _selectedPhysicalDevice = _physicalDevices[deviceIndex];
    }

    void Instance::SelectPhysicalDevice()
    {
        std::uint32_t bestScore = 0;
        for (auto itr = _physicalDevices.begin(); itr != _physicalDevices.end(); ++itr)
        {
            std::uint32_t deviceScore = itr->GetScore();
            if (deviceScore < bestScore)
                continue;

            bestScore = deviceScore;
            _selectedPhysicalDevice = *itr;
        }
    }

    LogicalDevice& Instance::GetLogicalDevice(std::uint32_t index)
    {
        return _logicalDevices[index];
    }

    std::vector<LogicalDevice>::iterator Instance::IterateLogicalDevices()
    {
        return _logicalDevices.begin();
    }

    PhysicalDevice& Instance::GetPhysicalDevice(std::uint32_t index)
    {
        return _physicalDevices[index];
    }

    std::vector<PhysicalDevice>::iterator Instance::IteratePhysicalDevices()
    {
        return _physicalDevices.begin();
    }

    void Instance::SetupDebugCallback()
    {
#ifdef ENABLE_VALIDATION_LAYERS
        VkDebugReportCallbackCreateInfoEXT createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
        createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
        createInfo.pfnCallback = details::globalDebugCallback;

        if (details::CreateDebugReportCallbackEXT(_instance, &createInfo, nullptr, &_debugReportCallback) != VK_SUCCESS)
            throw std::runtime_error("Failed to set up debug callback!");
#endif
    }

    namespace details
    {
        bool CheckValidationLayerSupport()
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

        std::vector<const char*> GetRequiredExtensions()
        {
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
}
