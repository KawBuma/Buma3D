#pragma once

#pragma warning(push)
#pragma warning(disable:26812)

// プラットフォーム
#ifndef B3D_PLATFORM_USING

#define B3D_PLATFORM_WINDOWS (1)
#define B3D_PLATFORM_ANDROID (2)

#ifdef _WIN64
#define B3D_PLATFORM_USING (B3D_PLATFORM_WINDOWS)
#endif // WIN64

#ifdef __ANDROID__
#define B3D_PLATFORM_USING (B3D_PLATFORM_ANDROID)
#endif // __ANDROID__


// プラットフォーム識別定数
#if B3D_PLATFORM_USING == B3D_PLATFORM_WINDOWS
#define B3D_PLATFORM_IS_USE_WINDOWS (1)
#define B3D_PLATFORM_IS_USE_ANDROID (0)
#endif // B3D_PLATFORM_USING == B3D_PLATFORM_WINDOWS

#if B3D_PLATFORM_USING == B3D_PLATFORM_ANDROID
#define B3D_PLATFORM_IS_USE_WINDOWS (0)
#define B3D_PLATFORM_IS_USE_ANDROID (1)
#endif // B3D_PLATFORM_USING == B3D_PLATFORM_ANDROID


// APIシンボル宣言の属性を定義
#if defined(B3D_DLLEXPORT)
#define B3D_DLL_API extern "C" __declspec(dllexport)
#define B3D_API 

#elif defined(B3D_DLLIMPORT)
#define B3D_DLL_API extern "C" __declspec(dllimport)
#define B3D_API 

#else // static link library
#define B3D_DLL_API 
#define B3D_API 

#endif

#if B3D_PLATFORM_USING == B3D_PLATFORM_WINDOWS

/* __declspec(novtable)
この形式の__declspecは、任意のクラス宣言に適用できます。
ただし、純粋なインターフェースクラス、つまり、それ自体ではインスタンス化されないクラスにのみ適用する必要があります。
__declspecは、コンパイラーがコードを生成して、クラスのコンストラクターおよびデストラクターでvfptrを初期化するのを停止します。
多くの場合、これにより、クラスに関連付けられているvtableへの参照のみが削除され、リンカによって削除されます。
この形式の__declspecを使用すると、コードサイズを大幅に削減できます。
novtableでマークされたクラスをインスタンス化してからクラスメンバーにアクセスしようとすると、アクセス違反が発生します。*/
#define B3D_INTERFACE struct B3D_API __declspec(novtable)

#else
#define B3D_INTERFACE struct B3D_API 
#endif

// 呼び出し規約
#if B3D_PLATFORM_IS_USE_WINDOWS
#define B3D_APIENTRY __stdcall
#else
#define B3D_APIENTRY
#endif // B3D_PLATFORM_IS_USE_WINDOWS


// 1度も参照されない場合を防ぐ
#define B3D_UNREFERENCED(...) (__VA_ARGS__)

#endif // !B3D_PLATFORM_USING

namespace buma3d
{

enum class PLATFORM_TYPE : uint32_t
{
      WINDOWS = 1
    , ANDROID = 2
};

inline constexpr PLATFORM_TYPE PLATFORM_USING = static_cast<PLATFORM_TYPE>(B3D_PLATFORM_USING);

inline constexpr bool PLATFORM_IS_USE_WINDOWS = PLATFORM_USING == PLATFORM_TYPE::WINDOWS;
inline constexpr bool PLATFORM_IS_USE_ANDROID = PLATFORM_USING == PLATFORM_TYPE::ANDROID;

}// namespace buma3d


// ヘッダのバージョン
namespace buma3d
{

inline constexpr uint32_t EncodeHeaderVersion(uint32_t _major, uint32_t _minor, uint32_t _patch)
{
    return ((((uint32_t)(_major)) << 22) | (((uint32_t)(_minor)) << 12) | ((uint32_t)(_patch)));
}

inline constexpr uint32_t B3D_HEADER_VERSION = EncodeHeaderVersion(0, 1, 4);

inline constexpr void DecodeHeaderVersion(uint32_t* _major, uint32_t* _minor, uint32_t* _patch)
{
    *_major = (B3D_HEADER_VERSION >> 22);
    *_minor = (B3D_HEADER_VERSION >> 12) & ~(~0 << 10);
    *_patch =  B3D_HEADER_VERSION        & ~(~0 << 12);
}

}// namespace buma3d


// インクルード
#include "./Util/Buma3DUtils.h"
#include <type_traits>


// 前方宣言, ポインタusing
namespace buma3d
{

#define DECLARE_SHARED_PTR(T)                          \
//using T##Ptr     = util::SharedPtr<I##T>;            \
//using T##WPtr    = util::WeakPtr<I##T>;              \
//using T##CPtr    = util::SharedPtr<const I##T>;      \
//using T##CWPtr   = util::WeakPtr<const I##T>;        \
//using T##RawPtr  = I##T*;                            \
//using T##CRawPtr = const I##T*

struct ISharedBase;                 DECLARE_SHARED_PTR(SharedBase);
struct IBlob;                       DECLARE_SHARED_PTR(Blob);
struct INameableObject;             DECLARE_SHARED_PTR(NameableObject);

struct IShaderModule;               DECLARE_SHARED_PTR(ShaderModule);

struct IDeviceFactory;              DECLARE_SHARED_PTR(DeviceFactory);
struct IDebugMessage;               DECLARE_SHARED_PTR(DebugMessage);
struct IDebugMessageQueue;          DECLARE_SHARED_PTR(DebugMessageQueue);
struct IDeviceAdapter;              DECLARE_SHARED_PTR(DeviceAdapter);
struct ISurface;                    DECLARE_SHARED_PTR(Surface);
struct IDevice;                     DECLARE_SHARED_PTR(Device);
struct IDeviceChild;                DECLARE_SHARED_PTR(DeviceChild);
struct ISwapChain;                  DECLARE_SHARED_PTR(SwapChain);

struct IResourceHeap;               DECLARE_SHARED_PTR(ResourceHeap);
struct IResource;                   DECLARE_SHARED_PTR(Resource);
struct IBuffer;                     DECLARE_SHARED_PTR(Buffer);
struct ITexture;                    DECLARE_SHARED_PTR(Texture);

struct IView;                       DECLARE_SHARED_PTR(View);
struct IVertexBufferView;           DECLARE_SHARED_PTR(VertexBufferView);
struct IIndexBufferView;            DECLARE_SHARED_PTR(IndexBufferView);
struct IConstantBufferView;         DECLARE_SHARED_PTR(ConstantBufferView);
struct IRenderTargetView;           DECLARE_SHARED_PTR(RenderTargetView);
struct IDepthStencilView;           DECLARE_SHARED_PTR(DepthStencilView);
struct IShaderResourceView;         DECLARE_SHARED_PTR(ShaderResourceView);
struct IUnorderedAccessView;        DECLARE_SHARED_PTR(UnorderedAccessView);
struct ISamplerView;                DECLARE_SHARED_PTR(SamplerView);
struct IStreamOutputBufferView;     DECLARE_SHARED_PTR(StreamOutputBufferView);

struct IFramebuffer;                DECLARE_SHARED_PTR(Framebuffer);
struct IRenderPass;                 DECLARE_SHARED_PTR(RenderPass);
struct IDescriptorPool;             DECLARE_SHARED_PTR(DescriptorPool);
struct IDescriptorSet;              DECLARE_SHARED_PTR(DescriptorSet);
struct IRootSignature;              DECLARE_SHARED_PTR(RootSignature);
struct IPipelineState;              DECLARE_SHARED_PTR(PipelineState);

struct ICommandQueue;               DECLARE_SHARED_PTR(CommandQueue);
struct ICommandAllocator;           DECLARE_SHARED_PTR(CommandAllocator);
struct ICommandList;                DECLARE_SHARED_PTR(CommandList);
struct IFence;                      DECLARE_SHARED_PTR(Fence);

struct IQueryHeap;                  DECLARE_SHARED_PTR(QueryHeap);
struct ICommandSignature;           DECLARE_SHARED_PTR(CommandSignature);

struct IAccelerationStructure;      DECLARE_SHARED_PTR(AccelerationStructure);

#undef DECLARE_SHARED_PTR

}// namespace buma3d

namespace buma3d
{

template<typename T>
struct TVECTOR2
{
    T x, y;
};

template<typename T>
struct TVECTOR3
{
    T x, y, z;
};

template<typename T>
struct TVECTOR4
{
    T x, y, z, w;
};

#define DEFINE_VECTORS(Name, T, _Name)     \
    using Name##2##_Name = TVECTOR2<T>;    \
    using Name##3##_Name = TVECTOR3<T>;    \
    using Name##4##_Name = TVECTOR4<T> 

DEFINE_VECTORS(INT  , int32_t    , );
DEFINE_VECTORS(UINT , uint32_t   , );
DEFINE_VECTORS(INT  , int64_t    , _64);
DEFINE_VECTORS(UINT , uint64_t   , _64);
DEFINE_VECTORS(FLOAT, float      , );

#undef DEFINE_VECTORS

struct OFFSET2D
{
    int32_t x;
    int32_t y;
};

struct EXTENT2D
{
    uint32_t width;
    uint32_t height;
};

struct OFFSET3D
{
    int32_t x;
    int32_t y;
    int32_t z;
};

struct EXTENT3D
{
    uint32_t width;
    uint32_t height;
    uint32_t depth;
};

struct COLOR3
{
    float r;
    float g;
    float b;
};

struct COLOR4
{
    float r;
    float g;
    float b;
    float a;
};

#if _HAS_CXX20
using Char8T = char8_t;
#else
/**
 * @brief Buma3Dでは、Unicode UTF-8文字列の使用を予定しています。
 *        現状実装ではstring、char*を使用しています。加えて、通常のchar文字リテラルを使用しており、現在はC++20ないし英数字以外のutf-8への対応が出来ていません。
 *        char*が使用されている場合、英数字を使用してください。
*/
using Char8T = char;
#endif 

using EnumT                 = uint32_t;
using EnumFlagsT            = EnumT;
using Enum64T               = uint64_t;
using EnumFlags64T          = Enum64T;
using Enum8T                = uint8_t;
using EnumFlags8T           = Enum8T;

using GpuVirtualAddress     = uint64_t;
using NodeMask              = uint32_t;

using SampleMask            = uint32_t;

inline constexpr NodeMask B3D_DEFAULT_NODE_MASK = 0x1;
inline constexpr uint32_t B3D_MAX_NODE_COUNT    = 32;

inline constexpr SampleMask B3D_DEFAULT_SAMPLE_MASK[]           = { ~0u, ~0u };
inline constexpr uint32_t   B3D_DEFAULT_STENCIL_COMPARE_MASK    = ~0u;
inline constexpr uint32_t   B3D_DEFAULT_STENCIL_WRITE_MASK      = ~0u;
inline constexpr uint32_t   B3D_DEFAULT_STENCIL_REFERENCE       = ~0u;

inline constexpr uint32_t B3D_USE_ALL_MIPS              = ~0u; // RESOURCE_DESC::num_mipsに指定された場合、利用可能な全てのミップレベルが割り当てられます。
inline constexpr uint32_t B3D_USE_REMAINING_MIP_LEVELS  = ~0u; // SUBRESOURCE_RANGE::mip_levelsに指定された場合、offsetから利用可能な全てのミップレベルをビューに含めます。
inline constexpr uint32_t B3D_USE_REMAINING_ARRAY_SIZES = ~0u; // SUBRESOURCE_RANGE::array_sizeに指定された場合、offsetから利用可能な全ての配列をビューに含めます。
inline constexpr uint32_t B3D_APPEND_ALIGNED_ELEMENT    = ~0u; // INPUT_ELEMENT_DESC::aligned_byte_offsetに指定された場合、オフセットが自動的に計算されます。
//inline constexpr uint64_t B3D_USE_WHOLE_SIZE          = ~0ull;
inline constexpr uint32_t B3D_UNUSED_ATTACHMENT         = ~0u;
inline constexpr uint32_t B3D_SUBPASS_EXTERNAL          = ~0u;
//inline constexpr uint32_t B3D_QUEUE_FAMILY_IGNORED    = ~0u;

inline constexpr float B3D_VIEWPORT_MIN_DEPTH = 0.f;
inline constexpr float B3D_VIEWPORT_MAX_DEPTH = 1.f;


enum BMRESULT : EnumT
{
      BMRESULT_SUCCEED
    , BMRESULT_SUCCEED_NOT_READY
    , BMRESULT_SUCCEED_TIMEOUT

    , BMRESULT_FAILED
    , BMRESULT_FAILED_OUT_OF_RANGE
    , BMRESULT_FAILED_DEVICE_REMOVED
    , BMRESULT_FAILED_INVALID_PARAMETER
    , BMRESULT_FAILED_NOT_SUPPORTED
    , BMRESULT_FAILED_NOT_SUPPORTED_FEATURE
    , BMRESULT_FAILED_NOT_SUPPORTED_BY_CURRENT_INTERNAL_API
    , BMRESULT_FAILED_OUT_OF_SYSTEM_MEMORY
    , BMRESULT_FAILED_OUT_OF_DEVICE_MEMORY
    , BMRESULT_FAILED_OUT_OF_POOL_MEMORY
    , BMRESULT_FAILED_FRAGMENTED_POOL
    , BMRESULT_FAILED_NOT_IMPLEMENTED
    , BMRESULT_FAILED_INVALID_CALL
    , BMRESULT_FAILED_TOO_MANY_OBJECTS
    , BMRESULT_FAILED_RESOURCE_SIZE_EXCEEDED
    , BMRESULT_FAILED_USAGE_INCOMPATIBLE
};

struct SCISSOR_RECT
{
    OFFSET2D offset;
    EXTENT2D extent;
};

struct VIEWPORT
{
    float x;         // ビューポート左上座標の値を指定します。
    float y;         // ビューポート左上座標の値を指定します。
    float width;     // xからの幅です。
    float height;    // yからの高さです。
    float min_depth; // ビューポートでの深度範囲の最小値です。 値は[0.f, 1.f]の範囲である必要があります(この値は正規化されており、整数型のフォーマットに対しても有効です)。 この値以下の深度値は、この値にクランプされます。
    float max_depth; // ビューポートでの深度範囲の最大値です。 値は[0.f, 1.f]の範囲である必要があります(この値は正規化されており、整数型のフォーマットに対しても有効です)。 この値以上の深度値は、この値にクランプされます。
};

struct MAPPED_RANGE
{
    uint64_t offset;
    uint64_t size;
};


/*
伝達関数
https://vook.vc/n/469
https://4k8ktv.jp/2016/03/19/itu-r-hdr-tv/
色空間について
https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkColorSpaceKHR.html
https://www.siliconstudio.co.jp/rd/presentations/files/CEDEC_KYUSHU2017/cedec_kyushu_2017_hdr_kawase.pdf
https://www.khronos.org/registry/DataFormat/specs/1.3/dataformat.1.3.html#TRANSFER_CONVERSION
*/

enum COLOR_SPACE : EnumT
{
      COLOR_SPACE_SRGB_NONLINEAR    // DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709
    , COLOR_SPACE_BT709_LINEAR      // DXGI_COLOR_SPACE_RGB_FULL_G10_NONE_P709
    , COLOR_SPACE_BT709_NONLINEAR   // DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709
    , COLOR_SPACE_HDR10_ST2084_PQ   // DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020

    //COLOR_SPACE_EXTENDED_SRGB_LINEAR
    //COLOR_SPACE_DISPLAY_P3_NONLINEAR
    //COLOR_SPACE_DISPLAY_P3_LINEAR
    //COLOR_SPACE_DCI_P3_NONLINEAR
    //COLOR_SPACE_BT2020_LINEAR
    //COLOR_SPACE_DOLBYVISION
    //COLOR_SPACE_HDR10_HLG
    //COLOR_SPACE_ADOBERGB_LINEAR
    //COLOR_SPACE_ADOBERGB_NONLINEAR
    , COLOR_SPACE_CUSTOM
};

enum RESOURCE_FORMAT : EnumT
{
    RESOURCE_FORMAT_UNKNOWN                                         // VK_FORMAT_UNDEFINED

    // 共通フォーマット(MUTABLE_FORMAT)
    // _TYPELESSフォーマットを指定すると、同じコンポーネント数、同じサイズを持つ複数のフォーマットを指定可能です。

    // R
    , RESOURCE_FORMAT_R8_TYPELESS                                   // VK_FORMAT_R8_* (MUTABLE_FORMAT)
    , RESOURCE_FORMAT_R8_UNORM                                      // VK_FORMAT_R8_UNORM
    , RESOURCE_FORMAT_R8_SNORM                                      // VK_FORMAT_R8_SNORM
    , RESOURCE_FORMAT_R8_UINT                                       // VK_FORMAT_R8_UINT
    , RESOURCE_FORMAT_R8_SINT                                       // VK_FORMAT_R8_SINT
    , RESOURCE_FORMAT_R16_TYPELESS                                  // VK_FORMAT_R16_* (MUTABLE_FORMAT)
    , RESOURCE_FORMAT_R16_UNORM                                     // VK_FORMAT_R16_UNORM
    , RESOURCE_FORMAT_R16_SNORM                                     // VK_FORMAT_R16_SNORM
    , RESOURCE_FORMAT_R16_UINT                                      // VK_FORMAT_R16_UINT
    , RESOURCE_FORMAT_R16_SINT                                      // VK_FORMAT_R16_SINT
    , RESOURCE_FORMAT_R16_FLOAT                                     // VK_FORMAT_R16_SFLOAT
    , RESOURCE_FORMAT_R32_TYPELESS                                  // VK_FORMAT_R32_* (MUTABLE_FORMAT)
    , RESOURCE_FORMAT_R32_UINT                                      // VK_FORMAT_R32_UINT
    , RESOURCE_FORMAT_R32_SINT                                      // VK_FORMAT_R32_SINT
    , RESOURCE_FORMAT_R32_FLOAT                                     // VK_FORMAT_R32_SFLOAT

    // RG
    , RESOURCE_FORMAT_R8G8_TYPELESS                                 // VK_FORMAT_R8G8_* (MUTABLE_FORMAT) 
    , RESOURCE_FORMAT_R8G8_UNORM                                    // VK_FORMAT_R8G8_UNORM
    , RESOURCE_FORMAT_R8G8_SNORM                                    // VK_FORMAT_R8G8_SNORM
    , RESOURCE_FORMAT_R8G8_UINT                                     // VK_FORMAT_R8G8_UINT
    , RESOURCE_FORMAT_R8G8_SINT                                     // VK_FORMAT_R8G8_SINT
    , RESOURCE_FORMAT_R16G16_TYPELESS                               // VK_FORMAT_R16G16_* (MUTABLE_FORMAT)
    , RESOURCE_FORMAT_R16G16_UNORM                                  // VK_FORMAT_R16G16_UNORM
    , RESOURCE_FORMAT_R16G16_SNORM                                  // VK_FORMAT_R16G16_SNORM
    , RESOURCE_FORMAT_R16G16_UINT                                   // VK_FORMAT_R16G16_UINT
    , RESOURCE_FORMAT_R16G16_SINT                                   // VK_FORMAT_R16G16_SINT
    , RESOURCE_FORMAT_R16G16_FLOAT                                  // VK_FORMAT_R16G16_SFLOAT
    , RESOURCE_FORMAT_R32G32_TYPELESS                               // VK_FORMAT_R32G32_* (MUTABLE_FORMAT)
    , RESOURCE_FORMAT_R32G32_UINT                                   // VK_FORMAT_R32G32_UINT
    , RESOURCE_FORMAT_R32G32_SINT                                   // VK_FORMAT_R32G32_SINT
    , RESOURCE_FORMAT_R32G32_FLOAT                                  // VK_FORMAT_R32G32_SFLOAT
    // GR

    // RGB
    , RESOURCE_FORMAT_R11G11B10_UFLOAT                              // VK_FORMAT_B10G11R11_UFLOAT_PACK32
    , RESOURCE_FORMAT_R32G32B32_TYPELESS                            // VK_FORMAT_R32G32B32_* (MUTABLE_FORMAT)
    , RESOURCE_FORMAT_R32G32B32_UINT                                // VK_FORMAT_R32G32B32_UINT
    , RESOURCE_FORMAT_R32G32B32_SINT                                // VK_FORMAT_R32G32B32_SINT
    , RESOURCE_FORMAT_R32G32B32_FLOAT                               // VK_FORMAT_R32G32B32_SFLOAT

    // BGR
    , RESOURCE_FORMAT_B5G6R5_UNORM                                  // VK_FORMAT_R5G6B5_UNORM_PACK16

    // RGBA
    , RESOURCE_FORMAT_R8G8B8A8_TYPELESS                             // VK_FORMAT_R8G8B8A8_* (MUTABLE_FORMAT)
    , RESOURCE_FORMAT_R8G8B8A8_UNORM                                // VK_FORMAT_R8G8B8A8_UNORM
    , RESOURCE_FORMAT_R8G8B8A8_UNORM_SRGB                           // VK_FORMAT_R8G8B8A8_SRGB
    , RESOURCE_FORMAT_R8G8B8A8_SNORM                                // VK_FORMAT_R8G8B8A8_SNORM
    , RESOURCE_FORMAT_R8G8B8A8_UINT                                 // VK_FORMAT_R8G8B8A8_UINT
    , RESOURCE_FORMAT_R8G8B8A8_SINT                                 // VK_FORMAT_R8G8B8A8_SINT
    , RESOURCE_FORMAT_R10G10B10A2_TYPELESS                          // VK_FORMAT_A2B10G10R10_*_PACK32 (MUTABLE_FORMAT)
    , RESOURCE_FORMAT_R10G10B10A2_UNORM                             // VK_FORMAT_A2B10G10R10_UNORM_PACK32
    , RESOURCE_FORMAT_R10G10B10A2_UINT                              // VK_FORMAT_A2B10G10R10_UINT_PACK32
    , RESOURCE_FORMAT_R16G16B16A16_TYPELESS                         // VK_FORMAT_R16G16B16A16_* (MUTABLE_FORMAT)
    , RESOURCE_FORMAT_R16G16B16A16_UNORM                            // VK_FORMAT_R16G16B16A16_UNORM
    , RESOURCE_FORMAT_R16G16B16A16_SNORM                            // VK_FORMAT_R16G16B16A16_SNORM
    , RESOURCE_FORMAT_R16G16B16A16_UINT                             // VK_FORMAT_R16G16B16A16_UINT
    , RESOURCE_FORMAT_R16G16B16A16_SINT                             // VK_FORMAT_R16G16B16A16_SINT
    , RESOURCE_FORMAT_R16G16B16A16_FLOAT                            // VK_FORMAT_R16G16B16A16_SFLOAT
    , RESOURCE_FORMAT_R32G32B32A32_TYPELESS                         // VK_FORMAT_R32G32B32A32_* (MUTABLE_FORMAT)
    , RESOURCE_FORMAT_R32G32B32A32_UINT                             // VK_FORMAT_R32G32B32A32_UINT
    , RESOURCE_FORMAT_R32G32B32A32_SINT                             // VK_FORMAT_R32G32B32A32_SINT
    , RESOURCE_FORMAT_R32G32B32A32_FLOAT                            // VK_FORMAT_R32G32B32A32_SFLOAT
    // BGRA
    , RESOURCE_FORMAT_B5G5R5A1_UNORM                                // VK_FORMAT_A1R5G5B5_UNORM_PACK16
    , RESOURCE_FORMAT_B8G8R8A8_TYPELESS                             // VK_FORMAT_B8G8R8A8_* (MUTABLE_FORMAT)
    , RESOURCE_FORMAT_B8G8R8A8_UNORM                                // VK_FORMAT_B8G8R8A8_UNORM
    , RESOURCE_FORMAT_B8G8R8A8_UNORM_SRGB                           // VK_FORMAT_B8G8R8A8_SRGB
    // ARGB
    // ABGR

    // RGBE
    , RESOURCE_FORMAT_R9G9B9E5_UFLOAT                               // VK_FORMAT_E5B9G9R9_UFLOAT_PACK32

    // 深度ステンシルフォーマット                        
    , RESOURCE_FORMAT_D16_UNORM                                     // VK_FORMAT_D16_UNORM
    , RESOURCE_FORMAT_D32_FLOAT                                     // VK_FORMAT_D32_SFLOAT
    , RESOURCE_FORMAT_D24_UNORM_S8_UINT                             // VK_FORMAT_D24_UNORM_S8_UINT
    , RESOURCE_FORMAT_D32_FLOAT_S8X24_UINT                          // VK_FORMAT_D32_SFLOAT_S8_UINT

    // 圧縮フォーマット                                
    , RESOURCE_FORMAT_BC1_TYPELESS                                  // VK_FORMAT_BC1_RGBA_UNORM_* (MUTABLE_FORMAT)
    , RESOURCE_FORMAT_BC1_UNORM                                     // VK_FORMAT_BC1_RGBA_UNORM_BLOCK
    , RESOURCE_FORMAT_BC1_UNORM_SRGB                                // VK_FORMAT_BC1_RGBA_SRGB_BLOCK
    , RESOURCE_FORMAT_BC2_TYPELESS                                  // VK_FORMAT_BC2_UNORM_* (MUTABLE_FORMAT)
    , RESOURCE_FORMAT_BC2_UNORM                                     // VK_FORMAT_BC2_UNORM_BLOCK
    , RESOURCE_FORMAT_BC2_UNORM_SRGB                                // VK_FORMAT_BC2_SRGB_BLOCK
    , RESOURCE_FORMAT_BC3_TYPELESS                                  // VK_FORMAT_BC3_UNORM_* (MUTABLE_FORMAT)
    , RESOURCE_FORMAT_BC3_UNORM                                     // VK_FORMAT_BC3_UNORM_BLOCK
    , RESOURCE_FORMAT_BC3_UNORM_SRGB                                // VK_FORMAT_BC3_SRGB_BLOCK
    , RESOURCE_FORMAT_BC4_TYPELESS                                  // VK_FORMAT_BC4_UNORM_* (MUTABLE_FORMAT)
    , RESOURCE_FORMAT_BC4_UNORM                                     // VK_FORMAT_BC4_UNORM_BLOCK
    , RESOURCE_FORMAT_BC4_SNORM                                     // VK_FORMAT_BC4_SNORM_BLOCK
    , RESOURCE_FORMAT_BC5_TYPELESS                                  // VK_FORMAT_BC5_UNORM_* (MUTABLE_FORMAT)
    , RESOURCE_FORMAT_BC5_UNORM                                     // VK_FORMAT_BC5_UNORM_BLOCK
    , RESOURCE_FORMAT_BC5_SNORM                                     // VK_FORMAT_BC5_SNORM_BLOCK
    , RESOURCE_FORMAT_BC6H_TYPELESS                                 // VK_FORMAT_BC6H_UFLOAT_* (MUTABLE_FORMAT)
    , RESOURCE_FORMAT_BC6H_UF16                                     // VK_FORMAT_BC6H_UFLOAT_BLOCK
    , RESOURCE_FORMAT_BC6H_SF16                                     // VK_FORMAT_BC6H_SFLOAT_BLOCK
    , RESOURCE_FORMAT_BC7_TYPELESS                                  // VK_FORMAT_BC7_UNORM_* (MUTABLE_FORMAT)
    , RESOURCE_FORMAT_BC7_UNORM                                     // VK_FORMAT_BC7_UNORM_BLOCK
    , RESOURCE_FORMAT_BC7_UNORM_SRGB                                // VK_FORMAT_BC7_SRGB_BLOCK

    // ビデオフォーマット                    
    , RESOURCE_FORMAT_Y8U8Y8V8_422_UNORM                            // YUY2 VK_FORMAT_G8B8G8R8_422_UNORM
    , RESOURCE_FORMAT_X6Y10X6U10X6Y10X6V10_422_UNORM                // Y210 VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16
    , RESOURCE_FORMAT_Y16U16Y16V16_422_UNORM                        // Y216 VK_FORMAT_G16B16G16R16_422_UNORM
    , RESOURCE_FORMAT_Y8_U8V8_2PLANE_420_UNORM                      // NV12 VK_FORMAT_G8_B8R8_2PLANE_420_UNORM
    , RESOURCE_FORMAT_Y8_U8V8_2PLANE_420_UNORM_PLANE0               //      VK_FORMAT_R8_UNORM
    , RESOURCE_FORMAT_Y8_U8V8_2PLANE_420_UNORM_PLANE1               //      VK_FORMAT_R8G8_UNORM
    , RESOURCE_FORMAT_X6Y10_X6U10X6V10_2PLANE_420_UNORM             // P010 VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16
    , RESOURCE_FORMAT_X6Y10_X6U10X6V10_2PLANE_420_UNORM_PLANE0      //      VK_FORMAT_R10X6_UNORM_PACK16 (D3D:R16)
    , RESOURCE_FORMAT_X6Y10_X6U10X6V10_2PLANE_420_UNORM_PLANE1      //      VK_FORMAT_R10X6G10X6_UNORM_2PACK16 (D3D: R16G16)
    , RESOURCE_FORMAT_Y16_U16V16_2PLANE_420_UNORM                   // P016 VK_FORMAT_G16_B16R16_2PLANE_420_UNORM
    , RESOURCE_FORMAT_Y16_U16V16_2PLANE_420_UNORM_PLANE0            //      VK_FORMAT_R16_UNORM
    , RESOURCE_FORMAT_Y16_U16V16_2PLANE_420_UNORM_PLANE1            //      VK_FORMAT_R16G16_UNORM

    // 圧縮フォーマット_VK
    , RESOURCE_FORMAT_BC1_RGB_TYPELESS_BLOCK_VK                     // VK_FORMAT_BC1_RGB_*_BLOCK (MUTABLE_FORMAT)
    , RESOURCE_FORMAT_BC1_RGB_UNORM_BLOCK_VK                        // VK_FORMAT_BC1_RGB_UNORM_BLOCK
    , RESOURCE_FORMAT_BC1_RGB_SRGB_BLOCK_VK                         // VK_FORMAT_BC1_RGB_SRGB_BLOCK
    , RESOURCE_FORMAT_ETC2_R8G8B8_TYPELESS_BLOCK_VK                 // VK_FORMAT_ETC2_R8G8B8_*_BLOCK (MUTABLE_FORMAT)
    , RESOURCE_FORMAT_ETC2_R8G8B8_UNORM_BLOCK_VK                    // VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK
    , RESOURCE_FORMAT_ETC2_R8G8B8_SRGB_BLOCK_VK                     // VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK
    , RESOURCE_FORMAT_ETC2_R8G8B8A1_TYPELESS_BLOCK_VK               // VK_FORMAT_ETC2_R8G8B8A1_*_BLOCK (MUTABLE_FORMAT)
    , RESOURCE_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK_VK                  // VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK
    , RESOURCE_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK_VK                   // VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK
    , RESOURCE_FORMAT_ETC2_R8G8B8A8_TYPELESS_BLOCK_VK               // VK_FORMAT_ETC2_R8G8B8A8_*_BLOCK (MUTABLE_FORMAT)
    , RESOURCE_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK_VK                  // VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK
    , RESOURCE_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK_VK                   // VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK
    , RESOURCE_FORMAT_EAC_R11_TYPELESS_BLOCK_VK                     // VK_FORMAT_EAC_R11_*_BLOCK (MUTABLE_FORMAT)
    , RESOURCE_FORMAT_EAC_R11_UNORM_BLOCK_VK                        // VK_FORMAT_EAC_R11_UNORM_BLOCK
    , RESOURCE_FORMAT_EAC_R11_SNORM_BLOCK_VK                        // VK_FORMAT_EAC_R11_SNORM_BLOCK
    , RESOURCE_FORMAT_EAC_R11G11_TYPELESS_BLOCK_VK                  // VK_FORMAT_EAC_R11G11_*_BLOCK (MUTABLE_FORMAT)
    , RESOURCE_FORMAT_EAC_R11G11_UNORM_BLOCK_VK                     // VK_FORMAT_EAC_R11G11_UNORM_BLOCK
    , RESOURCE_FORMAT_EAC_R11G11_SNORM_BLOCK_VK                     // VK_FORMAT_EAC_R11G11_SNORM_BLOCK
    , RESOURCE_FORMAT_ASTC_4x4_TYPELESS_BLOCK_VK                    // VK_FORMAT_ASTC_4x4_*_BLOCK (MUTABLE_FORMAT)
    , RESOURCE_FORMAT_ASTC_4x4_UNORM_BLOCK_VK                       // VK_FORMAT_ASTC_4x4_UNORM_BLOCK
    , RESOURCE_FORMAT_ASTC_4x4_SRGB_BLOCK_VK                        // VK_FORMAT_ASTC_4x4_SRGB_BLOCK
    , RESOURCE_FORMAT_ASTC_4x4_SFLOAT_BLOCK_EXT_VK                  // VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK_EXT
    , RESOURCE_FORMAT_ASTC_5x4_TYPELESS_BLOCK_VK                    // VK_FORMAT_ASTC_5x4_*_BLOCK (MUTABLE_FORMAT)
    , RESOURCE_FORMAT_ASTC_5x4_UNORM_BLOCK_VK                       // VK_FORMAT_ASTC_5x4_UNORM_BLOCK
    , RESOURCE_FORMAT_ASTC_5x4_SRGB_BLOCK_VK                        // VK_FORMAT_ASTC_5x4_SRGB_BLOCK
    , RESOURCE_FORMAT_ASTC_5x4_SFLOAT_BLOCK_EXT_VK                  // VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK_EXT
    , RESOURCE_FORMAT_ASTC_5x5_TYPELESS_BLOCK_VK                    // VK_FORMAT_ASTC_5x5_*_BLOCK (MUTABLE_FORMAT)
    , RESOURCE_FORMAT_ASTC_5x5_UNORM_BLOCK_VK                       // VK_FORMAT_ASTC_5x5_UNORM_BLOCK
    , RESOURCE_FORMAT_ASTC_5x5_SRGB_BLOCK_VK                        // VK_FORMAT_ASTC_5x5_SRGB_BLOCK
    , RESOURCE_FORMAT_ASTC_5x5_SFLOAT_BLOCK_EXT_VK                  // VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK_EXT
    , RESOURCE_FORMAT_ASTC_6x5_TYPELESS_BLOCK_VK                    // VK_FORMAT_ASTC_6x5_*_BLOCK (MUTABLE_FORMAT)
    , RESOURCE_FORMAT_ASTC_6x5_UNORM_BLOCK_VK                       // VK_FORMAT_ASTC_6x5_UNORM_BLOCK
    , RESOURCE_FORMAT_ASTC_6x5_SRGB_BLOCK_VK                        // VK_FORMAT_ASTC_6x5_SRGB_BLOCK
    , RESOURCE_FORMAT_ASTC_6x5_SFLOAT_BLOCK_EXT_VK                  // VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK_EXT
    , RESOURCE_FORMAT_ASTC_6x6_TYPELESS_BLOCK_VK                    // VK_FORMAT_ASTC_6x6_*_BLOCK (MUTABLE_FORMAT)
    , RESOURCE_FORMAT_ASTC_6x6_UNORM_BLOCK_VK                       // VK_FORMAT_ASTC_6x6_UNORM_BLOCK
    , RESOURCE_FORMAT_ASTC_6x6_SRGB_BLOCK_VK                        // VK_FORMAT_ASTC_6x6_SRGB_BLOCK
    , RESOURCE_FORMAT_ASTC_6x6_SFLOAT_BLOCK_EXT_VK                  // VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK_EXT
    , RESOURCE_FORMAT_ASTC_8x5_TYPELESS_BLOCK_VK                    // VK_FORMAT_ASTC_8x5_*_BLOCK (MUTABLE_FORMAT)
    , RESOURCE_FORMAT_ASTC_8x5_UNORM_BLOCK_VK                       // VK_FORMAT_ASTC_8x5_UNORM_BLOCK
    , RESOURCE_FORMAT_ASTC_8x5_SRGB_BLOCK_VK                        // VK_FORMAT_ASTC_8x5_SRGB_BLOCK
    , RESOURCE_FORMAT_ASTC_8x5_SFLOAT_BLOCK_EXT_VK                  // VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK_EXT
    , RESOURCE_FORMAT_ASTC_8x6_TYPELESS_BLOCK_VK                    // VK_FORMAT_ASTC_8x6_*_BLOCK (MUTABLE_FORMAT)
    , RESOURCE_FORMAT_ASTC_8x6_UNORM_BLOCK_VK                       // VK_FORMAT_ASTC_8x6_UNORM_BLOCK
    , RESOURCE_FORMAT_ASTC_8x6_SRGB_BLOCK_VK                        // VK_FORMAT_ASTC_8x6_SRGB_BLOCK
    , RESOURCE_FORMAT_ASTC_8x6_SFLOAT_BLOCK_EXT_VK                  // VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK_EXT
    , RESOURCE_FORMAT_ASTC_8x8_TYPELESS_BLOCK_VK                    // VK_FORMAT_ASTC_8x8_*_BLOCK (MUTABLE_FORMAT)
    , RESOURCE_FORMAT_ASTC_8x8_UNORM_BLOCK_VK                       // VK_FORMAT_ASTC_8x8_UNORM_BLOCK
    , RESOURCE_FORMAT_ASTC_8x8_SRGB_BLOCK_VK                        // VK_FORMAT_ASTC_8x8_SRGB_BLOCK
    , RESOURCE_FORMAT_ASTC_8x8_SFLOAT_BLOCK_EXT_VK                  // VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK_EXT
    , RESOURCE_FORMAT_ASTC_10x5_TYPELESS_BLOCK_VK                   // VK_FORMAT_ASTC_10x5_*_BLOCK (MUTABLE_FORMAT)
    , RESOURCE_FORMAT_ASTC_10x5_UNORM_BLOCK_VK                      // VK_FORMAT_ASTC_10x5_UNORM_BLOCK
    , RESOURCE_FORMAT_ASTC_10x5_SRGB_BLOCK_VK                       // VK_FORMAT_ASTC_10x5_SRGB_BLOCK
    , RESOURCE_FORMAT_ASTC_10x5_SFLOAT_BLOCK_EXT_VK                 // VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK_EXT
    , RESOURCE_FORMAT_ASTC_10x6_TYPELESS_BLOCK_VK                   // VK_FORMAT_ASTC_10x6_*_BLOCK (MUTABLE_FORMAT)
    , RESOURCE_FORMAT_ASTC_10x6_UNORM_BLOCK_VK                      // VK_FORMAT_ASTC_10x6_UNORM_BLOCK
    , RESOURCE_FORMAT_ASTC_10x6_SRGB_BLOCK_VK                       // VK_FORMAT_ASTC_10x6_SRGB_BLOCK
    , RESOURCE_FORMAT_ASTC_10x6_SFLOAT_BLOCK_EXT_VK                 // VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK_EXT
    , RESOURCE_FORMAT_ASTC_10x8_TYPELESS_BLOCK_VK                   // VK_FORMAT_ASTC_10x8_*_BLOCK (MUTABLE_FORMAT)
    , RESOURCE_FORMAT_ASTC_10x8_UNORM_BLOCK_VK                      // VK_FORMAT_ASTC_10x8_UNORM_BLOCK
    , RESOURCE_FORMAT_ASTC_10x8_SRGB_BLOCK_VK                       // VK_FORMAT_ASTC_10x8_SRGB_BLOCK
    , RESOURCE_FORMAT_ASTC_10x8_SFLOAT_BLOCK_EXT_VK                 // VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK_EXT
    , RESOURCE_FORMAT_ASTC_10x10_TYPELESS_BLOCK_VK                  // VK_FORMAT_ASTC_10x10_*_BLOCK (MUTABLE_FORMAT)
    , RESOURCE_FORMAT_ASTC_10x10_UNORM_BLOCK_VK                     // VK_FORMAT_ASTC_10x10_UNORM_BLOCK
    , RESOURCE_FORMAT_ASTC_10x10_SRGB_BLOCK_VK                      // VK_FORMAT_ASTC_10x10_SRGB_BLOCK
    , RESOURCE_FORMAT_ASTC_10x10_SFLOAT_BLOCK_EXT_VK                // VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK_EXT
    , RESOURCE_FORMAT_ASTC_12x10_TYPELESS_BLOCK_VK                  // VK_FORMAT_ASTC_12x10_*_BLOCK (MUTABLE_FORMAT)
    , RESOURCE_FORMAT_ASTC_12x10_UNORM_BLOCK_VK                     // VK_FORMAT_ASTC_12x10_UNORM_BLOCK
    , RESOURCE_FORMAT_ASTC_12x10_SRGB_BLOCK_VK                      // VK_FORMAT_ASTC_12x10_SRGB_BLOCK
    , RESOURCE_FORMAT_ASTC_12x10_SFLOAT_BLOCK_EXT_VK                // VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK_EXT
    , RESOURCE_FORMAT_ASTC_12x12_TYPELESS_BLOCK_VK                  // VK_FORMAT_ASTC_12x12_*_BLOCK (MUTABLE_FORMAT)
    , RESOURCE_FORMAT_ASTC_12x12_UNORM_BLOCK_VK                     // VK_FORMAT_ASTC_12x12_UNORM_BLOCK
    , RESOURCE_FORMAT_ASTC_12x12_SRGB_BLOCK_VK                      // VK_FORMAT_ASTC_12x12_SRGB_BLOCK
    , RESOURCE_FORMAT_ASTC_12x12_SFLOAT_BLOCK_EXT_VK                // VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK_EXT
    , RESOURCE_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG_VK                // VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG
    , RESOURCE_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG_VK                // VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG
    , RESOURCE_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG_VK                // VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG
    , RESOURCE_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG_VK                // VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG
    , RESOURCE_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG_VK                 // VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG
    , RESOURCE_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG_VK                 // VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG
    , RESOURCE_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG_VK                 // VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG
    , RESOURCE_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG_VK                 // VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG

/*
    // Vulkanフォーマット
    // R_VK
    , RESOURCE_FORMAT_R8_USCALED                                    // VK_FORMAT_R8_USCALED 
    , RESOURCE_FORMAT_R8_SSCALED                                    // VK_FORMAT_R8_SSCALED 
    , RESOURCE_FORMAT_R8_SRGB                                       // VK_FORMAT_R8_SRGB    
    , RESOURCE_FORMAT_R16_USCALED                                   // VK_FORMAT_R16_USCALED
    , RESOURCE_FORMAT_R16_SSCALED                                   // VK_FORMAT_R16_SSCALED
    , RESOURCE_FORMAT_R64_UINT                                      // VK_FORMAT_R64_UINT   
    , RESOURCE_FORMAT_R64_SINT                                      // VK_FORMAT_R64_SINT   
    , RESOURCE_FORMAT_R64_SFLOAT                                    // VK_FORMAT_R64_SFLOAT 

    // RG_VK
    , RESOURCE_FORMAT_R8G8_USCALED                                  // VK_FORMAT_R8G8_USCALED
    , RESOURCE_FORMAT_R8G8_SSCALED                                  // VK_FORMAT_R8G8_SSCALED
    , RESOURCE_FORMAT_R8G8_SRGB                                     // VK_FORMAT_R8G8_SRGB
    , RESOURCE_FORMAT_R64G64_UINT                                   // VK_FORMAT_R64G64_UINT
    , RESOURCE_FORMAT_R64G64_SINT                                   // VK_FORMAT_R64G64_SINT
    , RESOURCE_FORMAT_R64G64_SFLOAT                                 // VK_FORMAT_R64G64_SFLOAT
    // GR_VK
    , RESOURCE_FORMAT_G4R4_UNORM                                    // VK_FORMAT_R4G4_UNORM_PACK8
    , RESOURCE_FORMAT_R16G16_USCALED                                // VK_FORMAT_R16G16_USCALED
    , RESOURCE_FORMAT_R16G16_SSCALED                                // VK_FORMAT_R16G16_SSCALED

    // RGB_VK
    , RESOURCE_FORMAT_R5G6B5_UNORM                                  // VK_FORMAT_B5G6R5_UNORM_PACK16

    // RGB_VK
    , RESOURCE_FORMAT_R8G8B8_UNORM                                  // VK_FORMAT_R8G8B8_UNORM
    , RESOURCE_FORMAT_R8G8B8_SNORM                                  // VK_FORMAT_R8G8B8_SNORM
    , RESOURCE_FORMAT_R8G8B8_USCALED                                // VK_FORMAT_R8G8B8_USCALED
    , RESOURCE_FORMAT_R8G8B8_SSCALED                                // VK_FORMAT_R8G8B8_SSCALED
    , RESOURCE_FORMAT_R8G8B8_UINT                                   // VK_FORMAT_R8G8B8_UINT
    , RESOURCE_FORMAT_R8G8B8_SINT                                   // VK_FORMAT_R8G8B8_SINT
    , RESOURCE_FORMAT_R8G8B8_SRGB                                   // VK_FORMAT_R8G8B8_SRGB
    , RESOURCE_FORMAT_R64G64B64_UINT                                // VK_FORMAT_R64G64B64_UINT
    , RESOURCE_FORMAT_R64G64B64_SINT                                // VK_FORMAT_R64G64B64_SINT
    , RESOURCE_FORMAT_R64G64B64_SFLOAT                              // VK_FORMAT_R64G64B64_SFLOAT
    , RESOURCE_FORMAT_R16G16B16_UNORM                               // VK_FORMAT_R16G16B16_UNORM
    , RESOURCE_FORMAT_R16G16B16_SNORM                               // VK_FORMAT_R16G16B16_SNORM
    , RESOURCE_FORMAT_R16G16B16_USCALED                             // VK_FORMAT_R16G16B16_USCALED
    , RESOURCE_FORMAT_R16G16B16_SSCALED                             // VK_FORMAT_R16G16B16_SSCALED
    , RESOURCE_FORMAT_R16G16B16_UINT                                // VK_FORMAT_R16G16B16_UINT
    , RESOURCE_FORMAT_R16G16B16_SINT                                // VK_FORMAT_R16G16B16_SINT
    , RESOURCE_FORMAT_R16G16B16_FLOAT                               // VK_FORMAT_R16G16B16_SFLOAT
    // BGR_VK
    , RESOURCE_FORMAT_B8G8R8_UNORM                                  // VK_FORMAT_B8G8R8_UNORM
    , RESOURCE_FORMAT_B8G8R8_SNORM                                  // VK_FORMAT_B8G8R8_SNORM
    , RESOURCE_FORMAT_B8G8R8_USCALED                                // VK_FORMAT_B8G8R8_USCALED
    , RESOURCE_FORMAT_B8G8R8_SSCALED                                // VK_FORMAT_B8G8R8_SSCALED
    , RESOURCE_FORMAT_B8G8R8_UINT                                   // VK_FORMAT_B8G8R8_UINT
    , RESOURCE_FORMAT_B8G8R8_SINT                                   // VK_FORMAT_B8G8R8_SINT
    , RESOURCE_FORMAT_B8G8R8_SRGB                                   // VK_FORMAT_B8G8R8_SRGB

    // RGBA_VK
    , RESOURCE_FORMAT_R8G8B8A8_USCALED                              // VK_FORMAT_R8G8B8A8_USCALED
    , RESOURCE_FORMAT_R8G8B8A8_SSCALED                              // VK_FORMAT_R8G8B8A8_SSCALED
    , RESOURCE_FORMAT_R16G16B16A16_USCALED                          // VK_FORMAT_R16G16B16A16_USCALED
    , RESOURCE_FORMAT_R16G16B16A16_SSCALED                          // VK_FORMAT_R16G16B16A16_SSCALED
    , RESOURCE_FORMAT_R64G64B64A64_UINT                             // VK_FORMAT_R64G64B64A64_UINT
    , RESOURCE_FORMAT_R64G64B64A64_SINT                             // VK_FORMAT_R64G64B64A64_SINT
    , RESOURCE_FORMAT_R64G64B64A64_SFLOAT                           // VK_FORMAT_R64G64B64A64_SFLOAT
    , RESOURCE_FORMAT_R8G8B8A8_USCALED                              // VK_FORMAT_A8B8G8R8_USCALED_PACK32
    , RESOURCE_FORMAT_R8G8B8A8_SSCALED                              // VK_FORMAT_A8B8G8R8_SSCALED_PACK32
    , RESOURCE_FORMAT_R8G8B8A8_SRGB                                 // VK_FORMAT_A8B8G8R8_SRGB_PACK32
    , RESOURCE_FORMAT_R10G10B10A2_SNORM                             // VK_FORMAT_A2B10G10R10_SNORM_PACK32
    , RESOURCE_FORMAT_R10G10B10A2_USCALED                           // VK_FORMAT_A2B10G10R10_USCALED_PACK32
    , RESOURCE_FORMAT_R10G10B10A2_SSCALED                           // VK_FORMAT_A2B10G10R10_SSCALED_PACK32
    , RESOURCE_FORMAT_R10G10B10A2_SINT                              // VK_FORMAT_A2B10G10R10_SINT_PACK32
    // BGRA_VK
    , RESOURCE_FORMAT_B8G8R8A8_SNORM                                // VK_FORMAT_B8G8R8A8_SNORM
    , RESOURCE_FORMAT_B8G8R8A8_USCALED                              // VK_FORMAT_B8G8R8A8_USCALED
    , RESOURCE_FORMAT_B8G8R8A8_SSCALED                              // VK_FORMAT_B8G8R8A8_SSCALED
    , RESOURCE_FORMAT_B8G8R8A8_UINT                                 // VK_FORMAT_B8G8R8A8_UINT
    , RESOURCE_FORMAT_B8G8R8A8_SINT                                 // VK_FORMAT_B8G8R8A8_SINT
    , RESOURCE_FORMAT_B10G10R10A2_UNORM                             // VK_FORMAT_A2R10G10B10_UNORM_PACK32
    , RESOURCE_FORMAT_B10G10R10A2_SNORM                             // VK_FORMAT_A2R10G10B10_SNORM_PACK32
    , RESOURCE_FORMAT_B10G10R10A2_USCALED                           // VK_FORMAT_A2R10G10B10_USCALED_PACK32
    , RESOURCE_FORMAT_B10G10R10A2_SSCALED                           // VK_FORMAT_A2R10G10B10_SSCALED_PACK32
    , RESOURCE_FORMAT_B10G10R10A2_UINT                              // VK_FORMAT_A2R10G10B10_UINT_PACK32
    , RESOURCE_FORMAT_B10G10R10A2_SINT                              // VK_FORMAT_A2R10G10B10_SINT_PACK32
    // ARGB_VK
    , RESOURCE_FORMAT_A1R5G5B5_UNORM;                               // VK_FORMAT_B5G5R5A1_UNORM_PACK16
    , RESOURCE_FORMAT_A4R4G4B4_UNORM;                               // VK_FORMAT_B4G4R4A4_UNORM_PACK16
    // ABGR_VK
    , RESOURCE_FORMAT_A1B5G5R5_UNORM;                               // VK_FORMAT_R5G5B5A1_UNORM_PACK16
    , RESOURCE_FORMAT_A4B4G4R4_UNORM;                               // VK_FORMAT_R4G4B4A4_UNORM_PACK16

    // 深度ステンシルフォーマット_VK
    , RESOURCE_FORMAT_D24_UNORM_X8_TYPELESS                         // VK_FORMAT_X8_D24_UNORM_PACK32
    , RESOURCE_FORMAT_S8_UINT                                       // VK_FORMAT_S8_UINT
    , RESOURCE_FORMAT_D16_UNORM_S8_UINT                             // VK_FORMAT_D16_UNORM_S8_UINT

    // ビデオフォーマット_VK
    , RESOURCE_FORMAT_B8G8R8G8_422_UNORM                            // VK_FORMAT_B8G8R8G8_422_UNORM
    , RESOURCE_FORMAT_G8_B8_R8_3PLANE_420_UNORM                     // VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM
    , RESOURCE_FORMAT_G8_B8_R8_3PLANE_422_UNORM                     // VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM
    , RESOURCE_FORMAT_G8_B8R8_2PLANE_422_UNORM                      // VK_FORMAT_G8_B8R8_2PLANE_422_UNORM
    , RESOURCE_FORMAT_G8_B8_R8_3PLANE_444_UNORM                     // VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM
    , RESOURCE_FORMAT_X6R10_UNORM                                   // VK_FORMAT_R10X6_UNORM_PACK16
    , RESOURCE_FORMAT_G10X6R10X6_UNORM                              // VK_FORMAT_R10X6G10X6_UNORM_2PACK16
    , RESOURCE_FORMAT_A10X6B10X6G10X6R10X6_UNORM                    // VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16
    , RESOURCE_FORMAT_X6B10X6G10X6R10X6G10_422_UNORM                // VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16
    , RESOURCE_FORMAT_X6G10_X6B10_X6R10_3PLANE_420_UNORM            // VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16
    , RESOURCE_FORMAT_X6G10_X6B10_X6R10_3PLANE_422_UNORM            // VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16
    , RESOURCE_FORMAT_X6G10_X6B10X6R10_2PLANE_422_UNORM             // VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16
    , RESOURCE_FORMAT_X6G10_X6B10_X6R10_3PLANE_444_UNORM            // VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16
    , RESOURCE_FORMAT_X4R12_UNORM                                   // VK_FORMAT_R12X4_UNORM_PACK16
    , RESOURCE_FORMAT_X4R12X4G12_UNORM                              // VK_FORMAT_R12X4G12X4_UNORM_2PACK16
    , RESOURCE_FORMAT_X4R12X4G12X4B12X4A12_UNORM                    // VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16
    , RESOURCE_FORMAT_X4G12X4B12X4G12X4R12_422_UNORM                // VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16
    , RESOURCE_FORMAT_X4B12X4G12X4R12X4G12_422_UNORM                // VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16
    , RESOURCE_FORMAT_X4G12_X4B12_X4R12_3PLANE_420_UNORM            // VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16
    , RESOURCE_FORMAT_X4G12_X4B12X4R12_2PLANE_420_UNORM             // VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16
    , RESOURCE_FORMAT_X4G12_X4B12_X4R12_3PLANE_422_UNORM            // VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16
    , RESOURCE_FORMAT_X4G12_X4B12X4R12_2PLANE_422_UNORM             // VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16
    , RESOURCE_FORMAT_X4G12_X4B12_R12X4_3PLANE_444_UNORM            // VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16
    , RESOURCE_FORMAT_B16G16R16G16_422_UNORM                        // VK_FORMAT_B16G16R16G16_422_UNORM
    , RESOURCE_FORMAT_G16_B16_R16_3PLANE_420_UNORM                  // VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM
    , RESOURCE_FORMAT_G16_B16_R16_3PLANE_422_UNORM                  // VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM
    , RESOURCE_FORMAT_G16_B16R16_2PLANE_422_UNORM                   // VK_FORMAT_G16_B16R16_2PLANE_422_UNORM
    , RESOURCE_FORMAT_G16_B16_R16_3PLANE_444_UNORM                  // VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM

    //x
    , RESOURCE_FORMAT_R8G8B8A8_UNORM                                // same VK_FORMAT_A8B8G8R8_UNORM_PACK32
    , RESOURCE_FORMAT_R8G8B8A8_SNORM                                // same VK_FORMAT_A8B8G8R8_SNORM_PACK32
    , RESOURCE_FORMAT_R8G8B8A8_UINT                                 // same VK_FORMAT_A8B8G8R8_UINT_PACK32
    , RESOURCE_FORMAT_R8G8B8A8_SINT                                 // same VK_FORMAT_A8B8G8R8_SINT_PACK32
*/

};

struct TEXTURE_FORMAT_DESC
{
    RESOURCE_FORMAT        format;              // リソースの基となるフォーマットを指定します。*_TYPELESSを指定した場合、互換性のある追加のフォーマットをmutable_formatsに指定する必要がありますが、デフォルトでnum_mutable_formatsに0を指定し、互換性のあるフォーマットを全て選択できます。
    uint32_t               num_mutable_formats; // mutable_formats配列の要素の数です。formatが *_TYPELESSフォーマットでない場合この値は無視されます。
    const RESOURCE_FORMAT* mutable_formats;     // formatが *_TYPELESSフォーマットの際、ビューオブジェクト作成時に指定可能な追加のフォーマットを指定します。
};

struct RESOURCE_HEAP_ALLOCATION_INFO
{
    uint64_t required_alignment;
    uint64_t total_size_in_bytes;
    uint32_t heap_type_bits;        // RESOURCE_ALLOCATION_INFO::heap_type_bits のサブセットです。
};

struct RESOURCE_ALLOCATION_INFO
{
    uint64_t heap_offset;
    uint64_t alignment;
    uint64_t size_in_bytes;
    uint32_t heap_type_bits;
};

#pragma region factory

enum DEBUG_MESSAGE_CATEGORY_FLAG : EnumT
{
      DEBUG_MESSAGE_CATEGORY_FLAG_UNKNOWN                   = 0x1
    , DEBUG_MESSAGE_CATEGORY_FLAG_MISCELLANEOUS             = 0x2
    , DEBUG_MESSAGE_CATEGORY_FLAG_INITIALIZATION            = 0x4
    , DEBUG_MESSAGE_CATEGORY_FLAG_CLEANUP                   = 0x8
    , DEBUG_MESSAGE_CATEGORY_FLAG_COMPILATION               = 0x10
    , DEBUG_MESSAGE_CATEGORY_FLAG_STATE_CREATION            = 0x20
    , DEBUG_MESSAGE_CATEGORY_FLAG_STATE_SETTING             = 0x40
    , DEBUG_MESSAGE_CATEGORY_FLAG_STATE_GETTING             = 0x80
    , DEBUG_MESSAGE_CATEGORY_FLAG_RESOURCE_MANIPULATION     = 0x100
    , DEBUG_MESSAGE_CATEGORY_FLAG_EXECUTION                 = 0x200
    , DEBUG_MESSAGE_CATEGORY_FLAG_SHADER                    = 0x400
    , DEBUG_MESSAGE_CATEGORY_FLAG_B3D                       = 0x800
    , DEBUG_MESSAGE_CATEGORY_FLAG_B3D_DETAILS               = 0x1000

    // FIXME: DEBUG_MESSAGE_CATEGORY_FLAG_ALL
    , DEBUG_MESSAGE_CATEGORY_FLAG_ALL                       = DEBUG_MESSAGE_CATEGORY_FLAG_UNKNOWN
                                                            | DEBUG_MESSAGE_CATEGORY_FLAG_MISCELLANEOUS
                                                            | DEBUG_MESSAGE_CATEGORY_FLAG_INITIALIZATION
                                                            | DEBUG_MESSAGE_CATEGORY_FLAG_CLEANUP
                                                            | DEBUG_MESSAGE_CATEGORY_FLAG_COMPILATION
                                                            | DEBUG_MESSAGE_CATEGORY_FLAG_STATE_CREATION
                                                            | DEBUG_MESSAGE_CATEGORY_FLAG_STATE_SETTING
                                                            | DEBUG_MESSAGE_CATEGORY_FLAG_STATE_GETTING
                                                            | DEBUG_MESSAGE_CATEGORY_FLAG_RESOURCE_MANIPULATION
                                                            | DEBUG_MESSAGE_CATEGORY_FLAG_EXECUTION
                                                            | DEBUG_MESSAGE_CATEGORY_FLAG_SHADER
                                                            | DEBUG_MESSAGE_CATEGORY_FLAG_B3D        
                                                            | DEBUG_MESSAGE_CATEGORY_FLAG_B3D_DETAILS
};
using DEBUG_MESSAGE_CATEGORY_FLAGS = EnumFlagsT;

enum DEBUG_MESSAGE_SEVERITY : EnumT
{
      DEBUG_MESSAGE_SEVERITY_INFO
    , DEBUG_MESSAGE_SEVERITY_WARNING
    , DEBUG_MESSAGE_SEVERITY_ERROR
    , DEBUG_MESSAGE_SEVERITY_CORRUPTION
    , DEBUG_MESSAGE_SEVERITY_OTHER
    , DEBUG_MESSAGE_SEVERITY_END
};

/**
 * @brief ユーザー定義用のメッセージコールバック関数ポインタの宣言。
 * @param _severity メッセージの重要度が指定されます。 
 * @param _category メッセージのカテゴリが指定されます。 
 * @param _message メッセージ文字列。
 * @param _user_data DEBUG_MESSAGE_CALLABCK_DESC::user_data で指定されたユーザーデータ。
 * @note この関数は重要度、カテゴリが引数に渡るため、関数内でメッセージのフィルタリングが可能です。
*/
using PFN_Buma3DDebugMessageCallback = void (B3D_APIENTRY*)(DEBUG_MESSAGE_SEVERITY _severity, DEBUG_MESSAGE_CATEGORY_FLAG _category, const Char8T* const _message, void* _user_data);

struct DEBUG_MESSAGE_CALLABCK_DESC
{
    PFN_Buma3DDebugMessageCallback    Callback;     // ユーザー定義のメッセージコールバック関数へのポインタ。
    void*                             user_data;    // ユーザー定義のメッセージコールバック関数で使用可能なユーザーデータ。
};

// 有効にするデバッグ情報を設定します。
struct DEBUG_MESSAGE_DESC
{
    DEBUG_MESSAGE_SEVERITY             severity;                // 有効にするメッセージの重要度です。
    DEBUG_MESSAGE_CATEGORY_FLAGS       category_flags;          // 有効にするメッセージの重要度における、有効にするメッセージのカテゴリです。
    bool                               is_enabled_debug_break;  // 有効にするメッセージの重要度のメッセージを取得した際にデバッグブレイクを実行するかどうかを指定します。
};

enum GPU_BASED_VALIDATION_FLAG : EnumT
{
    GPU_BASED_VALIDATION_FLAG_NONE = 0x0
};
using GPU_BASED_VALIDATION_FLAGS = EnumFlagsT;

struct GPU_BASED_VALIDATION_DESC
{
    bool                        is_enabled;
    GPU_BASED_VALIDATION_FLAGS  flags;
};

struct DEVICE_FACTORY_DEBUG
{
    bool                           is_enabled;              // デバッグメッセージを有効にするかを指定します。
    DEBUG_MESSAGE_CALLABCK_DESC    debug_message_callback;  // デバッグメッセージ発行時のコールバック関数を指定します。
    uint32_t                       num_debug_messages;      // デバッグメッセージの重要度の数です [0 ~ DEBUG_MESSAGE_SEVERITY_END)。 この値が0の場合、キューにメッセージは追加されませんが、引き続きdebug_message_callbackに指定されたコールバック関数には全ての重要度、カテゴリのメッセージが通知されます。 
    const DEBUG_MESSAGE_DESC*      debug_messages;          // デバッグメッセージの重要度配列への有効なポインタです。
    GPU_BASED_VALIDATION_DESC      gpu_based_validation;    // 実装によるGPU検証を有効にするかを指定します。
};

enum DEVICE_FACTORY_FLAG : EnumT
{
    DEVICE_FACTORY_FLAG_NONE = 0x0
};
using DEVICE_FACTORY_FLAGS = EnumFlagsT;

struct DEVICE_FACTORY_DESC
{
    DEVICE_FACTORY_FLAGS flags;
    DEVICE_FACTORY_DEBUG debug;
};

enum DEVICE_ADAPTER_TYPE : EnumT
{
      DEVICE_ADAPTER_TYPE_OTHER
    , DEVICE_ADAPTER_TYPE_INTEGRATED_GPU
    , DEVICE_ADAPTER_TYPE_DISCRETE_GPU
    , DEVICE_ADAPTER_TYPE_VIRTUAL_GPU
    , DEVICE_ADAPTER_TYPE_CPU
};

struct DEVICE_ADAPTER_DESC
{
    char                device_name[256];       // デバイス名です。
    uint32_t            vendor_id;              // デバイスのベンダーIDです。
    uint32_t            device_id;              // デバイスのIDです。
    uint8_t             adapter_luid[8];        // アダプターのLUIDです。OSの終了時まで値が固有です。
    size_t              dedicated_video_memory; // デバイス専用のビデオメモリのサイズです。
    size_t              shared_system_memory;   // デバイスとホストが共有可能なメモリのサイズです。
    uint32_t            node_count;             // アダプターが抽象化している物理的なGPUの数です。
    DEVICE_ADAPTER_TYPE adapter_type;
};

/*
* NOTE: DXGI_SAMPLE_DESC::Quality(品質レベル)の定義は、ハードウェアベンダーが定義する必要がありますが、この情報を見つけるのに役立つ機能はD3Dによって提供されていません。: https://microsoft.github.io/DirectX-Specs/d3d/archive/D3D11_3_FunctionalSpec.htm#19.2.3%20Optional%20Multisample%20Support
*/

enum SAMPLE_COUNT_FLAG : EnumT
{
      SAMPLE_COUNT_FLAG_NOT_SUPPORTED = 0x0
    , SAMPLE_COUNT_FLAG_1             = 0x1
    , SAMPLE_COUNT_FLAG_2             = 0x2
    , SAMPLE_COUNT_FLAG_4             = 0x4
    , SAMPLE_COUNT_FLAG_8             = 0x8
    , SAMPLE_COUNT_FLAG_16            = 0x10
    , SAMPLE_COUNT_FLAG_32            = 0x20
    , SAMPLE_COUNT_FLAG_64            = 0x40
};
using SAMPLE_COUNT_FLAGS = EnumFlagsT;

#pragma endregion factory

#pragma region features

// TODO: FORMAT_FEATURE
enum FORMAT_FEATURE : EnumT
{
      FORMAT_FEATURE_TEXTURE_3D_AS_2D_ARRAY
    , FORMAT_FEATURE_SHADER_RESOURCE
    , FORMAT_FEATURE_UNORDERED_ACCESS
    , FORMAT_FEATURE_TYPED_SHADER_RESOURCE_BUFFER
    , FORMAT_FEATURE_TYPED_UNORDERED_ACCESS_BUFFER
};

struct DEVICE_ADAPTER_LIMITS
{
    uint32_t                max_texture_dimension_1d;                                   // D3D12_REQ_TEXTURE1D_U_DIMENSION
    uint32_t                max_texture_dimension_2d;                                   // D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION
    uint32_t                max_texture_dimension_3d;                                   // D3D12_REQ_TEXTURE3D_U_V_OR_W_DIMENSION
    uint32_t                max_texture_dimension_cube;                                 // D3D12_REQ_TEXTURECUBE_DIMENSION
    uint32_t                max_texture_array_size;                                     // D3D12_REQ_TEXTURE*D_ARRAY_AXIS_DIMENSION
    uint32_t                max_typed_buffer_elements;                                  // UINT32_MAX n/a
    uint32_t                max_constant_buffer_range;                                  // UINT32_MAX n/a
    uint32_t                max_unordered_access_buffer_range;                          // UINT32_MAX n/a
    uint32_t                max_push_32bit_constants_range;                             // D3D12_MAX_ROOT_COST(64) ルートシグネチャ内全てをプッシュ定数にした場合の最大値です。 
    uint32_t                max_memory_allocation_count;                                // UINT32_MAX n/a
    uint32_t                max_sampler_allocation_count;                               // D3D12_REQ_SAMPLER_OBJECT_COUNT_PER_DEVICE

    /**
     * @brief 1 (D3D12_ * _RESOURCE_PLACEMENT_ALIGNMENTで決定) 
     *        ヒープ内に線形リソースと非線形リソースが順序を問わず隣接する場合に、エイリアスを発生させない割り当てに必要なアライメントです。 
     *        この要求が満たされない場合、隣接するリソースはエイリアスします。 
    */
    uint64_t                buffer_texture_granularity;
    uint64_t                sparse_address_space_size;                                  // UINT64_MAX n/a

    //uint32_t              max_bound_register_spaces;                                  // ｍaxBoundDescriptorSets
    //uint32_t              max_per_stage_descriptor_samplers;                          // tier1:16 tier2:full-heap tier3:full-heap
    //uint32_t              max_per_stage_descriptor_constant_buffers;                  // 
    //uint32_t              max_per_stage_descriptor_unordered_access_buffers;          // 
    //uint32_t              max_per_stage_descriptor_srv_textures;                      // 
    //uint32_t              max_per_stage_descriptor_uav_textures;                      // 
    //uint32_t              max_per_stage_descriptor_input_attachments;                 // 
    //uint32_t              max_per_stage_resources;                                    // 
    //uint32_t              max_descriptor_set_samplers;                                // 
    //uint32_t              max_descriptor_set_constant_buffers;                        // 
    //uint32_t              max_descriptor_set_constant_buffers_dynamic;                // 
    //uint32_t              max_descriptor_set_unordered_access_buffers;                // 
    //uint32_t              max_descriptor_set_unordered_access_buffers_dynamic;        // 
    //uint32_t              max_descriptor_set_srv_textures;                            // 
    //uint32_t              max_descriptor_set_uav_textures;                            // 
    //uint32_t              max_descriptor_set_input_attachments;                       // 

    uint32_t                max_vertex_input_attributes;                                // D3D12_IA_VERTEX_INPUT_STRUCTURE_ELEMENT_COUNT
    uint32_t                max_vertex_input_bindings;                                  // D3D12_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT
    uint32_t                max_vertex_input_attribute_offset;                          // D3D12_IA_VERTEX_INPUT_STRUCTURE_ELEMENTS_COMPONENTS
    uint32_t                max_vertex_input_binding_stride;                            // 4 * D3D12_IA_VERTEX_INPUT_STRUCTURE_ELEMENT_COUNT * D3D12_IA_VERTEX_INPUT_STRUCTURE_ELEMENTS_COMPONENTS
    uint32_t                max_vertex_output_components;                               // D3D12_IA_VERTEX_INPUT_STRUCTURE_ELEMENTS_COMPONENTS
    uint32_t                max_vertex_instance_data_step_rate;                         // UINT32_MAX

    uint32_t                max_tessellation_generation_level;                          // D3D12_HS_MAXTESSFACTOR_UPPER_BOUND
    uint32_t                max_tessellation_patch_size;                                // D3D12_IA_PATCH_MAX_CONTROL_POINT_COUNT
    uint32_t                max_tessellation_control_per_vertex_input_components;       // D3D12_VS_INPUT_REGISTER_COUNT * D3D12_VS_INPUT_REGISTER_COMPONENTS
    uint32_t                max_tessellation_control_per_vertex_output_components;      // D3D12_VS_OUTPUT_REGISTER_COUNT * D3D12_VS_OUTPUT_REGISTER_COMPONENTS
    uint32_t                max_tessellation_control_per_patch_output_components;       // D3D12_HS_OUTPUT_PATCH_CONSTANT_REGISTER_SCALAR_COMPONENTS
    uint32_t                max_tessellation_control_total_output_components;           // D3D12_HS_OUTPUT_CONTROL_POINTS_MAX_TOTAL_SCALARS
    uint32_t                max_tessellation_evaluation_input_components;               // D3D12_DS_INPUT_CONTROL_POINT_REGISTER_COMPONENTS * D3D12_DS_INPUT_CONTROL_POINT_REGISTER_COUNT
    uint32_t                max_tessellation_evaluation_output_components;              // D3D12_DS_OUTPUT_CONTROL_POINT_REGISTER_COMPONENTS * D3D12_DS_OUTPUT_CONTROL_POINT_REGISTER_COUNT

    uint32_t                max_geometry_shader_invocations;                            // D3D12_GS_MAX_INSTANCE_COUNT
    uint32_t                max_geometry_input_components;                              // D3D12_GS_INPUT_REGISTER_COUNT * D3D12_GS_INPUT_REGISTER_COMPONENTS
    uint32_t                max_geometry_output_components;                             // D3D12_GS_OUTPUT_REGISTER_COUNT * D3D12_GS_OUTPUT_REGISTER_COMPONENTS
    uint32_t                max_geometry_output_vertices;                               // D3D12_GS_MAX_OUTPUT_VERTEX_COUNT_ACROSS_INSTANCES
    uint32_t                max_geometry_total_output_components;                       // D3D12_REQ_GS_INVOCATION_32BIT_OUTPUT_COMPONENT_LIMIT

    uint32_t                max_fragment_input_components;                              // D3D12_PS_INPUT_REGISTER_COUNT * D3D12_PS_INPUT_REGISTER_COMPONENTS
    uint32_t                max_fragment_output_attachments;                            // D3D12_PS_OUTPUT_REGISTER_COUNT
    uint32_t                max_fragment_dual_src_attachments;                          // 1 : https://microsoft.github.io/DirectX-Specs/d3d/archive/D3D11_3_FunctionalSpec.htm#17.6%20Dual%20Source%20Color%20Blending
    uint32_t                max_fragment_combined_output_resources;                     // D3D12_PS_OUTPUT_REGISTER_COUNT + D3D12_PS_CS_UAV_REGISTER_COUNT

    uint32_t                max_compute_shared_memory_size;                             // D3D12_CS_THREADID_REGISTER_COMPONENTS * D3D12_CS_THREAD_LOCAL_TEMP_REGISTER_POOL
    uint32_t                max_compute_work_group_count[3];                            // D3D12_CS_DISPATCH_MAX_THREAD_GROUPS_PER_DIMENSION
    uint32_t                max_compute_work_group_invocations;                         // D3D12_CS_THREAD_GROUP_MAX_THREADS_PER_GROUP
    uint32_t                max_compute_work_group_size[3];                             // D3D12_CS_THREAD_GROUP_MAX_{X,Y,Z}

    uint32_t                subpixel_precision_bits;                                    // D3D12_SUBPIXEL_FRACTIONAL_BIT_COUNT
    uint32_t                subtexel_precision_bits;                                    // D3D12_SUBTEXEL_FRACTIONAL_BIT_COUNT
    uint32_t                mipmap_precision_bits;                                      // D3D12_MIP_LOD_FRACTIONAL_BIT_COUNT

    uint32_t                max_draw_indexed_index_value;                               // UINT32_MAX (n/a)
    uint32_t                max_draw_indirect_count;                                    // UINT32_MAX (n/a)
    float                   max_sampler_lod_bias;                                       // D3D12_MIP_LOD_BIAS_MAX (D3D12_REQ_MIP_LEVELS ?)
    float                   max_sampler_anisotropy;                                     // D3D12_REQ_MAXANISOTROPY

    uint32_t                max_viewports;                                              // D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE
    uint32_t                max_viewport_dimensions[2];                                 // D3D12_VIEWPORT_BOUNDS_MAX
    float                   viewport_bounds_range[2];                                   // D3D12_VIEWPORT_BOUNDS_{ MIN,MAX }
    uint32_t                viewport_subpixel_bits;                                     // D3D12_SUBPIXEL_FRACTIONAL_BIT_COUNT

    size_t                  min_memory_map_alignment;                                   // 1 (n/a)
    uint64_t                min_srv_typed_buffer_offset_alignment;                      // D3D12_RAW_UAV_SRV_BYTE_ALIGNMENT uniformTexelBufferOffsetAlignmentBytes
    uint64_t                min_uav_typed_buffer_offset_alignment;                      // D3D12_RAW_UAV_SRV_BYTE_ALIGNMENT storageTexelBufferOffsetAlignmentBytes
    uint64_t                min_constant_buffer_offset_alignment;                       // D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT
    uint64_t                min_unordered_access_buffer_offset_alignment;               // 1 (n/a)
    int32_t                 min_texel_offset;                                           // D3D12_COMMONSHADER_TEXEL_OFFSET_MAX_NEGATIVE
    uint32_t                max_texel_offset;                                           // D3D12_COMMONSHADER_TEXEL_OFFSET_MAX_POSITIVE
    int32_t                 min_texel_gather_offset;                                    // -32 : https://microsoft.github.io/DirectX-Specs/d3d/archive/D3D11_3_FunctionalSpec.htm#22.4.5%20gather4_po_c
    uint32_t                max_texel_gather_offset;                                    // 31  : https://microsoft.github.io/DirectX-Specs/d3d/archive/D3D11_3_FunctionalSpec.htm#22.4.5%20gather4_po_c
    float                   min_interpolation_offset;                                   // -0.5f   : https://microsoft.github.io/DirectX-Specs/d3d/archive/D3D11_3_FunctionalSpec.htm#16.5.3%20Pull%20Model:%20Mapping%20Fixed%20Point%20Coordinates%20to%20Float%20Offsets%20on%20Sample%20Grid
    float                   max_interpolation_offset;                                   // 0.4375f : https://microsoft.github.io/DirectX-Specs/d3d/archive/D3D11_3_FunctionalSpec.htm#16.5.3%20Pull%20Model:%20Mapping%20Fixed%20Point%20Coordinates%20to%20Float%20Offsets%20on%20Sample%20Grid
    uint32_t                subpixel_interpolation_offset_bits;                         // 4       : https://microsoft.github.io/DirectX-Specs/d3d/archive/D3D11_3_FunctionalSpec.htm#16.5.3%20Pull%20Model:%20Mapping%20Fixed%20Point%20Coordinates%20to%20Float%20Offsets%20on%20Sample%20Grid

    uint32_t                max_framebuffer_width;                                      // D3D12_VIEWPORT_BOUNDS_MAX
    uint32_t                max_framebuffer_height;                                     // D3D12_VIEWPORT_BOUNDS_MAX
    //SAMPLE_COUNT_FLAG     framebuffer_color_sample_counts;                            
    //SAMPLE_COUNT_FLAG     framebuffer_depth_sample_counts;                            
    //SAMPLE_COUNT_FLAG     framebuffer_stencil_sample_counts;                          
    //SAMPLE_COUNT_FLAG     framebuffer_no_attachments_sample_counts;                   

    uint32_t                max_color_attachments;                                      // D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT
    //SAMPLE_COUNT_FLAG     sampled_texture_color_sample_counts;                        // 
    //SAMPLE_COUNT_FLAG     sampled_texture_integer_sample_counts;                      // 
    //SAMPLE_COUNT_FLAG     sampled_texture_depth_sample_counts;                        // 
    //SAMPLE_COUNT_FLAG     sampled_texture_stencil_sample_counts;                      // 
    //SAMPLE_COUNT_FLAG     storage_texture_sample_counts;                              // 
    //uint32_t              max_sample_mask_words;                                      // 
    //bool                  timestamp_compute_and_graphics;                             // 
    //float                 timestamp_period;                                           // 

    uint32_t                max_clip_distances;                                         // D3D12_CLIP_OR_CULL_DISTANCE_COUNT
    uint32_t                max_cull_distances;                                         // D3D12_CLIP_OR_CULL_DISTANCE_COUNT
    uint32_t                max_combined_clip_and_cull_distances;                       // D3D12_CLIP_OR_CULL_DISTANCE_COUNT

    //uint32_t              discrete_queue_priorities;

    float                   point_size_range[2];                                        // {1,1} 3.4.6 Point Rasterization Rules: https://microsoft.github.io/DirectX-Specs/d3d/archive/D3D11_3_FunctionalSpec.htm#3.4.6%20Point%20Rasterization%20Rules
    float                   line_width_range[2];                                        // {1,1} 3.4.3 Aliased Line Rasterization Rules : https://microsoft.github.io/DirectX-Specs/d3d/archive/D3D11_3_FunctionalSpec.htm#3.4.3%20Aliased%20Line%20Rasterization%20Rules
    float                   point_size_granularity;                                     // 1
    float                   line_width_granularity;                                     // 1
    //bool                  strict_lines;

    //bool                  standard_sample_locations;

    uint64_t                buffer_copy_offset_alignment;                               // D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT
    uint64_t                buffer_copy_row_pitch_alignment;                            // D3D12_TEXTURE_DATA_PITCH_ALIGNMENT
    uint64_t                non_coherent_atom_size;                                     // 1 
};

enum SHADER_STAGE_FLAG : EnumT
{
      SHADER_STAGE_FLAG_UNKNOWN      = 0x0
    , SHADER_STAGE_FLAG_VERTEX       = 0x1
    , SHADER_STAGE_FLAG_HULL         = 0x2
    , SHADER_STAGE_FLAG_DOMAIN       = 0x4
    , SHADER_STAGE_FLAG_GEOMETRY     = 0x8
    , SHADER_STAGE_FLAG_PIXEL        = 0x10
    , SHADER_STAGE_FLAG_COMPUTE      = 0x20
    , SHADER_STAGE_FLAG_TASK         = 0x40// AMPLIFICATION
    , SHADER_STAGE_FLAG_MESH         = 0x80
    , SHADER_STAGE_FLAG_RAYGEN       = 0x100
    , SHADER_STAGE_FLAG_ANY_HIT      = 0x200
    , SHADER_STAGE_FLAG_CLOSEST_HIT  = 0x400
    , SHADER_STAGE_FLAG_MISS         = 0x800
    , SHADER_STAGE_FLAG_INTERSECTION = 0x1000
    , SHADER_STAGE_FLAG_CALLABLE     = 0x2000
};
using SHADER_STAGE_FLAGS = EnumFlagsT;

// TODO: DEVICE_FEATURE

enum SUBGROUP_FEATURE_FLAG : EnumT
{
      WAVE_INTRINSICS_FEATURE_FLAG_NOT_SUPPORTED       = 0x0
    , WAVE_INTRINSICS_FEATURE_FLAG_BASIC               = 0x1
    , WAVE_INTRINSICS_FEATURE_FLAG_VOTE                = 0x2
    , WAVE_INTRINSICS_FEATURE_FLAG_ARITHMETIC          = 0x4
    , WAVE_INTRINSICS_FEATURE_FLAG_BALLOT              = 0x8
    , WAVE_INTRINSICS_FEATURE_FLAG_SHUFFLE             = 0x10
    , WAVE_INTRINSICS_FEATURE_FLAG_SHUFFLE_RELATIVE    = 0x20
    , WAVE_INTRINSICS_FEATURE_FLAG_CLUSTERED           = 0x40
    , WAVE_INTRINSICS_FEATURE_FLAG_QUAD                = 0x80
    , WAVE_INTRINSICS_FEATURE_FLAG_PARTITIONED         = 0x100
};
using WAVE_INTRINSICS_FEATURE_FLAGS = EnumFlagsT;

struct DEVICE_FEATURE_WAVE_INTRINSICS_PROPERTIES
{
    uint32_t                        wave_lane_count;
    SHADER_STAGE_FLAGS              supported_stages;
    WAVE_INTRINSICS_FEATURE_FLAGS   supported_operations;
    bool                            is_enabled_quad_operations_in_all_stages;
};

struct DEVICE_FEATURE_WAVE_LANE_COUNT_CONTROL_PROPERTIES
{
    bool                is_enabled_wave_lane_count_control;
    bool                is_enabled_compute_full_wave_lane_count;
    uint32_t            min_wave_lane_count;
    uint32_t            max_wave_lane_count;
    uint32_t            max_compute_workgroup_lane_count;
    SHADER_STAGE_FLAGS  required_lane_count_stages;
};

struct DEVICE_FEATURE_STREAM_OUTPUT_PROPERTIES
{
    bool                is_enabled_stream_output;
    uint32_t            max_stream_output_streams;                              // D3D12_SO_STREAM_COUNT
    uint32_t            max_stream_output_buffers;                              // D3D12_SO_BUFFER_SLOT_COUNT
    uint64_t            max_stream_output_buffer_size;                          // UINT64_MAX
    uint32_t            max_stream_output_stream_data_size;                     // D3D12_SO_BUFFER_MAX_STRIDE_IN_BYTES
    uint32_t            max_stream_output_buffer_data_size;                     // D3D12_SO_BUFFER_MAX_WRITE_WINDOW_IN_BYTES
    uint32_t            max_stream_output_buffer_data_stride;                   // D3D12_SO_BUFFER_MAX_STRIDE_IN_BYTES
    bool                is_enabled_stream_output_queries;                       // true
    bool                is_enabled_stream_output_streams_lines_triangles;       // false (When multiple GS output streams are used they must be pointlists)
    bool                is_enabled_stream_output_rasterization_stream_select;   // true 
};

#pragma endregion features

#pragma region swapchain

enum ROTATION_MODE : EnumT
{
      ROTATION_MODE_IDENTITY
    , ROTATION_MODE_ROTATE90
    , ROTATION_MODE_ROTATE180
    , ROTATION_MODE_ROTATE270
};

enum SURFACE_PLATFORM_DATA_TYPE : EnumT
{
      SURFACE_PLATFORM_DATA_TYPE_WINDOWS
    , SURFACE_PLATFORM_DATA_TYPE_ANDROID
};

struct SURFACE_PLATFORM_DATA_WINDOWS
{
    void* hinstance;
    void* hwnd;
};

struct SURFACE_PLATFORM_DATA_ANDROID
{
    void* a_native_window;
};

struct SURFACE_PLATFORM_DATA
{
    SURFACE_PLATFORM_DATA_TYPE     type;
    const void*                    data;
};

struct SURFACE_DESC
{
    SURFACE_PLATFORM_DATA platform_data;
};

struct DISPLAY_DESC
{
    uint32_t            display_index;
    ROTATION_MODE       rotation;
    COLOR_SPACE         color_space;
    EXTENT2D            resolution;
};

struct SURFACE_STATE
{
    DISPLAY_DESC        most_contained_display;
    OFFSET2D            offset;
    EXTENT2D            size;
};

struct SURFACE_FORMAT
{
    COLOR_SPACE         color_space;
    RESOURCE_FORMAT     format;
};

enum SWAP_CHAIN_BUFFER_FLAG : EnumT
{
      SWAP_CHAIN_BUFFER_FLAG_COPY_SRC                     = 0x1
    , SWAP_CHAIN_BUFFER_FLAG_COPY_DST                     = 0x2
    , SWAP_CHAIN_BUFFER_FLAG_SHADER_RESOURCE              = 0x4
    , SWAP_CHAIN_BUFFER_FLAG_UNORDERED_ACCESS             = 0x8
    , SWAP_CHAIN_BUFFER_FLAG_COLOR_ATTACHMENT             = 0x10
    , SWAP_CHAIN_BUFFER_FLAG_INPUT_ATTACHMENT             = 0x20
    , SWAP_CHAIN_BUFFER_FLAG_ALLOW_SIMULTANEOUS_ACCESS    = 0x40
};
using SWAP_CHAIN_BUFFER_FLAGS = EnumFlagsT;

struct SWAP_CHAIN_BUFFER_DESC
{
    uint32_t                   width;       // バックバッファの幅です。
    uint32_t                   height;      // バックバッファの高さです。
    uint32_t                   count;       // バックバッファの数です。
    TEXTURE_FORMAT_DESC        format_desc; // バックバッファのフォーマットを記述します。
    SWAP_CHAIN_BUFFER_FLAGS    flags;       // バックバッファの使用法を指定します。
};

enum SWAP_CHAIN_FLAG : EnumT
{
      SWAP_CHAIN_FLAG_NONE                             = 0x0
    , SWAP_CHAIN_FLAG_DISABLE_VERTICAL_SYNC            = 0x1 // 垂直同期を無効にすることを指定します。
    , SWAP_CHAIN_FLAG_ALLOW_DISCARD_AFTER_PRESENT      = 0x2 // プレゼントが完了したバックバッファの内容を保持する必要がないことを指定します。 
    , SWAP_CHAIN_FLAG_PROTECT_CONTENTS                 = 0x4 // TODO: SWAP_CHAIN_FLAG_PROTECT_CONTENTS
    , SWAP_CHAIN_FLAG_FULLSCREEN_EXCLUSIVE             = 0x8 // 排他フルスクリーンを有効にします。
};
using SWAP_CHAIN_FLAGS = EnumFlagsT;

enum SWAP_CHAIN_ALPHA_MODE : EnumT
{
      SWAP_CHAIN_ALPHA_MODE_DEFAULT             // inherit or opaque
    , SWAP_CHAIN_ALPHA_MODE_IGNORE              // opaque
    , SWAP_CHAIN_ALPHA_MODE_STRAIGHT            // post-multiplied
    , SWAP_CHAIN_ALPHA_MODE_PRE_MULTIPLIED
};

struct SWAP_CHAIN_DESC
{
    ISurface*                  surface;
    COLOR_SPACE                color_space;
    ROTATION_MODE              pre_roration;
    SWAP_CHAIN_BUFFER_DESC     buffer;
    SWAP_CHAIN_ALPHA_MODE      alpha_mode;
    SWAP_CHAIN_FLAGS           flags;
    uint32_t                   num_present_queues;    // present_queues配列の数です。この値が1の場合全てのバッファのプレゼント操作に同じキューが関連付けられます。
    ICommandQueue*const *      present_queues;        // バッファの対応するインデックスの要素に関連付けられるプレゼント操作に使用されるコマンドキューの配列です。バッファのインスタンスは各要素のキューと同じノードにのみ作成され、このインスタンスは、暗黙で全ての有効なノードに対して可視になります(visible_node_maskの有効なビットが全て指定されているようになります)。
};

struct FENCE_SUBMISSION
{
    uint32_t           num_fences;      // fences配列の要素数です。
    IFence*const *     fences;          // FENCE_TYPE_TIMELINEまたはFENCE_TYPE_BINARY_GPU_TO_GPUタイプのフェンスの配列へのポインタです。FENCE_TYPE_BINARY_GPU_TO_CPUタイプのフェンスは含まれていない必要があります。
    const uint64_t*    fence_values;    // FENCE_TYPE_TIMELINEがいずれかの要素に含まれていない場合でも要素数num_fencesの配列が必要です。FENCE_TYPE_BINARY_GPU_TO_GPUの場合、値は使用されません。
};

struct SWAP_CHAIN_ACQUIRE_NEXT_BUFFER_INFO
{
    IFence*        signal_fence;        // バックバッファが利用可能になったタイミングを通知するGPU_TO_GPUフェンスです。 nullptrでない場合signal_fence_to_cpuメンバもnullptrであってはなりません。
    IFence*        signal_fence_to_cpu; // バックバッファが利用可能になったタイミングを通知するGPU_TO_CPUフェンスです。 nullptrでない場合signal_fenceメンバもnullptrであってはなりません。
    uint32_t       timeout_millisec;    // CPUで待機するミリ秒単位の値です。
};

struct SWAP_CHAIN_PRESENT_INFO
{
    IFence*                 wait_fence;        // プレゼント操作を実行する前に待機するGPU_TO_GPUフェンスです。
    uint32_t                num_present_regions;
    const SCISSOR_RECT*     present_regions;
};

struct SWAP_CHAIN_HDR_METADATA
{
    // CIE 1931, CIE 15:2004 specifications (Section 7.3)
    FLOAT2 primary_red;                   // 色度座標でのマスタリングディスプレイの赤のプライマリです。 0-1 [nit]
    FLOAT2 primary_green;                 // 色度座標でのマスタリングディスプレイの緑のプライマリです。 0-1 [nit]
    FLOAT2 primary_blue;                  // 色度座標でのマスタリングディスプレイの青のプライマリです。 0-1 [nit]
    FLOAT2 white_point;                   // 色度座標でのマスタリングディスプレイのホワイトポイントです。 0-1 [nit]
    float  max_luminance;                 // マスタリングディスプレイの最大輝度です。 [nit]
    float  min_luminance;                 // マスタリングディスプレイの最小輝度です。 [nit]
    float  max_content_light_level;       // コンテンツの任意の場所で使用される最も明るいピクセルに対応する最大ニット値です。 [nit] ; SMPTE 2086 MaxCLL 
    float  max_frame_average_light_level; // コンテンツの平均輝度が最も明るいフレームの平均輝度に対応する最大ニット値です。 [nit] ; SMPTE 2086 MaxFALL
};

#pragma endregion swapchain

#pragma region device

enum COMMAND_TYPE : EnumT
{
      COMMAND_TYPE_DIRECT
    , COMMAND_TYPE_DIRECT_ONLY
    , COMMAND_TYPE_COMPUTE_ONLY
    , COMMAND_TYPE_COPY_ONLY
    , COMMAND_TYPE_VIDEO_DECODE
    , COMMAND_TYPE_VIDEO_PROCESS
    , COMMAND_TYPE_VIDEO_ENCODE

    , COMMAND_TYPE_NUM_TYPES
};

enum COMMAND_QUEUE_FLAG : EnumT
{
      COMMAND_QUEUE_FLAG_NONE                        = 0x0
    , COMMAND_QUEUE_FLAG_PRIORITY_GLOBAL_REALTIME    = 0x1
};
using COMMAND_QUEUE_FLAGS = EnumFlagsT;

enum COMMAND_QUEUE_PRIORITY : EnumT
{
      COMMAND_QUEUE_PRIORITY_DEFAULT
    , COMMAND_QUEUE_PRIORITY_MEDIUM
    , COMMAND_QUEUE_PRIORITY_HIGH
};

struct COMMAND_QUEUE_PROPERTIES
{
    COMMAND_TYPE    type;
    uint32_t        num_max_queues;
};

struct COMMAND_QUEUE_CREATE_DESC
{
    COMMAND_TYPE                       type;
    COMMAND_QUEUE_FLAGS                flags;
    uint32_t                           num_queues;
    const COMMAND_QUEUE_PRIORITY*      priorities;
    const NodeMask*                    node_masks;
};

enum DEVICE_FLAG : EnumT
{
    DEVICE_FLAG_NONE = 0x0
};
using DEVICE_FLAGS = EnumFlagsT;

struct DEVICE_DESC
{
    IDeviceAdapter*                        adapter;
    uint32_t                               num_queue_create_descs;
    const COMMAND_QUEUE_CREATE_DESC*       queue_create_descs;
    DEVICE_FLAGS                           flags;
};

#pragma endregion device

#pragma region resource

enum RESOURCE_HEAP_PROPERTY_FLAG : EnumT
{
      RESOURCE_HEAP_PROPERTY_FLAG_NONE                             = 0x0      // CPU_NOT_AVAILABLE
    , RESOURCE_HEAP_PROPERTY_FLAG_DEVICE_LOCAL                     = 0x1      // CPU_NOT_AVAILABLE
    , RESOURCE_HEAP_PROPERTY_FLAG_HOST_READABLE                    = 0x2      // CPU_WRITE_BACK   , HEAP_TYPE_READBACK
    , RESOURCE_HEAP_PROPERTY_FLAG_HOST_WRITABLE                    = 0x4      // CPU_WRITE_COMBINE, HEAP_TYPE_UPLOAD   HOST_VISIBLE = (READABLE | WRITABLE)
    , RESOURCE_HEAP_PROPERTY_FLAG_HOST_CACHED                      = 0x8      // CPU_WRITE_BACK   , HEAP_TYPE_READBACK (要:Flush,Invalidate)
    , RESOURCE_HEAP_PROPERTY_FLAG_HOST_COHERENT                    = 0x10     // CPU_WRITE_COMBINE, HEAP_TYPE_UPLOAD
    , RESOURCE_HEAP_PROPERTY_FLAG_MULTI_INSTANCE                   = 0x20     // creation_node_maskで複数のノードを指定可能です。このフラグが指定されていない場合は有効なノードを示す単一のビットのみ指定する必要があります。
    , RESOURCE_HEAP_PROPERTY_FLAG_SUBSET_ALLOCATION                = 0x40     // 各ノードへの割り当てがcreation_node_maskによって制限可能。それ以外は全てのノードで割り当てられる(依然として、任意のビットを指定することは有効です)。RESOURCE_HEAP_PROPERTY_FLAG_MULTI_INSTANCEが指定されていない場合依然としてビットは1つのみ指定可能。
    , RESOURCE_HEAP_PROPERTY_FLAG_VISIBLE_NODE_MASK                = 0x80     // ヒープの可視性をvisible_node_maskによって制限可能、それ以外は全てのビットで可視。
    , RESOURCE_HEAP_PROPERTY_FLAG_LAZILY_ALLOCATED                 = 0x100    // 
    , RESOURCE_HEAP_PROPERTY_FLAG_PROTECTED                        = 0x200    // 
    , RESOURCE_HEAP_PROPERTY_FLAG_ACCESS_GENERIC_MEMORY_READ_FIXED = 0x400    // このヒープから割り当てられたメモリへのアクセスマスクは変更出来ません。
    , RESOURCE_HEAP_PROPERTY_FLAG_ACCESS_COPY_DST_FIXED            = 0x800    // このヒープから割り当てられたメモリへのアクセスマスクは変更出来ません。
};
using RESOURCE_HEAP_PROPERTY_FLAGS = EnumFlagsT;

enum RESOURCE_HEAP_FLAG : EnumT
{
      RESOURCE_HEAP_FLAG_NONE                       = 0x0
    , RESOURCE_HEAP_FLAG_SHARED_EXPORT_TO_HANDLE    = 0x1// FLAG_SHARED
    , RESOURCE_HEAP_FLAG_SHARED_IMPORT_FROM_HANDLE  = 0x2// FLAG_SHARED
    , RESOURCE_HEAP_FLAG_SHARED_CROSS_ADAPTER       = 0x4
    , RESOURCE_HEAP_FLAG_PROTECTED                  = 0x10
};
using RESOURCE_HEAP_FLAGS = EnumFlagsT;

struct RESOURCE_HEAP_PROPERTIES
{
    uint32_t                     heap_index;
    RESOURCE_HEAP_PROPERTY_FLAGS flags;
};

struct RESOURCE_HEAP_DESC
{
    uint32_t             heap_index;
    uint64_t             size_in_bytes;
    uint64_t             alignment;
    RESOURCE_HEAP_FLAGS  flags;
    NodeMask             creation_node_mask;// ヒープのインスタンスを作成するノードのインデックスビットです。RESOURCE_HEAP_TYPE_MULTI_INSTANCEヒープの場合この複数のノードにメモリインスタンスを作成します。それ以外の場合単一のビットのみを指定する必要があります。
    NodeMask             visible_node_mask;// このヒープのインスタンスを使用できるデバイス内のノードです。RESOURCE_HEAP_PROPERTY_FLAG_VISIBLE_NODE_MASKヒープではない場合、この値はアプリケーションによって制御できず、creation_node_maskの全てのビットが指定されます。
};

struct BIND_RESOURCE_HEAP_INFO
{
    IResourceHeap*     src_heap;
    uint64_t           src_heap_offset;

    /**
     * @brief bind_node_masks配列の要素数です。
     *        src_heapがRESOURCE_HEAP_PROPERTY_FLAG_MULTI_INSTANCEを含む場合、この値はIDevice内のノード数と同じである必要があります。
     *        RESOURCE_HEAP_PROPERTY_FLAG_MULTI_INSTANCEを含まない場合、
     *        この値は0でなければならず、作成するリソース内の全インスタンスにバインドされるヒープインスタンスは、creation_node_maskで指定された単一のビットのみとなります。
    */
    uint32_t           num_bind_node_masks;

    /**
     * @brief dst_resourceの各インスタンスにバインドする、ヒープのヒープインスタンスを示す、単一のノードマスクを指定するNodeMask配列です。
     *        ここで、この配列要素に対するインデックスをiと定義し、iはデバイスの有効なノードのインデックスを示します。
     *        bind_node_masks[i]の値が(1<<i)である場合、そのリソースインスタンスがバインドするヒープインスタンスはローカルメモリにあります。
     *        それ以外の場合、ヒープのインスタンスはピアメモリにあります。
    */
    const NodeMask*    bind_node_masks;

    IResource*         dst_resource;
};

enum RESOURCE_DIMENSION : EnumT
{
      RESOURCE_DIMENSION_BUFFER
    , RESOURCE_DIMENSION_TEX1D
    , RESOURCE_DIMENSION_TEX2D
    , RESOURCE_DIMENSION_TEX3D
};

/**
 * @brief 発生しうるリソースのアクセスの形態を表現するフラグビットです。
*/
enum RESOURCE_ACCESS_FLAG : EnumT
{
      RESOURCE_ACCESS_FLAG_NONE                                     = 0x0           // D3D12_COMMON                                 0x0                                             VK_UNDEFINED

    , RESOURCE_ACCESS_FLAG_INDIRECT_ARGUMENT_READ                   = 0x1           // D3D12_INDIRECT_ARGUMENT                      INDIRECT_COMMAND_READ_BIT                       n/a
    , RESOURCE_ACCESS_FLAG_CONDITIONAL_RENDERING_READ               = 0x2           // D3D12_PREDICATION                            CONDITIONAL_RENDERING_READ_BIT_EXT              n/a

    , RESOURCE_ACCESS_FLAG_INDEX_READ                               = 0x4           // D3D12_INDEX_BUFFER                           INDEX_READ_BIT                                  n/a
    , RESOURCE_ACCESS_FLAG_VERTEX_ATTRIBUTE_READ                    = 0x8           // D3D12_VERTEX_AND_CONSTANT_BUFFER             VERTEX_ATTRIBUTE_READ_BIT                       n/a
    , RESOURCE_ACCESS_FLAG_CONSTANT_READ                            = 0x10          // D3D12_VERTEX_AND_CONSTANT_BUFFER             UNIFORM_READ_BIT                                n/a

    , RESOURCE_ACCESS_FLAG_SHADER_READ                              = 0x20          // D3D12_( | _NON_)PIXEL_SHADER_RESOURCE        SHADER_READ_BIT                                 SHADER_READ_ONLY_OPTIMAL
    , RESOURCE_ACCESS_FLAG_SHADER_WRITE                             = 0x40          // D3D12_UNORDERED_ACCESS                       SHADER_WRITE_BIT                                GENERAL

    , RESOURCE_ACCESS_FLAG_INPUT_ATTACHMENT_READ                    = 0x80          // D3D12_PIXEL_SHADER_RESOURCE                  INPUT_ATTACHMENT_READ_BIT                       SHADER_READ_ONLY_OPTIMAL

    , RESOURCE_ACCESS_FLAG_COLOR_ATTACHMENT_READ                    = 0x100         // D3D12_RENDER_TARGET                          COLOR_ATTACHMENT_READ_BIT                       COLOR_ATTACHMENT_OPTIMAL
    , RESOURCE_ACCESS_FLAG_COLOR_ATTACHMENT_WRITE                   = 0x200         // D3D12_RENDER_TARGET                          COLOR_ATTACHMENT_WRITE_BIT                      COLOR_ATTACHMENT_OPTIMAL

    , RESOURCE_ACCESS_FLAG_DEPTH_STENCIL_ATTACHMENT_READ            = 0x400         // D3D12_DEPTH_READ                             DEPTH_STENCIL_ATTACHMENT_READ_BIT               DEPTH_STENCIL_READ_ONLY_OPTIMAL , DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    , RESOURCE_ACCESS_FLAG_DEPTH_STENCIL_ATTACHMENT_WRITE           = 0x800         // D3D12_DEPTH_WRITE                            DEPTH_STENCIL_ATTACHMENT_WRITE_BIT              DEPTH_STENCIL_ATTACHMENT_OPTIMAL

    , RESOURCE_ACCESS_FLAG_COPY_READ                                = 0x1000        // D3D12_COPY_SOURCE                            TRANSFER_READ_BIT                               TRANSFER_SRC_OPTIMAL
    , RESOURCE_ACCESS_FLAG_COPY_WRITE                               = 0x2000        // D3D12_COPY_DEST                              TRANSFER_WRITE_BIT                              TRANSFER_DST_OPTIMAL

    , RESOURCE_ACCESS_FLAG_RESOLVE_READ                             = 0x4000        // D3D12_RESOLVE_SOURCE                         TRANSFER_READ_BIT                               TRANSFER_SRC_OPTIMAL
    , RESOURCE_ACCESS_FLAG_RESOLVE_WRITE                            = 0x8000        // D3D12_RESOLVE_DEST                           TRANSFER_WRITE_BIT                              TRANSFER_DST_OPTIMAL

    , RESOURCE_ACCESS_FLAG_HOST_READ                                = 0x10000       // D3D12_GENERIC_READ|COMMON                    HOST_READ_BIT                                   GENERAL , PREINITIALIZED  
    , RESOURCE_ACCESS_FLAG_HOST_WRITE                               = 0x20000       // D3D12_GENERIC_READ|COMMON|COPY_DST           HOST_WRITE_BIT                                  GENERAL , PREINITIALIZED  

    , RESOURCE_ACCESS_FLAG_GENERIC_MEMORY_READ                      = 0x40000       // D3D12_GENERIC_READ                           MEMORY_READ_BIT                                 GENERAL , PREINITIALIZED

    , RESOURCE_ACCESS_FLAG_STREAM_OUTPUT_WRITE                      = 0x80000       // D3D12_STREAM_OUTPUT                          TRANSFORM_FEEDBACK_WRITE_BIT_EXT                n/a  
    , RESOURCE_ACCESS_FLAG_STREAM_OUTPUT_COUNTER_READ               = 0x100000      // D3D12_STREAM_OUTPUT                          TRANSFORM_FEEDBACK_COUNTER_READ_BIT_EXT         n/a 
    , RESOURCE_ACCESS_FLAG_STREAM_OUTPUT_COUNTER_WRITE              = 0x200000      // D3D12_STREAM_OUTPUT                          TRANSFORM_FEEDBACK_COUNTER_WRITE_BIT_EXT        n/a  

    , RESOURCE_ACCESS_FLAG_ACCELERATION_STRUCTURE_READ              = 0x400000      // D3D12_RAYTRACING_ACCELERATION_STRUCTURE      ACCELERATION_STRUCTURE_READ_BIT_KHR             n/a
    , RESOURCE_ACCESS_FLAG_ACCELERATION_STRUCTURE_WRITE             = 0x800000      // D3D12_RAYTRACING_ACCELERATION_STRUCTURE      ACCELERATION_STRUCTURE_WRITE_BIT_KHR            n/a

    , RESOURCE_ACCESS_FLAG_SHADING_RATE_IMAGE_READ                  = 0x1000000     // D3D12_SHADING_RATE_SOURCE                    SHADING_RATE_IMAGE_READ_BIT_NV                  SHADING_RATE_OPTIMAL_NV

    , RESOURCE_ACCESS_FLAG_PRESENT                                  = RESOURCE_ACCESS_FLAG_NONE //                                  0x0                                             PRESENT_KHR


    //, RESOURCE_ACCESS_FLAG_MEMORY_WRITE                         = 0x00010000  // D3D12_GENERIC_READ                                                                            GENERAL , PREINITIALIZED  
    //, RESOURCE_ACCESS_FLAG_FRAGMENT_DENSITY_MAP_READ            = 0x01000000  // D3D12_SHADING_RATE_SOURCE                                                                     FRAGMENT_DENSITY_MAP_OPTIMAL_EXT
    //, RESOURCE_ACCESS_FLAG_COMMAND_PREPROCESS_READ              = 0x00020000                                                  COMMAND_PREPROCESS_READ_BIT_NV 
    //, RESOURCE_ACCESS_FLAG_COMMAND_PREPROCESS_WRITE             = 0x00040000                                                  COMMAND_PREPROCESS_WRITE_BIT_NV 
};
using RESOURCE_ACCESS_FLAGS = EnumFlagsT;

enum RESOURCE_STATE : EnumT
{
                                                            // D3D12_RESOURCE_STATE                         VkAccessFlagBits                                VkImageLayout
      RESOURCE_STATE_UNDEFINED                              // D3D12_COMMON                                 0x0                                             UNDEFINED

    , RESOURCE_STATE_INDIRECT_ARGUMENT_READ                 // D3D12_INDIRECT_ARGUMENT                      INDIRECT_COMMAND_READ_BIT                       n/a
    , RESOURCE_STATE_CONDITIONAL_RENDERING_READ             // D3D12_PREDICATION                            CONDITIONAL_RENDERING_READ_BIT_EXT              n/a

    , RESOURCE_STATE_INDEX_READ                             // D3D12_INDEX_BUFFER                           INDEX_READ_BIT                                  n/a
    , RESOURCE_STATE_VERTEX_ATTRIBUTE_READ                  // D3D12_VERTEX_AND_CONSTANT_BUFFER             VERTEX_ATTRIBUTE_READ_BIT                       n/a
    , RESOURCE_STATE_CONSTANT_READ                          // D3D12_VERTEX_AND_CONSTANT_BUFFER             UNIFORM_READ_BIT                                n/a

    , RESOURCE_STATE_SHADER_READ                            // D3D12_{PIXEL|NON_PIXEL}_SHADER_RESOURCE      SHADER_READ_BIT                                 SHADER_READ_ONLY_OPTIMAL
    , RESOURCE_STATE_SHADER_WRITE                           // D3D12_UNORDERED_ACCESS                       SHADER_WRITE_BIT                                GENERAL
    , RESOURCE_STATE_SHADER_READ_WRITE                      // D3D12_UNORDERED_ACCESS                       SHADER_{READ|WRITE}_BIT                         GENERAL

    , RESOURCE_STATE_INPUT_ATTACHMENT_READ                  // D3D12_PIXEL_SHADER_RESOURCE                  INPUT_ATTACHMENT_READ_BIT                       SHADER_READ_ONLY_OPTIMAL

    , RESOURCE_STATE_COLOR_ATTACHMENT_READ                  // D3D12_RENDER_TARGET                          COLOR_ATTACHMENT_READ_BIT                       COLOR_ATTACHMENT_OPTIMAL
    , RESOURCE_STATE_COLOR_ATTACHMENT_WRITE                 // D3D12_RENDER_TARGET                          COLOR_ATTACHMENT_WRITE_BIT                      COLOR_ATTACHMENT_OPTIMAL
    , RESOURCE_STATE_COLOR_ATTACHMENT_READ_WRITE            // D3D12_RENDER_TARGET                          COLOR_ATTACHMENT_{READ|WRITE}_BIT               COLOR_ATTACHMENT_OPTIMAL

    , RESOURCE_STATE_DEPTH_STENCIL_ATTACHMENT_READ          // D3D12_DEPTH_READ                             DEPTH_STENCIL_ATTACHMENT_READ_BIT               DEPTH/STENCIL_READ_ONLY_OPTIMAL
    , RESOURCE_STATE_DEPTH_STENCIL_ATTACHMENT_WRITE         // D3D12_DEPTH_WRITE                            DEPTH_STENCIL_ATTACHMENT_WRITE_BIT              DEPTH/STENCIL_ATTACHMENT_OPTIMAL
    , RESOURCE_STATE_DEPTH_STENCIL_ATTACHMENT_READ_WRITE    // D3D12_DEPTH_WRITE                            DEPTH_STENCIL_ATTACHMENT_WRITE_BIT              DEPTH/STENCIL_ATTACHMENT_OPTIMAL

    , RESOURCE_STATE_RESOLVE_COLOR_ATTACHMENT_WRITE         // D3D12_RESOLVE_DEST                           COLOR_ATTACHMENT_WRITE_BIT                      COLOR_ATTACHMENT_OPTIMAL
    , RESOURCE_STATE_RESOLVE_DEPTH_STENCIL_ATTACHMENT_WRITE // D3D12_RESOLVE_DEST                           DEPTH_STENCIL_ATTACHMENT_WRITE_BIT              DEPTH/STENCIL_ATTACHMENT_OPTIMAL

    , RESOURCE_STATE_COPY_SRC_READ                          // D3D12_COPY_SOURCE                            TRANSFER_READ_BIT                               TRANSFER_SRC_OPTIMAL
    , RESOURCE_STATE_COPY_DST_WRITE                         // D3D12_COPY_DEST                              TRANSFER_WRITE_BIT                              TRANSFER_DST_OPTIMAL

    , RESOURCE_STATE_RESOLVE_SRC_READ                       // D3D12_RESOLVE_SOURCE                         TRANSFER_READ_BIT                               TRANSFER_SRC_OPTIMAL
    , RESOURCE_STATE_RESOLVE_DST_WRITE                      // D3D12_RESOLVE_DEST                           TRANSFER_WRITE_BIT                              TRANSFER_DST_OPTIMAL

    , RESOURCE_STATE_HOST_READ                              // D3D12_GENERIC_READ|COMMON                    HOST_READ_BIT                                   GENERAL , PREINITIALIZED  
    , RESOURCE_STATE_HOST_WRITE                             // D3D12_GENERIC_READ|COMMON,COPY_DST           HOST_WRITE_BIT                                  GENERAL , PREINITIALIZED  
    , RESOURCE_STATE_HOST_READ_WRITE                        // D3D12_GENERIC_READ|COMMON,COPY_DST           HOST_{READ|WRITE}_BIT                           GENERAL , PREINITIALIZED  

    // D3D12: 深度ステンシルフォーマットの場合DEPTH_READも必須
    , RESOURCE_STATE_GENERIC_MEMORY_READ                    // D3D12_GENERIC_READ                           MEMORY_READ_BIT                                 GENERAL , PREINITIALIZED

    , RESOURCE_STATE_STREAM_OUTPUT_WRITE                    // D3D12_STREAM_OUTPUT                          TRANSFORM_FEEDBACK_WRITE_BIT_EXT                n/a
    , RESOURCE_STATE_STREAM_OUTPUT_COUNTER_READ             // D3D12_STREAM_OUTPUT                          TRANSFORM_FEEDBACK_COUNTER_READ_BIT_EXT         n/a
    , RESOURCE_STATE_STREAM_OUTPUT_COUNTER_WRITE            // D3D12_STREAM_OUTPUT                          TRANSFORM_FEEDBACK_COUNTER_WRITE_BIT_EXT        n/a
    , RESOURCE_STATE_STREAM_OUTPUT_COUNTER_READ_WRITE       // D3D12_STREAM_OUTPUT                          TRANSFORM_FEEDBACK_COUNTER_{READ|WRITE}_BIT_EXT n/a

    , RESOURCE_STATE_ACCELERATION_STRUCTURE_READ            // D3D12_RAYTRACING_ACCELERATION_STRUCTURE      ACCELERATION_STRUCTURE_READ_BIT_KHR             n/a
    , RESOURCE_STATE_ACCELERATION_STRUCTURE_WRITE           // D3D12_RAYTRACING_ACCELERATION_STRUCTURE      ACCELERATION_STRUCTURE_WRITE_BIT_KHR            n/a
    , RESOURCE_STATE_ACCELERATION_STRUCTURE_READ_WRITE      // D3D12_RAYTRACING_ACCELERATION_STRUCTURE      ACCELERATION_STRUCTURE_{READ|WRITE}_BIT_KHR     n/a

    , RESOURCE_STATE_SHADING_RATE_IMAGE_READ                // D3D12_SHADING_RATE_SOURCE                    SHADING_RATE_IMAGE_READ_BIT_NV                  SHADING_RATE_OPTIMAL_NV

    , RESOURCE_STATE_PRESENT                                // D3D12_PRESENT                                0x0                                             PRESENT_KHR

    //, RESOURCE_STATE_MEMORY_WRITE                         // D3D12_GENERIC_READ                           MEMORY_WRITE_BIT                                GENERAL
    //, RESOURCE_STATE_FRAGMENT_DENSITY_MAP_READ            // D3D12_SHADING_RATE_SOURCE                    FRAGMENT_DENSITY_MAP_READ_BIT                   FRAGMENT_DENSITY_MAP_OPTIMAL_EXT
    //, RESOURCE_STATE_COMMAND_PREPROCESS_READ                                                              COMMAND_PREPROCESS_READ_BIT_NV 
    //, RESOURCE_STATE_COMMAND_PREPROCESS_WRITE                                                             COMMAND_PREPROCESS_WRITE_BIT_NV 
};

/* 
VK_ACCESS_INDIRECT_COMMAND_READ_BIT                     PIPELINE_STAGE_DRAW_INDIRECT_BIT
VK_ACCESS_INDEX_READ_BIT                                PIPELINE_STAGE_VERTEX_INPUT_BIT
VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT                     PIPELINE_STAGE_VERTEX_INPUT_BIT

VK_ACCESS_UNIFORM_READ_BIT                              PIPELINE_STAGE_TASK_SHADER_BIT_NV, 
                                                        PIPELINE_STAGE_MESH_SHADER_BIT_NV, 
                                                        PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR, 
                                                        PIPELINE_STAGE_VERTEX_SHADER_BIT, 
                                                        PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT, 
                                                        PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT, 
                                                        PIPELINE_STAGE_GEOMETRY_SHADER_BIT, 
                                                        PIPELINE_STAGE_FRAGMENT_SHADER_BIT, or 
                                                        PIPELINE_STAGE_COMPUTE_SHADER_BIT

VK_ACCESS_SHADER_READ_BIT                               PIPELINE_STAGE_TASK_SHADER_BIT_NV, 
                                                        PIPELINE_STAGE_MESH_SHADER_BIT_NV, 
                                                        PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR, 
                                                        PIPELINE_STAGE_VERTEX_SHADER_BIT, 
                                                        PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT, 
                                                        PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT, 
                                                        PIPELINE_STAGE_GEOMETRY_SHADER_BIT, 
                                                        PIPELINE_STAGE_FRAGMENT_SHADER_BIT, or 
                                                        PIPELINE_STAGE_COMPUTE_SHADER_BIT

VK_ACCESS_SHADER_WRITE_BIT                              PIPELINE_STAGE_TASK_SHADER_BIT_NV, 
                                                        PIPELINE_STAGE_MESH_SHADER_BIT_NV, 
                                                        PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR, 
                                                        PIPELINE_STAGE_VERTEX_SHADER_BIT, 
                                                        PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT, 
                                                        PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT, 
                                                        PIPELINE_STAGE_GEOMETRY_SHADER_BIT, 
                                                        PIPELINE_STAGE_FRAGMENT_SHADER_BIT, or 
                                                        PIPELINE_STAGE_COMPUTE_SHADER_BIT

VK_ACCESS_INPUT_ATTACHMENT_READ_BIT                     PIPELINE_STAGE_FRAGMENT_SHADER_BIT
VK_ACCESS_COLOR_ATTACHMENT_READ_BIT                     PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT                    PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT             PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, or PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT
VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT            PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, or PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT
VK_ACCESS_TRANSFER_READ_BIT                             PIPELINE_STAGE_TRANSFER_BIT
VK_ACCESS_TRANSFER_WRITE_BIT                            PIPELINE_STAGE_TRANSFER_BIT
VK_ACCESS_HOST_READ_BIT                                 PIPELINE_STAGE_HOST_BIT
VK_ACCESS_HOST_WRITE_BIT                                PIPELINE_STAGE_HOST_BIT
VK_ACCESS_MEMORY_READ_BIT                               Any
VK_ACCESS_MEMORY_WRITE_BIT                              Any
VK_ACCESS_COLOR_ATTACHMENT_READ_NONCOHERENT_BIT_EXT     PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
VK_ACCESS_COMMAND_PREPROCESS_READ_BIT_NV                PIPELINE_STAGE_COMMAND_PREPROCESS_BIT_NV
VK_ACCESS_COMMAND_PREPROCESS_WRITE_BIT_NV               PIPELINE_STAGE_COMMAND_PREPROCESS_BIT_NV
VK_ACCESS_CONDITIONAL_RENDERING_READ_BIT_EXT            PIPELINE_STAGE_CONDITIONAL_RENDERING_BIT_EXT
VK_ACCESS_SHADING_RATE_IMAGE_READ_BIT_NV                PIPELINE_STAGE_SHADING_RATE_IMAGE_BIT_NV
VK_ACCESS_TRANSFORM_FEEDBACK_WRITE_BIT_EXT              PIPELINE_STAGE_TRANSFORM_FEEDBACK_BIT_EXT
VK_ACCESS_TRANSFORM_FEEDBACK_COUNTER_WRITE_BIT_EXT      PIPELINE_STAGE_TRANSFORM_FEEDBACK_BIT_EXT
VK_ACCESS_TRANSFORM_FEEDBACK_COUNTER_READ_BIT_EXT       PIPELINE_STAGE_DRAW_INDIRECT_BIT

VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR           PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, or PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR 
VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR          PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR
VK_ACCESS_FRAGMENT_DENSITY_MAP_READ_BIT_EXT             PIPELINE_STAGE_FRAGMENT_DENSITY_PROCESS_BIT_EXT
*/

/*
VK_IMAGE_LAYOUT_UNDEFINED
VK_IMAGE_LAYOUT_GENERAL
VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL
VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
VK_IMAGE_LAYOUT_PREINITIALIZED
VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL
VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL
VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL
VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL
VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL
VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL
VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR
VK_IMAGE_LAYOUT_SHADING_RATE_OPTIMAL_NV
VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT
*/

enum PIPELINE_STAGE_FLAG : EnumT
{
      PIPELINE_STAGE_FLAG_NONE                              = 0x0
    , PIPELINE_STAGE_FLAG_TOP_OF_PIPE                       = 0x1
    , PIPELINE_STAGE_FLAG_DRAW_INDIRECT                     = 0x2
    , PIPELINE_STAGE_FLAG_INPUT_ASSEMBLER                   = 0x4       // VERTEX_INPUT
    , PIPELINE_STAGE_FLAG_VERTEX_SHADER                     = 0x8
    , PIPELINE_STAGE_FLAG_HULL_SHADER                       = 0x10      // TESSELLATION_CONTROL_SHADER
    , PIPELINE_STAGE_FLAG_DOMAIN_SHADER                     = 0x20      // TESSELLATION_EVALUATION_SHADER
    , PIPELINE_STAGE_FLAG_GEOMETRY_SHADER                   = 0x40

    // BY_REGION
    , PIPELINE_STAGE_FLAG_PIXEL_SHADER                      = 0x80
    , PIPELINE_STAGE_FLAG_EARLY_DEPTH_STENCIL_TESTS         = 0x100     // EARLY_FRAGMENT_TESTS
    , PIPELINE_STAGE_FLAG_LATE_FRAGMENT_TESTS               = 0x200
    , PIPELINE_STAGE_FLAG_COLOR_ATTACHMENT_OUTPUT           = 0x400     // レンダーパス内でのresolve

    , PIPELINE_STAGE_FLAG_COMPUTE_SHADER                    = 0x800
    , PIPELINE_STAGE_FLAG_COPY_RESOLVE                      = 0x1000    // TRANSFER copy resolve dst src  レンダーパス外のみ

    , PIPELINE_STAGE_FLAG_BOTTOM_OF_PIPE                    = 0x2000

    , PIPELINE_STAGE_FLAG_HOST                              = 0x4000    // GENERIC_READ

    , PIPELINE_STAGE_FLAG_ALL_GRAPHICS                      = 0x8000
    , PIPELINE_STAGE_FLAG_ALL_COMMANDS                      = 0x10000

    , PIPELINE_STAGE_FLAG_STREAM_OUTPUT                     = 0x20000   // TRANSFORM_FEEDBACK
    , PIPELINE_STAGE_FLAG_CONDITIONAL_RENDERING             = 0x40000
    , PIPELINE_STAGE_FLAG_SHADING_RATE_IMAGE                = 0x80000   // SHADING_RATE_SOURCE
    , PIPELINE_STAGE_FLAG_RAY_TRACING_SHADER                = 0x100000 
    , PIPELINE_STAGE_FLAG_ACCELERATION_STRUCTURE_BUILD      = 0x200000
    , PIPELINE_STAGE_FLAG_TASK_SHADER                       = 0x400000  // AMPLIFICATION
    , PIPELINE_STAGE_FLAG_MESH_SHADER                       = 0x800000 

    //, PIPELINE_STAGE_FLAG_FRAGMENT_DENSITY_PROCESS          = 0x1000000
    //, PIPELINE_STAGE_FLAG_COMMAND_PROCESS                   = 0x80000
};
using PIPELINE_STAGE_FLAGS = EnumFlagsT;

enum TEXTURE_LAYOUT : EnumT
{
      TEXTURE_LAYOUT_UNKNOWN    
    , TEXTURE_LAYOUT_ROW_MAJOR
};

enum BUFFER_CREATE_FLAG : EnumT
{
    BUFFER_CREATE_FLAG_NONE = 0x0
};
using BUFFER_CREATE_FLAGS = EnumFlagsT;

enum TEXTURE_CREATE_FLAG : EnumT
{
      TEXTURE_CREATE_FLAG_NONE                              = 0x0
    , TEXTURE_CREATE_FLAG_ALIAS                             = 0x1
    , TEXTURE_CREATE_FLAG_2D_AS_CUBE_COMPATIBLE             = 0x2
    , TEXTURE_CREATE_FLAG_3D_AS_2D_ARRAY_COMPATIBLE         = 0x4
    , TEXTURE_CREATE_FLAG_BLOCK_TEXEL_VIEW_COMPATIBLE       = 0x8// TODO: 圧縮フォーマット
    , TEXTURE_CREATE_FLAG_SAMPLE_LOCATIONS_COMPATIBLE_DEPTH = 0x10
    , TEXTURE_CREATE_FLAG_SUBSAMPLED                        = 0x20
    //, TEXTURE_CREATE_FLAG_SPLIT_INSTANCE_BIND_REGIONS     = 0x40
};
using TEXTURE_CREATE_FLAGS = EnumFlagsT;

enum BUFFER_USAGE_FLAG : EnumT
{
      BUFFER_USAGE_FLAG_NONE                            = 0x0
    , BUFFER_USAGE_FLAG_COPY_SRC                        = 0x1       // このフラグは内部APIによる制約が無く、アプリケーションに影響する動作の変化が無い場合暗黙的に有効になります。
    , BUFFER_USAGE_FLAG_COPY_DST                        = 0x2       // このフラグは内部APIによる制約が無く、アプリケーションに影響する動作の変化が無い場合暗黙的に有効になります。
    , BUFFER_USAGE_FLAG_CONSTANT_BUFFER                 = 0x4       // このフラグは内部APIによる制約が無く、アプリケーションに影響する動作の変化が無い場合暗黙的に有効になります。
    , BUFFER_USAGE_FLAG_SHADER_RESOURCE_BUFFER          = 0x8       // 読み取り専用のシェーダリソースビューの作成を許可します。allow uniform
    , BUFFER_USAGE_FLAG_UNORDERED_ACCESS_BUFFER         = 0x10      // 読み取り/書き込みの順不同アクセスビューの作成を許可します。allow storage
    , BUFFER_USAGE_FLAG_STRUCTURED_BYTEADDRESS_TBUFFER  = 0x20      // シェーダー内オブジェクト(RW)StructuredBuffer, (RW)ByteAddressBuffer, tbuffer/TextureBufferの使用を許可します。 set uniform or storage BUFFER_USAGE_FLAG_ SHADER_RESOURCE または UNORDERED_ACCESS_BUFFERが含まれている必要があります。 このフラグは内部APIによる制約が無い場合暗黙的に有効になります。
    , BUFFER_USAGE_FLAG_TYPED_SHADER_RESOURCE_BUFFER    = 0x40      // 型付けされたバッファを読み取り専用のシェーダリソースビューでの使用を許可します。 set uniform_texel BUFFER_USAGE_FLAG_SHADER_RESOURCE_BUFFERが含まれている必要があります。 このフラグは内部APIによる制約が無く、アプリケーションに影響する動作の変化が無い場合暗黙的に有効になります。
    , BUFFER_USAGE_FLAG_TYPED_UNORDERED_ACCESS_BUFFER   = 0x80      // 型付けされたバッファを読み取り/書き込みの順不同アクセスビューでの使用を許可します。 set storage_texel BUFFER_USAGE_FLAG_UNORDERED_ACCESS_BUFFERが含まれている必要があります。 このフラグは内部APIによる制約が無く、アプリケーションに影響する動作の変化が無い場合暗黙的に有効になります。
    , BUFFER_USAGE_FLAG_INDEX_BUFFER                    = 0x100     // このフラグは内部APIによる制約が無く、アプリケーションに影響する動作の変化が無い場合暗黙的に有効になります。
    , BUFFER_USAGE_FLAG_VERTEX_BUFFER                   = 0x200     // このフラグは内部APIによる制約が無く、アプリケーションに影響する動作の変化が無い場合暗黙的に有効になります。
    , BUFFER_USAGE_FLAG_INDIRECT_BUFFER                 = 0x400     // このフラグは内部APIによる制約が無く、アプリケーションに影響する動作の変化が無い場合暗黙的に有効になります。
    , BUFFER_USAGE_FLAG_STREAM_OUTPUT_BUFFER            = 0x800     // このフラグは内部APIによる制約が無く、アプリケーションに影響する動作の変化が無い場合暗黙的に有効になります。
    , BUFFER_USAGE_FLAG_STREAM_OUTPUT_COUNTER_BUFFER    = 0x1000    // このフラグは内部APIによる制約が無く、アプリケーションに影響する動作の変化が無い場合暗黙的に有効になります。
    , BUFFER_USAGE_FLAG_CONDITIONAL_RENDERING           = 0x2000    // このフラグは内部APIによる制約が無く、アプリケーションに影響する動作の変化が無い場合暗黙的に有効になります。
    , BUFFER_USAGE_FLAG_RAY_TRACING                     = 0x4000    // このリソースのアクセスマスクはRESOURCE_ACCESS_FLAG_ACCELERATION_STRUCTURE_READ/WRITEのみ設定可能であることを指定します。
};
using BUFFER_USAGE_FLAGS = EnumFlagsT;

enum TEXTURE_USAGE_FLAG : EnumT
{
      TEXTURE_USAGE_FLAG_NONE                     = 0x0
    , TEXTURE_USAGE_FLAG_COPY_SRC                 = 0x1
    , TEXTURE_USAGE_FLAG_COPY_DST                 = 0x2
    , TEXTURE_USAGE_FLAG_SHADER_RESOURCE          = 0x4
    , TEXTURE_USAGE_FLAG_UNORDERED_ACCESS         = 0x8
    , TEXTURE_USAGE_FLAG_COLOR_ATTACHMENT         = 0x10
    , TEXTURE_USAGE_FLAG_DEPTH_STENCIL_ATTACHMENT = 0x20
    , TEXTURE_USAGE_FLAG_TRANSIENT_ATTACHMENT     = 0x40
    , TEXTURE_USAGE_FLAG_INPUT_ATTACHMENT         = 0x80
    , TEXTURE_USAGE_FLAG_SHADING_RATE_IMAGE       = 0x100
};
using TEXTURE_USAGE_FLAGS = EnumFlagsT;

enum RESOURCE_FLAG : EnumT
{
      RESOURCE_FLAG_NONE                             = 0x0
    , RESOURCE_FLAG_ALLOW_CROSS_ADAPTER              = 0x1
    , RESOURCE_FLAG_ALLOW_SIMULTANEOUS_ACCESS        = 0x2  // マルチキューから同時アクセス可能
    , RESOURCE_FLAG_PROTECTED                        = 0x4
    , RESOURCE_FLAG_ACCESS_GENERIC_MEMORY_READ_FIXED = 0x8  // このリソースのアクセスマスクが不変であることを指定します。
    , RESOURCE_FLAG_ACCESS_COPY_DST_FIXED            = 0x10 // このリソースのアクセスマスクが不変であることを指定します。
};
using RESOURCE_FLAGS = EnumFlagsT;

struct CLEAR_DEPTH_STENCIL_VALUE
{
    float       depth;
    uint32_t    stencil;
};

struct CLEAR_RENDER_TARGET_VALUE
{
    union
    {
        FLOAT4 float4;
        INT4   sint4;
        UINT4  uint4;
    };
};

struct CLEAR_VALUE
{
    union
    {
        CLEAR_RENDER_TARGET_VALUE color;
        CLEAR_DEPTH_STENCIL_VALUE depth_stencil;
    };
};

struct BUFFER_DESC
{
    uint64_t               size_in_bytes;
    BUFFER_CREATE_FLAGS    flags;
    BUFFER_USAGE_FLAGS     usage;
};

struct TEXTURE_DESC
{
    EXTENT3D             extent;    // RESOURCE_DIMENSION_TEX3D以外の場合.depthは1である必要があります。
    uint32_t             array_size;// RESOURCE_DIMENSION_TEX3Dの場合1である必要があります。
    uint32_t             mip_levels;
    uint32_t             sample_count;
    TEXTURE_FORMAT_DESC  format_desc;
    TEXTURE_LAYOUT       layout;
    const CLEAR_VALUE*   optimized_clear_value;
    TEXTURE_CREATE_FLAGS flags;
    TEXTURE_USAGE_FLAGS  usage;
};

struct RESOURCE_DESC
{
    RESOURCE_DIMENSION dimension;
    union
    {
        BUFFER_DESC  buffer;
        TEXTURE_DESC texture;
    };
    RESOURCE_FLAGS flags;
};

struct COMMITTED_RESOURCE_DESC
{
    uint32_t               heap_index;
    RESOURCE_HEAP_FLAGS    heap_flags;
    NodeMask               creation_node_mask;
    NodeMask               visible_node_mask;    
    RESOURCE_DESC          resource_desc;    
    uint32_t               num_bind_node_masks;// bind_node_masks配列の要素数です。内部で作成されるheap_indexヒープがRESOURCE_HEAP_PROPERTY_FLAG_MULTI_INSTANCEを含む場合、この値はIDevice内のノード数と同じである必要があります。RESOURCE_HEAP_PROPERTY_FLAG_MULTI_INSTANCEを含まない場合、この値は0でなければならず、作成するリソース内の全インスタンスにバインドされるヒープインスタンスは、creation_node_maskで指定された単一のビットのみとなります。
    const NodeMask*        bind_node_masks;// 作成されるリソースの各インスタンスにバインドする、ヒープのヒープインスタンスを示す、単一のノードマスクを指定するNodeMask配列です。ここで、この配列要素に対するインデックスをiと定義し、iはデバイスの有効なノードのインデックスを示します。bind_node_masks[i]の値が(1<<i)である場合、そのリソースインスタンスがバインドするヒープインスタンスはローカルメモリにあります。それ以外の場合、ヒープのインスタンスはピアメモリにあります。
};

enum RESOLVE_MODE : EnumT
{
      RESOLVE_MODE_DECOMPRESS
    , RESOLVE_MODE_MIN
    , RESOLVE_MODE_MAX
    , RESOLVE_MODE_AVERAGE
};

enum TEXTURE_ASPECT_FLAG : EnumT
{
      TEXTURE_ASPECT_FLAG_NONE      = 0x0
    , TEXTURE_ASPECT_FLAG_COLOR     = 0x1
    , TEXTURE_ASPECT_FLAG_DEPTH     = 0x2
    , TEXTURE_ASPECT_FLAG_STENCIL   = 0x4
    , TEXTURE_ASPECT_FLAG_METADATA  = 0x8
    , TEXTURE_ASPECT_FLAG_PLANE_0   = 0x10
    , TEXTURE_ASPECT_FLAG_PLANE_1   = 0x20
    , TEXTURE_ASPECT_FLAG_PLANE_2   = 0x40
    , TEXTURE_ASPECT_FLAG_PLANE_3   = 0x80
};
using TEXTURE_ASPECT_FLAGS = EnumFlagsT;

struct SUBRESOURCE_OFFSET
{
    TEXTURE_ASPECT_FLAGS    aspect;     // サブリソースの解釈方法を指定します。深度ステンシルフォーマットの場合、DEPTH, STENCILの単一ビット、あるいはその両方を指定します。 マルチプラナーフォーマットの場合、アクセスするプレーン番号の単一のビットを指定します。
    uint32_t                mip_slice;  // [0,SUBRESOURCE_DESC::num_mips)までの、アクセスするミップのオフセットです。
    uint32_t                array_slice;// [0,SUBRESOURCE_DESC::array_size)までの、アクセスする配列要素のオフセットです。
};

struct SUBRESOURCE_RANGE
{
    SUBRESOURCE_OFFSET        offset;        // アクセスが開始されるサブリソースのオフセットです。
    uint32_t                  mip_levels;    // offsetからのミップ数(範囲)を指定します。[offset.mip_slice, offset.mip_slice + mip_levels) またはB3D_USE_REMAINING_MIP_LEVELS
    uint32_t                  array_size;    // offsetからの配列要素数(範囲)を指定します。[offset.array_slice, offset.array_slice + array_size) またはB3D_USE_REMAINING_ARRAY_SIZES
};

#pragma endregion resource

#pragma region tiled resource

enum TILED_RESOURCE_FORMAT_FLAG : EnumT
{
      TILED_RESOURCE_FORMAT_FLAG_NONE                  = 0x0
    , TILED_RESOURCE_FORMAT_FLAG_SINGLE_MIPTAIL        = 0x1 // 画像配列の各要素毎のミップテイルが、1つのミップテイル領域に集約されることを示します。このフラグが指定されている場合、全てのミップテイルを一度にバインドする必要があります。
    , TILED_RESOURCE_FORMAT_FLAG_ALIGNED_MIP_SIZE      = 0x2 // 
//  , TILED_RESOURCE_FORMAT_FLAG_NONSTANDARD_TILE_SIZE = 0x4 // 
};
using TILED_RESOURCE_FORMAT_FLAGS = EnumFlagsT;

struct TILE_SHAPE
{
    uint32_t width_in_texels;
    uint32_t height_in_texels;
    uint32_t depth_in_texels;
};

struct TILED_RESOURCE_FORMAT_PROPERTIES
{
    TEXTURE_ASPECT_FLAGS           aspect;
    TILE_SHAPE                     tile_shape;
    TILED_RESOURCE_FORMAT_FLAGS    flags;
};

struct TILED_RESOURCE_MIPTAIL_PROPERTIES
{
    bool        is_required;        // 指定のリソースにミップテイルが必要かどうかを示します。ミップテイルが要求されない場合、この構造体の以下のメンバは0で埋められます。
    uint32_t    first_mip_slice;    // 画像配列の各要素毎の、ミップテイルが開始するミップレベルです。
    uint64_t    size;               // 画像配列の各要素毎の、ミップテイルに必要なメモリのサイズです。
    uint64_t    offset;             // 画像配列の各要素毎の、ミップテイルが開始するメモリのオフセットです。
    uint64_t    stride;             // 次の画像配列要素のミップテイルまでの間のメモリのサイズです。(MIPTAIL_REGION::offset_in_bytes = mip_tail.offset + (mip_tail.stride * array))
};

struct TILED_RESOURCE_ALLOCATION_INFO
{
    TILED_RESOURCE_FORMAT_PROPERTIES    format_properties;
    TILED_RESOURCE_MIPTAIL_PROPERTIES   mip_tail;
};

struct TILE_REGION_OFFSET
{
    uint32_t x; // VkSparseImageMemoryBind::offset::x
    uint32_t y; // VkSparseImageMemoryBind::offset::y
    uint32_t z; // VkSparseImageMemoryBind::offset::z
};

struct TILE_REGION_SIZE
{
    uint32_t width; // VkSparseImageMemoryBind::extent::width
    uint32_t height;// VkSparseImageMemoryBind::extent::height
    uint32_t depth; // VkSparseImageMemoryBind::extent::depth
};

struct MIPTAIL_REGION
{
    SUBRESOURCE_OFFSET    subresource;// ミップテイルまたはメタデータが開始される、ミップスライス、バインドするミップテイルの配列スライスを指定します。aspectにはTILED_RESOURCE_FORMAT_PROPERTIESからそれぞれ取得される値が指定されている必要があります。

    /**
     * @brief バイト単位での領域[base, base + TILED_RESOURCE_ALLOCATION_INFO::mip_tail.size)における、baseの値です。
     *        baseの値は、TILED_RESOURCE_ALLOCATION_INFOからそれぞれ取得される値を使用して mip_tail.offset + (mip_tail.stride * n) で決定されます。nはsubresource.array_sliceです。
     *        TILED_RESOURCE_ALLOCATION_INFO::format_propertiesにTILED_RESOURCE_FORMAT_FLAG_SINGLE_MIPTAILが指定されている場合、nを0に指定し、全ての配列のミップテイル要素は一度にバインドする必要があります。
     *        offset_in_bytesはアプリケーションに対して不透明であり、リソース内のメモリ配置を示しません。内部実装により解釈されます。
    */
    uint64_t            offset_in_bytes;
    uint64_t            size_in_bytes;  // バイト単位でのoffset_in_bytesからのサイズです(mip_tail.size)。
};

struct TILE_REGION
{
    SUBRESOURCE_OFFSET    subresource;
    TILE_REGION_OFFSET    tile_offset;  // タイル単位でのオフセットです。
    TILE_REGION_SIZE      tile_size;    // タイル単位でのオフセットからの領域を指定するサイズです。
};

enum TILED_RESOURCE_BIND_REGION_FLAG : EnumT
{
      TILED_RESOURCE_BIND_REGION_FLAG_NONE            = 0x0
    , TILED_RESOURCE_BIND_REGION_FLAG_BIND_TO_NULL    = 0x1 // ヒープのバインドを解除することを指定します。
    , TILED_RESOURCE_BIND_REGION_FLAG_MIPTAIL         = 0x2 // バインドする領域がミップテイルであることを指定します。
    , TILED_RESOURCE_BIND_REGION_FLAG_METADATA        = 0x4 // バインドする領域がメタデータであることを指定します。
};
using TILED_RESOURCE_BIND_REGION_FLAGS = EnumFlagsT;

struct TILED_RESOURCE_BIND_REGION
{
    union
    {
        TILE_REGION                     dst_region;
        MIPTAIL_REGION                  dst_miptail_region;// flagsにMIPTAILまたはMETADATAを指定する場合、この構造を使用する必要があります。
    };

    uint32_t                            heap_tile_offset; // タイル単位でのヒープのメモリオフセットです。
    TILED_RESOURCE_BIND_REGION_FLAGS    flags;
};

struct TILED_RESOURCE_BIND
{
    IResource*                            dst_resource;
    uint32_t                              num_regions;
    IResourceHeap*const *                 src_heaps;// regions配列の各要素に対応する、dst_resourceとバインドするヒープの配列です。TILED_RESOURCE_BIND_INFO::heaps_bind_node_maskのビットにインスタンスが作成されている必要があります。
    const TILED_RESOURCE_BIND_REGION*     regions;
};

struct TILED_RESOURCE_BIND_INFO
{
    FENCE_SUBMISSION              wait_fence;
    uint32_t                      num_binds;
    const TILED_RESOURCE_BIND*    binds;
    NodeMask                      resources_bind_node_mask; // binds配列のすべての要素のリソースにおける、バインドを実行するリソースインスタンスの単一のビットを指定するノードマスクです。RESOURCE_HEAP_PROPERTY_FLAG_VISIBLE_NODE_MASKを含む場合、この値は無視され、バインドするリソースの各インスタンスは、visible_node_maskの有効なビットのみとなります。この場合、一回のバインド操作ですべての有効なリソースインスタンスにバインドされます。
    NodeMask                      heaps_bind_node_mask;     // binds配列のすべての要素のヒープにおける、バインド先となるヒープインスタンスの単一のビットを指定するノードマスクです。RESOURCE_HEAP_PROPERTY_FLAG_VISIBLE_NODE_MASKを含む場合、この値は無視され、バインド先のヒープインスタンスはcreation_node_maskに指定された単一のビットに対応るすようになります。
    FENCE_SUBMISSION              signal_fence;
};

struct SUBMIT_TILE_BINDINGS_DESC
{
    uint32_t                            num_bind_infos;
    const TILED_RESOURCE_BIND_INFO*     bind_infos;
    IFence*                             signal_fence_to_cpu; // FENCE_TYPE_BINARY_GPU_TO_CPUフェンスです。    
};


#pragma endregion tiled resource

#pragma region view

enum COMPONENT_SWIZZLE : Enum8T
{
      COMPONENT_SWIZZLE_IDENTITY       // COMPONENT_MAPPING構造の各変数名に対応したコンポーネントが設定されることを指定します。
    , COMPONENT_SWIZZLE_ZERO           // コンポーネントの値が0に設定されることを指定します。
    , COMPONENT_SWIZZLE_ONE            // コンポーネントの値が1に設定されることを指定します。
    , COMPONENT_SWIZZLE_R
    , COMPONENT_SWIZZLE_G
    , COMPONENT_SWIZZLE_B
    , COMPONENT_SWIZZLE_A
};

struct COMPONENT_MAPPING
{
    COMPONENT_SWIZZLE r;
    COMPONENT_SWIZZLE g;
    COMPONENT_SWIZZLE b;
    COMPONENT_SWIZZLE a;
};

enum VIEW_TYPE : EnumT
{
      VIEW_TYPE_CONSTANT_BUFFER
    , VIEW_TYPE_SHADER_RESOURCE
    , VIEW_TYPE_UNORDERED_ACCESS
    , VIEW_TYPE_RENDER_TARGET
    , VIEW_TYPE_DEPTH_STENCIL
    , VIEW_TYPE_VERTEX_BUFFER
    , VIEW_TYPE_INDEX_BUFFER
    , VIEW_TYPE_STREAM_OUTPUT_BUFFER
    , VIEW_TYPE_SAMPLER
    , VIEW_TYPE_END
};

enum VIEW_DIMENSION : EnumT
{
      VIEW_DIMENSION_CONSTANT_BUFFER
    , VIEW_DIMENSION_BUFFER_TYPED
    , VIEW_DIMENSION_BUFFER_STRUCTURED
    , VIEW_DIMENSION_BUFFER_BYTEADDRESS
    , VIEW_DIMENSION_BUFFER_ACCELERATION_STRUCTURE
    , VIEW_DIMENSION_TEXTURE_1D
    , VIEW_DIMENSION_TEXTURE_1D_ARRAY
    , VIEW_DIMENSION_TEXTURE_2D
    , VIEW_DIMENSION_TEXTURE_2D_ARRAY
    , VIEW_DIMENSION_TEXTURE_3D
    , VIEW_DIMENSION_TEXTURE_CUBE        // +X, -X, +Y, -Y, +Z, -Z の順で面に対応します。TEXTURE_VIEW::subresource_range.array_sizeは6である必要があります。
    , VIEW_DIMENSION_TEXTURE_CUBE_ARRAY // array_sizeは6の倍数に揃える必要があります。ただし、array_sizeにB3D_USE_REMAINING_ARRAY_SIZESが指定された場合、offsetからの残りの配列サイズが6の倍数である必要があります。
    , VIEW_DIMENSION_VERTEX_BUFFER
    , VIEW_DIMENSION_INDEX_BUFFER
    , VIEW_DIMENSION_STREAM_OUTPUT_BUFFER
    , VIEW_DIMENSION_SAMPLER
    , VIEW_DIMENSION_END
};

// TODO: SHADER_RESOURCE_VIEW_FLAG
enum SHADER_RESOURCE_VIEW_FLAG : EnumT
{
      SHADER_RESOURCE_VIEW_FLAG_NONE                        = 0x0
    , SHADER_RESOURCE_VIEW_FLAG_DENY_INPUT_ATTACHMENT       = 0x1
};
using SHADER_RESOURCE_VIEW_FLAGS = EnumFlagsT;

enum UNORDERED_ACCESS_VIEW_FLAG : EnumT
{
    UNORDERED_ACCESS_VIEW_FLAG_NONE = 0x0
};
using UNORDERED_ACCESS_VIEW_FLAGS = EnumFlagsT;

enum RENDER_TARGET_VIEW_FLAG : EnumT
{
    RENDER_TARGET_VIEW_FLAG_NONE = 0x0
};
using RENDER_TARGET_VIEW_FLAGS = EnumFlagsT;

enum DEPTH_STENCIL_VIEW_FLAG : EnumT
{
      DEPTH_STENCIL_VIEW_FLAG_NONE                 = 0x0
    , DEPTH_STENCIL_VIEW_FLAG_READ_ONLY_DEPTH      = 0x1
    , DEPTH_STENCIL_VIEW_FLAG_READ_ONLY_STENCIL    = 0x2
};
using DEPTH_STENCIL_VIEW_FLAGS = EnumFlagsT;

struct BUFFER_VIEW
{
    uint64_t first_element;
    uint64_t num_elements;
    uint32_t structure_byte_stride;
};

struct BUFFER_RAYTRACING_ACCELERATION_STRUCTURE
{
    GpuVirtualAddress location;
};

struct TEXTURE_VIEW
{
    COMPONENT_MAPPING components;
    SUBRESOURCE_RANGE subresource_range;
};

struct VIEW_DESC
{
    VIEW_TYPE          type;
    RESOURCE_FORMAT    format;
    VIEW_DIMENSION     dimension;
};

struct CONSTANT_BUFFER_VIEW_DESC
{
    uint64_t buffer_offset;
    uint64_t size_in_bytes;
};

struct SHADER_RESOURCE_VIEW_DESC
{
    /**
     * @brief 作成したテクスチャのフォーマットが深度ステンシルフォーマットの場合、view.formatには同一のフォーマットを設定し、
     *          texture...aspectのフラグにはTEXTURE_ASPECT_FLAG_DEPTHまたはTEXTURE_ASPECT_FLAG_STENCILのいずれかのビットのみをを含める必要があります。
    */
    VIEW_DESC view;
    union
    {
        BUFFER_VIEW                              buffer;
        BUFFER_RAYTRACING_ACCELERATION_STRUCTURE acceleration_structure;
        TEXTURE_VIEW                             texture;
    };
    SHADER_RESOURCE_VIEW_FLAGS flags;
};

struct UNORDERED_ACCESS_VIEW_DESC
{
    VIEW_DESC view;
    union
    {
        BUFFER_VIEW  buffer;
        TEXTURE_VIEW texture;
    };
    uint64_t                    counter_offset_in_bytes;
    UNORDERED_ACCESS_VIEW_FLAGS flags;
};

struct RENDER_TARGET_VIEW_DESC
{
    VIEW_DESC                    view;

    /**
     * @brief dimensionにVIEW_DIMENSION_TEXTURE_2D_ARRAYを設定すれば、ジオメトリシェーダーによるキューブマップ(Tex2DArray[6])同時レンダリングを、各GBuffer毎に行うことも可能です。
     *        例えば、4つのGBufferを使用する場合、Tex2DArray[6]のレンダーターゲットを4つ用意する必要があります。全て同じ解像度、array_sizeである必要があります。
     *        TODO: 3Dテクスチャを2D配列指定する場合、元になるテクスチャオブジェクトに2D配列互換性が必要です。d3d12では3dは3dで作る必要があるが、vkは3dを2d配列として扱う必要があり、更に互換性が必要。12ではsrvの作成で2d配列として使う場合、dimension自体は3dである必要がある。
    */
    TEXTURE_VIEW                 texture;
    RENDER_TARGET_VIEW_FLAGS     flags;
};

struct DEPTH_STENCIL_VIEW_DESC
{
    VIEW_DESC                    view;
    TEXTURE_VIEW                 texture;
    DEPTH_STENCIL_VIEW_FLAGS     flags;
};

enum INDEX_TYPE : EnumT
{
      INDEX_TYPE_UINT16
    , INDEX_TYPE_UINT32
    , INDEX_TYPE_UINT8
};

struct INDEX_BUFFER_VIEW_DESC
{
    uint64_t      buffer_offset;
    uint32_t      size_in_bytes;
    INDEX_TYPE    index_type;
};

struct VERTEX_BUFFER_VIEW_DESC
{
    uint32_t        num_input_slots;    // このビューで設定する頂点バッファーのスロット数を指定します。 バインドを開始するスロットのオフセットの指定はコマンドリストで行います。
    const uint64_t* buffer_offsets;     // リソースのオフセットを指定する、要素数がnum_input_slotsの配列です。
    const uint32_t* sizes_in_bytes;     // 頂点バッファのサイズを指定する、要素数がnum_input_slotsの配列です。
    const uint32_t* strides_in_bytes;   // 頂点間のストライドを指定する、要素数がnum_input_slotsの配列です。
};

struct STREAM_OUTPUT_BUFFER_VIEW_DESC
{
    uint32_t        num_input_slots;
    const uint64_t* buffer_offsets;
    const uint64_t* sizes_in_bytes;
    const uint64_t* filled_size_counter_buffer_offsets; // ストリーム出力の結果が書き込まれたサイズを記録するためのバッファのオフセットを指定します。(BufferFilledSizeLocation)
};

enum SAMPLER_FILTER_MODE : EnumT
{
      SAMPLER_FILTER_MODE_STANDARD
    , SAMPLER_FILTER_MODE_ANISOTROPHIC
};

enum SAMPLER_FILTER_REDUCTION_MODE : EnumT
{
      SAMPLER_FILTER_REDUCTION_MODE_STANDARD       // サンプリングされた各テクセルの加重平均を計算します。
    , SAMPLER_FILTER_REDUCTION_MODE_COMPARISON     // サンプリングされた各テクセルを比較値(SampleCmp等に与えられる)と比較します。パーセンテージクローザーフィルタリングを使用可能です。
    , SAMPLER_FILTER_REDUCTION_MODE_MIN            // サンプリングされたテクセルの最小値を返します。
    , SAMPLER_FILTER_REDUCTION_MODE_MAX            // サンプリングされたテクセルの最大値を返します。
};

enum TEXTURE_SAMPLE_MODE : Enum8T
{
      TEXTURE_SAMPLE_MODE_POINT
    , TEXTURE_SAMPLE_MODE_LINEAR
    , TEXTURE_SAMPLE_MODE_CUBIC_IMG // TEXTURE_SAMPLE_MODE_DESC::mipはこのモード以外を設定する必要があります。
};

enum TEXTURE_ADDRESS_MODE : Enum8T
{
      TEXTURE_ADDRESS_MODE_WRAP
    , TEXTURE_ADDRESS_MODE_MIRROR
    , TEXTURE_ADDRESS_MODE_CLAMP
    , TEXTURE_ADDRESS_MODE_BORDER
    , TEXTURE_ADDRESS_MODE_MIRROR_ONCE
};

// TODO: カスタムボーダーカラーのサポート
enum BORDER_COLOR : EnumT
{
      BORDER_COLOR_TRANSPARENT_BLACK_FLOAT
    , BORDER_COLOR_TRANSPARENT_BLACK_INT
    , BORDER_COLOR_OPAQUE_BLACK_FLOAT
    , BORDER_COLOR_OPAQUE_BLACK_INT
    , BORDER_COLOR_OPAQUE_WHITE_FLOAT
    , BORDER_COLOR_OPAQUE_WHITE_INT
};

enum COMPARISON_FUNC : EnumT
{
      COMPARISON_FUNC_NEVER         // if (false) 
    , COMPARISON_FUNC_LESS          // if (src < dst) 
    , COMPARISON_FUNC_EQUAL         // if (src == dst) 
    , COMPARISON_FUNC_LESS_EQUAL    // if (src <= dst) 
    , COMPARISON_FUNC_GREATER       // if (src > dst) 
    , COMPARISON_FUNC_NOT_EQUAL     // if (src != dst) 
    , COMPARISON_FUNC_GREATER_EQUAL // if (src >= dst) 
    , COMPARISON_FUNC_ALWAYS        // if (true) 
};

struct SAMPLER_FILTER_DESC
{
    SAMPLER_FILTER_MODE           mode;
    SAMPLER_FILTER_REDUCTION_MODE reduction_mode;
    uint32_t                      max_anisotropy; // SAMPLER_FILTER_REDUCTION_MODE_ANISOTROPHICの場合に使用します。
    COMPARISON_FUNC               comparison_func;// SAMPLER_FILTER_REDUCTION_MODE_COMPARISONの場合に使用します。
};

struct TEXTURE_SAMPLE_MODE_DESC
{
    TEXTURE_SAMPLE_MODE minification;
    TEXTURE_SAMPLE_MODE magnification;
    TEXTURE_SAMPLE_MODE mip;
};

struct TEXTURE_ADDRESS_MODE_DESC
{
    TEXTURE_ADDRESS_MODE u;
    TEXTURE_ADDRESS_MODE v;
    TEXTURE_ADDRESS_MODE w;
};

struct SAMPLER_TEXTURE_DESC
{
    TEXTURE_SAMPLE_MODE_DESC  sample;
    TEXTURE_ADDRESS_MODE_DESC address;
};

struct SAMPLER_MIP_LOD_DESC
{
    float bias;
    float min;
    float max;
};

struct SAMPLER_DESC
{
    SAMPLER_FILTER_DESC  filter;
    SAMPLER_TEXTURE_DESC texture;
    SAMPLER_MIP_LOD_DESC mip_lod;
    BORDER_COLOR         border_color;
};


#pragma endregion view

#pragma region command

enum FENCE_TYPE : EnumT
{
    /**
     * @brief コマンドキューからホストへの同期を実行します。 
     *        ICommandQueue::Submit*を介してのみシグナル操作が可能です。 CPUシグナル(IFence::Signal)は使用出来ません。
     *        シグナル送信後、IFence::Waitを使用してCPUで待機することが出来ます。 
     *        待機完了後にシグナルの状態がリセットされることはありません。 そのため再度シグナルを送信する場合、IFence::Resetで事前に状態をリセットする必要があります。 
     *        従って、 単一のSignal -> 単一または任意の数のWait -> 単一のReset が同期操作のセットとなります。 
    */
    FENCE_TYPE_BINARY_GPU_TO_CPU,
      
    /**
     * @brief コマンドキューの実行間で同期を実行します。 
     *        ICommandQueue::Submit*を介してのみシグナル、待機操作が可能です。 IFence::Reset以外のメソッドを使用することは出来ません。
     *        キューでの待機完了後にシグナルの状態が自動的にリセットさます。 
     *        従って、 単一のSignal -> 単一のWait が同期操作のセットとなります。 
    */
    FENCE_TYPE_BINARY_GPU_TO_GPU,

    /**
     * @brief コマンドキューまたはホストの実行間で同期を実行します。 シグナルは値ベースで管理します。 
     *        ICommandQueue::Submit*を介してシグナル、待機操作が可能です。 IFenceのメソッドをフルサポートします。 
     *        このタイプの操作セットは、 単一のSignal -> 任意の数のWait です。
    */
    FENCE_TYPE_TIMELINE
};

enum FENCE_FLAG : EnumT
{
      FENCE_FLAG_NONE                 = 0x0
    , FENCE_FLAG_SHARED               = 0x1 // 別プロセスの同じアダプタで動作するデバイスへのフェンスの共有を許可します。
    , FENCE_FLAG_SHARED_CROSS_ADAPTER = 0x2 // 異なるアダプタで動作するデバイスへのフェンスの共有を許可します。
};
using FENCE_FLAGS = EnumFlagsT;

struct FENCE_DESC
{
    FENCE_TYPE      type;
    uint64_t        initial_value; // フェンスの初期値です。FENCE_TYPE_BINARY_*の場合、値を1以上に設定するとシグナルされた状態で作成されます。
    FENCE_FLAGS     flags;
};

struct COMMAND_QUEUE_DESC
{
    COMMAND_TYPE            type;
    uint32_t                queue_index;
    COMMAND_QUEUE_PRIORITY  priority;
    COMMAND_QUEUE_FLAGS     flags;
    NodeMask                node_mask;
};

struct SUBMIT_INFO
{
    FENCE_SUBMISSION    wait_fence;
    uint32_t            num_command_lists_to_execute;
    ICommandList**      command_lists_to_execute;
    FENCE_SUBMISSION    signal_fence;
};

struct SUBMIT_DESC
{
    uint32_t            num_submit_infos;
    const SUBMIT_INFO*  submit_infos;
    IFence*             signal_fence_to_cpu; // FENCE_TYPE_BINARY_GPU_TO_CPUフェンスです。
};

struct SUBMIT_SIGNAL_DESC
{
    FENCE_SUBMISSION    signal_fence;
    IFence*             signal_fence_to_cpu; // FENCE_TYPE_BINARY_GPU_TO_CPUフェンスです。
};

struct SUBMIT_WAIT_DESC
{
    FENCE_SUBMISSION    wait_fence;
};


enum COMMAND_LIST_LEVEL : EnumT
{
      COMMAND_LIST_LEVEL_PRIMARY   // プライマリコマンドリストを指定します。
    , COMMAND_LIST_LEVEL_SECONDARY // セカンダリ(バンドル)コマンドリストを割り当てます。 この値で作成されたコマンドリストは単体でキューに送信することは出来ません。 現在、COMMAND_TYPE_DIRECTのコマンドリスト、アロケーター以外で指定することは出来ません。
};

enum COMMAND_ALLOCATOR_FLAG : EnumT
{
      COMMAND_ALLOCATOR_FLAG_NONE                     = 0x0
    , COMMAND_ALLOCATOR_FLAG_TRANSIENT                = 0x1 // アロケータから割り当てられたコマンドリストの存続期間が短いことを指定します。 (比較的短い時間内にリセットまたは解放されます。)

    /*
    * @brief アロケータから割り当てられたコマンドリストは、記録時、記録終了時に個別にリセット可能であることを指定します。
    * @remark このフラグを使用して作成されたアロケーターを使用するコマンドリストは、フレーム毎等の短い周期でリセットするべきではありません。 
    *         実装によっては、コマンドリストでの個別のResetでは、アロケーターへメモリが返却されないにもかかわらず、新しいメモリを割り当てる可能性があるためです。 
    *         メモリの肥大化を回避するには、コマンドアロケータのResetを呼び出す必要があります。
    */
    , COMMAND_ALLOCATOR_FLAG_ALLOW_RESET_COMMAND_LIST = 0x1
};
using COMMAND_ALLOCATOR_FLAGS = EnumFlagsT;

struct COMMAND_ALLOCATOR_DESC
{
    COMMAND_TYPE            type;
    COMMAND_LIST_LEVEL      level; // 割り当てるコマンドリストのレベルを指定します。
    COMMAND_ALLOCATOR_FLAGS flags;
};

enum COMMAND_ALLOCATOR_RESET_FLAG : EnumT
{
      COMMAND_ALLOCATOR_RESET_FLAG_NONE              = 0x0
    , COMMAND_ALLOCATOR_RESET_FLAG_RELEASE_RESOURCES = 0x1 // 記録されたコマンドのリセットに加えて、そのメモリも解放することを指定します。 このフラグが指定されていない場合、記録された全てのコマンドはリセットされますが、メモリは再利用のために保持されます。
};
using COMMAND_ALLOCATOR_RESET_FLAGS = EnumFlagsT;

enum COMMAND_LIST_RESET_FLAG
{
    COMMAND_LIST_RESET_FLAG_NONE   = 0x0
};
using COMMAND_LIST_RESET_FLAGS = EnumFlagsT;

// NOTE: プライマリとセカンダリのディスクリプタは一致している必要がある。
struct COMMAND_LIST_DESC
{
    ICommandAllocator*  allocator;  // コマンドの記録に使用されるアロケーターを指定します。
    COMMAND_TYPE        type;       // このコマンドリストのタイプです。 この値はアロケーターが割り当て可能なタイプと一致している必要があります。
    COMMAND_LIST_LEVEL  level;      // 割り当てるコマンドリストのレベルを指定します。 この値はアロケーターが割り当て可能なレベルと一致している必要があります。
    NodeMask            node_mask;  // コマンドが実行されるノードのインデックスを示すの単一のビット指定します。
};

/**
 * @brief コマンドリストの状態を定義します。 
 * @note 実行中(PENDING)であるかどうかの判定は、同期オブジェクトを介して間接的に取得する必要があります。 
 *       この列挙の値は、コマンドリストが正常に使用される場合の状態遷移を理論的に表現するためのものです。 
 *       *記録されたリソースの破棄等の、コマンドアロケータ、コマンドリストの範囲外で発生したエラーを確認することはできません。
*/
enum COMMAND_LIST_STATE : EnumT
{
      COMMAND_LIST_STATE_INITIAL    // コマンドリストが割り当てられた、またはリセットされた状態です。
    , COMMAND_LIST_STATE_RECORDING  // コマンドを記録可能な状態です。
    , COMMAND_LIST_STATE_EXECUTABLE // コマンドをキューに送信、実行が可能な状態です。
    , COMMAND_LIST_STATE_INVALID    // アロケータのリセット、記録されたリソース等の破棄*、またはCOMMAND_LIST_BEGIN_FLAG_ONE_TIME_SUBMITによって、リセットや破棄以外の使用が不可能となった状態です。 
};

enum QUERY_FLAG : EnumT
{
      QUERY_FLAG_NONE               = 0x0
    , QUERY_FLAG_PRECISE_OCCLUSION  = 0x1 // オクルージョンクエリのサンプル数の結果に精度が必要であることを指定します。 このフラグが指定されていない場合、オクルージョンクエリの結果は0または0以外となり、精度が不要な場合に効果的です。 QUERY_HEAP_TYPE_OCCLUSION以外で使用することは出来ません。 
};
using QUERY_FLAGS = EnumFlagsT;

struct COMMAND_LIST_INHERITANCE_DESC
{
    IRenderPass*    render_pass;
    uint32_t        subpass;
    IFramebuffer*   framebuffer;
    QUERY_FLAGS     query_flags;
};

enum COMMAND_LIST_BEGIN_FLAG : EnumT
{
      COMMAND_LIST_BEGIN_FLAG_NONE                      = 0x0
    , COMMAND_LIST_BEGIN_FLAG_ONE_TIME_SUBMIT           = 0x1 // 一度キューに送信すると無効状態となる事を指定します。
    , COMMAND_LIST_BEGIN_FLAG_RENDER_PASS_CONTINUE      = 0x2 // 記録されるコマンドのスコープがレンダーパスインスタンス内であることを指定します。
    , COMMAND_LIST_BEGIN_FLAG_ALLOW_SIMULTANEOUS_USE    = 0x4 // バンドルを複数のコマンドリストに同時に記録して使用する事を指定します。 バンドルタイプ以外で使用出来ません。
};
using COMMAND_LIST_BEGIN_FLAGS = EnumFlagsT;

struct COMMAND_LIST_BEGIN_DESC
{
    COMMAND_LIST_BEGIN_FLAGS             flags;
    const COMMAND_LIST_INHERITANCE_DESC* inheritance_desc; // セカンダリコマンドリストの場合に参照されます。
};

enum STENCIL_FACE : EnumT
{
      STENCIL_FACE_FLAG_FRONT_AND_BACK
    , STENCIL_FACE_FLAG_FRONT     
    , STENCIL_FACE_FLAG_BACK
};

enum CLEAR_FLAG : EnumT
{
      CLEAR_FLAG_DEPTH   = 0x1
    , CLEAR_FLAG_STENCIL = 0x2
};
using CLEAR_FLAGS = EnumFlagsT;

enum QUERY_HEAP_TYPE : EnumT
{
      QUERY_HEAP_TYPE_OCCLUSION
    , QUERY_HEAP_TYPE_TIMESTAMP
    , QUERY_HEAP_TYPE_PIPELINE_STATISTICS
    , QUERY_HEAP_TYPE_SO_STATISTICS
    , QUERY_HEAP_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE
    , QUERY_HEAP_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_SIZE
    , QUERY_HEAP_TYPE_VIDEO_DECODE_STATISTICS

    //, QUERY_HEAP_TYPE_COPY_QUEUE_TIMESTAMP
};

struct QUERY_HEAP_DESC
{
    QUERY_HEAP_TYPE type;
    uint32_t        num_queries;
    NodeMask        node_mask;
};

enum PREDICATION_OP : EnumT
{
      PREDICATION_OP_EQUAL_ZERO
    , PREDICATION_OP_NOT_EQUAL_ZERO
};

struct QUERY_DATA_PIPELINE_STATISTICS
{
    uint64_t read_vertices;
    uint64_t read_primitives;
    uint64_t vs_invocations;
    uint64_t gs_invocations;
    uint64_t gs_primitives;
    uint64_t rasterized_primitives;
    uint64_t rendered_primitives;
    uint64_t ps_invocations;
    uint64_t hs_invocations;
    uint64_t ds_invocations;
    uint64_t cs_invocations;
};

struct QUERY_DATA_SO_STATISTICS
{
    uint64_t num_primitives_written;
    uint64_t primitives_storage_needed;
};

struct STREAM_OUTPUT_BUFFER_VIEW
{
    GpuVirtualAddress buffer_location;              // vkCmdBeginTransformFeedbackEXT::pCounterBuffers[i]
    uint64_t          size_in_bytes;                // 
    GpuVirtualAddress buffer_filled_size_location;  // vkCmdBeginTransformFeedbackEXT::pCounterBufferOffsets[i]
};

struct DRAW_ARGUMENTS
{
    uint32_t vertex_count_per_instance;
    uint32_t instance_count;
    uint32_t start_vertex_location;
    uint32_t start_instance_location;
};

struct DRAW_INDEXED_ARGUMENTS
{
    uint32_t index_count_per_instance;
    uint32_t instance_count;
    uint32_t start_index_location;
    int32_t  base_vertex_location;
    uint32_t start_instance_location;
};

struct DISPATCH_ARGUMENTS
{
    uint32_t thread_group_count_x;
    uint32_t thread_group_count_y;
    uint32_t thread_group_count_z;
};

struct INDIRECT_COMMAND_DESC
{
    IBuffer*    argument_buffer;
    uint64_t    argument_buffer_offset;
    IBuffer*    command_count_buffer;
    uint64_t    command_count_buffer_offset;
    uint32_t    max_command_count;
};

enum INDIRECT_ARGUMENT_TYPE : EnumT
{
      INDIRECT_ARGUMENT_TYPE_VERTEX_BUFFER_VIEW     // VK_INDIRECT_COMMANDS_TOKEN_TYPE_VERTEX_BUFFER_NV 
    , INDIRECT_ARGUMENT_TYPE_INDEX_BUFFER_VIEW      // VK_INDIRECT_COMMANDS_TOKEN_TYPE_INDEX_BUFFER_NV 
    , INDIRECT_ARGUMENT_TYPE_PUSH_32BIT_CONSTANT    // VK_INDIRECT_COMMANDS_TOKEN_TYPE_PUSH_CONSTANT_NV 
    , INDIRECT_ARGUMENT_TYPE_DRAW
    , INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED           // VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_INDEXED_NV
    , INDIRECT_ARGUMENT_TYPE_DISPATCH
    , INDIRECT_ARGUMENT_TYPE_DISPATCH_RAYS          // VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_NV
    , INDIRECT_ARGUMENT_TYPE_DISPATCH_MESH_TASKS    // VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_TASKS_NV
};

struct INDIRECT_ARGUMENT_DESC
{
    INDIRECT_ARGUMENT_TYPE type;
    union
    {
        struct
        {
            uint32_t slot;
        }    vertex_buffer;
        struct
        {
            uint32_t root_parameter_index;
            uint32_t dest_offset_in_32bit_values;
            uint32_t num_32bit_values_to_set;
        }    constant;
        struct
        {
            uint32_t root_parameter_index;
        }    constant_buffer_view;
        struct
        {
            uint32_t root_parameter_index;
        }    shader_resource_view;
        struct
        {
            uint32_t root_parameter_index;
        }    unordered_access_view;

    };
};

struct COMMAND_SIGNATURE_DESC
{
    uint32_t                        byte_stride;
    uint32_t                        num_argument_descs;
    const INDIRECT_ARGUMENT_DESC*   argument_descs;
    NodeMask                        nodes_mask;
};

// TODO: COMMAND_SIGNATURE
//struct GPU_VIRTUAL_ADDRESS_RANGE
//{
//    GpuVirtualAddress   start_address;
//    uint64_t            size_in_bytes;
//};
//
//struct GPU_VIRTUAL_ADDRESS_RANGE_AND_STRIDE
//{
//    GpuVirtualAddress   start_address;
//    uint64_t            size_in_bytes;
//    uint64_t            stride_in_bytes;
//};
//
//struct DISPATCH_RAYS_DESC
//{
//    GPU_VIRTUAL_ADDRESS_RANGE               ray_generation_shader_record;
//    GPU_VIRTUAL_ADDRESS_RANGE_AND_STRIDE    miss_shader_table;
//    GPU_VIRTUAL_ADDRESS_RANGE_AND_STRIDE    hit_group_table;
//    GPU_VIRTUAL_ADDRESS_RANGE_AND_STRIDE    callable_shader_table;
//    uint32_t                                width;
//    uint32_t                                height;
//    uint32_t                                depth;
//  VkBuffer                                raygenShaderBindingTableBuffer,
//  VkDeviceSize                            raygenShaderBindingOffset,
//  VkBuffer                                missShaderBindingTableBuffer,
//  VkDeviceSize                            missShaderBindingOffset,
//  VkDeviceSize                            missShaderBindingStride,
//  VkBuffer                                hitShaderBindingTableBuffer,
//  VkDeviceSize                            hitShaderBindingOffset,
//  VkDeviceSize                            hitShaderBindingStride,
//  VkBuffer                                callableShaderBindingTableBuffer,
//  VkDeviceSize                            callableShaderBindingOffset,
//  VkDeviceSize                            callableShaderBindingStride,
//  uint32_t                                width,
//  uint32_t                                height,
//  uint32_t                                depth
//};

//enum INDIRECT_ARGUMENT_INDEX_TYPE : EnumT
//{
//      INDIRECT_ARGUMENT_INDEX_TYPE_UINT16 = 57 // == DXGI_FORMAT_R16_UINT
//    , INDIRECT_ARGUMENT_INDEX_TYPE_UINT32 = 42 // == DXGI_FORMAT_R32_UINT
//    , INDIRECT_ARGUMENT_INDEX_TYPE_UINT8  = 62 // == DXGI_FORMAT_R8_UINT
//};
//
//struct INDIRECT_COMMANDS_LAYOUT_TOKEN
//{
//     INDIRECT_ARGUMENT_TYPE           type;
//     INDIRECT_COMMANDS_TOKEN_TYPE     token_type;
//      
//     // Vulkanではインダイレクト引数バッファを分割可能です。 コマンドシグネチャ作成時にstreamというバッファのスロットを指定することが出来、間接コマンド起動時に複数のstreamに複数のバッファを関連付けます。
//     // これにより、部分的なコマンド値の変更時にキャッシュ効率が向上する可能性があります。
//     uint32_t                         stream;
//     uint32_t                         offset;
// 
//     union
//     {
//         struct
//         {
//             uint32_t slot;
//         } vertex_buffer;
//         struct
//         {
//             IRootSignature* root_signature;
//             uint32_t        root_parameter_index;
//             uint32_t        dst_offset_in_32bit_values;
//             uint32_t        num_32bit_values_to_set;
//         } constant;
//     };
// 
//     uint32_t                            num_index_types;
//     const INDIRECT_ARGUMENT_INDEX_TYPE* index_types;
// 
//     // 他のAPIとのインデックスタイプ値に関する互換性を保つために使用されます。
//     // この値がnullptrではない場合、index_type_valuesの各インデックスの要素の値を対応するindex_typesの要素のVkIndexType値としてマッピングし、
//     // インデックスタイプを指定する間接コマンド用のデータ     を格納するバッファの値として使用することが可能です(map<uint32_t,VkIndexType>のように)。
//     // nullptrの場合バッファに設定する必要がある値は、VkIndexTypeの列挙と同等の値です。
//     const uint32_t*                     index_type_values;
//};

#pragma endregion command

#pragma region descriptor

/* Implicit binding number assignment: https://github.com/microsoft/DirectXShaderCompiler/blob/master/docs/SPIR-V.rst#implicit-binding-number-assignment

b - for constant buffer views (CBV)
CONSTANT_BUFFER/CBV_DYNAMIC/INLINE_32BIT_CONSTANTS
    CBUFFER                 : uniform buffer
    CONSTANTBUFFER          : uniform buffer

t - for shader resource views (SRV)
INPUT_ATTACHMENT
    TEXTURE2D               : input attachment
SRV_TEXTURE
    TEXTURE1D               : sampled image
    TEXTURE1DARRAY          : sampled image
    TEXTURE2D               : sampled image
    TEXTURE2DARRAY          : sampled image
    TEXTURE3D               : sampled image
    TEXTURECUBE             : sampled image
    TEXTURECUBEARRAY        : sampled image
    TEXTURE2DMS             : sampled image
    TEXTURE2DMSARRAY        : sampled image
SRV_BUFFER
    STRUCTUREDBUFFER        : storage buffer
    BYTEADDRESSBUFFER       : storage buffer
    TBUFFER                 : storage buffer https://blog.csdn.net/P_hantom/article/details/108083086 
    TEXTUREBUFFER           : storage buffer
SRV_TYPED_BUFFER
    BUFFER                  : uniform texel buffer

u - for unordered access views (UAV)
UAV_TEXTURE
    RWTEXTURE1D             : storage image
    RWTEXTURE1DARRAY        : storage image
    RWTEXTURE2D             : storage image
    RWTEXTURE2DARRAY        : storage image
    RWTEXTURE3D             : storage image
UAV_BUFFER
    RWBYTEADDRESSBUFFER     : storage buffer
    RWSTRUCTUREDBUFFER      : storage buffer
    APPENDSTRUCTUREDBUFFER  : storage buffer
    CONSUMESTRUCTUREDBUFFER : storage buffer
UAV_TYPED_BUFFER
    RWBUFFER                : storage texel buffer

s - for samplers
SAMPLER
STATIC_SAMPLER
    SAMPLER
    SAMPLER1D
    SAMPLER2D
    SAMPLER3D
    SAMPLERCUBE
    SAMPLERSTATE
    SAMPLERCOMPARISONSTATE

    D3D12_DESCRIPTOR_RANGE_TYPE_SRV
    D3D12_DESCRIPTOR_RANGE_TYPE_UAV
    D3D12_DESCRIPTOR_RANGE_TYPE_CBV
    
*/
enum DESCRIPTOR_TYPE : EnumT
{
      DESCRIPTOR_TYPE_CBV                               // register(b)   uniform buffer            D3D12_DESCRIPTOR_RANGE_TYPE_CBV
    , DESCRIPTOR_TYPE_INPUT_ATTACHMENT                  // register(t)   input attachment          D3D12_DESCRIPTOR_RANGE_TYPE_SRV
    , DESCRIPTOR_TYPE_SRV_TEXTURE                       // register(t)   sampled image             D3D12_DESCRIPTOR_RANGE_TYPE_SRV
    , DESCRIPTOR_TYPE_SRV_TYPED_BUFFER                  // register(t)   uniform texel buffer      D3D12_DESCRIPTOR_RANGE_TYPE_SRV
    , DESCRIPTOR_TYPE_SRV_BUFFER                        // register(t)   storage buffer            D3D12_DESCRIPTOR_RANGE_TYPE_SRV
    , DESCRIPTOR_TYPE_UAV_TEXTURE                       // register(u)   storage image             D3D12_DESCRIPTOR_RANGE_TYPE_UAV
    , DESCRIPTOR_TYPE_UAV_TYPED_BUFFER                  // register(u)   storage texel buffer      D3D12_DESCRIPTOR_RANGE_TYPE_UAV
    , DESCRIPTOR_TYPE_UAV_BUFFER                        // register(u)   storage buffer            D3D12_DESCRIPTOR_RANGE_TYPE_UAV
//  , DESCRIPTOR_TYPE_UAV_BUFFER_WITH_COUNTER           // register(u)   storage buffer         x2 D3D12_DESCRIPTOR_RANGE_TYPE_UAV
    , DESCRIPTOR_TYPE_SRV_ACCELERATION_STRUCTURE        // register(t)   acceleration structure    D3D12_DESCRIPTOR_RANGE_TYPE_SRV
    , DESCRIPTOR_TYPE_SAMPLER                           // register(s)   sampler                   D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER

    , DESCRIPTOR_TYPE_CBV_DYNAMIC                       // register(b)   uniform buffer dynamic    D3D12_ROOT_PARAMETER_TYPE_CBV
    , DESCRIPTOR_TYPE_SRV_BUFFER_DYNAMIC                // register(t)   storage buffer dynamic    D3D12_ROOT_PARAMETER_TYPE_SRV
    , DESCRIPTOR_TYPE_UAV_BUFFER_DYNAMIC                // register(u)   storage buffer dynamic    D3D12_ROOT_PARAMETER_TYPE_UAV

    , DESCRIPTOR_TYPE_NUM_TYPES
};

struct DESCRIPTOR_POOL_SIZE
{
    DESCRIPTOR_TYPE type;
    uint32_t        num_descriptors;
};

enum DESCRIPTOR_POOL_FLAG : EnumT
{
      DESCRIPTOR_POOL_FLAG_NONE                   = 0x0
    , DESCRIPTOR_POOL_FLAG_FREE_DESCRIPTOR_SET    = 0x1
    , DESCRIPTOR_POOL_FLAG_UPDATE_AFTER_BIND_POOL = 0x2 // UPDATE_AFTER_BINDフラグのディスクリプタを割り当て可能なプールが作成されることを示します。非UPDATE_AFTER_BINDフラグのディスクリプタは引き続き割り当て可能です。
};
using DESCRIPTOR_POOL_FLAGS = EnumFlagsT;

struct DESCRIPTOR_POOL_DESC
{
    DESCRIPTOR_POOL_FLAGS       flags;
    uint32_t                    max_sets_allocation_count;  // ディスクリプタセットを割り当て出来る回数の最大値を指定します。
    uint32_t                    max_num_register_space;     // 割り当てる任意のディスクリプタセットに対応するシグネチャ内に含まれる、最大のregister_space数を指定します。例えば、2つのregister_space(space1,space3や、space0,space1などの任意の組み合わせ)が使用されるシグネチャの場合、この値は2以上である必要があります。
    uint32_t                    num_pool_sizes;
    const DESCRIPTOR_POOL_SIZE* pool_sizes;
    NodeMask                    node_mask;                  // ディスクリプタが作成されるノードを指定します。TODO: D3D12で、複数のノードを指定可能にするかどうか。(VulkanではディスクリプタプールにNodeMaskの制約が無く、D3D12のCreationNodeMaskの為にVkDescriptorPoolが複数作成されることは避けたい。)
};


#pragma endregion descriptor

#pragma region descriptor updates

struct WRITE_DESCRIPTOR_RANGE
{
    uint32_t                        dst_range_index;            // 宛先ディスクリプタテーブル内のディスクリプタレンジのオフセットです。
    uint32_t                        dst_first_array_element;    // 宛先ディスクリプタレンジの、ディスクリプタの書き込み先配列オフセットです。
    uint32_t                        num_descriptors;            // src_viewsの要素数です。
    IView*const *                   src_views;                  // 関連付けるビューの配列です。 ビューのタイプはルートシグネチャで指定されたディスクリプタタイプとの互換性が必要です。 
};

struct WRITE_DESCRIPTOR_TABLE
{
    uint32_t                        dst_root_parameter_index;   // 宛先ルートパラメータのインデックスです。 
    uint32_t                        num_ranges;                 // rangesの要素数です。
    const WRITE_DESCRIPTOR_RANGE*   ranges;                     // テーブル内のレンジを指定するWRITE_DESCRIPTOR_RANGE配列です。
};

struct WRITE_DYNAMIC_DESCRIPTOR
{
    uint32_t                        dst_root_parameter_index;   // 宛先ルートパラメータのインデックスです。
    IView*                          src_view;                   // 宛先ディスクリプタに関連付けるバッファーリソースのビューです。 テクスチャリソース、型付きバッファのビューを含めることはできません。
    uint64_t                        src_view_buffer_offset;     // 指定のビューを基準にしたオフセットです。 
};

struct WRITE_DESCRIPTOR_SET
{
    IDescriptorSet*                 dst_set;                    // 書き込み先のディスクリプタセットです。
    uint32_t                        num_descriptor_tables;      // descriptor_tablesの要素数です。
    const WRITE_DESCRIPTOR_TABLE*   descriptor_tables;          // 宛先ルートパラメータとソースビューを指定するWRITE_DESCRIPTOR_TABLE構造の配列です。
    uint32_t                        num_dynamic_descriptors;    // dynamic_descriptorsの要素数です。
    const WRITE_DYNAMIC_DESCRIPTOR* dynamic_descriptors;        // 宛先ルートパラメータとソースビューを指定するWRITE_DYNAMIC_DESCRIPTOR構造の配列です。
};

struct COPY_DESCRIPTOR_RANGE
{
    uint32_t                        range_index;                // ソースまたは宛先のディスクリプタレンジを指定します。
    uint32_t                        first_array_element;        // ディスクリプタレンジにおける、最初の配列要素を指定します。
};

struct COPY_DESCRIPTOR_TABLE
{
    uint32_t                        src_root_parameter_index;   // コピーするディスクリプタテーブルのルートパラメータインデックスを指定します。
    uint32_t                        dst_root_parameter_index;   // 宛先ディスクリプタテーブルのルートパラメータインデックスを指定します。
    uint32_t                        num_ranges;                 // src_ranges, dst_ranges, num_descriptors 配列の要素数です。 
    const COPY_DESCRIPTOR_RANGE*    src_ranges;                 // コピーするディスクリプタレンジを指定するCOPY_DESCRIPTOR_RANGE構造の配列です。
    const COPY_DESCRIPTOR_RANGE*    dst_ranges;                 // 宛先のディスクリプタレンジを指定するCOPY_DESCRIPTOR_RANGE構造の配列です。
    const uint32_t*                 num_descriptors;            // src_rangesとdst_rangesの各first_array_elementからコピーされるディスクリプタの数を指定する配列です。
};

struct COPY_DYNAMIC_DESCRIPTOR
{
    uint32_t                        src_root_parameter_index;   // コピーするディスクリプタを指定するルートパラメータインデックスです。
    uint32_t                        dst_root_parameter_index;   // 宛先のディスクリプタを指定するルートパラメータインデックスです。
};

struct COPY_DESCRIPTOR_SET
{
    IDescriptorSet*                 src_set;                    // コピーするディスクリプタセットです。
    IDescriptorSet*                 dst_set;                    // 宛先のディスクリプタセットです。
    uint32_t                        num_descriptor_tables;      // descriptor_tables配列の要素数です。
    const COPY_DESCRIPTOR_TABLE*    descriptor_tables;          // コピーするディスクリプタテーブルを指定するCOPY_DESCRIPTOR_TABLE構造の配列です。
    uint32_t                        num_dynamic_descriptors;    // dynamic_descriptors配列の要素数です。
    const COPY_DYNAMIC_DESCRIPTOR*  dynamic_descriptors;        // コピーする動的ディスクリプタを指定するCOPY_DYNAMIC_DESCRIPTOR構造の配列です。
};

struct UPDATE_DESCRIPTOR_SET_DESC
{
    uint32_t                        num_write_descriptor_sets;
    const WRITE_DESCRIPTOR_SET*     write_descriptor_sets;
    uint32_t                        num_copy_descriptor_sets;
    const COPY_DESCRIPTOR_SET*      copy_descriptor_sets;
};

#pragma endregion descriptor updates

#pragma region root signature

enum SHADER_VISIBILITY : EnumT
{
      SHADER_VISIBILITY_VERTEX
    , SHADER_VISIBILITY_HULL
    , SHADER_VISIBILITY_DOMAIN
    , SHADER_VISIBILITY_GEOMETRY
    , SHADER_VISIBILITY_PIXEL
    , SHADER_VISIBILITY_MESH
    , SHADER_VISIBILITY_TASK                     // AMPLIFICATION
    , SHADER_VISIBILITY_ALL_GRAPHICS_COMPUTE     // 計算シェーダーを含む、すべてのグラフィックスパイプラインのシェーダーに対して可視であることを示します。レイトレーシングパイプラインの場合、この値である必要があります。
};

enum RAY_TRACING_SHADER_VISIBILITY_FLAG : EnumT
{
      RAY_TRACING_SHADER_VISIBILITY_FLAG_NONE         = 0x0
    , RAY_TRACING_SHADER_VISIBILITY_FLAG_RAYGEN       = 0x1
    , RAY_TRACING_SHADER_VISIBILITY_FLAG_ANY_HIT      = 0x2
    , RAY_TRACING_SHADER_VISIBILITY_FLAG_CLOSEST_HIT  = 0x4
    , RAY_TRACING_SHADER_VISIBILITY_FLAG_MISS         = 0x8
    , RAY_TRACING_SHADER_VISIBILITY_FLAG_INTERSECTION = 0x10
    , RAY_TRACING_SHADER_VISIBILITY_FLAG_CALLABLE     = 0x20
};
using RAY_TRACING_SHADER_VISIBILITY_FLAGS = EnumFlagsT;

enum DESCRIPTOR_FLAG : EnumT
{
      DESCRIPTOR_FLAG_NONE                                              = 0x0   // コマンドリストにセットされた後、ディスクリプタ自体が静的であり、コマンドリストの破棄またはリセットを行うまで変更出来ません。 これは、コマンドリストにセットされた後、ディスクリプタが指すリソースの参照を、CopyDescriptors等によって変更できない事を示します。
    , DESCRIPTOR_FLAG_DESCRIPTORS_UPDATE_AFTER_BIND                     = 0x1   // コマンドリストにセットされた後、ディスクリプタ自体が揮発性である事を指定します。 コマンドリストを送信し、実行が完了するまでの間を除き、CopyDescriptors等によって変更することが出来ます。 DESCRIPTORS_VOLATILE
    , DESCRIPTOR_FLAG_DESCRIPTORS_UPDATE_UNUSED_WHILE_PENDING           = 0x2   // コマンドリストにセットされていないディスクリプタを更新出来ることを指定します。 このフラグは内部APIによる制約が無く、アプリケーションに影響する動作の変化が無い場合暗黙的に有効になります。
    , DESCRIPTOR_FLAG_PARTIALLY_BOUND                                   = 0x4   // リソースがシェーダー側で参照されない場合、ディスクリプタへリソースを設定する必要が無いことを指定します。 このフラグは内部APIによる制約が無く、アプリケーションに影響する動作の変化が無い場合暗黙的に有効になります。
//  , DESCRIPTOR_FLAG_DATA_VOLATILE                                     = 0x8   // コマンドリストにセットされた後、ディスクリプタが指すリソースが揮発性である事を指定します。 コマンドリストを送信し、実行が完了するまでの間を除き、CPUによってリソースのデータを操作可能です。
//  , DESCRIPTOR_FLAG_DATA_STATIC_WHILE_SET_AT_EXECUTE                  = 0x10  // コマンドリストにセットされた後、ディスクリプタが指すリソースを参照するドローコールが実行されている間、そのデータが静的である事を指定します。(もう一度セットし直すことでリソースが変更された事をドライバに通知することが出来ます。)
//  , DESCRIPTOR_FLAG_DATA_STATIC                                       = 0x20  // コマンドリストにセットされた後、ディスクリプタが指すリソースは静的であり、コマンドリストの破棄またはリセットを行うまで変更出来ません。
//  , DESCRIPTOR_FLAG_DESCRIPTORS_STATIC_KEEPING_BUFFER_BOUNDS_CHECKS   = 0x10000
};
using DESCRIPTOR_FLAGS = EnumFlagsT;

struct DESCRIPTOR_RANGE
{
    DESCRIPTOR_TYPE         type;               // _DYNAMIC以外の値を指定する必要があります。
    uint32_t                num_descriptors;    // 例: Texture2D tex[3] : register(base_shader_register)の場合、register(base_shader_register)~register(base_shader_register+2) までのレジスタにセットされます。
    uint32_t                base_shader_register;
    uint32_t                register_space;
    DESCRIPTOR_FLAGS        flags;
};

struct ROOT_DESCRIPTOR_TABLE
{
    uint32_t                num_descriptor_ranges;
    const DESCRIPTOR_RANGE* descriptor_ranges;
};

struct PUSH_32BIT_CONSTANTS
{
    uint32_t                shader_register;
    uint32_t                register_space;     // register_spaceの指定は可能ですが、現状register_spaceを変更してもROOT_PARAMETER::inline_constantsの制約を回避することは出来ません。 (例えば、2つのinline_constantsパラメータを仮定したとき、register_spaceがそれぞれ異なっていても、両方が頂点シェーダー可視の場合前述の制約が存在します。)
    uint32_t                num32_bit_values;   // 32ビット値の数を指定します。バイト単位では無いことに注意してください。
};

struct ROOT_DYNAMIC_DESCRIPTOR
{
    DESCRIPTOR_TYPE         type;               // _DYNAMICの付く値のみを指定する必要があります。
    uint32_t                shader_register;
    uint32_t                register_space;
    DESCRIPTOR_FLAGS        flags;              // DESCRIPTOR_FLAG_DESCRIPTORS_UPDATE_AFTER_BINDは設定されていない必要があります。
};

enum ROOT_PARAMETER_TYPE : EnumT
{
      ROOT_PARAMETER_TYPE_PUSH_32BIT_CONSTANTS    // register(b)    push_descriptor D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS
    , ROOT_PARAMETER_TYPE_DYNAMIC_DESCRIPTOR      //                                D3D12_ROOT_PARAMETER_TYPE_CBV/_SRV/_UAV
    , ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE        //                                D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE
};

struct ROOT_PARAMETER
{
    SHADER_VISIBILITY shader_visibility;
    ROOT_PARAMETER_TYPE type;
    union
    {
        /**
         * @brief 各コマンドリストの記録時にセット可能な32ビット値の定数バッファです。 コマンドリストの記録時に値が直接保存されるため、ディスクリプタを割り当てる必要がありません。 
         *        ROOT_SIGNATURE_DESC::parameters内に、 同じshader_visibilityを持ったinline_constantsを使用するパラメータ が含まれていない必要があります。
         *        shader_visibilityがALL_GRAPHICS_COMPUTEの場合全て可視となり、inline_constantsはそれ以上含めることは出来ません。さらに、それ以前、または以降に別のinline_constantsパラメータが存在してはなりません。
         *        push-constantsによる制約です。
        */
        PUSH_32BIT_CONSTANTS    inline_constants;
        ROOT_DYNAMIC_DESCRIPTOR dynamic_descriptor;
        ROOT_DESCRIPTOR_TABLE   descriptor_table;
    };
};

struct STATIC_SAMPLER
{
    uint32_t             shader_register;
    uint32_t             register_space;
    SHADER_VISIBILITY    shader_visibility;
    ISamplerView*        sampler;
};

/**
 * @brief ROOT_SIGNATURE_DESC::parametersの各シェーダー可視性に指定されたステージを、ルートシグネチャレベルで拒否、または(SHADER_VISIBILITYレベルで可視である場合)許可します。
 *        例えば、ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESSがセットされる場合、
 *        SHADER_VISIBILITY_VERTEX, SHADER_VISIBILITY_ALL_GRAPHICS_COMPUTEの可視性が指定され全てのパラメータは、頂点シェーダーの可視性がマスクされ、不可視になります。
 *
 *        また、レイトレーシングシェーダーステージに対するルートシグネチャであるかどうかも指定します。
*/
enum ROOT_SIGNATURE_FLAG : EnumT
{
      ROOT_SIGNATURE_FLAG_NONE                                  = 0x0   // デフォルトでは、頂点入力アセンブラ、頂点シェーダー、ピクセルシェーダー、または(ALL_GRAPHICS_COMPUTEの場合)計算シェーダーが有効です。
    , ROOT_SIGNATURE_FLAG_DENY_INPUT_ASSEMBLER_INPUT_LAYOUT     = 0x1   // 頂点入力アセンブラ                      を拒否します。
    , ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS        = 0x2   // 頂点シェーダー                          を拒否します。
    , ROOT_SIGNATURE_FLAG_ALLOW_HULL_SHADER_ROOT_ACCESS         = 0x4   // ハル(テセレーション操作)シェーダー      を 許可します。
    , ROOT_SIGNATURE_FLAG_ALLOW_DOMAIN_SHADER_ROOT_ACCESS       = 0x8   // ドメイン(テセレーション評価)シェーダー  を 許可します。
    , ROOT_SIGNATURE_FLAG_ALLOW_GEOMETRY_SHADER_ROOT_ACCESS     = 0x10  // ジオメトリシェーダー                    を 許可します。
    , ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS         = 0x20  // ピクセルシェーダー                      を拒否します。
    , ROOT_SIGNATURE_FLAG_ALLOW_STREAM_OUTPUT                   = 0x40  // ストリーム出力                          を 許可します。(TRANSFORM_FEEDBACK)
    , ROOT_SIGNATURE_FLAG_ALLOW_MESH_SHADER_ROOT_ACCESS         = 0x80  // メッシュシェーダー                      を 許可します。
    , ROOT_SIGNATURE_FLAG_ALLOW_TASK_SHADER_ROOT_ACCESS         = 0x100 // タスク(アンプリフィケーション)シェーダーを 許可します。

    /* 
    * NOTE: D3D12のローカルルートシグネチャの概念を取り除き、Vulkanベースの設計に寄せます。
    *       D3D12では D3D12_SHADER_VISIBILITYを使用して特定のレイトレーシングシェーダーのみに対して可視のルートシグネチャ を作成する事が出来ず、
    *       レイトレーシングパイプラインである状態オブジェクトの作成時にシェーダーに対してローカルルートシグネチャを関連付けなければ、
    *       特定のレイトレーシングシェーダーのみに対して可視にすることが出来ません。D3D12_SHADER_VISIBILITYによる可視性を設定するインターフェースから逸脱しています。
    *       VulkanにそのようなAPIは無く、今まで通りにvkCmdBindDescriptorSetsを介して特定のレイトレーシングシェーダー可視のセットを割り当てることが可能です。(ローカルディスクリプタセットの概念はありません)。
    *       ただし、Vulkanはシェーダーバインディングテーブルのレコードには、ユニフォームバッファのデータを直接記録することしかできません(32ビット定数との互換性があります)。
    *
    *       Vulkan:
    *           - シェーダー可視性は、ディスクリプタセットレイアウトバインディングレベルで(VkShaderStageFlagsによって複数のステージを指定して)任意のシェーダーに可視性を設定出来ます。
    *           - シェーダーレコードには値を直接書き込むユニフォームバッファ(ShaderRecordBuffer)しか使用出来ず、D3D12のローカルルートシグネチャとの互換性がありません。
    *       D3D12:
    *           - シェーダー可視性は、ルートパラメータレベルでは全てのレイトレーシングシェーダーステージに対して可視(D3D12_SHADER_VISIBILITY_ALL)でなければなりません。
    *             また、D3D12_SHADER_VISIBILITYは特定の単一のグラフィックスパイプラインのシェーダーステージ、または全て(_ALL)しか設定出来ず、VulkanのVkShaderStageFlagsとの互換性がありません。
    *             但し、状態オブジェクト作成時にROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATUREを使用して作成されたルートシグネチャを特定のレイトレーシングシェーダー(ローカル)に対して関連付ける事で可視性を操作できます。
    *           - シェーダーレコードには上記のローカルルートシグネチャに対応するGPUディスクリプタハンドル、32ビット定数を書き込む事ができます。
    * 
    *       これらの事から両APIの間では、レイトレーシングシェーダーへの可視性を操作するインターフェースが異なるため、抽象化が困難です。
    *       現状、レイトレーシングシェーダーへのリソースバインディングは以下の制限を設ける必要があります:
    *           - 全てのレイトレーシングシェーダーステージに対して可視のルートシグネチャのみに、通常の定数バッファやテクスチャ等のリソースをバインドする。
    *           - シェーダーレコードには32ビット定数のみを含める(i.e. シェーダーレコードバッファへのディスクリプタハンドルの書き込みが不要であり、かつVulkanのShaderRecordBufferと互換であるため)。
    *       これにより、APIレベルでの可視性の設定は出来なくなりますが、RegisterSpace等の機能を使用してアプリケーション側で可視性の管理を行うことは可能です(DESCRIPTOR_FLAG等の制約は引き続き有効です)。
    *
    *       要約すると、ローカルルートシグネチャの機能をほとんど使用せず、定数バッファやテクスチャは全てのレイトレーシングシェーダーステージに対して可視であり、シェーダーレコードに32ビット定数のみを含めるという制限を設ける必要があります。
    * 
    * TODO: パイプラインのキャッシュや、ディスクリプタのコピーの制約は、今後検証する必要があります。
    */

    // WARNING: レイトレーシングAPIはWIPです。現在は対応していません。
    /* 
    * @brief レイトレーシング用ルートシグネチャである事を指定します。
    *        このフラグを指定する場合、他の全てのフラグは設定されていない必要があり、ROOT_SIGNATURE_DESC::parameters内の全てのシェーダー可視性はALL_GRAPHICS_COMPUTEである必要があります。(LOCAL_ROOT_SIGNATURE)
    */
    , ROOT_SIGNATURE_FLAG_RAY_TRACING_SHADER_VISIBILITY = 0x200 
};
using ROOT_SIGNATURE_FLAGS = EnumFlagsT;

enum SHADER_REGISTER_TYPE : EnumT
{
      SHADER_REGISTER_TYPE_T // register(t*) タイプです。
    , SHADER_REGISTER_TYPE_S // register(s*) タイプです。
    , SHADER_REGISTER_TYPE_U // register(u*) タイプです。
    , SHADER_REGISTER_TYPE_B // register(b*) タイプです。
};

// -fvk-register_type-shift register_shift register_space
// RegisterをVkDescriptorSetLayoutBinding::bindingにリマップするために使用する、各レジスタタイプ、レジスタスペースごとの各Registerのオフセットを指定します。
struct SHADER_REGISTER_SHIFT
{
    SHADER_REGISTER_TYPE    register_type;
    uint32_t                register_shift;
    uint32_t                register_space;
};

struct ROOT_SIGNATURE_DESC
{
    /*
    FIXME: 同一のshader_registerが指定されたDESCRIPTOR_TABLEタイプのparametersにおいて、フラグにDESCRIPTOR_FLAG_DESCRIPTORS_UPDATE_AFTER_BINDビットが1つでも設定されている場合、
           その同一のshader_registerが指定されたすべてのparametersに、ROOT_PARAMETER_TYPE_DYNAMIC_DESCRIPTOR(DESCRIPTOR_TYPE_CBV_DYNAMICまたはDESCRIPTOR_TYPE_UAV_BUFFER_DYNAMICのディスクリプタタイプ)が含まれていてはなりません。
           現状、Vulkanの場合のみでの制約であり、D3D12ではそのような制約はありません。
    */

    ROOT_SIGNATURE_FLAGS                flags;
    RAY_TRACING_SHADER_VISIBILITY_FLAGS raytracing_shader_visibilities; // flagsにROOT_SIGNATURE_FLAG_RAY_TRACING_SHADER_VISIBILITYが指定されている場合、可視とするシェーダーをステージを指定します。
    uint32_t                            num_parameters;
    const ROOT_PARAMETER*               parameters;
    uint32_t                            num_static_samplers;
    const STATIC_SAMPLER*               static_samplers;
    uint32_t                            num_register_shifts;
    const SHADER_REGISTER_SHIFT*        register_shifts;
};

#pragma endregion root signature

#pragma region render pass

enum FRAMEBUFFER_FLAG : EnumT
{
    FRAMEBUFFER_FLAG_NONE = 0x0
};
using FRAMEBUFFER_FLAGS = EnumFlagsT;

struct FRAMEBUFFER_DESC
{
    FRAMEBUFFER_FLAGS flags;
    IRenderPass*      render_pass;
    uint32_t          num_attachments;  // render_passで指定されたアタッチメントの数と同じである必要があります。
    IView*const *     attachments;      // IRenderTargetView(color,resolve attachments)、IShaderResourceView(input attachments)またはIDepthStencilView(depth/stencil attachments)を指定します。
};

enum ATTACHMENT_FLAG : EnumT
{
      ATTACHMENT_FLAG_NONE      = 0x0
    , ATTACHMENT_FLAG_MAY_ALIAS = 0x1
};
using ATTACHMENT_FLAGS = EnumFlagsT;

enum ATTACHMENT_LOAD_OP : EnumT
{
      ATTACHMENT_LOAD_OP_LOAD       // レンダーパス開始時点でのデータが保持されます。 
    , ATTACHMENT_LOAD_OP_CLEAR      // レンダーパス開始時にバッファをクリアします。 RENDER_PASS_BEGIN_DESCで指定されたクリア値が適用されます。
    , ATTACHMENT_LOAD_OP_DONT_CARE  // レンダーパス開始時にバッファが破棄される可能性があり値は未定義になります。 バッファの以前の値が必要無い場合に効果的です。
};

enum ATTACHMENT_STORE_OP : EnumT
{
      ATTACHMENT_STORE_OP_STORE     // レンダーパス終了時にバッファに値が書き込まれることが保証されます。 
    , ATTACHMENT_STORE_OP_DONT_CARE // レンダーパス終了時にバッファに値が書き込まれることが保証されず、結果の値は未定義です。 
};

struct ATTACHMENT_DESC
{
    ATTACHMENT_FLAGS        flags;
    RESOURCE_FORMAT         format;
    uint32_t                sample_count;
    ATTACHMENT_LOAD_OP      load_op;                // このアタッチメントが最初に利用される際に行われる操作を指定します。
    ATTACHMENT_STORE_OP     store_op;               // レンダーパスが終了する際に行われる操作を指定します。
    ATTACHMENT_LOAD_OP      stencil_load_op;        // formatが深度ステンシルフォーマットの場合、ステンシルに対して操作を指定する場合に使用します。それ以外の場合、この値は無視されます。
    ATTACHMENT_STORE_OP     stencil_store_op;       // formatが深度ステンシルフォーマットの場合、ステンシルに対して操作を指定する場合に使用します。それ以外の場合、この値は無視されます。
    RESOURCE_STATE          begin_state;            // レンダーパス開始時の状態です。 実際にレンダーパスが開始される際、アタッチメントの状態はこの値である必要があります。 この状態に遷移するサブリソースの影響範囲は、FRAMEBUFFER_DESC::attachmentsの各要素のIViewが参照する全てのサブリソースによって定義されます。 
    RESOURCE_STATE          end_state;              // レンダーパス終了時の状態です。 終了時、アタッチメントはこの状態に遷移しています。 この状態に遷移するサブリソースの影響範囲は、FRAMEBUFFER_DESC::attachmentsの各要素のIViewが参照する全てのサブリソースによって定義されます。 
    RESOURCE_STATE          stencil_begin_state;    // 深度とステンシルに対して個別のレイアウトを指定する場合に使用します。それ以外の場合、RESOURCE_STATE_UNDEFINEDです。 この状態に遷移するサブリソースの影響範囲は、FRAMEBUFFER_DESC::attachmentsの各要素のIViewが参照する全てのステンシルサブリソースによって定義されます。 
    RESOURCE_STATE          stencil_end_state;      // 深度とステンシルに対して個別のレイアウトを指定する場合に使用します。それ以外の場合、RESOURCE_STATE_UNDEFINEDです。 この状態に遷移するサブリソースの影響範囲は、FRAMEBUFFER_DESC::attachmentsの各要素のIViewが参照する全てのステンシルサブリソースによって定義されます。 
};

struct ATTACHMENT_REFERENCE
{
    uint16_t                attachment_index;               // RENDER_PASS_DESC::attachments配列に対応するインデックスです。
    RESOURCE_STATE          state_at_pass;                  // サブパス内でのアタッチメントのレイアウトを指定します。 この状態に遷移するサブリソースの影響範囲は、FRAMEBUFFER_DESC::attachments[attachment_index]の各要素のIViewが参照する全てのサブリソースによって定義されます。 
    RESOURCE_STATE          stencil_state_at_pass;          // 深度とステンシルに対して個別のレイアウトを指定する(例えば、ステンシルのみをアタッチメントとして使用し、深度を読み取り専用にするビュー)場合に使用します。それ以外の場合、RESOURCE_STATE_UNDEFINEDです。 この状態に遷移するサブリソースの影響範囲は、FRAMEBUFFER_DESC::attachments[attachment_index]の各要素のIViewが参照する全てのステンシルサブリソースによって定義されます。 
    TEXTURE_ASPECT_FLAGS    input_attachment_aspect_mask;   // SUBPASS_DESC::input_attachmentsとして使用する際に使用するアスペクトです。(COLOR、DEPTH、STENCIL)
};

enum SUBPASS_FLAG : EnumT
{
    SUBPASS_FLAG_NONE = 0x0
};
using SUBPASS_FLAGS = EnumFlagsT;

enum PIPELINE_BIND_POINT : EnumT
{
      PIPELINE_BIND_POINT_GRAPHICS
    , PIPELINE_BIND_POINT_COMPUTE
    , PIPELINE_BIND_POINT_RAY_TRACING
};

enum DEPENDENCY_FLAG : EnumT
{
      DEPENDENCY_FLAG_NONE          = 0x0
    , DEPENDENCY_FLAG_BY_REGION     = 0x1
    , DEPENDENCY_FLAG_VIEW_LOCAL    = 0x2
};
using DEPENDENCY_FLAGS = EnumFlagsT;

// TODO: SUBPASS_EXTERNALの定義
struct SUBPASS_DESC
{
    SUBPASS_FLAGS               flags;
    PIPELINE_BIND_POINT         pipeline_bind_point;        // 現在、pipeline_bind_pointはPIPELINE_BIND_POINT_GRAPHICSである必要があります。
    uint32_t                    view_mask;                  // いずれかのビューマスクに有効なビットが存在する場合、マルチビューレンダリングが有効であると定義されます。 この場合、他の全てのサブパスでも有効なビットが存在する必要があります。
    uint32_t                    num_input_attachments;
    const ATTACHMENT_REFERENCE* input_attachments;
    uint32_t                    num_color_attachments;
    const ATTACHMENT_REFERENCE* color_attachments;
    const ATTACHMENT_REFERENCE* resolve_attachments;        // nullptr、ATTACHMENT_UNUSED(-1)でない限り、この要素のそれぞれがcolor_attachmentsの要素に対応し、アタッチメント毎にマルチサンプル解決操作が定義されます。
    const ATTACHMENT_REFERENCE* depth_stencil_attachment;
//  const ATTACHMENT_REFERENCE* resolve_depth_stencil_attachment;
    uint32_t                    num_preserve_attachment;
    const uint32_t*             preserve_attachments;       // 各要素のインデックスのアタッチメントは、このサブパスでは使用されず、このサブパス実行中において値の保持が保証されることを定義します。
};

struct SUBPASS_DEPENDENCY
{
    uint32_t                    src_subpass;
    uint32_t                    dst_subpass;
    PIPELINE_STAGE_FLAGS        src_stage_mask;
    PIPELINE_STAGE_FLAGS        dst_stage_mask;
    RESOURCE_ACCESS_FLAGS       src_access;
    RESOURCE_ACCESS_FLAGS       dst_access;
    DEPENDENCY_FLAGS            dependency_flags;// サブパスに複数のビューがある(popcount(view_mask) > 1)場合は、dependency_flagsにDEPENDENCY_FLAG_VIEW_LOCALを含める必要があります。

    /**
     * @brief 依存関係がビューローカルの場合、dstViewは、「ソースサブパスの」ビュー dstView + view_offset に依存します。 
     *        srcView, dstViewはそれぞれ、ソースサブパスと宛先サブパスに存在するビューのインデックスです。
     *        言い換えると、宛先で使用されている各ビューのビットは、view_offsetでシフトされます。 宛先の各ビューは、ソースで使用されている、シフトされた結果の各ビットに対応する各ビューに依存します。 
     *        この際、シフトされた結果の各ビットに対応するビューがソースで使用されていなかった(有効なビットが存在しなかった)場合、宛先のビューはそのビットのビューに依存しません。
    */
    int32_t                     view_offset; // DEPENDENCY_FLAG_VIEW_LOCALが指定されていない場合、値は0である必要があります。
};

enum RENDER_PASS_FLAG : EnumT
{
    RENDER_PASS_FLAG_NONE = 0x0
};
using RENDER_PASS_FLAGS = EnumFlagsT;

struct RENDER_PASS_DESC
{
    RENDER_PASS_FLAGS           flags;
    uint32_t                    num_attachments;
    const ATTACHMENT_DESC*      attachments;
    uint32_t                    num_subpasses;
    const SUBPASS_DESC*         subpasses;
    uint32_t                    num_dependencies;
    const SUBPASS_DEPENDENCY*   dependencies;
    uint32_t                    num_correlated_view_masks;
    const uint32_t*             correlated_view_masks; // 同時にレンダリングする方が効率的なビューのセットを示すビューマスクの配列へのポインタです。これはドライバーへの最適化のヒントであり、動作に影響を与えません。
};

enum SUBPASS_CONTENTS : EnumT
{
      SUBPASS_CONTENTS_INLINE
    , SUBPASS_CONTENTS_SECONDARY_COMMAND_LISTS
};

struct RENDER_PASS_BEGIN_DESC
{
    IRenderPass*            render_pass;
    IFramebuffer*           framebuffer;        // フレームバッファーの作成時に指定されたFRAMEBUFFER_DESCのrender_passメンバーと互換性がある必要があります。 
    uint32_t                num_clear_values;
    const CLEAR_VALUE*      clear_values;
};

struct SUBPASS_BEGIN_DESC
{
    SUBPASS_CONTENTS contents;
};

struct SUBPASS_END_DESC
{
    /* currently reserved structure */
};

#pragma endregion render pass

#pragma region pipeline

enum SHADER_MODULE_FLAG : EnumT
{
    SHADER_MODULE_FLAG_NONE = 0x0
};
using SHADER_MODULE_FLAGS = EnumFlagsT;

struct SHADER_BYTECODE
{
    size_t          bytecode_length;
    const void*     shader_bytecode;
};

struct SHADER_MODULE_DESC
{
    SHADER_MODULE_FLAGS flags;
    SHADER_BYTECODE     bytecode;
};

struct SO_DECLARATION_ENTRY
{
    uint32_t        stream;         // どの出力バッファ（スロット）に送信されるかを指定します。
    const char*     semantic_name;  // セマンティック名。1つのインデックスにつき、最大4コンポーネント(xyzw)を表現出来ます。(float4x4の場合、4コンポーネントの、(インデックスのみ異なる)同じセマンティックが4つ存在します。)
    uint32_t        semantic_index; // セマンティック名に関するインデックスです。(例: MATRIXi の "i")
    uint8_t         start_component;// セマンティック名に関する、書き込みを開始するエントリのコンポーネントのオフセットです。
    uint8_t         component_count;// start_componentからのコンポーネント数です。[start_component, component_count) 
    uint8_t         output_slot;    // パイプラインにバインドされている関連付けられたストリーム出力バッファーのスロットです。buffer_strides[output_slot]を超えるデータは書き込まれません。 XfbBuffers
};

struct STREAM_OUTPUT_DESC
{
    uint32_t                       num_entries;
    const SO_DECLARATION_ENTRY*    entries;             // ...xfb_offset,xfb_stride  
    uint32_t                       num_buffer_strides;  // vkCmdBindTransformFeedbackBuffersEXT::bindingCount
    const uint32_t*                buffer_strides;      // ストリーム出力時の各頂点の開始間の間隔。 ストリーム出力中に、ストリーム出力構造のサイズを超える量はメモリ内で変更されません。 vkCmdBindTransformFeedbackBuffersEXT::pSizes 
    uint32_t                       rasterized_stream;   // VkPipelineRasterizationStateStreamCreateInfoEXT::rasterizationStream
};

enum STENCIL_OP : EnumT
{
      STENCIL_OP_KEEP                   // 現在の値を保持します。 (STENCIL_OP_KEEP)
    , STENCIL_OP_ZERO                   // 値を0に設定します。 (STENCIL_OP_ZERO)
    , STENCIL_OP_REPLACE                // 値をDEPTH_STENCILOP_DESC::referenceに設定します。 (STENCIL_OP_REPLACE)
    , STENCIL_OP_INCREMENT_AND_CLAMP    // 現在の値をインクリメントし、表現可能な最大の符号なし値にクランプします。 (STENCIL_OP_INCR_SAT)
    , STENCIL_OP_DECREMENT_AND_CLAMP    // 現在の値をデクリメントし、0にクランプします。 (STENCIL_OP_DECR_SAT)
    , STENCIL_OP_INVERT                 // 現在のステンシル値を反転します。(STENCIL_OP_INVERT)
    , STENCIL_OP_INCREMENT_AND_WRAP     // 現在の値をインクリメントし、最大値を超えた場合は0にラップします。 (STENCIL_OP_INCR)
    , STENCIL_OP_DECREMENT_AND_WRAP     // 現在の値をデクリメントし、値が0を下回ると、表現可能な最大値にラップします。 (STENCIL_OP_DECR)
};

struct DEPTH_STENCILOP_DESC
{
    STENCIL_OP      fail_op;
    STENCIL_OP      depth_fail_op;
    STENCIL_OP      pass_op;
    COMPARISON_FUNC comparison_func;

    /**
     * @brief ステンシルのreference値とアタッチメントの値srとsaは、論理AND演算を使用して、compare_mask scとそれぞれ独立して結合され、マスクされたreference値とアタッチメントの値s'rとs'aが作成されます。 
     *        s'rとs'aは、COMPARISON_FUNCで指定された操作で、それぞれAとBとして使用されます。 (StencilReadMask)
    */
    uint32_t        compare_mask;

    /**
     * @brief ステンシルアタッチメント値saは、write_maskによって次のように定義された書き込みマスクswに従って、生成されたステンシル値sgで更新されます。
    */
    uint32_t        write_mask;

    /**
     * @brief ステンシルテストでは、各サンプルのフレームバッファ座標(xf、yf)およびサンプルインデックスiでの深度/ステンシルアタッチメントのステンシルアタッチメント値saを、ステンシルreference値と比較します。 (SetStencilReference)
    */
    uint32_t        reference;
};

struct DEPTH_STENCIL_STATE_DESC
{
    bool                    is_enabled_depth_test;
    bool                    is_enabled_depth_write;
    COMPARISON_FUNC         depth_comparison_func;
    bool                    is_enabled_depth_bounds_test;
    float                   min_depth_bounds;
    float                   max_depth_bounds;
    bool                    is_enabled_stencil_test;
    DEPTH_STENCILOP_DESC    stencil_front_face;             // D3D12の場合、stencil _front/_back _face.compare_mask,write_mask,reference は同一の値である必要がありますが、暗黙的にRASTERIZATION_STATE_DESC::cull_modeに指定されていない面のパラメータを優先します。(CULL_MODE_NONEの場合、背面が優先されます。)
    DEPTH_STENCILOP_DESC    stencil_back_face;              // D3D12の場合、stencil _front/_back _face.compare_mask,write_mask,reference は同一の値である必要がありますが、暗黙的にRASTERIZATION_STATE_DESC::cull_modeに指定されていない面のパラメータを優先します。(CULL_MODE_NONEの場合、背面が優先されます。)
};

/**
 * @brief Rs0、Gs0、Bs0、As0は、任意の出力のソースカラー、またはデュアルソースブレンドモードで使用されるソースカラー0のR、G、B、Aコンポーネントをそれぞれ表します。 
 *        Rs1、Gs1、Bs1、As1は、デュアルソースブレンドモードで使用されるソースカラー1のR、G、B、Aコンポーネントをそれぞれ表します。 
 *        Rd、Gd、Bd、Adは、宛先の色のR、G、B、Aコンポーネントを表します。 これは、セットされているカラーアタッチメントに現在ある色です。 
 *        Rc、Gc、Bc、Acは、それぞれブレンド定数のR、G、B、Aコンポーネントを表します。 (BLEND_STATE_DESC::blend_constants)
 * @remark デュアルソースブレンド機能により、出力0と出力1の両方を、アタッチメント0とのブレンド操作への入力ソースとして同時に使用できます。 この機能を利用するには、SRC1の列挙を使用します。
 *         Vulkanによる追加の使用法として、ソース1を"blending unit (blender)"のインデックス1へ入力することを指定する必要があり、
 *         hlslでSV_Target1のセマンティックを持つ出力に、[[vk::index(1)]]の装飾を追加する必要があります: [[vk::index(1)]] float4 output1 : SV_Target1;
 *         D3Dでは、ソース1がSV_Target1として定義されます。
*/
enum BLEND_FACTOR : EnumT
{
                                            // (   R ,    G ,    B )                     ,    A
      BLEND_FACTOR_ZERO                     // (   0 ,    0 ,    0 )                     ,    0
    , BLEND_FACTOR_ONE                      // (   1 ,    1 ,    1 )                     ,    1
    , BLEND_FACTOR_SRC_COLOR                // (  Rs0,   Gs0,   Bs0)                     ,   As0
    , BLEND_FACTOR_SRC_COLOR_INVERTED       // (1-Rs0, 1-Gs0, 1-Bs0)                     , 1-As0
    , BLEND_FACTOR_DST_COLOR                // (  Rd ,   Gd ,   Bd )                     ,   Ad
    , BLEND_FACTOR_DST_COLOR_INVERTED       // (1-Rd , 1-Gd , 1-Bd )                     , 1-Ad
    , BLEND_FACTOR_SRC_ALPHA                // (  As0,   As0,   As0)                     ,   As0
    , BLEND_FACTOR_SRC_ALPHA_INVERTED       // (1-As0, 1-As0, 1-As0)                     , 1-As0
    , BLEND_FACTOR_SRC_ALPHA_SATURATE       // (   f ,    f ,    f );f = min(As0, 1-Ad)  ,    1
    , BLEND_FACTOR_DST_ALPHA                // (  Ad ,   Ad ,   Ad )                     ,   Ad
    , BLEND_FACTOR_DST_ALPHA_INVERTED       // (1-Ad , 1-Ad , 1-Ad )                     , 1-Ad
    , BLEND_FACTOR_SRC1_COLOR               // (  Rs1,   Gs1,   Bs1)                     ,   As1
    , BLEND_FACTOR_SRC1_COLOR_INVERTED      // (1-Rs1, 1-Gs1, 1-Bs1)                     , 1-As1
    , BLEND_FACTOR_SRC1_ALPHA               // (  As1,   As1,   As1)                     ,   As1
    , BLEND_FACTOR_SRC1_ALPHA_INVERTED      // (1-As1, 1-As1, 1-As1)                     , 1-As1
    , BLEND_FACTOR_BLEND_CONSTANT           // (  Rc ,   Gc ,   Bc )                     ,   Ac
    , BLEND_FACTOR_BLEND_CONSTANT_INVERTED  // (1-Rc , 1-Gc , 1-Bc )                     , 1-Ac
};

/**
 * @brief Rs0、Gs0、Bs0、As0は、それぞれ最初のソースカラーのR、G、B、Aコンポーネントを表します。 
          Rd、Gd、Bd、Adは、宛先の色のR、G、B、Aコンポーネントを表します。 セットされているカラーアタッチメントに現在ある色です。 
          Sr、Sg、Sb、Saは、それぞれソースブレンドファクターのR、G、B、Aコンポーネントを表します。 
          Dr、Dg、Db、Daは、それぞれ宛先ブレンドファクターのR、G、B、Aコンポーネントを表します。 
*/
enum BLEND_OP : EnumT
{
      BLEND_OP_ADD               // R = Rs0×Sr + Rd×Dr; G = Gs0 × Sg + Gd×Dg; B = Bs0×Sb + Bd×Db; A = As0×Sa + Ad×Da;
    , BLEND_OP_SUBTRACT          // R = Rs0×Sr - Rd×Dr; G = Gs0 × Sg - Gd×Dg; B = Bs0×Sb - Bd×Db; A = As0×Sa - Ad×Da;
    , BLEND_OP_REVERSE_SUBTRACT  // R = Rd×Dr - Rs0×Sr; G = Gd×Dg - Gs0×Sg;   B = Bd×Db - Bs0×Sb; A = Ad×Da - As0×Sa;
    , BLEND_OP_MIN               // R = min(Rs0, Rd);   G = min(Gs0, Gd);     B = min(Bs0, Bd);   A = min(As0, Ad);
    , BLEND_OP_MAX               // R = max(Rs0, Rd);   G = max(Gs0, Gd);     B = max(Bs0, Bd);   A = max(As0, Ad);
};

enum COLOR_WRITE_FLAG : Enum8T
{
      COLOR_WRITE_FLAG_NONE     = 0x0
    , COLOR_WRITE_FLAG_RED      = 0x1
    , COLOR_WRITE_FLAG_GREEN    = 0x2
    , COLOR_WRITE_FLAG_BLUE     = 0x4
    , COLOR_WRITE_FLAG_ALPHA    = 0x8
    , COLOR_WRITE_FLAG_ALL      = COLOR_WRITE_FLAG_RED | COLOR_WRITE_FLAG_GREEN | COLOR_WRITE_FLAG_BLUE | COLOR_WRITE_FLAG_ALPHA
};
using COLOR_WRITE_FLAGS = EnumFlags8T;

/**
 * @brief sは、更新されるカラーアタッチメントに対応する出力のRs0、Gs0、Bs0、またはAs0コンポーネント値です。
          dは、カラーアタッチメントのR、G、B、またはAコンポーネントの値です。
*/
enum LOGIC_OP : EnumT
{
      LOGIC_OP_CLEAR            // all 0
    , LOGIC_OP_SET              // all 1s
    , LOGIC_OP_COPY             //     s
    , LOGIC_OP_COPY_INVERTED    //    ~s
    , LOGIC_OP_NO_OP            //     d
    , LOGIC_OP_INVERT           //    ~d
    , LOGIC_OP_AND              //     s &  d
    , LOGIC_OP_NAND             //  ~( s &  d)   
    , LOGIC_OP_OR               //     s |  d
    , LOGIC_OP_NOR              //  ~( s |  d)
    , LOGIC_OP_XOR              //     s ^  d
    , LOGIC_OP_EQUIVALENT       //  ~( s ^  d)
    , LOGIC_OP_AND_REVERSE      //     s & ~d
    , LOGIC_OP_AND_INVERTED     //    ~s &  d
    , LOGIC_OP_OR_REVERSE       //     s | ~d
    , LOGIC_OP_OR_INVERTED      //    ~s |  d
};

struct RENDER_TARGET_BLEND_DESC
{
    bool                        is_enabled_blend;
    BLEND_FACTOR                src_blend;
    BLEND_FACTOR                dst_blend;
    BLEND_OP                    blend_op;
    BLEND_FACTOR                src_blend_alpha;
    BLEND_FACTOR                dst_blend_alpha;
    BLEND_OP                    blend_op_alpha;
    COLOR_WRITE_FLAGS           color_write_mask;
};

struct BLEND_STATE_DESC
{
    bool                                is_enabled_independent_blend;   // 各attachments毎に異なるブレンドパラメータを指定可能にします。is_enabled_logic_opはfalseである必要があります。
    bool                                is_enabled_logic_op;            // 論理演算によって値を操作する場合に指定します。この際、attachments[0].is_enabled_blendはfalseである必要があります。
    LOGIC_OP                            logic_op;                       // is_enabled_logic_opがtrueの際に使用する、論理演算の操作です。
    uint32_t                            num_attachments;                // is_enabled_independent_blendがtrueの場合、attachments配列の要素数として扱われますが、is_enabled_independent_blendの値に関係なく、サブパスのnum_color_attachmentsと等しい必要があります。
    const RENDER_TARGET_BLEND_DESC*     attachments;                    // is_enabled_independent_blendがfalseの場合、配列の先頭の要素のみが参照され、以降の値が無視されます。従って、is_enabled_independent_blendがfalseの場合に限りnum_attachments以下の要素数の配列を指定することは有効です。
    COLOR4                              blend_constants;
};

enum PRIMITIVE_TOPOLOGY : EnumT
{
      PRIMITIVE_TOPOLOGY_UNDEFINED
    , PRIMITIVE_TOPOLOGY_POINT_LIST                 // VK_PRIMITIVE_TOPOLOGY_POINT_LIST
    , PRIMITIVE_TOPOLOGY_LINE_LIST                  // VK_PRIMITIVE_TOPOLOGY_LINE_LIST
    , PRIMITIVE_TOPOLOGY_LINE_STRIP                 // VK_PRIMITIVE_TOPOLOGY_LINE_STRIP
    , PRIMITIVE_TOPOLOGY_TRIANGLE_LIST              // VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST
    , PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP             // VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP
    , PRIMITIVE_TOPOLOGY_LINE_LIST_ADJACENCY        // VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY
    , PRIMITIVE_TOPOLOGY_LINE_STRIP_ADJACENCY       // VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY
    , PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_ADJACENCY    // VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY
    , PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_ADJACENCY   // VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY
    , PRIMITIVE_TOPOLOGY_PATCH_LIST                 // VK_PRIMITIVE_TOPOLOGY_PATCH_LIST
};

enum INPUT_CLASSIFICATION : EnumT
{
      INPUT_CLASSIFICATION_PER_VERTEX_DATA   // 描画インスタンスの頂点データは全てのインスタンスに対して同一です。 instance_data_step_rateは0である必要があります。
    , INPUT_CLASSIFICATION_PER_INSTANCE_DATA // 描画インスタンスの頂点データがinstance_data_step_rate毎にオフセットされます。instance_data_step_rateが0の場合、同じ頂点属性がすべてのインスタンスに適用されます。
};

struct INPUT_ELEMENT_DESC
{
    const char*                 semantic_name;          // セマンティック名です。 (float4x4の場合、同じセマンティック名であり、異なるsemantic_indexを持つ4コンポーネントINPUT_ELEMENT_DESCが4つ必要です。)
    uint32_t                    semantic_index;         // セマンティック名に関するインデックスです。 1つのインデックスにつき32ビットの型を最大4コンポーネント(xyzw)表現出来ます。
    RESOURCE_FORMAT             format;                 // この要素のフォーマットです。 VkVertexInputAttributeDescription::format
    uint32_t                    aligned_byte_offset;    // 指定のスロットのデータの開始(0)からこの要素へのバイトオフセットです。値は4バイトに整列されている必要があります。 B3D_APPEND_ALIGNED_ELEMENTを指定した場合、INPUT_SLOT_DESC::elementsの要素の順序で、input_slotへの4バイトアラインオフセットが自動的に計算されます。 VkVertexInputAttributeDescription::offset 
};

struct INPUT_SLOT_DESC
{
    /**
     * @brief スロット番号を指定します。 
     * @note slot_numberと、hlslでの頂点入力の順序の不一致に注意してください。 
     *       hlslでは、slot_number 0 のスロットから順番に展開され、そのスロット内の各エレメントはaligned_byte_offsetに従って、またはB3D_APPEND_ALIGNED_ELEMENTが指定されている場合は配列の要素の順番に展開されます。
    */
    uint32_t                    slot_number;
    uint32_t                    stride_in_bytes;        // 頂点毎の要素間のバイト単位のサイズです。バインドする頂点バッファビューの頂点ストライドは、この値と同一である必要があります。 VkVertexInputBindingDescription::stride
    INPUT_CLASSIFICATION        classification;         // VkVertexInputBindingDescription::inputRate
    uint32_t                    instance_data_step_rate;// VkVertexInputBindingDivisorDescriptionEXT::divisor 
    uint32_t                    num_elements;
    const INPUT_ELEMENT_DESC*   elements;
};

struct INPUT_LAYOUT_DESC
{
    uint32_t                    num_input_slots;
    const INPUT_SLOT_DESC*      input_slots;
};

enum FILL_MODE : EnumT
{
      FILL_MODE_POINT
    , FILL_MODE_SOLID
    , FILL_MODE_WIREFRAME
};

enum CULL_MODE : EnumT
{
      CULL_MODE_NONE
    , CULL_MODE_FRONT
    , CULL_MODE_BACK
};

enum LINE_RASTERIZATION_MODE : EnumT
{
      LINE_RASTERIZATION_MODE_DEFAULT               // MultisampleEnable = true , AntialiasedLineEnable = false
    , LINE_RASTERIZATION_MODE_ALIASED               // MultisampleEnable = false, AntialiasedLineEnable = false
    , LINE_RASTERIZATION_MODE_RECTANGULAR           // MultisampleEnable = true , AntialiasedLineEnable = false
    , LINE_RASTERIZATION_MODE_RECTANGULAR_SMOOTH    // MultisampleEnable = false, AntialiasedLineEnable = true
};

struct RASTERIZATION_STATE_DESC
{
    FILL_MODE               fill_mode;
    CULL_MODE               cull_mode;
    bool                    is_front_counter_clockwise;
    bool                    is_enabled_depth_clip;          // VkPipelineRasterizationDepthClipStateCreateInfoEXT::depthClipEnable
    bool                    is_enabled_depth_bias;          // VkPipelineRasterizationStateCreateInfo::depthBiasEnable
    int32_t                 depth_bias_scale;               // is_enabled_depth_biasがfalseの場合、この値は0である必要があります。 VkPipelineRasterizationStateCreateInfo::depthBiasConstantFactor
    float                   depth_bias_clamp;               // is_enabled_depth_biasがfalseの場合、この値は0である必要があります。 VkPipelineRasterizationStateCreateInfo::depthBiasClamp
    float                   depth_bias_slope_scale;         // is_enabled_depth_biasがfalseの場合、この値は0である必要があります。 VkPipelineRasterizationStateCreateInfo::depthBiasSlopeFactor
    bool                    is_enabled_conservative_raster; // VkPipelineRasterizationConservativeStateCreateInfoEXT 
    LINE_RASTERIZATION_MODE line_rasterization_mode;        // ラインレンダリング時に使用されます。
    float                   line_width;                     // D3D12では、ライン幅が1に固定されるため、値は無視されます。
};

enum SHADING_RATE : EnumT
{
      SHADING_RATE_ONE_INVOCATION_PER_1X1
    , SHADING_RATE_ONE_INVOCATION_PER_1X2
    , SHADING_RATE_ONE_INVOCATION_PER_2X1
    , SHADING_RATE_ONE_INVOCATION_PER_2X2
    , SHADING_RATE_ONE_INVOCATION_PER_2X4
    , SHADING_RATE_ONE_INVOCATION_PER_4X2
    , SHADING_RATE_ONE_INVOCATION_PER_4X4
};

enum PIPELINE_STATE_FLAG : EnumT
{
    PIPELINE_STATE_FLAG_NONE = 0x0
};
using PIPELINE_STATE_FLAGS = EnumFlagsT;

enum PIPELINE_SHADER_STAGE_FLAG : EnumT
{
    PIPELINE_SHADER_STAGE_FLAG_NONE = 0x0
};
using PIPELINE_SHADER_STAGE_FLAGS = EnumFlagsT;

struct PIPELINE_SHADER_STAGE_DESC
{
    PIPELINE_SHADER_STAGE_FLAGS flags;
    SHADER_STAGE_FLAG           stage;  // このシェーダーのステージタイプを表す単一のビットを指定します。
    IShaderModule*              module;
    const char*                 entry_point_name;
};

struct INPUT_ASSEMBLY_STATE_DESC
{
    PRIMITIVE_TOPOLOGY topology;
};

struct TESSELLATION_STATE_DESC
{
    uint32_t patch_control_points; // パッチ制御点の数は0以上である必要があります。
};

struct VIEWPORT_STATE_DESC
{
    uint32_t            num_viewports;      // このパイプラインで使用するビューポートの数です。DYNAMIC_STATE_VIEWPORTが有効かどうかに関わらず指定する必要があります。
    const VIEWPORT*     viewports;          // DYNAMIC_STATE_VIEWPORTが無効な場合に使用するビューポートの配列です。 DYNAMIC_STATE_VIEWPORTが有効の場合、値はnullptrである必要があります。
    uint32_t            num_scissor_rects;  // このパイプラインで使用するシザー矩形の数です。  DYNAMIC_STATE_SCISSORが有効かどうかに関わらず指定する必要があります。
    const SCISSOR_RECT* scissor_rects;      // DYNAMIC_STATE_SCISSORが無効な場合に使用するシザー矩形の配列です。 DYNAMIC_STATE_SCISSORが有効の場合、値はnullptrである必要があります。
};

/**
 * @brief (0,0)はピクセルの左上、(1,1)はピクセルの右下です。
*/
struct SAMPLE_POSITION
{
    float x;
    float y;
};

struct SAMPLE_POSITION_DESC
{
    uint32_t                sample_positions_per_pixel; // NumSamplesPerPixel
    EXTENT2D                sample_position_grid_size;  // NumPixels
    uint32_t                num_sample_positions;       // NumSamplesPerPixel * NumPixels
    const SAMPLE_POSITION*  sample_positions;           // pSamplePositions
};

struct SAMPLE_POSITION_STATE_DESC
{
    bool                        is_enabled;
    const SAMPLE_POSITION_DESC* desc;
};

struct MULTISAMPLE_STATE_DESC
{
    /**
     * @brief UAVへのマルチサンプルレンダリング、またはカラーアタッチメントのリソース以上のサンプル数でマルチサンプルされた値を書き込む場合にサンプル数を変更します。 
     *        深度ステンシルアタッチメントを使用せず、深度/ステンシルテストが有効でないレンダーパスである必要があります。それ以外の場合この値はサブパスで使用する各アタッチメントのサンプル数を指定する必要があります。 
     * @remark FIXME: VulkanのValidUsageに基づき、AMDのハードウェアが使用されている場合、上記の制限に加えてカラーアタッチメントは使用出来ず、UAVへの書き込みのみが有効です。
     *                D3D12では、ForcedSampleCountを有効にする場合、カラーアタッチメントのリソースのサンプル数が1である必要があります。
    */
    uint32_t            rasterization_samples;

    /**
     * @brief カバレッジマスクとAND演算によって比較(テスト)されるビットマスクです。比較に成功した場合｢サンプルカバレッジマスク｣のビットが有効になります。 
     *        32サンプルカウント毎に1つのsample_maskが必要です。nullptrの場合、デフォルトで全てのビットに1が指定されているかのようになります。 
     * @remark ｢カバレッジマスク｣は、ラスタライズ時に、1ピクセル内の各サンプル位置にプリミティブが存在したかを表現するビットマスクであり、(サンプル数とは関係無く)各ピクセルシェーダーに対して生成されます。
     *         ｢サンプルカバレッジマスク(カラーサンプルカバレッジ)｣は、AND演算後のマスクの結果です。 このマスクの各ビットは、1ピクセル内の各サンプル位置に対応するインデックスとして表現されます。
     *         対応するビットの値が1のサンプル位置を含むピクセルシェーダーの結果は、出力マージステージ(フレームバッファステージ)に到達します。
    */
    const SampleMask*   sample_masks;

    /**
     * @brief 特定の場合を除き、SV_Target0出力のアルファ値を実装依存の変換を介してビットマスクに変換し、ラスタライザからのカバレッジマスクとAND演算を行う事を有効にします。
     * @remark この機能が無効となる特定の場合として、SV_Coverageの出力が存在する場合、SV_Target0では無い場合、または、SV_Target0出力が出力構造のメンバの先頭に存在しない場合が該当します。
     * @note この機能が有効である場合でも、sample_masksの動作は依然として実行されます。
    */
    bool                is_enabled_alpha_to_coverage;

    /**
     * @brief Sample-frequency execution を有効にし、hlsl入力構造体のメンバに sample補間修飾子、また SV_SampleIndexを持つ入力を宣言できるかどうかを指定します。 https://docs.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-struct#interpolation-modifiers-introduced-in-shader-model-4
     * @remark D3D12では、rasterization_samplesにアタッチメントと異なるサンプル数が指定されている場合(ForcedSampleCount != 0)、この機能をオフにする必要があります。
     * @note  Direct3DではVkPipelineMultisampleStateCreateInfo::minSampleShadingと同等の機能はありませんでした。(sample補間修飾子の無い入力構造はPixel-frequencyで実行されます。)
    */
    bool                is_enabled_sample_rate_shading;

    SAMPLE_POSITION_STATE_DESC sample_position_state;
};

enum DYNAMIC_STATE : EnumT
{
      DYNAMIC_STATE_VIEWPORT                      // RSSetViewports
    , DYNAMIC_STATE_SCISSOR                       // RSSetScissorRects
    , DYNAMIC_STATE_BLEND_CONSTANTS               // OMSetBlendFactor
    , DYNAMIC_STATE_DEPTH_BOUNDS                  // OMSetDepthBounds
    , DYNAMIC_STATE_STENCIL_REFERENCE             // OMSetStencilRef
    , DYNAMIC_STATE_SAMPLE_POSITIONS              // SetSamplePositions _EXT
    , DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE   // IASetVertexBuffers::pBufferStrides _EXT
    , DYNAMIC_STATE_VIEWPORT_SHADING_RATE         // RSSetShadingRate _NV 
    , DYNAMIC_STATE_LINE_WIDTH                    // Vulkan only
    , DYNAMIC_STATE_DEPTH_BIAS                    // Vulkan only
    , DYNAMIC_STATE_STENCIL_COMPARE_MASK          // Vulkan only
    , DYNAMIC_STATE_STENCIL_WRITE_MASK            // Vulkan only
    //, DYNAMIC_STATE_VIEWPORT_W_SCALING            // Vulkan only _NV  
    //, DYNAMIC_STATE_VIEWPORT_COARSE_SAMPLE_ORDER  // Vulkan only RSSetShadingRate _NV 

    , DYNAMIC_STATE_END
};

struct DYNAMIC_STATE_DESC
{
    uint32_t             num_dynamic_states;
    const DYNAMIC_STATE* dynamic_states;
};

struct GRAPHICS_PIPELINE_STATE_DESC
{
    IRootSignature*                             root_signature;
    IRenderPass*                                render_pass;
    uint32_t                                    subpass;
    NodeMask                                    node_mask; // パイプラインステートオブジェクトが構築されるノードを示す単一のビットを指定します。
    PIPELINE_STATE_FLAGS                        flags;

    uint32_t                                    num_shader_stages;
    const PIPELINE_SHADER_STAGE_DESC*           shader_stages;
    const INPUT_LAYOUT_DESC*                    input_layout;
    const INPUT_ASSEMBLY_STATE_DESC*            input_assembly_state;
    const TESSELLATION_STATE_DESC*              tessellation_state; // PRIMITIVE_TOPOLOGY_PATCH_LISTの場合に使用します。
    const VIEWPORT_STATE_DESC*                  viewport_state;
    const RASTERIZATION_STATE_DESC*             rasterization_state;
    const STREAM_OUTPUT_DESC*                   stream_output;      // 現在はD3D12のみの対応です。
    const MULTISAMPLE_STATE_DESC*               multisample_state;
    const DEPTH_STENCIL_STATE_DESC*             depth_stencil_state;
    const BLEND_STATE_DESC*                     blend_state;
    const DYNAMIC_STATE_DESC*                   dynamic_state;
};

struct COMPUTE_PIPELINE_STATE_DESC
{
    IRootSignature*             root_signature;
    NodeMask                    node_mask;
    PIPELINE_SHADER_STAGE_DESC  shader_stage;
};

struct RAY_TRACING_PIPELINE_STATE_DESC
{
    // TODO: RAY_TRACING_PIPELINE_STATE_DESC
};


#pragma endregion pipeline

#pragma region command list arguments

struct CMD_PUSH_32BIT_CONSTANTS
{
    uint32_t    root_parameter_index;
    uint32_t    num32_bit_values_to_set;
    const void* src_data;
    uint32_t    dst_offset_in_32bit_values;
};

struct DYNAMIC_DESCRIPTOR_OFFSET
{
    uint32_t    root_parameter_index;
    uint32_t    offset;
};

struct CMD_BIND_DESCRIPTOR_SET
{
    IDescriptorSet*                     descriptor_set;
    uint32_t                            num_dynamic_descriptor_offsets; // 更新する動的ディスクリプタのオフセットの配列の要素数です。 
    const DYNAMIC_DESCRIPTOR_OFFSET*    dynamic_descriptor_offsets;     // 更新する動的ディスクリプタのオフセットの配列です。 
};

struct QUERY_DESC
{
    IQueryHeap* query_heap;                 // query_indexのクエリが格納されるクエリヒープです。
    uint32_t    query_index;                // query_heap内のクエリのインデックスです。
    QUERY_FLAGS flags;                      // クエリの動作を制御するフラグです。
    uint32_t    so_statistics_stream_index; // QUERY_HEAP_TYPE_SO_STATISTICSクエリの場合に使用するストリーム出力のインデックスです。
};

struct CMD_RESET_QUERY_HEAP_RANGE
{
    IQueryHeap* query_heap;
    uint32_t    first_query;
    uint32_t    num_queries;
};

struct CMD_WRITE_ACCELERATION_STRUCTURE
{
    const QUERY_DESC*                       query_desc;
    uint32_t                                num_acceleration_structures;
    const IAccelerationStructure*const *    acceleration_structures;
};

struct CMD_RESOLVE_QUERY_DATA
{
    const QUERY_DESC*   first_query; // 結果を取得する最初のクエリです。 この最初のクエリに続くnum_queries数のクエリはQUERY_DESC::query_indexを除く全ての値が同一である必要があります。
    uint32_t            num_queries; // 結果を取得するクエリの数です。first_queryからの範囲を指定します。
    IBuffer*            dst_buffer;
    uint64_t            dst_buffer_offset;
};

struct CMD_BIND_VERTEX_BUFFER_VIEWS
{
    uint32_t                    start_slot;
    uint32_t                    num_views;
    IVertexBufferView*const *   views;
};

struct CMD_BIND_STREAM_OUTPUT_BUFFER_VIEWS
{
    uint32_t                            start_slot;
    uint32_t                            num_views;
    IStreamOutputBufferView*const *     views;
};

struct CMD_BEGIN_STREAM_OUTPUT
{
    /* currently reserved structure */
};

struct CMD_END_STREAM_OUTPUT
{
    /* currently reserved structure */
};

struct CMD_EXECUTE_INDIRECT
{
    ICommandSignature*  command_signature;
    uint32_t            max_command_count;
    IResource*          argument_buffer;
    uint64_t            argument_buffer_offset;
    IResource*          count_buffer;
    uint64_t            count_buffer_offset;
};

struct BUFFER_COPY_REGION
{
    uint64_t    src_offset;
    uint64_t    dst_offset;
    uint64_t    size_in_bytes;
};

struct CMD_COPY_BUFFER_REGION
{
    IBuffer*                    src_buffer;
    IBuffer*                    dst_buffer;
    uint32_t                    num_regions;
    const BUFFER_COPY_REGION*   regions;
};

struct SUBRESOURCE_ARRAY
{
    SUBRESOURCE_OFFSET offset; // コピーする最初のミップレベル、配列オフセット、アスペクトを指定します。 offset.array_sliceは3Dテクスチャのzオフセットを表現するものではありません。

    /**
     * @brief ソース、または宛先のoffset.array_sliceからの、コピーする配列数です。 
     *        3Dテクスチャの場合、この値は1である必要があります; 3Dテクスチャのdepthを表現するものではありません(代わりに、TEXTURE_COPY_REGION::copy_extent->depthを使用します)。
     *        ソース、または宛先のテクスチャの次元に応じて、ソース、または宛先それぞれのarray_countまたはcopy_extent->depthの値は、(ソース、または宛先の)array_countまたはcopy_extent->depthと同一である必要があります。
    */
    uint32_t           array_count;
};

struct TEXTURE_COPY_REGION
{
    SUBRESOURCE_ARRAY   src_subresource;    // マルチプラナーフォーマット用アスペクト以外の場合、.offset.aspectはdst_subresource.offset.aspectと一致している必要があります。
    const OFFSET3D*     src_offset;         // nullptrの場合、(0,0,0)が使用されます。 3Dテクスチャの以外の場合、zの値は0にする必要があります(1Dの場合、yも同様です)。
    SUBRESOURCE_ARRAY   dst_subresource;    // マルチプラナーフォーマット用アスペクト以外の場合、.offset.aspectはsrc_subresource.offset.aspectと一致している必要があります。
    const OFFSET3D*     dst_offset;         // nullptrの場合、(0,0,0)が使用されます。 3Dテクスチャの以外の場合、zの値は0にする必要があります(1Dの場合、yも同様です)。

    /**
     * @brief コピーするソースの範囲です。 nullptrの場合ソースのサイズが指定されます。 
     *        宛先、ソースのサイズを超えてはなりません。 3Dテクスチャの以外の場合、depthの値は1にする必要があります(1Dの場合、heightも同様です)。
     * @note D3D12で深度ステンシルまたはマルチサンプルリソースでCopyTextureRegionを使用する場合は、サブリソースの矩形全体をコピーする必要があり、オフセットは全て0である必要があります。
     *       TODO: D3D12の制約に従うか、あるいはオプションによってアプリケーションにコピー時の制約を判断出来るようにAPIを追加します。
    */
    const EXTENT3D*     copy_extent;
};

struct CMD_COPY_TEXTURE_REGION
{
    ITexture*                   src_texture; 
    ITexture*                   dst_texture; 
    uint32_t                    num_regions;
    const TEXTURE_COPY_REGION*  regions;
};

struct BUFFER_SUBRESOURCE_LAYOUT
{
    uint64_t                    offset;          // 画像データのコピー元またはコピー先のバッファオブジェクトの先頭からのバイト単位のオフセットです。

    // buffer_row_lengthとbuffer_image_heightは、バッファメモリ内のより大きな2次元または3次元画像のサブ領域(subregion)をテクセルで指定し、アドレッシング計算を制御します。
    // これらの値のいずれかがゼロの場合、バッファメモリのそのアスペクトは、imageExtentに従って密にパックされていると見なされます。
    uint64_t                    row_pitch;      // バッファに配置されている画像データのレイアウトを指定します。 
    uint32_t                    texture_height; // D3D12の場合、この値は0です。(texture_extent.heightに固定されます。)
};

struct BUFFER_TEXTURE_COPY_REGION
{
    BUFFER_SUBRESOURCE_LAYOUT       buffer_layout;
    SUBRESOURCE_ARRAY               texture_subresource;
    OFFSET3D*                       texture_offset;     // ソース、または宛先のオフセットです。
    EXTENT3D*                       texture_extent;     // ソース、または宛先のオフセットからのサイズです。
};

struct CMD_COPY_BUFFER_TO_TEXTURE
{
    IBuffer*                            src_buffer;
    ITexture*                           dst_texture;
    uint32_t                            num_regions;
    const BUFFER_TEXTURE_COPY_REGION*   regions;    // 各要素のoffset.aspectには単一のビットのみが含まれている必要があります(例えば、深度とステンシルは同時にコピー出来ません)。
};

struct CMD_COPY_TEXTURE_TO_BUFFER
{
    ITexture*                           src_texture;
    IBuffer*                            dst_buffer;
    uint32_t                            num_regions;
    const BUFFER_TEXTURE_COPY_REGION*   regions;    // 各要素のoffset.aspectには単一のビットのみが含まれている必要があります(例えば、深度とステンシルは同時にコピー出来ません)。
};

struct TEXTURE_RESOLVE_REGION
{
    SUBRESOURCE_OFFSET  src_subresource;
    const OFFSET2D*     src_offset;
    SUBRESOURCE_OFFSET  dst_subresource;
    const OFFSET2D*     dst_offset;
    const EXTENT2D*     resolve_extent;
    uint32_t            array_count;
};

struct CMD_RESOLVE_TEXTURE_REGION
{
    ITexture*                       src_texture; 
    ITexture*                       dst_texture;
    uint32_t                        num_regions;
    const TEXTURE_RESOLVE_REGION*   regions;
};

struct CLEAR_ATTACHMENT
{
    TEXTURE_ASPECT_FLAGS    aspect_mask;
    uint32_t                color_attachment;
    const CLEAR_VALUE*      clear_value;
};

struct CMD_CLEAR_ATTACHMENTS
{
    uint32_t                num_attachments;
    const CLEAR_ATTACHMENT* attachments;
    uint32_t                num_rects;
    const SCISSOR_RECT*     rects;
};

enum RESOURCE_BARRIER_FLAG : EnumT
{
      RESOURCE_BARRIER_FLAG_NONE               = 0x0

      /*
      * @brief 異なるキュータイプ間でリソースを共有する場合、ソースのキューから宛先のキューへのリソースの所有権の転送が必要です。 
      *        このフラグを指定する場合、キューファミリの所有権転送が定義され、異種キュー間でのリソースの値が保持されます。 
      *        この際、バリアは2回の操作に分けて実行する必要があり、初めにソースキューでの「解放」操作のバリアを実行し、宛先キューで「取得」操作のバリアを実行する必要があります。 
      *        src_state と dst_state は取得操作と解放操作で同一である必要があります。 
      */
    , RESOURCE_BARRIER_FLAG_OWNERSHIP_TRANSFER = 0x1 
};
using RESOURCE_BARRIER_FLAGS = EnumFlagsT;

struct BUFFER_BARRIER_DESC
{
    IBuffer*                buffer;
    RESOURCE_STATE          src_state;
    RESOURCE_STATE          dst_state;
    COMMAND_TYPE            src_queue_type;// src_queue_typeが現在のコマンドリストタイプと同一の場合、所有権の解放操作が定義されます。 barrier_flagsにRESOURCE_BARRIER_FLAG_OWNERSHIP_TRANSFERが含まれていない場合無視されます。
    COMMAND_TYPE            dst_queue_type;// dst_queue_typeが現在のコマンドリストタイプと同一の場合、所有権の取得操作が定義されます。 barrier_flagsにRESORUCE_BARRIER_FLAG_OWNERSHIP_TRANSFERが含まれていない場合無視されます。
    RESOURCE_BARRIER_FLAG   barrier_flags;
};

/**
 * @brief バリア範囲の指定元を決定します。
*/
enum TEXTURE_BARRIER_TYPE : EnumT
{
      TEXTURE_BARRIER_TYPE_BARRIER_RANGE // TEXTURE_BARRIER_RANGE内のサブリソースに対してバリアを定義します。barrier_rangeはnullptrであってはなりません。
    , TEXTURE_BARRIER_TYPE_VIEW          // IView*内のサブリソースに対してバリアを定義します。viewはnullptrであってはなりません。IViewの参照するリソースはテクスチャである必要があります。
};
struct TEXTURE_BARRIER_RANGE
{
    ITexture*                   texture;
    uint32_t                    num_subresource_ranges;
    const SUBRESOURCE_RANGE*    subresource_ranges;
};
struct TEXTURE_BARRIER_DESC
{
    TEXTURE_BARRIER_TYPE type;
    union 
    {
        const TEXTURE_BARRIER_RANGE*    barrier_range;
        IView*                          view;
    };

    RESOURCE_STATE          src_state;
    RESOURCE_STATE          dst_state;
    COMMAND_TYPE            src_queue_type;// barrier_flagsにRESOURCE_BARRIER_FLAG_OWNERSHIP_TRANSFERが含まれていない場合無視されます。
    COMMAND_TYPE            dst_queue_type;// barrier_flagsにRESORUCE_BARRIER_FLAG_OWNERSHIP_TRANSFERが含まれていない場合無視されます。
    RESOURCE_BARRIER_FLAG   barrier_flags;
};

struct CMD_PIPELINE_BARRIER
{
    PIPELINE_STAGE_FLAGS            src_stages;
    PIPELINE_STAGE_FLAGS            dst_stages;
    DEPENDENCY_FLAGS                dependency_flags;
    uint32_t                        num_buffer_barriers;
    const BUFFER_BARRIER_DESC*      buffer_barriers;
    uint32_t                        num_texture_barriers;
    const TEXTURE_BARRIER_DESC*     texture_barriers;
};

struct CMD_SET_DEPTH_BIAS
{
    float depth_bias_scale;
    float depth_bias_clamp;
    float depth_bias_slope_scale;
};

#pragma endregion command list arguments

#pragma region interfaces

/**
 * @brief アプリケーション側で実装するメモリアロケータインターフェースです。
*/
struct IAllocator
{
    virtual ~IAllocator() {}

    virtual void* 
        MAlloc(size_t _size, size_t _alignment) = 0;

    virtual void* 
        Realloc(void* _ptr, size_t _size, size_t _alignment) = 0;

    virtual void 
        Free(void* _ptr) = 0;

};

struct ALLOCATOR_DESC
{
    IAllocator* custom_allocator;           // アプリケーション定義のアロケーターを指定します。 nullptrの場合、デフォルトのアロケータが使用されます。
    bool        is_enabled_allocator_debug;  // メモリのデバッグを利用するかを指定します。
};


using PFN_Buma3DInitialize               = BMRESULT (B3D_APIENTRY*)(const ALLOCATOR_DESC& _desc);
using PFN_Buma3DGetInternalHeaderVersion = uint32_t (B3D_APIENTRY*)();
using PFN_Buma3DCreateDeviceFactory      = BMRESULT (B3D_APIENTRY*)(const DEVICE_FACTORY_DESC& _desc, IDeviceFactory** _dst);
using PFN_Buma3DUninitialize             = void     (B3D_APIENTRY*)();

/**
 * @brief ライブラリの初期化処理を実行します。
 * @param _desc メモリアロケーターの記述構造。
 * @return 成功した場合BMRESULT_SUCCEEDが返ります。
*/
B3D_DLL_API BMRESULT
B3D_APIENTRY Buma3DInitialize(const ALLOCATOR_DESC& _desc);

/**
 * @brief 実装側で利用されている B3D_HEADER_VERSION の値を取得します。
 * @return uint32_t 実装側のB3D_HEADER_VERSIONが返ります。
*/
B3D_DLL_API uint32_t
B3D_APIENTRY Buma3DGetInternalHeaderVersion();

/**
 * @brief デバイスファクトリを作成します。
 * @param _desc デバイスファクトリ作成情報。
 * @param _dst 作成された実体を受け取るインターフェースのポインタのポインタ。
 * @return 成功した場合BMRESULT_SUCCEED、デバッグレイヤの非サーポート等により失敗した場合BMRESULT_FAILEDが返ります。
*/
B3D_DLL_API BMRESULT
B3D_APIENTRY Buma3DCreateDeviceFactory(const DEVICE_FACTORY_DESC& _desc, IDeviceFactory** _dst);

/**
 * @brief ライブラリの終了処理を実行します。
 * @note ALLOCATOR_DESC::is_enabled_allocator_debugがtrueで、メモリリークが発生した場合リークしたメモリの情報を出力します。
*/
B3D_DLL_API void
B3D_APIENTRY Buma3DUninitialize();


// FIXME: 参照カウントは内部実装と共有です。
B3D_INTERFACE ISharedBase
{
protected:
    /**
     * @brief このクラスを継承する全てのクラスはRelease関数によってリソース破棄を行います。
    */
    B3D_APIENTRY ~ISharedBase() {}

public:
    /**
     * @brief 参照カウントを増やします。
    */
    virtual void
        B3D_APIENTRY AddRef() = 0;

    /**
     * @brief 参照カウントを減らします。
     * @return 減少後の参照カウントの値。
     * @note この関数呼び出しによって参照カウントが0になった際、このインターフェースを継承するインターフェースの、以降の全ての呼び出しは無効です。
    */
    virtual uint32_t
        B3D_APIENTRY Release() = 0;

    /**
     * @brief 現在の参照カウントを返します。
     * @return 現在の参照カウントの値。
    */
    virtual uint32_t
        B3D_APIENTRY GetRefCount() const = 0;

    //virtual void
    //    B3D_APIENTRY AddWeakRef() = 0;

    //virtual void
    //    B3D_APIENTRY ReleaseWeak() = 0;

    //virtual void
    //    B3D_APIENTRY GetWeakRefCount() const = 0;
    
    //virtual bool 
    //    B3D_APIENTRY IsExpired() const = 0;

    /**
     * @brief 動的キャストを行います。
     * @tparam DerivedT このインターフェースを継承するインターフェース。
     * @return キャスト後のポインタ。無効な型変換の場合nullptrです。
     * @remark 返されたポインタのAddRef()による参照カウントの増加、Release()による減少は呼び出し側の責任です。
    */
    template<typename DerivedT>
    [[nodiscard]] DerivedT*
        B3D_APIENTRY DynamicCastFromThis()
    { return dynamic_cast<DerivedT*>(this); }

    /**
     * @brief 静的キャストを行います。
     * @tparam DerivedT このインターフェースを継承するインターフェース
     * @return キャスト後のポインタ
     * @remark 返されたポインタのAddRef()による参照カウントの増加、Release()による減少は呼び出し側の責任です。
    */
    template<typename DerivedT>
    [[nodiscard]] DerivedT*
        B3D_APIENTRY StaticCastFromThis()
    { return static_cast<DerivedT*>(this); }

    /**
     * @brief 動的キャストを行います。
     * @tparam DerivedT このインターフェースを継承するインターフェース。
     * @return キャスト後のポインタ。無効な型変換の場合nullptrです。
     * @remark 返されたポインタのAddRef()による参照カウントの増加、Release()による減少は呼び出し側の責任です。
    */
    template<typename DerivedT>
    [[nodiscard]] const DerivedT*
        B3D_APIENTRY DynamicCastFromThis() const
    { return dynamic_cast<const DerivedT*>(this); }

    /**
     * @brief 静的キャストを行います。
     * @tparam DerivedT このインターフェースを継承するインターフェース
     * @return キャスト後のポインタ
     * @remark 返されたポインタのAddRef()による参照カウントの増加、Release()による減少は呼び出し側の責任です。
    */
    template<typename DerivedT>
    [[nodiscard]] const DerivedT*
        B3D_APIENTRY StaticCastFromThis() const
    { return static_cast<const DerivedT*>(this); }

    // TODO: IsStaticCastに変更

    template<typename DerivedT, bool IsDynamicCast, std::enable_if_t<IsDynamicCast, int> = 0>
    [[nodiscard]] DerivedT*
        B3D_APIENTRY As()
    { return DynamicCastFromThis<DerivedT>(); }

    template<typename DerivedT, bool IsDynamicCast = false, std::enable_if_t<!IsDynamicCast, int> = 0>
    [[nodiscard]] DerivedT*
        B3D_APIENTRY As()
    { return StaticCastFromThis<DerivedT>(); }

    template<typename DerivedT, bool IsDynamicCast, std::enable_if_t<IsDynamicCast, int> = 0>
    [[nodiscard]] const DerivedT*
        B3D_APIENTRY As() const
    { return DynamicCastFromThis<const DerivedT>(); }

    template<typename DerivedT, bool IsDynamicCast = false, std::enable_if_t<!IsDynamicCast, int> = 0>
    [[nodiscard]] const DerivedT*
        B3D_APIENTRY As() const
    { return StaticCastFromThis<const DerivedT>(); }

};

B3D_INTERFACE IBlob : public ISharedBase
{
protected:
    B3D_APIENTRY ~IBlob() {}

public:
    virtual size_t
        B3D_APIENTRY GetBufferSize() const = 0;

    virtual void*
        B3D_APIENTRY GetBufferPointer() const = 0;

};

B3D_INTERFACE INameableObject : public ISharedBase
{
protected: 
    B3D_APIENTRY ~INameableObject() {}

public:
    /**
     * @brief SetName関数で設定したデバッグ用オブジェクト名を取得します。
     * @return オブジェクトに設定されている UTF-8 null終端文字配列へのポインタ。オブジェクトに名前が設定されていない、またはデバッグ機能が有効でない場合、nullptrが返ります。
     * @remark この関数呼び出しによるデバッグ用オブジェクト名の設定は、デバイスファクトリ作成時のデバッグ機能が有効である必要があります。
    */
    virtual const char*
        B3D_APIENTRY GetName() const = 0;

    /**
     * @brief オブジェクトにデバッグに使用する名前を設定します。
     * @param _name オブジェクトに設定する UTF-8 null終端文字配列へのポインタ。nullptrを指定した場合、名前の設定を解放します。
     * @return 成功した場合、BMRESULT_SUCCEEDが返ります。デバッグ機能が有効でない場合、BMRESULT_FAILED以下のコードが返ります。
     * @remark この関数呼び出しによるデバッグ用オブジェクト名の設定は、デバイスファクトリ作成時のデバッグ機能が有効である必要があります。
    */
    virtual BMRESULT
        B3D_APIENTRY SetName(const char* _name) = 0;

};

B3D_INTERFACE IDeviceFactory : public INameableObject
{
protected:
    B3D_APIENTRY ~IDeviceFactory() {}

public:
    virtual const DEVICE_FACTORY_DESC&
        B3D_APIENTRY GetDesc() const = 0;

    /**
     * @brief デバッグメッセージオブジェクトが格納されるキューを取得します。
     * @param _dst メッセージオブジェクトキューの取得先。
     * @return DEVICE_FACTORY_DESC::debug::is_enabledがtrueで、処理が正常に終了した場合BMRESULT_SUCCEED、それ以外の場合BMRESULT_FAILEDが返ります。
    */
    virtual BMRESULT
        B3D_APIENTRY GetDebugMessageQueue(
            IDebugMessageQueue** _dst) = 0;

    /**
     * @brief 指定されたインデックスのデバイスアダプタを取得します。
     * @param _adapter_index 取得するアダプタのインデックス。
     * @param _dst_adapter アダプタを返すIDeviceAdapterへのポインタのポインタ。
     * @return 成功した場合、BMRESULT_SUCCEEDが返ります。
     *         _adapter_indexに領域外インデックスが指定された場合、BMRESULT_FAILED_OUT_OF_RANGEを返します(このコードを介してアダプタの要素数を識別することができます)。
    */
    virtual BMRESULT
        B3D_APIENTRY EnumAdapters(
              uint32_t         _adapter_index
            , IDeviceAdapter** _dst_adapter) = 0;

    /**
     * @brief 指定されたパラメーターでデバイスを作成します。
     * @param _desc デバイスの作成に使用する記述。
     * @param _dst 作成されたデバイスを返すIDeviceへのポインタのポインタ。
     * @return 成功した場合、BMRESULT_SUCCEEDが返ります。
    */
    virtual BMRESULT
        B3D_APIENTRY CreateDevice(
              const DEVICE_DESC& _desc
            , IDevice**          _dst) = 0;

};

B3D_INTERFACE IDebugMessageQueue : public ISharedBase
{
protected:
    B3D_APIENTRY ~IDebugMessageQueue() {}

public:
    /**
     * @brief メッセージオブジェクトを取得します。取得されたメッセージは内部キューから削除され、存在する場合次のメッセージを取得します。
     * @param _dst_message メッセージオブジェクトの取得先。キューにメッセージが存在しない場合は取得されません。
     * @return キューにメッセージが存在し、取得された場合BMRESULT_SUCCEED、キューにメッセージが存在しない場合BMRESULT_FAILEDが返ります。
    */
    virtual BMRESULT
        B3D_APIENTRY PopMessage(
            IDebugMessage** _dst_message) = 0;

    /**
     * @brief キューが格納するメッセージの最大数を取得します。
     * @return キューが格納するメッセージの最大数。
     * @remark デフォルトでは、この値は1024に設定されます。
    */
    virtual size_t
        B3D_APIENTRY GetMessageCountLimit() = 0;

    /**
     * @brief キューが格納するメッセージの最大数を設定します。
     * @param _message_count_limit 格納するメッセージの最大数。
     * @remark メッセージ数がこの関数で設定した最大数を超えた場合。最も古いメッセージがキューから削除されます。現在の最大数より小さい値が設定された場合、以前のメッセージはキューから削除されます。
    */
    virtual void
        B3D_APIENTRY SetMessageCountLimit(
            size_t _message_count_limit) = 0;

    /**
     * @brief 現在キューに存在するメッセージオブジェクトの数を取得します。
     * @return 現在キューに存在するメッセージオブジェクトの数を取得します。 
    */
    virtual size_t
        B3D_APIENTRY GetNumStoredMessages() = 0;

    /**
     * @brief 現在格納されているメッセージオブジェクトをクリアします。
    */
    virtual void
        B3D_APIENTRY ClearStoredMessages() = 0;

};

B3D_INTERFACE IDebugMessage : public ISharedBase
{
protected:
    B3D_APIENTRY ~IDebugMessage() {}

public:
    /**
     * @brief メッセージを取得します。
     * @return UTF-8 null終端文字配列です。
    */
    virtual const Char8T*
        B3D_APIENTRY GetString() const = 0;

};

B3D_INTERFACE IDeviceAdapter : public INameableObject
{
protected:
    B3D_APIENTRY ~IDeviceAdapter() {}

public:
    virtual const DEVICE_ADAPTER_DESC&
        B3D_APIENTRY GetDesc() const = 0;

    /**
     * @brief 作成可能なコマンドキューのプロパティを取得します。
     * @param [out] _properties 値が設定されるCOMMAND_QUEUE_PROPERTIES構造の配列。nullptrの場合値は設定されません。
     * @return _propertiesに必要なCOMMAND_QUEUE_PROPERTIES構造の配列要素数を返します。
    */
    virtual uint32_t
        B3D_APIENTRY GetCommandQueueProperties(
            COMMAND_QUEUE_PROPERTIES* _properties) = 0;

    /**
     * @brief リソースやパイプライン作成時に利用可能なパラメータの最大または最小値、または考慮する必要がある制限値を取得します。
     * @param[out] _dst_limits DEVICE_ADAPTER_LIMITS構造へのポインターです。 値はnullptrであってはなりません。
    */
    virtual void
        B3D_APIENTRY GetDeviceAdapterLimits(
            DEVICE_ADAPTER_LIMITS* _dst_limits) = 0;

    /**
     * @brief 画像を出力する機構を抽象化するサーフェスオブジェクトを作成します。
     * @param _desc サーフェス作成情報。
     * @param _dst 作成された実体を受け取るインターフェースのポインタのポインタ。
     * @return 作成に成功した場合BMRESULT_SUCCEEDが返ります。
    */
    virtual BMRESULT
        B3D_APIENTRY CreateSurface(
              const SURFACE_DESC& _desc
            , ISurface**          _dst) = 0;

    /**
     * @brief 指定のコマンドキュータイプが、指定のサーフェスでPresent操作をサポートするかどうかを取得します。
     * @param _queue_type Present操作をサポートするかどうかを取得するキュータイプを指定します。
     * @param _surface _queue_typeのキューがPresent操作をサポートするかどうかを取得するサーフェスを指定します。
     * @return プレゼント操作をサポートする場合BMRESULT_SUCCEEDを返します。
    */
    virtual BMRESULT
        B3D_APIENTRY QueryPresentationSupport(
              COMMAND_TYPE      _queue_type
            , const ISurface*   _surface) = 0;

};

B3D_INTERFACE ISurface : public INameableObject
{
protected:
     B3D_APIENTRY ~ISurface() {}

public:
    virtual const SURFACE_DESC&
        B3D_APIENTRY GetDesc() const = 0;

    virtual SURFACE_STATE
        B3D_APIENTRY GetState() = 0;

    /**
     * @brief スワップチェインの作成時に利用可能なサポートされている色空間とフォーマットのペアを取得します。
     * @param[out] _dst サポートされる色空間とフォーマットのペアの情報を書き込むSURFACE_FORMAT構造の配列です。nullptrの場合値は書き込まれません。
     * @return _dst配列の要素数を返します。
    */
    virtual uint32_t
        B3D_APIENTRY GetSupportedSurfaceFormats(
            SURFACE_FORMAT* _dst) = 0;

};

B3D_INTERFACE IDevice : public INameableObject
{
protected:
     B3D_APIENTRY ~IDevice() {}

public:
    virtual const DEVICE_DESC&
        B3D_APIENTRY GetDesc() const = 0;

    /**
     * @brief 全ての有効なノードインデックスのビットがセットされたノードマスクを取得します。
     * @return ノードマスクです。
    */
    virtual NodeMask
        B3D_APIENTRY GetValidNodeMask() const = 0;

    /**
     * @brief 存在するヒープのプロパティを取得します。
     * @param [out] _dst_properties 値が書き込まれるRESOURCE_HEAP_PROPERTIES構造の配列。nullptrの場合値は書き込まれません。
     * @return _dst_propertiesに必要なRESOURCE_HEAP_PROPERTIES構造の配列要素数を返します。
    */
    virtual uint32_t
        B3D_APIENTRY GetResourceHeapProperties(
            RESOURCE_HEAP_PROPERTIES* _dst_properties) const = 0;

    /**
     * @brief ヒープの作成、リソースの作成に必要なメモリの割り当て情報を取得します。
     * @param [in] _num_resources _resources配列の要素数です。
     * @param [in] _resources 1つのヒープに割り当てるリソースオブジェクトの配列です。
     * @param [out] _dst_infos 1つのヒープに割り当てるための情報を取得する配列です。
     * @param [out] _dst_heap_info _resourcesの全てのリソースを作成するために必要なヒープの情報を取得します。
     * @return 正常に終了した場合、BMRESULT_SUCCEEDが返ります。_resources配列の要素の全てに対応可能なヒープタイプ見つからなかった場合、BMRESULT_FAILEDが返ります。
     * @remark _resources配列内のリソースオブジェクトの順序によってヒープの合計サイズが変動します。
     *         これは各リソースのアライメント要求が異なる場合に発生する可能性があります。(C++における、構造体の変数アライメントによるサイズの変動と同様です。)
     */
    virtual BMRESULT
        B3D_APIENTRY GetResourceAllocationInfo(
              uint32_t                       _num_resources
            , const IResource*const *        _resources
            , RESOURCE_ALLOCATION_INFO*      _dst_infos
            , RESOURCE_HEAP_ALLOCATION_INFO* _dst_heap_info) const = 0;

    // TODO: InfoやDESC、PROPERTIES などの名前がつく関数、構造の用途に一貫性をもたせる。(リネームも検討する)

    /**
     * @brief CreateReservedResourceから作成されたリソースをバインドする際に必要な情報を取得します。
     * @param [in] _reserved_resource 情報を取得するCreateReservedResourceから作成されたリソース。
     * @param [out] _dst_infos バインドする際に必要な情報を取得します。nullptrの場合値は書き込まれません。
     * @return _dst_infosに必要なTILED_RESOURCE_ALLOCATION_INFO構造の配列要素数を返します。タイルリソースに対応しない、またはエラーの場合0が返ります。
     * @remark 内部APIの実装上の仕様により、_dst_infosで取得する情報が複数必要な場合があります。
     *         この場合、この関数の戻り値は1以上になり、_dst_infosの各要素のパラメータが示す情報(アスペクト、ミップテイル)を使用してバインドする必要があります。
    */
    virtual uint32_t
        B3D_APIENTRY GetTiledResourceAllocationInfo(
              const IResource*                _reserved_resource
            , TILED_RESOURCE_ALLOCATION_INFO* _dst_infos) const = 0;

    /**
     * @brief 指定された複数のルートシグネチャに対応するディスクリプタプール、セットを作成する際に必要なDESCRIPTOR_POOL_SIZE配列を取得します。
     * @param [in] _num_root_signatures _root_signatures、_num_descriptor_sets 配列の要素数です。
     * @param [in] _root_signatures ルートシグネチャの配列です。 配列の要素順に従って、必要なプールサイズが_dst_sizesの各要素に加算されます。
     * @param [in] _num_descriptor_sets _root_signaturesの各要素に対応する、uint32_t配列です。 割り当てるディスクリプタセットの数を指定します。
     * @param [out] _dst_max_num_register_space 各ルートシグネチャで最大のregister_spaceの数を取得します。(register_space番号自体の最大値ではありません。) nullptrの場合値は書き込まれません。
     * @param [out] _dst_sizes 必要なプールサイズを取得します。nullptrの場合値は書き込まれません。
     * @return _dst_sizesに必要なDESCRIPTOR_POOL_SIZE構造の配列要素数を返します。
     * @note _dst_sizes配列に書き込まれるDESCRIPTOR_TYPEの順序は不定です。必要な割り当てが存在しないDESCRIPTOR_TYPEはスキップされます。
     * @remark ディスクリプタセットの解放により断片化が生じた場合、引数に指定された要件のディスクリプタセットの数を完全には割り当てられなくなる可能性があります。
    */
    virtual uint32_t
        B3D_APIENTRY GetDescriptorPoolSizesAllocationInfo(
              uint32_t                      _num_root_signatures
            , const IRootSignature*const *  _root_signatures
            , const uint32_t*               _num_descriptor_sets
            , uint32_t*                     _dst_max_num_register_space
            , DESCRIPTOR_POOL_SIZE*         _dst_sizes) const = 0;

    /**
     * @brief 指定されたコマンドキューを取得します。
     * @param _queue_type 取得するキューのタイプ。
     * @param _queue_index 取得するキューのタイプ内のコマンドキューのインデックス。
     * @param _dst 要求されたコマンドキューを受け取ります。 参照カウントが増加することに注意してください。
     * @return 成功した場合、BMRESULT_SUCCEEDが返ります。指定された_queue_typeのコマンドキューが存在しない場合、BMRESULT_FAILEDが返ります。
     *         _queue_indexに領域外インデックスが指定された場合、BMRESULT_FAILED_OUT_OF_RANGEを返します。
    */
    virtual BMRESULT
        B3D_APIENTRY GetCommandQueue(
              COMMAND_TYPE    _queue_type
            , uint32_t        _queue_index
            , ICommandQueue** _dst) const = 0;

    /**
     * @brief このデバイスで作成されたすべてのコマンドキューで現在実行中のすべての処理が完了するまでCPUで待機します。
     * @return デバイスが削除された場合、BMRESULT_FAILED_DEVICE_REMOVEDが返ります。
    */
    virtual BMRESULT
        B3D_APIENTRY WaitIdle() = 0;

    /**
     * @brief コマンドアロケーターを作成します。
     * @param _desc コマンドアロケーターの作成に使用される情報を指定します。
     * @param _dst  作成されたコマンドアロケーターを受け取ります。
     * @return 指定されたコマンドのタイプがサポートされていない場合、BMRESULT_FAILED以下を返します。
    */
    virtual BMRESULT
        B3D_APIENTRY CreateCommandAllocator(
              const COMMAND_ALLOCATOR_DESC& _desc
            , ICommandAllocator**           _dst) = 0;

    /**
     * @brief 指定のアロケーターに関連付けられたコマンドリストを作成します。
     * @param _desc コマンドリストの作成に使用される情報を指定します。
     * @param _dst 作成されたコマンドリストを受け取ります。
     * @return 指定されたコマンドのタイプがサポートされていない、またはアロケータのタイプと一致しない場合、BMRESULT_FAILED以下を返します。
    */
    virtual BMRESULT
        B3D_APIENTRY AllocateCommandList(
              const COMMAND_LIST_DESC& _desc
            , ICommandList**           _dst) = 0;


    virtual BMRESULT
        B3D_APIENTRY CreateFence(
              const FENCE_DESC& _desc
            , IFence**          _dst) = 0;


    virtual BMRESULT
        B3D_APIENTRY CreateRootSignature(
              const ROOT_SIGNATURE_DESC& _desc
            , IRootSignature**           _dst) = 0;

    virtual BMRESULT
        B3D_APIENTRY CreateDescriptorPool(
              const DESCRIPTOR_POOL_DESC& _desc
            , IDescriptorPool**           _dst) = 0;

    virtual BMRESULT
        B3D_APIENTRY UpdateDescriptorSets(
            const UPDATE_DESCRIPTOR_SET_DESC& _update_desc) = 0;

    /**
     * @brief シェーダーモジュールを作成します。
     * @param _desc シェーダーモジュールの作成に使用されるシェーダーコードを指定します。 TODO: HLSL、または DXIL/SIPR-V 
     * @param _dst 作成されたシェーダーモジュールを受け取ります。
     * @return BMRESULT
    */
    virtual BMRESULT
        B3D_APIENTRY CreateShaderModule(
              const SHADER_MODULE_DESC& _desc
            , IShaderModule**           _dst) = 0;    

    virtual BMRESULT
        B3D_APIENTRY CreateGraphicsPipelineState(
              const GRAPHICS_PIPELINE_STATE_DESC& _desc
            , IPipelineState**                    _dst) = 0;

    virtual BMRESULT
        B3D_APIENTRY CreateComputePipelineState(
              const COMPUTE_PIPELINE_STATE_DESC& _desc
            , IPipelineState**                   _dst) = 0;

    virtual BMRESULT
        B3D_APIENTRY CreateRayTracingPipelineState(
              const RAY_TRACING_PIPELINE_STATE_DESC& _desc
            , IPipelineState**                       _dst) = 0;


    virtual BMRESULT
        B3D_APIENTRY CreateSwapChain(
              const SWAP_CHAIN_DESC& _desc
            , ISwapChain**           _dst) = 0;

    virtual BMRESULT
        B3D_APIENTRY CreateResourceHeap(
              const RESOURCE_HEAP_DESC& _desc
            , IResourceHeap**           _dst) = 0;

    /**
     * @brief  CreatePlacedResourceで作成されたリソースを指定のヒープとバインドします。 
     * @param _num_bind_infos _bind_infos配列の要素数です。
     * @param _bind_infos バインドするリソースとヒープの情報を指定するBIND_RESOURCE_HEAP_INFO構造の配列です。
     * @return いずれかの要素のバインドに失敗した場合、BMRESULT_FAILED以下の値が返ります。
    */
    virtual BMRESULT
        B3D_APIENTRY BindResourceHeaps(
              uint32_t                       _num_bind_infos
            , const BIND_RESOURCE_HEAP_INFO* _bind_infos) = 0;

    virtual BMRESULT
        B3D_APIENTRY CreatePlacedResource(
              const RESOURCE_DESC& _desc
            , IResource**          _dst) = 0;

    virtual BMRESULT
        B3D_APIENTRY CreateReservedResource(
              const RESOURCE_DESC& _desc
            , IResource**          _dst) = 0;

    virtual BMRESULT
        B3D_APIENTRY CreateCommittedResource(
              const COMMITTED_RESOURCE_DESC& _desc
            , IResource**                    _dst) = 0;


    virtual BMRESULT
        B3D_APIENTRY CreateVertexBufferView(
              IBuffer*                       _buffer
            , const VERTEX_BUFFER_VIEW_DESC& _desc
            , IVertexBufferView**            _dst) = 0;

    virtual BMRESULT
        B3D_APIENTRY CreateIndexBufferView(
              IBuffer*                      _buffer
            , const INDEX_BUFFER_VIEW_DESC& _desc
            , IIndexBufferView**            _dst) = 0;

    virtual BMRESULT
        B3D_APIENTRY CreateConstantBufferView(
              IBuffer*                         _buffer
            , const CONSTANT_BUFFER_VIEW_DESC& _desc
            , IConstantBufferView**            _dst) = 0;

    virtual BMRESULT
        B3D_APIENTRY CreateShaderResourceView(
              IResource*                       _resource
            , const SHADER_RESOURCE_VIEW_DESC& _desc
            , IShaderResourceView**            _dst) = 0;

    virtual BMRESULT
        B3D_APIENTRY CreateUnorderedAccessView(
              IResource*                        _resource
            , IBuffer*                          _resource_for_counter_buffer
            , const UNORDERED_ACCESS_VIEW_DESC& _desc
            , IUnorderedAccessView**            _dst) = 0;

    virtual BMRESULT
        B3D_APIENTRY CreateRenderTargetView(
              IResource*                     _resource
            , const RENDER_TARGET_VIEW_DESC& _desc
            , IRenderTargetView**            _dst) = 0;

    virtual BMRESULT
        B3D_APIENTRY CreateDepthStencilView(
              IResource*                     _resource
            , const DEPTH_STENCIL_VIEW_DESC& _desc
            , IDepthStencilView**            _dst) = 0;

    virtual BMRESULT
        B3D_APIENTRY CreateSampler(
              const SAMPLER_DESC& _desc
            , ISamplerView**      _dst) = 0;

    virtual BMRESULT
        B3D_APIENTRY CreateStreamOutputBufferView(
              IBuffer*                              _buffer
            , IBuffer*                              _filled_size_counter_buffer
            , const STREAM_OUTPUT_BUFFER_VIEW_DESC& _desc
            , IStreamOutputBufferView**             _dst) = 0;


    virtual BMRESULT
        B3D_APIENTRY CreateQueryHeap(
            const QUERY_HEAP_DESC& _desc
            , IQueryHeap**         _dst) = 0;

    //virtual BMRESULT 
    //    B3D_APIENTRY CreateCommandSignature(
    //        const COMMAND_SIGNATURE_DESC&    _desc
    //        , CommandSignaturePtr*            _dst) = 0;

    virtual BMRESULT
        B3D_APIENTRY CreateRenderPass(
              const RENDER_PASS_DESC&   _desc
            , IRenderPass**             _dst) = 0;

    virtual BMRESULT
        B3D_APIENTRY CreateFramebuffer(
              const FRAMEBUFFER_DESC&   _desc
            , IFramebuffer**            _dst) = 0;


    //virtual BMRESULT
    //    B3D_APIENTRY CreateSharedHandle(
    //    _In_  ID3D12DeviceChild* pObject,
    //    _In_opt_  const SECURITY_ATTRIBUTES* pAttributes,
    //    DWORD Access,
    //    _In_opt_  LPCWSTR Name,
    //    _Out_  HANDLE* pHandle) = 0;
    //
    //virtual BMRESULT
    //    B3D_APIENTRY OpenSharedHandle(
    //    void*                    _handle
    //    , NameableObjectPtr*    _dst) = 0;
    //
    //virtual BMRESULT
    //    B3D_APIENTRY OpenSharedHandleByName(
    //    _In_  LPCWSTR Name,
    //    DWORD Access,
    //    /* [annotation][out] */
    //    _Out_  HANDLE* pNTHandle) = 0;

};

B3D_INTERFACE IDeviceChild : public INameableObject
{
protected:
    B3D_APIENTRY ~IDeviceChild() {}

public:
    /**
     * @brief デバイスを取得します。
     * @remark 返されたポインタのAddRef()による参照カウントの増加、Release()による減少は呼び出し側の責任です。
    */
    virtual IDevice*
        B3D_APIENTRY GetDevice() const = 0;

};

B3D_INTERFACE ISwapChain : public IDeviceChild
{
protected:
    B3D_APIENTRY ~ISwapChain() {}

public:
    virtual const SWAP_CHAIN_DESC&
        B3D_APIENTRY GetDesc() const = 0;

    /**
     * @brief バックバッファを取得します。
     * @param _buffer_idnex 取得するバックバッファのインデックスを指定します。
     * @param _dst 指定のバックバッファを取得します。 参照カウントが増加することに注意してください。
     * @return 成功した場合、BMRESULT_SUCCEEDが返ります。
    */
    virtual BMRESULT
        B3D_APIENTRY GetBuffer(
              uint32_t   _buffer_idnex
            , ITexture** _dst) = 0;

    /**
     * @brief 取得可能なバックバッファのインデックスを取得し、指定されている場合そのインデックスのバックバッファのプレゼント操作が完了した際にシグナルを送信します。
     * @param      _info             バッファインデックスの取得時に使用するパラメータを指定します。 
     * @param[out] _dst_buffer_index 次のプレゼント操作に使用するバックバッファのインデックスを取得します。
     * @return 使用可能バッファが存在せず、タイムアウトした場合BMRESULT_SUCCEED_TIMEOUT、timeoutがゼロの場合BMRESULT_SUCCEED_NOT_READYが返ります。 
     * @remark 取得したインデックスのバッファはアプリケーションによって所有された状態になり、任意のコマンドでバッファの内容を編集することができます。 
     *         この所有権はISwapChain::Presentを呼び出すことによって解放する必要があります。
     *         加えて、この関数とISwapChain::Presentは1つの操作セットです。 Presentを呼び出すまで、再びこの関数を呼び出してはなりません。
     * @note バッファの取得(acquire)と、そのバッファが利用可能(available)になることは別々に扱われます。
     *       たとえば、AcquireNextBufferを呼び出し、_dst_buffer_indexにacquireしたバッファのインデックスが設定されたとしても、そのバッファは現在プレゼント操作が実行中である可能性があります。
     *       プレゼントの実行中(バッファ利用不可時)にカラーアタッチメントとして書き込みを行うと、ディスプレイへの表示結果が破壊される可能性もあります。
     *       利用可能(available)になるタイミングをアプリケーションが知るためには、SWAP_CHAIN_ACQUIRE_NEXT_BUFFER_INFOにsignal_fenceとsignal_fence_to_cpuを渡し、これらのシグナルを待機します。
    */
    virtual BMRESULT
        B3D_APIENTRY AcquireNextBuffer(
              const SWAP_CHAIN_ACQUIRE_NEXT_BUFFER_INFO& _info
            , uint32_t*                                  _dst_buffer_index) = 0;

    /**
     * @brief AcquireNextBufferで取得したインデックスのバックバッファをディスプレイに提示する操作を、キューに追加します。
     * @param _info プレゼント操作に使用するパラメータを指定します。
     * @return 成功した場合、BMRESULT_SUCCEEDが返ります。
     * @remark この関数を呼び出す前にISwapChain::AcquireNextBufferが呼び出されている必要があります。
    */
    virtual BMRESULT
        B3D_APIENTRY Present(
            const SWAP_CHAIN_PRESENT_INFO& _info) = 0;

    /**
     * @brief スワップチェインを再作成します。
     * @param _desc 再作成に使用されるパラメータを指定します。
     * @return 成功した場合、BMRESULT_SUCCEEDが返ります。
     * @remark この関数が呼び出される前に、全てのプレゼント操作が完了し、ISwapChain::GetBufferから取得したバックバッファを解放する必要があります。
    */
    virtual BMRESULT
        B3D_APIENTRY Recreate(
            const SWAP_CHAIN_DESC& _desc) = 0;

    /**
     * @brief ディスプレイのHDRメタ情報を設定します。
     * @param _metadata メタ情報を指定するSWAP_CHAIN_HDR_METADATA構造です。
     * @return 成功した場合、BMRESULT_SUCCEEDが返ります。
    */
    virtual BMRESULT
        B3D_APIENTRY SetHDRMetaData(
            const SWAP_CHAIN_HDR_METADATA& _metadata) = 0;

};

B3D_INTERFACE IResourceHeap : public IDeviceChild
{
protected:
    B3D_APIENTRY ~IResourceHeap() {}

public:
    virtual const RESOURCE_HEAP_DESC&
        B3D_APIENTRY GetDesc() const = 0;

    /**
     * @brief このヒープが所有するメモリへのマッピングを開始します。
     * @param[in] _range_to_map マップする領域を指定します。デフォルトの場合、全ての領域をマップします。
     *                          一部の内部APIによってはアライメントの制約があり、指定された領域がオフセットされ実際にマップされる領域は拡張される可能性があります。
     * @return HOST_READABLE、HOST_WRITABLEでない場合、または既にマップされている場合この関数呼び出しは失敗します。
    */
    virtual BMRESULT
        B3D_APIENTRY Map(
            const MAPPED_RANGE* _range_to_map = nullptr) = 0;

    /**
     * @brief Map()によってマップされた実際の領域と、その実際の領域のデータへのポインタを取得します。
     * @param[out] _mapped_range マップされている領域を取得します。
     * @param[out] _dst_mapped_data マップれされているデータへのポインタを取得します。
     * @return マップされていない場合この関数呼び出しは失敗します。
    */
    virtual BMRESULT
        B3D_APIENTRY GetMappedData(
              MAPPED_RANGE* _mapped_range
            , void**        _dst_mapped_data) const = 0;

    /**
     * @brief 指定のマップされた範囲のキャッシュをGPUメモリに書き込みます。全ての引数がデフォルトの場合、マップした全ての領域をフラッシュします。
     * @param     _num_ranges フラッシュする範囲を指定する_rangesの要素数です。
     * @param[in] _ranges     フラッシュする範囲を指定するMAPPED_RANGEの配列です。(GetMappedDataPointerによって取得される範囲内。)
     * @return HOST_WRITABLEでない、またはマップされていない場合この関数呼び出しは失敗します。
    */
    virtual BMRESULT
        B3D_APIENTRY FlushMappedRanges(
              uint32_t            _num_ranges = 1
            , const MAPPED_RANGE* _ranges     = nullptr) = 0;

    /**
     * @brief 指定のマップされた範囲のキャッシュを一旦無効化し、新たにGPUのデータで更新します。全ての引数がデフォルトの場合、マップした全ての領域を無効化します。
     * @param     _num_ranges 無効化する範囲を指定する_rangesの要素数です。
     * @param[in] _ranges     無効化する範囲を指定するMAPPED_RANGEの配列です。(GetMappedDataPointerによって取得される範囲内。)
     * @return HOST_READABLEでない、またはマップされていない場合この関数呼び出しは失敗します。
    */
    virtual BMRESULT
        B3D_APIENTRY InvalidateMappedRanges(
              uint32_t            _num_ranges = 1
            , const MAPPED_RANGE* _ranges     = nullptr) = 0;

    /**
     * @brief このヒープが所有するメモリへのマッピングを解除します。
     * @param[in] _used_range マップを開始してから、実際に使用した範囲を指定します。(GetMappedDataPointerによって取得される範囲内。)
     *                         デフォルトの場合、マップした全ての領域を指定します。
     * @return マップされていない場合この関数呼び出しは失敗します。
    */
    virtual BMRESULT
        B3D_APIENTRY Unmap(
            const MAPPED_RANGE* _used_range = nullptr) = 0;

};

B3D_INTERFACE IResource : public IDeviceChild
{
protected:
    B3D_APIENTRY ~IResource() {}

public:
    virtual const RESOURCE_DESC&
        B3D_APIENTRY GetDesc() const = 0;

    /**
     * @brief リソースとバインドしたヒープを取得します。
     * @return リソースが未バインドの場合、CreateReservedResourceから作成したリソースの場合、またはスワップチェインバッファの場合、nullptrが返ります。
    */
    virtual IResourceHeap*
        B3D_APIENTRY GetHeap() const = 0;

};

B3D_INTERFACE IBuffer : public IResource
{
protected:
    B3D_APIENTRY ~IBuffer() {}

public:
    virtual GpuVirtualAddress
        B3D_APIENTRY GetGPUVirtualAddress() const = 0;

};

B3D_INTERFACE ITexture : public IResource
{
protected:
    B3D_APIENTRY ~ITexture() {}

public:

};

B3D_INTERFACE IView : public IDeviceChild
{
protected:
    B3D_APIENTRY ~IView() {}

public:
    virtual const VIEW_DESC& 
        B3D_APIENTRY GetViewDesc() const = 0;

    virtual IResource* 
        B3D_APIENTRY GetResource() const = 0;

    virtual const BUFFER_VIEW*
        B3D_APIENTRY GetBufferView() const = 0;

    virtual const TEXTURE_VIEW*
        B3D_APIENTRY GetTextureView() const = 0;

};

B3D_INTERFACE IVertexBufferView : public IView
{
protected:
    B3D_APIENTRY ~IVertexBufferView() {}

public:
    virtual const VERTEX_BUFFER_VIEW_DESC&
        B3D_APIENTRY GetDesc() const = 0;

};

B3D_INTERFACE IIndexBufferView : public IView
{
protected:
    B3D_APIENTRY ~IIndexBufferView() {}

public:
    virtual const INDEX_BUFFER_VIEW_DESC&
        B3D_APIENTRY GetDesc() const = 0;

};

B3D_INTERFACE IConstantBufferView : public IView
{
protected:
    B3D_APIENTRY ~IConstantBufferView() {}

public:
    virtual const CONSTANT_BUFFER_VIEW_DESC&
        B3D_APIENTRY GetDesc() const = 0;

};

B3D_INTERFACE IRenderTargetView : public IView
{
protected:
    B3D_APIENTRY ~IRenderTargetView() {}

public:
    virtual const RENDER_TARGET_VIEW_DESC&
        B3D_APIENTRY GetDesc() const = 0;

};

B3D_INTERFACE IDepthStencilView : public IView
{
protected:
    B3D_APIENTRY ~IDepthStencilView() {}

public:
    virtual const DEPTH_STENCIL_VIEW_DESC&
        B3D_APIENTRY GetDesc() const = 0;

};

B3D_INTERFACE IShaderResourceView : public IView
{
protected:
    B3D_APIENTRY ~IShaderResourceView() {}

public:
    virtual const SHADER_RESOURCE_VIEW_DESC&
        B3D_APIENTRY GetDesc() const = 0;

};

B3D_INTERFACE IUnorderedAccessView : public IView
{
protected:
    B3D_APIENTRY ~IUnorderedAccessView() {}

public:
    virtual const UNORDERED_ACCESS_VIEW_DESC&
        B3D_APIENTRY GetDesc() const = 0;

    virtual IBuffer*
        B3D_APIENTRY GetCounterBuffer() const = 0;

};

B3D_INTERFACE ISamplerView : public IView
{
protected:
    B3D_APIENTRY ~ISamplerView() {}

public:
    virtual const SAMPLER_DESC&
        B3D_APIENTRY GetDesc() const = 0;

};

B3D_INTERFACE IStreamOutputBufferView : public IView
{
protected:
    B3D_APIENTRY ~IStreamOutputBufferView() {}

public:
    virtual const STREAM_OUTPUT_BUFFER_VIEW_DESC&
        B3D_APIENTRY GetDesc() const = 0;

    virtual IBuffer*
        B3D_APIENTRY GetFilledSizeCounterBuffer() const = 0;

};

B3D_INTERFACE IFramebuffer : public IDeviceChild
{
protected:
    B3D_APIENTRY ~IFramebuffer() {}

public:
    virtual const FRAMEBUFFER_DESC&
        B3D_APIENTRY GetDesc() const = 0;

};

B3D_INTERFACE IRenderPass : public IDeviceChild
{
protected:
    B3D_APIENTRY ~IRenderPass() {}

public:
    virtual const RENDER_PASS_DESC&
        B3D_APIENTRY GetDesc() const = 0;

};

B3D_INTERFACE IDescriptorPool : public IDeviceChild
{
protected:
    B3D_APIENTRY ~IDescriptorPool() {}

public:
    virtual const DESCRIPTOR_POOL_DESC&
        B3D_APIENTRY GetDesc() const = 0;

    /**
     * @brief 現在このプールから割り当てられているディスクリプタセットの数を取得します。
     * @return ディスクリプタセットの割り当てカウントです。
    */
    virtual uint32_t
        B3D_APIENTRY GetCurrentAllocationCount() = 0;

    /**
     * @brief 割り当ての管理をリセットし、このプールから以前に割り当てた全てのディスクリプタセットの使用を無効化します。
     * @remark 無効となったディスクリプタセットは、アプリケーションで増加させた参照カウントを解放する必要があります。
    */
    virtual void
        B3D_APIENTRY ResetPoolAndInvalidateAllocatedSets() = 0;

    /**
     * @brief 指定のシグネチャに必要なディスクリプタを割り当てたディスクリプタセットを作成します。成功した場合、割り当てカウントが増加します。
     * @param [in] _root_signature _dstのシグネチャを指定します。
     * @param [out] _dst 作成されたIDescriptorSetを取得します。 DESCRIPTOR_POOL_FLAG_FREE_DESCRIPTOR_SETを使用して作成されたプールの場合、IDescriptorSetが解放される時、割り当てられていたディスクリプタはプールに返還されます。
     *                   そうでない場合返還はされず、ResetPoolAndInvalidateAllocatedSets()を介してのみ割り当てカウントを減少させることができます。
     * @return 断片化、またはディスクリプタセットの割り当て回数の上限を超える場合、BMRESULT_FAILED以下を返します。
     * @remark プールの作成時または最後にリセットされてからプールから割り当てられたすべてのセットが、(各タイプの)同じ数のディスクリプタを使用し、要求された割り当ても同じ数の(各タイプの)ディスクリプタを使用する場合、断片化によって割り当てが失敗することはありません。 
     *         単一のルートシグネチャ専用でプールを作成する場合に有効です。
    */
    virtual BMRESULT
        B3D_APIENTRY AllocateDescriptorSet(
              IRootSignature*   _root_signature
            , IDescriptorSet**  _dst) = 0;

};

B3D_INTERFACE IDescriptorSet : public IDeviceChild
{
protected:
    B3D_APIENTRY ~IDescriptorSet() {}

public:
    /**
     * @brief このセットに対応するのシグネチャを取得します。 戻り値を保持して利用する場合、参照カウントの増加はアプリケーションの責任です。
     * @return IRootSignature* 
    */
    virtual IRootSignature*
        B3D_APIENTRY GetRootSignature() const = 0;

    /**
     * @brief 割り当て元のプールを取得します。 戻り値を保持して利用する場合、参照カウントの増加はアプリケーションの責任です。
     * @return IDescriptorPool* 
    */
    virtual IDescriptorPool*
        B3D_APIENTRY GetPool() const = 0;

    /**
     * @brief 現在のプールからの割り当てが有効かどうかを返します。 セットを割り当てた後にプールがリセットされた場合、このオブジェクトは無効になります。
     * @return 割り当てが有効な場合true、無効な場合falseを返します。
    */
    virtual bool
        B3D_APIENTRY IsValid() const = 0;

    /**
     * @brief 同じルートシグネチャで割り当てられたディスクリプタセットの全てのディスクリプタをコピーします。
     * @param _src コピーするディスクリプタセットです。 ルートシグネチャは同一である必要があります。
     * @return 正常にコピーされた場合BMRESULT_SUCCEEDが返ります。
    */
    virtual BMRESULT
        B3D_APIENTRY CopyDescriptorSet(
            IDescriptorSet* _src) = 0;

};

B3D_INTERFACE IRootSignature : public IDeviceChild
{
protected:
    B3D_APIENTRY ~IRootSignature() {}

public:
    virtual const ROOT_SIGNATURE_DESC& 
        B3D_APIENTRY GetDesc() const = 0;

    /**
     * @brief このルートシグネチャに対応するディスクリプタプール、セットを作成する際に必要なプールサイズの情報を取得します。
     * @param _num_descriptor_sets 割り当てるディスクリプタセットの数を指定します。
     * @param[out] _dst_num_register_space ルートシグネチャ内に含まれるregister_spaceの数を取得します。(register_space番号自体の最大値ではありません。) nullptrの場合値は書き込まれません。
     * @param[out] _dst_sizes 必要なプールサイズを取得します。nullptrの場合値は書き込まれません。
     * @return _dst_sizesに必要なDESCRIPTOR_POOL_SIZE構造の配列要素数を返します。
     * @remark この関数はROOT_SIGNATURE_DESCと_num_descriptor_setsに基づきプールサイズを計算するヘルパー関数です。 プールサイズを指定する際、この関数を必ず使用する必要はありません。
     * @note 複数のルートシグネチャを1つのプールで割り当てる際のサイズを取得する場合、IDevice::GetDescriptorPoolSizesAllocationInfoの利用を検討してください。
     *       _dst_sizes配列に書き込まれるDESCRIPTOR_TYPEの順序は不定です。必要な割り当てが存在しないDESCRIPTOR_TYPEはスキップされます。
    */
    virtual uint32_t
        B3D_APIENTRY GetDescriptorPoolRequirementSizes(
              uint32_t              _num_descriptor_sets
            , uint32_t*             _dst_num_register_space
            , DESCRIPTOR_POOL_SIZE* _dst_sizes) const = 0;

};

/**
 * @brief 
*/
B3D_INTERFACE IShaderModule : public IDeviceChild
{
protected:
    B3D_APIENTRY ~IShaderModule() {}

public:

};

B3D_INTERFACE IPipelineState : public IDeviceChild
{
protected:
    B3D_APIENTRY ~IPipelineState() {}

public:
    /**
     * @brief パイプラインのタイプを取得します。
     * @return PIPELINE_BIND_POINT
    */
    virtual PIPELINE_BIND_POINT
        B3D_APIENTRY GetPipelineBindPoint() const = 0;

    /**
     * @brief パイプラインキャッシュを取得します。 
     * @param [out] _dst キャッシュされたパイプラインのデータを保持するblobです。 取得された場合、参照カウントが増加することに注意してください。
     * @return 成功した場合BMRESULT_SUCCEEDが返ります。
    */
    virtual BMRESULT
        B3D_APIENTRY GetCachedBlob(
            IBlob** _dst) = 0;

};

B3D_INTERFACE ICommandQueue : public IDeviceChild
{
protected:
    B3D_APIENTRY ~ICommandQueue() {}

public:
    virtual const COMMAND_QUEUE_DESC&
        B3D_APIENTRY GetDesc() const = 0;

    /**
     * @brief このコマンドキューで現在実行中のすべての処理が完了するまでCPUで待機します。
     * @return デバイスが削除された場合、BMRESULT_FAILED_DEVICE_REMOVEDが返ります。
    */
    virtual BMRESULT
        B3D_APIENTRY WaitIdle() = 0;

    virtual BMRESULT
        B3D_APIENTRY SubmitTileBindings(
            const SUBMIT_TILE_BINDINGS_DESC& _desc) = 0;

    virtual BMRESULT
        B3D_APIENTRY Submit(
            const SUBMIT_DESC& _desc) = 0;

    virtual BMRESULT
        B3D_APIENTRY SubmitSignal(
            const SUBMIT_SIGNAL_DESC& _desc) = 0;

    virtual BMRESULT
        B3D_APIENTRY SubmitWait(
            const SUBMIT_WAIT_DESC& _desc) = 0;

    virtual BMRESULT
        B3D_APIENTRY InsertMarker(
              const char*   _marker_name
            , const COLOR4* _color) = 0;

    virtual BMRESULT
        B3D_APIENTRY BeginMarker(
              const char*    _marker_name
            , const COLOR4*  _color) = 0;

    virtual BMRESULT
        B3D_APIENTRY EndMarker() = 0;

    virtual BMRESULT
        B3D_APIENTRY GetTimestampFrequency(
            uint64_t* _dst_frequency) const = 0;

    virtual BMRESULT
        B3D_APIENTRY GetClockCalibration(
              uint64_t* _dst_gpu_timestamp
            , uint64_t* _dst_cpu_timestamp) const = 0;

};

B3D_INTERFACE ICommandAllocator : public IDeviceChild
{
protected:
    B3D_APIENTRY ~ICommandAllocator() {}

public:
    virtual const COMMAND_ALLOCATOR_DESC&
        B3D_APIENTRY GetDesc() const = 0;

    /**
     * @brief コマンドアロケーターをリセットします。 割り当てられた全てのコマンドリストは全て初期状態となり、再び記録を開始することが可能になります。 
     * @param _flags リセットで使用する追加のフラグを指定します。
     * @return リセットに成功した場合、BMRESULT_SUCCEEDが返ります。
     * @remark このアロケーターから割り当てられたどのコマンドリストも現在記録状態であってはなりません。
    */
    virtual BMRESULT
        B3D_APIENTRY Reset(COMMAND_ALLOCATOR_RESET_FLAGS _flags) = 0;

};

B3D_INTERFACE ICommandList : public IDeviceChild
{
protected:
    B3D_APIENTRY ~ICommandList() {}

public:
    virtual const COMMAND_LIST_DESC&
        B3D_APIENTRY GetDesc() const = 0;

    /**
     * @brief コマンドアロケーターを取得します。
     * @return ICommandAllocator*
     * @remark 返されたポインタのAddRef()による参照カウントの増加、Release()による減少は呼び出し側の責任です。
    */
    virtual ICommandAllocator*
        B3D_APIENTRY GetCommandAllocator() const = 0;

    /**
     * @brief 記録されたコマンドをリセットし、初期状態にします。
     * @param _flags リセットで使用する追加のフラグを指定します。 このフラグは現在予約されています。
     * @return リセットに成功した場合、BMRESULT_SUCCEEDが返ります。 
     * @remark 現在の状態が記録、実行中であってはなりません。 
     *         次の制約は暫定で必要ですが、廃止される可能性があります: リセットされる際にこのコマンドリストが使用するアロケータに関連付けられた他のすべてのコマンドリストは、現在記録状態であってはなりません。 
    */
    virtual BMRESULT
        B3D_APIENTRY Reset(COMMAND_LIST_RESET_FLAGS _flags) = 0;

    /**
     * @brief コマンドリストの記録を開始します。 記録状態に移行します。 
     * @param _begin_desc コマンドリストの記録開始時に使用する情報を指定します。
     * @return 記録開始に成功した場合、BMRESULT_SUCCEEDが返ります。 
     * @remark このコマンドリストが使用するアロケータに関連付けられた他のすべてのコマンドリストは、現在記録状態であってはなりません。 
     *         現在の状態が記録、実行中であってはなりません。
     *         RESET_COMMAND_BUFFER_BITフラグが設定されていないアロケータから割り当てられた場合、現在の状態は初期状態のみである必要があります。
    */
    virtual BMRESULT
        B3D_APIENTRY BeginRecord(const COMMAND_LIST_BEGIN_DESC& _begin_desc) = 0;

    /**
     * @brief コマンドの記録を終了し、実行可能状態に移行します。 
     * @return 記録終了に成功した場合、BMRESULT_SUCCEEDが返ります。 
     * @remark 現在の状態は記録状態のみである必要があります。
    */
    virtual BMRESULT
        B3D_APIENTRY EndRecord() = 0;

    /* Begin recording commands */

    virtual void
        B3D_APIENTRY PipelineBarrier(
            const CMD_PIPELINE_BARRIER& _args) = 0;

    virtual void
        B3D_APIENTRY SetPipelineState(
            IPipelineState* _pipeline_state) = 0;

    virtual void
        B3D_APIENTRY SetRootSignature(
              PIPELINE_BIND_POINT   _bind_point
            , IRootSignature*       _root_signature) = 0;

    virtual void
        B3D_APIENTRY BindDescriptorSet(
              PIPELINE_BIND_POINT               _bind_point
            , const CMD_BIND_DESCRIPTOR_SET&    _args) = 0;

    virtual void
        B3D_APIENTRY Push32BitConstants(
              PIPELINE_BIND_POINT               _bind_point
            , const CMD_PUSH_32BIT_CONSTANTS&   _args) = 0;

    virtual void
        B3D_APIENTRY BindIndexBufferView(
            IIndexBufferView* _view) = 0;

    virtual void
        B3D_APIENTRY BindVertexBufferViews(
            const CMD_BIND_VERTEX_BUFFER_VIEWS& _args) = 0;

    virtual void
        B3D_APIENTRY BindStreamOutputBufferViews(
            const CMD_BIND_STREAM_OUTPUT_BUFFER_VIEWS& _args) = 0;

    virtual void
        B3D_APIENTRY SetBlendConstants(
            const COLOR4& _blend_constants) = 0;

    virtual void
        B3D_APIENTRY SetStencilReference(
              STENCIL_FACE  _faces_to_set
            , uint32_t      _stencil_ref) = 0;

    virtual void
        B3D_APIENTRY SetShadingRate(
            SHADING_RATE _base_shading_rate) = 0;

    virtual void
        B3D_APIENTRY SetDepthBounds(
              float _min_depth_bounds
            , float _max_depth_bounds) = 0;

    virtual void
        B3D_APIENTRY SetSamplePositions(
            const SAMPLE_POSITION_DESC& _sample_position) = 0;

    virtual void
        B3D_APIENTRY SetLineWidth(
            float _line_width) = 0;

    // TODO: SetDepthBias 各APIのdepth_bias_scaleの使用法の差を確認する。
    virtual void
        B3D_APIENTRY SetDepthBias(
            const CMD_SET_DEPTH_BIAS& _args) = 0;

    virtual void 
        B3D_APIENTRY SetStencilCompareMask(
              STENCIL_FACE  _faces_to_set
            , uint32_t      _compare_mask) = 0;

    virtual void 
        B3D_APIENTRY SetStencilWriteMask(
              STENCIL_FACE  _faces_to_set
            , uint32_t      _write_mask) = 0;

    virtual void
        B3D_APIENTRY ResetQueryHeapRange(
            const CMD_RESET_QUERY_HEAP_RANGE& _args) = 0;

    virtual void
        B3D_APIENTRY BeginQuery(
            const QUERY_DESC& _query_desc) = 0;

    virtual void
        B3D_APIENTRY EndQuery(
            const QUERY_DESC& _query_desc) = 0;

    virtual void
        B3D_APIENTRY WriteTimeStamp(
            const QUERY_DESC& _query_desc) = 0;

    virtual void
        B3D_APIENTRY WriteAccelerationStructuresProperties(
            const CMD_WRITE_ACCELERATION_STRUCTURE& _args) = 0;

    virtual void
        B3D_APIENTRY ResolveQueryData(
            const CMD_RESOLVE_QUERY_DATA& _args) = 0;

    virtual void
        B3D_APIENTRY BeginConditionalRendering(
              IBuffer*          _buffer
            , uint64_t          _aligned_buffer_offset
            , PREDICATION_OP    _operation) = 0;

    virtual void
        B3D_APIENTRY EndConditionalRendering() = 0;

    virtual void
        B3D_APIENTRY InsertMarker(
          const char*    _marker_name
        , const COLOR4*  _color) = 0;

    virtual void
        B3D_APIENTRY BeginMarker(
          const char*    _marker_name
        , const COLOR4*  _color) = 0;

    virtual void
        B3D_APIENTRY EndMarker() = 0;

    //virtual void
    //    B3D_APIENTRY CopyResource(
    //          IResource* _dst_resource
    //        , IResource* _src_resource) = 0;

    virtual void
        B3D_APIENTRY CopyBufferRegion(
            const CMD_COPY_BUFFER_REGION& _args) = 0;

    virtual void
        B3D_APIENTRY CopyTextureRegion(
            const CMD_COPY_TEXTURE_REGION& _args) = 0;

    virtual void
        B3D_APIENTRY CopyBufferToTexture(
            const CMD_COPY_BUFFER_TO_TEXTURE& _args) = 0;
    
    virtual void
        B3D_APIENTRY CopyTextureToBuffer(
            const CMD_COPY_TEXTURE_TO_BUFFER& _args) = 0;

    virtual void
        B3D_APIENTRY ResolveTextureRegion(
            const CMD_RESOLVE_TEXTURE_REGION& _args) = 0;

    virtual void
        B3D_APIENTRY ClearDepthStencilView(
              IDepthStencilView*                 _view
            , const CLEAR_DEPTH_STENCIL_VALUE&   _clear_values) = 0;

    virtual void
        B3D_APIENTRY ClearRenderTargetView(
              IRenderTargetView*                _view
            , const CLEAR_RENDER_TARGET_VALUE&  _clear_values) = 0;

    virtual void
        B3D_APIENTRY SetViewports(
              uint32_t        _num_viewports
            , const VIEWPORT* _viewports) = 0;

    virtual void
        B3D_APIENTRY SetScissorRects(
              uint32_t            _num_scissor_rects
            , const SCISSOR_RECT* _scissor_rects) = 0;

    virtual void
        B3D_APIENTRY BeginRenderPass(
              const RENDER_PASS_BEGIN_DESC& _render_pass_begin
            , const SUBPASS_BEGIN_DESC&     _subpass_begin) = 0;

    /* Begin render pass scope */

    virtual void
        B3D_APIENTRY NextSubpass(
              const SUBPASS_BEGIN_DESC&     _subpass_begin
            , const SUBPASS_END_DESC&       _subpass_end) = 0;

    virtual void
        B3D_APIENTRY EndRenderPass(
            const SUBPASS_END_DESC& _subpass_end) = 0;

    virtual void
        B3D_APIENTRY BeginStreamOutput(
            const CMD_BEGIN_STREAM_OUTPUT& _args) = 0;

    virtual void
        B3D_APIENTRY EndStreamOutput(
            const CMD_END_STREAM_OUTPUT& _args) = 0;

    virtual void
        B3D_APIENTRY ClearAttachments(
            const CMD_CLEAR_ATTACHMENTS& _args) = 0;

    virtual void
        B3D_APIENTRY Draw(
            const DRAW_ARGUMENTS& _args) = 0;

    virtual void
        B3D_APIENTRY DrawIndexed(
            const DRAW_INDEXED_ARGUMENTS& _args) = 0;

    virtual void
        B3D_APIENTRY DrawIndirect(
            const INDIRECT_COMMAND_DESC& _command_desc) = 0;

    virtual void
        B3D_APIENTRY DrawIndexedIndirect(
            const INDIRECT_COMMAND_DESC& _command_desc) = 0;

    virtual void
        B3D_APIENTRY DispatchMeshTasks(
            uint32_t _thread_group_count_x) = 0;

    // virtual void
    //     B3D_APIENTRY ExecuteIndirect(
    //         const CMD_EXECUTE_INDIRECT& _args) = 0;

    /* End render pass scope */

    virtual void
        B3D_APIENTRY Dispatch(
            const DISPATCH_ARGUMENTS& _args) = 0;

    virtual void
        B3D_APIENTRY DispatchIndirect(
            const INDIRECT_COMMAND_DESC& _command_desc) = 0;

    virtual void
        B3D_APIENTRY ExecuteBundles(
              uint32_t              _num_secondary_command_lists
            , ICommandList* const*  _secondary_command_lists) = 0;

    // virtual void
    //     B3D_APIENTRY BuildRaytracingAccelerationStructure(
    //         const BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC& _desc) = 0;
    // 
    // virtual void
    //     B3D_APIENTRY CopyRaytracingAccelerationStructure(
    //           GpuVirtualAddress                           _dest_acceleration_structure_data
    //         , GpuVirtualAddress                           _source_acceleration_structure_data
    //         , RAYTRACING_ACCELERATION_STRUCTURE_COPY_MODE _mode) = 0;
    // 
    // virtual void
    //     B3D_APIENTRY DispatchRays(
    //         const DISPATCH_RAYS_DESC& _desc) = 0;

    /* End recording commands */

};

B3D_INTERFACE IFence : public IDeviceChild
{
protected:
    B3D_APIENTRY ~IFence() {}

public:
    virtual const FENCE_DESC&
        B3D_APIENTRY GetDesc() const = 0;
    
    /**
     * @brief フェンスをリセット、またはFENCE_DESCの値で再作成します。 
     * @return リセットに成功した場合、BMRESULT_SUCCEEDが返ります。
     * @remark FENCE_TYPE_BINARY_GPU_TO_CPUフェンスの場合、シグナルの再送信前にこの関数を呼び出してリセットする必要があります。
     *         それ以外の場合、フェンスはFENCE_DESCのパラメータに基づいて再作成されます。
     *         必要である場合を除いて、この関数はFENCE_TYPE_BINARY_GPU_TO_CPU以外のフェンスで呼び出すべきではありません。
    */
    virtual BMRESULT
        B3D_APIENTRY Reset() = 0;

    /**
     * @brief 現在のフェンスの状態、値を取得します。FENCE_TYPE_BINARY_GPU_TO_GPUの場合この関数呼び出しは無効です。
     * @param [out] _value 完了したフェンス値が代入されます。FENCE_TYPE_BINARY_GPU_TO_CPUの場合この値は操作されません。
     * @return 以前に設定されたシグナル値の最大値まで完了している場合BMRESULT_SUCCEED、完了していない場合BMRESULT_SUCCEED_NOT_READYが返ります。
     *         FENCE_TYPE_BINARY_GPU_TO_CPUの場合、シグナルされていた場合BMRESULT_SUCCEED、それ以外の場合BMRESULT_SUCCEED_NOT_READYが返ります。
     * @remark デバイスが削除された場合、BMRESULT_FAILED_DEVICE_REMOVEDが返ります。
    */
    virtual BMRESULT
        B3D_APIENTRY GetCompletedValue(
            uint64_t* _value) const = 0;

    /**
     * @brief 指定のシグナル値に達するまで、またはシグナルされるまで待機します。FENCE_TYPE_BINARY_GPU_TO_GPUの場合この関数呼び出しは無効です。
     * @param _value 待機するシグナル値。FENCE_TYPE_BINARY_GPU_TO_CPUの場合この値は使用されません。
     * @param _timeout_millisec 待機継続するミリ秒単位の時間。ゼロを指定すると呼び出し時点での状態を即時に返します。
     * @return _timeout_millisec時間内に_valueの値が完了した場合BMRESULT_SUCCEEDが返ります。時間内に完了しなかった場合、BMRESULT_SUCCEED_TIMEOUTが返ります。 
     *         FENCE_TYPE_BINARY_GPU_TO_CPUの場合、時間内にシグナルされた場合BMRESULT_SUCCEED、それ以外の場合BMRESULT_SUCCEED_TIMEOUTが返ります。
     *         どちらの場合でも、_timeout_millisecにゼロが指定されており、現在シグナルされていない場合、BMRESULT_SUCCEED_NOT_READYが返ります。
     * @remark デバイスが削除された場合、BMRESULT_FAILED_DEVICE_REMOVEDが返ります。
    */
    virtual BMRESULT
        B3D_APIENTRY Wait(
              uint64_t _value
            , uint32_t _timeout_millisec) = 0;

    /**
     * @brief CPUからシグナル値を設定します。FENCE_TYPE_BINARY_*の場合この関数呼び出しは無効です。
     * @param _value シグナル値。 _valueは現在またはシグナル予定のフェンス値より大きい必要があります。
     * @return CPUシグナル操作が成功した場合、BMRESULT_SUCCEEDが返ります。それ以外の場合、BMRESULT_FAILED以下のエラーが返ります。
     * @remark この関数は非スレッドセーフです。
    */
    virtual BMRESULT
        B3D_APIENTRY Signal(
            uint64_t _value) = 0;

};

B3D_INTERFACE IQueryHeap : public IDeviceChild
{
protected:
    B3D_APIENTRY ~IQueryHeap() {}

public:
    virtual const QUERY_HEAP_DESC&
        B3D_APIENTRY GetDesc() const = 0;

};

// TODO: ICommandSignature 
B3D_INTERFACE ICommandSignature : public IDeviceChild
{
protected:
    B3D_APIENTRY ~ICommandSignature() {}

public:
    virtual const COMMAND_SIGNATURE_DESC&
        B3D_APIENTRY GetDesc() const = 0;

};

// TODO: IAccelerationStructure 
B3D_INTERFACE IAccelerationStructure : public IDeviceChild
{
protected:
    B3D_APIENTRY ~IAccelerationStructure() {}

public:

    virtual IBuffer*
        B3D_APIENTRY GetBuffer() const = 0;

};


#pragma endregion interfaces


}// namespace buma3d

#pragma warning(pop)
