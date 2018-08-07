/*
* UI overlay class using ImGui
*
* Copyright (C) 2017 by Sascha Willems - www.saschawillems.de
*
* This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/

#include "UI.hpp"

#include <imgui.h>

#include "Helpers.hpp"
#include "Pipelines.hpp"
#include "Utils.hpp"

using namespace vkx;
using namespace vkx::ui;

void UIOverlay::create(const UIOverlayCreateInfo& createInfo)
{
    this->createInfo = createInfo;

    // Init ImGui
    // Color scheme
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowPadding = ImVec2(15, 15);
    style.WindowRounding = 5.0f;
    style.FramePadding = ImVec2(5, 5);
    style.FrameRounding = 4.0f;
    style.ItemSpacing = ImVec2(12, 8);
    style.ItemInnerSpacing = ImVec2(8, 6);
    style.IndentSpacing = 25.0f;
    style.ScrollbarSize = 15.0f;
    style.ScrollbarRounding = 9.0f;
    style.GrabMinSize = 5.0f;
    style.WindowBorderSize = 1.0f;
    style.GrabRounding = 3.0f;

    style.Colors[ImGuiCol_Text] = ImVec4(0.80f, 0.80f, 0.83f, 1.00f);
    style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.05f, 0.07f, 0.90f);
    style.Colors[ImGuiCol_ChildWindowBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
    style.Colors[ImGuiCol_PopupBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
    style.Colors[ImGuiCol_Border] = ImVec4(0.06f, 0.05f, 0.07f, 0.88f);
    style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.92f, 0.91f, 0.88f, 0.00f);
    style.Colors[ImGuiCol_FrameBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
    style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
    style.Colors[ImGuiCol_TitleBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
    style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.00f, 0.98f, 0.95f, 0.75f);
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
    style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
    style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
    style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
    style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
    style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
    // style.Colors[ImGuiCol_ComboBg] = ImVec4(0.19f, 0.18f, 0.21f, 1.00f); // Merged with PopupBg
    style.Colors[ImGuiCol_CheckMark] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
    style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
    style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
    style.Colors[ImGuiCol_Button] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
    style.Colors[ImGuiCol_Header] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
    style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
    style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
    style.Colors[ImGuiCol_Column] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
    style.Colors[ImGuiCol_ColumnHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
    style.Colors[ImGuiCol_ColumnActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
    style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
    style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
    // style->Colors[ImGuiCol_CloseButton] = ImVec4(0.40f, 0.39f, 0.38f, 0.16f); // Now uses regular button colors
    // style->Colors[ImGuiCol_CloseButtonHovered] = ImVec4(0.40f, 0.39f, 0.38f, 0.39f);
    // style->Colors[ImGuiCol_CloseButtonActive] = ImVec4(0.40f, 0.39f, 0.38f, 1.00f);
    style.Colors[ImGuiCol_PlotLines] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
    style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
    style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
    style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
    style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.25f, 1.00f, 0.00f, 0.43f);
    style.Colors[ImGuiCol_ModalWindowDarkening] = ImVec4(1.00f, 0.98f, 0.95f, 0.73f);

    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->AddFontFromFileTTF("./resources/fonts/Ruda-Bold.ttf", 15.6f);
    io.Fonts->AddFontFromFileTTF("./resources/fonts/Ruda-Bold.ttf", 13.0f);
    io.Fonts->AddFontFromFileTTF("./resources/fonts/Ruda-Bold.ttf", 18.2f);
    io.Fonts->AddFontFromFileTTF("./resources/fonts/Ruda-Bold.ttf", 23.4f);

    io.DisplaySize = ImVec2((float)createInfo.size.width, (float)createInfo.size.height);
    io.FontGlobalScale = scale;

    prepareResources();
    if (createInfo.renderPass)
        renderPass = createInfo.renderPass;
    else
        prepareRenderPass();
    preparePipeline();
}

/** Free up all Vulkan resources acquired by the UI overlay */
UIOverlay::~UIOverlay()
{
}

