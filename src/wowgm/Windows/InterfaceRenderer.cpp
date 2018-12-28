#include "InterfaceRenderer.hpp"

#include <graphics/vulkan/Device.hpp>
#include <graphics/vulkan/Swapchain.hpp>
#include <graphics/vulkan/RenderPass.hpp>
#include <graphics/vulkan/Shader.hpp>
#include <graphics/vulkan/Pipeline.hpp>

namespace wowgm
{
    using namespace gfx::vk;

    InterfaceRenderer::InterfaceRenderer(Swapchain* swapchain) : Renderer(swapchain)
    {
        _renderPass = new RenderPass(GetDevice());

        uint32_t attachmentIndex = _renderPass->AddAttachment(AttachmentType::Color, VkAttachmentDescription{
            0,                                          // flags
            GetSwapchain()->GetFormat().format,         // format
            VK_SAMPLE_COUNT_1_BIT,                      // samples
            VK_ATTACHMENT_LOAD_OP_CLEAR,                // loadOp
            VK_ATTACHMENT_STORE_OP_STORE,               // storeOp
            VK_ATTACHMENT_LOAD_OP_DONT_CARE,            // stencilLoadOp
            VK_ATTACHMENT_STORE_OP_DONT_CARE,           // stencilStoreOp
            VK_IMAGE_LAYOUT_UNDEFINED,                  // initialLayout
            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,            // finalLayout
        });

        _renderPass->BeginSubpass();
        _renderPass->AddAttachmentReference(gfx::vk::AttachmentType::Color, attachmentIndex, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
        _renderPass->FinalizeSubpass();

        Shader* fragmentShader = Shader::FromDisk(GetDevice(), "./resources/shaders/frag.spv", "main", VK_SHADER_STAGE_FRAGMENT_BIT);
        Shader* vertexShader = Shader::FromDisk(GetDevice(), "./resources/shaders/vert.spv", "main", VK_SHADER_STAGE_VERTEX_BIT);

        GraphicsPipelineCreateInfo pipelineCreateInfo;
        pipelineCreateInfo.renderPass = _renderPass;
        pipelineCreateInfo.shaders.push_back(fragmentShader);
        pipelineCreateInfo.shaders.push_back(vertexShader);

        // Our viewport - and the scissors
        pipelineCreateInfo.viewportState.viewports.push_back({
            0.0f,
            0.0f,
            float(GetSwapchain()->GetExtent().width),
            float(GetSwapchain()->GetExtent().height),
            0.0f,
            1.0f
        });
        pipelineCreateInfo.viewportState.scissors.push_back({
            { 0, 0 },
            GetSwapchain()->GetExtent()
        });

        /// Rasterization
        pipelineCreateInfo.rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;
        pipelineCreateInfo.rasterizationState.frontFace = VK_FRONT_FACE_CLOCKWISE;
        pipelineCreateInfo.rasterizationState.depthBiasEnable = VK_FALSE;
        pipelineCreateInfo.rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
        pipelineCreateInfo.rasterizationState.lineWidth = 1.0f;
        pipelineCreateInfo.rasterizationState.depthClampEnable = VK_FALSE;

        /// Multisampling
        pipelineCreateInfo.multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        pipelineCreateInfo.multisampleState.sampleShadingEnable = VK_FALSE;
        /// Color blend
        pipelineCreateInfo.colorBlendState.logicOpEnable = VK_FALSE;
        pipelineCreateInfo.colorBlendState.logicOp = VK_LOGIC_OP_COPY;
        pipelineCreateInfo.colorBlendState.attachments.push_back(VkPipelineColorBlendAttachmentState{
            VK_FALSE,                                                                                                   // VkBool32                 blendEnable;
            VK_BLEND_FACTOR_ZERO,                                                                                       // VkBlendFactor            srcColorBlendFactor;
            VK_BLEND_FACTOR_ZERO,                                                                                       // VkBlendFactor            dstColorBlendFactor;
            VK_BLEND_OP_ADD,                                                                                            // VkBlendOp                colorBlendOp;
            VK_BLEND_FACTOR_ZERO,                                                                                       // VkBlendFactor            srcAlphaBlendFactor;
            VK_BLEND_FACTOR_ZERO,                                                                                       // VkBlendFactor            dstAlphaBlendFactor;
            VK_BLEND_OP_ADD,                                                                                            // VkBlendOp                alphaBlendOp;
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,  // VkColorComponentFlags    colorWriteMask;
        });

        VkResult result = Pipeline::Create(GetDevice(), &pipelineCreateInfo, &_pipeline);
        BOOST_ASSERT_MSG(result == VK_SUCCESS, "Unable to create a pipeline");

        delete fragmentShader;
        delete vertexShader;
    }

    InterfaceRenderer::~InterfaceRenderer()
    {
        // Deletes the pipeline, its layout, and the render pass.
        delete _pipeline;
    }

    void InterfaceRenderer::Render()
    {

    }

    Pipeline* InterfaceRenderer::GetPipeline() const
    {
        return _pipeline;
    }
}