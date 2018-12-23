#include <graphics/window.hpp>

#include <mutex>

namespace gfx
{
    void Window::setSizeLimits(const glm::uvec2& minSize, const glm::uvec2& maxSize)
    {
        glfwSetWindowSizeLimits(window, minSize.x, minSize.y, (maxSize.x != 0) ? maxSize.x : minSize.x, (maxSize.y != 0) ? maxSize.y : minSize.y);
    }

    void Window::runWindowLoop(const std::function<void()>& frameHandler)
    {
        while (0 == glfwWindowShouldClose(window))
        {
            glfwPollEvents();
            frameHandler();
        }
    }

    void Window::terminate()
    {
        glfwTerminate();
    }

    void Window::createWindow(const glm::uvec2& size, const glm::ivec2& position)
    {
        static std::once_flag glfwInitFlag;
        std::call_once(glfwInitFlag, []() -> void {
            int32_t resultCode = glfwInit();
            if (resultCode != GLFW_TRUE)
            {
                // throw somehow
            }
        });

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

    vk::SurfaceKHR Window::createSurface(const vk::Instance& instance, const vk::AllocationCallbacks* pAllocator)
    {
        return createWindowSurface(window, instance, pAllocator);
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


    void Window::setTitle(const std::string& title) {
        glfwSetWindowTitle(window, title.c_str());
    }

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

    void Window::destroyWindow()
    {
        glfwDestroyWindow(window);
        window = nullptr;
    }

    void Window::makeCurrent() const {
        glfwMakeContextCurrent(window);
    }

    void Window::present() const {
        glfwSwapBuffers(window);
    }

    void Window::showWindow(bool show)
    {
        if (show)
            glfwShowWindow(window);
        else
            glfwHideWindow(window);
    }
}