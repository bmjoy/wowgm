
#include <graphics/vulkan/Instance.hpp>
#include <graphics/vulkan/PhysicalDevice.hpp>

#include <shared/threading/thread_pool.hpp>
#include <shared/log/log.hpp>

#include <sstream>

namespace gfx::vk
{
    Instance::Instance(const InstanceCreateInfo* pCreateInfo)
    {
        InstanceCreateInfo* mutableCreateInfo = const_cast<InstanceCreateInfo*>(pCreateInfo);

#if _DEBUG
        mutableCreateInfo->enabledLayerNames.push_back("VK_LAYER_LUNARG_standard_validation");

        if (pCreateInfo->debugUtils.messengerCallback != nullptr)
            mutableCreateInfo->enabledExtensionNames.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

        if (pCreateInfo->debugReport.callback != nullptr)
            mutableCreateInfo->enabledExtensionNames.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
#endif
        { // Remove unsupported layers
            uint32_t supportedLayerCount;
            vkEnumerateInstanceLayerProperties(&supportedLayerCount, nullptr);
            std::vector<VkLayerProperties> supportedLayers(supportedLayerCount);
            vkEnumerateInstanceLayerProperties(&supportedLayerCount, supportedLayers.data());

            for (auto itr = mutableCreateInfo->enabledLayerNames.begin(); itr != mutableCreateInfo->enabledLayerNames.end();)
            {
                auto supportedLayerItr = std::find_if(supportedLayers.begin(), supportedLayers.end(), [&itr](VkLayerProperties properties) -> bool {
                    return strcmp(*itr, properties.layerName) == 0;
                });

                if (supportedLayerItr != supportedLayers.end())
                    ++itr;
                else
                    itr = mutableCreateInfo->enabledLayerNames.erase(itr);
            }

            std::stringstream lss;
            for (auto&& itr : mutableCreateInfo->enabledLayerNames)
                lss << itr << ", ";

            auto layersStr = lss.str();
            layersStr = layersStr.substr(0, layersStr.size() - 2);

            LOG_GRAPHICS("Enabled Vulkan layers: {}", layersStr.c_str());
        }

        { // Remove unsupported extensions
            uint32_t supportedExtensionCount;
            vkEnumerateInstanceExtensionProperties(nullptr, &supportedExtensionCount, nullptr);
            std::vector<VkExtensionProperties> supportedExtensions(supportedExtensionCount);
            vkEnumerateInstanceExtensionProperties(nullptr, &supportedExtensionCount, supportedExtensions.data());

            for (auto itr = mutableCreateInfo->enabledExtensionNames.begin(); itr != mutableCreateInfo->enabledExtensionNames.end();)
            {
                auto supportedExtensionItr = std::find_if(supportedExtensions.begin(), supportedExtensions.end(), [&itr](VkExtensionProperties properties) -> bool {
                    return strcmp(*itr, properties.extensionName) == 0;
                });

                if (supportedExtensionItr != supportedExtensions.end())
                    ++itr;
                else
                    itr = mutableCreateInfo->enabledExtensionNames.erase(itr);
            }

            std::stringstream xss;
            for (auto&& itr : mutableCreateInfo->enabledExtensionNames)
                xss << itr << ", ";

            auto extStr = xss.str();
            extStr = extStr.substr(0, extStr.size() - 2);

            LOG_GRAPHICS("Enabled Vulkan extensions: {}", extStr.c_str());
        }

        VkInstanceCreateInfo createInfo{ };
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.flags = pCreateInfo->flags;
        createInfo.ppEnabledExtensionNames = pCreateInfo->enabledExtensionNames.data();
        createInfo.enabledExtensionCount   = pCreateInfo->enabledExtensionNames.size();
        createInfo.ppEnabledLayerNames     = pCreateInfo->enabledLayerNames.data();
        createInfo.enabledLayerCount       = pCreateInfo->enabledLayerNames.size();

        VkApplicationInfo applicationInfo { };
        applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        applicationInfo.apiVersion = pCreateInfo->pApplicationInfo->apiVersion;
        applicationInfo.applicationVersion = pCreateInfo->pApplicationInfo->applicationVersion;
        applicationInfo.engineVersion = pCreateInfo->pApplicationInfo->engineVersion;
        applicationInfo.pApplicationName = pCreateInfo->pApplicationInfo->pApplicationName;
        applicationInfo.pEngineName = pCreateInfo->pApplicationInfo->pEngineName;

        createInfo.pApplicationInfo = &applicationInfo;

        VkResult result = vkCreateInstance(&createInfo, nullptr, &_handle);
        BOOST_ASSERT_MSG(result == VK_SUCCESS, "Failed to create a vkInstance");

        // Get all the physical devices
        uint32_t physicalDeviceCount = 0;
        result = vkEnumeratePhysicalDevices(_handle, &physicalDeviceCount, nullptr);

        std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
        result = vkEnumeratePhysicalDevices(_handle, &physicalDeviceCount, physicalDevices.data());

        _physicalDevices.resize(physicalDeviceCount);
        for (uint32_t i = 0; i < physicalDeviceCount; ++i)
            _physicalDevices[i] = new PhysicalDevice(this, physicalDevices[i]);

        // Create a thread pool with an unique worker thread for now.
        _threadPool = new thread_pool(1);

#if _DEBUG
        // Install debug callbacks if needed
        if (pCreateInfo->debugUtils.messengerCallback != nullptr)
        {
            auto debugUtilsCallbackInstaller = PFN_vkCreateDebugUtilsMessengerEXT(vkGetInstanceProcAddr(_handle, "vkCreateDebugUtilsMessengerEXT"));
            if (debugUtilsCallbackInstaller != nullptr)
            {
                VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
                debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
                debugCreateInfo.messageSeverity = pCreateInfo->debugUtils.messageSeverity;
                debugCreateInfo.messageType = pCreateInfo->debugUtils.messageType;
                debugCreateInfo.pfnUserCallback = pCreateInfo->debugUtils.messengerCallback;
                debugCreateInfo.pUserData = pCreateInfo->debugUtils.pUserData; // Optional

                result = debugUtilsCallbackInstaller(_handle, &debugCreateInfo, nullptr, &_debugUtilsUserCallback);
                BOOST_ASSERT_MSG(result == VK_SUCCESS, "Failed to install a VK_EXT_debug_utils callback handler");
            }
        }
        else if (pCreateInfo->debugReport.callback != nullptr)
        {
            auto debugReportCallbackInstaller = PFN_vkCreateDebugReportCallbackEXT(vkGetInstanceProcAddr(_handle, "vkCreateDebugReportCallbackEXT"));
            if (debugReportCallbackInstaller != nullptr)
            {
                VkDebugReportCallbackCreateInfoEXT debugCreateInfo{};
                debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
                debugCreateInfo.flags = pCreateInfo->debugReport.flags;
                debugCreateInfo.pfnCallback = pCreateInfo->debugReport.callback;
                debugCreateInfo.pUserData = pCreateInfo->debugReport.pUserData;

                result = debugReportCallbackInstaller(_handle, &debugCreateInfo, nullptr, &_debugReportCallback);
                BOOST_ASSERT_MSG(result == VK_SUCCESS, "Failed to install a VK_EXT_debug_report callback handler");
            }
        }

        auto contains_extension = [](std::vector<const char*> const& extensions, const char* needle) -> bool {
            return std::find_if(std::begin(extensions), std::end(extensions), [&needle](auto itr) -> bool {
                return strcmp(needle, itr) == 0;
            }) != std::end(extensions);
        };

        if (contains_extension(pCreateInfo->enabledExtensionNames, VK_EXT_DEBUG_UTILS_EXTENSION_NAME))
            vkSetDebugUtilsObjectNameEXT = PFN_vkSetDebugUtilsObjectNameEXT(vkGetInstanceProcAddr(_handle, "vkSetDebugUtilsObjectNameEXT"));

        if (vkSetDebugUtilsObjectNameEXT == nullptr && contains_extension(pCreateInfo->enabledExtensionNames, VK_EXT_DEBUG_REPORT_EXTENSION_NAME))
            vkDebugMarkerSetObjectNameEXT = PFN_vkDebugMarkerSetObjectNameEXT(vkGetInstanceProcAddr(_handle, "vkDebugMarkerSetObjectNameEXT"));

#endif
    }

    Instance::~Instance()
    {
        delete _threadPool;

        for (auto&& itr : _physicalDevices)
            delete itr;

        _physicalDevices.clear();

#if _DEBUG
        if (_debugUtilsUserCallback != nullptr)
        {
            auto debugDestroyUtilsMessenger = PFN_vkDestroyDebugUtilsMessengerEXT(vkGetInstanceProcAddr(_handle, "vkDestroyDebugUtilsMessengerEXT"));
            if (debugDestroyUtilsMessenger != nullptr)
                debugDestroyUtilsMessenger(_handle, _debugUtilsUserCallback, nullptr);
        }
        else if (_debugReportCallback != nullptr)
        {
            auto destroyDebugReportCallback = PFN_vkDestroyDebugReportCallbackEXT(vkGetInstanceProcAddr(_handle, "vkDestroyDebugReportCallbackEXT"));
            if (destroyDebugReportCallback != nullptr)
                destroyDebugReportCallback(_handle, _debugReportCallback, nullptr);
        }
#endif

        vkDestroyInstance(_handle, nullptr);
        _handle = VK_NULL_HANDLE;
    }
}
