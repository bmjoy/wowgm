#include "shaders.hpp"
#include "FileSystem.hpp"

using namespace wowgm::filesystem;

vk::ShaderModule vks::shaders::loadShaderModule(const vk::Device& device, const std::string& fileName)
{
    vk::ShaderModule result;
    {
        auto file = DiskFileSystem::Instance()->OpenFile(fileName, LoadStrategy::Memory);
        result = device.createShaderModule({ {}, file->GetFileSize(), reinterpret_cast<const uint32_t*>(file->GetData()) });
    }
    return result;
}

// Load a SPIR-V shader
vk::PipelineShaderStageCreateInfo vks::shaders::loadShader(const vk::Device& device,
                                                           const std::string& fileName,
                                                           vk::ShaderStageFlagBits stage,
                                                           const char* entryPoint) {
    vk::PipelineShaderStageCreateInfo shaderStage;
    shaderStage.stage = stage;
    shaderStage.module = loadShaderModule(device, fileName);
    shaderStage.pName = entryPoint;
    return shaderStage;
}
