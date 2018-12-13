#pragma once

#include <vector>
#include <cstring>
#include <functional>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>

namespace gfx
{
    class Window
    {
        public:
            static void terminate();

            static std::vector<std::string> getRequiredInstanceExtensions();

            static vk::SurfaceKHR createWindowSurface(GLFWwindow* window, const vk::Instance& instance, const vk::AllocationCallbacks* pAllocator = nullptr);

            vk::SurfaceKHR createSurface(const vk::Instance& instance, const vk::AllocationCallbacks* pAllocator = nullptr);

            void createWindow(const glm::uvec2& size, const glm::ivec2& position = { -1, -1 });

            void destroyWindow();
            void makeCurrent() const;
            void present() const;
            void showWindow(bool show = true);

            void setTitle(const std::string& title);
            void setSizeLimits(const glm::uvec2& minSize, const glm::uvec2& maxSize = {});

            void runWindowLoop(const std::function<void()>& frameHandler);

            //
            // Event handlers are called by the GLFW callback mechanism and should not be called directly
            //

            virtual void onWindowResized(const glm::uvec2& newSize) = 0;
            virtual void onWindowClosed() = 0;

            // Keyboard handling
            virtual void onKeyEvent(int key, int scancode, int action, int mods);

            virtual void onKeyPressed(int key, int mods) = 0;
            virtual void onKeyReleased(int key, int mods) = 0;

            // Mouse handling
            virtual void onMouseButtonEvent(int button, int action, int mods);

            virtual void onMousePressed(int button, int mods) = 0;
            virtual void onMouseReleased(int button, int mods) = 0;
            virtual void onMouseMoved(const glm::vec2& newPos) = 0;
            virtual void onMouseScrolled(float delta) = 0;

        private:
            static void KeyboardHandler(GLFWwindow* window, int key, int scancode, int action, int mods);
            static void MouseButtonHandler(GLFWwindow* window, int button, int action, int mods);
            static void MouseMoveHandler(GLFWwindow* window, double posx, double posy);
            static void MouseScrollHandler(GLFWwindow* window, double xoffset, double yoffset);
            static void CloseHandler(GLFWwindow* window);
            static void FramebufferSizeHandler(GLFWwindow* window, int width, int height);

            GLFWwindow* window{ nullptr };
    };
}