/*
* Vulkan Example base class
*
* Copyright (C) 2016 by Sascha Willems - www.saschawillems.de
*
* This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/
#include "VulkanBase.hpp"

#include <imgui.h>

#include "UI.hpp"
#include "Storage.hpp"
#include "Filesystem.hpp"

using namespace vkx;

// Avoid doing work in the ctor as it can't make use of overridden virtual functions
// Instead, use the `prepare` and `run` methods
BaseWindow::BaseWindow()
{
    camera.setPerspective(60.0f, _size, 0.1f, 256.0f);
}

BaseWindow::~BaseWindow()
{
    context.queue.waitIdle();
    context.device.waitIdle();

    // Clean up Vulkan resources
    swapChain.destroy();
    // FIXME destroy surface

    if (descriptorPool)
        device.destroyDescriptorPool(descriptorPool);

    if (!commandBuffers.empty())
    {
        device.freeCommandBuffers(cmdPool, commandBuffers);
        commandBuffers.clear();
    }

    device.destroyRenderPass(renderPass);
    for (uint32_t i = 0; i < framebuffers.size(); i++)
        device.destroyFramebuffer(framebuffers[i]);

    _depthStencil.destroy();

    device.destroySemaphore(semaphores.acquireComplete);
    device.destroySemaphore(semaphores.renderComplete);
    device.destroySemaphore(semaphores.overlayComplete);

    ui.destroy();

    context.destroy();

    glfwDestroyWindow(_window);
    glfwTerminate();
}

void BaseWindow::Run()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    glfwInit();
    SetupWindow();
    InitVulkan();
    SetupSwapchain();
    Prepare();

    RenderLoop();

    // Once we exit the render loop, wait for everything to become idle before proceeding to the descructor.
    context.queue.waitIdle();
    context.device.waitIdle();

    ImGui::DestroyContext();
}

void BaseWindow::GetEnabledFeatures()
{
}

void BaseWindow::InitVulkan()
{
    // TODO make this less stupid
    context.setDeviceFeaturesPicker([this](const vk::PhysicalDevice& device, vk::PhysicalDeviceFeatures2& features) {
        if (deviceFeatures.textureCompressionBC) {
            enabledFeatures.textureCompressionBC = VK_TRUE;
        }
        else if (context.deviceFeatures.textureCompressionASTC_LDR) {
            enabledFeatures.textureCompressionASTC_LDR = VK_TRUE;
        }
        else if (context.deviceFeatures.textureCompressionETC2) {
            enabledFeatures.textureCompressionETC2 = VK_TRUE;
        }
        if (deviceFeatures.samplerAnisotropy) {
            enabledFeatures.samplerAnisotropy = VK_TRUE;
        }
        GetEnabledFeatures();
    });

#if _DEBUG
    context.setValidationEnabled(true);
#endif
    context.requireExtensions(glfw::Window::getRequiredInstanceExtensions());
    context.requireDeviceExtensions({ VK_KHR_SWAPCHAIN_EXTENSION_NAME });
    context.createInstance(_version);

    surface = glfw::Window::createWindowSurface(_window, context.instance);

    context.createDevice(surface);

    // Find a suitable depth format
    depthFormat = context.getSupportedDepthFormat();

    // Create synchronization objects

    // A semaphore used to synchronize image presentation
    // Ensures that the image is displayed before we start submitting new commands to the queu
    semaphores.acquireComplete = device.createSemaphore({});
    // A semaphore used to synchronize command submission
    // Ensures that the image is not presented until all commands have been sumbitted and executed
    semaphores.renderComplete = device.createSemaphore({});

    semaphores.overlayComplete = device.createSemaphore({});

    renderWaitSemaphores.push_back(semaphores.acquireComplete);
    renderWaitStages.push_back(vk::PipelineStageFlagBits::eBottomOfPipe);
    renderSignalSemaphores.push_back(semaphores.renderComplete);
}

void BaseWindow::SetupSwapchain()
{
    swapChain.setup(context.physicalDevice, context.device, context.queue, context.queueIndices.graphics);
    swapChain.setSurface(surface);
}

bool BaseWindow::PlatformLoopCondition()
{
    if (glfwWindowShouldClose(_window))
        return false;

    glfwPollEvents();

    if (0 != glfwJoystickPresent(0)) {
        // FIXME implement joystick handling
        int axisCount{ 0 };
        const float* axes = glfwGetJoystickAxes(0, &axisCount);
        if (axisCount >= 2)
        {
            _gamePadState.axisLeft.x = axes[0] * 0.01f;
            _gamePadState.axisLeft.y = axes[1] * -0.01f;
        }
        if (axisCount >= 4)
        {
            _gamePadState.axisRight.x = axes[0] * 0.01f;
            _gamePadState.axisRight.y = axes[1] * -0.01f;
        }
        if (axisCount >= 6)
        {
            float lt = (axes[4] + 1.0f) / 2.0f;
            float rt = (axes[5] + 1.0f) / 2.0f;
            _gamePadState.rz = (rt - lt);
        }
        uint32_t newButtons{ 0 };
        static uint32_t oldButtons{ 0 };
        {
            int buttonCount{ 0 };
            const uint8_t* buttons = glfwGetJoystickButtons(0, &buttonCount);
            for (uint8_t i = 0; i < buttonCount && i < 64; ++i)
                if (0 != buttons[i])
                    newButtons |= (1 << i);
        }
        auto changedButtons = newButtons & ~oldButtons;
        if (changedButtons & 0x01)
            KeyPressed(GAMEPAD_BUTTON_A);
        if (changedButtons & 0x02)
            KeyPressed(GAMEPAD_BUTTON_B);
        if (changedButtons & 0x04)
            KeyPressed(GAMEPAD_BUTTON_X);
        if (changedButtons & 0x08)
            KeyPressed(GAMEPAD_BUTTON_Y);
        if (changedButtons & 0x10)
            KeyPressed(GAMEPAD_BUTTON_L1);
        if (changedButtons & 0x20)
            KeyPressed(GAMEPAD_BUTTON_R1);
        oldButtons = newButtons;
    }
    else
        memset(&_gamePadState, 0, sizeof(_gamePadState));
    return true;
}

void BaseWindow::RenderLoop()
{
    std::chrono::time_point<std::chrono::high_resolution_clock> tStart;

    while (PlatformLoopCondition())
    {
        OnFrame();

        // Render frame
        if (_prepared)
        {
            auto elapsedTime = Render();

            Update(float(elapsedTime.count()) / 1000.0f);
        }
    }
}

std::string BaseWindow::getWindowTitle()
{
    return _title;
}

void BaseWindow::SetupInterface()
{
    _settings.overlay = _settings.overlay && !_benchmark.active;
    if (!_settings.overlay)
        return;

    struct vkx::ui::UIOverlayCreateInfo overlayCreateInfo;
    // Setup default overlay creation info
    overlayCreateInfo.copyQueue = queue;
    overlayCreateInfo.framebuffers = framebuffers;
    overlayCreateInfo.colorformat = swapChain.colorFormat;
    overlayCreateInfo.depthformat = depthFormat;
    overlayCreateInfo.size = _size;

    // Virtual function call for example to customize overlay creation
    OnSetupInterface(overlayCreateInfo);
    ui.create(overlayCreateInfo);

    for (auto& shader : overlayCreateInfo.shaders)
    {
        context.device.destroyShaderModule(shader.module);
        shader.module = vk::ShaderModule{};
    }
    UpdateOverlay();
}

void BaseWindow::Prepare()
{
    cmdPool = context.getCommandPool();

    swapChain.create(_size, enableVsync);
    SetupDepthStencil();
    SetupRenderPass();
    SetupRenderPassBeginInfo();
    SetupFrameBuffer();
    SetupInterface();
    LoadAssets();
}

void BaseWindow::SetupRenderPassBeginInfo()
{
    clearValues.clear();
    clearValues.push_back(vks::util::clearColor(glm::vec4(0.1, 0.1, 0.1, 1.0)));
    clearValues.push_back(vk::ClearDepthStencilValue{ 1.0f, 0 });

    renderPassBeginInfo = vk::RenderPassBeginInfo();
    renderPassBeginInfo.renderPass = renderPass;
    renderPassBeginInfo.renderArea.extent = _size;
    renderPassBeginInfo.clearValueCount = (uint32_t)clearValues.size();
    renderPassBeginInfo.pClearValues = clearValues.data();
}

void BaseWindow::AllocateCommandBuffers()
{
    ClearCommandBuffers();
    // Create one command buffer per image in the swap chain

    // Command buffers store a reference to the
    // frame buffer inside their render pass info
    // so for static usage without having to rebuild
    // them each frame, we use one per frame buffer
    commandBuffers = device.allocateCommandBuffers({ cmdPool, vk::CommandBufferLevel::ePrimary, swapChain.imageCount });
}

void BaseWindow::ClearCommandBuffers()
{
    if (commandBuffers.empty())
        return;

    context.trashCommandBuffers(cmdPool, commandBuffers);
    // FIXME find a better way to ensure that the draw and text buffers are no longer in use before
    // executing them within this command buffer.
    context.queue.waitIdle();
    context.device.waitIdle();
    context.recycle();
}

void BaseWindow::BuildCommandBuffers()
{
    // Destroy and recreate command buffers if already present
    AllocateCommandBuffers();

    vk::CommandBufferBeginInfo cmdBufInfo{ vk::CommandBufferUsageFlagBits::eSimultaneousUse };
    for (size_t i = 0; i < swapChain.imageCount; ++i)
    {
        const auto& cmdBuffer = commandBuffers[i];
        cmdBuffer.reset(vk::CommandBufferResetFlagBits::eReleaseResources);
        cmdBuffer.begin(cmdBufInfo);
        UpdateCommandBufferPreDraw(cmdBuffer);
        // Let child classes execute operations outside the renderpass, like buffer barriers or query pool operations
        renderPassBeginInfo.framebuffer = framebuffers[i];
        cmdBuffer.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);
        UpdateDrawCommandBuffer(cmdBuffer);
        cmdBuffer.endRenderPass();
        UpdateCommandBufferPostDraw(cmdBuffer);
        cmdBuffer.end();
    }
}

void BaseWindow::PrepareFrame()
{
    // Acquire the next image from the swap chaing
    auto resultValue = swapChain.acquireNextImage(semaphores.acquireComplete);
    if (resultValue.result == vk::Result::eSuboptimalKHR)
    {
        ivec2 newSize;
        glfwGetWindowSize(_window, &newSize.x, &newSize.y);
        WindowResize(newSize);
        resultValue = swapChain.acquireNextImage(semaphores.acquireComplete);
    }
    currentBuffer = resultValue.value;
}

void BaseWindow::SubmitFrame()
{
    bool submitOverlay = _settings.overlay && ui.visible;
    if (submitOverlay)
    {
        vk::SubmitInfo submitInfo;
        // Wait for color attachment output to finish before rendering the text overlay
        vk::PipelineStageFlags stageFlags = vk::PipelineStageFlagBits::eBottomOfPipe;
        submitInfo.pWaitDstStageMask = &stageFlags;
        // Wait for render complete semaphore
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = &semaphores.renderComplete;
        // Signal ready with UI overlay complete semaphpre
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = &semaphores.overlayComplete;

        // Submit current UI overlay command buffer
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &ui.cmdBuffers[currentBuffer];
        queue.submit({ submitInfo }, {});
    }
    swapChain.queuePresent(submitOverlay ? semaphores.overlayComplete : semaphores.renderComplete);
}

void BaseWindow::SetupDepthStencil()
{
    _depthStencil.destroy();

    vk::ImageAspectFlags aspect = vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil;
    vk::ImageCreateInfo depthStencilCreateInfo;
    depthStencilCreateInfo.imageType = vk::ImageType::e2D;
    depthStencilCreateInfo.extent = vk::Extent3D { _size.width, _size.height, 1 };
    depthStencilCreateInfo.format = depthFormat;
    depthStencilCreateInfo.mipLevels = 1;
    depthStencilCreateInfo.arrayLayers = 1;
    depthStencilCreateInfo.usage = vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eTransferSrc;
    _depthStencil = context.createImage(depthStencilCreateInfo);

    context.setImageLayout(_depthStencil.image, aspect, vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal);

    vk::ImageViewCreateInfo depthStencilView;
    depthStencilView.viewType = vk::ImageViewType::e2D;
    depthStencilView.format = depthFormat;
    depthStencilView.subresourceRange.aspectMask = aspect;
    depthStencilView.subresourceRange.levelCount = 1;
    depthStencilView.subresourceRange.layerCount = 1;
    depthStencilView.image = _depthStencil.image;
    _depthStencil.view = device.createImageView(depthStencilView);
}

void BaseWindow::SetupFrameBuffer() {
    // Recreate the frame buffers
    if (!framebuffers.empty()) {
        for (uint32_t i = 0; i < framebuffers.size(); i++)
            device.destroyFramebuffer(framebuffers[i]);
        framebuffers.clear();
    }

    vk::ImageView attachments[2];

    // Depth/Stencil attachment is the same for all frame buffers
    attachments[1] = _depthStencil.view;

    vk::FramebufferCreateInfo framebufferCreateInfo;
    framebufferCreateInfo.renderPass = renderPass;
    framebufferCreateInfo.attachmentCount = 2;
    framebufferCreateInfo.pAttachments = attachments;
    framebufferCreateInfo.width = _size.width;
    framebufferCreateInfo.height = _size.height;
    framebufferCreateInfo.layers = 1;

    // Create frame buffers for every swap chain image
    framebuffers = swapChain.createFramebuffers(framebufferCreateInfo);
}

void BaseWindow::SetupRenderPass() {
    if (renderPass) {
        device.destroyRenderPass(renderPass);
    }

    std::vector<vk::AttachmentDescription> attachments;
    attachments.resize(2);

    // Color attachment
    attachments[0].format = colorformat;
    attachments[0].loadOp = vk::AttachmentLoadOp::eClear;
    attachments[0].storeOp = vk::AttachmentStoreOp::eStore;
    attachments[0].initialLayout = vk::ImageLayout::eUndefined;
    attachments[0].finalLayout = vk::ImageLayout::ePresentSrcKHR;

    // Depth attachment
    attachments[1].format = depthFormat;
    attachments[1].loadOp = vk::AttachmentLoadOp::eClear;
    attachments[1].storeOp = vk::AttachmentStoreOp::eDontCare;
    attachments[1].stencilLoadOp = vk::AttachmentLoadOp::eClear;
    attachments[1].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    attachments[1].initialLayout = vk::ImageLayout::eUndefined;
    attachments[1].finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

    // Only one depth attachment, so put it first in the references
    vk::AttachmentReference depthReference;
    depthReference.attachment = 1;
    depthReference.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

    std::vector<vk::AttachmentReference> colorAttachmentReferences;
    {
        vk::AttachmentReference colorReference;
        colorReference.attachment = 0;
        colorReference.layout = vk::ImageLayout::eColorAttachmentOptimal;
        colorAttachmentReferences.push_back(colorReference);
    }


    using vPSFB = vk::PipelineStageFlagBits;
    using vAFB = vk::AccessFlagBits;
    std::vector<vk::SubpassDependency> subpassDependencies{
        {
            0, VK_SUBPASS_EXTERNAL,
            vPSFB::eColorAttachmentOutput, vPSFB::eBottomOfPipe,
            vAFB::eColorAttachmentRead | vAFB::eColorAttachmentWrite, vAFB::eMemoryRead,
            vk::DependencyFlagBits::eByRegion
        },
        {
            VK_SUBPASS_EXTERNAL, 0,
            vPSFB::eBottomOfPipe, vPSFB::eColorAttachmentOutput,
            vAFB::eMemoryRead, vAFB::eColorAttachmentRead | vAFB::eColorAttachmentWrite,
            vk::DependencyFlagBits::eByRegion
        },
    };
    std::vector<vk::SubpassDescription> subpasses{
        {
            {}, vk::PipelineBindPoint::eGraphics,
            // Input attachment references
            0, nullptr,
            // Color / resolve attachment references
            (uint32_t)colorAttachmentReferences.size(), colorAttachmentReferences.data(), nullptr,
            // Depth stecil attachment reference,
            &depthReference,
            // Preserve attachments
            0, nullptr
        },
    };

    vk::RenderPassCreateInfo renderPassInfo;
    renderPassInfo.attachmentCount = (uint32_t)attachments.size();
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = (uint32_t)subpasses.size();
    renderPassInfo.pSubpasses = subpasses.data();
    renderPassInfo.dependencyCount = (uint32_t)subpassDependencies.size();
    renderPassInfo.pDependencies = subpassDependencies.data();
    renderPass = device.createRenderPass(renderPassInfo);
}

void BaseWindow::AddRenderWaitSemaphore(const vk::Semaphore& semaphore, const vk::PipelineStageFlags& waitStages) {
    renderWaitSemaphores.push_back(semaphore);
    renderWaitStages.push_back(waitStages);
}

void BaseWindow::DrawCurrentCommandBuffer() {
    vk::Fence fence = swapChain.getSubmitFence();
    {
        uint32_t fenceIndex = currentBuffer;
        context.dumpster.push_back([fenceIndex, this] { swapChain.clearSubmitFence(fenceIndex); });
    }

    // Command buffer(s) to be sumitted to the queue
    context.emptyDumpster(fence);
    {
        vk::SubmitInfo submitInfo;
        submitInfo.waitSemaphoreCount = (uint32_t)renderWaitSemaphores.size();
        submitInfo.pWaitSemaphores = renderWaitSemaphores.data();
        submitInfo.pWaitDstStageMask = renderWaitStages.data();

        submitInfo.signalSemaphoreCount = (uint32_t)renderSignalSemaphores.size();
        submitInfo.pSignalSemaphores = renderSignalSemaphores.data();
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = commandBuffers.data() + currentBuffer;
        // Submit to queue
        context.queue.submit(submitInfo, fence);
    }

    context.recycle();
}

void BaseWindow::Draw() {
    // Get next image in the swap chain (back/front buffer)
    PrepareFrame();
    // Execute the compiled command buffer for the current swap chain image
    DrawCurrentCommandBuffer();
    // Push the rendered frame to the surface
    SubmitFrame();
}

std::chrono::microseconds BaseWindow::Render()
{
    if (!_prepared)
        return std::chrono::microseconds(0);

    auto tStart = std::chrono::high_resolution_clock::now();
    Draw();
    auto tEnd = std::chrono::high_resolution_clock::now();

    auto elapsedTime = tEnd - tStart;
    return std::chrono::duration_cast<std::chrono::microseconds>(elapsedTime);
}

void BaseWindow::Update(float deltaTime)
{
    camera.update(deltaTime);
    if (camera.moving())
        viewUpdated = true;

    UpdateOverlay();

    // Check gamepad state
    const float deadZone = 0.0015f;
    // todo : check if gamepad is present
    // todo : time based and relative axis positions
    if (camera.type != Camera::CameraType::firstperson) {
        // Rotate
        if (std::abs(_gamePadState.axisLeft.x) > deadZone)
        {
            camera.rotate(glm::vec3(0.0f, _gamePadState.axisLeft.x * 0.5f, 0.0f));
            viewUpdated = true;
        }

        if (std::abs(_gamePadState.axisLeft.y) > deadZone)
        {
            camera.rotate(glm::vec3(_gamePadState.axisLeft.y * 0.5f, 0.0f, 0.0f));
            viewUpdated = true;
        }

        // Zoom
        if (std::abs(_gamePadState.axisRight.y) > deadZone)
        {
            camera.dolly(_gamePadState.axisRight.y * 0.01f * zoomSpeed);
            viewUpdated = true;
        }
    }
    else
        viewUpdated |= camera.updatePad(_gamePadState.axisLeft, _gamePadState.axisRight, deltaTime);

    if (viewUpdated)
    {
        viewUpdated = false;
        ViewChanged();
    }
}

void BaseWindow::WindowResize(const glm::uvec2& newSize)
{
    if (!_prepared)
        return;

    _prepared = false;

    queue.waitIdle();
    device.waitIdle();

    // Recreate swap chain
    _size.width = newSize.x;
    _size.height = newSize.y;
    swapChain.create(_size, enableVsync);

    SetupDepthStencil();
    SetupFrameBuffer();
    SetupRenderPassBeginInfo();

    if (_settings.overlay)
        ui.resize(_size, framebuffers);

    // Notify derived class
    WindowResized();

    // Command buffers need to be recreated as they may store
    // references to the recreated frame buffer
    ClearCommandBuffers();
    AllocateCommandBuffers();
    BuildCommandBuffers();

    ViewChanged();

    _prepared = true;
}

void BaseWindow::UpdateOverlay()
{
    if (!_settings.overlay)
        return;

    ImGuiIO& io = ImGui::GetIO();

    io.DisplaySize = ImVec2(float(_size.width), float(_size.height));

    io.MousePos = ImVec2(mousePos.x, mousePos.y);
    io.MouseDown[0] = _mouseButtons.left;
    io.MouseDown[1] = _mouseButtons.right;

    ImGui::NewFrame();

    ImGui::Begin("WowGM", nullptr);
    ImGui::Text(context.deviceProperties.deviceName);
    ImGui::End();

    OnUpdateOverlay();

    ImGui::Render();

    ui.update();
}

void BaseWindow::MouseMoved(const glm::vec2& newPos)
{
    auto imgui = ImGui::GetIO();
    if (imgui.WantCaptureMouse)
    {
        mousePos = newPos;
        return;
    }

    glm::vec2 deltaPos = mousePos - newPos;
    if (deltaPos == vec2())
        return;

    const auto& dx = deltaPos.x;
    const auto& dy = deltaPos.y;
    bool handled = false;
    if (_settings.overlay)
    {
        ImGuiIO& io = ImGui::GetIO();
        handled = io.WantCaptureMouse;
    }

    if (handled)
        return;

    if (_mouseButtons.left)
    {
        camera.rotate(glm::vec3(dy * camera.rotationSpeed, -dx * camera.rotationSpeed, 0.0f));
        viewUpdated = true;
    }
    if (_mouseButtons.right)
    {
        camera.dolly(dy * .005f * zoomSpeed);
        viewUpdated = true;
    }
    if (_mouseButtons.middle) {
        camera.translate(glm::vec3(-dx * 0.01f, -dy * 0.01f, 0.0f));
        viewUpdated = true;
    }
    mousePos = newPos;
}

void BaseWindow::MouseScrolled(float delta)
{
    camera.translate(glm::vec3(0.0f, 0.0f, delta * 0.005f * zoomSpeed));
    viewUpdated = true;
}

void BaseWindow::KeyPressed(uint32_t key)
{
    if (camera.firstperson)
    {
        switch (key)
        {
            case GLFW_KEY_W:
                camera.keys.up = true;
                break;
            case GLFW_KEY_S:
                camera.keys.down = true;
                break;
            case GLFW_KEY_A:
                camera.keys.left = true;
                break;
            case GLFW_KEY_D:
                camera.keys.right = true;
                break;
        }
    }

    switch (key)
    {
        case GLFW_KEY_P:
            _paused = !_paused;
            break;

        case GLFW_KEY_F1:
            ui.visible = !ui.visible;
            break;

        case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(_window, 1);
            break;

        default:
            break;
    }
}

void BaseWindow::KeyReleased(uint32_t key)
{
    if (camera.firstperson)
    {
        switch (key)
        {
            case GLFW_KEY_W:
                camera.keys.up = false;
                break;
            case GLFW_KEY_S:
                camera.keys.down = false;
                break;
            case GLFW_KEY_A:
                camera.keys.left = false;
                break;
            case GLFW_KEY_D:
                camera.keys.right = false;
                break;
        }
    }
}

void BaseWindow::SetupWindow()
{
    bool fullscreen = false;

    // Check command line arguments
    for (int32_t i = 0; i < __argc; i++)
        if (__argv[i] == std::string("-fullscreen"))
            fullscreen = true;

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    auto monitor = glfwGetPrimaryMonitor();
    auto mode = glfwGetVideoMode(monitor);
    _size.width = mode->width;
    _size.height = mode->height;

    if (fullscreen)
        _window = glfwCreateWindow(_size.width, _size.height, "WowGM", monitor, nullptr);
    else
    {
        _size.width /= 2;
        _size.height /= 2;
        _window = glfwCreateWindow(_size.width, _size.height, "WowGM", nullptr, nullptr);
    }

    glfwSetWindowUserPointer(_window, this);
    glfwSetKeyCallback(_window, KeyboardHandler);
    glfwSetMouseButtonCallback(_window, MouseHandler);
    glfwSetCursorPosCallback(_window, MouseMoveHandler);
    glfwSetWindowCloseCallback(_window, CloseHandler);
    glfwSetFramebufferSizeCallback(_window, FramebufferSizeHandler);
    glfwSetScrollCallback(_window, MouseScrollHandler);
    if (!_window)
        throw std::runtime_error("Could not create window");
}

void BaseWindow::MouseAction(int button, int action, int mods)
{
    switch (button)
    {
        case GLFW_MOUSE_BUTTON_LEFT:
            _mouseButtons.left = action == GLFW_PRESS;
            break;
        case GLFW_MOUSE_BUTTON_RIGHT:
            _mouseButtons.right = action == GLFW_PRESS;
            break;
        case GLFW_MOUSE_BUTTON_MIDDLE:
            _mouseButtons.middle = action == GLFW_PRESS;
            break;
    }
}

void BaseWindow::KeyboardHandler(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    auto example = static_cast<BaseWindow*>(glfwGetWindowUserPointer(window));
    switch (action) {
    case GLFW_PRESS:
        example->KeyPressed(key);
        break;

    case GLFW_RELEASE:
        example->KeyReleased(key);
        break;

    default:
        break;
    }
}

void BaseWindow::MouseHandler(GLFWwindow* window, int button, int action, int mods)
{
    auto example = static_cast<BaseWindow*>(glfwGetWindowUserPointer(window));
    example->MouseAction(button, action, mods);
}

void BaseWindow::MouseMoveHandler(GLFWwindow* window, double posx, double posy)
{
    auto example = static_cast<BaseWindow*>(glfwGetWindowUserPointer(window));
    example->MouseMoved(glm::vec2(posx, posy));
}

void BaseWindow::MouseScrollHandler(GLFWwindow* window, double xoffset, double yoffset)
{
    auto example = static_cast<BaseWindow*>(glfwGetWindowUserPointer(window));
    example->MouseScrolled((float)yoffset);
}

void BaseWindow::CloseHandler(GLFWwindow* window)
{
    auto example = static_cast<BaseWindow*>(glfwGetWindowUserPointer(window));
    example->_prepared = false;
    glfwSetWindowShouldClose(window, 1);
}

void BaseWindow::FramebufferSizeHandler(GLFWwindow* window, int width, int height)
{
    auto example = static_cast<BaseWindow*>(glfwGetWindowUserPointer(window));
    example->WindowResize(glm::uvec2(width, height));
}
