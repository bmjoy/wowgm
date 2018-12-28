#pragma once

#include <graphics/window.hpp>

#include "InterfaceRenderer.hpp"

namespace gfx::vk {
    class Instance;
    class Device;
    class Swapchain;
    class Pipeline;
}

namespace wowgm
{
    class Window final : public gfx::Window
    {
    public:
        Window(int32_t width, int32_t height, std::string const& title);
        ~Window();

    protected:
        void onWindowResized(const glm::uvec2& newSize) override;
        void onWindowClosed() override;

        void onKeyPressed(int key, int mods) override;
        void onKeyReleased(int key, int mods) override;

        void onMousePressed(int button, int mods) override;
        void onMouseReleased(int button, int mods) override;
        void onMouseMoved(const glm::vec2& newPos) override;
        void onMouseScrolled(float delta) override;

    public:
        gfx::vk::Device* GetDevice() const { return _device; }
        gfx::vk::Swapchain* GetSwapchain() const { return _swapchain; }

    private:
        gfx::vk::Instance* _instance;
        gfx::vk::Device* _device;
        gfx::vk::Swapchain* _swapchain;
        VkSurfaceKHR _surface;

        InterfaceRenderer* _interfaceRenderer;
    };
}