#include <imgui.h>

#include "Window.hpp"
#include "ClientServices.hpp"
#include "AuthResult.hpp"
#include "RealmList.hpp"
#include "CharacterPackets.hpp"

using namespace wowgm;
using namespace wowgm::protocol::authentification;
using namespace wowgm::protocol::world::packets;

Window::Window(std::string const& title)
{
    camera.dolly(-2.5f);
    _title = title;

    memset(_realmAddress, 0, 100);
    memset(_accountName, 0, 100);
    memset(_accountPassword, 0, 100);
    strcpy(_realmAddress, "127.0.0.1:3724");
}

Window::~Window()
{
    vertices.destroy();
    indices.destroy();
    uniformDataVS.destroy();

    device.destroyPipeline(pipeline);
    device.destroyPipelineLayout(pipelineLayout);
    device.destroyDescriptorSetLayout(descriptorSetLayout);
}

void Window::Prepare()
{
    BaseWindow::Prepare();
    PrepareVertices();
    PrepareUniformBuffers();
    SetupDescriptorSetLayout();
    PreparePipelines();
    SetupDescriptorPool();
    SetupDescriptorSet();

    // Update the drawCmdBuffers with the required drawing commands
    BuildCommandBuffers();
    _Prepared = true;
}

void Window::UpdateDrawCommandBuffer(const vk::CommandBuffer& cmdBuffer)
{
    cmdBuffer.setViewport(0, viewport());
    cmdBuffer.setScissor(0, scissor());
    cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 0, descriptorSet, nullptr);
    cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);
    cmdBuffer.bindVertexBuffers(0, vertices.buffer, { 0 });
    cmdBuffer.bindIndexBuffer(indices.buffer, 0, vk::IndexType::eUint32);
    cmdBuffer.drawIndexed(indexCount, 1, 0, 0, 1);
}

void Window::PrepareVertices()
{
    // Setup vertices
    std::vector<Vertex> vertexBuffer{
        // XYZ Position           RGB Color
        { {  1.0f,  1.0f, 0.0f }, { 1.0f, 0.0f, 0.0f } },
        { { -1.0f,  1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
        { {  0.0f, -1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } },
    };
    vertices = context.stageToDeviceBuffer<Vertex>(vk::BufferUsageFlagBits::eVertexBuffer, vertexBuffer);

    // Setup indices
    std::vector<uint32_t> indexBuffer = { 0, 1, 2 };
    indexCount = (uint32_t)indexBuffer.size();
    indices = context.stageToDeviceBuffer(vk::BufferUsageFlagBits::eIndexBuffer, indexBuffer);
}

void Window::PrepareUniformBuffers()
{
    uniformBufferObject.projectionMatrix = getProjection();
    uniformBufferObject.viewMatrix = glm::translate(glm::mat4(1.0f), camera.position);
    uniformBufferObject.modelMatrix = glm::inverse(camera.matrices.skyboxView);
    uniformDataVS = context.createUniformBuffer(uniformBufferObject);
}

void Window::SetupDescriptorSetLayout()
{
    // Setup layout of descriptors used in this example
    // Basically connects the different shader stages to descriptors
    // for binding uniform buffers, image samplers, etc.
    // So every shader binding should map to one descriptor set layout
    // binding

    // Binding 0 : Uniform buffer (Vertex shader)
    vk::DescriptorSetLayoutBinding layoutBinding;
    layoutBinding.descriptorType = vk::DescriptorType::eUniformBuffer;
    layoutBinding.descriptorCount = 1;
    layoutBinding.stageFlags = vk::ShaderStageFlagBits::eVertex;
    layoutBinding.pImmutableSamplers = NULL;

    vk::DescriptorSetLayoutCreateInfo descriptorLayout;
    descriptorLayout.bindingCount = 1;
    descriptorLayout.pBindings = &layoutBinding;

    descriptorSetLayout = device.createDescriptorSetLayout(descriptorLayout, nullptr);

    // Create the pipeline layout that is used to generate the rendering pipelines that
    // are based on this descriptor set layout
    // In a more complex scenario you would have different pipeline layouts for different
    // descriptor set layouts that could be reused
    vk::PipelineLayoutCreateInfo pPipelineLayoutCreateInfo;
    pPipelineLayoutCreateInfo.setLayoutCount = 1;
    pPipelineLayoutCreateInfo.pSetLayouts = &descriptorSetLayout;
    pipelineLayout = device.createPipelineLayout(pPipelineLayoutCreateInfo);
}

void Window::PreparePipelines()
{
    // Create our rendering pipeline used in this example
    // Vulkan uses the concept of rendering pipelines to encapsulate
    // fixed states
    // This replaces OpenGL's huge (and cumbersome) state machine
    // A pipeline is then stored and hashed on the GPU making
    // pipeline changes much faster than having to set dozens of
    // states
    // In a real world application you'd have dozens of pipelines
    // for every shader set used in a scene
    // Note that there are a few states that are not stored with
    // the pipeline. These are called dynamic states and the
    // pipeline only stores that they are used with this pipeline,
    // but not their states

    // Vertex input state
    vks::pipelines::GraphicsPipelineBuilder pipelineBuilder{ device, pipelineLayout, renderPass };
    pipelineBuilder.vertexInputState.bindingDescriptions = {
        { 0, sizeof(Vertex), vk::VertexInputRate::eVertex },
    };
    pipelineBuilder.vertexInputState.attributeDescriptions = {
        { 0, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, pos) },
        { 1, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, col) },
    };

    // No culling
    pipelineBuilder.rasterizationState.cullMode = vk::CullModeFlagBits::eNone;
    // Depth and stencil state
    pipelineBuilder.depthStencilState = { false };
    // Load shaders
    // Shaders are loaded from the SPIR-V format, which can be generated from glsl
    pipelineBuilder.loadShader("resources/shaders/geometry/vert.spv", vk::ShaderStageFlagBits::eVertex);
    pipelineBuilder.loadShader("resources/shaders/geometry/frag.spv", vk::ShaderStageFlagBits::eFragment);
    // Create rendering pipeline
    pipeline = pipelineBuilder.create(context.pipelineCache);
}

