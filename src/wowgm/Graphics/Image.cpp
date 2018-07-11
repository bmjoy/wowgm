#include "Image.hpp"

namespace wowgm::graphics
{
    Image::Image(VkImage image) : _image(image)
    {

    }

    VkImage Image::GetVkImage()
    {
        return _image;
    }
}