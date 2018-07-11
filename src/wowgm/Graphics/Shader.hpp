#pragma once

#include <vulkan/vulkan.h>

#include <iostream>
#include <string>

namespace wowgm::graphics
{
    class LogicalDevice;

    class Shader
    {
    public:
        Shader(LogicalDevice* device, const std::string& fileName);
        ~Shader();

        VkShaderModule GetVkShaderModule() { return _shaderModule; }
        operator VkShaderModule() const { return _shaderModule; }

        LogicalDevice* GetLogicalDevice();

    private:
        LogicalDevice* _device;

        VkShaderModule _shaderModule;
    };
}
