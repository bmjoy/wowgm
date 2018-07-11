#include "Shader.hpp"
#include "LogicalDevice.hpp"

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>

#include <vector>

namespace wowgm::graphics
{
    Shader::Shader(LogicalDevice* device, VkShaderStageFlagBits stage, const std::string& stageName, const std::string& fileName) : _logicalDevice(device)
    {
        std::ifstream fs(fileName, std::ios::binary);
        if (!fs.is_open())
            throw std::runtime_error("Unable to open shader file!");

        std::size_t fileSize = boost::filesystem::file_size(fileName);
        std::vector<char> byteCode(fileSize);

        fs.seekg(0);
        fs.read(byteCode.data(), fileSize);
        fs.close();

        VkShaderModuleCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = byteCode.size();
        // This works because the default allocator for std::vector already aligns to the worst case scenario.
        createInfo.pCode = reinterpret_cast<const uint32_t*>(byteCode.data());

        if (vkCreateShaderModule(*_logicalDevice, &createInfo, nullptr, &_shaderModule) != VK_SUCCESS)
            throw std::runtime_error("Unable to create shader module");

        _shaderStageInfo = { };
        _shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        _shaderStageInfo.stage = stage;
        _shaderStageInfo.module = _shaderModule;
        _shaderStageInfo.pName = stageName.c_str();
    }

    Shader::~Shader()
    {
        vkDestroyShaderModule(*_logicalDevice, _shaderModule, nullptr);
    }

    Shader* Shader::CreateVertexShader(LogicalDevice* device, const std::string& stageName, const std::string& fileName)
    {
        return new Shader(device, VK_SHADER_STAGE_VERTEX_BIT, stageName, fileName);
    }

    Shader* Shader::CreateFragmentShader(LogicalDevice* device, const std::string& stageName, const std::string& fileName)
    {
        return new Shader(device, VK_SHADER_STAGE_FRAGMENT_BIT, stageName, fileName);
    }

    Shader* Shader::CreateGeometryShader(LogicalDevice* device, const std::string& stageName, const std::string& fileName)
    {
        return new Shader(device, VK_SHADER_STAGE_GEOMETRY_BIT, stageName, fileName);
    }

    LogicalDevice* Shader::GetLogicalDevice()
    {
        return _logicalDevice;
    }
}