void Window::SetupDescriptorPool()
{
    // We need to tell the API the number of max. requested descriptors per type
    vk::DescriptorPoolSize typeCounts[1];
    // This example only uses one descriptor type (uniform buffer) and only
    // requests one descriptor of this type
    typeCounts[0].type = vk::DescriptorType::eUniformBuffer;
    typeCounts[0].descriptorCount = 1;
    // For additional types you need to add new entries in the type count list
    // E.g. for two combined image samplers :
    // typeCounts[1].type = vk::DescriptorType::eCombinedImageSampler;
    // typeCounts[1].descriptorCount = 2;

    // Create the global descriptor pool
    // All descriptors used in this example are allocated from this pool
    vk::DescriptorPoolCreateInfo descriptorPoolInfo;
    descriptorPoolInfo.poolSizeCount = 1;
    descriptorPoolInfo.pPoolSizes = typeCounts;
    // Set the max. number of sets that can be requested
    // Requesting descriptors beyond maxSets will result in an error
    descriptorPoolInfo.maxSets = 1;
    descriptorPool = device.createDescriptorPool(descriptorPoolInfo);
}

void Window::SetupDescriptorSet()
{
    // Allocate a new descriptor set from the global descriptor pool
    descriptorSet = device.allocateDescriptorSets({ descriptorPool, 1, &descriptorSetLayout })[0];

    // Update the descriptor set determining the shader binding points
    // For every binding point used in a shader there needs to be one
    // descriptor set matching that binding point

    // Binding 0 : Uniform buffer
    std::vector<vk::WriteDescriptorSet> writeDescriptorSets{
        { descriptorSet, 0, 0, 1, vk::DescriptorType::eUniformBuffer, nullptr, &uniformDataVS.descriptor },
    };
    device.updateDescriptorSets(writeDescriptorSets, nullptr);
}

