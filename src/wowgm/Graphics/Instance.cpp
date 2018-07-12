#include "Instance.hpp"
#include "Logger.hpp"
#include "Surface.hpp"
#include "PhysicalDevice.hpp"
#include "LogicalDevice.hpp"
#include "SharedGraphicsDefines.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <set>

/// Execution chain
/// 1. Create an Instance
/// 2. Setup the debug callbacks
/// 3. Bind a Surface
/// 4. Select a PhysicalDevice
/// 5. Create a LogicalDevice
/// 6. Create a SwapChain
/// 7. Create ImageViews for the Swap chain (TODO: Fold to 6.)
/// 8. Create a Pipeline

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
    }

    std::unique_ptr<Instance> Instance::Create(const char* applicationName, const char* engineName)
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
        createInfo.enabledLayerCount = static_cast<std::uint32_t>(details::requiredValidationLayers.size());
        createInfo.ppEnabledLayerNames = details::requiredValidationLayers.data();
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

        return std::make_unique<Instance>(instance);
    }

    Instance::Instance(VkInstance instance) : _instance(instance)
    {
        _surface = VK_NULL_HANDLE;
    }

    Instance::~Instance()
    {
        // Dstroy surface
        delete _surface;
        _surface = nullptr;

#ifdef ENABLE_VALIDATION_LAYERS
        details::DestroyDebugReportCallbackEXT(_instance, _debugReportCallback, nullptr);
#endif

        // Destroy instance
        vkDestroyInstance(_instance, nullptr);
        _instance = VK_NULL_HANDLE;

        // Destroy physical devices
        _physicalDevices.clear();
    }

    VkInstance Instance::GetVkInstance()
    {
        return _instance;
    }

    LogicalDevice* Instance::GetLogicalDevice()
    {
        return _logicalDevice;
    }

    LogicalDevice* Instance::CreateLogicalDevice()
    {
        QueueFamilyIndices& indices = GetPhysicalDevice()->GetQueues();

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<std::int32_t> uniqueQueueFamilies = { indices.Graphics, indices.Present };

        float queuePriority = 1.0f;
        for (int queueFamily : uniqueQueueFamilies) {
            VkDeviceQueueCreateInfo queueCreateInfo = {};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        VkPhysicalDeviceFeatures deviceFeatures = {};

        VkDeviceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();

        createInfo.pEnabledFeatures = &deviceFeatures;

        createInfo.enabledExtensionCount = static_cast<uint32_t>(details::requiredDeviceExtensions.size());
        createInfo.ppEnabledExtensionNames = details::requiredDeviceExtensions.data();

#ifdef ENABLE_VALIDATION_LAYERS
        createInfo.enabledLayerCount = static_cast<uint32_t>(details::requiredValidationLayers.size());
        createInfo.ppEnabledLayerNames = details::requiredValidationLayers.data();
#else
        createInfo.enabledLayerCount = 0;
#endif

        VkDevice device;

        if (vkCreateDevice(*GetPhysicalDevice(), &createInfo, nullptr, &device) != VK_SUCCESS)
            throw std::runtime_error("failed to create logical device!");

        _logicalDevice = new LogicalDevice(device, indices);
        return _logicalDevice;
    }

    Surface* Instance::CreateSurface(Window* window)
    {
        VkSurfaceKHR surface;
        VkResult windowCreateResult = glfwCreateWindowSurface(_instance, window->GetHandle(), nullptr, &surface);
        if (windowCreateResult != VK_SUCCESS)
            throw std::runtime_error("Unable to create a surface");

        _surface = new Surface(this, surface, window->GetWidth(), window->GetHeight());

        // NOTE: This was previously in the ctor but we need _surface for emplace
        std::uint32_t physicalDeviceCount = 0;
        vkEnumeratePhysicalDevices(_instance, &physicalDeviceCount, nullptr);

        std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
        vkEnumeratePhysicalDevices(_instance, &physicalDeviceCount, physicalDevices.data());

        _physicalDevices.resize(physicalDeviceCount);
        for (std::uint32_t i = 0; i < physicalDeviceCount; ++i)
            _physicalDevices[i] = std::make_unique<PhysicalDevice>(physicalDevices[i], _surface);

        _SelectPhysicalDevice();
        // -----------------------------------------------------------------------

        return _surface;
    }

    void Instance::_SelectPhysicalDevice()
    {
        std::uint32_t bestScore = 0;
        std::uint32_t index = 0;
        for (auto itr = _physicalDevices.begin(); itr != _physicalDevices.end(); ++itr, ++index)
        {
            std::uint32_t deviceScore = (*itr)->GetScore();
            if (deviceScore < bestScore)
                continue;

            bestScore = deviceScore;
            _selectedPhysicalDevice = index;
        }
    }

    PhysicalDevice* Instance::GetPhysicalDevice(std::uint32_t index)
    {
        return _physicalDevices[index].get();
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

    PhysicalDevice* Instance::GetPhysicalDevice()
    {
        return GetPhysicalDevice(_selectedPhysicalDevice);
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

            for (const char* layerName : requiredValidationLayers)
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