void UIOverlay::destroy()
{
    if (commandPool)
    {
        vertexBuffer.destroy();
        indexBuffer.destroy();
        font.destroy();
        context.device.destroyDescriptorSetLayout(descriptorSetLayout);
        context.device.destroyDescriptorPool(descriptorPool);
        context.device.destroyPipelineLayout(pipelineLayout);
        context.device.destroyPipeline(pipeline);
        if (!createInfo.renderPass)
            context.device.destroyRenderPass(renderPass);
        context.device.freeCommandBuffers(commandPool, cmdBuffers);
        context.device.destroyCommandPool(commandPool);
        context.device.destroyFence(fence);
    }
}

/** Prepare all vulkan resources required to render the UI overlay */
void UIOverlay::prepareResources()
{
    ImGuiIO& io = ImGui::GetIO();

    // Create font texture
    std::vector<uint8_t> fontData;
    int texWidth, texHeight;
    {
        unsigned char* fontBuffer;
        io.Fonts->GetTexDataAsRGBA32(&fontBuffer, &texWidth, &texHeight);
        vk::DeviceSize uploadSize = texWidth * texHeight * 4 * sizeof(char);
        fontData.resize(uploadSize);
        memcpy(fontData.data(), fontBuffer, uploadSize);
    }

    // Create target image for copy
    vk::ImageCreateInfo imageInfo;
    imageInfo.imageType = vk::ImageType::e2D;
    imageInfo.format = vk::Format::eR8G8B8A8Unorm;
    imageInfo.extent.width = texWidth;
    imageInfo.extent.height = texHeight;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.usage = vk::ImageUsageFlagBits::eSampled;

    font = context.stageToDeviceImage(imageInfo, fontData);

    // Image view
    vk::ImageViewCreateInfo viewInfo;
    viewInfo.image = font.image;
    viewInfo.viewType = vk::ImageViewType::e2D;
    viewInfo.format = vk::Format::eR8G8B8A8Unorm;
    viewInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.layerCount = 1;
    font.view = context.device.createImageView(viewInfo);

    // Font texture Sampler
    vk::SamplerCreateInfo samplerInfo;
    samplerInfo.magFilter = vk::Filter::eLinear;
    samplerInfo.minFilter = vk::Filter::eLinear;
    samplerInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
    samplerInfo.addressModeU = vk::SamplerAddressMode::eClampToEdge;
    samplerInfo.addressModeV = vk::SamplerAddressMode::eClampToEdge;
    samplerInfo.addressModeW = vk::SamplerAddressMode::eClampToEdge;
    samplerInfo.borderColor = vk::BorderColor::eFloatOpaqueWhite;
    font.sampler = context.device.createSampler(samplerInfo);

    // Command buffer

    vk::CommandPoolCreateInfo cmdPoolInfo;
    cmdPoolInfo.queueFamilyIndex = context.queueIndices.graphics;
    cmdPoolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
    commandPool = context.device.createCommandPool(cmdPoolInfo);

    // Descriptor pool
    vk::DescriptorPoolSize poolSize;
    poolSize.type = vk::DescriptorType::eCombinedImageSampler;
    poolSize.descriptorCount = 1;
    descriptorPool = context.device.createDescriptorPool({ {}, 2, 1, &poolSize });

    // Descriptor set layout
    vk::DescriptorSetLayoutBinding setLayoutBinding{ 0, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment };

    descriptorSetLayout = context.device.createDescriptorSetLayout({ {}, 1, &setLayoutBinding });

    // Descriptor set
    vk::DescriptorSetAllocateInfo allocInfo;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.pSetLayouts = &descriptorSetLayout;
    allocInfo.descriptorSetCount = 1;
    descriptorSet = context.device.allocateDescriptorSets(allocInfo)[0];

    vk::DescriptorImageInfo fontDescriptor;
    fontDescriptor.imageView = font.view;
    fontDescriptor.sampler = font.sampler;
    fontDescriptor.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
    vk::WriteDescriptorSet writeDescriptorSet;
    writeDescriptorSet.descriptorType = vk::DescriptorType::eCombinedImageSampler;
    writeDescriptorSet.pImageInfo = &fontDescriptor;
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.dstSet = descriptorSet;
    context.device.updateDescriptorSets(writeDescriptorSet, {});

    // Pipeline layout
    // Push constants for UI rendering parameters
    vk::PushConstantRange pushConstantRange{ vk::ShaderStageFlagBits::eVertex, 0, sizeof(PushConstBlock) };
    vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo{ {}, 1, &descriptorSetLayout, 1, &pushConstantRange };
    pipelineLayout = context.device.createPipelineLayout(pipelineLayoutCreateInfo);

    // Command buffer execution fence
    fence = context.device.createFence(vk::FenceCreateInfo{});
}

