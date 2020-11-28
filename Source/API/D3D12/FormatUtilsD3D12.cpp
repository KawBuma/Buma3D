#include "Buma3DPCH.h"
#include "FormatUtilsD3D12.h"

namespace buma3d
{
namespace util
{

// 列挙などのコンバート

DXGI_COLOR_SPACE_TYPE GetNativeColorSpace(COLOR_SPACE _cs)
{
    /* NOTE: msdnより、DXGIの"G22"の付く値はVulkanの"sRGB"伝達関数である可能性が高い。
    さらにVulkan仕様より "この「ITU」のOETFは、指数が0.5の単純なべき乗関数によって近似されます（したがって、OETF-1 は、指数が2.0の単純なべき関数によって非常に厳密に近似されます）。"https://www.khronos.org/registry/DataFormat/specs/1.3/dataformat.1.3.html#TRANSFER_ITU
    もしこの憶測が正しければDXGIの"G22"伝達関数は Vulkan仕様の"sRGB"伝達関数と同じになるので、Vulkanの、伝達関数 ITU(SMPTE 170M)を使う"BT709"が付いた値と互換性が無いことになる。 
    従ってRGB_FULL_G22_NONE_P709を除く(この値はまさにVK_COLOR_SPACE_SRGB_NONLINEAR_KHRとマッチするので)、DXGIの"P709"の付く全ては
    DXGI_COLOR_SPACE_CUSTOMに設定してこちら側で管理する必要がある。
    ただ、色空間についての知識も経験もまだ無いので検証して確認してみる。*/
    switch (_cs)
    {
    case COLOR_SPACE_SRGB_NONLINEAR  : return DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709;
    case COLOR_SPACE_BT709_LINEAR    : return DXGI_COLOR_SPACE_RGB_FULL_G10_NONE_P709;
    case COLOR_SPACE_BT709_NONLINEAR : return DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709;
    case COLOR_SPACE_HDR10_ST2084_PQ : return DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020;
    default:
        return DXGI_COLOR_SPACE_CUSTOM;
    }
}

COLOR_SPACE GetB3DColorSpace(DXGI_COLOR_SPACE_TYPE _cs)
{
    switch (_cs)
    {
    case DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709    : return COLOR_SPACE_SRGB_NONLINEAR;// COLOR_SPACE_BT709_NONLINEAR
    case DXGI_COLOR_SPACE_RGB_FULL_G10_NONE_P709    : return COLOR_SPACE_BT709_LINEAR;
    case DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020 : return COLOR_SPACE_HDR10_ST2084_PQ;
    case DXGI_COLOR_SPACE_CUSTOM                    : return COLOR_SPACE_CUSTOM;
    default:
        return COLOR_SPACE_CUSTOM;
    }
}

DXGI_FORMAT GetNativeFormat(RESOURCE_FORMAT _format)
{
    switch (_format)
    {
    case RESOURCE_FORMAT_UNKNOWN                                 : return DXGI_FORMAT_UNKNOWN;

    // R
    case RESOURCE_FORMAT_R8_TYPELESS                             : return DXGI_FORMAT_R8_TYPELESS;
    case RESOURCE_FORMAT_R8_UNORM                                : return DXGI_FORMAT_R8_UNORM;
    case RESOURCE_FORMAT_R8_SNORM                                : return DXGI_FORMAT_R8_SNORM;
    case RESOURCE_FORMAT_R8_UINT                                 : return DXGI_FORMAT_R8_UINT;
    case RESOURCE_FORMAT_R8_SINT                                 : return DXGI_FORMAT_R8_SINT;

    case RESOURCE_FORMAT_R16_TYPELESS                            : return DXGI_FORMAT_R16_TYPELESS;
    case RESOURCE_FORMAT_R16_UNORM                               : return DXGI_FORMAT_R16_UNORM;
    case RESOURCE_FORMAT_R16_SNORM                               : return DXGI_FORMAT_R16_SNORM;
    case RESOURCE_FORMAT_R16_UINT                                : return DXGI_FORMAT_R16_UINT;
    case RESOURCE_FORMAT_R16_SINT                                : return DXGI_FORMAT_R16_SINT;
    case RESOURCE_FORMAT_R16_FLOAT                               : return DXGI_FORMAT_R16_FLOAT;

    case RESOURCE_FORMAT_R32_TYPELESS                            : return DXGI_FORMAT_R32_TYPELESS;
    case RESOURCE_FORMAT_R32_UINT                                : return DXGI_FORMAT_R32_UINT;
    case RESOURCE_FORMAT_R32_SINT                                : return DXGI_FORMAT_R32_SINT;
    case RESOURCE_FORMAT_R32_FLOAT                               : return DXGI_FORMAT_R32_FLOAT;

    // RG
    case RESOURCE_FORMAT_R8G8_TYPELESS                           : return DXGI_FORMAT_R8G8_TYPELESS;
    case RESOURCE_FORMAT_R8G8_UNORM                              : return DXGI_FORMAT_R8G8_UNORM;
    case RESOURCE_FORMAT_R8G8_SNORM                              : return DXGI_FORMAT_R8G8_SNORM;
    case RESOURCE_FORMAT_R8G8_UINT                               : return DXGI_FORMAT_R8G8_UINT;
    case RESOURCE_FORMAT_R8G8_SINT                               : return DXGI_FORMAT_R8G8_SINT;

    case RESOURCE_FORMAT_R16G16_TYPELESS                         : return DXGI_FORMAT_R16G16_TYPELESS;
    case RESOURCE_FORMAT_R16G16_UNORM                            : return DXGI_FORMAT_R16G16_UNORM;
    case RESOURCE_FORMAT_R16G16_SNORM                            : return DXGI_FORMAT_R16G16_SNORM;
    case RESOURCE_FORMAT_R16G16_UINT                             : return DXGI_FORMAT_R16G16_UINT;
    case RESOURCE_FORMAT_R16G16_SINT                             : return DXGI_FORMAT_R16G16_SINT;
    case RESOURCE_FORMAT_R16G16_FLOAT                            : return DXGI_FORMAT_R16G16_FLOAT;

    case RESOURCE_FORMAT_R32G32_TYPELESS                         : return DXGI_FORMAT_R32G32_TYPELESS;
    case RESOURCE_FORMAT_R32G32_UINT                             : return DXGI_FORMAT_R32G32_UINT;
    case RESOURCE_FORMAT_R32G32_SINT                             : return DXGI_FORMAT_R32G32_SINT;
    case RESOURCE_FORMAT_R32G32_FLOAT                            : return DXGI_FORMAT_R32G32_FLOAT;
    // GR

    // RGB
    case RESOURCE_FORMAT_R11G11B10_UFLOAT                        : return DXGI_FORMAT_R11G11B10_FLOAT;

    case RESOURCE_FORMAT_R32G32B32_TYPELESS                      : return DXGI_FORMAT_R32G32B32_TYPELESS;
    case RESOURCE_FORMAT_R32G32B32_UINT                          : return DXGI_FORMAT_R32G32B32_UINT;
    case RESOURCE_FORMAT_R32G32B32_SINT                          : return DXGI_FORMAT_R32G32B32_SINT;
    case RESOURCE_FORMAT_R32G32B32_FLOAT                         : return DXGI_FORMAT_R32G32B32_FLOAT;
    // BGR 
    case RESOURCE_FORMAT_B5G6R5_UNORM                            : return DXGI_FORMAT_B5G6R5_UNORM;

    // RGBA
    case RESOURCE_FORMAT_R8G8B8A8_TYPELESS                       : return DXGI_FORMAT_R8G8B8A8_TYPELESS;
    case RESOURCE_FORMAT_R8G8B8A8_UNORM                          : return DXGI_FORMAT_R8G8B8A8_UNORM;
    case RESOURCE_FORMAT_R8G8B8A8_UNORM_SRGB                     : return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    case RESOURCE_FORMAT_R8G8B8A8_SNORM                          : return DXGI_FORMAT_R8G8B8A8_SNORM;
    case RESOURCE_FORMAT_R8G8B8A8_UINT                           : return DXGI_FORMAT_R8G8B8A8_UINT;
    case RESOURCE_FORMAT_R8G8B8A8_SINT                           : return DXGI_FORMAT_R8G8B8A8_SINT;

    case RESOURCE_FORMAT_R10G10B10A2_TYPELESS                    : return DXGI_FORMAT_R10G10B10A2_TYPELESS;
    case RESOURCE_FORMAT_R10G10B10A2_UNORM                       : return DXGI_FORMAT_R10G10B10A2_UNORM;
    case RESOURCE_FORMAT_R10G10B10A2_UINT                        : return DXGI_FORMAT_R10G10B10A2_UINT;

    case RESOURCE_FORMAT_R16G16B16A16_TYPELESS                   : return DXGI_FORMAT_R16G16B16A16_TYPELESS;
    case RESOURCE_FORMAT_R16G16B16A16_UNORM                      : return DXGI_FORMAT_R16G16B16A16_UNORM;
    case RESOURCE_FORMAT_R16G16B16A16_SNORM                      : return DXGI_FORMAT_R16G16B16A16_SNORM;
    case RESOURCE_FORMAT_R16G16B16A16_UINT                       : return DXGI_FORMAT_R16G16B16A16_UINT;
    case RESOURCE_FORMAT_R16G16B16A16_SINT                       : return DXGI_FORMAT_R16G16B16A16_SINT;
    case RESOURCE_FORMAT_R16G16B16A16_FLOAT                      : return DXGI_FORMAT_R16G16B16A16_FLOAT;

    case RESOURCE_FORMAT_R32G32B32A32_TYPELESS                   : return DXGI_FORMAT_R32G32B32A32_TYPELESS;
    case RESOURCE_FORMAT_R32G32B32A32_UINT                       : return DXGI_FORMAT_R32G32B32A32_UINT;
    case RESOURCE_FORMAT_R32G32B32A32_SINT                       : return DXGI_FORMAT_R32G32B32A32_SINT;
    case RESOURCE_FORMAT_R32G32B32A32_FLOAT                      : return DXGI_FORMAT_R32G32B32A32_FLOAT;
    // BGRA 
    case RESOURCE_FORMAT_B5G5R5A1_UNORM                          : return DXGI_FORMAT_B5G5R5A1_UNORM;

    case RESOURCE_FORMAT_B8G8R8A8_TYPELESS                       : return DXGI_FORMAT_B8G8R8A8_TYPELESS;
    case RESOURCE_FORMAT_B8G8R8A8_UNORM                          : return DXGI_FORMAT_B8G8R8A8_UNORM;
    case RESOURCE_FORMAT_B8G8R8A8_UNORM_SRGB                     : return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;

    // RGBE
    case RESOURCE_FORMAT_R9G9B9E5_UFLOAT                         : return DXGI_FORMAT_R9G9B9E5_SHAREDEXP;

    // 深度フォーマット
    case RESOURCE_FORMAT_D16_UNORM                               : return DXGI_FORMAT_D16_UNORM;
    case RESOURCE_FORMAT_D32_FLOAT                               : return DXGI_FORMAT_D32_FLOAT;
    case RESOURCE_FORMAT_D24_UNORM_S8_UINT                       : return DXGI_FORMAT_D24_UNORM_S8_UINT;
    case RESOURCE_FORMAT_D32_FLOAT_S8X24_UINT                    : return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;

    // 圧縮フォーマット
    case RESOURCE_FORMAT_BC1_TYPELESS                            : return DXGI_FORMAT_BC1_TYPELESS;
    case RESOURCE_FORMAT_BC1_UNORM                               : return DXGI_FORMAT_BC1_UNORM;
    case RESOURCE_FORMAT_BC1_UNORM_SRGB                          : return DXGI_FORMAT_BC1_UNORM_SRGB;

    case RESOURCE_FORMAT_BC2_TYPELESS                            : return DXGI_FORMAT_BC2_TYPELESS;
    case RESOURCE_FORMAT_BC2_UNORM                               : return DXGI_FORMAT_BC2_UNORM;
    case RESOURCE_FORMAT_BC2_UNORM_SRGB                          : return DXGI_FORMAT_BC2_UNORM_SRGB;

    case RESOURCE_FORMAT_BC3_TYPELESS                            : return DXGI_FORMAT_BC3_TYPELESS;
    case RESOURCE_FORMAT_BC3_UNORM                               : return DXGI_FORMAT_BC3_UNORM;
    case RESOURCE_FORMAT_BC3_UNORM_SRGB                          : return DXGI_FORMAT_BC3_UNORM_SRGB;

    case RESOURCE_FORMAT_BC4_TYPELESS                            : return DXGI_FORMAT_BC4_TYPELESS;
    case RESOURCE_FORMAT_BC4_UNORM                               : return DXGI_FORMAT_BC4_UNORM;
    case RESOURCE_FORMAT_BC4_SNORM                               : return DXGI_FORMAT_BC4_SNORM;

    case RESOURCE_FORMAT_BC5_TYPELESS                            : return DXGI_FORMAT_BC5_TYPELESS;
    case RESOURCE_FORMAT_BC5_UNORM                               : return DXGI_FORMAT_BC5_UNORM;
    case RESOURCE_FORMAT_BC5_SNORM                               : return DXGI_FORMAT_BC5_SNORM;

    case RESOURCE_FORMAT_BC6H_TYPELESS                           : return DXGI_FORMAT_BC6H_TYPELESS;
    case RESOURCE_FORMAT_BC6H_UF16                               : return DXGI_FORMAT_BC6H_UF16;
    case RESOURCE_FORMAT_BC6H_SF16                               : return DXGI_FORMAT_BC6H_SF16;

    case RESOURCE_FORMAT_BC7_TYPELESS                            : return DXGI_FORMAT_BC7_TYPELESS;
    case RESOURCE_FORMAT_BC7_UNORM                               : return DXGI_FORMAT_BC7_UNORM;
    case RESOURCE_FORMAT_BC7_UNORM_SRGB                          : return DXGI_FORMAT_BC7_UNORM_SRGB;

    // ビデオフォーマット
    case RESOURCE_FORMAT_Y8U8Y8V8_422_UNORM                      : return DXGI_FORMAT_YUY2;
    case RESOURCE_FORMAT_X6Y10X6U10X6Y10X6V10_422_UNORM          : return DXGI_FORMAT_Y210;
    case RESOURCE_FORMAT_Y16U16Y16V16_422_UNORM                  : return DXGI_FORMAT_Y216;
    case RESOURCE_FORMAT_Y8_U8V8_2PLANE_420_UNORM                : return DXGI_FORMAT_NV12;
    case RESOURCE_FORMAT_Y8_U8V8_2PLANE_420_UNORM_PLANE0         : return DXGI_FORMAT_R8_UNORM;
    case RESOURCE_FORMAT_Y8_U8V8_2PLANE_420_UNORM_PLANE1         : return DXGI_FORMAT_R8G8_UNORM;
    case RESOURCE_FORMAT_X6Y10_X6U10X6V10_2PLANE_420_UNORM       : return DXGI_FORMAT_P010;
    case RESOURCE_FORMAT_X6Y10_X6U10X6V10_2PLANE_420_UNORM_PLANE0: return DXGI_FORMAT_R16_UNORM;
    case RESOURCE_FORMAT_X6Y10_X6U10X6V10_2PLANE_420_UNORM_PLANE1: return DXGI_FORMAT_R16G16_UNORM;
    case RESOURCE_FORMAT_Y16_U16V16_2PLANE_420_UNORM             : return DXGI_FORMAT_P016;
    case RESOURCE_FORMAT_Y16_U16V16_2PLANE_420_UNORM_PLANE0      : return DXGI_FORMAT_R16_UNORM;
    case RESOURCE_FORMAT_Y16_U16V16_2PLANE_420_UNORM_PLANE1      : return DXGI_FORMAT_R16G16_UNORM;

    // 圧縮フォーマット_VK
    // case RESOURCE_FORMAT_BC1_RGB_TYPELESS_BLOCK_VK            :
    // case RESOURCE_FORMAT_BC1_RGB_UNORM_BLOCK_VK               :
    // case RESOURCE_FORMAT_BC1_RGB_SRGB_BLOCK_VK                :
    // 
    // case RESOURCE_FORMAT_ETC2_R8G8B8_TYPELESS_BLOCK_VK        :
    // case RESOURCE_FORMAT_ETC2_R8G8B8_UNORM_BLOCK_VK           :
    // case RESOURCE_FORMAT_ETC2_R8G8B8_SRGB_BLOCK_VK            :
    // case RESOURCE_FORMAT_ETC2_R8G8B8A1_TYPELESS_BLOCK_VK      :
    // case RESOURCE_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK_VK         :
    // case RESOURCE_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK_VK          :
    // case RESOURCE_FORMAT_ETC2_R8G8B8A8_TYPELESS_BLOCK_VK      :
    // case RESOURCE_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK_VK         :
    // case RESOURCE_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK_VK          :
    // 
    // case RESOURCE_FORMAT_EAC_R11_TYPELESS_BLOCK_VK            :
    // case RESOURCE_FORMAT_EAC_R11_UNORM_BLOCK_VK               :
    // case RESOURCE_FORMAT_EAC_R11_SNORM_BLOCK_VK               :
    // case RESOURCE_FORMAT_EAC_R11G11_TYPELESS_BLOCK_VK         :
    // case RESOURCE_FORMAT_EAC_R11G11_UNORM_BLOCK_VK            :
    // case RESOURCE_FORMAT_EAC_R11G11_SNORM_BLOCK_VK            :
    // 
    // case RESOURCE_FORMAT_ASTC_4x4_TYPELESS_BLOCK_VK           :
    // case RESOURCE_FORMAT_ASTC_4x4_UNORM_BLOCK_VK              :
    // case RESOURCE_FORMAT_ASTC_4x4_SRGB_BLOCK_VK               :
    // case RESOURCE_FORMAT_ASTC_4x4_SFLOAT_BLOCK_EXT_VK         :
    // case RESOURCE_FORMAT_ASTC_5x4_TYPELESS_BLOCK_VK           :
    // case RESOURCE_FORMAT_ASTC_5x4_UNORM_BLOCK_VK              :
    // case RESOURCE_FORMAT_ASTC_5x4_SRGB_BLOCK_VK               :
    // case RESOURCE_FORMAT_ASTC_5x4_SFLOAT_BLOCK_EXT_VK         :
    // case RESOURCE_FORMAT_ASTC_5x5_TYPELESS_BLOCK_VK           :
    // case RESOURCE_FORMAT_ASTC_5x5_UNORM_BLOCK_VK              :
    // case RESOURCE_FORMAT_ASTC_5x5_SRGB_BLOCK_VK               :
    // case RESOURCE_FORMAT_ASTC_5x5_SFLOAT_BLOCK_EXT_VK         :
    // case RESOURCE_FORMAT_ASTC_6x5_TYPELESS_BLOCK_VK           :
    // case RESOURCE_FORMAT_ASTC_6x5_UNORM_BLOCK_VK              :
    // case RESOURCE_FORMAT_ASTC_6x5_SRGB_BLOCK_VK               :
    // case RESOURCE_FORMAT_ASTC_6x5_SFLOAT_BLOCK_EXT_VK         :
    // case RESOURCE_FORMAT_ASTC_6x6_TYPELESS_BLOCK_VK           :
    // case RESOURCE_FORMAT_ASTC_6x6_UNORM_BLOCK_VK              :
    // case RESOURCE_FORMAT_ASTC_6x6_SRGB_BLOCK_VK               :
    // case RESOURCE_FORMAT_ASTC_6x6_SFLOAT_BLOCK_EXT_VK         :
    // case RESOURCE_FORMAT_ASTC_8x5_TYPELESS_BLOCK_VK           :
    // case RESOURCE_FORMAT_ASTC_8x5_UNORM_BLOCK_VK              :
    // case RESOURCE_FORMAT_ASTC_8x5_SRGB_BLOCK_VK               :
    // case RESOURCE_FORMAT_ASTC_8x5_SFLOAT_BLOCK_EXT_VK         :
    // case RESOURCE_FORMAT_ASTC_8x6_TYPELESS_BLOCK_VK           :
    // case RESOURCE_FORMAT_ASTC_8x6_UNORM_BLOCK_VK              :
    // case RESOURCE_FORMAT_ASTC_8x6_SRGB_BLOCK_VK               :
    // case RESOURCE_FORMAT_ASTC_8x6_SFLOAT_BLOCK_EXT_VK         :
    // case RESOURCE_FORMAT_ASTC_8x8_TYPELESS_BLOCK_VK           :
    // case RESOURCE_FORMAT_ASTC_8x8_UNORM_BLOCK_VK              :
    // case RESOURCE_FORMAT_ASTC_8x8_SRGB_BLOCK_VK               :
    // case RESOURCE_FORMAT_ASTC_8x8_SFLOAT_BLOCK_EXT_VK         :
    // case RESOURCE_FORMAT_ASTC_10x5_TYPELESS_BLOCK_VK          :
    // case RESOURCE_FORMAT_ASTC_10x5_UNORM_BLOCK_VK             :
    // case RESOURCE_FORMAT_ASTC_10x5_SRGB_BLOCK_VK              :
    // case RESOURCE_FORMAT_ASTC_10x5_SFLOAT_BLOCK_EXT_VK        :
    // case RESOURCE_FORMAT_ASTC_10x6_TYPELESS_BLOCK_VK          :
    // case RESOURCE_FORMAT_ASTC_10x6_UNORM_BLOCK_VK             :
    // case RESOURCE_FORMAT_ASTC_10x6_SRGB_BLOCK_VK              :
    // case RESOURCE_FORMAT_ASTC_10x6_SFLOAT_BLOCK_EXT_VK        :
    // case RESOURCE_FORMAT_ASTC_10x8_TYPELESS_BLOCK_VK          :
    // case RESOURCE_FORMAT_ASTC_10x8_UNORM_BLOCK_VK             :
    // case RESOURCE_FORMAT_ASTC_10x8_SRGB_BLOCK_VK              :
    // case RESOURCE_FORMAT_ASTC_10x8_SFLOAT_BLOCK_EXT_VK        :
    // case RESOURCE_FORMAT_ASTC_10x10_TYPELESS_BLOCK_VK         :
    // case RESOURCE_FORMAT_ASTC_10x10_UNORM_BLOCK_VK            :
    // case RESOURCE_FORMAT_ASTC_10x10_SRGB_BLOCK_VK             :
    // case RESOURCE_FORMAT_ASTC_10x10_SFLOAT_BLOCK_EXT_VK       :
    // case RESOURCE_FORMAT_ASTC_12x10_TYPELESS_BLOCK_VK         :
    // case RESOURCE_FORMAT_ASTC_12x10_UNORM_BLOCK_VK            :
    // case RESOURCE_FORMAT_ASTC_12x10_SRGB_BLOCK_VK             :
    // case RESOURCE_FORMAT_ASTC_12x10_SFLOAT_BLOCK_EXT_VK       :
    // case RESOURCE_FORMAT_ASTC_12x12_TYPELESS_BLOCK_VK         :
    // case RESOURCE_FORMAT_ASTC_12x12_UNORM_BLOCK_VK            :
    // case RESOURCE_FORMAT_ASTC_12x12_SRGB_BLOCK_VK             :
    // case RESOURCE_FORMAT_ASTC_12x12_SFLOAT_BLOCK_EXT_VK       :
    // 
    // case RESOURCE_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG_VK       :
    // case RESOURCE_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG_VK       :
    // case RESOURCE_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG_VK       :
    // case RESOURCE_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG_VK       :
    // case RESOURCE_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG_VK        :
    // case RESOURCE_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG_VK        :
    // case RESOURCE_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG_VK        :
    // case RESOURCE_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG_VK        :

    default:
        return DXGI_FORMAT_UNKNOWN;
    }
}

RESOURCE_FORMAT GetB3DFormat(DXGI_FORMAT _format)
{
    switch (_format)
    {
    case DXGI_FORMAT_UNKNOWN                                         : return RESOURCE_FORMAT_UNKNOWN;

    // R
    case DXGI_FORMAT_R8_TYPELESS                                     : return RESOURCE_FORMAT_R8_TYPELESS;
    case DXGI_FORMAT_R8_UNORM                                        : return RESOURCE_FORMAT_R8_UNORM;// RESOURCE_FORMAT_Y8_U8V8_2PLANE_420_UNORM_PLANE0
    case DXGI_FORMAT_R8_SNORM                                        : return RESOURCE_FORMAT_R8_SNORM;
    case DXGI_FORMAT_R8_UINT                                         : return RESOURCE_FORMAT_R8_UINT;
    case DXGI_FORMAT_R8_SINT                                         : return RESOURCE_FORMAT_R8_SINT;

    case DXGI_FORMAT_R16_TYPELESS                                    : return RESOURCE_FORMAT_R16_TYPELESS;
    case DXGI_FORMAT_R16_UNORM                                       : return RESOURCE_FORMAT_R16_UNORM;// RESOURCE_FORMAT_Y16_U16V16_2PLANE_420_UNORM_PLANE0
    case DXGI_FORMAT_R16_SNORM                                       : return RESOURCE_FORMAT_R16_SNORM;
    case DXGI_FORMAT_R16_UINT                                        : return RESOURCE_FORMAT_R16_UINT;
    case DXGI_FORMAT_R16_SINT                                        : return RESOURCE_FORMAT_R16_SINT;
    case DXGI_FORMAT_R16_FLOAT                                       : return RESOURCE_FORMAT_R16_FLOAT;

    case DXGI_FORMAT_R32_TYPELESS                                    : return RESOURCE_FORMAT_R32_TYPELESS;
    case DXGI_FORMAT_R32_UINT                                        : return RESOURCE_FORMAT_R32_UINT;
    case DXGI_FORMAT_R32_SINT                                        : return RESOURCE_FORMAT_R32_SINT;
    case DXGI_FORMAT_R32_FLOAT                                       : return RESOURCE_FORMAT_R32_FLOAT;

    // RG
    case DXGI_FORMAT_R8G8_TYPELESS                                   : return RESOURCE_FORMAT_R8G8_TYPELESS;
    case DXGI_FORMAT_R8G8_UNORM                                      : return RESOURCE_FORMAT_R8G8_UNORM;// RESOURCE_FORMAT_Y8_U8V8_2PLANE_420_UNORM_PLANE1
    case DXGI_FORMAT_R8G8_SNORM                                      : return RESOURCE_FORMAT_R8G8_SNORM;
    case DXGI_FORMAT_R8G8_UINT                                       : return RESOURCE_FORMAT_R8G8_UINT;
    case DXGI_FORMAT_R8G8_SINT                                       : return RESOURCE_FORMAT_R8G8_SINT;

    case DXGI_FORMAT_R16G16_TYPELESS                                 : return RESOURCE_FORMAT_R16G16_TYPELESS;
    case DXGI_FORMAT_R16G16_UNORM                                    : return RESOURCE_FORMAT_R16G16_UNORM;// RESOURCE_FORMAT_Y16_U16V16_2PLANE_420_UNORM_PLANE1
    case DXGI_FORMAT_R16G16_SNORM                                    : return RESOURCE_FORMAT_R16G16_SNORM;
    case DXGI_FORMAT_R16G16_UINT                                     : return RESOURCE_FORMAT_R16G16_UINT;
    case DXGI_FORMAT_R16G16_SINT                                     : return RESOURCE_FORMAT_R16G16_SINT;
    case DXGI_FORMAT_R16G16_FLOAT                                    : return RESOURCE_FORMAT_R16G16_FLOAT;

    case DXGI_FORMAT_R32G32_TYPELESS                                 : return RESOURCE_FORMAT_R32G32_TYPELESS;
    case DXGI_FORMAT_R32G32_UINT                                     : return RESOURCE_FORMAT_R32G32_UINT;
    case DXGI_FORMAT_R32G32_SINT                                     : return RESOURCE_FORMAT_R32G32_SINT;
    case DXGI_FORMAT_R32G32_FLOAT                                    : return RESOURCE_FORMAT_R32G32_FLOAT;
    // GR

    // RGB
    case DXGI_FORMAT_R11G11B10_FLOAT                                 : return RESOURCE_FORMAT_R11G11B10_UFLOAT;

    case DXGI_FORMAT_R32G32B32_TYPELESS                              : return RESOURCE_FORMAT_R32G32B32_TYPELESS;
    case DXGI_FORMAT_R32G32B32_UINT                                  : return RESOURCE_FORMAT_R32G32B32_UINT;
    case DXGI_FORMAT_R32G32B32_SINT                                  : return RESOURCE_FORMAT_R32G32B32_SINT;
    case DXGI_FORMAT_R32G32B32_FLOAT                                 : return RESOURCE_FORMAT_R32G32B32_FLOAT;
    // BGR
    case DXGI_FORMAT_B5G6R5_UNORM                                    : return RESOURCE_FORMAT_B5G6R5_UNORM;

    // RGBA
    case DXGI_FORMAT_R8G8B8A8_TYPELESS                               : return RESOURCE_FORMAT_R8G8B8A8_TYPELESS;
    case DXGI_FORMAT_R8G8B8A8_UNORM                                  : return RESOURCE_FORMAT_R8G8B8A8_UNORM;
    case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB                             : return RESOURCE_FORMAT_R8G8B8A8_UNORM_SRGB;
    case DXGI_FORMAT_R8G8B8A8_SNORM                                  : return RESOURCE_FORMAT_R8G8B8A8_SNORM;
    case DXGI_FORMAT_R8G8B8A8_UINT                                   : return RESOURCE_FORMAT_R8G8B8A8_UINT;
    case DXGI_FORMAT_R8G8B8A8_SINT                                   : return RESOURCE_FORMAT_R8G8B8A8_SINT;

    case DXGI_FORMAT_R10G10B10A2_TYPELESS                            : return RESOURCE_FORMAT_R10G10B10A2_TYPELESS;
    case DXGI_FORMAT_R10G10B10A2_UNORM                               : return RESOURCE_FORMAT_R10G10B10A2_UNORM;
    case DXGI_FORMAT_R10G10B10A2_UINT                                : return RESOURCE_FORMAT_R10G10B10A2_UINT;

    case DXGI_FORMAT_R16G16B16A16_TYPELESS                           : return RESOURCE_FORMAT_R16G16B16A16_TYPELESS;
    case DXGI_FORMAT_R16G16B16A16_UNORM                              : return RESOURCE_FORMAT_R16G16B16A16_UNORM;
    case DXGI_FORMAT_R16G16B16A16_SNORM                              : return RESOURCE_FORMAT_R16G16B16A16_SNORM;
    case DXGI_FORMAT_R16G16B16A16_UINT                               : return RESOURCE_FORMAT_R16G16B16A16_UINT;
    case DXGI_FORMAT_R16G16B16A16_SINT                               : return RESOURCE_FORMAT_R16G16B16A16_SINT;
    case DXGI_FORMAT_R16G16B16A16_FLOAT                              : return RESOURCE_FORMAT_R16G16B16A16_FLOAT;

    case DXGI_FORMAT_R32G32B32A32_TYPELESS                           : return RESOURCE_FORMAT_R32G32B32A32_TYPELESS;
    case DXGI_FORMAT_R32G32B32A32_UINT                               : return RESOURCE_FORMAT_R32G32B32A32_UINT;
    case DXGI_FORMAT_R32G32B32A32_SINT                               : return RESOURCE_FORMAT_R32G32B32A32_SINT;
    case DXGI_FORMAT_R32G32B32A32_FLOAT                              : return RESOURCE_FORMAT_R32G32B32A32_FLOAT;
    // BGRA
    case DXGI_FORMAT_B5G5R5A1_UNORM                                  : return RESOURCE_FORMAT_B5G5R5A1_UNORM;

    case DXGI_FORMAT_B8G8R8A8_TYPELESS                               : return RESOURCE_FORMAT_B8G8R8A8_TYPELESS;
    case DXGI_FORMAT_B8G8R8A8_UNORM                                  : return RESOURCE_FORMAT_B8G8R8A8_UNORM;
    case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB                             : return RESOURCE_FORMAT_B8G8R8A8_UNORM_SRGB;

    // RGBE
    case DXGI_FORMAT_R9G9B9E5_SHAREDEXP                              : return RESOURCE_FORMAT_R9G9B9E5_UFLOAT;

    // 深度フォーマット
    case DXGI_FORMAT_D16_UNORM                                       : return RESOURCE_FORMAT_D16_UNORM;
    case DXGI_FORMAT_D32_FLOAT                                       : return RESOURCE_FORMAT_D32_FLOAT;
    case DXGI_FORMAT_D24_UNORM_S8_UINT                               : return RESOURCE_FORMAT_D24_UNORM_S8_UINT;
    case DXGI_FORMAT_D32_FLOAT_S8X24_UINT                            : return RESOURCE_FORMAT_D32_FLOAT_S8X24_UINT;

    // 圧縮フォーマット
    case DXGI_FORMAT_BC1_TYPELESS                                    : return RESOURCE_FORMAT_BC1_TYPELESS;
    case DXGI_FORMAT_BC1_UNORM                                       : return RESOURCE_FORMAT_BC1_UNORM;
    case DXGI_FORMAT_BC1_UNORM_SRGB                                  : return RESOURCE_FORMAT_BC1_UNORM_SRGB;

    case DXGI_FORMAT_BC2_TYPELESS                                    : return RESOURCE_FORMAT_BC2_TYPELESS;
    case DXGI_FORMAT_BC2_UNORM                                       : return RESOURCE_FORMAT_BC2_UNORM;
    case DXGI_FORMAT_BC2_UNORM_SRGB                                  : return RESOURCE_FORMAT_BC2_UNORM_SRGB;

    case DXGI_FORMAT_BC3_TYPELESS                                    : return RESOURCE_FORMAT_BC3_TYPELESS;
    case DXGI_FORMAT_BC3_UNORM                                       : return RESOURCE_FORMAT_BC3_UNORM;
    case DXGI_FORMAT_BC3_UNORM_SRGB                                  : return RESOURCE_FORMAT_BC3_UNORM_SRGB;

    case DXGI_FORMAT_BC4_TYPELESS                                    : return RESOURCE_FORMAT_BC4_TYPELESS;
    case DXGI_FORMAT_BC4_UNORM                                       : return RESOURCE_FORMAT_BC4_UNORM;
    case DXGI_FORMAT_BC4_SNORM                                       : return RESOURCE_FORMAT_BC4_SNORM;

    case DXGI_FORMAT_BC5_TYPELESS                                    : return RESOURCE_FORMAT_BC5_TYPELESS;
    case DXGI_FORMAT_BC5_UNORM                                       : return RESOURCE_FORMAT_BC5_UNORM;
    case DXGI_FORMAT_BC5_SNORM                                       : return RESOURCE_FORMAT_BC5_SNORM;

    case DXGI_FORMAT_BC6H_TYPELESS                                   : return RESOURCE_FORMAT_BC6H_TYPELESS;
    case DXGI_FORMAT_BC6H_UF16                                       : return RESOURCE_FORMAT_BC6H_UF16;
    case DXGI_FORMAT_BC6H_SF16                                       : return RESOURCE_FORMAT_BC6H_SF16;

    case DXGI_FORMAT_BC7_TYPELESS                                    : return RESOURCE_FORMAT_BC7_TYPELESS;
    case DXGI_FORMAT_BC7_UNORM                                       : return RESOURCE_FORMAT_BC7_UNORM;
    case DXGI_FORMAT_BC7_UNORM_SRGB                                  : return RESOURCE_FORMAT_BC7_UNORM_SRGB;

    // ビデオフォーマット
    case DXGI_FORMAT_YUY2                                            : return RESOURCE_FORMAT_Y8U8Y8V8_422_UNORM;
    case DXGI_FORMAT_Y210                                            : return RESOURCE_FORMAT_X6Y10X6U10X6Y10X6V10_422_UNORM;
    case DXGI_FORMAT_Y216                                            : return RESOURCE_FORMAT_Y16U16Y16V16_422_UNORM;
    case DXGI_FORMAT_NV12                                            : return RESOURCE_FORMAT_Y8_U8V8_2PLANE_420_UNORM;
//  case DXGI_FORMAT_Y8_U8V8_2PLANE_420_UNORM_PLANE0                 : return RESOURCE_FORMAT_Y8_U8V8_2PLANE_420_UNORM_PLANE0;
//  case DXGI_FORMAT_Y8_U8V8_2PLANE_420_UNORM_PLANE1                 : return RESOURCE_FORMAT_Y8_U8V8_2PLANE_420_UNORM_PLANE1;
    case DXGI_FORMAT_P010                                            : return RESOURCE_FORMAT_X6Y10_X6U10X6V10_2PLANE_420_UNORM;
//  case DXGI_FORMAT_X6Y10_X6U10X6V10_2PLANE_420_UNORM_PLANE0        : return RESOURCE_FORMAT_X6Y10_X6U10X6V10_2PLANE_420_UNORM_PLANE0;
//  case DXGI_FORMAT_X6Y10_X6U10X6V10_2PLANE_420_UNORM_PLANE1        : return RESOURCE_FORMAT_X6Y10_X6U10X6V10_2PLANE_420_UNORM_PLANE1;
    case DXGI_FORMAT_P016                                            : return RESOURCE_FORMAT_Y16_U16V16_2PLANE_420_UNORM;
//  case DXGI_FORMAT_Y16_U16V16_2PLANE_420_UNORM_PLANE0              : return RESOURCE_FORMAT_Y16_U16V16_2PLANE_420_UNORM_PLANE0;
//  case DXGI_FORMAT_Y16_U16V16_2PLANE_420_UNORM_PLANE1              : return RESOURCE_FORMAT_Y16_U16V16_2PLANE_420_UNORM_PLANE1;

    // 圧縮フォーマット_VK
    //case VK_FORMAT_BC1_RGB_UNORM_BLOCK                             : return RESOURCE_FORMAT_BC1_RGB_UNORM_BLOCK_VK;
    //case VK_FORMAT_BC1_RGB_SRGB_BLOCK                              : return RESOURCE_FORMAT_BC1_RGB_SRGB_BLOCK_VK;

    //case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK                         : return RESOURCE_FORMAT_ETC2_R8G8B8_UNORM_BLOCK_VK;
    //case VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK                          : return RESOURCE_FORMAT_ETC2_R8G8B8_SRGB_BLOCK_VK;
    //case VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK                       : return RESOURCE_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK_VK;
    //case VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK                        : return RESOURCE_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK_VK;
    //case VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK                       : return RESOURCE_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK_VK;
    //case VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK                        : return RESOURCE_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK_VK;

    //case VK_FORMAT_EAC_R11_UNORM_BLOCK                             : return RESOURCE_FORMAT_EAC_R11_UNORM_BLOCK_VK;
    //case VK_FORMAT_EAC_R11_SNORM_BLOCK                             : return RESOURCE_FORMAT_EAC_R11_SNORM_BLOCK_VK;
    //case VK_FORMAT_EAC_R11G11_UNORM_BLOCK                          : return RESOURCE_FORMAT_EAC_R11G11_UNORM_BLOCK_VK;
    //case VK_FORMAT_EAC_R11G11_SNORM_BLOCK                          : return RESOURCE_FORMAT_EAC_R11G11_SNORM_BLOCK_VK;

    //case VK_FORMAT_ASTC_4x4_UNORM_BLOCK                            : return RESOURCE_FORMAT_ASTC_4x4_UNORM_BLOCK_VK;
    //case VK_FORMAT_ASTC_4x4_SRGB_BLOCK                             : return RESOURCE_FORMAT_ASTC_4x4_SRGB_BLOCK_VK;
    //case VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK_EXT                       : return RESOURCE_FORMAT_ASTC_4x4_SFLOAT_BLOCK_EXT_VK;
    //case VK_FORMAT_ASTC_5x4_UNORM_BLOCK                            : return RESOURCE_FORMAT_ASTC_5x4_UNORM_BLOCK_VK;
    //case VK_FORMAT_ASTC_5x4_SRGB_BLOCK                             : return RESOURCE_FORMAT_ASTC_5x4_SRGB_BLOCK_VK;
    //case VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK_EXT                       : return RESOURCE_FORMAT_ASTC_5x4_SFLOAT_BLOCK_EXT_VK;
    //case VK_FORMAT_ASTC_5x5_UNORM_BLOCK                            : return RESOURCE_FORMAT_ASTC_5x5_UNORM_BLOCK_VK;
    //case VK_FORMAT_ASTC_5x5_SRGB_BLOCK                             : return RESOURCE_FORMAT_ASTC_5x5_SRGB_BLOCK_VK;
    //case VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK_EXT                       : return RESOURCE_FORMAT_ASTC_5x5_SFLOAT_BLOCK_EXT_VK;
    //case VK_FORMAT_ASTC_6x5_UNORM_BLOCK                            : return RESOURCE_FORMAT_ASTC_6x5_UNORM_BLOCK_VK;
    //case VK_FORMAT_ASTC_6x5_SRGB_BLOCK                             : return RESOURCE_FORMAT_ASTC_6x5_SRGB_BLOCK_VK;
    //case VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK_EXT                       : return RESOURCE_FORMAT_ASTC_6x5_SFLOAT_BLOCK_EXT_VK;
    //case VK_FORMAT_ASTC_6x6_UNORM_BLOCK                            : return RESOURCE_FORMAT_ASTC_6x6_UNORM_BLOCK_VK;
    //case VK_FORMAT_ASTC_6x6_SRGB_BLOCK                             : return RESOURCE_FORMAT_ASTC_6x6_SRGB_BLOCK_VK;
    //case VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK_EXT                       : return RESOURCE_FORMAT_ASTC_6x6_SFLOAT_BLOCK_EXT_VK;
    //case VK_FORMAT_ASTC_8x5_UNORM_BLOCK                            : return RESOURCE_FORMAT_ASTC_8x5_UNORM_BLOCK_VK;
    //case VK_FORMAT_ASTC_8x5_SRGB_BLOCK                             : return RESOURCE_FORMAT_ASTC_8x5_SRGB_BLOCK_VK;
    //case VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK_EXT                       : return RESOURCE_FORMAT_ASTC_8x5_SFLOAT_BLOCK_EXT_VK;
    //case VK_FORMAT_ASTC_8x6_UNORM_BLOCK                            : return RESOURCE_FORMAT_ASTC_8x6_UNORM_BLOCK_VK;
    //case VK_FORMAT_ASTC_8x6_SRGB_BLOCK                             : return RESOURCE_FORMAT_ASTC_8x6_SRGB_BLOCK_VK;
    //case VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK_EXT                       : return RESOURCE_FORMAT_ASTC_8x6_SFLOAT_BLOCK_EXT_VK;
    //case VK_FORMAT_ASTC_8x8_UNORM_BLOCK                            : return RESOURCE_FORMAT_ASTC_8x8_UNORM_BLOCK_VK;
    //case VK_FORMAT_ASTC_8x8_SRGB_BLOCK                             : return RESOURCE_FORMAT_ASTC_8x8_SRGB_BLOCK_VK;
    //case VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK_EXT                       : return RESOURCE_FORMAT_ASTC_8x8_SFLOAT_BLOCK_EXT_VK;
    //case VK_FORMAT_ASTC_10x5_UNORM_BLOCK                           : return RESOURCE_FORMAT_ASTC_10x5_UNORM_BLOCK_VK;
    //case VK_FORMAT_ASTC_10x5_SRGB_BLOCK                            : return RESOURCE_FORMAT_ASTC_10x5_SRGB_BLOCK_VK;
    //case VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK_EXT                      : return RESOURCE_FORMAT_ASTC_10x5_SFLOAT_BLOCK_EXT_VK;
    //case VK_FORMAT_ASTC_10x6_UNORM_BLOCK                           : return RESOURCE_FORMAT_ASTC_10x6_UNORM_BLOCK_VK;
    //case VK_FORMAT_ASTC_10x6_SRGB_BLOCK                            : return RESOURCE_FORMAT_ASTC_10x6_SRGB_BLOCK_VK;
    //case VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK_EXT                      : return RESOURCE_FORMAT_ASTC_10x6_SFLOAT_BLOCK_EXT_VK;
    //case VK_FORMAT_ASTC_10x8_UNORM_BLOCK                           : return RESOURCE_FORMAT_ASTC_10x8_UNORM_BLOCK_VK;
    //case VK_FORMAT_ASTC_10x8_SRGB_BLOCK                            : return RESOURCE_FORMAT_ASTC_10x8_SRGB_BLOCK_VK;
    //case VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK_EXT                      : return RESOURCE_FORMAT_ASTC_10x8_SFLOAT_BLOCK_EXT_VK;
    //case VK_FORMAT_ASTC_10x10_UNORM_BLOCK                          : return RESOURCE_FORMAT_ASTC_10x10_UNORM_BLOCK_VK;
    //case VK_FORMAT_ASTC_10x10_SRGB_BLOCK                           : return RESOURCE_FORMAT_ASTC_10x10_SRGB_BLOCK_VK;
    //case VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK_EXT                     : return RESOURCE_FORMAT_ASTC_10x10_SFLOAT_BLOCK_EXT_VK;
    //case VK_FORMAT_ASTC_12x10_UNORM_BLOCK                          : return RESOURCE_FORMAT_ASTC_12x10_UNORM_BLOCK_VK;
    //case VK_FORMAT_ASTC_12x10_SRGB_BLOCK                           : return RESOURCE_FORMAT_ASTC_12x10_SRGB_BLOCK_VK;
    //case VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK_EXT                     : return RESOURCE_FORMAT_ASTC_12x10_SFLOAT_BLOCK_EXT_VK;
    //case VK_FORMAT_ASTC_12x12_UNORM_BLOCK                          : return RESOURCE_FORMAT_ASTC_12x12_UNORM_BLOCK_VK;
    //case VK_FORMAT_ASTC_12x12_SRGB_BLOCK                           : return RESOURCE_FORMAT_ASTC_12x12_SRGB_BLOCK_VK;
    //case VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK_EXT                     : return RESOURCE_FORMAT_ASTC_12x12_SFLOAT_BLOCK_EXT_VK;

    //case VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG                     : return RESOURCE_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG_VK;
    //case VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG                     : return RESOURCE_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG_VK;
    //case VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG                     : return RESOURCE_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG_VK;
    //case VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG                     : return RESOURCE_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG_VK;
    //case VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG                      : return RESOURCE_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG_VK;
    //case VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG                      : return RESOURCE_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG_VK;
    //case VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG                      : return RESOURCE_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG_VK;
    //case VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG                      : return RESOURCE_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG_VK;

    default:
        return RESOURCE_FORMAT_UNKNOWN;
    }
}


FormatPropertiesD3D12::FormatPropertiesD3D12()
{
}

FormatPropertiesD3D12::~FormatPropertiesD3D12() 
{
}

void FormatPropertiesD3D12::Init(DeviceD3D12* _device)
{
    for (int i = FORMAT_OFFSET; i < FORMAT_MAX; i++)
        PrepareAndEvaluateFormatProperties(SCAST<DXGI_FORMAT>(i), _device);
}

void FormatPropertiesD3D12::PrepareAndEvaluateFormatProperties(DXGI_FORMAT _format, DeviceD3D12* _device)
{
    auto&& props = formats_props[_format];
    CreateProperties(props);
    GetFormatProperties(_format, props.get(), _device);
    if (!HasSupportedAnyFeature(props.get()))
        props.reset();

    props->b3d_format = GetB3DFormat(_format);
}

void FormatPropertiesD3D12::CreateProperties(util::SharedPtr<FORMAT_PROPERTIES_D3D12>& _props)
{
    _props = B3DMakeShared(FORMAT_PROPERTIES_D3D12);
}

void FormatPropertiesD3D12::GetFormatProperties(DXGI_FORMAT _format, FORMAT_PROPERTIES_D3D12* _dst_format_props, DeviceD3D12* _device)
{
    _dst_format_props->format_props = &_device->GetDeviceAdapter()->GetFeatureData().formats_data[_format];
}

bool FormatPropertiesD3D12::HasSupportedAnyFeature(FORMAT_PROPERTIES_D3D12* _props)
{
    return _props->format_props->support.Support1 != D3D12_FORMAT_SUPPORT1_NONE || 
           _props->format_props->support.Support2 != D3D12_FORMAT_SUPPORT2_NONE;
}


void FormatCompatibilityChecker::Init(const FormatPropertiesD3D12& _format_properties_d3d12)
{
    auto&& props = _format_properties_d3d12.GetFormatsProperties();

    /* OPTIMIZE: FormatCompatibilityChecker */

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

void FormatCompatibilityChecker::PrepareCompatibleFormatsList(
    RESOURCE_FORMAT                                                               _typeless_format
    , const std::initializer_list<RESOURCE_FORMAT>&                               _all_compatible_formats
    , const util::UnordMap<DXGI_FORMAT,util::SharedPtr<FORMAT_PROPERTIES_D3D12>>& _props
)
{
    auto&& compatible_foramt = compatible_formats[_typeless_format]      = B3DMakeShared(TYPELESS_COMPATIBLE_FORMATS);
    auto&& cf                = compatible_foramt->compatible_formats     = B3DMakeShared(util::DyArray<RESOURCE_FORMAT>);
    auto&& cvdxgi            = compatible_foramt->compatible_dxgiformats = B3DMakeShared(util::DyArray<DXGI_FORMAT>);

    cf->reserve(_all_compatible_formats.size());
    cvdxgi->reserve(_all_compatible_formats.size());
    for (auto& b3df : _all_compatible_formats)
    {
        auto dxgif = GetNativeFormat(b3df);
        if (dxgif != DXGI_FORMAT_UNKNOWN && _props.at(dxgif) != nullptr)
        {
            cf->push_back(b3df);
            cvdxgi->push_back(dxgif);
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
        _compatible_formats->compatible_dxgiformats->shrink_to_fit();
    }
}

}// namespace util
}// namespace buma3d
