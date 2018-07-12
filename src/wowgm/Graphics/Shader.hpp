#pragma once

#include <vulkan/vulkan.h>

#include <string>
#include <boost/filesystem/fstream.hpp>

namespace wowgm::graphics
{
    class LogicalDevice;

    class Shader
    {
    public:
        static Shader* CreateVertexShader(LogicalDevice* device, const std::string& stageName, const std::string& fileName);
        static Shader* CreateFragmentShader(LogicalDevice* device, const std::string& stageName, const std::string& fileName);
        static Shader* CreateGeometryShader(LogicalDevice* device, const std::string& stageName, const std::string& fileName);

        Shader(LogicalDevice* device, VkShaderStageFlagBits stage, const std::string& stageName, const std::string& fileName);
        ~Shader();

        Shader(Shader const&) = delete;

        VkShaderModule GetVkShaderModule() { return _shaderModule; }
        VkPipelineShaderStageCreateInfo GetVkShaderStageInfo() { return _shaderStageInfo; }

        operator VkShaderModule() const { return _shaderModule; }

        LogicalDevice* GetLogicalDevice();

    private:
        LogicalDevice* _logicalDevice;

        VkShaderModule _shaderModule;
        VkPipelineShaderStageCreateInfo _shaderStageInfo;
    };
}