/** Prepare a separate pipeline for the UI overlay rendering decoupled from the main application */
void UIOverlay::preparePipeline()
{
    // Setup graphics pipeline for UI rendering
    vks::pipelines::GraphicsPipelineBuilder pipelineBuilder(context.device, pipelineLayout, renderPass);
    pipelineBuilder.depthStencilState = { false };
    pipelineBuilder.rasterizationState.cullMode = vk::CullModeFlagBits::eNone;

    // Enable blending
    pipelineBuilder.colorBlendState.blendAttachmentStates.resize(createInfo.attachmentCount);
    for (uint32_t i = 0; i < createInfo.attachmentCount; i++)
    {
        auto& blendAttachmentState = pipelineBuilder.colorBlendState.blendAttachmentStates[i];
        blendAttachmentState.blendEnable = VK_TRUE;
        blendAttachmentState.srcColorBlendFactor = vk::BlendFactor::eSrcAlpha;
        blendAttachmentState.dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
        blendAttachmentState.srcAlphaBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
        blendAttachmentState.dstAlphaBlendFactor = vk::BlendFactor::eZero;
    }

    pipelineBuilder.multisampleState.rasterizationSamples = createInfo.rasterizationSamples;

    // Load default shaders if not specified by example
    if (!createInfo.shaders.empty())
        pipelineBuilder.shaderStages = createInfo.shaders;
    else
    {
        pipelineBuilder.loadShader(vkx::getAssetPath() + "shaders/imgui/vert.spv", vk::ShaderStageFlagBits::eVertex);
        pipelineBuilder.loadShader(vkx::getAssetPath() + "shaders/imgui/frag.spv", vk::ShaderStageFlagBits::eFragment);
    }

    // Vertex bindings an attributes based on ImGui vertex definition
    pipelineBuilder.vertexInputState.bindingDescriptions = { { 0, sizeof(ImDrawVert), vk::VertexInputRate::eVertex } };
    pipelineBuilder.vertexInputState.attributeDescriptions = {
        { 0, 0, vk::Format::eR32G32Sfloat, offsetof(ImDrawVert, pos) },   // Location 0: Position
        { 1, 0, vk::Format::eR32G32Sfloat, offsetof(ImDrawVert, uv) },    // Location 1: UV
        { 2, 0, vk::Format::eR8G8B8A8Unorm, offsetof(ImDrawVert, col) },  // Location 2: Color
    };
    pipeline = pipelineBuilder.create(context.pipelineCache);
}

