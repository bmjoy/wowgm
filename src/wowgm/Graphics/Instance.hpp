#pragma once

#include "SharedGraphicsDefines.hpp"

#include <vector>
#include <cstdint>

#include <vulkan/vulkan.h>

class GLFWwindow;

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

        VkInstance GetInstance();

        void SetupDebugCallback();

        PhysicalDevice& GetPhysicalDevice(std::uint32_t index);

        std::vector<PhysicalDevice>::iterator IteratePhysicalDevices();
        void SelectPhysicalDevice(std::uint32_t deviceIndex);

        Surface* CreateSurface(GLFWwindow* window);

        LogicalDevice* CreateLogicalDevice();
    private:
        void SelectPhysicalDevice();

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
