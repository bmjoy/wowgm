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
        Instance(Instance&&) = delete;
        Instance(const Instance&) = delete;

        struct ctor_tag { };

    public:
        static std::unique_ptr<Instance> Create(const char* applicationName, const char* engineName);

        explicit Instance(ctor_tag, VkInstance instance);

        ~Instance();

        void SetupDebugCallback();

        PhysicalDevice* GetPhysicalDevice();

        Surface* CreateSurface(Window* window);

        LogicalDevice* CreateLogicalDevice();

        LogicalDevice* GetLogicalDevice();

        operator VkInstance() const { return _instance; }

    private:
        void _SelectPhysicalDevice();

    private:
        VkInstance _instance;

        LogicalDevice* _logicalDevice;
        Surface* _surface;

#ifdef ENABLE_VALIDATION_LAYERS
        VkDebugReportCallbackEXT _debugReportCallback;
#endif

        std::uint32_t _selectedPhysicalDevice;
        std::vector<std::unique_ptr<PhysicalDevice>> _physicalDevices;
    };
}
