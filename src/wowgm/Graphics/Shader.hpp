#pragma once

#include <vulkan/vulkan.h>

#include <string>
#include <boost/filesystem/fstream.hpp>

namespace wowgm::graphics
{
    class LogicalDevice;

    class Shader
    {
        Shader(Shader&&) = delete;
        Shader(Shader const&) = delete;

    public:
        static Shader* CreateVertexShader(LogicalDevice* device, const std::string& entryPointName, const std::string& fileName);
        static Shader* CreateFragmentShader(LogicalDevice* device, const std::string& entryPointName, const std::string& fileName);
        static Shader* CreateGeometryShader(LogicalDevice* device, const std::string& entryPointName, const std::string& fileName);

        Shader(LogicalDevice* device, VkShaderStageFlagBits stage, const std::string& entryPointName, const std::string& fileName);
        ~Shader();

        VkShaderModule GetVkShaderModule() { return _shaderModule; }
        VkPipelineShaderStageCreateInfo& GetVkShaderStageInfo() { return _shaderStageInfo; }

        operator VkShaderModule() const { return _shaderModule; }

        LogicalDevice* GetLogicalDevice();

    private:
        LogicalDevice* _logicalDevice;

        std::string _name;

        VkShaderModule _shaderModule;
        VkPipelineShaderStageCreateInfo _shaderStageInfo;
    };
}
