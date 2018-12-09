#pragma once

#include <unordered_map>
#include <vulkan/vulkan.h>
#include <memory>

#include <vk_mem_alloc.h>

#include <boost/iterator/transform_iterator.hpp>

#include "Defines.hpp"

namespace vez
{
    typedef std::vector<uint32_t> DescriptorSetLayoutHash;

    inline bool IsDepthStencilFormat(VkFormat format)
    {
        switch (format)
        {
            case VK_FORMAT_D16_UNORM:
            case VK_FORMAT_D16_UNORM_S8_UINT:
            case VK_FORMAT_D24_UNORM_S8_UINT:
            case VK_FORMAT_D32_SFLOAT:
            case VK_FORMAT_D32_SFLOAT_S8_UINT:
                return true;

            default:
                return false;
        }
    }

    inline bool FormatHasStencil(VkFormat format)
    {
        switch (format)
        {
            case VK_FORMAT_D16_UNORM_S8_UINT:
            case VK_FORMAT_D24_UNORM_S8_UINT:
            case VK_FORMAT_D32_SFLOAT_S8_UINT:
                return true;

            default:
                return false;
        }
    }

    inline VkImageAspectFlags GetImageAspectFlags(VkFormat format)
    {
        switch (format)
        {
            case VK_FORMAT_D16_UNORM:
            case VK_FORMAT_D32_SFLOAT:
                return VK_IMAGE_ASPECT_DEPTH_BIT;

            case VK_FORMAT_D16_UNORM_S8_UINT:
            case VK_FORMAT_D24_UNORM_S8_UINT:
            case VK_FORMAT_D32_SFLOAT_S8_UINT:
                return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;

            default:
                return VK_IMAGE_ASPECT_COLOR_BIT;
        }
    }

    inline bool IsCompressedImageFormat(VkFormat format)
    {
        switch (format)
        {
            case VK_FORMAT_BC1_RGB_UNORM_BLOCK:
            case VK_FORMAT_BC1_RGB_SRGB_BLOCK:
            case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:
            case VK_FORMAT_BC1_RGBA_SRGB_BLOCK:
            case VK_FORMAT_BC2_UNORM_BLOCK:
            case VK_FORMAT_BC2_SRGB_BLOCK:
            case VK_FORMAT_BC3_UNORM_BLOCK:
            case VK_FORMAT_BC3_SRGB_BLOCK:
            case VK_FORMAT_BC4_UNORM_BLOCK:
            case VK_FORMAT_BC4_SNORM_BLOCK:
            case VK_FORMAT_BC5_UNORM_BLOCK:
            case VK_FORMAT_BC5_SNORM_BLOCK:
            case VK_FORMAT_BC6H_UFLOAT_BLOCK:
            case VK_FORMAT_BC6H_SFLOAT_BLOCK:
            case VK_FORMAT_BC7_UNORM_BLOCK:
            case VK_FORMAT_BC7_SRGB_BLOCK:
            case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK:
            case VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK:
            case VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK:
            case VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK:
            case VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK:
            case VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK:
            case VK_FORMAT_EAC_R11_UNORM_BLOCK:
            case VK_FORMAT_EAC_R11_SNORM_BLOCK:
            case VK_FORMAT_EAC_R11G11_UNORM_BLOCK:
            case VK_FORMAT_EAC_R11G11_SNORM_BLOCK:
            case VK_FORMAT_ASTC_4x4_UNORM_BLOCK:
            case VK_FORMAT_ASTC_4x4_SRGB_BLOCK:
            case VK_FORMAT_ASTC_5x4_UNORM_BLOCK:
            case VK_FORMAT_ASTC_5x4_SRGB_BLOCK:
            case VK_FORMAT_ASTC_5x5_UNORM_BLOCK:
            case VK_FORMAT_ASTC_5x5_SRGB_BLOCK:
            case VK_FORMAT_ASTC_6x5_UNORM_BLOCK:
            case VK_FORMAT_ASTC_6x5_SRGB_BLOCK:
            case VK_FORMAT_ASTC_6x6_UNORM_BLOCK:
            case VK_FORMAT_ASTC_6x6_SRGB_BLOCK:
            case VK_FORMAT_ASTC_8x5_UNORM_BLOCK:
            case VK_FORMAT_ASTC_8x5_SRGB_BLOCK:
            case VK_FORMAT_ASTC_8x6_UNORM_BLOCK:
            case VK_FORMAT_ASTC_8x6_SRGB_BLOCK:
            case VK_FORMAT_ASTC_8x8_UNORM_BLOCK:
            case VK_FORMAT_ASTC_8x8_SRGB_BLOCK:
            case VK_FORMAT_ASTC_10x5_UNORM_BLOCK:
            case VK_FORMAT_ASTC_10x5_SRGB_BLOCK:
            case VK_FORMAT_ASTC_10x6_UNORM_BLOCK:
            case VK_FORMAT_ASTC_10x6_SRGB_BLOCK:
            case VK_FORMAT_ASTC_10x8_UNORM_BLOCK:
            case VK_FORMAT_ASTC_10x8_SRGB_BLOCK:
            case VK_FORMAT_ASTC_10x10_UNORM_BLOCK:
            case VK_FORMAT_ASTC_10x10_SRGB_BLOCK:
            case VK_FORMAT_ASTC_12x10_UNORM_BLOCK:
            case VK_FORMAT_ASTC_12x10_SRGB_BLOCK:
            case VK_FORMAT_ASTC_12x12_UNORM_BLOCK:
            case VK_FORMAT_ASTC_12x12_SRGB_BLOCK:
                return true;

            default:
                return false;
        }
    }

