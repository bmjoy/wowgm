#include "Renderer.hpp"

#include <graphics/vulkan/Device.hpp>
#include <graphics/vulkan/Instance.hpp>
#include <graphics/vulkan/Swapchain.hpp>

namespace wowgm
{
    using namespace gfx::vk;

    Renderer::Renderer(Swapchain* swapchain)
        : _swapchain(swapchain)
    {

    }

    void Renderer::SetSwapchain(Swapchain* swapchain)
    {
        // Update the swapchain
        _swapchain = swapchain;
    }

    Renderer::~Renderer()
    {
    }

    Swapchain* Renderer::GetSwapchain() const {
        return _swapchain;
    }

    Device* Renderer::GetDevice() const {
        return GetSwapchain()->GetDevice();
    }

    Instance* Renderer::GetInstance() const {
        return GetDevice()->GetInstance();
    }
}