/** Prepare a separate render pass for rendering the UI as an overlay */
void UIOverlay::prepareRenderPass()
{
    vk::AttachmentDescription attachments[2] = {};

    // Color attachment
    attachments[0].format = createInfo.colorformat;
    attachments[0].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    attachments[0].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    attachments[0].initialLayout = vk::ImageLayout::ePresentSrcKHR;
    attachments[0].finalLayout = vk::ImageLayout::ePresentSrcKHR;

    // Depth attachment
    attachments[1].format = createInfo.depthformat;
    attachments[1].loadOp = vk::AttachmentLoadOp::eDontCare;
    attachments[1].storeOp = vk::AttachmentStoreOp::eDontCare;
    attachments[1].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    attachments[1].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    attachments[1].finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

    vk::AttachmentReference colorReference{ 0, vk::ImageLayout::eColorAttachmentOptimal };
    vk::AttachmentReference depthReference{ 1, vk::ImageLayout::eDepthStencilAttachmentOptimal };
    vk::SubpassDependency subpassDependencies[2];

    // Transition from final to initial (VK_SUBPASS_EXTERNAL refers to all commmands executed outside of the actual renderpass)
    subpassDependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    subpassDependencies[0].srcStageMask = vk::PipelineStageFlagBits::eBottomOfPipe;
    subpassDependencies[0].dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    subpassDependencies[0].srcAccessMask = vk::AccessFlagBits::eMemoryRead;
    subpassDependencies[0].dstAccessMask = vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite;
    subpassDependencies[0].dependencyFlags = vk::DependencyFlagBits::eByRegion;

    // Transition from initial to final
    subpassDependencies[1].srcSubpass = 0;
    subpassDependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
    subpassDependencies[1].srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    subpassDependencies[1].dstStageMask = vk::PipelineStageFlagBits::eBottomOfPipe;
    subpassDependencies[1].srcAccessMask = vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite;
    subpassDependencies[1].dstAccessMask = vk::AccessFlagBits::eMemoryRead;
    subpassDependencies[1].dependencyFlags = vk::DependencyFlagBits::eByRegion;

    vk::SubpassDescription subpassDescription;
    subpassDescription.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
    subpassDescription.colorAttachmentCount = 1;
    subpassDescription.pColorAttachments = &colorReference;
    subpassDescription.pDepthStencilAttachment = &depthReference;

    vk::RenderPassCreateInfo renderPassInfo;
    renderPassInfo.attachmentCount = 2;
    renderPassInfo.pAttachments = attachments;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpassDescription;
    renderPassInfo.dependencyCount = 2;
    renderPassInfo.pDependencies = subpassDependencies;

    renderPass = context.device.createRenderPass(renderPassInfo);
}