    inline uint32_t GetUncompressedImageFormatSize(VkFormat format)
    {
        switch (format)
        {
            case VK_FORMAT_R4G4_UNORM_PACK8: return 1;
            case VK_FORMAT_R4G4B4A4_UNORM_PACK16: return 2;
            case VK_FORMAT_B4G4R4A4_UNORM_PACK16: return 2;
            case VK_FORMAT_R5G6B5_UNORM_PACK16: return 2;
            case VK_FORMAT_B5G6R5_UNORM_PACK16: return 2;
            case VK_FORMAT_R5G5B5A1_UNORM_PACK16: return 2;
            case VK_FORMAT_B5G5R5A1_UNORM_PACK16: return 2;
            case VK_FORMAT_A1R5G5B5_UNORM_PACK16: return 2;
            case VK_FORMAT_R8_UNORM: return 1;
            case VK_FORMAT_R8_SNORM: return 1;
            case VK_FORMAT_R8_USCALED: return 1;
            case VK_FORMAT_R8_SSCALED: return 1;
            case VK_FORMAT_R8_UINT: return 1;
            case VK_FORMAT_R8_SINT: return 1;
            case VK_FORMAT_R8_SRGB: return 1;
            case VK_FORMAT_R8G8_UNORM: return 2;
            case VK_FORMAT_R8G8_SNORM: return 2;
            case VK_FORMAT_R8G8_USCALED: return 2;
            case VK_FORMAT_R8G8_SSCALED: return 2;
            case VK_FORMAT_R8G8_UINT: return 2;
            case VK_FORMAT_R8G8_SINT: return 2;
            case VK_FORMAT_R8G8_SRGB: return 2;
            case VK_FORMAT_R8G8B8_UNORM: return 3;
            case VK_FORMAT_R8G8B8_SNORM: return 3;
            case VK_FORMAT_R8G8B8_USCALED: return 3;
            case VK_FORMAT_R8G8B8_SSCALED: return 3;
            case VK_FORMAT_R8G8B8_UINT: return 3;
            case VK_FORMAT_R8G8B8_SINT: return 3;
            case VK_FORMAT_R8G8B8_SRGB: return 3;
            case VK_FORMAT_B8G8R8_UNORM: return 3;
            case VK_FORMAT_B8G8R8_SNORM: return 3;
            case VK_FORMAT_B8G8R8_USCALED: return 3;
            case VK_FORMAT_B8G8R8_SSCALED: return 3;
            case VK_FORMAT_B8G8R8_UINT: return 3;
            case VK_FORMAT_B8G8R8_SINT: return 3;
            case VK_FORMAT_B8G8R8_SRGB: return 3;
            case VK_FORMAT_R8G8B8A8_UNORM: return 4;
            case VK_FORMAT_R8G8B8A8_SNORM: return 4;
            case VK_FORMAT_R8G8B8A8_USCALED: return 4;
            case VK_FORMAT_R8G8B8A8_SSCALED: return 4;
            case VK_FORMAT_R8G8B8A8_UINT: return 4;
            case VK_FORMAT_R8G8B8A8_SINT: return 4;
            case VK_FORMAT_R8G8B8A8_SRGB: return 4;
            case VK_FORMAT_B8G8R8A8_UNORM: return 4;
            case VK_FORMAT_B8G8R8A8_SNORM: return 4;
            case VK_FORMAT_B8G8R8A8_USCALED: return 4;
            case VK_FORMAT_B8G8R8A8_SSCALED: return 4;
            case VK_FORMAT_B8G8R8A8_UINT: return 4;
            case VK_FORMAT_B8G8R8A8_SINT: return 4;
            case VK_FORMAT_B8G8R8A8_SRGB: return 4;
            case VK_FORMAT_A8B8G8R8_UNORM_PACK32: return 4;
            case VK_FORMAT_A8B8G8R8_SNORM_PACK32: return 4;
            case VK_FORMAT_A8B8G8R8_USCALED_PACK32: return 4;
            case VK_FORMAT_A8B8G8R8_SSCALED_PACK32: return 4;
            case VK_FORMAT_A8B8G8R8_UINT_PACK32: return 4;
            case VK_FORMAT_A8B8G8R8_SINT_PACK32: return 4;
            case VK_FORMAT_A8B8G8R8_SRGB_PACK32: return 4;
            case VK_FORMAT_A2R10G10B10_UNORM_PACK32: return 4;
            case VK_FORMAT_A2R10G10B10_SNORM_PACK32: return 4;
            case VK_FORMAT_A2R10G10B10_USCALED_PACK32: return 4;
            case VK_FORMAT_A2R10G10B10_SSCALED_PACK32: return 4;
            case VK_FORMAT_A2R10G10B10_UINT_PACK32: return 4;
            case VK_FORMAT_A2R10G10B10_SINT_PACK32: return 4;
            case VK_FORMAT_A2B10G10R10_UNORM_PACK32: return 4;
            case VK_FORMAT_A2B10G10R10_SNORM_PACK32: return 4;
            case VK_FORMAT_A2B10G10R10_USCALED_PACK32: return 4;
            case VK_FORMAT_A2B10G10R10_SSCALED_PACK32: return 4;
            case VK_FORMAT_A2B10G10R10_UINT_PACK32: return 4;
            case VK_FORMAT_A2B10G10R10_SINT_PACK32: return 4;
            case VK_FORMAT_R16_UNORM: return 2;
            case VK_FORMAT_R16_SNORM: return 2;
            case VK_FORMAT_R16_USCALED: return 2;
            case VK_FORMAT_R16_SSCALED: return 2;
            case VK_FORMAT_R16_UINT: return 2;
            case VK_FORMAT_R16_SINT: return 2;
            case VK_FORMAT_R16_SFLOAT: return 2;
            case VK_FORMAT_R16G16_UNORM: return 4;
            case VK_FORMAT_R16G16_SNORM: return 4;
            case VK_FORMAT_R16G16_USCALED: return 4;
            case VK_FORMAT_R16G16_SSCALED: return 4;
            case VK_FORMAT_R16G16_UINT: return 4;
            case VK_FORMAT_R16G16_SINT: return 4;
            case VK_FORMAT_R16G16_SFLOAT: return 6;
            case VK_FORMAT_R16G16B16_UNORM: return 6;
            case VK_FORMAT_R16G16B16_SNORM: return 6;
            case VK_FORMAT_R16G16B16_USCALED: return 6;
            case VK_FORMAT_R16G16B16_SSCALED: return 6;
            case VK_FORMAT_R16G16B16_UINT: return 6;
            case VK_FORMAT_R16G16B16_SINT: return 6;
            case VK_FORMAT_R16G16B16_SFLOAT: return 6;
            case VK_FORMAT_R16G16B16A16_UNORM: return 8;
            case VK_FORMAT_R16G16B16A16_SNORM: return 8;
            case VK_FORMAT_R16G16B16A16_USCALED: return 8;
            case VK_FORMAT_R16G16B16A16_SSCALED: return 8;
            case VK_FORMAT_R16G16B16A16_UINT: return 8;
            case VK_FORMAT_R16G16B16A16_SINT: return 8;
            case VK_FORMAT_R16G16B16A16_SFLOAT: return 8;
            case VK_FORMAT_R32_UINT: return 4;
            case VK_FORMAT_R32_SINT: return 4;
            case VK_FORMAT_R32_SFLOAT: return 4;
            case VK_FORMAT_R32G32_UINT: return 8;
            case VK_FORMAT_R32G32_SINT: return 8;
            case VK_FORMAT_R32G32_SFLOAT: return 8;
            case VK_FORMAT_R32G32B32_UINT: return 12;
            case VK_FORMAT_R32G32B32_SINT: return 12;
            case VK_FORMAT_R32G32B32_SFLOAT: return 12;
            case VK_FORMAT_R32G32B32A32_UINT: return 16;
            case VK_FORMAT_R32G32B32A32_SINT: return 16;
            case VK_FORMAT_R32G32B32A32_SFLOAT: return 16;
            case VK_FORMAT_R64_UINT: return 8;
            case VK_FORMAT_R64_SINT: return 8;
            case VK_FORMAT_R64_SFLOAT: return 8;
            case VK_FORMAT_R64G64_UINT: return 16;
            case VK_FORMAT_R64G64_SINT: return 16;
            case VK_FORMAT_R64G64_SFLOAT: return 16;
            case VK_FORMAT_R64G64B64_UINT: return 24;
            case VK_FORMAT_R64G64B64_SINT: return 24;
            case VK_FORMAT_R64G64B64_SFLOAT: return 24;
            case VK_FORMAT_R64G64B64A64_UINT: return 32;
            case VK_FORMAT_R64G64B64A64_SINT: return 32;
            case VK_FORMAT_R64G64B64A64_SFLOAT: return 32;
            case VK_FORMAT_B10G11R11_UFLOAT_PACK32: return 4;
            case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32: return 4;
            case VK_FORMAT_D16_UNORM: return 2;
            case VK_FORMAT_X8_D24_UNORM_PACK32: return 4;
            case VK_FORMAT_D32_SFLOAT: return 4;
            case VK_FORMAT_S8_UINT: return 1;
            case VK_FORMAT_D16_UNORM_S8_UINT: return 3;
            case VK_FORMAT_D24_UNORM_S8_UINT: return 4;
            case VK_FORMAT_D32_SFLOAT_S8_UINT: return 4;
            default: return 0;
        }
    }

