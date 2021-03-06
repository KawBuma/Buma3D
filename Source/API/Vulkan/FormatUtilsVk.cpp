#include "Buma3DPCH.h"
#include "FormatUtilsVk.h"

namespace buma3d
{
namespace util
{

// TODO: これら関数のパラメータを事前に構造化し配列で管理する。
// 列挙などのコンバート

VkFormat GetNativeFormat(RESOURCE_FORMAT _format)
{
    switch (_format)
    {
    case RESOURCE_FORMAT_UNKNOWN                                 : return VK_FORMAT_UNDEFINED;

    // R
    case RESOURCE_FORMAT_R8_TYPELESS                             : return VK_FORMAT_R8_UNORM;
    case RESOURCE_FORMAT_R8_UNORM                                : return VK_FORMAT_R8_UNORM;
    case RESOURCE_FORMAT_R8_SNORM                                : return VK_FORMAT_R8_SNORM;
    case RESOURCE_FORMAT_R8_UINT                                 : return VK_FORMAT_R8_UINT;
    case RESOURCE_FORMAT_R8_SINT                                 : return VK_FORMAT_R8_SINT;

    case RESOURCE_FORMAT_R16_TYPELESS                            : return VK_FORMAT_R16_UNORM;
    case RESOURCE_FORMAT_R16_UNORM                               : return VK_FORMAT_R16_UNORM;
    case RESOURCE_FORMAT_R16_SNORM                               : return VK_FORMAT_R16_SNORM;
    case RESOURCE_FORMAT_R16_UINT                                : return VK_FORMAT_R16_UINT;
    case RESOURCE_FORMAT_R16_SINT                                : return VK_FORMAT_R16_SINT;
    case RESOURCE_FORMAT_R16_FLOAT                               : return VK_FORMAT_R16_SFLOAT;

    case RESOURCE_FORMAT_R32_TYPELESS                            : return VK_FORMAT_R32_UINT;
    case RESOURCE_FORMAT_R32_UINT                                : return VK_FORMAT_R32_UINT;
    case RESOURCE_FORMAT_R32_SINT                                : return VK_FORMAT_R32_SINT;
    case RESOURCE_FORMAT_R32_FLOAT                               : return VK_FORMAT_R32_SFLOAT;

    // RG
    case RESOURCE_FORMAT_R8G8_TYPELESS                           : return VK_FORMAT_R8G8_UNORM;
    case RESOURCE_FORMAT_R8G8_UNORM                              : return VK_FORMAT_R8G8_UNORM;
    case RESOURCE_FORMAT_R8G8_SNORM                              : return VK_FORMAT_R8G8_SNORM;
    case RESOURCE_FORMAT_R8G8_UINT                               : return VK_FORMAT_R8G8_UINT;
    case RESOURCE_FORMAT_R8G8_SINT                               : return VK_FORMAT_R8G8_SINT;

    case RESOURCE_FORMAT_R16G16_TYPELESS                         : return VK_FORMAT_R16G16_UNORM;
    case RESOURCE_FORMAT_R16G16_UNORM                            : return VK_FORMAT_R16G16_UNORM;
    case RESOURCE_FORMAT_R16G16_SNORM                            : return VK_FORMAT_R16G16_SNORM;
    case RESOURCE_FORMAT_R16G16_UINT                             : return VK_FORMAT_R16G16_UINT;
    case RESOURCE_FORMAT_R16G16_SINT                             : return VK_FORMAT_R16G16_SINT;
    case RESOURCE_FORMAT_R16G16_FLOAT                            : return VK_FORMAT_R16G16_SFLOAT;

    case RESOURCE_FORMAT_R32G32_TYPELESS                         : return VK_FORMAT_R32G32_UINT;
    case RESOURCE_FORMAT_R32G32_UINT                             : return VK_FORMAT_R32G32_UINT;
    case RESOURCE_FORMAT_R32G32_SINT                             : return VK_FORMAT_R32G32_SINT;
    case RESOURCE_FORMAT_R32G32_FLOAT                            : return VK_FORMAT_R32G32_SFLOAT;
    // GR

    // RGB
    case RESOURCE_FORMAT_R11G11B10_UFLOAT                        : return VK_FORMAT_B10G11R11_UFLOAT_PACK32;

    case RESOURCE_FORMAT_R32G32B32_TYPELESS                      : return VK_FORMAT_R32G32B32_UINT;
    case RESOURCE_FORMAT_R32G32B32_UINT                          : return VK_FORMAT_R32G32B32_UINT;
    case RESOURCE_FORMAT_R32G32B32_SINT                          : return VK_FORMAT_R32G32B32_SINT;
    case RESOURCE_FORMAT_R32G32B32_FLOAT                         : return VK_FORMAT_R32G32B32_SFLOAT;
    // BGR
    case RESOURCE_FORMAT_B5G6R5_UNORM                            : return VK_FORMAT_R5G6B5_UNORM_PACK16;

    // RGBA
    case RESOURCE_FORMAT_R8G8B8A8_TYPELESS                       : return VK_FORMAT_R8G8B8A8_UNORM;
    case RESOURCE_FORMAT_R8G8B8A8_UNORM                          : return VK_FORMAT_R8G8B8A8_UNORM;
    case RESOURCE_FORMAT_R8G8B8A8_UNORM_SRGB                     : return VK_FORMAT_R8G8B8A8_SRGB;
    case RESOURCE_FORMAT_R8G8B8A8_SNORM                          : return VK_FORMAT_R8G8B8A8_SNORM;
    case RESOURCE_FORMAT_R8G8B8A8_UINT                           : return VK_FORMAT_R8G8B8A8_UINT;
    case RESOURCE_FORMAT_R8G8B8A8_SINT                           : return VK_FORMAT_R8G8B8A8_SINT;

    case RESOURCE_FORMAT_R10G10B10A2_TYPELESS                    : return VK_FORMAT_A2B10G10R10_UNORM_PACK32;
    case RESOURCE_FORMAT_R10G10B10A2_UNORM                       : return VK_FORMAT_A2B10G10R10_UNORM_PACK32;
    case RESOURCE_FORMAT_R10G10B10A2_UINT                        : return VK_FORMAT_A2B10G10R10_UINT_PACK32;

    case RESOURCE_FORMAT_R16G16B16A16_TYPELESS                   : return VK_FORMAT_R16G16B16A16_UNORM;
    case RESOURCE_FORMAT_R16G16B16A16_UNORM                      : return VK_FORMAT_R16G16B16A16_UNORM;
    case RESOURCE_FORMAT_R16G16B16A16_SNORM                      : return VK_FORMAT_R16G16B16A16_SNORM;
    case RESOURCE_FORMAT_R16G16B16A16_UINT                       : return VK_FORMAT_R16G16B16A16_UINT;
    case RESOURCE_FORMAT_R16G16B16A16_SINT                       : return VK_FORMAT_R16G16B16A16_SINT;
    case RESOURCE_FORMAT_R16G16B16A16_FLOAT                      : return VK_FORMAT_R16G16B16A16_SFLOAT;

    case RESOURCE_FORMAT_R32G32B32A32_TYPELESS                   : return VK_FORMAT_R32G32B32A32_UINT;
    case RESOURCE_FORMAT_R32G32B32A32_UINT                       : return VK_FORMAT_R32G32B32A32_UINT;
    case RESOURCE_FORMAT_R32G32B32A32_SINT                       : return VK_FORMAT_R32G32B32A32_SINT;
    case RESOURCE_FORMAT_R32G32B32A32_FLOAT                      : return VK_FORMAT_R32G32B32A32_SFLOAT;
    // BGRA
    case RESOURCE_FORMAT_B5G5R5A1_UNORM                          : return VK_FORMAT_A1R5G5B5_UNORM_PACK16;

    case RESOURCE_FORMAT_B8G8R8A8_TYPELESS                       : return VK_FORMAT_B8G8R8A8_UNORM;
    case RESOURCE_FORMAT_B8G8R8A8_UNORM                          : return VK_FORMAT_B8G8R8A8_UNORM;
    case RESOURCE_FORMAT_B8G8R8A8_UNORM_SRGB                     : return VK_FORMAT_B8G8R8A8_SRGB;

    // RGBE
    case RESOURCE_FORMAT_R9G9B9E5_UFLOAT                         : return VK_FORMAT_E5B9G9R9_UFLOAT_PACK32;

    // 深度フォーマット
    case RESOURCE_FORMAT_D16_UNORM                               : return VK_FORMAT_D16_UNORM;
    case RESOURCE_FORMAT_D32_FLOAT                               : return VK_FORMAT_D32_SFLOAT;
    case RESOURCE_FORMAT_D24_UNORM_S8_UINT                       : return VK_FORMAT_D24_UNORM_S8_UINT;
    case RESOURCE_FORMAT_D32_FLOAT_S8X24_UINT                    : return VK_FORMAT_D32_SFLOAT_S8_UINT;

    // 圧縮フォーマット
    case RESOURCE_FORMAT_BC1_TYPELESS                            : return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
    case RESOURCE_FORMAT_BC1_UNORM                               : return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
    case RESOURCE_FORMAT_BC1_UNORM_SRGB                          : return VK_FORMAT_BC1_RGBA_SRGB_BLOCK;

    case RESOURCE_FORMAT_BC2_TYPELESS                            : return VK_FORMAT_BC2_UNORM_BLOCK;
    case RESOURCE_FORMAT_BC2_UNORM                               : return VK_FORMAT_BC2_UNORM_BLOCK;
    case RESOURCE_FORMAT_BC2_UNORM_SRGB                          : return VK_FORMAT_BC2_SRGB_BLOCK;

    case RESOURCE_FORMAT_BC3_TYPELESS                            : return VK_FORMAT_BC3_UNORM_BLOCK;
    case RESOURCE_FORMAT_BC3_UNORM                               : return VK_FORMAT_BC3_UNORM_BLOCK;
    case RESOURCE_FORMAT_BC3_UNORM_SRGB                          : return VK_FORMAT_BC3_SRGB_BLOCK;

    case RESOURCE_FORMAT_BC4_TYPELESS                            : return VK_FORMAT_BC4_UNORM_BLOCK;
    case RESOURCE_FORMAT_BC4_UNORM                               : return VK_FORMAT_BC4_UNORM_BLOCK;
    case RESOURCE_FORMAT_BC4_SNORM                               : return VK_FORMAT_BC4_SNORM_BLOCK;

    case RESOURCE_FORMAT_BC5_TYPELESS                            : return VK_FORMAT_BC5_UNORM_BLOCK;
    case RESOURCE_FORMAT_BC5_UNORM                               : return VK_FORMAT_BC5_UNORM_BLOCK;
    case RESOURCE_FORMAT_BC5_SNORM                               : return VK_FORMAT_BC5_SNORM_BLOCK;

    case RESOURCE_FORMAT_BC6H_TYPELESS                           : return VK_FORMAT_BC6H_UFLOAT_BLOCK;
    case RESOURCE_FORMAT_BC6H_UF16                               : return VK_FORMAT_BC6H_UFLOAT_BLOCK;
    case RESOURCE_FORMAT_BC6H_SF16                               : return VK_FORMAT_BC6H_SFLOAT_BLOCK;

    case RESOURCE_FORMAT_BC7_TYPELESS                            : return VK_FORMAT_BC7_UNORM_BLOCK;
    case RESOURCE_FORMAT_BC7_UNORM                               : return VK_FORMAT_BC7_UNORM_BLOCK;
    case RESOURCE_FORMAT_BC7_UNORM_SRGB                          : return VK_FORMAT_BC7_SRGB_BLOCK;

    // ビデオフォーマット
    case RESOURCE_FORMAT_Y8U8Y8V8_422_UNORM                      : return VK_FORMAT_G8B8G8R8_422_UNORM;
    case RESOURCE_FORMAT_X6Y10X6U10X6Y10X6V10_422_UNORM          : return VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16;
    case RESOURCE_FORMAT_Y16U16Y16V16_422_UNORM                  : return VK_FORMAT_G16B16G16R16_422_UNORM;
    case RESOURCE_FORMAT_Y8_U8V8_2PLANE_420_UNORM                : return VK_FORMAT_G8_B8R8_2PLANE_420_UNORM;
    case RESOURCE_FORMAT_Y8_U8V8_2PLANE_420_UNORM_PLANE0         : return VK_FORMAT_R8_UNORM;
    case RESOURCE_FORMAT_Y8_U8V8_2PLANE_420_UNORM_PLANE1         : return VK_FORMAT_R8G8_UNORM;
    case RESOURCE_FORMAT_X6Y10_X6U10X6V10_2PLANE_420_UNORM       : return VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16;
    case RESOURCE_FORMAT_X6Y10_X6U10X6V10_2PLANE_420_UNORM_PLANE0: return VK_FORMAT_R10X6_UNORM_PACK16;
    case RESOURCE_FORMAT_X6Y10_X6U10X6V10_2PLANE_420_UNORM_PLANE1: return VK_FORMAT_R10X6G10X6_UNORM_2PACK16;
    case RESOURCE_FORMAT_Y16_U16V16_2PLANE_420_UNORM             : return VK_FORMAT_G16_B16R16_2PLANE_420_UNORM;
    case RESOURCE_FORMAT_Y16_U16V16_2PLANE_420_UNORM_PLANE0      : return VK_FORMAT_R16_UNORM;
    case RESOURCE_FORMAT_Y16_U16V16_2PLANE_420_UNORM_PLANE1      : return VK_FORMAT_R16G16_UNORM;

    // 圧縮フォーマット_VK
    case RESOURCE_FORMAT_BC1_RGB_TYPELESS_BLOCK_VK               : return VK_FORMAT_BC1_RGB_UNORM_BLOCK;
    case RESOURCE_FORMAT_BC1_RGB_UNORM_BLOCK_VK                  : return VK_FORMAT_BC1_RGB_UNORM_BLOCK;
    case RESOURCE_FORMAT_BC1_RGB_SRGB_BLOCK_VK                   : return VK_FORMAT_BC1_RGB_SRGB_BLOCK;

    case RESOURCE_FORMAT_ETC2_R8G8B8_TYPELESS_BLOCK_VK           : return VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK;
    case RESOURCE_FORMAT_ETC2_R8G8B8_UNORM_BLOCK_VK              : return VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK;
    case RESOURCE_FORMAT_ETC2_R8G8B8_SRGB_BLOCK_VK               : return VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK;

    case RESOURCE_FORMAT_ETC2_R8G8B8A1_TYPELESS_BLOCK_VK         : return VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK;
    case RESOURCE_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK_VK            : return VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK;
    case RESOURCE_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK_VK             : return VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK;

    case RESOURCE_FORMAT_ETC2_R8G8B8A8_TYPELESS_BLOCK_VK         : return VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK;
    case RESOURCE_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK_VK            : return VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK;
    case RESOURCE_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK_VK             : return VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK;

    case RESOURCE_FORMAT_EAC_R11_TYPELESS_BLOCK_VK               : return VK_FORMAT_EAC_R11_UNORM_BLOCK;
    case RESOURCE_FORMAT_EAC_R11_UNORM_BLOCK_VK                  : return VK_FORMAT_EAC_R11_UNORM_BLOCK;
    case RESOURCE_FORMAT_EAC_R11_SNORM_BLOCK_VK                  : return VK_FORMAT_EAC_R11_SNORM_BLOCK;

    case RESOURCE_FORMAT_EAC_R11G11_TYPELESS_BLOCK_VK            : return VK_FORMAT_EAC_R11G11_UNORM_BLOCK;
    case RESOURCE_FORMAT_EAC_R11G11_UNORM_BLOCK_VK               : return VK_FORMAT_EAC_R11G11_UNORM_BLOCK;
    case RESOURCE_FORMAT_EAC_R11G11_SNORM_BLOCK_VK               : return VK_FORMAT_EAC_R11G11_SNORM_BLOCK;

    case RESOURCE_FORMAT_ASTC_4x4_TYPELESS_BLOCK_VK              : return VK_FORMAT_ASTC_4x4_UNORM_BLOCK;
    case RESOURCE_FORMAT_ASTC_4x4_UNORM_BLOCK_VK                 : return VK_FORMAT_ASTC_4x4_UNORM_BLOCK;
    case RESOURCE_FORMAT_ASTC_4x4_SRGB_BLOCK_VK                  : return VK_FORMAT_ASTC_4x4_SRGB_BLOCK;
    case RESOURCE_FORMAT_ASTC_4x4_SFLOAT_BLOCK_EXT_VK            : return VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK_EXT;

    case RESOURCE_FORMAT_ASTC_5x4_TYPELESS_BLOCK_VK              : return VK_FORMAT_ASTC_5x4_UNORM_BLOCK;
    case RESOURCE_FORMAT_ASTC_5x4_UNORM_BLOCK_VK                 : return VK_FORMAT_ASTC_5x4_UNORM_BLOCK;
    case RESOURCE_FORMAT_ASTC_5x4_SRGB_BLOCK_VK                  : return VK_FORMAT_ASTC_5x4_SRGB_BLOCK;
    case RESOURCE_FORMAT_ASTC_5x4_SFLOAT_BLOCK_EXT_VK            : return VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK_EXT;

    case RESOURCE_FORMAT_ASTC_5x5_TYPELESS_BLOCK_VK              : return VK_FORMAT_ASTC_5x5_UNORM_BLOCK;
    case RESOURCE_FORMAT_ASTC_5x5_UNORM_BLOCK_VK                 : return VK_FORMAT_ASTC_5x5_UNORM_BLOCK;
    case RESOURCE_FORMAT_ASTC_5x5_SRGB_BLOCK_VK                  : return VK_FORMAT_ASTC_5x5_SRGB_BLOCK;
    case RESOURCE_FORMAT_ASTC_5x5_SFLOAT_BLOCK_EXT_VK            : return VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK_EXT;

    case RESOURCE_FORMAT_ASTC_6x5_TYPELESS_BLOCK_VK              : return VK_FORMAT_ASTC_6x5_UNORM_BLOCK;
    case RESOURCE_FORMAT_ASTC_6x5_UNORM_BLOCK_VK                 : return VK_FORMAT_ASTC_6x5_UNORM_BLOCK;
    case RESOURCE_FORMAT_ASTC_6x5_SRGB_BLOCK_VK                  : return VK_FORMAT_ASTC_6x5_SRGB_BLOCK;
    case RESOURCE_FORMAT_ASTC_6x5_SFLOAT_BLOCK_EXT_VK            : return VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK_EXT;

    case RESOURCE_FORMAT_ASTC_6x6_TYPELESS_BLOCK_VK              : return VK_FORMAT_ASTC_6x6_UNORM_BLOCK;
    case RESOURCE_FORMAT_ASTC_6x6_UNORM_BLOCK_VK                 : return VK_FORMAT_ASTC_6x6_UNORM_BLOCK;
    case RESOURCE_FORMAT_ASTC_6x6_SRGB_BLOCK_VK                  : return VK_FORMAT_ASTC_6x6_SRGB_BLOCK;
    case RESOURCE_FORMAT_ASTC_6x6_SFLOAT_BLOCK_EXT_VK            : return VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK_EXT;

    case RESOURCE_FORMAT_ASTC_8x5_TYPELESS_BLOCK_VK              : return VK_FORMAT_ASTC_8x5_UNORM_BLOCK;
    case RESOURCE_FORMAT_ASTC_8x5_UNORM_BLOCK_VK                 : return VK_FORMAT_ASTC_8x5_UNORM_BLOCK;
    case RESOURCE_FORMAT_ASTC_8x5_SRGB_BLOCK_VK                  : return VK_FORMAT_ASTC_8x5_SRGB_BLOCK;
    case RESOURCE_FORMAT_ASTC_8x5_SFLOAT_BLOCK_EXT_VK            : return VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK_EXT;

    case RESOURCE_FORMAT_ASTC_8x6_TYPELESS_BLOCK_VK              : return VK_FORMAT_ASTC_8x6_UNORM_BLOCK;
    case RESOURCE_FORMAT_ASTC_8x6_UNORM_BLOCK_VK                 : return VK_FORMAT_ASTC_8x6_UNORM_BLOCK;
    case RESOURCE_FORMAT_ASTC_8x6_SRGB_BLOCK_VK                  : return VK_FORMAT_ASTC_8x6_SRGB_BLOCK;
    case RESOURCE_FORMAT_ASTC_8x6_SFLOAT_BLOCK_EXT_VK            : return VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK_EXT;

    case RESOURCE_FORMAT_ASTC_8x8_TYPELESS_BLOCK_VK              : return VK_FORMAT_ASTC_8x8_UNORM_BLOCK;
    case RESOURCE_FORMAT_ASTC_8x8_UNORM_BLOCK_VK                 : return VK_FORMAT_ASTC_8x8_UNORM_BLOCK;
    case RESOURCE_FORMAT_ASTC_8x8_SRGB_BLOCK_VK                  : return VK_FORMAT_ASTC_8x8_SRGB_BLOCK;
    case RESOURCE_FORMAT_ASTC_8x8_SFLOAT_BLOCK_EXT_VK            : return VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK_EXT;

    case RESOURCE_FORMAT_ASTC_10x5_TYPELESS_BLOCK_VK             : return VK_FORMAT_ASTC_10x5_UNORM_BLOCK;
    case RESOURCE_FORMAT_ASTC_10x5_UNORM_BLOCK_VK                : return VK_FORMAT_ASTC_10x5_UNORM_BLOCK;
    case RESOURCE_FORMAT_ASTC_10x5_SRGB_BLOCK_VK                 : return VK_FORMAT_ASTC_10x5_SRGB_BLOCK;
    case RESOURCE_FORMAT_ASTC_10x5_SFLOAT_BLOCK_EXT_VK           : return VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK_EXT;

    case RESOURCE_FORMAT_ASTC_10x6_TYPELESS_BLOCK_VK             : return VK_FORMAT_ASTC_10x6_UNORM_BLOCK;
    case RESOURCE_FORMAT_ASTC_10x6_UNORM_BLOCK_VK                : return VK_FORMAT_ASTC_10x6_UNORM_BLOCK;
    case RESOURCE_FORMAT_ASTC_10x6_SRGB_BLOCK_VK                 : return VK_FORMAT_ASTC_10x6_SRGB_BLOCK;
    case RESOURCE_FORMAT_ASTC_10x6_SFLOAT_BLOCK_EXT_VK           : return VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK_EXT;

    case RESOURCE_FORMAT_ASTC_10x8_TYPELESS_BLOCK_VK             : return VK_FORMAT_ASTC_10x8_UNORM_BLOCK;
    case RESOURCE_FORMAT_ASTC_10x8_UNORM_BLOCK_VK                : return VK_FORMAT_ASTC_10x8_UNORM_BLOCK;
    case RESOURCE_FORMAT_ASTC_10x8_SRGB_BLOCK_VK                 : return VK_FORMAT_ASTC_10x8_SRGB_BLOCK;
    case RESOURCE_FORMAT_ASTC_10x8_SFLOAT_BLOCK_EXT_VK           : return VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK_EXT;

    case RESOURCE_FORMAT_ASTC_10x10_TYPELESS_BLOCK_VK            : return VK_FORMAT_ASTC_10x10_UNORM_BLOCK;
    case RESOURCE_FORMAT_ASTC_10x10_UNORM_BLOCK_VK               : return VK_FORMAT_ASTC_10x10_UNORM_BLOCK;
    case RESOURCE_FORMAT_ASTC_10x10_SRGB_BLOCK_VK                : return VK_FORMAT_ASTC_10x10_SRGB_BLOCK;
    case RESOURCE_FORMAT_ASTC_10x10_SFLOAT_BLOCK_EXT_VK          : return VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK_EXT;

    case RESOURCE_FORMAT_ASTC_12x10_TYPELESS_BLOCK_VK            : return VK_FORMAT_ASTC_12x10_UNORM_BLOCK;
    case RESOURCE_FORMAT_ASTC_12x10_UNORM_BLOCK_VK               : return VK_FORMAT_ASTC_12x10_UNORM_BLOCK;
    case RESOURCE_FORMAT_ASTC_12x10_SRGB_BLOCK_VK                : return VK_FORMAT_ASTC_12x10_SRGB_BLOCK;
    case RESOURCE_FORMAT_ASTC_12x10_SFLOAT_BLOCK_EXT_VK          : return VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK_EXT;

    case RESOURCE_FORMAT_ASTC_12x12_TYPELESS_BLOCK_VK            : return VK_FORMAT_ASTC_12x12_UNORM_BLOCK;
    case RESOURCE_FORMAT_ASTC_12x12_UNORM_BLOCK_VK               : return VK_FORMAT_ASTC_12x12_UNORM_BLOCK;
    case RESOURCE_FORMAT_ASTC_12x12_SRGB_BLOCK_VK                : return VK_FORMAT_ASTC_12x12_SRGB_BLOCK;
    case RESOURCE_FORMAT_ASTC_12x12_SFLOAT_BLOCK_EXT_VK          : return VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK_EXT;

    case RESOURCE_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG_VK          : return VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG;
    case RESOURCE_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG_VK          : return VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG;
    case RESOURCE_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG_VK          : return VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG;
    case RESOURCE_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG_VK          : return VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG;
    case RESOURCE_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG_VK           : return VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG;
    case RESOURCE_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG_VK           : return VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG;
    case RESOURCE_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG_VK           : return VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG;
    case RESOURCE_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG_VK           : return VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG;

    default:
        return VK_FORMAT_UNDEFINED;
    }
}

RESOURCE_FORMAT GetB3DFormat(VkFormat _format)
{
    switch (_format)
    {
    case VK_FORMAT_UNDEFINED                                : return RESOURCE_FORMAT_UNKNOWN;

    // R
    case VK_FORMAT_R8_UNORM                                 : return RESOURCE_FORMAT_R8_UNORM;// RESOURCE_FORMAT_Y8_U8V8_2PLANE_420_UNORM_PLANE0
    case VK_FORMAT_R8_SNORM                                 : return RESOURCE_FORMAT_R8_SNORM;
    case VK_FORMAT_R8_UINT                                  : return RESOURCE_FORMAT_R8_UINT;
    case VK_FORMAT_R8_SINT                                  : return RESOURCE_FORMAT_R8_SINT;
    case VK_FORMAT_R16_UNORM                                : return RESOURCE_FORMAT_R16_UNORM;// RESOURCE_FORMAT_Y16_U16V16_2PLANE_420_UNORM_PLANE0
    case VK_FORMAT_R16_SNORM                                : return RESOURCE_FORMAT_R16_SNORM;
    case VK_FORMAT_R16_UINT                                 : return RESOURCE_FORMAT_R16_UINT;
    case VK_FORMAT_R16_SINT                                 : return RESOURCE_FORMAT_R16_SINT;
    case VK_FORMAT_R16_SFLOAT                               : return RESOURCE_FORMAT_R16_FLOAT;
    case VK_FORMAT_R32_UINT                                 : return RESOURCE_FORMAT_R32_UINT;
    case VK_FORMAT_R32_SINT                                 : return RESOURCE_FORMAT_R32_SINT;
    case VK_FORMAT_R32_SFLOAT                               : return RESOURCE_FORMAT_R32_FLOAT;

    // RG
    case VK_FORMAT_R8G8_UNORM                               : return RESOURCE_FORMAT_R8G8_UNORM;// RESOURCE_FORMAT_Y8_U8V8_2PLANE_420_UNORM_PLANE1
    case VK_FORMAT_R8G8_SNORM                               : return RESOURCE_FORMAT_R8G8_SNORM;
    case VK_FORMAT_R8G8_UINT                                : return RESOURCE_FORMAT_R8G8_UINT;
    case VK_FORMAT_R8G8_SINT                                : return RESOURCE_FORMAT_R8G8_SINT;
    case VK_FORMAT_R16G16_UNORM                             : return RESOURCE_FORMAT_R16G16_UNORM;// RESOURCE_FORMAT_Y16_U16V16_2PLANE_420_UNORM_PLANE1
    case VK_FORMAT_R16G16_SNORM                             : return RESOURCE_FORMAT_R16G16_SNORM;
    case VK_FORMAT_R16G16_UINT                              : return RESOURCE_FORMAT_R16G16_UINT;
    case VK_FORMAT_R16G16_SINT                              : return RESOURCE_FORMAT_R16G16_SINT;
    case VK_FORMAT_R16G16_SFLOAT                            : return RESOURCE_FORMAT_R16G16_FLOAT;
    case VK_FORMAT_R32G32_UINT                              : return RESOURCE_FORMAT_R32G32_UINT;
    case VK_FORMAT_R32G32_SINT                              : return RESOURCE_FORMAT_R32G32_SINT;
    case VK_FORMAT_R32G32_SFLOAT                            : return RESOURCE_FORMAT_R32G32_FLOAT;
    // GR

    // RGB
    case VK_FORMAT_B10G11R11_UFLOAT_PACK32                  : return RESOURCE_FORMAT_R11G11B10_UFLOAT;
    case VK_FORMAT_R32G32B32_UINT                           : return RESOURCE_FORMAT_R32G32B32_UINT;
    case VK_FORMAT_R32G32B32_SINT                           : return RESOURCE_FORMAT_R32G32B32_SINT;
    case VK_FORMAT_R32G32B32_SFLOAT                         : return RESOURCE_FORMAT_R32G32B32_FLOAT;
    // BGR
    case VK_FORMAT_R5G6B5_UNORM_PACK16                      : return RESOURCE_FORMAT_B5G6R5_UNORM;

    // RGBA
    case VK_FORMAT_R8G8B8A8_UNORM                           : return RESOURCE_FORMAT_R8G8B8A8_UNORM;
    case VK_FORMAT_R8G8B8A8_SRGB                            : return RESOURCE_FORMAT_R8G8B8A8_UNORM_SRGB;
    case VK_FORMAT_R8G8B8A8_SNORM                           : return RESOURCE_FORMAT_R8G8B8A8_SNORM;
    case VK_FORMAT_R8G8B8A8_UINT                            : return RESOURCE_FORMAT_R8G8B8A8_UINT;
    case VK_FORMAT_R8G8B8A8_SINT                            : return RESOURCE_FORMAT_R8G8B8A8_SINT;
    case VK_FORMAT_A2B10G10R10_UNORM_PACK32                 : return RESOURCE_FORMAT_R10G10B10A2_UNORM;
    case VK_FORMAT_A2B10G10R10_UINT_PACK32                  : return RESOURCE_FORMAT_R10G10B10A2_UINT;
    case VK_FORMAT_R16G16B16A16_UNORM                       : return RESOURCE_FORMAT_R16G16B16A16_UNORM;
    case VK_FORMAT_R16G16B16A16_SNORM                       : return RESOURCE_FORMAT_R16G16B16A16_SNORM;
    case VK_FORMAT_R16G16B16A16_UINT                        : return RESOURCE_FORMAT_R16G16B16A16_UINT;
    case VK_FORMAT_R16G16B16A16_SINT                        : return RESOURCE_FORMAT_R16G16B16A16_SINT;
    case VK_FORMAT_R16G16B16A16_SFLOAT                      : return RESOURCE_FORMAT_R16G16B16A16_FLOAT;
    case VK_FORMAT_R32G32B32A32_UINT                        : return RESOURCE_FORMAT_R32G32B32A32_UINT;
    case VK_FORMAT_R32G32B32A32_SINT                        : return RESOURCE_FORMAT_R32G32B32A32_SINT;
    case VK_FORMAT_R32G32B32A32_SFLOAT                      : return RESOURCE_FORMAT_R32G32B32A32_FLOAT;
    // BGRA
    case VK_FORMAT_A1R5G5B5_UNORM_PACK16                    : return RESOURCE_FORMAT_B5G5R5A1_UNORM;
    case VK_FORMAT_B8G8R8A8_UNORM                           : return RESOURCE_FORMAT_B8G8R8A8_UNORM;
    case VK_FORMAT_B8G8R8A8_SRGB                            : return RESOURCE_FORMAT_B8G8R8A8_UNORM_SRGB;

    // RGBE
    case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32                   : return RESOURCE_FORMAT_R9G9B9E5_UFLOAT;

    // 深度フォーマット
    case VK_FORMAT_D16_UNORM                                : return RESOURCE_FORMAT_D16_UNORM;
    case VK_FORMAT_D32_SFLOAT                               : return RESOURCE_FORMAT_D32_FLOAT;
    case VK_FORMAT_D24_UNORM_S8_UINT                        : return RESOURCE_FORMAT_D24_UNORM_S8_UINT;
    case VK_FORMAT_D32_SFLOAT_S8_UINT                       : return RESOURCE_FORMAT_D32_FLOAT_S8X24_UINT;

    // 圧縮フォーマット
    case VK_FORMAT_BC1_RGBA_UNORM_BLOCK                     : return RESOURCE_FORMAT_BC1_UNORM;
    case VK_FORMAT_BC1_RGBA_SRGB_BLOCK                      : return RESOURCE_FORMAT_BC1_UNORM_SRGB;
    case VK_FORMAT_BC2_UNORM_BLOCK                          : return RESOURCE_FORMAT_BC2_UNORM;
    case VK_FORMAT_BC2_SRGB_BLOCK                           : return RESOURCE_FORMAT_BC2_UNORM_SRGB;
    case VK_FORMAT_BC3_UNORM_BLOCK                          : return RESOURCE_FORMAT_BC3_UNORM;
    case VK_FORMAT_BC3_SRGB_BLOCK                           : return RESOURCE_FORMAT_BC3_UNORM_SRGB;
    case VK_FORMAT_BC4_UNORM_BLOCK                          : return RESOURCE_FORMAT_BC4_UNORM;
    case VK_FORMAT_BC4_SNORM_BLOCK                          : return RESOURCE_FORMAT_BC4_SNORM;
    case VK_FORMAT_BC5_UNORM_BLOCK                          : return RESOURCE_FORMAT_BC5_UNORM;
    case VK_FORMAT_BC5_SNORM_BLOCK                          : return RESOURCE_FORMAT_BC5_SNORM;
    case VK_FORMAT_BC6H_UFLOAT_BLOCK                        : return RESOURCE_FORMAT_BC6H_UF16;
    case VK_FORMAT_BC6H_SFLOAT_BLOCK                        : return RESOURCE_FORMAT_BC6H_SF16;
    case VK_FORMAT_BC7_UNORM_BLOCK                          : return RESOURCE_FORMAT_BC7_UNORM;
    case VK_FORMAT_BC7_SRGB_BLOCK                           : return RESOURCE_FORMAT_BC7_UNORM_SRGB;

    // ビデオフォーマット
    case VK_FORMAT_G8B8G8R8_422_UNORM                       : return RESOURCE_FORMAT_Y8U8Y8V8_422_UNORM;
    case VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16   : return RESOURCE_FORMAT_X6Y10X6U10X6Y10X6V10_422_UNORM;
    case VK_FORMAT_G16B16G16R16_422_UNORM                   : return RESOURCE_FORMAT_Y16U16Y16V16_422_UNORM;
    case VK_FORMAT_G8_B8R8_2PLANE_420_UNORM                 : return RESOURCE_FORMAT_Y8_U8V8_2PLANE_420_UNORM;
    //case VK_FORMAT_R8_UNORM                               : return RESOURCE_FORMAT_Y8_U8V8_2PLANE_420_UNORM_PLANE0;
    //case VK_FORMAT_R8G8_UNORM                             : return RESOURCE_FORMAT_Y8_U8V8_2PLANE_420_UNORM_PLANE1;
    case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16: return RESOURCE_FORMAT_X6Y10_X6U10X6V10_2PLANE_420_UNORM;
    case VK_FORMAT_R10X6_UNORM_PACK16                       : return RESOURCE_FORMAT_X6Y10_X6U10X6V10_2PLANE_420_UNORM_PLANE0;
    case VK_FORMAT_R10X6G10X6_UNORM_2PACK16                 : return RESOURCE_FORMAT_X6Y10_X6U10X6V10_2PLANE_420_UNORM_PLANE1;
    case VK_FORMAT_G16_B16R16_2PLANE_420_UNORM              : return RESOURCE_FORMAT_Y16_U16V16_2PLANE_420_UNORM;
    //case VK_FORMAT_R16_UNORM                              : return RESOURCE_FORMAT_Y16_U16V16_2PLANE_420_UNORM_PLANE0;
    //case VK_FORMAT_R16G16_UNORM                           : return RESOURCE_FORMAT_Y16_U16V16_2PLANE_420_UNORM_PLANE1;

    // 圧縮フォーマット_VK
    case VK_FORMAT_BC1_RGB_UNORM_BLOCK                      : return RESOURCE_FORMAT_BC1_RGB_UNORM_BLOCK_VK;
    case VK_FORMAT_BC1_RGB_SRGB_BLOCK                       : return RESOURCE_FORMAT_BC1_RGB_SRGB_BLOCK_VK;

    case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK                  : return RESOURCE_FORMAT_ETC2_R8G8B8_UNORM_BLOCK_VK;
    case VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK                   : return RESOURCE_FORMAT_ETC2_R8G8B8_SRGB_BLOCK_VK;
    case VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK                : return RESOURCE_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK_VK;
    case VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK                 : return RESOURCE_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK_VK;
    case VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK                : return RESOURCE_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK_VK;
    case VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK                 : return RESOURCE_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK_VK;

    case VK_FORMAT_EAC_R11_UNORM_BLOCK                      : return RESOURCE_FORMAT_EAC_R11_UNORM_BLOCK_VK;
    case VK_FORMAT_EAC_R11_SNORM_BLOCK                      : return RESOURCE_FORMAT_EAC_R11_SNORM_BLOCK_VK;
    case VK_FORMAT_EAC_R11G11_UNORM_BLOCK                   : return RESOURCE_FORMAT_EAC_R11G11_UNORM_BLOCK_VK;
    case VK_FORMAT_EAC_R11G11_SNORM_BLOCK                   : return RESOURCE_FORMAT_EAC_R11G11_SNORM_BLOCK_VK;

    case VK_FORMAT_ASTC_4x4_UNORM_BLOCK                     : return RESOURCE_FORMAT_ASTC_4x4_UNORM_BLOCK_VK;
    case VK_FORMAT_ASTC_4x4_SRGB_BLOCK                      : return RESOURCE_FORMAT_ASTC_4x4_SRGB_BLOCK_VK;
    case VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK_EXT                : return RESOURCE_FORMAT_ASTC_4x4_SFLOAT_BLOCK_EXT_VK;
    case VK_FORMAT_ASTC_5x4_UNORM_BLOCK                     : return RESOURCE_FORMAT_ASTC_5x4_UNORM_BLOCK_VK;
    case VK_FORMAT_ASTC_5x4_SRGB_BLOCK                      : return RESOURCE_FORMAT_ASTC_5x4_SRGB_BLOCK_VK;
    case VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK_EXT                : return RESOURCE_FORMAT_ASTC_5x4_SFLOAT_BLOCK_EXT_VK;
    case VK_FORMAT_ASTC_5x5_UNORM_BLOCK                     : return RESOURCE_FORMAT_ASTC_5x5_UNORM_BLOCK_VK;
    case VK_FORMAT_ASTC_5x5_SRGB_BLOCK                      : return RESOURCE_FORMAT_ASTC_5x5_SRGB_BLOCK_VK;
    case VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK_EXT                : return RESOURCE_FORMAT_ASTC_5x5_SFLOAT_BLOCK_EXT_VK;
    case VK_FORMAT_ASTC_6x5_UNORM_BLOCK                     : return RESOURCE_FORMAT_ASTC_6x5_UNORM_BLOCK_VK;
    case VK_FORMAT_ASTC_6x5_SRGB_BLOCK                      : return RESOURCE_FORMAT_ASTC_6x5_SRGB_BLOCK_VK;
    case VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK_EXT                : return RESOURCE_FORMAT_ASTC_6x5_SFLOAT_BLOCK_EXT_VK;
    case VK_FORMAT_ASTC_6x6_UNORM_BLOCK                     : return RESOURCE_FORMAT_ASTC_6x6_UNORM_BLOCK_VK;
    case VK_FORMAT_ASTC_6x6_SRGB_BLOCK                      : return RESOURCE_FORMAT_ASTC_6x6_SRGB_BLOCK_VK;
    case VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK_EXT                : return RESOURCE_FORMAT_ASTC_6x6_SFLOAT_BLOCK_EXT_VK;
    case VK_FORMAT_ASTC_8x5_UNORM_BLOCK                     : return RESOURCE_FORMAT_ASTC_8x5_UNORM_BLOCK_VK;
    case VK_FORMAT_ASTC_8x5_SRGB_BLOCK                      : return RESOURCE_FORMAT_ASTC_8x5_SRGB_BLOCK_VK;
    case VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK_EXT                : return RESOURCE_FORMAT_ASTC_8x5_SFLOAT_BLOCK_EXT_VK;
    case VK_FORMAT_ASTC_8x6_UNORM_BLOCK                     : return RESOURCE_FORMAT_ASTC_8x6_UNORM_BLOCK_VK;
    case VK_FORMAT_ASTC_8x6_SRGB_BLOCK                      : return RESOURCE_FORMAT_ASTC_8x6_SRGB_BLOCK_VK;
    case VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK_EXT                : return RESOURCE_FORMAT_ASTC_8x6_SFLOAT_BLOCK_EXT_VK;
    case VK_FORMAT_ASTC_8x8_UNORM_BLOCK                     : return RESOURCE_FORMAT_ASTC_8x8_UNORM_BLOCK_VK;
    case VK_FORMAT_ASTC_8x8_SRGB_BLOCK                      : return RESOURCE_FORMAT_ASTC_8x8_SRGB_BLOCK_VK;
    case VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK_EXT                : return RESOURCE_FORMAT_ASTC_8x8_SFLOAT_BLOCK_EXT_VK;
    case VK_FORMAT_ASTC_10x5_UNORM_BLOCK                    : return RESOURCE_FORMAT_ASTC_10x5_UNORM_BLOCK_VK;
    case VK_FORMAT_ASTC_10x5_SRGB_BLOCK                     : return RESOURCE_FORMAT_ASTC_10x5_SRGB_BLOCK_VK;
    case VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK_EXT               : return RESOURCE_FORMAT_ASTC_10x5_SFLOAT_BLOCK_EXT_VK;
    case VK_FORMAT_ASTC_10x6_UNORM_BLOCK                    : return RESOURCE_FORMAT_ASTC_10x6_UNORM_BLOCK_VK;
    case VK_FORMAT_ASTC_10x6_SRGB_BLOCK                     : return RESOURCE_FORMAT_ASTC_10x6_SRGB_BLOCK_VK;
    case VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK_EXT               : return RESOURCE_FORMAT_ASTC_10x6_SFLOAT_BLOCK_EXT_VK;
    case VK_FORMAT_ASTC_10x8_UNORM_BLOCK                    : return RESOURCE_FORMAT_ASTC_10x8_UNORM_BLOCK_VK;
    case VK_FORMAT_ASTC_10x8_SRGB_BLOCK                     : return RESOURCE_FORMAT_ASTC_10x8_SRGB_BLOCK_VK;
    case VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK_EXT               : return RESOURCE_FORMAT_ASTC_10x8_SFLOAT_BLOCK_EXT_VK;
    case VK_FORMAT_ASTC_10x10_UNORM_BLOCK                   : return RESOURCE_FORMAT_ASTC_10x10_UNORM_BLOCK_VK;
    case VK_FORMAT_ASTC_10x10_SRGB_BLOCK                    : return RESOURCE_FORMAT_ASTC_10x10_SRGB_BLOCK_VK;
    case VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK_EXT              : return RESOURCE_FORMAT_ASTC_10x10_SFLOAT_BLOCK_EXT_VK;
    case VK_FORMAT_ASTC_12x10_UNORM_BLOCK                   : return RESOURCE_FORMAT_ASTC_12x10_UNORM_BLOCK_VK;
    case VK_FORMAT_ASTC_12x10_SRGB_BLOCK                    : return RESOURCE_FORMAT_ASTC_12x10_SRGB_BLOCK_VK;
    case VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK_EXT              : return RESOURCE_FORMAT_ASTC_12x10_SFLOAT_BLOCK_EXT_VK;
    case VK_FORMAT_ASTC_12x12_UNORM_BLOCK                   : return RESOURCE_FORMAT_ASTC_12x12_UNORM_BLOCK_VK;
    case VK_FORMAT_ASTC_12x12_SRGB_BLOCK                    : return RESOURCE_FORMAT_ASTC_12x12_SRGB_BLOCK_VK;
    case VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK_EXT              : return RESOURCE_FORMAT_ASTC_12x12_SFLOAT_BLOCK_EXT_VK;

    case VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG              : return RESOURCE_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG_VK;
    case VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG              : return RESOURCE_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG_VK;
    case VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG              : return RESOURCE_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG_VK;
    case VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG              : return RESOURCE_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG_VK;
    case VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG               : return RESOURCE_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG_VK;
    case VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG               : return RESOURCE_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG_VK;
    case VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG               : return RESOURCE_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG_VK;
    case VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG               : return RESOURCE_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG_VK;

    default:
        return RESOURCE_FORMAT_UNKNOWN;
    }
}


VulkanFormatProperties::VulkanFormatProperties()
    : instance   {}
    , physical_device {}
    , device   {}
{
    // TODO: VkPhysicalDeviceImageFormatInfo2
    //{
    //   VkPhysicalDeviceImageFormatInfo2    info      { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_FORMAT_INFO_2 };
    //   VkImageFormatListCreateInfo      format_list_create_info  { VK_STRUCTURE_TYPE_IMAGE_FORMAT_LIST_CREATE_INFO };
    //   VkImageStencilUsageCreateInfo     stencil_usage_create_info { VK_STRUCTURE_TYPE_IMAGE_STENCIL_USAGE_CREATE_INFO };
    //   VkPhysicalDeviceExternalImageFormatInfo   external_image_format_info { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_IMAGE_FORMAT_INFO };
    //   VkPhysicalDeviceImageDrmFormatModifierInfoEXT drm_format_modifier_info { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_DRM_FORMAT_MODIFIER_INFO_EXT };
    //   VkPhysicalDeviceImageViewImageFormatInfoEXT  image_view_image_format_info{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_VIEW_IMAGE_FORMAT_INFO_EXT };
    //   
    //   VkImageFormatProperties2      props          { VK_STRUCTURE_TYPE_IMAGE_FORMAT_PROPERTIES_2 };
    //   VkExternalImageFormatProperties     external_image_format_props     { VK_STRUCTURE_TYPE_EXTERNAL_IMAGE_FORMAT_PROPERTIES };
    //   VkFilterCubicImageViewImageFormatPropertiesEXT filter_cubic_image_view_image_format_props { VK_STRUCTURE_TYPE_FILTER_CUBIC_IMAGE_VIEW_IMAGE_FORMAT_PROPERTIES_EXT };
    //   VkSamplerYcbcrConversionImageFormatProperties sampler_ycbcr_conversion_image_format_props { VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_IMAGE_FORMAT_PROPERTIES };
    //   VkTextureLODGatherFormatPropertiesAMD   texture_lod_gather_format_props_amd   { VK_STRUCTURE_TYPE_TEXTURE_LOD_GATHER_FORMAT_PROPERTIES_AMD };
    //   
    //   const void** last_info_pnext = &info.pNext;
    //   const void** last_props_pnext = &props.pNext;
    //   
    //   // tilingは、pNextチェーンにVkPhysicalDeviceImageDrmFormatModifierInfoEXTが含まれている場合にのみ、VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXTでなければなりません。
    //   info.format = _format;
    //   info.type = _imageType;
    //   info.tiling = _tiling;
    //   info.usage = _usage;
    //   info.flags = _flags;
    //   
    //   // tilingがVK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXTであり、flagsにVK_IMAGE_CREATE_MUTABLE_FORMAT_BITが含まれている場合、pNextチェーンには、viewFormatCountがゼロ以外のVkImageFormatListCreateInfo構造を含める必要があります。
    //   if (info.tiling == VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT && info.flags & VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT)
    //   {
    //    format_list_create_info.viewFormatCount;
    //    format_list_create_info.pViewFormats;
    //    last_info_pnext = util::ConnectPNextChains(last_info_pnext, format_list_create_info);
    //   }
    //   
    //   // この構造をVkPhysicalDeviceImageFormatInfo2のpNextチェーンに含めて、vkGetPhysicalDeviceImageFormatProperties2を使用して画像のステンシルアスペクト用の個別のusageフラグセットなど、画像作成パラメーターの組み合わせに固有の追加機能をクエリすることもできます。
    //   if (false)
    //   {
    //    stencil_usage_create_info.stencilUsage;
    //    last_info_pnext = util::ConnectPNextChains(last_info_pnext, stencil_usage_create_info);
    //   }
    //   
    //   // Linux DRM フォーマット修飾子 と互換性のある画像機能をクエリする
    //   if (info.tiling == VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT)// VkPhysicalDeviceImageFormatInfo2::tilingをVK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXTに設定し
    //   {
    //    drm_format_modifier_info.drmFormatModifier;
    //    drm_format_modifier_info.sharingMode;
    //    drm_format_modifier_info.queueFamilyIndexCount;
    //    drm_format_modifier_info.pQueueFamilyIndices;
    //    last_info_pnext = util::ConnectPNextChains(last_info_pnext, drm_format_modifier_info);
    //   }
    //   
    //   /*特定の画像フォーマットと特定の画像ビュータイプでキュービックフィルタリングを使用できるかどうかを判断するには、
    //   VkPhysicalDeviceImageViewImageFormatInfoEXT構造をVkPhysicalDeviceImageFormatInfo2構造のpNextチェーンに追加し、
    //   VkFilterCubicImageViewImageFormatPropertiesEXT構造をVkImageFormatProperties2構造のpNextチェーンに追加します。*/
    //   if (false)
    //   {
    //    image_view_image_format_info.imageViewType;
    //    last_info_pnext = util::ConnectPNextChains(last_info_pnext, image_view_image_format_info);
    //    last_props_pnext = util::ConnectPNextChains(last_props_pnext, filter_cubic_image_view_image_format_props);
    //   }
    //   
    //   // マルチプラナー形式をサポートするために必要な結合画像サンプラーの数を決定する
    //   if (false)
    //   {
    //    last_props_pnext = util::ConnectPNextChains(last_props_pnext, sampler_ycbcr_conversion_image_format_props);
    //   }
    //   
    //   // 明示的なLODやバイアス引数の値を取るテクスチャギャザー関数を特定の画像形式で使用できるかどうかを判断する
    //   if (false)
    //   {
    //    // VK_AMD_texture_gather_bias_lod拡張機能によって導入されたように、画像フォーマットをテクスチャギャザーバイアス/ LOD関数で使用できる場合、通知します。
    //    last_props_pnext = util::ConnectPNextChains(last_props_pnext, texture_lod_gather_format_props_amd);
    //   }
    //   
    //   /*外部メモリハンドルタイプと互換性のあるイメージ機能を判別するには、
    //   VkPhysicalDeviceExternalImageFormatInfo構造をVkPhysicalDeviceImageFormatInfo2構造のpNextチェーンに追加し、VkExternalImageFormatProperties構造をVkImageFormatProperties2構造のpNextチェーンに追加します。*/
    //   if (false)
    //   {
    //    external_image_format_info.handleType;
    //    last_info_pnext = util::ConnectPNextChains(last_info_pnext, external_image_format_info);
    //    last_props_pnext = util::ConnectPNextChains(last_props_pnext, external_image_format_props);
    //   }
    //
    //#if B3D_PLATFORM_IS_USED_ANDROID
    // // 特定のイメージ作成パラメーターに対して最適なAndroidハードウェアバッファー使用フラグを取得する
    // VkAndroidHardwareBufferUsageANDROID android_hardware_buffer_usage{ VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_USAGE_ANDROID };
    //   if (false)
    //   {
    //    last_props_pnext = util::ConnectPNextChains(last_props_pnext, android_hardware_buffer_usage);
    //   }
    //#endif // B3D_PLATFORM_IS_USED_ANDROID
    //  
    //     // vkGetPhysicalDeviceImageFormatProperties2へのパラメータの組み合わせがvkCreateImageで使用するための実装でサポートされていない場合、imageFormatPropertiesのすべてのメンバーがゼロで埋められます。
    //     auto vkr = vkGetPhysicalDeviceImageFormatProperties2(physical_device, &info, &props);
    //     auto mbr = VKR_TRACE_IF_FAILED(vkr);
    //     if (mbr != BMRESULT_SUCCEED)
    //     { 
    //     
    //     }
    //  }
    //  VkFormatProperties fp{};
    //  vkGetPhysicalDeviceFormatProperties(physical_device, VK_FORMAT_R16_UINT, &fp);
    //  auto image_features = fp.linearTilingFeatures | fp.optimalTilingFeatures;
    //  
    //  if (fp.bufferFeatures)
    //   data->Support1 |= D3D12_FORMAT_SUPPORT1_BUFFER;
    //  
    //  if (fp.bufferFeatures & VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT)
    //   data->Support1 |= D3D12_FORMAT_SUPPORT1_IA_VERTEX_BUFFER;
    //  
    //  if (data->Format == DXGI_FORMAT_R16_UINT || data->Format == DXGI_FORMAT_R32_UINT)
    //   data->Support1 |= D3D12_FORMAT_SUPPORT1_IA_INDEX_BUFFER;
    //  
    //  if (image_features)
    //   data->Support1 |= D3D12_FORMAT_SUPPORT1_TEXTURE1D | 
    //         D3D12_FORMAT_SUPPORT1_TEXTURE2D | 
    //         D3D12_FORMAT_SUPPORT1_TEXTURE3D | 
    //         D3D12_FORMAT_SUPPORT1_TEXTURECUBE;
    //  
    //  if (image_features & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT)
    //  {
    //   data->Support1 |= D3D12_FORMAT_SUPPORT1_SHADER_LOAD | D3D12_FORMAT_SUPPORT1_MULTISAMPLE_LOAD | D3D12_FORMAT_SUPPORT1_SHADER_GATHER;
    //   
    //   if (image_features & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)
    //    data->Support1 |= D3D12_FORMAT_SUPPORT1_SHADER_SAMPLE | D3D12_FORMAT_SUPPORT1_MIP;
    //  
    //   if (format->vk_aspect_mask & VK_IMAGE_ASPECT_DEPTH_BIT)
    //    data->Support1 |= D3D12_FORMAT_SUPPORT1_SHADER_SAMPLE_COMPARISON | D3D12_FORMAT_SUPPORT1_SHADER_GATHER_COMPARISON;
    //  }
    //  
    //  if (image_features & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT)
    //   data->Support1 |= D3D12_FORMAT_SUPPORT1_RENDER_TARGET | D3D12_FORMAT_SUPPORT1_MULTISAMPLE_RENDERTARGET;
    //  
    //  if (image_features & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT)
    //   data->Support1 |= D3D12_FORMAT_SUPPORT1_BLENDABLE;
    //  
    //  if (image_features & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
    //   data->Support1 |= D3D12_FORMAT_SUPPORT1_DEPTH_STENCIL;
    //  
    //  if (image_features & VK_FORMAT_FEATURE_BLIT_SRC_BIT)
    //   data->Support1 |= D3D12_FORMAT_SUPPORT1_MULTISAMPLE_RESOLVE;
    //  
    //  if (image_features & VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT)
    //   data->Support1 |= D3D12_FORMAT_SUPPORT1_TYPED_UNORDERED_ACCESS_VIEW;
    //  
    //  if (image_features & VK_FORMAT_FEATURE_STORAGE_IMAGE_ATOMIC_BIT)
    //   data->Support2 |= D3D12_FORMAT_SUPPORT2_UAV_ATOMIC_ADD  
    //         | D3D12_FORMAT_SUPPORT2_UAV_ATOMIC_BITWISE_OPS  
    //         | D3D12_FORMAT_SUPPORT2_UAV_ATOMIC_COMPARE_STORE_OR_COMPARE_EXCHANGE  
    //         | D3D12_FORMAT_SUPPORT2_UAV_ATOMIC_EXCHANGE 
    //         | D3D12_FORMAT_SUPPORT2_UAV_ATOMIC_SIGNED_MIN_OR_MAX  
    //         | D3D12_FORMAT_SUPPORT2_UAV_ATOMIC_UNSIGNED_MIN_OR_MAX;
}

void VulkanFormatProperties::Init(VkInstance _instance, VkPhysicalDevice _physical_device, VkDevice _device)
{
    instance        = _instance;
    physical_device = _physical_device;
    device          = _device;

    for (int i = FORMAT_OFFSET; i <= FORMAT_MAX; i++)
        PrepareAndEvaluateFormatProperties(SCAST<VkFormat>(i));

    for (int i = MULTIPLANAR_FORMAT_OFFSET; i <= MULTIPLANAR_FORMAT_MAX; i++)
        PrepareAndEvaluateFormatProperties(SCAST<VkFormat>(i));

    for (int i = PVRTC_FORMAT_OFFSET; i <= PVRTC_FORMAT_MAX; i++)
        PrepareAndEvaluateFormatProperties(SCAST<VkFormat>(i));

    for (int i = EXT_ASTC_FORMAT_OFFSET; i <= EXT_ASTC_FORMAT_MAX; i++)
        PrepareAndEvaluateFormatProperties(SCAST<VkFormat>(i));
}

void VulkanFormatProperties::PrepareAndEvaluateFormatProperties(VkFormat _format)
{
    auto&& props = formats_props[_format];
    CreateProperties(props);
    GetVkFormatProperties2(_format, props.get());
    if (HasSupportedAnyFeature(props.get()))
        props->b3d_format = GetB3DFormat(_format);
    else
        props.reset();
}

void VulkanFormatProperties::CreateProperties(util::SharedPtr<VULKAN_FORMAT_PROPERTIES>& _props)
{
    _props = B3DMakeShared(VULKAN_FORMAT_PROPERTIES);
    _props->format_props.sType = VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_2;
    // TODO: VulkanFormatProperties::CreateProperties: VkDrmFormatModifierPropertiesListEXT, VkDrmFormatModifierPropertiesEXT
    if (false)
    { 
        _props->drm_format_modifier_props_list  = B3DMakeUniqueArgs(VkDrmFormatModifierPropertiesListEXT, VkDrmFormatModifierPropertiesListEXT{ VK_STRUCTURE_TYPE_DRM_FORMAT_MODIFIER_PROPERTIES_LIST_EXT } );
        _props->drm_format_modifier_props       = B3DMakeUnique(util::DyArray<VkDrmFormatModifierPropertiesEXT>);
    }
}

void VulkanFormatProperties::GetVkFormatProperties2(VkFormat _format, VULKAN_FORMAT_PROPERTIES* _dst_format_props)
{
    auto fp = &_dst_format_props->format_props;  
    vkGetPhysicalDeviceFormatProperties2(physical_device, _format, fp);

    if (_dst_format_props->drm_format_modifier_props_list)
    {
        auto&& mods_list= *_dst_format_props->drm_format_modifier_props_list;
        auto&& mods       = *_dst_format_props->drm_format_modifier_props;

        // pDrmFormatModifierPropertiesがNULLの場合、vkGetPhysicalDeviceFormatProperties2は、照会されたフォーマットと互換性のある修飾子の数をdrmFormatModifierCountに返します。
        mods.resize(mods_list.drmFormatModifierCount);
        mods_list.pDrmFormatModifierProperties = mods.data();
        
        vkGetPhysicalDeviceFormatProperties2(physical_device, _format, fp);
    }
}

bool VulkanFormatProperties::HasSupportedAnyFeature(VULKAN_FORMAT_PROPERTIES* _props)
{
    return _props->format_props.formatProperties.bufferFeatures        != 0 ||
           _props->format_props.formatProperties.linearTilingFeatures  != 0 ||
           _props->format_props.formatProperties.optimalTilingFeatures != 0;
}


// TEXTURE_FORMAT_DESC内に含まれるフォーマットが有効かどうか調べて結果を返します。
const util::SharedPtr<TYPELESS_COMPATIBLE_FORMATS> FormatCompatibilityChecker::CheckCompatibility(const TEXTURE_FORMAT_DESC& _desc) const
{
    // 使用中のデバイスで利用可能な互換性のあるフォーマットは存在しません。
    auto&& f = compatible_formats.at(_desc.format);
    if (f == nullptr)
        return nullptr;

    auto&& cf = *f->compatible_formats;
    for (uint32_t i = 0, size = _desc.num_mutable_formats; i < size; i++)
    {
        if (std::find(cf.begin(), cf.end(), _desc.mutable_formats[i]) == cf.end())
            return nullptr;
    }

    return f;
}

void FormatCompatibilityChecker::Init(const VulkanFormatProperties& _vulkan_format_properties)
{
    auto&& props = _vulkan_format_properties.GetFormatsProperties();

    /* OPTIMIZE */

    PrepareCompatibleFormatsList(RESOURCE_FORMAT_R8_TYPELESS,
                                 { RESOURCE_FORMAT_R8_UNORM
                                 , RESOURCE_FORMAT_R8_SNORM
                                 , RESOURCE_FORMAT_R8_UINT
                                 , RESOURCE_FORMAT_R8_SINT }
                                 , props);

    PrepareCompatibleFormatsList(RESOURCE_FORMAT_R16_TYPELESS,
                                 { RESOURCE_FORMAT_R16_UNORM
                                 , RESOURCE_FORMAT_R16_SNORM
                                 , RESOURCE_FORMAT_R16_UINT
                                 , RESOURCE_FORMAT_R16_SINT
                                 , RESOURCE_FORMAT_R16_FLOAT }
                                 , props);

    PrepareCompatibleFormatsList(RESOURCE_FORMAT_R32_TYPELESS,
                                 { RESOURCE_FORMAT_R32_UINT
                                 , RESOURCE_FORMAT_R32_SINT
                                 , RESOURCE_FORMAT_R32_FLOAT }
                                 , props);

    PrepareCompatibleFormatsList(RESOURCE_FORMAT_R8G8_TYPELESS,
                                 { RESOURCE_FORMAT_R8G8_UNORM
                                 , RESOURCE_FORMAT_R8G8_SNORM
                                 , RESOURCE_FORMAT_R8G8_UINT
                                 , RESOURCE_FORMAT_R8G8_SINT }
                                 , props);

    PrepareCompatibleFormatsList(RESOURCE_FORMAT_R16G16_TYPELESS,
                                 { RESOURCE_FORMAT_R16G16_UNORM
                                 , RESOURCE_FORMAT_R16G16_SNORM
                                 , RESOURCE_FORMAT_R16G16_UINT
                                 , RESOURCE_FORMAT_R16G16_SINT
                                 , RESOURCE_FORMAT_R16G16_FLOAT }
                                 , props);

    PrepareCompatibleFormatsList(RESOURCE_FORMAT_R32G32_TYPELESS,
                                 { RESOURCE_FORMAT_R32G32_UINT
                                 , RESOURCE_FORMAT_R32G32_SINT
                                 , RESOURCE_FORMAT_R32G32_FLOAT }
                                 , props);

    PrepareCompatibleFormatsList(RESOURCE_FORMAT_R32G32B32_TYPELESS,
                                 { RESOURCE_FORMAT_R32G32B32_UINT
                                 , RESOURCE_FORMAT_R32G32B32_SINT
                                 , RESOURCE_FORMAT_R32G32B32_FLOAT }
                                 , props);

    PrepareCompatibleFormatsList(RESOURCE_FORMAT_R8G8B8A8_TYPELESS,
                                 { RESOURCE_FORMAT_R8G8B8A8_UNORM
                                 , RESOURCE_FORMAT_R8G8B8A8_UNORM_SRGB
                                 , RESOURCE_FORMAT_R8G8B8A8_SNORM
                                 , RESOURCE_FORMAT_R8G8B8A8_UINT
                                 , RESOURCE_FORMAT_R8G8B8A8_SINT }
                                 , props);

    PrepareCompatibleFormatsList(RESOURCE_FORMAT_R10G10B10A2_TYPELESS,
                                 { RESOURCE_FORMAT_R10G10B10A2_UNORM
                                 , RESOURCE_FORMAT_R10G10B10A2_UINT }
                                 , props);

    PrepareCompatibleFormatsList(RESOURCE_FORMAT_R16G16B16A16_TYPELESS,
                                 { RESOURCE_FORMAT_R16G16B16A16_UNORM   
                                 , RESOURCE_FORMAT_R16G16B16A16_SNORM   
                                 , RESOURCE_FORMAT_R16G16B16A16_UINT   
                                 , RESOURCE_FORMAT_R16G16B16A16_SINT   
                                 , RESOURCE_FORMAT_R16G16B16A16_FLOAT }
                                 , props);

    PrepareCompatibleFormatsList(RESOURCE_FORMAT_R32G32B32A32_TYPELESS,
                                 { RESOURCE_FORMAT_R32G32B32A32_UINT
                                 , RESOURCE_FORMAT_R32G32B32A32_SINT
                                 , RESOURCE_FORMAT_R32G32B32A32_FLOAT }
                                 , props);

    PrepareCompatibleFormatsList(RESOURCE_FORMAT_B8G8R8A8_TYPELESS,
                                 { RESOURCE_FORMAT_B8G8R8A8_UNORM
                                 , RESOURCE_FORMAT_B8G8R8A8_UNORM_SRGB }
                                 , props);

    PrepareCompatibleFormatsList( RESOURCE_FORMAT_BC1_TYPELESS,
                                 { RESOURCE_FORMAT_BC1_UNORM
                                 , RESOURCE_FORMAT_BC1_UNORM_SRGB }
                                 , props);

    PrepareCompatibleFormatsList(RESOURCE_FORMAT_BC2_TYPELESS,
                                 { RESOURCE_FORMAT_BC2_UNORM
                                 , RESOURCE_FORMAT_BC2_UNORM_SRGB }
                                 , props);

    PrepareCompatibleFormatsList(RESOURCE_FORMAT_BC3_TYPELESS,
                                 { RESOURCE_FORMAT_BC3_UNORM
                                 , RESOURCE_FORMAT_BC3_UNORM_SRGB }
                                 , props);

    PrepareCompatibleFormatsList(RESOURCE_FORMAT_BC4_TYPELESS,
                                 { RESOURCE_FORMAT_BC4_UNORM
                                 , RESOURCE_FORMAT_BC4_SNORM }
                                 , props);

    PrepareCompatibleFormatsList(RESOURCE_FORMAT_BC5_TYPELESS,
                                 { RESOURCE_FORMAT_BC5_UNORM
                                 , RESOURCE_FORMAT_BC5_SNORM }
                                 , props);

    PrepareCompatibleFormatsList(RESOURCE_FORMAT_BC6H_TYPELESS,
                                 { RESOURCE_FORMAT_BC6H_UF16
                                 , RESOURCE_FORMAT_BC6H_SF16 }
                                 , props);

    PrepareCompatibleFormatsList(RESOURCE_FORMAT_BC7_TYPELESS,
                                 { RESOURCE_FORMAT_BC7_UNORM
                                 , RESOURCE_FORMAT_BC7_UNORM_SRGB }
                                 , props);

    PrepareCompatibleFormatsList(RESOURCE_FORMAT_BC1_RGB_TYPELESS_BLOCK_VK,
                                 { RESOURCE_FORMAT_BC1_RGB_UNORM_BLOCK_VK
                                 , RESOURCE_FORMAT_BC1_RGB_SRGB_BLOCK_VK }
                                 , props);

    PrepareCompatibleFormatsList(RESOURCE_FORMAT_ETC2_R8G8B8_TYPELESS_BLOCK_VK,
                                 { RESOURCE_FORMAT_ETC2_R8G8B8_UNORM_BLOCK_VK
                                 , RESOURCE_FORMAT_ETC2_R8G8B8_SRGB_BLOCK_VK }
                                 , props);

    PrepareCompatibleFormatsList(RESOURCE_FORMAT_ETC2_R8G8B8A1_TYPELESS_BLOCK_VK,
                                 { RESOURCE_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK_VK
                                 , RESOURCE_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK_VK }
                                 , props);

    PrepareCompatibleFormatsList(RESOURCE_FORMAT_ETC2_R8G8B8A8_TYPELESS_BLOCK_VK,
                                 { RESOURCE_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK_VK
                                 , RESOURCE_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK_VK }
                                 , props);

    PrepareCompatibleFormatsList(RESOURCE_FORMAT_EAC_R11_TYPELESS_BLOCK_VK,
                                 { RESOURCE_FORMAT_EAC_R11_UNORM_BLOCK_VK
                                 , RESOURCE_FORMAT_EAC_R11_SNORM_BLOCK_VK }
                                 , props);

    PrepareCompatibleFormatsList(RESOURCE_FORMAT_EAC_R11G11_TYPELESS_BLOCK_VK,
                                 { RESOURCE_FORMAT_EAC_R11G11_UNORM_BLOCK_VK
                                 , RESOURCE_FORMAT_EAC_R11G11_SNORM_BLOCK_VK }
                                 , props);

    PrepareCompatibleFormatsList(RESOURCE_FORMAT_ASTC_4x4_TYPELESS_BLOCK_VK,
                                 { RESOURCE_FORMAT_ASTC_4x4_UNORM_BLOCK_VK
                                 , RESOURCE_FORMAT_ASTC_4x4_SRGB_BLOCK_VK
                                 , RESOURCE_FORMAT_ASTC_4x4_SFLOAT_BLOCK_EXT_VK }
                                 , props);

    PrepareCompatibleFormatsList(RESOURCE_FORMAT_ASTC_5x4_TYPELESS_BLOCK_VK,
                                 { RESOURCE_FORMAT_ASTC_5x4_UNORM_BLOCK_VK
                                 , RESOURCE_FORMAT_ASTC_5x4_SRGB_BLOCK_VK
                                 , RESOURCE_FORMAT_ASTC_5x4_SFLOAT_BLOCK_EXT_VK }
                                 , props);

    PrepareCompatibleFormatsList(RESOURCE_FORMAT_ASTC_5x5_TYPELESS_BLOCK_VK,
                                 { RESOURCE_FORMAT_ASTC_5x5_UNORM_BLOCK_VK
                                 , RESOURCE_FORMAT_ASTC_5x5_SRGB_BLOCK_VK
                                 , RESOURCE_FORMAT_ASTC_5x5_SFLOAT_BLOCK_EXT_VK }
                                 , props);

    PrepareCompatibleFormatsList(RESOURCE_FORMAT_ASTC_6x5_TYPELESS_BLOCK_VK,
                                 { RESOURCE_FORMAT_ASTC_6x5_UNORM_BLOCK_VK
                                 , RESOURCE_FORMAT_ASTC_6x5_SRGB_BLOCK_VK
                                 , RESOURCE_FORMAT_ASTC_6x5_SFLOAT_BLOCK_EXT_VK }
                                 , props);

    PrepareCompatibleFormatsList(RESOURCE_FORMAT_ASTC_6x6_TYPELESS_BLOCK_VK,
                                 { RESOURCE_FORMAT_ASTC_6x6_UNORM_BLOCK_VK
                                 , RESOURCE_FORMAT_ASTC_6x6_SRGB_BLOCK_VK
                                 , RESOURCE_FORMAT_ASTC_6x6_SFLOAT_BLOCK_EXT_VK }
                                 , props);

    PrepareCompatibleFormatsList(RESOURCE_FORMAT_ASTC_8x5_TYPELESS_BLOCK_VK,
                                 { RESOURCE_FORMAT_ASTC_8x5_UNORM_BLOCK_VK
                                 , RESOURCE_FORMAT_ASTC_8x5_SRGB_BLOCK_VK
                                 , RESOURCE_FORMAT_ASTC_8x5_SFLOAT_BLOCK_EXT_VK }
                                 , props);

    PrepareCompatibleFormatsList(RESOURCE_FORMAT_ASTC_8x6_TYPELESS_BLOCK_VK,
                                 { RESOURCE_FORMAT_ASTC_8x6_UNORM_BLOCK_VK
                                 , RESOURCE_FORMAT_ASTC_8x6_SRGB_BLOCK_VK
                                 , RESOURCE_FORMAT_ASTC_8x6_SFLOAT_BLOCK_EXT_VK }
                                 , props);

    PrepareCompatibleFormatsList(RESOURCE_FORMAT_ASTC_8x8_TYPELESS_BLOCK_VK,
                                 { RESOURCE_FORMAT_ASTC_8x8_UNORM_BLOCK_VK
                                 , RESOURCE_FORMAT_ASTC_8x8_SRGB_BLOCK_VK
                                 , RESOURCE_FORMAT_ASTC_8x8_SFLOAT_BLOCK_EXT_VK }
                                 , props);

    PrepareCompatibleFormatsList(RESOURCE_FORMAT_ASTC_10x5_TYPELESS_BLOCK_VK,
                                 { RESOURCE_FORMAT_ASTC_10x5_UNORM_BLOCK_VK
                                 , RESOURCE_FORMAT_ASTC_10x5_SRGB_BLOCK_VK
                                 , RESOURCE_FORMAT_ASTC_10x5_SFLOAT_BLOCK_EXT_VK }
                                 , props);

    PrepareCompatibleFormatsList(RESOURCE_FORMAT_ASTC_10x6_TYPELESS_BLOCK_VK,
                                 { RESOURCE_FORMAT_ASTC_10x6_UNORM_BLOCK_VK
                                 , RESOURCE_FORMAT_ASTC_10x6_SRGB_BLOCK_VK
                                 , RESOURCE_FORMAT_ASTC_10x6_SFLOAT_BLOCK_EXT_VK }
                                 , props);

    PrepareCompatibleFormatsList(RESOURCE_FORMAT_ASTC_10x8_TYPELESS_BLOCK_VK,
                                 { RESOURCE_FORMAT_ASTC_10x8_UNORM_BLOCK_VK
                                 , RESOURCE_FORMAT_ASTC_10x8_SRGB_BLOCK_VK
                                 , RESOURCE_FORMAT_ASTC_10x8_SFLOAT_BLOCK_EXT_VK }
                                 , props);

    PrepareCompatibleFormatsList(RESOURCE_FORMAT_ASTC_10x10_TYPELESS_BLOCK_VK,
                                 { RESOURCE_FORMAT_ASTC_10x10_UNORM_BLOCK_VK
                                 , RESOURCE_FORMAT_ASTC_10x10_SRGB_BLOCK_VK
                                 , RESOURCE_FORMAT_ASTC_10x10_SFLOAT_BLOCK_EXT_VK }
                                 , props);

    PrepareCompatibleFormatsList(RESOURCE_FORMAT_ASTC_12x10_TYPELESS_BLOCK_VK,
                                 { RESOURCE_FORMAT_ASTC_12x10_UNORM_BLOCK_VK
                                 , RESOURCE_FORMAT_ASTC_12x10_SRGB_BLOCK_VK
                                 , RESOURCE_FORMAT_ASTC_12x10_SFLOAT_BLOCK_EXT_VK }
                                 , props);

    PrepareCompatibleFormatsList(RESOURCE_FORMAT_ASTC_12x12_TYPELESS_BLOCK_VK,
                                 { RESOURCE_FORMAT_ASTC_12x12_UNORM_BLOCK_VK
                                 , RESOURCE_FORMAT_ASTC_12x12_SRGB_BLOCK_VK
                                 , RESOURCE_FORMAT_ASTC_12x12_SFLOAT_BLOCK_EXT_VK }
                                 , props);

}

void FormatCompatibilityChecker::PrepareCompatibleFormatsList(
    RESOURCE_FORMAT                                                              _typeless_format
    , const std::initializer_list<RESOURCE_FORMAT>&                              _all_compatible_formats
    , const util::UnordMap<VkFormat, util::SharedPtr<VULKAN_FORMAT_PROPERTIES>>& _props
)
{
    auto&& compatible_foramt = compatible_formats[_typeless_format]    = B3DMakeShared(TYPELESS_COMPATIBLE_FORMATS);
    auto&& cf                = compatible_foramt->compatible_formats   = B3DMakeShared(util::DyArray<RESOURCE_FORMAT>);
    auto&& cvkf              = compatible_foramt->compatible_vkformats = B3DMakeShared(util::DyArray<VkFormat>);

    cf->reserve(_all_compatible_formats.size());
    cvkf->reserve(_all_compatible_formats.size());
    for (auto& b3df : _all_compatible_formats)
    {
        auto vkf = GetNativeFormat(b3df);
        if (_props.at(vkf) != nullptr)
        {
            cf->push_back(b3df);
            cvkf->push_back(vkf);
        }
    }
    ResetCompatibleFormatsListIfEmpty(compatible_foramt);
}

void FormatCompatibilityChecker::ResetCompatibleFormatsListIfEmpty(util::SharedPtr<TYPELESS_COMPATIBLE_FORMATS> _compatible_formats)
{
    if (_compatible_formats->compatible_formats->empty())
    {
        _compatible_formats.reset();
    }
    else
    {
        _compatible_formats->compatible_formats->shrink_to_fit();
        _compatible_formats->compatible_vkformats->shrink_to_fit();
    }
}


}// namespace util
}// namespace buma3d