/** Update the command buffers to reflect UI changes */
void UIOverlay::updateCommandBuffers()
{
    // TODO: Make this eOneTimeSubmit, causes validation errors on exit if we do
    vk::CommandBufferBeginInfo cmdBufInfo{ vk::CommandBufferUsageFlagBits::eSimultaneousUse };

    vk::RenderPassBeginInfo renderPassBeginInfo;
    renderPassBeginInfo.renderPass = renderPass;
    renderPassBeginInfo.renderArea.extent = createInfo.size;
    renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(createInfo.clearValues.size());
    renderPassBeginInfo.pClearValues = createInfo.clearValues.data();

    ImGuiIO& io = ImGui::GetIO();

    const vk::Viewport viewport{ 0.0f, 0.0f, io.DisplaySize.x, io.DisplaySize.y, 0.0f, 1.0f };
    const vk::Rect2D scissor{ {}, vk::Extent2D{ (uint32_t)io.DisplaySize.x, (uint32_t)io.DisplaySize.y } };
    // UI scale and translate via push constants
    pushConstBlock.scale = glm::vec2(2.0f / io.DisplaySize.x, 2.0f / io.DisplaySize.y);
    pushConstBlock.translate = glm::vec2(-1.0f);

    if (cmdBuffers.size())
    {
        // this is probably where the validation error happens
        context.trashAll<vk::CommandBuffer>(cmdBuffers,
                                            [&](const std::vector<vk::CommandBuffer>& buffers) { context.device.freeCommandBuffers(commandPool, buffers); });
        cmdBuffers.clear();
    }

    cmdBuffers = context.device.allocateCommandBuffers({ commandPool, vk::CommandBufferLevel::ePrimary, (uint32_t)createInfo.framebuffers.size() });

    for (size_t i = 0; i < cmdBuffers.size(); ++i)
    {
        renderPassBeginInfo.framebuffer = createInfo.framebuffers[i];

        const auto& cmdBuffer = cmdBuffers[i];
        cmdBuffer.begin(cmdBufInfo);

        cmdBuffer.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);
        cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);
        cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 0, descriptorSet, {});
        cmdBuffer.bindVertexBuffers(0, vertexBuffer.buffer, { 0 });
        cmdBuffer.bindIndexBuffer(indexBuffer.buffer, 0, vk::IndexType::eUint16);
        cmdBuffer.setViewport(0, viewport);
        cmdBuffer.setScissor(0, scissor);

        cmdBuffer.pushConstants(pipelineLayout, vk::ShaderStageFlagBits::eVertex, 0, vk::ArrayProxy<const PushConstBlock>{ pushConstBlock });

        // Render commands
        ImDrawData* imDrawData = ImGui::GetDrawData();
        int32_t vertexOffset = 0;
        int32_t indexOffset = 0;
        for (int32_t j = 0; j < imDrawData->CmdListsCount; j++)
        {
            const ImDrawList* cmd_list = imDrawData->CmdLists[j];

            for (int32_t k = 0; k < cmd_list->CmdBuffer.Size; k++)
            {
                const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[k];
                vk::Rect2D scissorRect;
                scissorRect.offset.x = std::max((int32_t)(pcmd->ClipRect.x), 0);
                scissorRect.offset.y = std::max((int32_t)(pcmd->ClipRect.y), 0);
                scissorRect.extent.width = (uint32_t)(pcmd->ClipRect.z - pcmd->ClipRect.x);
                scissorRect.extent.height = (uint32_t)(pcmd->ClipRect.w - pcmd->ClipRect.y);
                cmdBuffer.setScissor(0, scissorRect);
                cmdBuffer.drawIndexed(pcmd->ElemCount, 1, indexOffset, vertexOffset, 0);
                indexOffset += pcmd->ElemCount;
            }

            vertexOffset += cmd_list->VtxBuffer.Size;
        }

        // Add empty subpasses if requested
        if (createInfo.subpassCount > 1)
            for (uint32_t j = 1; j < createInfo.subpassCount; j++)
                cmdBuffer.nextSubpass(vk::SubpassContents::eInline);

        cmdBuffer.endRenderPass();

        cmdBuffer.end();
    }
}

