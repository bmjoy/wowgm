#include "Image.hpp"

namespace wowgm::graphics
{
    Image::Image(VkImage image) : _image(image)
    {

    }

    Image::Image()
    {

    }

    VkImage Image::GetVkImage()
    {
        return _image;
    }
}