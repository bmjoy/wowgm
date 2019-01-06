#include "InterfaceRenderer.hpp"
#include "Window.hpp"

#include <graphics/vulkan/Device.hpp>
#include <graphics/vulkan/Swapchain.hpp>
#include <graphics/vulkan/RenderPass.hpp>
#include <graphics/vulkan/Shader.hpp>
#include <graphics/vulkan/Pipeline.hpp>
#include <graphics/vulkan/CommandBuffer.hpp>
#include <graphics/vulkan/DescriptorSetLayout.hpp>
#include <graphics/vulkan/DescriptorSetLayoutCache.hpp>
#include <graphics/vulkan/Queue.hpp>
#include <graphics/vulkan/Buffer.hpp>
#include <graphics/vulkan/Image.hpp>
#include <graphics/vulkan/ImageView.hpp>

#include <extstd/misc/alignment.hpp>

#include <GLFW/glfw3.h>

#include <imgui/imgui.h>

#ifdef max
#undef max
#undef min
#endif

namespace wowgm
{
    using namespace gfx::vk;

    InterfaceRenderer::InterfaceRenderer(Swapchain* swapchain) : Renderer(swapchain)
    {
        _transferSemaphore = GetDevice()->CreateSemaphore();

        ImGuiStyle * style = &ImGui::GetStyle();

        style->WindowPadding = ImVec2(15, 15);
        style->WindowRounding = 5.0f;
        style->FramePadding = ImVec2(5, 5);
        style->FrameRounding = 4.0f;
        style->ItemSpacing = ImVec2(12, 8);
        style->ItemInnerSpacing = ImVec2(8, 6);
        style->IndentSpacing = 25.0f;
        style->ScrollbarSize = 15.0f;
        style->ScrollbarRounding = 9.0f;
        style->GrabMinSize = 5.0f;
        style->GrabRounding = 3.0f;

        style->Colors[ImGuiCol_Text] = ImVec4(0.80f, 0.80f, 0.83f, 1.00f);
        style->Colors[ImGuiCol_TextDisabled] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
        style->Colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
        style->Colors[ImGuiCol_ChildWindowBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
        style->Colors[ImGuiCol_PopupBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
        style->Colors[ImGuiCol_Border] = ImVec4(0.80f, 0.80f, 0.83f, 0.88f);
        style->Colors[ImGuiCol_BorderShadow] = ImVec4(0.92f, 0.91f, 0.88f, 0.00f);
        style->Colors[ImGuiCol_FrameBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
        style->Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
        style->Colors[ImGuiCol_FrameBgActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
        style->Colors[ImGuiCol_TitleBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
        style->Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.00f, 0.98f, 0.95f, 0.75f);
        style->Colors[ImGuiCol_TitleBgActive] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
        style->Colors[ImGuiCol_MenuBarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
        style->Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
        style->Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
        style->Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
        style->Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
        // style->Colors[ImGuiCol_ComboBg] = ImVec4(0.19f, 0.18f, 0.21f, 1.00f);
        style->Colors[ImGuiCol_CheckMark] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
        style->Colors[ImGuiCol_SliderGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
        style->Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
        style->Colors[ImGuiCol_Button] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
        style->Colors[ImGuiCol_ButtonHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
        style->Colors[ImGuiCol_ButtonActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
        style->Colors[ImGuiCol_Header] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
        style->Colors[ImGuiCol_HeaderHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
        style->Colors[ImGuiCol_HeaderActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
        style->Colors[ImGuiCol_Column] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
        style->Colors[ImGuiCol_ColumnHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
        style->Colors[ImGuiCol_ColumnActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
        style->Colors[ImGuiCol_ResizeGrip] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        style->Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
        style->Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
        // style->Colors[ImGuiCol_CloseButton] = ImVec4(0.40f, 0.39f, 0.38f, 0.16f);
        // style->Colors[ImGuiCol_CloseButtonHovered] = ImVec4(0.40f, 0.39f, 0.38f, 0.39f);
        // style->Colors[ImGuiCol_CloseButtonActive] = ImVec4(0.40f, 0.39f, 0.38f, 1.00f);
        style->Colors[ImGuiCol_PlotLines] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
        style->Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
        style->Colors[ImGuiCol_PlotHistogram] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
        style->Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
        style->Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.25f, 1.00f, 0.00f, 0.43f);
        style->Colors[ImGuiCol_ModalWindowDarkening] = ImVec4(1.00f, 0.98f, 0.95f, 0.73f);
    }

    void InterfaceRenderer::initializeRenderPass(RenderPass* renderPass)
    {
        _renderPass = renderPass;

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
    }

    void InterfaceRenderer::initializePipeline(Window* window)
    {
        Shader* fragmentShader = Shader::FromDisk(GetDevice(), "./resources/shaders/imgui/frag.spv", "main", VK_SHADER_STAGE_FRAGMENT_BIT);
        Shader* vertexShader = Shader::FromDisk(GetDevice(), "./resources/shaders/imgui/vert.spv", "main", VK_SHADER_STAGE_VERTEX_BIT);

        GraphicsPipelineCreateInfo pipelineCreateInfo;
        pipelineCreateInfo.renderPass = _renderPass;
        _renderPass = nullptr; // Don't need it anymore
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

        /// Topology
        pipelineCreateInfo.inputAssemblyState.primitiveRestartEnable = VK_FALSE;
        pipelineCreateInfo.inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

        /// Dynamic states
        pipelineCreateInfo.dynamicState.scissors = true;

        /// Multisampling
        pipelineCreateInfo.multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        pipelineCreateInfo.multisampleState.sampleShadingEnable = VK_FALSE;

        /// Color blend
        pipelineCreateInfo.colorBlendState.logicOpEnable = VK_FALSE;
        pipelineCreateInfo.colorBlendState.logicOp = VK_LOGIC_OP_COPY;
        pipelineCreateInfo.colorBlendState.attachments.push_back(VkPipelineColorBlendAttachmentState{
            VK_TRUE,                                                                                                    // VkBool32                 blendEnable;
            VK_BLEND_FACTOR_SRC_ALPHA,                                                                                  // VkBlendFactor            srcColorBlendFactor;
            VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,                                                                        // VkBlendFactor            dstColorBlendFactor;
            VK_BLEND_OP_ADD,                                                                                            // VkBlendOp                colorBlendOp;
            VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,                                                                        // VkBlendFactor            srcAlphaBlendFactor;
            VK_BLEND_FACTOR_ZERO,                                                                                       // VkBlendFactor            dstAlphaBlendFactor;
            VK_BLEND_OP_ADD,                                                                                            // VkBlendOp                alphaBlendOp;
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,  // VkColorComponentFlags    colorWriteMask;
        });

        /// Vertex attributes and bindings
        std::vector<VkVertexInputAttributeDescription> vertexInputAttributes = {
            VkVertexInputAttributeDescription{ 0, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(ImDrawVert, pos) },  // Location 0: Position
            VkVertexInputAttributeDescription{ 1, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(ImDrawVert, uv) },   // Location 1: UV
            VkVertexInputAttributeDescription{ 2, 0, VK_FORMAT_R8G8B8A8_UNORM, offsetof(ImDrawVert, col) }, // Location 0: Color
        };

        pipelineCreateInfo.vertexInputState.vertexAttributeDescriptions = vertexInputAttributes;
        pipelineCreateInfo.vertexInputState.vertexBindingDescriptions.push_back({ 0, sizeof(ImDrawVert), VK_VERTEX_INPUT_RATE_VERTEX });

        VkResult result = Pipeline::Create(GetDevice(), &pipelineCreateInfo, &_pipeline);
        BOOST_ASSERT_MSG(result == VK_SUCCESS, "Unable to create a pipeline");

        delete fragmentShader;
        delete vertexShader;

        ImGuiIO& io = ImGui::GetIO();
        io.DisplaySize.x = GetSwapchain()->GetExtent().width;
        io.DisplaySize.y = GetSwapchain()->GetExtent().height;

        io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
        io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;

        io.KeyMap[ImGuiKey_Tab] = GLFW_KEY_TAB;
        io.KeyMap[ImGuiKey_LeftArrow] = GLFW_KEY_LEFT;
        io.KeyMap[ImGuiKey_RightArrow] = GLFW_KEY_RIGHT;
        io.KeyMap[ImGuiKey_UpArrow] = GLFW_KEY_UP;
        io.KeyMap[ImGuiKey_DownArrow] = GLFW_KEY_DOWN;
        io.KeyMap[ImGuiKey_PageUp] = GLFW_KEY_PAGE_UP;
        io.KeyMap[ImGuiKey_PageDown] = GLFW_KEY_PAGE_DOWN;
        io.KeyMap[ImGuiKey_Home] = GLFW_KEY_HOME;
        io.KeyMap[ImGuiKey_End] = GLFW_KEY_END;
        io.KeyMap[ImGuiKey_Insert] = GLFW_KEY_INSERT;
        io.KeyMap[ImGuiKey_Delete] = GLFW_KEY_DELETE;
        io.KeyMap[ImGuiKey_Backspace] = GLFW_KEY_BACKSPACE;
        io.KeyMap[ImGuiKey_Space] = GLFW_KEY_SPACE;
        io.KeyMap[ImGuiKey_Enter] = GLFW_KEY_ENTER;
        io.KeyMap[ImGuiKey_Escape] = GLFW_KEY_ESCAPE;
        io.KeyMap[ImGuiKey_A] = GLFW_KEY_A;
        io.KeyMap[ImGuiKey_C] = GLFW_KEY_C;
        io.KeyMap[ImGuiKey_V] = GLFW_KEY_V;
        io.KeyMap[ImGuiKey_X] = GLFW_KEY_X;
        io.KeyMap[ImGuiKey_Y] = GLFW_KEY_Y;
        io.KeyMap[ImGuiKey_Z] = GLFW_KEY_Z;

        io.ImeWindowHandle = (void*) window->getHandle();

        ImFont* font = io.Fonts->AddFontFromFileTTF("./resources/fonts/Ruda-Bold.ttf", 12.0f, nullptr, nullptr);
        _fontStore["PlayRegular"] = font;

        { // Upload the textures
            unsigned char* pixels;
            int32_t width, height;

            io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
            size_t dataSize = width * height * 4;

            ImageCreateInfo imageCreateInfo;
            imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
            imageCreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
            imageCreateInfo.extent.width = width;
            imageCreateInfo.extent.height = height;
            imageCreateInfo.extent.depth = 1u;
            imageCreateInfo.mipLevels = 1;
            imageCreateInfo.arrayLayers = 1;
            imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
            imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
            imageCreateInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
            imageCreateInfo.pImageName = "Interface font atlas";

            Queue* transferQueue = GetDevice()->GetQueueByFlags(VK_QUEUE_GRAPHICS_BIT, 0);
            imageCreateInfo.queueFamilyIndices.push_back(transferQueue->GetFamilyIndex());

            _fontTexture = GetDevice()->CreateImage(VMA_MEMORY_USAGE_GPU_ONLY, 0, &imageCreateInfo);
            BOOST_ASSERT_MSG(_fontTexture != nullptr, "Failed to create font texture for interface");

            ImageViewCreateInfo imageViewCreateInfo;
            imageViewCreateInfo.image = _fontTexture;
            imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            imageViewCreateInfo.subresourceRange.layerCount = 1;
            imageViewCreateInfo.subresourceRange.levelCount = 1;
            imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
            imageViewCreateInfo.subresourceRange.baseMipLevel = 0;

            _fontTextureView = GetDevice()->CreateImageView(&imageViewCreateInfo);
            BOOST_ASSERT_MSG(_fontTextureView != nullptr, "Failed to create font texture view for interface");

            SamplerCreateInfo samplerCreateInfo;
            samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
            samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
            samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
            samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            samplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
            samplerCreateInfo.mipLodBias = 0.0f;
            samplerCreateInfo.anisotropyEnable = VK_FALSE;
            samplerCreateInfo.maxAnisotropy = 1.0f;
            samplerCreateInfo.compareEnable = VK_FALSE;
            samplerCreateInfo.compareOp = VK_COMPARE_OP_NEVER;
            samplerCreateInfo.minLod = -1000.0f;
            samplerCreateInfo.maxLod = 10000.0f;
            samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;
            _fontSampler = GetDevice()->CreateSampler(&samplerCreateInfo);
            // BOOST_ASSERT_MSG(_fontSampler != nullptr, "Failed to create interface font sampler");

            GetPipeline()->GetDescriptorSetLayout(0)->UpdateBinding(0, _fontSampler, _fontTextureView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

            // Create an upload buffer
            BufferCreateInfo bufferCreateInfo;
            bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            bufferCreateInfo.queueFamilyIndices.push_back(transferQueue->GetFamilyIndex());
            bufferCreateInfo.size = dataSize;
            bufferCreateInfo.pBufferName = "Texture upload buffer";

            Buffer* uploadBuffer = GetDevice()->CreateBuffer(VMA_MEMORY_USAGE_CPU_TO_GPU, 0, &bufferCreateInfo);
            BOOST_ASSERT_MSG(uploadBuffer != nullptr, "Failed to create the texture upload buffer");

            void* mapData = nullptr;
            VkResult result = GetDevice()->MapBuffer(uploadBuffer, &mapData);
            BOOST_ASSERT_MSG(result == VK_SUCCESS, "Failed to map texture upload buffer");

            memcpy(mapData, pixels, dataSize);

            GetDevice()->FlushBuffer(uploadBuffer);
            GetDevice()->UnmapBuffer(uploadBuffer);

            CommandBuffer* uploadCommandBuffer;
            GetDevice()->AllocateCommandBuffers(transferQueue, nullptr, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1, &uploadCommandBuffer);

            result = uploadCommandBuffer->Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, nullptr);
            BOOST_ASSERT_MSG(result == VK_SUCCESS, "Failed to record transfer command buffer");

            ImageMemoryBarrier memoryBarrier;
            memoryBarrier.srcAccessMask = 0;
            memoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            memoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            memoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            memoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            memoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            memoryBarrier.image = _fontTexture;
            memoryBarrier.subresourceRange.layerCount = 1;
            memoryBarrier.subresourceRange.levelCount = 1;
            memoryBarrier.subresourceRange.baseArrayLayer = 0;
            memoryBarrier.subresourceRange.baseMipLevel = 0;
            uploadCommandBuffer->PipelineBarrier(&memoryBarrier, VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0);

            BufferImageCopy imageCopyParams{};
            imageCopyParams.imageExtent.width = width;
            imageCopyParams.imageExtent.height = height;
            imageCopyParams.imageExtent.depth = 1;
            imageCopyParams.imageOffset.x = 0;
            imageCopyParams.imageOffset.y = 0;
            imageCopyParams.imageOffset.z = 0;
            imageCopyParams.imageSubresource.layerCount = 1;
            imageCopyParams.imageSubresource.mipLevel = 0;
            uploadCommandBuffer->CopyBufferToImage(uploadBuffer, _fontTexture, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageCopyParams);

            memoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            memoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            memoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            memoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            uploadCommandBuffer->PipelineBarrier(&memoryBarrier, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0);

            uploadCommandBuffer->End();

            VkFence uploadFence = GetDevice()->CreateFence(0);
            BOOST_ASSERT_MSG(uploadFence != VK_NULL_HANDLE, "failed to create texture upload fence");

            SubmitInfo uploadSubmit;
            uploadSubmit.commandBuffers.push_back(uploadCommandBuffer);

            result = transferQueue->Submit(1, &uploadSubmit, &uploadFence);
            BOOST_ASSERT_MSG(result == VK_SUCCESS, "Failed to submit transfer commands");

            result = transferQueue->WaitForFences(1, &uploadFence, std::numeric_limits<std::uint64_t>::max());

            // Release anything we won't use anymore
            GetDevice()->DestroyFence(uploadFence);
            delete uploadCommandBuffer;
            GetDevice()->DestroyBuffer(uploadBuffer);
        }
    }

    InterfaceRenderer::~InterfaceRenderer()
    {
        // Deletes the pipeline, its layout.
        delete _pipeline;

        GetDevice()->DestroyBuffer(_vertexBuffer);
        GetDevice()->DestroyBuffer(_indexBuffer);
        GetDevice()->DestroyBuffer(_stagingBuffer);

        GetDevice()->DestroySampler(_fontSampler);

        GetDevice()->DestroyImage(_fontTexture);
        GetDevice()->DestroyImageView(_fontTextureView);

        delete _transferCommandBuffer;

        if (_transferSemaphore != VK_NULL_HANDLE)
            GetDevice()->DestroySemaphore(_transferSemaphore);
    }

    void InterfaceRenderer::onRenderQuery(CommandBuffer* commandBuffer)
    {
        ImDrawData* imDrawData = ImGui::GetDrawData();
        if (imDrawData == nullptr || imDrawData->CmdListsCount == 0)
        {
            VkRect2D scissor;
            scissor.offset.x = 0;
            scissor.offset.y = 0;
            scissor.extent.width = GetSwapchain()->GetExtent().width;
            scissor.extent.height = GetSwapchain()->GetExtent().height;

            commandBuffer->SetScissor(0, 1, &scissor);
            return;
        }

        commandBuffer->BeginLabel("InterfaceRenderer::onFrame", {1.0f, 0.0f, 0.0f, 0.0f});

        commandBuffer->BindPipeline(GetPipeline());
        commandBuffer->BindIndexBuffer(_indexBuffer, 0, VK_INDEX_TYPE_UINT16);
        {
            VkDeviceSize offset = 0;
            commandBuffer->BindVertexBuffers(0, 1, &_vertexBuffer, &offset);
        }
        commandBuffer->BindDescriptorSet(0, 1, &GetPipeline()->GetDescriptorSetLayout(0)->GetDescriptorSet(0), 0, nullptr);

        uint32_t idxOffset = 0, vtxOffset = 0;

        float scale[4];
        scale[0] = 2.0f / imDrawData->DisplaySize.x;
        scale[1] = 2.0f / imDrawData->DisplaySize.y;
        // translation
        scale[2] = -1.0f - imDrawData->DisplayPos.x * scale[0];
        scale[3] = -1.0f - imDrawData->DisplayPos.y * scale[1];

        commandBuffer->PushConstants(0, scale);

        for (int32_t i = 0; i < imDrawData->CmdListsCount; ++i)
        {
            ImDrawList* commands = imDrawData->CmdLists[i];
            for (int32_t j = 0; j < commands->CmdBuffer.Size; ++j)
            {
                ImDrawCmd* command = &commands->CmdBuffer[j];

                VkRect2D scissor;
                scissor.offset.x = (uint32_t)std::max(command->ClipRect.x - imDrawData->DisplayPos.x, 0.0f);
                scissor.offset.y = (uint32_t)std::max(command->ClipRect.y - imDrawData->DisplayPos.y, 0.0f);
                scissor.extent.width = std::ceil(command->ClipRect.z - command->ClipRect.x);
                scissor.extent.height = std::ceil(command->ClipRect.w - command->ClipRect.y);

                commandBuffer->SetScissor(0, 1, &scissor);
                commandBuffer->DrawIndexed(command->ElemCount, 1, idxOffset, vtxOffset, 0);

                idxOffset += command->ElemCount;
            }
            vtxOffset += commands->VtxBuffer.size();
        }

        commandBuffer->EndLabel();
    }

    void InterfaceRenderer::_RenderInterface()
    {
        static char data[1200];

        ImGui::Begin("Test frame");
        ImGui::Text("Hello world");
        ImGui::InputText("bla", data, sizeof(data));
        ImGui::End();

        ImGui::ShowDemoWindow();
    }

    void InterfaceRenderer::beforeRenderQuery(gfx::vk::CommandBuffer* buffer)
    {
        Queue* graphicsQueue = GetDevice()->GetQueueByFlags(VK_QUEUE_GRAPHICS_BIT, 0);
        Queue* transferQueue = GetDevice()->GetQueueByFlags(VK_QUEUE_TRANSFER_BIT, 0);

        _waitNeeded = false;

        ImGui::NewFrame();
        _RenderInterface();
        ImGui::Render();

        ImDrawData* imDrawData = ImGui::GetDrawData();
        if (imDrawData == nullptr
            || imDrawData->CmdListsCount == 0
            || _vertexCount == imDrawData->TotalVtxCount
            || _indexCount == imDrawData->TotalIdxCount)
            return;

        VkDeviceSize vertexBufferSize = imDrawData->TotalVtxCount * sizeof(ImDrawVert);
        VkDeviceSize indexBufferSize = imDrawData->TotalIdxCount * sizeof(ImDrawIdx);

        _vertexCount = imDrawData->TotalVtxCount;
        _indexCount = imDrawData->TotalIdxCount;

        if (vertexBufferSize == 0 || indexBufferSize == 0)
            return;

        _waitNeeded = graphicsQueue->GetHandle() != transferQueue->GetHandle();

        std::vector<ImDrawVert> vertices;
        std::vector<ImDrawIdx> indices;

        for (int32_t i = 0; i < imDrawData->CmdListsCount; ++i)
        {
            ImDrawList* drawList = imDrawData->CmdLists[i];
            vertices.insert(vertices.end(), drawList->VtxBuffer.begin(), drawList->VtxBuffer.end());
            indices.insert(indices.end(), drawList->IdxBuffer.begin(), drawList->IdxBuffer.end());
        }

        gfx::vk::BufferCreateInfo createInfo;
        createInfo.queueFamilyIndices.push_back(graphicsQueue->GetFamilyIndex());
        if (_waitNeeded)
            createInfo.queueFamilyIndices.push_back(transferQueue->GetFamilyIndex());

        // Previous buffer can't fit the indices.
        if (_indexBuffer != nullptr && _indexBuffer->GetSize() <= indexBufferSize)
        {
            _indexBuffer->ScheduleRelease();
            _indexBuffer = nullptr;
        }

        if (_indexBuffer == nullptr)
        {
            createInfo.size = extstd::misc::align_up(uint32_t(indexBufferSize * 1.2f), 4u);
            createInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
            createInfo.pBufferName = "Interface Index Buffer";
            _indexBuffer = GetDevice()->CreateBuffer(VMA_MEMORY_USAGE_GPU_ONLY, 0u, &createInfo);
            BOOST_ASSERT_MSG(_indexBuffer != nullptr, "Unable to allocate a buffer for the interface index buffer");
        }

        // Previous buffer can't fit the indices.
        if (_vertexBuffer != nullptr && _vertexBuffer->GetSize() <= vertexBufferSize)
        {
            _vertexBuffer->ScheduleRelease();
            _vertexBuffer = nullptr;
        }

        if (_vertexBuffer == nullptr)
        {
            createInfo.size = extstd::misc::align_up(uint32_t(vertexBufferSize * 1.2f), 4u);
            createInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
            createInfo.pBufferName = "Interface Vertex Buffer";
            _vertexBuffer = GetDevice()->CreateBuffer(VMA_MEMORY_USAGE_GPU_ONLY, 0u, &createInfo);
            BOOST_ASSERT_MSG(_vertexBuffer != nullptr, "Unable to allocate a buffer for the interface vertex buffer");
        }

        UploadData(buffer, vertices, indices);
    }

    void InterfaceRenderer::UploadData(gfx::vk::CommandBuffer* frameCommandBuffer, std::vector<ImDrawVert> const& vertices, std::vector<ImDrawIdx> const& indices)
    {
        Queue* graphicsQueue = GetDevice()->GetQueueByFlags(VK_QUEUE_GRAPHICS_BIT, 0);
        Queue* transferQueue = GetDevice()->GetQueueByFlags(VK_QUEUE_TRANSFER_BIT, 0);

        if (_stagingBuffer == nullptr)
        {
            BufferCreateInfo bufferCreateInfo;
            bufferCreateInfo.size = _vertexCount * sizeof(ImDrawVert) + _indexCount * sizeof(ImDrawIdx);
            bufferCreateInfo.queueFamilyIndices.push_back(transferQueue->GetFamilyIndex());
            bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            bufferCreateInfo.pBufferName = "Interface staging buffer";
            _stagingBuffer = GetDevice()->CreateBuffer(VMA_MEMORY_USAGE_CPU_TO_GPU, 0, &bufferCreateInfo);
            BOOST_ASSERT_MSG(_stagingBuffer != nullptr, "Failed to create a staging buffer");
        }

        uint8_t* mappedData = nullptr;
        VkResult result = GetDevice()->MapBuffer(_stagingBuffer, (void**)&mappedData);
        BOOST_ASSERT_MSG(result == VK_SUCCESS, "Failed to map the interface staging buffer");

        memcpy(mappedData, vertices.data(), vertices.size() * sizeof(ImDrawVert));
        memcpy(mappedData + vertices.size() * sizeof(ImDrawVert), indices.data(), indices.size() * sizeof(ImDrawIdx));
        GetDevice()->UnmapBuffer(_stagingBuffer);

        gfx::vk::CommandBuffer* uploadCommandBuffer;

        if (graphicsQueue->GetHandle() != transferQueue->GetHandle())
        {
            if (_transferCommandBuffer == nullptr)
            {
                // If the transfer queue and the graphics queue are not the same,
                // we need to create a new command buffer and introduce a wait semaphore

                result = GetDevice()->AllocateCommandBuffers(transferQueue, nullptr, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1, &_transferCommandBuffer);
                BOOST_ASSERT_MSG(result == VK_SUCCESS, "Failed to allocate a command buffer for data tansfer");
                _transferCommandBuffer->SetName("Interface upload command buffer");

            }

            uploadCommandBuffer = _transferCommandBuffer;

            result = uploadCommandBuffer->Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, nullptr);
            BOOST_ASSERT_MSG(result == VK_SUCCESS, "Failed to start recording");
        }
        else
            uploadCommandBuffer = frameCommandBuffer;

        // Copy vertices first
        VkBufferCopy copyOp;
        copyOp.srcOffset = 0;
        copyOp.size = vertices.size() * sizeof(ImDrawVert);
        copyOp.dstOffset = 0;

        uploadCommandBuffer->CopyBuffer(_stagingBuffer, _vertexBuffer, 1, &copyOp);

        copyOp.srcOffset = copyOp.size;
        copyOp.size = indices.size() * sizeof(ImDrawIdx);
        uploadCommandBuffer->CopyBuffer(_stagingBuffer, _indexBuffer, 1, &copyOp);

        // We don't need to start and begin recording if the transfer queue and the graphics queue are identical
        // Because within this hook we are already in a recording state.
        if (graphicsQueue->GetHandle() != transferQueue->GetHandle())
        {
            uploadCommandBuffer->End();

            // But now we need to submit the command buffer to the transfer queue.
            gfx::vk::SubmitInfo submitInfo;
            submitInfo.commandBuffers.push_back(uploadCommandBuffer);
            submitInfo.signalSemaphores.push_back(_transferSemaphore);
            transferQueue->Submit(1, &submitInfo, nullptr);
        }
    }

    void InterfaceRenderer::afterRenderQuery(gfx::vk::CommandBuffer* buffer, std::vector<std::pair<VkSemaphore, VkPipelineStageFlags>>& waitSemaphores)
    {
        if (_waitNeeded)
            waitSemaphores.push_back(std::make_pair(_transferSemaphore, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT));
    }

    Pipeline* InterfaceRenderer::GetPipeline() const
    {
        return _pipeline;
    }
}