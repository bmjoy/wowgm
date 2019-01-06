#include <graphics/window.hpp>
#include <graphics/vulkan/Instance.hpp>

#include <shared/assert/assert.hpp>

#include <mutex>

#include <imgui/imgui.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#ifdef _WIN32
#undef WINAPI
#endif

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

namespace gfx
{
    Window::Window(int32_t width, int32_t height, std::string const& title)
    {
        static std::once_flag glfwInitFlag;
        std::call_once(glfwInitFlag, []() -> void {
            int32_t resultCode = glfwInit();
            if (resultCode != GLFW_TRUE) {
                // throw somehow
            }
        });

        // Prevent GLFW from creating stuff for us
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        // Disable window resize
        window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
        // if (position != glm::ivec2{ -1, -1 })
        //     glfwSetWindowPos(window, position.x, position.y);

        glfwSetWindowUserPointer(window, this);
        glfwSetKeyCallback(window, KeyboardHandler);
        glfwSetMouseButtonCallback(window, MouseButtonHandler);
        glfwSetCursorPosCallback(window, MouseMoveHandler);
        glfwSetWindowCloseCallback(window, CloseHandler);
        glfwSetFramebufferSizeCallback(window, FramebufferSizeHandler);
        glfwSetScrollCallback(window, MouseScrollHandler);
    }

    Window::~Window()
    {
        glfwDestroyWindow(window);

        static std::once_flag glfwCleaner;
        std::call_once(glfwCleaner, []() -> void {
            glfwTerminate();
        });
    }

    HWND Window::getHandle()
    {
        return glfwGetWin32Window(window);
    }

    void Window::setSizeLimits(const glm::uvec2& minSize, const glm::uvec2& maxSize)
    {
        glfwSetWindowSizeLimits(window, minSize.x, minSize.y, (maxSize.x != 0) ? maxSize.x : minSize.x, (maxSize.y != 0) ? maxSize.y : minSize.y);
    }

    void Window::runWindowLoop(std::function<void()> frameHandler)
    {
        while (glfwWindowShouldClose(window) == 0)
        {
            glfwPollEvents();

            onFrame();
            frameHandler();
        }
    }

#if defined(VK_NULL_HANDLE)
    VkSurfaceKHR Window::createSurface(vk::Instance* instance, const VkAllocationCallbacks* pAllocator)
    {
        VkSurfaceKHR surface;
        VkResult result = glfwCreateWindowSurface(instance->GetHandle(), window, pAllocator, &surface);
        if (result != VK_SUCCESS)
            shared::assert::throw_with_trace("An error occured while GLFW was creating a surface!");
        return surface;
    }

    std::vector<const char*> Window::getRequiredInstanceExtensions()
    {
        uint32_t count = 0;
        const char** names = glfwGetRequiredInstanceExtensions(&count);

        std::vector<const char*> result(names, names + count);
        return result;
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
        ImGuiIO& io = ImGui::GetIO();

        switch (action) {
            case GLFW_PRESS:
                onKeyPressed(key, mods);
                io.KeysDown[key] = true;
                break;
            case GLFW_RELEASE:
                onKeyReleased(key, mods);
                io.KeysDown[key] = false;
                break;
            default:
                break;
        }

    }

    void Window::showWindow(bool show)
    {
        if (show)
            glfwShowWindow(window);
        else
            glfwHideWindow(window);
    }
}