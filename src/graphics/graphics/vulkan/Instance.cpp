
#include "Instance.hpp"
#include "PhysicalDevice.hpp"


namespace gfx::vk
{
    VkResult Instance::Create(const InstanceCreateInfo* pCreateInfo, Instance** ppInstance)
    {
        VkInstanceCreateInfo createInfo{ };
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.flags = pCreateInfo->flags;
        createInfo.ppEnabledExtensionNames = pCreateInfo->ppEnabledExtensionNames;
        createInfo.enabledExtensionCount = pCreateInfo->enabledExtensionCount;
        createInfo.ppEnabledLayerNames = pCreateInfo->ppEnabledLayerNames;
        createInfo.enabledLayerCount = pCreateInfo->enabledLayerCount;

        VkApplicationInfo applicationInfo{};
        applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        applicationInfo.apiVersion = pCreateInfo->pApplicationInfo->apiVersion;
        applicationInfo.applicationVersion = pCreateInfo->pApplicationInfo->applicationVersion;
        applicationInfo.engineVersion = pCreateInfo->pApplicationInfo->engineVersion;
        applicationInfo.pApplicationName = pCreateInfo->pApplicationInfo->pApplicationName;
        applicationInfo.pEngineName = pCreateInfo->pApplicationInfo->pEngineName;

        createInfo.pApplicationInfo = &applicationInfo;

        VkInstance instanceHandle = VK_NULL_HANDLE;
        VkResult result = vkCreateInstance(&createInfo, nullptr, &instanceHandle);
        if (result != VK_SUCCESS)
            return result;

        Instance* instance = new Instance;
        instance->_handle = instanceHandle;

        // Get all the physical devices
        uint32_t physicalDeviceCount = 0;
        result = vkEnumeratePhysicalDevices(instanceHandle, &physicalDeviceCount, nullptr);
        if (result != VK_SUCCESS)
            return result;

        if (physicalDeviceCount < 0)
            return VK_SUCCESS;

        std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
        result = vkEnumeratePhysicalDevices(instanceHandle, &physicalDeviceCount, physicalDevices.data());
        if (result != VK_SUCCESS)
            return result;

        instance->_physicalDevices.resize(physicalDeviceCount);
        for (uint32_t i = 0; i < physicalDeviceCount; ++i)
            instance->_physicalDevices[i] = new PhysicalDevice(instance, physicalDevices[i]);

        // Create a thread pool with an unique worker thread for now.
        instance->_threadPool = new ThreadPool(1);

#if _DEBUG
        // Install debug callbacks if needed
        if (pCreateInfo->debugUtils.messengerCallback != nullptr)
        {

            auto debugUtilsCallbackInstaller = PFN_vkCreateDebugUtilsMessengerEXT(vkGetInstanceProcAddr(instanceHandle, "vkCreateDebugUtilsMessengerEXT"));
            if (debugUtilsCallbackInstaller != nullptr)
            {
                VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
                debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
                debugCreateInfo.messageSeverity = pCreateInfo->debugUtils.messageSeverity;
                debugCreateInfo.messageType = pCreateInfo->debugUtils.messageType;
                debugCreateInfo.pfnUserCallback = pCreateInfo->debugUtils.messengerCallback;
                debugCreateInfo.pUserData = nullptr; // Optional

                result = debugUtilsCallbackInstaller(instanceHandle, &debugCreateInfo, nullptr, &instance->_debugUtilsUserCallback);
                if (result != VK_SUCCESS)
                    return result;
            }
        }
        else if (pCreateInfo->debugReport.callback != nullptr)
        {
            auto debugReportCallbackInstaller = PFN_vkCreateDebugReportCallbackEXT(vkGetInstanceProcAddr(instanceHandle, "vkCreateDebugReportCallbackEXT"));
            if (debugReportCallbackInstaller != nullptr)
            {
                VkDebugReportCallbackCreateInfoEXT debugCreateInfo{};
                debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
                debugCreateInfo.flags = pCreateInfo->debugReport.flags;
                debugCreateInfo.pfnCallback = pCreateInfo->debugReport.callback;

                result = debugReportCallbackInstaller(instanceHandle, &debugCreateInfo, nullptr, &instance->_debugReportCallback);
                if (result != VK_SUCCESS)
                    return result;
            }
        }

        instance->vkSetDebugUtilsObjectNameEXT = PFN_vkSetDebugUtilsObjectNameEXT(vkGetInstanceProcAddr(instanceHandle, "vkSetDebugUtilsObjectNameEXT"));
        if (instance->vkSetDebugUtilsObjectNameEXT == nullptr)
            instance->vkDebugMarkerSetObjectNameEXT = PFN_vkDebugMarkerSetObjectNameEXT(vkGetInstanceProcAddr(instanceHandle, "vkDebugMarkerSetObjectNameEXT"));
#endif

        *ppInstance = instance;
        return VK_SUCCESS;
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