void Window::OnUpdateOverlay()
{
    if (_state == WindowState::WelcomeScreen)
    {
        auto size = ImGui::GetIO().DisplaySize;

        ImGui::SetNextWindowPos({ 0, 0 });
        ImGui::SetNextWindowSize({ float(size.x), float(size.y) });
        ImGui::Begin("##WelcomeScreen", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize);

        static const char* APPLICATION_NAME = "WowGM";

        auto position_x = 0;
        auto position_y = (size.y - 380) / 2;
        auto content_width = 0;

        ImGui::SetCursorPosY(position_y);
        ImGui::PushFont(ui.GetFont(4));
        auto logo_size = ImGui::CalcTextSize(APPLICATION_NAME);

        position_x = (size.x - logo_size.x) / 2;
        position_y += logo_size.y;
        content_width = logo_size.x;

        ImGui::SetCursorPosX(position_x);
        ImGui::Text(APPLICATION_NAME);
        ImGui::PopFont();

#if _DEBUG
        static const char* DEBUG_LABEL = "DEVELOPMENT BUILD - FOR DEBUGGING ONLY";

        position_y += 5.0f; // 5.0f pixel margin between logo and debug label
        ImGui::SetCursorPosY(position_y);
        ImGui::PushFont(ui.GetFont(5));
        auto label_size = ImGui::CalcTextSize(DEBUG_LABEL);
        ImGui::SetCursorPosX((size.x - label_size.x) / 2);
        ImGui::Text(DEBUG_LABEL);
        ImGui::PopFont();
        position_y += label_size.y;
#endif

        position_y += 15.0f;

        ImGui::SetCursorPosX(position_x);
        ImGui::SetCursorPosY(position_y);
        ImGui::PushFont(ui.GetFont(2));
        ImGui::TextColored({ 1.0f, 1.0f, 0.0f, 1.0f }, "Game installation path");
        ImGui::PopFont();
        ImGui::PushItemWidth(content_width);
        ImGui::SetCursorPosX(position_x);
        ImGui::InputText("##GameDataFolder", _gameDataLocation, sizeof(_gameDataLocation), ImGuiInputTextFlags_CallbackCharFilter, [](ImGuiTextEditCallbackData* callbackData) -> int {
#if PLATFORM == PLATFORM_WINDOWS
            switch (callbackData->EventChar)
            {
            case '<':
            case '>':
            case '"':
            case '\\':
            case '|':
            case '?':
            case '*':
                return 1;
            }
#endif
            return 0;
        });

        MpqFileSystem::Instance()->Initialize(_gameDataLocation);

        {
            bool binaryFound = DiskFileSystem::Instance()->FileExists("/Wow.exe", _gameDataLocation);

            ImGui::SetCursorPosX(position_x);
            ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
            ImGui::TextColored(ImVec4(binaryFound ? 0.0f : 1.0f, binaryFound ? 1.0f : 0.0f, 0.0f, 1.0f), binaryFound ? "Executable located." : "Invalid path");
            ImGui::PopFont();

            position_y += 15.0f;
        }

        position_y += 70.0f;
        ImGui::SetCursorPosY(position_y);
        ImGui::SetCursorPosX(position_x);
        ImGui::PushFont(ui.GetFont(2));
        ImGui::TextColored({ 1.0f, 1.0f, 0.0f, 1.0f }, "Realm informations");
        ImGui::PopFont();
        ImGui::SetCursorPosX(position_x);

        auto realm_label_size = ImGui::CalcTextSize("Realm address");

        ImGui::PushItemWidth(content_width - realm_label_size.x - ImGui::GetStyle().ItemInnerSpacing.x * 2);
        ImGui::InputText("Realm address##RealmAddress", _realmAddress, sizeof(_realmAddress));

        position_y += 70.0f;

        ImGui::SetCursorPosY(position_y);
        ImGui::SetCursorPosX(position_x);
        ImGui::PushFont(ui.GetFont(2));
        ImGui::TextColored({ 1.0f, 1.0f, 0.0f, 1.0f }, "Credentials");
        ImGui::PopFont();

        auto account_name_label_size = ImGui::CalcTextSize("Account name");
        ImGui::SetCursorPosX(position_x);
        ImGui::PushItemWidth(content_width - account_name_label_size.x - ImGui::GetStyle().ItemInnerSpacing.x * 2);
        ImGui::InputText("Account name##AccountName", _accountName, sizeof(_accountName));

        ImGui::SetCursorPosX(position_x);
        ImGui::PushItemWidth(content_width - account_name_label_size.x - ImGui::GetStyle().ItemInnerSpacing.x * 2);
        ImGui::InputText("Password##AccountPassword", _accountPassword, sizeof(_accountPassword), ImGuiInputTextFlags_Password);

        position_y += 100.0f;
        auto submit_button_size = ImGui::CalcTextSize("Connect");
        ImGui::SetCursorPosY(position_y);
        ImGui::SetCursorPosX((size.x - (submit_button_size.x + ImGui::GetStyle().ItemInnerSpacing.x * 2)) / 2);
        if (ImGui::Button("Connect##GruntConnect"))
        {
            sClientServices->AsyncConnect(_accountName, _accountPassword, _realmAddress, *sClientServices->GetHostPort());
            _state = WindowState::RealmSelection;
        }

        ImGui::End();
    }

    if (sClientServices->GetAvailableRealmCount() > 1)
    {
        ImGui::SetNextWindowPos({ 10.0f, 35.0f });

        if (ImGui::Begin("Realm selection", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse))
        {
            for (std::uint32_t i = 0; i < sClientServices->GetAvailableRealmCount(); ++i)
            {
                AuthRealmInfo* realmInfo = sClientServices->GetRealmInfo(i);

                ImGuiIO& io = ImGui::GetIO();
                ImGui::PushFont(io.Fonts->Fonts[2]);
                ImGui::TextColored({ 1.0f, 1.0f, 0.0f, 1.0f }, realmInfo->Name.c_str());
                ImGui::PopFont();
                ImGui::SameLine();

                std::stringstream ss;
                ss << realmInfo->GetEndpoint();

                if (ImGui::Button(ss.str().c_str()))
                    sClientServices->ConnectToRealm(*realmInfo);
            }

            ImGui::End();
        }
    }

    if (sClientServices->GetCharacterCount() != 0)
    {
        ImGui::SetNextWindowPos({ 10.0f, 35.0f });

        if (ImGui::Begin("Character selection", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse))
        {
            auto i = 0u;
            for (auto&& characterInfo : sClientServices->GetCharacters())
            {
                if (ImGui::Button(characterInfo.Name.c_str()))
                    sClientServices->EnterWorld(characterInfo);

                ImGui::Text("Level %u <Gender> <Class> <Race>", characterInfo.Level);
#if _DEBUG
                ImGui::Text(characterInfo.GUID.ToString().c_str());
#endif

                if (++i < sClientServices->GetCharacterCount())
                    ImGui::Separator();
            }

            ImGui::End();
        }
    }

    if (sClientServices->IsInWorld())
    {
        if (ImGui::Begin("Chat window", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize))
        {

            ImGui::End();
        }
    }

    // Must be at the end, for modal windows
    ImGui::BeginMainMenuBar();
    if (ImGui::BeginMenu("File"))
    {
        ImGui::MenuItem("Options", nullptr, &_showOptionsWindow);
        ImGui::EndMenu();
    }

    if (_showOptionsWindow && ImGui::Begin("Options", &_showOptionsWindow))
    {
        ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[2]);
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Game data:");
        ImGui::PopFont();

        ImGui::PushItemWidth(-1);
        ImGui::InputText("##GameDataFolder", _gameDataLocation, sizeof(_gameDataLocation), ImGuiInputTextFlags_CallbackCharFilter, [](ImGuiTextEditCallbackData* callbackData) -> int {
#if PLATFORM == PLATFORM_WINDOWS
            switch (callbackData->EventChar)
            {
                case '<':
                case '>':
                case '"':
                case '\\':
                case '|':
                case '?':
                case '*':
                    return 1;
            }
#endif
            return 0;
        });
        ImGui::PopItemWidth();

        MpqFileSystem::Instance()->Initialize(_gameDataLocation);

        bool binaryFound = DiskFileSystem::Instance()->FileExists("/Wow.exe", _gameDataLocation);

        ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
        ImGui::TextColored(ImVec4(binaryFound ? 0.0f : 1.0f, binaryFound ? 1.0f : 0.0f, 0.0f, 1.0f), binaryFound ? "Executable located." : "Invalid path");
        ImGui::PopFont();

        ImGui::End();
    }

    ImGui::EndMainMenuBar();
}