#pragma once

#include <vector>
#include <cstring>
#include <functional>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

namespace gfx
{
    namespace vk
    {
        class Instance;
    }

    class Window
    {
        public:
            Window(int32_t width, int32_t height, std::string const& title);
            virtual ~Window();

        protected:
#if defined(VK_NULL_HANDLE)
            static std::vector<const char*> getRequiredInstanceExtensions();

            VkSurfaceKHR createSurface(vk::Instance* instance, const VkAllocationCallbacks* pAllocator = nullptr);
#endif

            //> Controls wether or not the window is visible.
            void showWindow(bool show = true);

            //> Sets the size of the window.
            void setTitle(const std::string& title);

            //> Sets minimum and maximum size for the window.
            void setSizeLimits(const glm::uvec2& minSize, const glm::uvec2& maxSize = {});

        public:
            //> Blocks forever until the window is getting closed, invoking the callback provided.
            void runWindowLoop(std::function<void()> frameHandler);

            virtual void onFrame() = 0;

        private:
            void onMouseButtonEvent(int button, int action, int mods);
            void onKeyEvent(int key, int scancode, int action, int mods);

        protected:
            //> Called when the window is resized
            virtual void onWindowResized(const glm::uvec2& newSize) = 0;
            //> Called when the window is closing.
            virtual void onWindowClosed() = 0;

            //> Called when a key is pressed,
            virtual void onKeyPressed(int key, int mods) = 0;
            //> Called when a key is released.
            virtual void onKeyReleased(int key, int mods) = 0;

            //> Called when a mouse button has been pressed.
            virtual void onMousePressed(int button, int mods) = 0;
            //> Called when a mouse button is released.
            virtual void onMouseReleased(int button, int mods) = 0;
            //> Called when the cursor has moved.
            virtual void onMouseMoved(const glm::vec2& newPos) = 0;
            //> Called when the scrolling wheel has been used.
            virtual void onMouseScrolled(float delta) = 0;

        private:

            //> Event handling dispatchers.
            static void KeyboardHandler(GLFWwindow* window, int key, int scancode, int action, int mods);
            static void MouseButtonHandler(GLFWwindow* window, int button, int action, int mods);
            static void MouseMoveHandler(GLFWwindow* window, double posx, double posy);
            static void MouseScrollHandler(GLFWwindow* window, double xoffset, double yoffset);
            static void CloseHandler(GLFWwindow* window);
            static void FramebufferSizeHandler(GLFWwindow* window, int width, int height);

            //> The handle of the underlying GLFW window object.
            GLFWwindow* window{ nullptr };
    };
}