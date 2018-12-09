#pragma once

#include "VEZ.hpp"
#include <cstdint>
#include <string_view>

namespace vez
{
    class Queue
    {
    public:
        Queue(Device* device, VkQueue queue, std::uint32_t familyIndex, std::uint32_t index, const VkQueueFamilyProperties& properties);

        Device* GetDevice() const { return _device; }
        VkQueue GetHandle() const { return _queue; }
        std::uint32_t GetFamilyIndex() const { return _familyIndex; }
        std::uint32_t GetIndex() const { return _index; }
        VkQueueFlags GetFlags() const { return _properties.queueFlags; }

        // VkResult Submit(std::uint32_t submitCount, ...);
        VkResult WaitIdle();
        VkResult Present(const PresentInfo* pPresentInfo);
        VkResult Submit(std::uint32_t submitCount, const SubmitInfo* pSubmits, VkFence* pFence);

        VkResult BeginLabel(std::string_view label, float (&color)[4]);
        VkResult EndLabel();
        VkResult InsertLabel(std::string_view label, float (&color)[4]);

    private:
        VkResult _InitializeLabelPointers();
    private:
        Device* _device = nullptr;
        VkQueue _queue = VK_NULL_HANDLE;
        std::uint32_t _familyIndex = 0;
        std::uint32_t _index = 0;
        VkQueueFamilyProperties _properties;

        static PFN_vkQueueEndDebugUtilsLabelEXT _vkQueueEndDebugUtilsLabelEXT;
        static PFN_vkQueueBeginDebugUtilsLabelEXT _vkQueueBeginDebugUtilsLabelEXT;
        static PFN_vkQueueInsertDebugUtilsLabelEXT _vkQueueInsertDebugUtilsLabelEXT;
    };
}