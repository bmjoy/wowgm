#pragma once

#include "SharedGraphicsDefines.hpp"
#include "Window.hpp"

#include <vector>
#include <cstdint>
#include <memory>

#include <vulkan/vulkan.h>

namespace wowgm::graphics
{
    class Surface;
    class Queue;
    class LogicalDevice;
    class PhysicalDevice;
    class Semaphore;
    class CommandBuffer;

    class Instance
    {
    public:
        Instance(VkInstance instance);
        static std::unique_ptr<Instance> Create(const char* applicationName, const char* engineName);
        ~Instance();

        Instance(Instance&&) = delete;
        Instance(Instance const&) = delete;

        void SetupDebugCallback();

        PhysicalDevice* GetPhysicalDevice(std::uint32_t index);
        PhysicalDevice* GetPhysicalDevice();

        Surface* CreateSurface(Window* window);

        LogicalDevice* CreateLogicalDevice();
        LogicalDevice* GetLogicalDevice();

        operator VkInstance() const { return _instance; }

    private:
        void _SelectPhysicalDevice();

    private:
        LogicalDevice* _logicalDevice;

        std::uint32_t _selectedPhysicalDevice;
        std::vector<std::unique_ptr<PhysicalDevice>> _physicalDevices;

        VkInstance _instance;
        Surface* _surface;

#ifdef ENABLE_VALIDATION_LAYERS
        VkDebugReportCallbackEXT _debugReportCallback;
#endif

    };
}
