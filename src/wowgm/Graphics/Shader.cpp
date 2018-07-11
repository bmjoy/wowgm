#include "Shader.hpp"
#include "LogicalDevice.hpp"

#include <fstream>
#include <vector>

namespace wowgm::graphics
{
    Shader::Shader(LogicalDevice* device, const std::string& fileName) : _device(device)
    {
        std::ifstream fs(fileName, std::ios::ate | std::ios::binary);
        if (!fs.is_open())
            throw std::runtime_error("Unable to open shader file!");

        std::size_t fileSize = fs.tellg();
        std::vector<char> byteCode(fileSize);

        fs.seekg(0);
        fs.read(byteCode.data(), fileSize);
        fs.close();

        VkShaderModuleCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = byteCode.size();
        // This works because the default allocator for std::vector already aligns to the worst case scenario.
        createInfo.pCode = reinterpret_cast<const uint32_t*>(byteCode.data());

        if (vkCreateShaderModule(*_device, &createInfo, nullptr, &_shaderModule) != VK_SUCCESS)
            throw std::runtime_error("Unable to create shader module");
    }

    Shader::~Shader()
    {
        vkDestroyShaderModule(*_device, _shaderModule, nullptr);
    }
}