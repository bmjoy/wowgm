#pragma once

#include <graphics/vulkan/VK.hpp>
#include <vector>

namespace gfx::vk
{
    class Image;
    class ImageView;
    class Framebuffer;
    class RenderPass;

    struct SwapchainSupport
    {
        VkSurfaceCapabilitiesKHR capabilities = {};
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    class Swapchain final
    {
    public:
        ~Swapchain();

        static VkResult Create(Device* pDevice, const SwapchainCreateInfo* pCreateInfo, Swapchain** ppSwapchain);

        Device* GetDevice() { return _device; }
        VkSwapchainKHR GetHandle() { return _handle; }

        const SwapchainCreateInfo& GetCreateInfo() { return _createInfo; }

        VkSurfaceFormatKHR GetFormat() { return _format; }

        VkSurfaceKHR GetSurface() const { return _surface; }

        VkResult AcquireNextImage(uint32_t* pImageIndex, VkSemaphore* pImageAcquiredSemaphore);

        Image* GetImage(uint32_t index);
        uint32_t GetImageIndex(Image* image);

        ImageView* GetImageView(uint32_t index);

        VkExtent2D const& GetExtent() const;

        Framebuffer* CreateFrameBuffer(uint32_t frameIndex, RenderPass* renderPass);

    private:
        VkResult Allocate();

        VkSwapchainKHR _handle = VK_NULL_HANDLE;
        VkSurfaceFormatKHR _format{};
        VkSurfaceKHR _surface = VK_NULL_HANDLE;

        SwapchainSupport _swapchainSupport;
        SwapchainCreateInfo _createInfo;

        Device* _device = nullptr;

        std::vector<Image*> _images;
        std::vector<ImageView*> _imageViews;
    };
}