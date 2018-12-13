#include "Reflection.hpp"
#include "VK.hpp"

#include <spirv_glsl.hpp>

namespace gfx::vk
{
    namespace SPIRV
    {
        class CustomCompiler : public spirv_cross::CompilerGLSL
        {
        public:
            CustomCompiler(const std::vector<uint32_t>& spirv) : spirv_cross::CompilerGLSL(spirv)
            {

            }

            VkAccessFlags GetAccessFlags(const spirv_cross::SPIRType& type)
            {
                // SPIRV-Cross hack to get the correct readonly and writeonly attributes on ssbos.
                // This isn't working correctly via Compiler::get_decoration(id, spv::DecorationNonReadable) for example.
                // So the code below is extracted from private methods within spirv_cross.cpp.
                // The spirv_cross executable correctly outputs the attributes when converting spirv back to GLSL,
                // but it's own reflection code does not :-(
                auto all_members_flag_mask = spirv_cross::Bitset(~0ULL);
                for (auto i = 0U; i < type.member_types.size(); ++i)
                    all_members_flag_mask.merge_and(get_member_decoration_bitset(type.self, i));

                auto base_flags = meta[type.self].decoration.decoration_flags;
                base_flags.merge_or(spirv_cross::Bitset(all_members_flag_mask));

                VkAccessFlags access = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
                if (base_flags.get(spv::DecorationNonReadable))
                    access = VK_ACCESS_SHADER_WRITE_BIT;
                else if (base_flags.get(spv::DecorationNonWritable))
                    access = VK_ACCESS_SHADER_READ_BIT;

                return access;
            }
        };

