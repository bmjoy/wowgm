#include "Queue.hpp"
#include "Device.hpp"
#include "PhysicalDevice.hpp"
#include "Instance.hpp"

#include <mutex>
#include <thread>

namespace vez
{
    Queue::Queue(Device* device, VkQueue queue, std::uint32_t familyIndex, std::uint32_t index, const VkQueueFamilyProperties& properties)
    {
        _device = device;
        _queue = queue;
        _familyIndex = familyIndex;
        _index = index;
        _properties = properties;
    }

    VkResult Queue::WaitIdle()
    {
        return vkQueueWaitIdle(GetHandle());
    }

    VkResult Queue::_InitializeLabelPointers()
    {
        static std::once_flag onceFlag;
        std::call_once(onceFlag, [&](VkInstance instance) -> void {
            _vkQueueEndDebugUtilsLabelEXT    = PFN_vkQueueEndDebugUtilsLabelEXT(vkGetInstanceProcAddr(instance, "vkQueueEndDebugUtilsLabelEXT"));
            _vkQueueBeginDebugUtilsLabelEXT  = PFN_vkQueueBeginDebugUtilsLabelEXT(vkGetInstanceProcAddr(instance, "vkQueueBeginDebugUtilsLabelEXT"));
            _vkQueueInsertDebugUtilsLabelEXT = PFN_vkQueueInsertDebugUtilsLabelEXT(vkGetInstanceProcAddr(instance, "vkQueueInsertDebugUtilsLabelEXT"));
        }, _device->GetPhysicalDevice()->GetInstance()->GetHandle());

        if (vkQueueEndDebugUtilsLabelEXT == nullptr)
            return VK_ERROR_LAYER_NOT_PRESENT;

        return VK_SUCCESS;
    }

    VkResult Queue::BeginLabel(std::string_view label, float (&color)[4])
    {
        if (_vkQueueBeginDebugUtilsLabelEXT != nullptr)
        {
            VkDebugUtilsLabelEXT labelInfo{};
            labelInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
            memcpy(labelInfo.color, color, sizeof(color));
            labelInfo.pLabelName = label.data();

            _vkQueueBeginDebugUtilsLabelEXT(_queue, &labelInfo);
            return VK_SUCCESS;
        }

        return VK_ERROR_LAYER_NOT_PRESENT;
    }

    VkResult Queue::EndLabel()
    {
        if (_vkQueueEndDebugUtilsLabelEXT != nullptr)
        {
            _vkQueueEndDebugUtilsLabelEXT(_queue);
            return VK_SUCCESS;
        }

        return VK_ERROR_LAYER_NOT_PRESENT;
    }

    VkResult Queue::InsertLabel(std::string_view label, float (&color)[4])
    {
        if (_vkQueueInsertDebugUtilsLabelEXT != nullptr)
        {
            VkDebugUtilsLabelEXT labelInfo{};
            labelInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
            memcpy(labelInfo.color, color, sizeof(color));
            labelInfo.pLabelName = label.data();

            _vkQueueInsertDebugUtilsLabelEXT(_queue, &labelInfo);
            return VK_SUCCESS;
        }

        return VK_ERROR_LAYER_NOT_PRESENT;
    }
}
