#include <graphics/vulkan/ImageView.hpp>
#include <graphics/vulkan/Image.hpp>
#include <graphics/vulkan/Device.hpp>

namespace gfx::vk
{
    ImageView::~ImageView()
    {
        if (_handle != VK_NULL_HANDLE)
            vkDestroyImageView(_device->GetHandle(), _handle, nullptr);

        _handle = VK_NULL_HANDLE;
        _device = nullptr;
        _image = nullptr;
    }
}