/** Update vertex and index buffer containing the imGui elements when required */
void UIOverlay::update()
{
    ImDrawData* imDrawData = ImGui::GetDrawData();
    bool updateCmdBuffers = true;

    if (!imDrawData)
        return;

    // Note: Alignment is done inside buffer creation
    vk::DeviceSize vertexBufferSize = imDrawData->TotalVtxCount * sizeof(ImDrawVert);
    vk::DeviceSize indexBufferSize = imDrawData->TotalIdxCount * sizeof(ImDrawIdx);
    // Update buffers only if vertex or index count has been changed compared to current buffer size

    // Vertex buffer
    if (!vertexBuffer || (vertexCount != imDrawData->TotalVtxCount))
    {
        vertexCount = imDrawData->TotalVtxCount;
        if (vertexBuffer)
        {
            vertexBuffer.unmap();
            context.trash<vks::Buffer>(vertexBuffer);
            vertexBuffer = vks::Buffer();
        }
        vertexBuffer = context.createBuffer(vk::BufferUsageFlagBits::eVertexBuffer, vk::MemoryPropertyFlagBits::eHostVisible, vertexBufferSize);
        vertexBuffer.map();
        updateCmdBuffers = true;
    }

    // Index buffer
    vk::DeviceSize indexSize = imDrawData->TotalIdxCount * sizeof(ImDrawIdx);
    if (!indexBuffer || (indexCount < imDrawData->TotalIdxCount))
    {
        indexCount = imDrawData->TotalIdxCount;
        if (indexBuffer)
        {
            indexBuffer.unmap();
            indexBuffer.destroy();
        }
        indexBuffer = context.createBuffer(vk::BufferUsageFlagBits::eIndexBuffer, vk::MemoryPropertyFlagBits::eHostVisible, indexBufferSize);
        indexBuffer.map();
        updateCmdBuffers = true;
    }

    // Upload data
    ImDrawVert* vtxDst = (ImDrawVert*)vertexBuffer.mapped;
    ImDrawIdx* idxDst = (ImDrawIdx*)indexBuffer.mapped;

    for (int n = 0; n < imDrawData->CmdListsCount; n++)
    {
        const ImDrawList* cmd_list = imDrawData->CmdLists[n];
        memcpy(vtxDst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
        memcpy(idxDst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
        vtxDst += cmd_list->VtxBuffer.Size;
        idxDst += cmd_list->IdxBuffer.Size;
    }

    // Flush to make writes visible to GPU
    vertexBuffer.flush();
    indexBuffer.flush();

    if (updateCmdBuffers)
        updateCommandBuffers();
}

void UIOverlay::resize(const vk::Extent2D& size, const std::vector<vk::Framebuffer>& framebuffers)
{
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2((float)(size.width), (float)(size.height));
    createInfo.size = size;
    createInfo.framebuffers = framebuffers;
    updateCommandBuffers();
}

/** Submit the overlay command buffers to a queue */
void UIOverlay::submit(const vk::Queue& queue, uint32_t bufferindex, vk::SubmitInfo submitInfo) const
{
    if (!visible)
        return;

    submitInfo.pCommandBuffers = &cmdBuffers[bufferindex];
    submitInfo.commandBufferCount = 1;

    queue.submit(submitInfo, fence);
    context.device.waitForFences(fence, VK_TRUE, UINT64_MAX);
    context.device.resetFences(fence);
}

bool UIOverlay::header(const char* caption) const
{
    return ImGui::CollapsingHeader(caption, ImGuiTreeNodeFlags_DefaultOpen);
}

bool UIOverlay::checkBox(const char* caption, bool* value) const
{
    return ImGui::Checkbox(caption, value);
}

bool UIOverlay::checkBox(const char* caption, int32_t* value) const
{
    bool val = (*value == 1);
    bool res = ImGui::Checkbox(caption, &val);
    *value = val;
    return res;
}

bool UIOverlay::inputFloat(const char* caption, float* value, float step, uint32_t precision) const {
    return ImGui::InputFloat(caption, value, step, step * 10.0f, precision);
}

bool UIOverlay::sliderFloat(const char* caption, float* value, float min, float max) const
{
    return ImGui::SliderFloat(caption, value, min, max);
}

bool UIOverlay::sliderInt(const char* caption, int32_t* value, int32_t min, int32_t max) const
{
    return ImGui::SliderInt(caption, value, min, max);
}

bool UIOverlay::comboBox(const char* caption, int32_t* itemindex, const std::vector<std::string>& items) const
{
    if (items.empty())
        return false;

    std::vector<const char*> charitems;
    charitems.reserve(items.size());
    for (size_t i = 0; i < items.size(); i++)
        charitems.push_back(items[i].c_str());

    uint32_t itemCount = static_cast<uint32_t>(charitems.size());
    return ImGui::Combo(caption, itemindex, &charitems[0], itemCount, itemCount);
}

bool UIOverlay::button(const char* caption) const
{
    return ImGui::Button(caption);
}

void UIOverlay::text(const char* formatstr, ...) const
{
    va_list args;
    va_start(args, formatstr);
    ImGui::TextV(formatstr, args);
    va_end(args);
}