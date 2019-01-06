#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <cstdint>

#include <graphics/vulkan/VK.hpp>
#include <graphics/vulkan/Device.hpp>
#include <graphics/vulkan/Traits.hpp>

namespace shared::threading
{
    class thread_pool;
}

namespace gfx::vk
{
    using namespace shared::threading;

    class Instance
    {
    public:
        Instance(const InstanceCreateInfo* pCreateInfo);

        VkInstance GetHandle() const { return _handle; }
        std::vector<PhysicalDevice*> const& GetPhysicalDevices() const { return _physicalDevices; }
        thread_pool* GetThreadPool() const { return _threadPool; }

        Instance() { }
        ~Instance();

        Instance(const Instance&) = delete;
        Instance(Instance&&) = delete;

        template <typename T>
        VkResult SetObjectName(Device* device, uint64_t objectHandleValue, std::string_view objectName)
        {
#if _DEBUG
            if (vkSetDebugUtilsObjectNameEXT != nullptr)
            {
                VkDebugUtilsObjectNameInfoEXT nameInfo{};
                nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
                nameInfo.pObjectName = objectName.data();
                nameInfo.objectHandle = objectHandleValue;
                nameInfo.objectType = gfx::vk::traits<T>::object_type;

                return vkSetDebugUtilsObjectNameEXT(device->GetHandle(), &nameInfo);
            }
            else if (vkDebugMarkerSetObjectNameEXT != nullptr)
            {
                VkDebugMarkerObjectNameInfoEXT nameInfo{};
                nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT;
                nameInfo.pObjectName = objectName.data();
                nameInfo.objectType = gfx::vk::traits<T>::object_type_ext;
                nameInfo.object = uint64_t(objectHandleValue);

                return vkDebugMarkerSetObjectNameEXT(device->GetHandle(), &nameInfo);
            }
#endif
            return VK_ERROR_LAYER_NOT_PRESENT;
        }

    private:
        VkInstance _handle = VK_NULL_HANDLE;
        std::vector<PhysicalDevice*> _physicalDevices;
        thread_pool* _threadPool = nullptr;

        VkDebugUtilsMessengerEXT _debugUtilsUserCallback = nullptr;
        VkDebugReportCallbackEXT _debugReportCallback = nullptr;

        PFN_vkSetDebugUtilsObjectNameEXT vkSetDebugUtilsObjectNameEXT = nullptr;
        PFN_vkDebugMarkerSetObjectNameEXT vkDebugMarkerSetObjectNameEXT = nullptr;
    };
}
