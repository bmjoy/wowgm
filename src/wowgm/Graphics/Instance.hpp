#pragma once

#include "SharedGraphicsDefines.hpp"
#include "Window.hpp"

#include <vector>
#include <cstdint>

#include <vulkan/vulkan.h>

namespace wowgm::graphics
{
    class Surface;
    class Queue;
    class LogicalDevice;
    class PhysicalDevice;

    class Instance
    {
    public:
        static Instance* Create(const char* applicationName, const char* engineName);

        Instance(VkInstance instance);
        ~Instance();

        VkInstance GetVkInstance();

        void SetupDebugCallback();

        PhysicalDevice& GetPhysicalDevice(std::uint32_t index);
        PhysicalDevice* GetSelectedPhysicalDevice();

        std::vector<PhysicalDevice>::iterator IteratePhysicalDevices();
        void SelectPhysicalDevice(std::uint32_t deviceIndex);

        Surface* CreateSurface(Window* window);

        LogicalDevice* CreateLogicalDevice();
        LogicalDevice* GetLogicalDevice();

        operator VkInstance() const { return _instance; }

    private:
        void _SelectPhysicalDevice();

    private:
        LogicalDevice* _logicalDevice;
        PhysicalDevice* _selectedPhysicalDevice;

        std::vector<PhysicalDevice> _physicalDevices;

        VkInstance _instance;
        Surface* _surface;

#ifdef ENABLE_VALIDATION_LAYERS
        VkDebugReportCallbackEXT _debugReportCallback;
#endif
    };
}
