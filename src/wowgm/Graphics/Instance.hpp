#pragma once

#include "SharedGraphicsDefines.hpp"

#include "LogicalDevice.hpp"
#include "PhysicalDevice.hpp"

#include <vector>
#include <cstdint>

#include <vulkan/vulkan.h>

namespace wowgm::graphics
{
    class Instance
    {
    public:
        static Instance* Create(const char* applicationName, const char* engineName);

        Instance(VkInstance instance);
        ~Instance();

        void SetSurface(VkSurfaceKHR surface);

        void SetupDebugCallback();

        LogicalDevice& GetLogicalDevice(std::uint32_t index);
        PhysicalDevice& GetPhysicalDevice(std::uint32_t index);

        std::vector<LogicalDevice>::iterator IterateLogicalDevices();
        std::vector<PhysicalDevice>::iterator IteratePhysicalDevices();

        void SelectPhysicalDevice(std::uint32_t deviceIndex);

    private:
        void SelectPhysicalDevice();

    private:
        std::vector<LogicalDevice> _logicalDevices;
        std::vector<PhysicalDevice> _physicalDevices;

        VkInstance _instance;
        VkSurfaceKHR _surface;

        PhysicalDevice _selectedPhysicalDevice;

#ifdef ENABLE_VALIDATION_LAYERS
        VkDebugReportCallbackEXT _debugReportCallback;
#endif
    };
}
