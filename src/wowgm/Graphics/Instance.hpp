#pragma once

#include "SharedGraphicsDefines.hpp"
#include "Window.hpp"

#include <vector>
#include <cstdint>
#include <memory>
#include <string>
#include <sstream>
#include <vulkan/vulkan.h>


namespace wowgm::graphics
{
    class Surface;
    class Queue;
    class LogicalDevice;
    class PhysicalDevice;
    class Semaphore;
    class CommandBuffer;
    class SwapChain;

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
        SwapChain* CreateSwapChain();

        LogicalDevice* GetLogicalDevice();

        operator VkInstance() const { return _instance; }

        std::string ToString();
        void ToString(std::stringstream& ss);

    private:
        void _SelectPhysicalDevice();

    private:
        VkInstance _instance;

        LogicalDevice* _logicalDevice;
        std::vector<Surface*> _ownedSurfaces;

#ifdef ENABLE_VALIDATION_LAYERS
        VkDebugReportCallbackEXT _debugReportCallback;
#endif

        std::uint32_t _selectedPhysicalDevice;
        std::vector<std::unique_ptr<PhysicalDevice>> _physicalDevices;
    };
}
