#pragma once

#include <string_view>
#include <cstdint>
#include <atomic>
#include <chrono>
#include <thread>
#include <mutex>
#include <array>
#include <list>
#include <queue>
#include <functional>
#include <future>

#include <glm/vec2.hpp>

#include <boost/circular_buffer.hpp>

#include "Defines.hpp"

#if PLATFORM == PLATFORM_WINDOWS
# define VK_USE_PLATFORM_WIN32_KHR
# include "VEZ.hpp"
# include "Shader.hpp"
#else
# pragma error "Not supported yet"
#endif

struct GLFWwindow;

namespace vez
{
    class Instance;
    class PhysicalDevice;
    class Device;

    class Pipeline;
    class CommandBuffer;

    class Image;
    class ImageView;
    class Sampler;

    class Buffer;

    class Framebuffer;
}

class GameWindow
{
public:
    explicit GameWindow(std::string_view title, std::uint32_t width = 1024, std::uint32_t height = 768);
    virtual ~GameWindow();

public:
    void InitializeWindow(bool fullScreen = false);
    void InitializeVulkan();
    void InitializeInterface();

    void ToggleVSync();
    void ToggleFullscreen(std::int32_t selectedMonitor = -1);

    void OnResize(std::uint32_t width, std::uint32_t height);
    void Draw();
    void OnKeyUp(int key, int mods);
    void OnKeyDown(int key, int mods);
    void OnMouseUp(int button, float x, float y);
    void OnMouseDown(int button, float x, float y);
    void OnMouseScroll(float x, float y);

    void PrepareForRelease();

    void Run();
    void Update(std::chrono::microseconds previousFrameTime);

    void DrawLoadingScreen();

    void UpdateCommandBuffers();
    void UpdateGeometryBuffers();

private:
    void _UpdateInterfaceCommandBuffers();
    void _UpdateInterfaceGeometryBuffers();

    void _DrawFrameTimeHistogram();

    VkResult _InitializeVulkanInstance();
    VkResult _SelectVulkanPhysicalDevice();
    VkResult _InitializeVulkanDevice();
    VkResult _InitializeVulkanSwapchain();
    VkResult _InitializeVulkanFramebuffer();
    VkResult _InitializeInterfacePipeline();

    void _PrepareDelayedDestruction(VkFence fence);
    void _ExecuteDelayedDestructionRequests();

    template <typename T>
    inline void DelayedReleaseResource(T const& value, std::function<void(T)> destructor)
    {
        constexpr bool isVulkanObject = vez::traits::vulkan_type<T>::type != VK_OBJECT_TYPE_UNKNOWN;
        constexpr bool isVezObject = !std::is_same<typename vez::traits::vez_type<T>::underlying_type, std::nullptr_t>::value;

        if constexpr (isVulkanObject || isVezObject)
        {
            if (value == VK_NULL_HANDLE)
                return;
        }

        _dumpster.push_back([=]() -> void {
            destructor(value);
        });
    }

private:

    void(GameWindow::*_interfaceDrawCallback)() = &GameWindow::DrawLoadingScreen;

    std::string_view _title;
    std::string_view _engineName = "WowGM";

    GLFWwindow* _window;

    std::int32_t _previousWindowData[4] = { 0, 0, 0, 0 };
    std::uint32_t _width;
    std::uint32_t _height;

    bool _vsync = false;

    vez::Instance*       _instance = nullptr;
    vez::PhysicalDevice* _physicalDevice = nullptr;
    vez::Device*         _device = nullptr;
    vez::Queue*          _graphicsQueue = nullptr;
    vez::Swapchain*      _swapchain = nullptr;

    struct {
        // Data related to the framebuffer
        struct {
            // Multiple images, one for each of the sample
            std::vector<vez::Image*> ColorImages;
            std::vector<vez::ImageView*> ColorImageViews;

            // This is only used if multisampling is enabled and is the target to present to the user.
            vez::Image* ResolveImage = nullptr;
            vez::ImageView* ResolveImageView = nullptr;

            vez::Framebuffer* Handle = nullptr;
            struct {
                VkSampleCountFlagBits Sampling = VK_SAMPLE_COUNT_1_BIT;
            } Parameters;
        } Framebuffer;

        // Data related to the font texture.
        struct {
            vez::Image* Texture = nullptr;
            vez::Sampler* Sampler = nullptr;
            vez::ImageView* TextureView = nullptr;
        } Font;

        // Data related to primitive buffers
        struct PrimitiveWrapper {
            vez::Buffer* Buffer = nullptr;
            uint32_t PrimitiveCount = 0;
        } Vertex, Index;

        // Data related to the frames
        struct {
            constexpr static const uint32_t MAX_FRAMES = 3;

            uint32_t Index = 0;
            std::array<vez::CommandBuffer*, MAX_FRAMES> CommandBuffers;
        } Frames;

        // Data related to the rendering pipeline
        struct {
            vez::Pipeline* Handle = nullptr;
            std::array<vez::Shader*, 2> Shaders;

            struct {
                glm::vec2 Scale;
                glm::vec2 Translate;
            } PushConstants;
        } Pipeline;
    } __Interface; // Not yet wired

    VkSurfaceKHR _surface = VK_NULL_HANDLE;
    struct {
        vez::Image* ColorImage = nullptr;
        vez::ImageView* ColorImageView = nullptr;

        vez::Image* DepthStencilImage = nullptr;
        vez::ImageView* DepthStencilImageView = nullptr;

        vez::Framebuffer* Handle = nullptr;
        struct {
            VkSampleCountFlagBits Sampling = VK_SAMPLE_COUNT_1_BIT;
        } Parameters;
    } _frameBuffer;

    vez::Buffer* _interfaceVertexBuffer = nullptr;
    vez::Buffer* _interfaceIndexBuffer = nullptr;

    struct {
        std::array<vez::CommandBuffer*, 3> Interface{ nullptr, nullptr, nullptr }; // One command buffer per frame
        std::uint32_t FrameIndex = 0;
    } _commandBuffers;

    struct {
        vez::Pipeline* Handle = nullptr;
        // ...
    } _geometryPipeline;

    struct {
        vez::Pipeline* Handle = nullptr;

        struct {
            glm::vec2 Scale;
            glm::vec2 Translate;
        } PushConstants;

        std::array<vez::Shader*, 2> Shaders;
    } _interfacePipeline;

    std::once_flag glfwInitializer;

    vez::Image* _interfaceFontTexture = nullptr;
    vez::ImageView* _interfaceFontTextureView = nullptr;
    vez::Sampler* _interfaceFontSampler = nullptr;

    std::uint32_t _interfaceVertexCount = 0;
    std::uint32_t _interfaceIndexCount = 0;

    std::list<std::function<void()>> _dumpster;
    std::queue<std::pair<VkFence, std::function<void()>>> _recycler;

    std::atomic<bool> _mpqSystemInitialized;
    boost::circular_buffer<float> _frameDrawTimes;
    bool _showFramesMetrics = true;
};
