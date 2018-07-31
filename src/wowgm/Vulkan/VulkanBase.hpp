/*
* Vulkan Example base class
*
* Copyright (C) 2016 by Sascha Willems - www.saschawillems.de
*
* This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/

#pragma once

#include "Common.hpp"

#include "VKS.hpp"
#include "Helpers.hpp"
#include "Filesystem.hpp"
#include "Model.hpp"
#include "Shaders.hpp"
#include "Pipelines.hpp"
#include "Texture.hpp"

#include "UI.hpp"
#include "Utils.hpp"
#include "Camera.hpp"
#include "Compute.hpp"

#define GAMEPAD_BUTTON_A 0x1000
#define GAMEPAD_BUTTON_B 0x1001
#define GAMEPAD_BUTTON_X 0x1002
#define GAMEPAD_BUTTON_Y 0x1003
#define GAMEPAD_BUTTON_L1 0x1004
#define GAMEPAD_BUTTON_R1 0x1005
#define GAMEPAD_BUTTON_START 0x1006

namespace vkx {

    struct UpdateOperation {
        const vk::Buffer buffer;
        const vk::DeviceSize size;
        const vk::DeviceSize offset;
        const uint32_t* data;

        template <typename T>
        UpdateOperation(const vk::Buffer& buffer, const T& data, vk::DeviceSize offset = 0)
            : buffer(buffer)
            , size(sizeof(T))
            , offset(offset)
            , data((uint32_t*)&data) {
            assert(0 == (sizeof(T) % 4));
            assert(0 == (offset % 4));
        }
    };

    class BaseWindow
    {
        protected:
            BaseWindow();
            ~BaseWindow();

            using vAF = vk::AccessFlagBits;
            using vBU = vk::BufferUsageFlagBits;
            using vDT = vk::DescriptorType;
            using vF = vk::Format;
            using vIL = vk::ImageLayout;
            using vIT = vk::ImageType;
            using vIVT = vk::ImageViewType;
            using vIU = vk::ImageUsageFlagBits;
            using vIA = vk::ImageAspectFlagBits;
            using vMP = vk::MemoryPropertyFlagBits;
            using vPS = vk::PipelineStageFlagBits;
            using vSS = vk::ShaderStageFlagBits;

        public:
            void Run();
            // Called if the window is resized and some resources have to be recreatesd
            void WindowResize(const glm::uvec2& newSize);

        private:
            // Set to true when the debug marker extension is detected
            bool enableDebugMarkers{ false };
            // fps timer (one second interval)
            float fpsTimer = 0.0f;
            // Get window title with example name, device, et.
            std::string getWindowTitle();

        protected:
            bool enableVsync { false };
            // Command buffers used for rendering
            std::vector<vk::CommandBuffer> commandBuffers;
            std::vector<vk::ClearValue> clearValues;
            vk::RenderPassBeginInfo renderPassBeginInfo;
            vk::Viewport viewport() { return vks::util::viewport(_size); }
            vk::Rect2D scissor() { return vks::util::rect2D(_size); }

            virtual void ClearCommandBuffers() final;
            virtual void AllocateCommandBuffers() final;
            virtual void SetupRenderPassBeginInfo();
            virtual void BuildCommandBuffers();

        protected:

            // Color buffer format
            vk::Format colorformat{ vk::Format::eB8G8R8A8Unorm };

            // Depth buffer format...  selected during Vulkan initialization
            vk::Format depthFormat{ vk::Format::eUndefined };

            // Global render pass for frame buffer writes
            vk::RenderPass renderPass;

            // List of available frame buffers (same as number of swap chain images)
            std::vector<vk::Framebuffer> framebuffers;
            // Active frame buffer index
            uint32_t currentBuffer = 0;
            // Descriptor set pool
            vk::DescriptorPool descriptorPool;

            void AddRenderWaitSemaphore(const vk::Semaphore& semaphore, const vk::PipelineStageFlags& waitStages = vk::PipelineStageFlagBits::eBottomOfPipe);

            std::vector<vk::Semaphore> renderWaitSemaphores;
            std::vector<vk::PipelineStageFlags> renderWaitStages;
            std::vector<vk::Semaphore> renderSignalSemaphores;

            vks::Context context;
            const vk::PhysicalDevice& physicalDevice{ context.physicalDevice };
            const vk::Device& device{ context.device };
            const vk::Queue& queue{ context.queue };
            const vk::PhysicalDeviceFeatures& deviceFeatures{ context.deviceFeatures };
            vk::PhysicalDeviceFeatures& enabledFeatures{ context.enabledFeatures };
            vkx::ui::UIOverlay ui{ context };

            vk::SurfaceKHR surface;
            // Wraps the swap chain to present images (framebuffers) to the windowing system
            vks::SwapChain swapChain;

            // Synchronization semaphores
            struct {
                // Swap chain image presentation
                vk::Semaphore acquireComplete;
                // Command buffer submission and execution
                vk::Semaphore renderComplete;
                // UI buffer submission and execution
                vk::Semaphore overlayComplete;
    #if 0
                vk::Semaphore transferComplete;
    #endif
            } semaphores;

            // Returns the base asset path (for shaders, models, textures) depending on the os
            const std::string& getAssetPath() { return ::vkx::getAssetPath(); }

        protected:
            struct Settings {
                bool validation = false;
                bool fullscreen = false;
                bool vsync      = false;
                bool overlay    = true;
            } _settings;

            struct {
                bool left   = false;
                bool right  = false;
                bool middle = false;
            } _mouseButtons;

            struct {
                bool active = false;
            } _benchmark;

            // Command buffer pool
            vk::CommandPool cmdPool;

            bool        _prepared    = false;
            uint32_t    _version     = VK_MAKE_VERSION(1, 1, 0);
            vk::Extent2D _size       { 1280, 720 };
            uint32_t&    _width      { _size.width };
            uint32_t&    _height     { _size.height };

            vk::ClearColorValue defaultClearColor = vks::util::clearColor(glm::vec4({ 0.025f, 0.025f, 0.025f, 1.0f }));
            vk::ClearDepthStencilValue defaultClearDepth{ 1.0f, 0 };

            // Defines a frame rate independent timer value clamped from -1.0...1.0
            // For use in animations, rotations, etc.
            float _timer = 0.0f;
            float _timerSpeed = 0.25f; // Multiplier for speeding up (or slowing down) the global timer

            bool _paused = false;

            // Use to adjust mouse rotation speed
            float rotationSpeed = 1.0f;
            // Use to adjust mouse zoom speed
            float zoomSpeed = 1.0f;

            Camera camera;
            glm::vec2 mousePos;
            bool viewUpdated{ false };

            std::string _title;
            std::string _name;
            vks::Image _depthStencil;

            // Gamepad state (only one pad supported)
            struct
            {
                glm::vec2 axisLeft = glm::vec2(0.0f);
                glm::vec2 axisRight = glm::vec2(0.0f);
                float rz{ 0.0f };
            } _gamePadState;

            void UpdateOverlay();

            virtual void OnUpdateOverlay() { }
            virtual void OnSetupInterface(vkx::ui::UIOverlayCreateInfo& uiCreateInfo) { }

            virtual void InitVulkan();
            virtual void SetupSwapchain();
            virtual void SetupWindow();
            virtual void GetEnabledFeatures();
            virtual void Draw();
            virtual void Render();
            virtual void Update(float deltaTime);
            virtual void ViewChanged() { }

            // Called when the window has been resized
            // Can be overriden in derived class to recreate or rebuild resources attached to the frame buffer / swapchain
            virtual void WindowResized() { }

            void SetupDepthStencil();

            virtual void SetupFrameBuffer();

            // Setup a default render pass
            // Can be overriden in derived class to setup a custom render pass (e.g. for MSAA)
            virtual void SetupRenderPass();

            void SetupInterface();

            virtual void UpdateCommandBufferPreDraw(const vk::CommandBuffer& commandBuffer) { }
            virtual void UpdateDrawCommandBuffer(const vk::CommandBuffer& commandBuffer) { }
            virtual void UpdateCommandBufferPostDraw(const vk::CommandBuffer& commandBuffer) { }
            void DrawCurrentCommandBuffer();

            virtual void Prepare();

            virtual void LoadAssets() { }

            bool PlatformLoopCondition();

            // Start the main render loop
            void RenderLoop();

            // Prepare the frame for workload submission
            // - Acquires the next image from the swap chain
            // - Submits a post present barrier
            // - Sets the default wait and signal semaphores
            void PrepareFrame();

            // Submit the frames' workload
            // - Submits the text overlay (if enabled)
            // -
            void SubmitFrame();

            virtual const glm::mat4& getProjection() const { return camera.matrices.perspective; }

            virtual const glm::mat4& getView() const { return camera.matrices.view; }

            // Called if a key is pressed
            // Can be overriden in derived class to do custom key handling
            virtual void KeyPressed(uint32_t key);
            virtual void KeyReleased(uint32_t key);

            virtual void MouseMoved(const glm::vec2& newPos);
            virtual void MouseScrolled(float delta);

        private:

            // OS specific
            GLFWwindow* _window{ nullptr };

            // Keyboard movement handler
            virtual void MouseAction(int buttons, int action, int mods);
            static void KeyboardHandler(GLFWwindow* window, int key, int scancode, int action, int mods);
            static void MouseHandler(GLFWwindow* window, int button, int action, int mods);
            static void MouseMoveHandler(GLFWwindow* window, double posx, double posy);
            static void MouseScrollHandler(GLFWwindow* window, double xoffset, double yoffset);
            static void FramebufferSizeHandler(GLFWwindow* window, int width, int height);
            static void CloseHandler(GLFWwindow* window);
    };
}  // namespace vkx