        bool ReflectResources(std::vector<uint32_t> const& spirvCode, VkShaderStageFlagBits shaderStage, std::vector<PipelineResource>& resourceContainer)
        {
            CustomCompiler compiler(spirvCode);
            spirv_cross::CompilerGLSL::Options opts = compiler.get_common_options();
            opts.enable_420pack_extension = true;
            compiler.set_common_options(opts);

            auto shaderResources = compiler.get_shader_resources();
            for (auto&& resource : shaderResources.stage_inputs)
            {
                const auto& typeInfo = compiler.get_type_from_variable(resource.id);

                PipelineResource pipelineResource{};
                pipelineResource.stages = shaderStage;
                pipelineResource.resourceType = PIPELINE_RESOURCE_TYPE_INPUT;
                pipelineResource.access = VK_ACCESS_SHADER_READ_BIT;
                pipelineResource.location = compiler.get_decoration(resource.id, spv::DecorationLocation);
                pipelineResource.vecSize = typeInfo.vecsize;
                pipelineResource.arraySize = (typeInfo.array.size() == 0) ? 1 : typeInfo.array[0];
                pipelineResource.columns = typeInfo.columns;
                switch (typeInfo.basetype)
                {
                    case spirv_cross::SPIRType::Int:
                        pipelineResource.baseType = PIPELINE_RESOURCE_BASE_TYPE_INT;
                        break;
                    case spirv_cross::SPIRType::UInt:
                        pipelineResource.baseType = PIPELINE_RESOURCE_BASE_TYPE_UINT;
                        break;
                    case spirv_cross::SPIRType::Float:
                        pipelineResource.baseType = PIPELINE_RESOURCE_BASE_TYPE_FLOAT;
                        break;
                    case spirv_cross::SPIRType::Double:
                        pipelineResource.baseType = PIPELINE_RESOURCE_BASE_TYPE_DOUBLE;
                        break;
                    default:
                        pipelineResource.baseType = PIPELINE_RESOURCE_BASE_TYPE_UNKNOWN;
                        break;
                }
                memcpy(pipelineResource.name, resource.name.c_str(), std::min(sizeof(pipelineResource.name), resource.name.length()));
                resourceContainer.push_back(pipelineResource);
            }

            for (auto&& resource : shaderResources.stage_outputs)
            {
                const auto& typeInfo = compiler.get_type_from_variable(resource.id);

                PipelineResource pipelineResource{};
                pipelineResource.stages = shaderStage;
                pipelineResource.resourceType = PIPELINE_RESOURCE_TYPE_OUTPUT;
                pipelineResource.access = VK_ACCESS_SHADER_WRITE_BIT;
                pipelineResource.location = compiler.get_decoration(resource.id, spv::DecorationLocation);
                pipelineResource.vecSize = typeInfo.vecsize;
                pipelineResource.arraySize = (typeInfo.array.size() == 0) ? 1 : typeInfo.array[0];
                pipelineResource.columns = typeInfo.columns;
                switch (typeInfo.basetype)
                {
                   case spirv_cross::SPIRType::Int:
                        pipelineResource.baseType = PIPELINE_RESOURCE_BASE_TYPE_INT;
                        break;
                    case spirv_cross::SPIRType::UInt:
                        pipelineResource.baseType = PIPELINE_RESOURCE_BASE_TYPE_UINT;
                        break;
                    case spirv_cross::SPIRType::Float:
                        pipelineResource.baseType = PIPELINE_RESOURCE_BASE_TYPE_FLOAT;
                        break;
                    case spirv_cross::SPIRType::Double:
                        pipelineResource.baseType = PIPELINE_RESOURCE_BASE_TYPE_DOUBLE;
                        break;
                    default:
                        pipelineResource.baseType = PIPELINE_RESOURCE_BASE_TYPE_UNKNOWN;
                        break;
                }
                memcpy(pipelineResource.name, resource.name.c_str(), std::min(sizeof(pipelineResource.name), resource.name.length()));
                resourceContainer.push_back(pipelineResource);
            }

            for (auto& resource : shaderResources.uniform_buffers)
            {
                const auto& typeInfo = compiler.get_type_from_variable(resource.id);

                // Get the size of the uniform buffer block.
                // TODO: drill through struct hiearchy to correctly compute size.
                uint32_t size = 0;
                for (auto i = 0U; i < typeInfo.member_types.size(); ++i)
                {
                    auto memberType = compiler.get_type(typeInfo.member_types[i]);
                    uint32_t typeSize = (memberType.width >> 3);
                    size += typeSize * memberType.vecsize * ((memberType.array.size() == 0) ? 1 : memberType.array[0]);
                }

                PipelineResource pipelineResource = {};
                pipelineResource.stages = shaderStage;
                pipelineResource.resourceType = PIPELINE_RESOURCE_TYPE_UNIFORM_BUFFER;
                pipelineResource.access = VK_ACCESS_UNIFORM_READ_BIT;
                pipelineResource.set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
                pipelineResource.binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
                pipelineResource.arraySize = (typeInfo.array.size() == 0) ? 1 : typeInfo.array[0];
                pipelineResource.size = size;
                pipelineResource.columns = typeInfo.columns;
                memcpy(pipelineResource.name, resource.name.c_str(), std::min(sizeof(pipelineResource.name), resource.name.length()));
                resourceContainer.push_back(pipelineResource);
            }

            for (auto& resource : shaderResources.storage_buffers)
            {
                const auto& typeInfo = compiler.get_type_from_variable(resource.id);

                // Get access flags for variable.
                auto access = compiler.GetAccessFlags(typeInfo);

                // Get the size of the storage buffer block.
                // TODO: drill through struct hierarchy to correctly compute size.
                uint32_t size = 0;
                for (auto i = 0U; i < typeInfo.member_types.size(); ++i)
                {
                    auto memberType = compiler.get_type(typeInfo.member_types[i]);
                    uint32_t typeSize = (memberType.width >> 3);
                    size += typeSize * memberType.vecsize * ((memberType.array.size() == 0) ? 1 : memberType.array[0]);
                }

                PipelineResource pipelineResource = {};
                pipelineResource.stages = shaderStage;
                pipelineResource.resourceType = PIPELINE_RESOURCE_TYPE_STORAGE_BUFFER;
                pipelineResource.access = access;
                pipelineResource.set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
                pipelineResource.binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
                pipelineResource.arraySize = (typeInfo.array.size() == 0) ? 1 : typeInfo.array[0];
                pipelineResource.size = size;
                pipelineResource.columns = typeInfo.columns;
                memcpy(pipelineResource.name, resource.name.c_str(), std::min(sizeof(pipelineResource.name), resource.name.length()));
                resourceContainer.push_back(pipelineResource);
            }

            for (auto& resource : shaderResources.separate_samplers)
            {
                const auto& typeInfo = compiler.get_type_from_variable(resource.id);

                PipelineResource pipelineResource = {};
                pipelineResource.stages = shaderStage;
                pipelineResource.resourceType = PIPELINE_RESOURCE_TYPE_SAMPLER;
                pipelineResource.access = VK_ACCESS_SHADER_READ_BIT;
                pipelineResource.set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
                pipelineResource.binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
                pipelineResource.arraySize = (typeInfo.array.size() == 0) ? 1 : typeInfo.array[0];
                pipelineResource.columns = typeInfo.columns;
                memcpy(pipelineResource.name, resource.name.c_str(), std::min(sizeof(pipelineResource.name), resource.name.length()));
                resourceContainer.push_back(pipelineResource);
            }

            for (auto& resource : shaderResources.sampled_images)
            {
                const auto& typeInfo = compiler.get_type_from_variable(resource.id);

                PipelineResource pipelineResource = {};
                pipelineResource.stages = shaderStage;
                pipelineResource.resourceType = (typeInfo.image.dim == spv::Dim::DimBuffer) ? PIPELINE_RESOURCE_TYPE_UNIFORM_TEXEL_BUFFER : PIPELINE_RESOURCE_TYPE_COMBINED_IMAGE_SAMPLER;
                pipelineResource.access = VK_ACCESS_SHADER_READ_BIT;
                pipelineResource.set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
                pipelineResource.binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
                pipelineResource.arraySize = (typeInfo.array.size() == 0) ? 1 : typeInfo.array[0];
                pipelineResource.columns = typeInfo.columns;
                memcpy(pipelineResource.name, resource.name.c_str(), std::min(sizeof(pipelineResource.name), resource.name.length()));
                resourceContainer.push_back(pipelineResource);
            }

            for (auto& resource : shaderResources.separate_images)
            {
                const auto& typeInfo = compiler.get_type_from_variable(resource.id);

                PipelineResource pipelineResource = {};
                pipelineResource.stages = shaderStage;
                pipelineResource.resourceType = PIPELINE_RESOURCE_TYPE_SAMPLED_IMAGE;
                pipelineResource.access = VK_ACCESS_SHADER_READ_BIT;
                pipelineResource.set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
                pipelineResource.binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
                pipelineResource.arraySize = (typeInfo.array.size() == 0) ? 1 : typeInfo.array[0];
                pipelineResource.columns = typeInfo.columns;
                memcpy(pipelineResource.name, resource.name.c_str(), std::min(sizeof(pipelineResource.name), resource.name.length()));
                resourceContainer.push_back(pipelineResource);
            }

            for (auto& resource : shaderResources.storage_images)
            {
                auto nonReadable = compiler.get_decoration(resource.id, spv::DecorationNonReadable);
                auto nonWriteable = compiler.get_decoration(resource.id, spv::DecorationNonWritable);
                VkAccessFlags access = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
                if (nonReadable) access = VK_ACCESS_SHADER_WRITE_BIT;
                else if (nonWriteable) access = VK_ACCESS_SHADER_READ_BIT;

                const auto& typeInfo = compiler.get_type_from_variable(resource.id);

                PipelineResource pipelineResource = {};
                pipelineResource.stages = shaderStage;
                pipelineResource.resourceType = (typeInfo.image.dim == spv::Dim::DimBuffer) ? PIPELINE_RESOURCE_TYPE_STORAGE_TEXEL_BUFFER : PIPELINE_RESOURCE_TYPE_STORAGE_IMAGE;
                pipelineResource.access = access;
                pipelineResource.set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
                pipelineResource.binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
                pipelineResource.arraySize = (typeInfo.array.size() == 0) ? 1 : typeInfo.array[0];
                pipelineResource.columns = typeInfo.columns;
                memcpy(pipelineResource.name, resource.name.c_str(), std::min(sizeof(pipelineResource.name), resource.name.length()));
                resourceContainer.push_back(pipelineResource);
            }

            for (auto& resource : shaderResources.subpass_inputs)
            {
                PipelineResource pipelineResource = {};
                pipelineResource.resourceType = PIPELINE_RESOURCE_TYPE_INPUT_ATTACHMENT;
                pipelineResource.stages = VK_SHADER_STAGE_FRAGMENT_BIT;
                pipelineResource.access = VK_ACCESS_SHADER_READ_BIT;
                pipelineResource.inputAttachmentIndex = compiler.get_decoration(resource.id, spv::DecorationInputAttachmentIndex);
                pipelineResource.set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
                pipelineResource.binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
                pipelineResource.arraySize = 1;
                memcpy(pipelineResource.name, resource.name.c_str(), std::min(sizeof(pipelineResource.name), resource.name.length()));
                resourceContainer.push_back(pipelineResource);
            }

            for (auto& resource : shaderResources.push_constant_buffers)
            {
                const auto& typeInfo = compiler.get_type_from_variable(resource.id);

                // Get the offset and size of the given push constant buffer.
                uint32_t offset = ~0;
                uint32_t size = 0;
                for (auto i = 0U; i < typeInfo.member_types.size(); ++i)
                {
                    auto memberType = compiler.get_type(typeInfo.member_types[i]);
                    uint32_t typeSize = (memberType.width >> 3);
                    offset = std::min(offset, compiler.get_member_decoration(typeInfo.self, i, spv::DecorationOffset));
                    size += typeSize * memberType.vecsize * memberType.columns * ((memberType.array.size() == 0) ? 1 : memberType.array[0]);
                }

                PipelineResource pipelineResource = {};
                pipelineResource.stages = shaderStage;
                pipelineResource.resourceType = PIPELINE_RESOURCE_TYPE_PUSH_CONSTANT_BUFFER;
                pipelineResource.access = VK_ACCESS_SHADER_READ_BIT;
                pipelineResource.offset = offset;
                pipelineResource.size = size;
                pipelineResource.columns = typeInfo.columns;
                memcpy(pipelineResource.name, resource.name.c_str(), std::min(sizeof(pipelineResource.name), resource.name.length()));
                resourceContainer.push_back(pipelineResource);
            }

            return true;
        }
    }
}