    inline void GetCompressedImageFormatInfo(VkFormat format, uint32_t& blockSize, uint32_t& blockHeight, uint32_t& pixelHeight)
    {
        switch (format)
        {
            case VK_FORMAT_BC1_RGB_UNORM_BLOCK:
            case VK_FORMAT_BC1_RGB_SRGB_BLOCK:
            case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:
            case VK_FORMAT_BC1_RGBA_SRGB_BLOCK:
                blockSize = 8U;
                blockHeight = pixelHeight = 4U;
                break;
            case VK_FORMAT_BC2_UNORM_BLOCK:
            case VK_FORMAT_BC2_SRGB_BLOCK:
            case VK_FORMAT_BC3_UNORM_BLOCK:
            case VK_FORMAT_BC3_SRGB_BLOCK:
            case VK_FORMAT_BC4_UNORM_BLOCK:
            case VK_FORMAT_BC4_SNORM_BLOCK:
            case VK_FORMAT_BC5_UNORM_BLOCK:
            case VK_FORMAT_BC5_SNORM_BLOCK:
            case VK_FORMAT_BC6H_UFLOAT_BLOCK:
            case VK_FORMAT_BC6H_SFLOAT_BLOCK:
            case VK_FORMAT_BC7_UNORM_BLOCK:
            case VK_FORMAT_BC7_SRGB_BLOCK:
                blockSize = 16U;
                blockHeight = pixelHeight = 4U;
                break;
            case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK:
            case VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK:
            case VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK:
            case VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK:
            case VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK:
            case VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK:
            case VK_FORMAT_EAC_R11_UNORM_BLOCK:
            case VK_FORMAT_EAC_R11_SNORM_BLOCK:
                blockSize = 8U;
                blockHeight = pixelHeight = 4U;
                break;
            case VK_FORMAT_EAC_R11G11_UNORM_BLOCK:
            case VK_FORMAT_EAC_R11G11_SNORM_BLOCK:
            case VK_FORMAT_ASTC_4x4_UNORM_BLOCK:
            case VK_FORMAT_ASTC_4x4_SRGB_BLOCK:
                blockSize = 16U;
                blockHeight = pixelHeight = 4U;
                break;
            case VK_FORMAT_ASTC_5x4_UNORM_BLOCK:
            case VK_FORMAT_ASTC_5x4_SRGB_BLOCK:
                blockSize = 16U;
                blockHeight = 5U;
                pixelHeight = 4U;
                break;
            case VK_FORMAT_ASTC_5x5_UNORM_BLOCK:
            case VK_FORMAT_ASTC_5x5_SRGB_BLOCK:
                blockSize = 16U;
                blockHeight = pixelHeight = 5U;
                break;
            case VK_FORMAT_ASTC_6x5_UNORM_BLOCK:
            case VK_FORMAT_ASTC_6x5_SRGB_BLOCK:
                blockSize = 16U;
                blockHeight = 6U;
                pixelHeight = 5U;
                break;
            case VK_FORMAT_ASTC_6x6_UNORM_BLOCK:
            case VK_FORMAT_ASTC_6x6_SRGB_BLOCK:
                blockSize = 16U;
                blockHeight = pixelHeight = 6U;
                break;
            case VK_FORMAT_ASTC_8x5_UNORM_BLOCK:
            case VK_FORMAT_ASTC_8x5_SRGB_BLOCK:
                blockSize = 16U;
                blockHeight = 8U;
                pixelHeight = 5U;
                break;
            case VK_FORMAT_ASTC_8x6_UNORM_BLOCK:
            case VK_FORMAT_ASTC_8x6_SRGB_BLOCK:
                blockSize = 16U;
                blockHeight = 8U;
                pixelHeight = 6U;
                break;
            case VK_FORMAT_ASTC_8x8_UNORM_BLOCK:
            case VK_FORMAT_ASTC_8x8_SRGB_BLOCK:
                blockSize = 16U;
                blockHeight = pixelHeight = 8U;
                break;
            case VK_FORMAT_ASTC_10x5_UNORM_BLOCK:
            case VK_FORMAT_ASTC_10x5_SRGB_BLOCK:
                blockSize = 16U;
                blockHeight = 10U;
                pixelHeight = 5U;
                break;
            case VK_FORMAT_ASTC_10x6_UNORM_BLOCK:
            case VK_FORMAT_ASTC_10x6_SRGB_BLOCK:
                blockSize = 16U;
                blockHeight = 10U;
                pixelHeight = 6U;
                break;
            case VK_FORMAT_ASTC_10x8_UNORM_BLOCK:
            case VK_FORMAT_ASTC_10x8_SRGB_BLOCK:
                blockSize = 16U;
                blockHeight = 10U;
                pixelHeight = 8U;
                break;
            case VK_FORMAT_ASTC_10x10_UNORM_BLOCK:
            case VK_FORMAT_ASTC_10x10_SRGB_BLOCK:
                blockSize = 16U;
                blockHeight = pixelHeight = 10U;
                break;
            case VK_FORMAT_ASTC_12x10_UNORM_BLOCK:
            case VK_FORMAT_ASTC_12x10_SRGB_BLOCK:
                blockSize = 16U;
                blockHeight = 12U;
                pixelHeight = 10U;
                break;
            case VK_FORMAT_ASTC_12x12_UNORM_BLOCK:
            case VK_FORMAT_ASTC_12x12_SRGB_BLOCK:
                blockSize = 16U;
                blockHeight = pixelHeight = 12U;
                break;

            default:
                blockSize = blockHeight = pixelHeight = 0U;
                break;
        }
    }
}

