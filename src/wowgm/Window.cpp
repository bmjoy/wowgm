#include "Window.hpp"
#include "ClientServices.hpp"

#include <imgui.h>

using namespace wowgm;

Window::Window(std::string const& title)
{
    camera.dolly(-2.5f);
    _title = title;

    memset(_realmAddress, 0, 100);
    memset(_accountName, 0, 100);
    memset(_accountPassword, 0, 100);
    strcpy(_realmAddress, "127.0.0.1");
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
    _prepared = true;
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

void Window::PrepareVertices() {
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

void Window::SetupDescriptorSetLayout() {
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
    pipelineBuilder.loadShader(getAssetPath() + "shaders/geometry/vert.spv", vk::ShaderStageFlagBits::eVertex);
    pipelineBuilder.loadShader(getAssetPath() + "shaders/geometry/frag.spv", vk::ShaderStageFlagBits::eFragment);
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
    ImGui::BeginMainMenuBar();
    ImGui::EndMainMenuBar();

    ImGui::SetNextWindowPos({ 10, 10 }, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSizeConstraints({ 300.0f, 165.0f }, { 300.0f, 165.0f });

    ImGui::Begin("Connect", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse);

    ImGui::PushItemWidth(125.0f);
    ImGui::InputText("##RealmHost", _realmAddress, 100);
    ImGui::PopItemWidth();

    ImGui::SameLine();

    ImGui::PushItemWidth(57.0f);
    ImGui::InputInt("Realm##RealmPort", reinterpret_cast<std::int32_t*>(sClientServices->GetHostPort()), 0, 0);
    ImGui::PopItemWidth();

    ImGui::InputText("Account name##GruntAccountName", _accountName, 100);
    ImGui::InputText("Password##GruntAccountPassword", _accountPassword, 100, ImGuiInputTextFlags_Password);

    if (ImGui::Button("Connect##ConnectionButton"))
        sClientServices->AsyncConnect(_accountName, _accountPassword, _realmAddress, *sClientServices->GetHostPort());

    ImGui::End();
}