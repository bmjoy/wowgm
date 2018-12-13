#include "glfw.hpp"

#include <mutex>

namespace glfw {

bool Window::init()
{
    return GLFW_TRUE == glfwInit();
}

void Window::terminate()
{
    glfwTerminate();
}

void Window::createWindow(const glm::uvec2& size, const glm::ivec2& position)
{
    // Disable window resize
    window = glfwCreateWindow(size.x, size.y, "Window Title", nullptr, nullptr);
    if (position != glm::ivec2{ -1, -1 })
        glfwSetWindowPos(window, position.x, position.y);

    glfwSetWindowUserPointer(window, this);
    glfwSetKeyCallback(window, KeyboardHandler);
    glfwSetMouseButtonCallback(window, MouseButtonHandler);
    glfwSetCursorPosCallback(window, MouseMoveHandler);
    glfwSetWindowCloseCallback(window, CloseHandler);
    glfwSetFramebufferSizeCallback(window, FramebufferSizeHandler);
    glfwSetScrollCallback(window, MouseScrollHandler);
}

#if defined(VULKAN_HPP)
std::vector<std::string> Window::getRequiredInstanceExtensions()
{
    uint32_t count = 0;
    const char** names = glfwGetRequiredInstanceExtensions(&count);

    std::vector<std::string> result(names, names + count);
    return result;
}

vk::SurfaceKHR Window::createWindowSurface(GLFWwindow* window, const vk::Instance& instance, const vk::AllocationCallbacks* pAllocator) {
    VkSurfaceKHR rawSurface;
    vk::Result result =
        static_cast<vk::Result>(glfwCreateWindowSurface((VkInstance)instance, window, reinterpret_cast<const VkAllocationCallbacks*>(pAllocator), &rawSurface));
    return vk::createResultValue(result, rawSurface, "vk::CommandBuffer::begin");
}
#endif

void Window::KeyboardHandler(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    Window* example = (Window*)glfwGetWindowUserPointer(window);
    example->onKeyEvent(key, scancode, action, mods);
}

void Window::MouseButtonHandler(GLFWwindow* window, int button, int action, int mods)
{
    Window* example = (Window*)glfwGetWindowUserPointer(window);
    example->onMouseButtonEvent(button, action, mods);
}

void Window::MouseMoveHandler(GLFWwindow* window, double posx, double posy)
{
    Window* example = (Window*)glfwGetWindowUserPointer(window);
    example->onMouseMoved(glm::vec2(posx, posy));
}

void Window::MouseScrollHandler(GLFWwindow* window, double xoffset, double yoffset)
{
    Window* example = (Window*)glfwGetWindowUserPointer(window);
    example->onMouseScrolled((float)yoffset);
}

void Window::CloseHandler(GLFWwindow* window)
{
    Window* example = (Window*)glfwGetWindowUserPointer(window);
    example->onWindowClosed();
}

void Window::FramebufferSizeHandler(GLFWwindow* window, int width, int height)
{
    Window* example = (Window*)glfwGetWindowUserPointer(window);
    example->onWindowResized(glm::uvec2(width, height));
}

void Window::onMouseButtonEvent(int button, int action, int mods)
{
    switch (action) {
        case GLFW_PRESS:
            onMousePressed(button, mods);
            break;
        case GLFW_RELEASE:
            onMouseReleased(button, mods);
            break;
        default:
            break;
    }
}

void Window::onKeyEvent(int key, int scancode, int action, int mods)
{
    switch (action) {
        case GLFW_PRESS:
            onKeyPressed(key, mods);
            break;
        case GLFW_RELEASE:
            onKeyReleased(key, mods);
            break;
        default:
            break;
    }
}

void destroyWindow()
            {
                glfwDestroyWindow(window);
                window = nullptr;
            }

            void makeCurrent() const { glfwMakeContextCurrent(window); }

            void present() const { glfwSwapBuffers(window); }

            void showWindow(bool show = true)
            {
                if (show)
                    glfwShowWindow(window);
                else
                    glfwHideWindow(window);
            }

}  // namespace glfw