namespace std
{
    template <template <typename, typename, typename...> typename KeyValueContainer, typename... Ts>
    struct container_traits
    {
        using map_type = KeyValueContainer<Ts...>;
        using allocator_type = typename map_type::allocator_type;

        using key_type = typename map_type::key_type;
        using value_type = typename map_type::value_type;

        using allocator_traits = ::std::allocator_traits<allocator_type>;

        template <typename U, template <typename, typename> typename Vector = vector>
        using container_type = Vector<U, typename allocator_traits::template rebind_alloc<U>>;

        template <template <typename, typename> typename Vector = vector>
        using value_container_type = container_type<value_type, Vector>;

        template <template <typename, typename> typename Vector = vector>
        using key_container_type = container_type<key_type, Vector>;
    };

    namespace
    {
        template <typename Key, typename T, typename U = T, template <typename...> typename Container, typename... Ts>
        static inline auto to_u_vector(Container<Key, T, Ts...> const& map)
        {
            using container_traits_t = container_traits<Container, Key, T, Ts...>;
            using vector_type = typename container_traits_t::template container_type<U>;

            auto mutator = [](pair<const Key, T> const& kv) -> U { return std::get<U>(kv); };
            auto itr = boost::make_transform_iterator(map.begin(), mutator);
            auto end = boost::make_transform_iterator(map.end(), mutator);

            vector_type range(itr, end);
            return range;
        }
    }

    template <typename Key, typename T, typename... Ts>
    static inline auto to_value_vector(unordered_map<Key, T, Ts...> const& map) -> typename container_traits<unordered_map, Key, T, Ts...>::template container_type<T>
    {
#if COMPILER == COMPILER_MICROSOFT
        // Just fix your deduction rules...
        return to_u_vector<Key, T, T, unordered_map, Ts...>(map);
#else
        return to_u_vector<T>(map);
#endif
    }

    template <typename Key, typename T, typename... Ts>
    static inline auto to_key_vector(unordered_map<Key, T, Ts...> const& map) -> typename container_traits<unordered_map, Key, T, Ts...>::template container_type<Key>
    {
#if COMPILER == COMPILER_MICROSOFT
        // Just fix your deduction rules...
        return to_u_vector<Key, T, Key, unordered_map, Ts...>(map);
#else
        return to_u_vector<Key>(map);
#endif
    }
}
