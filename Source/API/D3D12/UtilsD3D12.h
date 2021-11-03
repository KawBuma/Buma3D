#pragma once

#define HR_TRACE_IF_FAILED(x) buma3d::util::HRCheckResult(this, x, __FILE__, __LINE__)
#define HR_TRACE_IF_FAILED_EX(device_or_device_child, x) buma3d::util::HRCheckResult(device_or_device_child, x, __FILE__, __LINE__)

#define B3D_ADD_DEBUG_MSG(severity, category, msg) if (buma3d::util::IsEnabledDebug(this)) { buma3d::util::AddDebugMessage(this, severity, category, msg); }
#define B3D_ADD_DEBUG_MSG2(severity, category, .../*msgs*/) if (buma3d::util::IsEnabledDebug(this)) { buma3d::util::AddDebugMessage(this, severity, category, buma3d::hlp::StringConvolution(__VA_ARGS__).c_str()); }
#define B3D_ADD_DEBUG_MSG_EX(device_or_device_child, severity, category, msg) if (buma3d::util::IsEnabledDebug(device_or_device_child)) { buma3d::util::AddDebugMessage(device_or_device_child, severity, category, msg); }
#define B3D_ADD_DEBUG_MSG_EX2(device_or_device_child, severity, category, .../*msgs*/) if (buma3d::util::IsEnabledDebug(device_or_device_child)) { buma3d::util::AddDebugMessage(device_or_device_child, severity, category, buma3d::hlp::StringConvolution(__VA_ARGS__).c_str()); }

#define B3D_ADD_DEBUG_MSG_INFO_B3D(.../*msgs*/) if (buma3d::util::IsEnabledDebug(this)) { buma3d::util::AddDebugMessage(this, buma3d::DEBUG_MESSAGE_SEVERITY_OTHER, buma3d::DEBUG_MESSAGE_CATEGORY_FLAG_B3D_DETAILS, buma3d::hlp::StringConvolution( __FUNCTION__ ": ", __VA_ARGS__).c_str()); }

namespace buma3d
{
namespace hlp
{

template <typename T, std::enable_if_t<std::is_base_of_v<IUnknown, T>, int> = 0>
inline ULONG SafeRelease(T*& _com)
{
    if (_com)
    {
        auto count = _com->Release();
        _com = nullptr;
        return count;
    }
    return 0;
}

template <typename T, std::enable_if_t<std::is_base_of_v<IUnknown, T>, int> = 0>
inline ULONG GetRefCount(T* _com)
{
    if (_com)
    {
        auto count = _com->AddRef();
        _com->Release();
        return count - 1;
    }
    return 0;
}

}
}

namespace buma3d
{
namespace util

{
inline DXGI_INFO_QUEUE_MESSAGE_SEVERITY GetNativeMessageSeverity(DEBUG_MESSAGE_SEVERITY _severity)
{
    switch (_severity)
    {
    case buma3d::DEBUG_MESSAGE_SEVERITY_INFO       : return DXGI_INFO_QUEUE_MESSAGE_SEVERITY_INFO;
    case buma3d::DEBUG_MESSAGE_SEVERITY_WARNING    : return DXGI_INFO_QUEUE_MESSAGE_SEVERITY_WARNING;
    case buma3d::DEBUG_MESSAGE_SEVERITY_ERROR      : return DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR;
    case buma3d::DEBUG_MESSAGE_SEVERITY_CORRUPTION : return DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION;
    case buma3d::DEBUG_MESSAGE_SEVERITY_OTHER      : return DXGI_INFO_QUEUE_MESSAGE_SEVERITY_MESSAGE;
    default:
        return DXGI_INFO_QUEUE_MESSAGE_SEVERITY(-1);
    }
}

inline DEBUG_MESSAGE_SEVERITY GetB3DMessageSeverity(DXGI_INFO_QUEUE_MESSAGE_SEVERITY _severity)
{
    switch (_severity)
    {
    case DXGI_INFO_QUEUE_MESSAGE_SEVERITY_INFO       : return buma3d::DEBUG_MESSAGE_SEVERITY_INFO;
    case DXGI_INFO_QUEUE_MESSAGE_SEVERITY_WARNING    : return buma3d::DEBUG_MESSAGE_SEVERITY_WARNING;
    case DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR      : return buma3d::DEBUG_MESSAGE_SEVERITY_ERROR;
    case DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION : return buma3d::DEBUG_MESSAGE_SEVERITY_CORRUPTION;
    case DXGI_INFO_QUEUE_MESSAGE_SEVERITY_MESSAGE    : return buma3d::DEBUG_MESSAGE_SEVERITY_OTHER;
    default:
        return DEBUG_MESSAGE_SEVERITY(-1);
    }
}

inline uint32_t GetNativeMessageCategoriesMask(DEBUG_MESSAGE_CATEGORY_FLAGS _flags)
{
    // DXGI_INFO_QUEUE_MESSAGE_CATEGORY_UNKNOWN       == 0x1;
    // DXGI_INFO_QUEUE_MESSAGE_CATEGORY_MISCELLANEOUS == 0x2;
    // ...

    uint32_t result = 0;

    if (_flags & buma3d::DEBUG_MESSAGE_CATEGORY_FLAG_UNKNOWN)
        hlp::SetBit(result, SCAST<uint32_t>(DXGI_INFO_QUEUE_MESSAGE_CATEGORY_UNKNOWN));

    if (_flags & buma3d::DEBUG_MESSAGE_CATEGORY_FLAG_MISCELLANEOUS)
        hlp::SetBit(result, SCAST<uint32_t>(DXGI_INFO_QUEUE_MESSAGE_CATEGORY_MISCELLANEOUS));

    if (_flags & buma3d::DEBUG_MESSAGE_CATEGORY_FLAG_INITIALIZATION)
        hlp::SetBit(result, SCAST<uint32_t>(DXGI_INFO_QUEUE_MESSAGE_CATEGORY_INITIALIZATION));

    if (_flags & buma3d::DEBUG_MESSAGE_CATEGORY_FLAG_CLEANUP)
        hlp::SetBit(result, SCAST<uint32_t>(DXGI_INFO_QUEUE_MESSAGE_CATEGORY_CLEANUP));

    if (_flags & buma3d::DEBUG_MESSAGE_CATEGORY_FLAG_COMPILATION)
        hlp::SetBit(result, SCAST<uint32_t>(DXGI_INFO_QUEUE_MESSAGE_CATEGORY_COMPILATION));

    if (_flags & buma3d::DEBUG_MESSAGE_CATEGORY_FLAG_STATE_CREATION)
        hlp::SetBit(result, SCAST<uint32_t>(DXGI_INFO_QUEUE_MESSAGE_CATEGORY_STATE_CREATION));

    if (_flags & buma3d::DEBUG_MESSAGE_CATEGORY_FLAG_STATE_SETTING)
        hlp::SetBit(result, SCAST<uint32_t>(DXGI_INFO_QUEUE_MESSAGE_CATEGORY_STATE_SETTING));

    if (_flags & buma3d::DEBUG_MESSAGE_CATEGORY_FLAG_STATE_GETTING)
        hlp::SetBit(result, SCAST<uint32_t>(DXGI_INFO_QUEUE_MESSAGE_CATEGORY_STATE_GETTING));

    if (_flags & buma3d::DEBUG_MESSAGE_CATEGORY_FLAG_RESOURCE_MANIPULATION)
        hlp::SetBit(result, SCAST<uint32_t>(DXGI_INFO_QUEUE_MESSAGE_CATEGORY_RESOURCE_MANIPULATION));

    if (_flags & buma3d::DEBUG_MESSAGE_CATEGORY_FLAG_EXECUTION)
        hlp::SetBit(result, SCAST<uint32_t>(DXGI_INFO_QUEUE_MESSAGE_CATEGORY_EXECUTION));

    if (_flags & buma3d::DEBUG_MESSAGE_CATEGORY_FLAG_SHADER)
        hlp::SetBit(result, SCAST<uint32_t>(DXGI_INFO_QUEUE_MESSAGE_CATEGORY_SHADER));

    return result;
}

inline DEBUG_MESSAGE_CATEGORY_FLAG GetB3DMessageCategory(DXGI_INFO_QUEUE_MESSAGE_CATEGORY _category)
{
    switch (_category)
    {
    case DXGI_INFO_QUEUE_MESSAGE_CATEGORY_UNKNOWN               : return buma3d::DEBUG_MESSAGE_CATEGORY_FLAG_UNKNOWN;
    case DXGI_INFO_QUEUE_MESSAGE_CATEGORY_MISCELLANEOUS         : return buma3d::DEBUG_MESSAGE_CATEGORY_FLAG_MISCELLANEOUS;
    case DXGI_INFO_QUEUE_MESSAGE_CATEGORY_INITIALIZATION        : return buma3d::DEBUG_MESSAGE_CATEGORY_FLAG_INITIALIZATION;
    case DXGI_INFO_QUEUE_MESSAGE_CATEGORY_CLEANUP               : return buma3d::DEBUG_MESSAGE_CATEGORY_FLAG_CLEANUP;
    case DXGI_INFO_QUEUE_MESSAGE_CATEGORY_COMPILATION           : return buma3d::DEBUG_MESSAGE_CATEGORY_FLAG_COMPILATION;
    case DXGI_INFO_QUEUE_MESSAGE_CATEGORY_STATE_CREATION        : return buma3d::DEBUG_MESSAGE_CATEGORY_FLAG_STATE_CREATION;
    case DXGI_INFO_QUEUE_MESSAGE_CATEGORY_STATE_SETTING         : return buma3d::DEBUG_MESSAGE_CATEGORY_FLAG_STATE_SETTING;
    case DXGI_INFO_QUEUE_MESSAGE_CATEGORY_STATE_GETTING         : return buma3d::DEBUG_MESSAGE_CATEGORY_FLAG_STATE_GETTING;
    case DXGI_INFO_QUEUE_MESSAGE_CATEGORY_RESOURCE_MANIPULATION : return buma3d::DEBUG_MESSAGE_CATEGORY_FLAG_RESOURCE_MANIPULATION;
    case DXGI_INFO_QUEUE_MESSAGE_CATEGORY_EXECUTION             : return buma3d::DEBUG_MESSAGE_CATEGORY_FLAG_EXECUTION;
    case DXGI_INFO_QUEUE_MESSAGE_CATEGORY_SHADER                : return buma3d::DEBUG_MESSAGE_CATEGORY_FLAG_SHADER;
    default:
        return buma3d::DEBUG_MESSAGE_CATEGORY_FLAG_UNKNOWN;
    }
}

// 最下位ビットに存在するDXGI_INFO_QUEUE_MESSAGE_CATEGORY を取得します。
// ビットがセットされていない場合、DXGI_INFO_QUEUE_MESSAGE_CATEGORY(-1)が返ります。
inline DXGI_INFO_QUEUE_MESSAGE_CATEGORY GetNativeCategoryFromMaskLSB(DEBUG_MESSAGE_CATEGORY_FLAGS _flags)
{
    return SCAST<DXGI_INFO_QUEUE_MESSAGE_CATEGORY>(hlp::GetFirstBitIndex(uint32_t(_flags)));
}

inline DXGI_MODE_ROTATION GetNativeRotationMode(ROTATION_MODE _rotation)
{
    switch (_rotation)
    {
    case buma3d::ROTATION_MODE_IDENTITY : return DXGI_MODE_ROTATION_IDENTITY;
    case buma3d::ROTATION_MODE_ROTATE90 : return DXGI_MODE_ROTATION_ROTATE90;
    case buma3d::ROTATION_MODE_ROTATE180: return DXGI_MODE_ROTATION_ROTATE180;
    case buma3d::ROTATION_MODE_ROTATE270: return DXGI_MODE_ROTATION_ROTATE270;
    default:
        return DXGI_MODE_ROTATION_UNSPECIFIED;
    }
}

inline ROTATION_MODE GetB3DRotationMode(DXGI_MODE_ROTATION _rotation)
{
    switch (_rotation)
    {
    case DXGI_MODE_ROTATION_UNSPECIFIED :
    case DXGI_MODE_ROTATION_IDENTITY    : return buma3d::ROTATION_MODE_IDENTITY;
    case DXGI_MODE_ROTATION_ROTATE90    : return buma3d::ROTATION_MODE_ROTATE90;
    case DXGI_MODE_ROTATION_ROTATE180   : return buma3d::ROTATION_MODE_ROTATE180;
    case DXGI_MODE_ROTATION_ROTATE270   : return buma3d::ROTATION_MODE_ROTATE270;
    default:
        return ROTATION_MODE_IDENTITY;
    }
}

inline D3D12_COMMAND_LIST_TYPE GetNativeCommandType(COMMAND_TYPE _type)
{
    switch (_type)
    {
    case buma3d::COMMAND_TYPE_DIRECT        : return D3D12_COMMAND_LIST_TYPE_DIRECT;
    case buma3d::COMMAND_TYPE_DIRECT_ONLY   : return D3D12_COMMAND_LIST_TYPE_DIRECT;
    case buma3d::COMMAND_TYPE_COMPUTE_ONLY  : return D3D12_COMMAND_LIST_TYPE_COMPUTE;
    case buma3d::COMMAND_TYPE_COPY_ONLY     : return D3D12_COMMAND_LIST_TYPE_COPY;
    case buma3d::COMMAND_TYPE_VIDEO_DECODE  : return D3D12_COMMAND_LIST_TYPE_VIDEO_DECODE;
    case buma3d::COMMAND_TYPE_VIDEO_PROCESS : return D3D12_COMMAND_LIST_TYPE_VIDEO_PROCESS;
    case buma3d::COMMAND_TYPE_VIDEO_ENCODE  : return D3D12_COMMAND_LIST_TYPE_VIDEO_ENCODE;
    default:
        return D3D12_COMMAND_LIST_TYPE_DIRECT;
    }
}

inline D3D12_HEAP_FLAGS GetNativeHeapFlags(RESOURCE_HEAP_FLAGS _flags)
{
    D3D12_HEAP_FLAGS result = D3D12_HEAP_FLAG_NONE;

    if (_flags & (RESOURCE_HEAP_FLAG_SHARED_EXPORT_TO_HANDLE | RESOURCE_HEAP_FLAG_SHARED_IMPORT_FROM_HANDLE))
        result |= D3D12_HEAP_FLAG_SHARED;

    if (_flags & RESOURCE_HEAP_FLAG_SHARED_CROSS_ADAPTER)
        result |= D3D12_HEAP_FLAG_SHARED_CROSS_ADAPTER;

    return result;
}

inline D3D12_RESOURCE_DIMENSION GetNativeResourceDimension(RESOURCE_DIMENSION _dimention)
{
    switch (_dimention)
    {
    case buma3d::RESOURCE_DIMENSION_BUFFER : return D3D12_RESOURCE_DIMENSION_BUFFER;
    case buma3d::RESOURCE_DIMENSION_TEX1D  : return D3D12_RESOURCE_DIMENSION_TEXTURE1D;
    case buma3d::RESOURCE_DIMENSION_TEX2D  : return D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    case buma3d::RESOURCE_DIMENSION_TEX3D  : return D3D12_RESOURCE_DIMENSION_TEXTURE3D;
    default:
        return D3D12_RESOURCE_DIMENSION_UNKNOWN;
    }
}

inline RESOURCE_DIMENSION GetB3DResourceDimension(D3D12_RESOURCE_DIMENSION _dimention)
{
    switch (_dimention)
    {
    case D3D12_RESOURCE_DIMENSION_BUFFER    : return buma3d::RESOURCE_DIMENSION_BUFFER;
    case D3D12_RESOURCE_DIMENSION_TEXTURE1D : return buma3d::RESOURCE_DIMENSION_TEX1D;
    case D3D12_RESOURCE_DIMENSION_TEXTURE2D : return buma3d::RESOURCE_DIMENSION_TEX2D;
    case D3D12_RESOURCE_DIMENSION_TEXTURE3D : return buma3d::RESOURCE_DIMENSION_TEX3D;
    default:
        return RESOURCE_DIMENSION(-1);
    }
}

inline D3D12_TEXTURE_LAYOUT GetNativeTextureLayout(TEXTURE_LAYOUT _layout)
{
    // NOTE: 64KB_UNDEFINED_SWIZZLE, 64KB_STANDARD_SWIZZLE に関しては、この関数の外部で指定する必要があります。
    switch (_layout)
    {
    case buma3d::TEXTURE_LAYOUT_UNKNOWN   : return D3D12_TEXTURE_LAYOUT_UNKNOWN;
    case buma3d::TEXTURE_LAYOUT_ROW_MAJOR : return D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    default:
        return D3D12_TEXTURE_LAYOUT(-1);
    }
}

inline TEXTURE_LAYOUT GetB3DTextureLayout(D3D12_TEXTURE_LAYOUT _layout)
{
    // NOTE: 64KB_UNDEFINED_SWIZZLE, 64KB_STANDARD_SWIZZLE に関しては、この関数の外部で指定する必要があります。
    switch (_layout)
    {
    case D3D12_TEXTURE_LAYOUT_UNKNOWN   : return buma3d::TEXTURE_LAYOUT_UNKNOWN;
    case D3D12_TEXTURE_LAYOUT_ROW_MAJOR : return buma3d::TEXTURE_LAYOUT_ROW_MAJOR;
    default:
        return TEXTURE_LAYOUT(-1);
    }
}

inline D3D12_RESOURCE_FLAGS GetNativeResourceBufFlags(RESOURCE_FLAGS _flags, BUFFER_USAGE_FLAGS _buf_usage)
{
    D3D12_RESOURCE_FLAGS result = D3D12_RESOURCE_FLAG_NONE;

    if (_flags & RESOURCE_FLAG_ALLOW_CROSS_ADAPTER)
        result |= D3D12_RESOURCE_FLAG_ALLOW_CROSS_ADAPTER;

    if (_buf_usage & BUFFER_USAGE_FLAG_UNORDERED_ACCESS_BUFFER)
        result |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

    // NOTE: D3D12の全てのバッファリソースは、キュー間のリソース同時アクセスが常に有効とされるため、このフラグを明示的に指定することは無効と定義されています。
    // if (_flags & RESOURCE_FLAG_ALLOW_SIMULTANEOUS_ACCESS)
    //     result |= D3D12_RESOURCE_FLAG_ALLOW_SIMULTANEOUS_ACCESS;

    return result;
}

inline D3D12_RESOURCE_FLAGS GetNativeResourceTexFlags(RESOURCE_FLAGS _flags, TEXTURE_USAGE_FLAGS _tex_usage)
{
    D3D12_RESOURCE_FLAGS result = D3D12_RESOURCE_FLAG_NONE;

    if (!(_tex_usage & TEXTURE_USAGE_FLAG_SHADER_RESOURCE) && !(_tex_usage & TEXTURE_USAGE_FLAG_INPUT_ATTACHMENT) &&
        _tex_usage & TEXTURE_USAGE_FLAG_DEPTH_STENCIL_ATTACHMENT)// D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCILと一緒に使用する必要があります。
        result |= D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;

    if (_tex_usage & TEXTURE_USAGE_FLAG_UNORDERED_ACCESS)
        result |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

    if (_tex_usage & TEXTURE_USAGE_FLAG_COLOR_ATTACHMENT)
        result |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

    if (_tex_usage & TEXTURE_USAGE_FLAG_DEPTH_STENCIL_ATTACHMENT)
        result |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    if (_flags & RESOURCE_FLAG_ALLOW_CROSS_ADAPTER)
        result |= D3D12_RESOURCE_FLAG_ALLOW_CROSS_ADAPTER;

    if (_flags & RESOURCE_FLAG_ALLOW_SIMULTANEOUS_ACCESS)
        result |= D3D12_RESOURCE_FLAG_ALLOW_SIMULTANEOUS_ACCESS;

    //if (_flags & /*TODO: Vulkanのビデオ用インターフェースが登場したら*/)
    // result |= D3D12_RESOURCE_FLAG_VIDEO_DECODE_REFERENCE_ONLY;

    return result;
}

// mip_levelsがB3D_USE_ALL_MIPSの場合生成可能なmipの最大値を算出します。
inline uint32_t CalcMipLevels(const TEXTURE_DESC& _desc)
{
    if (_desc.mip_levels == B3D_USE_ALL_MIPS)
        return 1 + SCAST<uint32_t>(floorf(log2f(SCAST<float>(std::max({ _desc.extent.width, _desc.extent.height, _desc.extent.depth })))));
    else
        return _desc.mip_levels;
}

template<typename T>
inline T CalcMipExtents(uint32_t _mip_slice, const EXTENT3D& _extent_mip0)
{
    return {  std::max(_extent_mip0.width  >> _mip_slice, 1ui32)
            , std::max(_extent_mip0.height >> _mip_slice, 1ui32)
            , std::max(_extent_mip0.depth  >> _mip_slice, 1ui32) };
}
template<typename T>
inline T CalcMipExtents2D(uint32_t _mip_slice, const EXTENT3D& _extent_mip0)
{
    return {  std::max(_extent_mip0.width  >> _mip_slice, 1ui32)
            , std::max(_extent_mip0.height >> _mip_slice, 1ui32) };
}

inline UINT3 CalcMipExtents(uint32_t _mip_slice, uint32_t _width_mip0, uint32_t _height_mip0, uint32_t _depth_mip0)
{
    return {  std::max(_width_mip0  >> _mip_slice, 1ui32)
            , std::max(_height_mip0 >> _mip_slice, 1ui32)
            , std::max(_depth_mip0  >> _mip_slice, 1ui32) };
}

inline UINT3 CalcNextMipExtents(uint32_t _width, uint32_t _height, uint32_t _depth)
{
    return {  std::max(_width  >> 1, 1ui32)
            , std::max(_height >> 1, 1ui32)
            , std::max(_depth  >> 1, 1ui32) };
}

inline void GetNativeResourceDesc(const RESOURCE_DESC& _desc, D3D12_RESOURCE_DESC* _result)
{
    _result->Dimension = GetNativeResourceDimension(_desc.dimension);
    _result->Alignment = 0; // 0 (auto)
    if (_desc.dimension == RESOURCE_DIMENSION_BUFFER)
    {
        _result->Width              = _desc.buffer.size_in_bytes;
        _result->Height             = 1;
        _result->DepthOrArraySize   = 1;
        _result->Format             = DXGI_FORMAT_UNKNOWN;
        _result->SampleDesc.Count   = 1;
        _result->SampleDesc.Quality = 0;
        _result->MipLevels          = 1;
        _result->Layout             = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        _result->Flags              = GetNativeResourceBufFlags(_desc.flags, _desc.buffer.usage);
    }
    else
    {
        _result->Width              = _desc.texture.extent.width;
        _result->Height             = _desc.texture.extent.height;
        _result->DepthOrArraySize   = SCAST<UINT16>(_desc.dimension == RESOURCE_DIMENSION_TEX3D ? _desc.texture.extent.depth : _desc.texture.array_size);
        _result->MipLevels          = SCAST<UINT16>(util::CalcMipLevels(_desc.texture));
        _result->SampleDesc.Count   = _desc.texture.sample_count;
        _result->SampleDesc.Quality = 0;
        _result->Layout             = GetNativeTextureLayout(_desc.texture.layout);
        _result->Flags              = GetNativeResourceTexFlags(_desc.flags, _desc.texture.usage);

        if (_desc.texture.usage & TEXTURE_USAGE_FLAG_DEPTH_STENCIL_ATTACHMENT &&
            _desc.texture.usage & (TEXTURE_USAGE_FLAG_SHADER_RESOURCE | TEXTURE_USAGE_FLAG_INPUT_ATTACHMENT))
        {
            // usageに深度ステンシルが指定されており、深度ステンシル以外のusageも含んで作成された場合、フォーマットをTYPELESSとして使用します。
            // DSVとSRVの両方を作成するために必要です: https://docs.microsoft.com/en-us/windows/win32/direct3d11/d3d10-graphics-programming-guide-depth-stencil
            _result->Format = GetNativeDepthStencilTypelessFormat(_desc.texture.format_desc.format);
        }
        else
        {
            _result->Format = GetNativeFormat(_desc.texture.format_desc.format);
        }
    }
}

inline D3D12_CLEAR_VALUE* GetNativeClearValue(RESOURCE_FORMAT _format, const CLEAR_VALUE& _cv, D3D12_CLEAR_VALUE* _result)
{
    _result->Format = GetNativeFormat(_format);
    std::memcpy(_result->Color, &_cv.color.float4.x, sizeof(_cv.color.float4));
    return _result;
}

/**
 * @brief 必要な場合、D3D12の仕様による固定されたステートを返します。それ以外の場合、D3D12_RESOURCE_STATE_COMMONが返ります。
 * @param _type ヒープタイプを指定します。
 * @param _is_ac_buffer 加速構造バッファとして使用する場合にtrueを指定する必要があります。これは、D3D12の仕様に基いた固定ステートを返すために必要です。
 * @return 引数のタイプが未定義の場合、D3D12_RESOURCE_STATES(-1)が返ります。
 * @note 加速構造バッファにも作成時に固定ステートが必要ですが、D3D12_HEAP_TYPEによる識別が不可能なため、別のアプローチが必要でした。
 *       (更にヒープタイプもDEFAULTまたはCUSTOMヒープに制限される。)
*/
inline D3D12_RESOURCE_STATES GetFixedStateFromHeapType(D3D12_HEAP_TYPE _type, bool _is_as_buffer)
{
    if (_is_as_buffer)
        return D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;

    switch (_type)
    {
    case D3D12_HEAP_TYPE_UPLOAD   : return D3D12_RESOURCE_STATE_GENERIC_READ;
    case D3D12_HEAP_TYPE_READBACK : return D3D12_RESOURCE_STATE_COPY_DEST;

    case D3D12_HEAP_TYPE_DEFAULT:
    case D3D12_HEAP_TYPE_CUSTOM:
        return D3D12_RESOURCE_STATE_COMMON;

    default:// 未定義
        return D3D12_RESOURCE_STATES(-1);
    }
}

inline D3D12_RESOURCE_STATES GetFixedStateFromResourceFlags(RESOURCE_FLAGS _flags, bool _is_as_buffer)
{
    if (_is_as_buffer)
        return D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;

    else if (_flags & RESOURCE_FLAG_ACCESS_GENERIC_MEMORY_READ_FIXED)
        return D3D12_RESOURCE_STATE_GENERIC_READ;

    else if (_flags & RESOURCE_FLAG_ACCESS_COPY_DST_FIXED)
        return D3D12_RESOURCE_STATE_COPY_DEST;

    else
        return D3D12_RESOURCE_STATE_COMMON;
}

inline uint32_t GetNativeAspectFlags(TEXTURE_ASPECT_FLAGS _flags)
{
    if (_flags == buma3d::TEXTURE_ASPECT_FLAG_NONE)
        return 0;

    else if (_flags & buma3d::TEXTURE_ASPECT_FLAG_DEPTH && _flags & buma3d::TEXTURE_ASPECT_FLAG_STENCIL)
        return 0;

    else if (_flags & (buma3d::TEXTURE_ASPECT_FLAG_COLOR | buma3d::TEXTURE_ASPECT_FLAG_DEPTH | buma3d::TEXTURE_ASPECT_FLAG_PLANE_0))
        return 0;

    else if (_flags & (buma3d::TEXTURE_ASPECT_FLAG_STENCIL | buma3d::TEXTURE_ASPECT_FLAG_PLANE_1))
        return 1;

    else if (_flags & buma3d::TEXTURE_ASPECT_FLAG_PLANE_2)
        return 2;

    // if (_flags & buma3d::TEXTURE_ASPECT_FLAG_METADATA)
    // if (_flags & buma3d::TEXTURE_ASPECT_FLAG_PLANE_3)

    return (uint32_t)-1;
}

inline uint32_t ConvertNativeSubresourceOffset(uint32_t _num_mips, uint32_t _array_size, const SUBRESOURCE_OFFSET& _offset)
{
    return _offset.mip_slice + ((_num_mips * _offset.array_slice) + (_num_mips * _array_size * GetNativeAspectFlags(_offset.aspect)));
}

inline uint32_t ConvertNativeSubresourceOffset(uint32_t _num_mips, uint32_t _array_size, const SUBRESOURCE_OFFSET& _offset, int32_t _plane_offset)
{
    return _offset.mip_slice + ((_num_mips * _offset.array_slice) + (_num_mips * _array_size * SCAST<uint32_t>(SCAST<int32_t>(GetNativeAspectFlags(_offset.aspect)) + _plane_offset)));
}

inline uint32_t CalcSubresourceOffset(uint32_t _num_mips, uint32_t _array_size, uint32_t _mip_slice, uint32_t _array_slice = 0, uint32_t _plane_slice = 0)
{
    return _mip_slice + ((_num_mips * _array_slice) + (_num_mips * _array_size * _plane_slice));
}

inline D3D12_SHADER_COMPONENT_MAPPING GetNativeComponentSwizzle(COMPONENT_SWIZZLE _swizzle)
{
    switch (_swizzle)
    {
    case buma3d::COMPONENT_SWIZZLE_ZERO  : return D3D12_SHADER_COMPONENT_MAPPING_FORCE_VALUE_0;
    case buma3d::COMPONENT_SWIZZLE_ONE   : return D3D12_SHADER_COMPONENT_MAPPING_FORCE_VALUE_1;
    case buma3d::COMPONENT_SWIZZLE_R     : return D3D12_SHADER_COMPONENT_MAPPING_FROM_MEMORY_COMPONENT_0;
    case buma3d::COMPONENT_SWIZZLE_G     : return D3D12_SHADER_COMPONENT_MAPPING_FROM_MEMORY_COMPONENT_1;
    case buma3d::COMPONENT_SWIZZLE_B     : return D3D12_SHADER_COMPONENT_MAPPING_FROM_MEMORY_COMPONENT_2;
    case buma3d::COMPONENT_SWIZZLE_A     : return D3D12_SHADER_COMPONENT_MAPPING_FROM_MEMORY_COMPONENT_3;

    //case buma3d::COMPONENT_SWIZZLE_IDENTITY:
    default:
        return D3D12_SHADER_COMPONENT_MAPPING(-1);
    }
}

inline UINT* ConvertNativeComponentMapping(const COMPONENT_MAPPING& _mapping, UINT* _result)
{
    *_result = D3D12_ENCODE_SHADER_4_COMPONENT_MAPPING
    (
          (_mapping.r == buma3d::COMPONENT_SWIZZLE_IDENTITY ? D3D12_SHADER_COMPONENT_MAPPING_FROM_MEMORY_COMPONENT_0 : GetNativeComponentSwizzle(_mapping.r))
        , (_mapping.g == buma3d::COMPONENT_SWIZZLE_IDENTITY ? D3D12_SHADER_COMPONENT_MAPPING_FROM_MEMORY_COMPONENT_1 : GetNativeComponentSwizzle(_mapping.g))
        , (_mapping.b == buma3d::COMPONENT_SWIZZLE_IDENTITY ? D3D12_SHADER_COMPONENT_MAPPING_FROM_MEMORY_COMPONENT_2 : GetNativeComponentSwizzle(_mapping.b))
        , (_mapping.a == buma3d::COMPONENT_SWIZZLE_IDENTITY ? D3D12_SHADER_COMPONENT_MAPPING_FROM_MEMORY_COMPONENT_3 : GetNativeComponentSwizzle(_mapping.a))
    );
    return _result;
}

inline DXGI_FORMAT GetNativeIndexType(INDEX_TYPE _type)
{
    switch (_type)
    {
    case buma3d::INDEX_TYPE_UINT16 : return DXGI_FORMAT_R16_UINT;
    case buma3d::INDEX_TYPE_UINT32 : return DXGI_FORMAT_R32_UINT;
    case buma3d::INDEX_TYPE_UINT8  : return DXGI_FORMAT_R8_UINT;
    default:
        return DXGI_FORMAT(-1);
    }
}

inline bool IsIdentifyComponentMapping(const COMPONENT_MAPPING& _mapping)
{
    return (_mapping.r == COMPONENT_SWIZZLE_R || _mapping.r == COMPONENT_SWIZZLE_IDENTITY) &&
            (_mapping.g == COMPONENT_SWIZZLE_G || _mapping.g == COMPONENT_SWIZZLE_IDENTITY) &&
            (_mapping.b == COMPONENT_SWIZZLE_B || _mapping.b == COMPONENT_SWIZZLE_IDENTITY) &&
            (_mapping.a == COMPONENT_SWIZZLE_A || _mapping.a == COMPONENT_SWIZZLE_IDENTITY);
}

inline D3D12_DSV_FLAGS GetNativeDepthStencilViewFlags(DEPTH_STENCIL_VIEW_FLAGS _flags)
{
    D3D12_DSV_FLAGS result = D3D12_DSV_FLAG_NONE;

    if (_flags & DEPTH_STENCIL_VIEW_FLAG_READ_ONLY_DEPTH)
        result |= D3D12_DSV_FLAG_READ_ONLY_DEPTH;

    if (_flags & DEPTH_STENCIL_VIEW_FLAG_READ_ONLY_STENCIL)
        result |= D3D12_DSV_FLAG_READ_ONLY_STENCIL;

    return result;
}

inline D3D12_COMPARISON_FUNC GetNativeComparisonFunc(COMPARISON_FUNC _cf)
{
    switch (_cf)
    {
    case buma3d::COMPARISON_FUNC_NEVER         : return D3D12_COMPARISON_FUNC_NEVER;
    case buma3d::COMPARISON_FUNC_LESS          : return D3D12_COMPARISON_FUNC_LESS;
    case buma3d::COMPARISON_FUNC_EQUAL         : return D3D12_COMPARISON_FUNC_EQUAL;
    case buma3d::COMPARISON_FUNC_LESS_EQUAL    : return D3D12_COMPARISON_FUNC_LESS_EQUAL;
    case buma3d::COMPARISON_FUNC_GREATER       : return D3D12_COMPARISON_FUNC_GREATER;
    case buma3d::COMPARISON_FUNC_NOT_EQUAL     : return D3D12_COMPARISON_FUNC_NOT_EQUAL;
    case buma3d::COMPARISON_FUNC_GREATER_EQUAL : return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
    case buma3d::COMPARISON_FUNC_ALWAYS        : return D3D12_COMPARISON_FUNC_ALWAYS;
    default:
        return D3D12_COMPARISON_FUNC(-1);
    }
}

inline D3D12_TEXTURE_ADDRESS_MODE GetNativeAddressMode(const TEXTURE_ADDRESS_MODE _address_mode)
{
    switch (_address_mode)
    {
    case buma3d::TEXTURE_ADDRESS_MODE_WRAP        : return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    case buma3d::TEXTURE_ADDRESS_MODE_MIRROR      : return D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
    case buma3d::TEXTURE_ADDRESS_MODE_CLAMP       : return D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    case buma3d::TEXTURE_ADDRESS_MODE_BORDER      : return D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    case buma3d::TEXTURE_ADDRESS_MODE_MIRROR_ONCE : return D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE;
    default:
        return D3D12_TEXTURE_ADDRESS_MODE(-1);
    }
}

inline D3D12_FILTER_TYPE GetNativeTextureSampleMode(const TEXTURE_SAMPLE_MODE _sample_mode)
{
    switch (_sample_mode)
    {
    case buma3d::TEXTURE_SAMPLE_MODE_POINT  : return D3D12_FILTER_TYPE_POINT;
    case buma3d::TEXTURE_SAMPLE_MODE_LINEAR : return D3D12_FILTER_TYPE_LINEAR;

    //case buma3d::TEXTURE_SAMPLE_MODE_CUBIC_IMG:
    default:
        return D3D12_FILTER_TYPE(-1);
    }
}

inline D3D12_FILTER_REDUCTION_TYPE GetNativeFilterReductionMode(SAMPLER_FILTER_REDUCTION_MODE _reduction_mode)
{
    switch (_reduction_mode)
    {
    case buma3d::SAMPLER_FILTER_REDUCTION_MODE_STANDARD   : return D3D12_FILTER_REDUCTION_TYPE_STANDARD;
    case buma3d::SAMPLER_FILTER_REDUCTION_MODE_COMPARISON : return D3D12_FILTER_REDUCTION_TYPE_COMPARISON;
    case buma3d::SAMPLER_FILTER_REDUCTION_MODE_MIN        : return D3D12_FILTER_REDUCTION_TYPE_MINIMUM;
    case buma3d::SAMPLER_FILTER_REDUCTION_MODE_MAX        : return D3D12_FILTER_REDUCTION_TYPE_MAXIMUM;
    default:
        return D3D12_FILTER_REDUCTION_TYPE(-1);
    }
}

inline D3D12_STATIC_BORDER_COLOR GetNativeStaticBorderColor(BORDER_COLOR _border_color)
{
    switch (_border_color)
    {
    case buma3d::BORDER_COLOR_TRANSPARENT_BLACK_FLOAT : return D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
    case buma3d::BORDER_COLOR_TRANSPARENT_BLACK_INT   : return D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
    case buma3d::BORDER_COLOR_OPAQUE_BLACK_FLOAT      : return D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
    case buma3d::BORDER_COLOR_OPAQUE_BLACK_INT        : return D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
    case buma3d::BORDER_COLOR_OPAQUE_WHITE_FLOAT      : return D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
    case buma3d::BORDER_COLOR_OPAQUE_WHITE_INT        : return D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;

    default:
        return D3D12_STATIC_BORDER_COLOR(-1);
    }
}

inline void GetNativeBorderColor(BORDER_COLOR _border_color, FLOAT _dst_color[4])
{
    switch (_border_color)
    {
    case buma3d::BORDER_COLOR_TRANSPARENT_BLACK_FLOAT:
    case buma3d::BORDER_COLOR_TRANSPARENT_BLACK_INT:
        _dst_color[0] = 0.f;
        _dst_color[1] = 0.f;
        _dst_color[2] = 0.f;
        _dst_color[3] = 0.f;
        break;

    case buma3d::BORDER_COLOR_OPAQUE_BLACK_FLOAT:
    case buma3d::BORDER_COLOR_OPAQUE_BLACK_INT:
        _dst_color[0] = 0.f;
        _dst_color[1] = 0.f;
        _dst_color[2] = 0.f;
        _dst_color[3] = 1.f;
        break;

    case buma3d::BORDER_COLOR_OPAQUE_WHITE_FLOAT:
    case buma3d::BORDER_COLOR_OPAQUE_WHITE_INT  :
        _dst_color[0] = 1.f;
        _dst_color[1] = 1.f;
        _dst_color[2] = 1.f;
        _dst_color[3] = 1.f;
        break;

    default:
        break;
    }
}

inline D3D12_ROOT_PARAMETER_TYPE GetNativeRootParameterTypeForDynamicDescriptor(DESCRIPTOR_TYPE _type)
{
    switch (_type)
    {
    case buma3d::DESCRIPTOR_TYPE_CBV:
    case buma3d::DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
    case buma3d::DESCRIPTOR_TYPE_SRV_TEXTURE:
    case buma3d::DESCRIPTOR_TYPE_SRV_TYPED_BUFFER:
    case buma3d::DESCRIPTOR_TYPE_SRV_BUFFER:
    case buma3d::DESCRIPTOR_TYPE_UAV_TEXTURE:
    case buma3d::DESCRIPTOR_TYPE_UAV_TYPED_BUFFER:
    case buma3d::DESCRIPTOR_TYPE_UAV_BUFFER:
    case buma3d::DESCRIPTOR_TYPE_SRV_ACCELERATION_STRUCTURE:
    case buma3d::DESCRIPTOR_TYPE_SAMPLER:
        return D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;

    case buma3d::DESCRIPTOR_TYPE_CBV_DYNAMIC        : return D3D12_ROOT_PARAMETER_TYPE_CBV;
    case buma3d::DESCRIPTOR_TYPE_SRV_BUFFER_DYNAMIC : return D3D12_ROOT_PARAMETER_TYPE_SRV;
    case buma3d::DESCRIPTOR_TYPE_UAV_BUFFER_DYNAMIC : return D3D12_ROOT_PARAMETER_TYPE_UAV;

    default:
        return D3D12_ROOT_PARAMETER_TYPE(-1);
    }
}

inline D3D12_SHADER_VISIBILITY GetNativeShaderVisibility(SHADER_VISIBILITY _shader_visibility)
{
    switch (_shader_visibility)
    {
    case buma3d::SHADER_VISIBILITY_VERTEX               : return D3D12_SHADER_VISIBILITY_VERTEX;
    case buma3d::SHADER_VISIBILITY_HULL                 : return D3D12_SHADER_VISIBILITY_HULL;
    case buma3d::SHADER_VISIBILITY_DOMAIN               : return D3D12_SHADER_VISIBILITY_DOMAIN;
    case buma3d::SHADER_VISIBILITY_GEOMETRY             : return D3D12_SHADER_VISIBILITY_GEOMETRY;
    case buma3d::SHADER_VISIBILITY_PIXEL                : return D3D12_SHADER_VISIBILITY_PIXEL;
    case buma3d::SHADER_VISIBILITY_MESH                 : return D3D12_SHADER_VISIBILITY_MESH;
    case buma3d::SHADER_VISIBILITY_TASK                 : return D3D12_SHADER_VISIBILITY_AMPLIFICATION;
    case buma3d::SHADER_VISIBILITY_ALL_GRAPHICS_COMPUTE : return D3D12_SHADER_VISIBILITY_ALL;

    default:
        return D3D12_SHADER_VISIBILITY(-1);
    }
}

inline D3D12_SHADER_VISIBILITY GetNativeShaderVisibility(SHADER_STAGE_FLAGS _shader_visibility)
{
    if (_shader_visibility == SHADER_STAGE_FLAG_ALL)
        return D3D12_SHADER_VISIBILITY_ALL;

    switch (_shader_visibility)
    {
    case SHADER_STAGE_FLAG_VERTEX       : return D3D12_SHADER_VISIBILITY_VERTEX;
    case SHADER_STAGE_FLAG_HULL         : return D3D12_SHADER_VISIBILITY_HULL;
    case SHADER_STAGE_FLAG_DOMAIN       : return D3D12_SHADER_VISIBILITY_DOMAIN;
    case SHADER_STAGE_FLAG_GEOMETRY     : return D3D12_SHADER_VISIBILITY_GEOMETRY;
    case SHADER_STAGE_FLAG_PIXEL        : return D3D12_SHADER_VISIBILITY_PIXEL;

    case SHADER_STAGE_FLAG_COMPUTE      : return D3D12_SHADER_VISIBILITY_ALL;

    case SHADER_STAGE_FLAG_TASK         : return D3D12_SHADER_VISIBILITY_AMPLIFICATION;
    case SHADER_STAGE_FLAG_MESH         : return D3D12_SHADER_VISIBILITY_MESH;

    case SHADER_STAGE_FLAG_RAYGEN       : return D3D12_SHADER_VISIBILITY_ALL;
    case SHADER_STAGE_FLAG_ANY_HIT      : return D3D12_SHADER_VISIBILITY_ALL;
    case SHADER_STAGE_FLAG_CLOSEST_HIT  : return D3D12_SHADER_VISIBILITY_ALL;
    case SHADER_STAGE_FLAG_MISS         : return D3D12_SHADER_VISIBILITY_ALL;
    case SHADER_STAGE_FLAG_INTERSECTION : return D3D12_SHADER_VISIBILITY_ALL;
    case SHADER_STAGE_FLAG_CALLABLE     : return D3D12_SHADER_VISIBILITY_ALL;

    default:
        break;
    }

    /*
    NOTE: 複数のシェーダーステージを含む場合、暗黙的にD3D12_SHADER_VISIBILITY_ALLとします。
          Vulkanベースのレイアウトでは、registerでのt,b,s,uの区別はなく、
          異なるVISIBILITYを利用しても、同じbase_shader_registerに異なるリソースタイプを指定できないため、このような動作は有効であると考えます:
          VISIBILITY_VERTEX: register(b0,space0)
          VISIBILITY_PIXEL : register(t0,space0) // だが、既に0は使用されているため無効

          D3D12_SHADER_VISIBILITY_ALLによってフラグで指定されているステージ以外に対しても可視になりますが、
          DESCRIPTOR_SET_LAYOUT_DESC::bindingsそれぞれが指定するvisibilityのステージについて、全ての要素で指定されないステージはDENY_*_ROOT_ACCESSによって無効化することが可能です。
          例えば、bindings[0]がVSとPSステージに対して可視の場合 HS, DS, GS, AS, MS のステージのDENY_*_ROOT_ACCESSフラグを指定可能です。
          bindings[1]にて DS, GS が可視の場合bindings[0]で可視だったDS, GSはマスクされ、結果として HS, AS, MS のステージのDENY_*_ROOT_ACCESSフラグを指定可能になります。

          パイプラインレイアウトに追加される全てのbindingで上記のDENY_*_ROOT_ACCESSマスクの条件を満たす必要がります。
          以下の場合、SHADER_STAGE_ALLによってすべてのステージが指定されたため DENY_*_ROOT_ACCESS を設定することは出来ません:
          レイアウト             : 可能な DENY_*_ROOT_ACCESS フラグ
          L0 SHADER_STAGE_VERTEX : --, HS, DS, GS, PS, AS, MS
          L1 SHADER_STAGE_PIXEL  : VS, HS, DS, GS, --, AS, MS
          L2 SHADER_STAGE_ALL    : --, --, --, --, --, --, --
    */
    if (_shader_visibility & ( SHADER_STAGE_FLAG_VERTEX
                             | SHADER_STAGE_FLAG_HULL
                             | SHADER_STAGE_FLAG_DOMAIN
                             | SHADER_STAGE_FLAG_GEOMETRY
                             | SHADER_STAGE_FLAG_PIXEL

                             | SHADER_STAGE_FLAG_COMPUTE

                             | SHADER_STAGE_FLAG_TASK
                             | SHADER_STAGE_FLAG_MESH

                             | SHADER_STAGE_FLAG_RAYGEN
                             | SHADER_STAGE_FLAG_ANY_HIT
                             | SHADER_STAGE_FLAG_CLOSEST_HIT
                             | SHADER_STAGE_FLAG_MISS
                             | SHADER_STAGE_FLAG_INTERSECTION
                             | SHADER_STAGE_FLAG_CALLABLE
                              ))
    {
        return D3D12_SHADER_VISIBILITY_ALL;
    }

    return D3D12_SHADER_VISIBILITY(-1);
}

// TODO: GetNativeDescriptorFlags: Vulkanとの互換を確認。
inline D3D12_ROOT_DESCRIPTOR_FLAGS GetNativeDescriptorFlags(ROOT_PARAMETER_TYPE _type, DESCRIPTOR_FLAGS _flags)
{
    D3D12_ROOT_DESCRIPTOR_FLAGS result = D3D12_ROOT_DESCRIPTOR_FLAG_NONE;

    B3D_UNREFERENCED(_type, _flags);

    //if (_flags & DESCRIPTOR_FLAG_DESCRIPTORS_UPDATE_AFTER_BIND)
    //    result |= D3D12_ROOT_DESCRIPTOR_FLAG_;

    //if (_flags & DESCRIPTOR_FLAG_DESCRIPTORS_UPDATE_UNUSED_WHILE_PENDING)
    //    result |= D3D12_ROOT_DESCRIPTOR_FLAG_;

    //if (_flags & DESCRIPTOR_FLAG_PARTIALLY_BOUND)
    //    result |= D3D12_ROOT_DESCRIPTOR_FLAG_;

    //if (_flags & DESCRIPTOR_FLAG_VARIABLE_DESCRIPTOR_COUNT)
    //    result |= D3D12_ROOT_DESCRIPTOR_FLAG_;


    // _DATAフラグはVulkanに存在しませんが、フラグ外で同等の指定を行う機能が存在するかどうかを調査します。

    //if (_flags & DESCRIPTOR_FLAG_DATA_VOLATILE)
    //    result |= D3D12_ROOT_DESCRIPTOR_FLAG_;

    //if (_flags & DESCRIPTOR_FLAG_DATA_STATIC_WHILE_SET_AT_EXECUTE)
    //    result |= D3D12_ROOT_DESCRIPTOR_FLAG_;

    //if (_flags & DESCRIPTOR_FLAG_DATA_STATIC)
    //    result |= D3D12_ROOT_DESCRIPTOR_FLAG_;

    //if (_flags & DESCRIPTOR_FLAG_DESCRIPTORS_STATIC_KEEPING_BUFFER_BOUNDS_CHECKS)
    //    result |= D3D12_ROOT_DESCRIPTOR_FLAG_;

    return result;
}

inline D3D12_ROOT_DESCRIPTOR_FLAGS GetNativeDescriptorFlags(DESCRIPTOR_TYPE _type, DESCRIPTOR_FLAGS _flags)
{
    D3D12_ROOT_DESCRIPTOR_FLAGS result = D3D12_ROOT_DESCRIPTOR_FLAG_NONE;
    B3D_UNREFERENCED(_type);

    if (_flags & DESCRIPTOR_FLAG_DATA_VOLATILE)                           result |= D3D12_ROOT_DESCRIPTOR_FLAG_DATA_VOLATILE;
    if (_flags & DESCRIPTOR_FLAG_DATA_STATIC_WHILE_SET_AT_EXECUTE)        result |= D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC_WHILE_SET_AT_EXECUTE;
    if (_flags & DESCRIPTOR_FLAG_DATA_STATIC)                             result |= D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC;

    return result;
}

// FIXME: GetNativeDescriptorRangeFlags
inline D3D12_DESCRIPTOR_RANGE_FLAGS GetNativeDescriptorRangeFlags(DESCRIPTOR_TYPE _type, DESCRIPTOR_FLAGS _flags)
{
    D3D12_DESCRIPTOR_RANGE_FLAGS result = D3D12_DESCRIPTOR_RANGE_FLAG_NONE;

    if (_flags == buma3d::DESCRIPTOR_FLAG_DESCRIPTORS_UPDATE_AFTER_BIND)
    {
        result |= D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE;

        // D3D12_DESCRIPTOR_RANGE_FLAG_NONEのデフォルト値を再現します。
        if (_type == buma3d::DESCRIPTOR_TYPE_UAV_TEXTURE ||
            _type == buma3d::DESCRIPTOR_TYPE_UAV_BUFFER ||
            _type == buma3d::DESCRIPTOR_TYPE_UAV_TYPED_BUFFER)
        {
            result |= D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE;
        }
        else
        {
            result |= D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC_WHILE_SET_AT_EXECUTE;
        }
    }
    else
    {
        if (_flags & buma3d::DESCRIPTOR_FLAG_DESCRIPTORS_UPDATE_AFTER_BIND)           result |= D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE;
        if (_flags & buma3d::DESCRIPTOR_FLAG_DESCRIPTORS_UPDATE_UNUSED_WHILE_PENDING) result |= D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC_WHILE_SET_AT_EXECUTE;
        if (_flags & buma3d::DESCRIPTOR_FLAG_DATA_VOLATILE)                           result |= D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE;
        if (_flags & buma3d::DESCRIPTOR_FLAG_DATA_STATIC_WHILE_SET_AT_EXECUTE)        result |= D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC_WHILE_SET_AT_EXECUTE;
        if (_flags & buma3d::DESCRIPTOR_FLAG_DATA_STATIC)                             result |= D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC;
    }

    return result;
}

inline D3D12_DESCRIPTOR_RANGE_TYPE GetNativeDescriptorRangeType(DESCRIPTOR_TYPE _type)
{
    switch (_type)
    {
    case buma3d::DESCRIPTOR_TYPE_CBV                        : return D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
    case buma3d::DESCRIPTOR_TYPE_INPUT_ATTACHMENT           : return D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    case buma3d::DESCRIPTOR_TYPE_SRV_TEXTURE                : return D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    case buma3d::DESCRIPTOR_TYPE_SRV_BUFFER                 : return D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    case buma3d::DESCRIPTOR_TYPE_SRV_TYPED_BUFFER           : return D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    case buma3d::DESCRIPTOR_TYPE_UAV_TEXTURE                : return D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
    case buma3d::DESCRIPTOR_TYPE_UAV_BUFFER                 : return D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
    case buma3d::DESCRIPTOR_TYPE_UAV_TYPED_BUFFER           : return D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
    case buma3d::DESCRIPTOR_TYPE_SRV_ACCELERATION_STRUCTURE : return D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    case buma3d::DESCRIPTOR_TYPE_SAMPLER                    : return D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;

    default:
        return D3D12_DESCRIPTOR_RANGE_TYPE(-1);
    }
}

inline D3D12_ROOT_SIGNATURE_FLAGS CalcDenyRootAccessPossibility(SHADER_STAGE_FLAGS _accumulated_visibility_flags)
{
    if (_accumulated_visibility_flags == SHADER_STAGE_FLAG_ALL)
        return D3D12_ROOT_SIGNATURE_FLAG_NONE;

    static constexpr D3D12_ROOT_SIGNATURE_FLAGS DENY_ALL_GRAPHICS =
          D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS
        | D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS
        | D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS
        | D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS
        | D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS
        | D3D12_ROOT_SIGNATURE_FLAG_DENY_AMPLIFICATION_SHADER_ROOT_ACCESS
        | D3D12_ROOT_SIGNATURE_FLAG_DENY_MESH_SHADER_ROOT_ACCESS;

    auto&& f = _accumulated_visibility_flags;

    // レイアウト全体を通して、computeまたはレイトレーシングパイプラインに対してのみ可視である場合、グラフィックスパイプラインステージを除外します。
    if ((f & SHADER_STAGE_FLAG_ALL_GRAPGICS) == SHADER_STAGE_FLAG_NONE &&
        (f & (  SHADER_STAGE_FLAG_COMPUTE
              | SHADER_STAGE_FLAG_RAYGEN
              | SHADER_STAGE_FLAG_ANY_HIT
              | SHADER_STAGE_FLAG_CLOSEST_HIT
              | SHADER_STAGE_FLAG_MISS
              | SHADER_STAGE_FLAG_INTERSECTION
              | SHADER_STAGE_FLAG_CALLABLE
              ))
        )
    {
        return DENY_ALL_GRAPHICS;
    }

    // それ以外の場合、除外可能なステージを1対1で見つけます。
    D3D12_ROOT_SIGNATURE_FLAGS result = D3D12_ROOT_SIGNATURE_FLAG_NONE;
    if (!(f & buma3d::SHADER_STAGE_FLAG_VERTEX)   ) result |= D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS;
    if (!(f & buma3d::SHADER_STAGE_FLAG_HULL)     ) result |= D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;
    if (!(f & buma3d::SHADER_STAGE_FLAG_DOMAIN)   ) result |= D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;
    if (!(f & buma3d::SHADER_STAGE_FLAG_GEOMETRY) ) result |= D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
    if (!(f & buma3d::SHADER_STAGE_FLAG_PIXEL)    ) result |= D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;
    if (!(f & buma3d::SHADER_STAGE_FLAG_TASK)     ) result |= D3D12_ROOT_SIGNATURE_FLAG_DENY_AMPLIFICATION_SHADER_ROOT_ACCESS;
    if (!(f & buma3d::SHADER_STAGE_FLAG_MESH)     ) result |= D3D12_ROOT_SIGNATURE_FLAG_DENY_MESH_SHADER_ROOT_ACCESS;

    return result;
}

inline D3D12_ROOT_SIGNATURE_FLAGS GetNativeRootSignatureFlags(ROOT_SIGNATURE_FLAGS _flags)
{
    D3D12_ROOT_SIGNATURE_FLAGS result =
          D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
        | D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS
        | D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS
        | D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS
        | D3D12_ROOT_SIGNATURE_FLAG_DENY_MESH_SHADER_ROOT_ACCESS
        | D3D12_ROOT_SIGNATURE_FLAG_DENY_AMPLIFICATION_SHADER_ROOT_ACCESS;

    if (_flags & buma3d::ROOT_SIGNATURE_FLAG_DENY_INPUT_ASSEMBLER_INPUT_LAYOUT) result &= ~D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
    if (_flags & buma3d::ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS   ) result |=  D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS;
    if (_flags & buma3d::ROOT_SIGNATURE_FLAG_ALLOW_HULL_SHADER_ROOT_ACCESS    ) result &= ~D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;
    if (_flags & buma3d::ROOT_SIGNATURE_FLAG_ALLOW_DOMAIN_SHADER_ROOT_ACCESS  ) result &= ~D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;
    if (_flags & buma3d::ROOT_SIGNATURE_FLAG_ALLOW_GEOMETRY_SHADER_ROOT_ACCESS) result &= ~D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
    if (_flags & buma3d::ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS    ) result |=  D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;
    if (_flags & buma3d::ROOT_SIGNATURE_FLAG_ALLOW_STREAM_OUTPUT              ) result |=  D3D12_ROOT_SIGNATURE_FLAG_ALLOW_STREAM_OUTPUT;
    if (_flags & buma3d::ROOT_SIGNATURE_FLAG_ALLOW_MESH_SHADER_ROOT_ACCESS    ) result &= ~D3D12_ROOT_SIGNATURE_FLAG_DENY_MESH_SHADER_ROOT_ACCESS;
    if (_flags & buma3d::ROOT_SIGNATURE_FLAG_ALLOW_TASK_SHADER_ROOT_ACCESS    ) result &= ~D3D12_ROOT_SIGNATURE_FLAG_DENY_AMPLIFICATION_SHADER_ROOT_ACCESS;
    if (_flags & buma3d::ROOT_SIGNATURE_FLAG_RAY_TRACING_SHADER_VISIBILITY    ) result |=  D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE;

    return result;
}

inline D3D12_ROOT_SIGNATURE_FLAGS GetNativePipelineLayoutFlags(PIPELINE_LAYOUT_FLAGS _flags)
{
    D3D12_ROOT_SIGNATURE_FLAGS result = D3D12_ROOT_SIGNATURE_FLAG_NONE;

    if (_flags & PIPELINE_LAYOUT_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT)   result |=  D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
    if (_flags & PIPELINE_LAYOUT_FLAG_ALLOW_STREAM_OUTPUT)                  result |=  D3D12_ROOT_SIGNATURE_FLAG_ALLOW_STREAM_OUTPUT;

    return result;
}

inline D3D12_RESOURCE_STATES GetNativeResourceState(RESOURCE_STATE _state)
{
    switch (_state)
    {
    case buma3d::RESOURCE_STATE_UNDEFINED                              : return D3D12_RESOURCE_STATE_COMMON;
    case buma3d::RESOURCE_STATE_INDIRECT_ARGUMENT_READ                 : return D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT;
    case buma3d::RESOURCE_STATE_CONDITIONAL_RENDERING_READ             : return D3D12_RESOURCE_STATE_PREDICATION;
    case buma3d::RESOURCE_STATE_INDEX_READ                             : return D3D12_RESOURCE_STATE_INDEX_BUFFER;
    case buma3d::RESOURCE_STATE_VERTEX_ATTRIBUTE_READ                  : return D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
    case buma3d::RESOURCE_STATE_CONSTANT_READ                          : return D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
    case buma3d::RESOURCE_STATE_SHADER_READ                            : return D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
    case buma3d::RESOURCE_STATE_SHADER_WRITE                           : return D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
    case buma3d::RESOURCE_STATE_SHADER_READ_WRITE                      : return D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
    case buma3d::RESOURCE_STATE_INPUT_ATTACHMENT_READ                  : return D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    case buma3d::RESOURCE_STATE_COLOR_ATTACHMENT_READ                  : return D3D12_RESOURCE_STATE_RENDER_TARGET;
    case buma3d::RESOURCE_STATE_COLOR_ATTACHMENT_WRITE                 : return D3D12_RESOURCE_STATE_RENDER_TARGET;
    case buma3d::RESOURCE_STATE_COLOR_ATTACHMENT_READ_WRITE            : return D3D12_RESOURCE_STATE_RENDER_TARGET;
    case buma3d::RESOURCE_STATE_DEPTH_STENCIL_ATTACHMENT_READ          : return D3D12_RESOURCE_STATE_DEPTH_READ;
    case buma3d::RESOURCE_STATE_DEPTH_STENCIL_ATTACHMENT_WRITE         : return D3D12_RESOURCE_STATE_DEPTH_WRITE;
    case buma3d::RESOURCE_STATE_DEPTH_STENCIL_ATTACHMENT_READ_WRITE    : return D3D12_RESOURCE_STATE_DEPTH_WRITE;
    case buma3d::RESOURCE_STATE_RESOLVE_COLOR_ATTACHMENT_WRITE         : return D3D12_RESOURCE_STATE_RESOLVE_DEST;
    case buma3d::RESOURCE_STATE_RESOLVE_DEPTH_STENCIL_ATTACHMENT_WRITE : return D3D12_RESOURCE_STATE_RESOLVE_DEST;
    case buma3d::RESOURCE_STATE_COPY_SRC_READ                          : return D3D12_RESOURCE_STATE_COPY_SOURCE;
    case buma3d::RESOURCE_STATE_COPY_DST_WRITE                         : return D3D12_RESOURCE_STATE_COPY_DEST;
    case buma3d::RESOURCE_STATE_RESOLVE_SRC_READ                       : return D3D12_RESOURCE_STATE_RESOLVE_SOURCE;
    case buma3d::RESOURCE_STATE_RESOLVE_DST_WRITE                      : return D3D12_RESOURCE_STATE_RESOLVE_DEST;
    case buma3d::RESOURCE_STATE_HOST_READ                              : return D3D12_RESOURCE_STATE_GENERIC_READ /*| COMMON           */;
    case buma3d::RESOURCE_STATE_HOST_WRITE                             : return D3D12_RESOURCE_STATE_GENERIC_READ /*| COMMON, COPY_DST */;
    case buma3d::RESOURCE_STATE_HOST_READ_WRITE                        : return D3D12_RESOURCE_STATE_GENERIC_READ /*| COMMON, COPY_DST */;
    case buma3d::RESOURCE_STATE_GENERIC_MEMORY_READ                    : return D3D12_RESOURCE_STATE_GENERIC_READ;
    case buma3d::RESOURCE_STATE_STREAM_OUTPUT_WRITE                    : return D3D12_RESOURCE_STATE_STREAM_OUT;
    case buma3d::RESOURCE_STATE_STREAM_OUTPUT_COUNTER_READ             : return D3D12_RESOURCE_STATE_STREAM_OUT;
    case buma3d::RESOURCE_STATE_STREAM_OUTPUT_COUNTER_WRITE            : return D3D12_RESOURCE_STATE_STREAM_OUT;
    case buma3d::RESOURCE_STATE_STREAM_OUTPUT_COUNTER_READ_WRITE       : return D3D12_RESOURCE_STATE_STREAM_OUT;
    case buma3d::RESOURCE_STATE_ACCELERATION_STRUCTURE_READ            : return D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;
    case buma3d::RESOURCE_STATE_ACCELERATION_STRUCTURE_WRITE           : return D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;
    case buma3d::RESOURCE_STATE_ACCELERATION_STRUCTURE_READ_WRITE      : return D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;
    case buma3d::RESOURCE_STATE_SHADING_RATE_ATTACHMENT_READ           : return D3D12_RESOURCE_STATE_SHADING_RATE_SOURCE;
    case buma3d::RESOURCE_STATE_PRESENT                                : return D3D12_RESOURCE_STATE_PRESENT;

    default:
        return D3D12_RESOURCE_STATES(-1);
    }
}

inline D3D12_RESOURCE_STATES GetNativeResourceAccessFlags(RESOURCE_ACCESS_FLAGS _flags)
{
    D3D12_RESOURCE_STATES result = D3D12_RESOURCE_STATE_COMMON;

    if (_flags & RESOURCE_ACCESS_FLAG_NONE                              ) result |= D3D12_RESOURCE_STATE_COMMON;
    if (_flags & RESOURCE_ACCESS_FLAG_INDIRECT_ARGUMENT_READ            ) result |= D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT;
    if (_flags & RESOURCE_ACCESS_FLAG_CONDITIONAL_RENDERING_READ        ) result |= D3D12_RESOURCE_STATE_PREDICATION;
    if (_flags & RESOURCE_ACCESS_FLAG_INDEX_READ                        ) result |= D3D12_RESOURCE_STATE_INDEX_BUFFER;
    if (_flags & RESOURCE_ACCESS_FLAG_VERTEX_ATTRIBUTE_READ             ) result |= D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
    if (_flags & RESOURCE_ACCESS_FLAG_CONSTANT_READ                     ) result |= D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
    if (_flags & RESOURCE_ACCESS_FLAG_SHADER_READ                       ) result |= D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    if (_flags & RESOURCE_ACCESS_FLAG_SHADER_WRITE                      ) result |= D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
    if (_flags & RESOURCE_ACCESS_FLAG_INPUT_ATTACHMENT_READ             ) result |= D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    if (_flags & RESOURCE_ACCESS_FLAG_COLOR_ATTACHMENT_READ             ) result |= D3D12_RESOURCE_STATE_RENDER_TARGET;
    if (_flags & RESOURCE_ACCESS_FLAG_COLOR_ATTACHMENT_WRITE            ) result |= D3D12_RESOURCE_STATE_RENDER_TARGET;
    if (_flags & RESOURCE_ACCESS_FLAG_DEPTH_STENCIL_ATTACHMENT_READ     ) result |= D3D12_RESOURCE_STATE_DEPTH_READ;
    if (_flags & RESOURCE_ACCESS_FLAG_DEPTH_STENCIL_ATTACHMENT_WRITE    ) result |= D3D12_RESOURCE_STATE_DEPTH_WRITE;
    if (_flags & RESOURCE_ACCESS_FLAG_COPY_READ                         ) result |= D3D12_RESOURCE_STATE_COPY_SOURCE;
    if (_flags & RESOURCE_ACCESS_FLAG_COPY_WRITE                        ) result |= D3D12_RESOURCE_STATE_COPY_DEST;
    if (_flags & RESOURCE_ACCESS_FLAG_RESOLVE_READ                      ) result |= D3D12_RESOURCE_STATE_RESOLVE_SOURCE;
    if (_flags & RESOURCE_ACCESS_FLAG_RESOLVE_WRITE                     ) result |= D3D12_RESOURCE_STATE_RESOLVE_DEST;
    if (_flags & RESOURCE_ACCESS_FLAG_HOST_READ                         ) result |= D3D12_RESOURCE_STATE_GENERIC_READ | D3D12_RESOURCE_STATE_COMMON;
    if (_flags & RESOURCE_ACCESS_FLAG_HOST_WRITE                        ) result |= D3D12_RESOURCE_STATE_GENERIC_READ | D3D12_RESOURCE_STATE_COMMON /*| D3D12_RESOURCE_STATE_COPY_DEST*/;
    if (_flags & RESOURCE_ACCESS_FLAG_GENERIC_MEMORY_READ               ) result |= D3D12_RESOURCE_STATE_GENERIC_READ;
    if (_flags & RESOURCE_ACCESS_FLAG_STREAM_OUTPUT_WRITE               ) result |= D3D12_RESOURCE_STATE_STREAM_OUT;
    if (_flags & RESOURCE_ACCESS_FLAG_STREAM_OUTPUT_COUNTER_READ        ) result |= D3D12_RESOURCE_STATE_STREAM_OUT;
    if (_flags & RESOURCE_ACCESS_FLAG_STREAM_OUTPUT_COUNTER_WRITE       ) result |= D3D12_RESOURCE_STATE_STREAM_OUT;
    if (_flags & RESOURCE_ACCESS_FLAG_ACCELERATION_STRUCTURE_READ       ) result |= D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;
    if (_flags & RESOURCE_ACCESS_FLAG_ACCELERATION_STRUCTURE_WRITE      ) result |= D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;
    if (_flags & RESOURCE_ACCESS_FLAG_SHADING_RATE_ATTACHMENT_READ      ) result |= D3D12_RESOURCE_STATE_SHADING_RATE_SOURCE;
    if (_flags & RESOURCE_ACCESS_FLAG_PRESENT                           ) result |= D3D12_RESOURCE_STATE_PRESENT;

    return result;
}

inline void ConvertNativeScissorRect(const SCISSOR_RECT& _rect, D3D12_RECT* _dst)
{
    _dst->left   = _rect.offset.x;
    _dst->top    = _rect.offset.y;
    _dst->right  = _rect.offset.x + SCAST<int32_t>(_rect.extent.width );
    _dst->bottom = _rect.offset.y + SCAST<int32_t>(_rect.extent.height);
}

inline void ConvertNativeScissorRect(const OFFSET2D& _offset, const EXTENT2D& _extent, D3D12_RECT* _dst)
{
    _dst->left   = _offset.x;
    _dst->top    = _offset.y;
    _dst->right  = _offset.x + SCAST<int32_t>(_extent.width);
    _dst->bottom = _offset.y + SCAST<int32_t>(_extent.height);
}

inline void ConvertNativeViewport(const VIEWPORT& _viewport, D3D12_VIEWPORT* _dst)
{
    _dst->TopLeftX = _viewport.x;
    _dst->TopLeftY = _viewport.y;
    _dst->Width    = _viewport.width;
    _dst->Height   = _viewport.height;
    _dst->MinDepth = _viewport.min_depth;
    _dst->MaxDepth = _viewport.max_depth;
}

inline D3D12_QUERY_HEAP_TYPE GetNativeQueryHeapType(QUERY_HEAP_TYPE _type)
{
    switch (_type)
    {
    case buma3d::QUERY_HEAP_TYPE_OCCLUSION                  : return D3D12_QUERY_HEAP_TYPE_OCCLUSION;
    case buma3d::QUERY_HEAP_TYPE_TIMESTAMP                  : return D3D12_QUERY_HEAP_TYPE_TIMESTAMP;
    case buma3d::QUERY_HEAP_TYPE_PIPELINE_STATISTICS        : return D3D12_QUERY_HEAP_TYPE_PIPELINE_STATISTICS;
    case buma3d::QUERY_HEAP_TYPE_SO_STATISTICS              : return D3D12_QUERY_HEAP_TYPE_SO_STATISTICS;
    case buma3d::QUERY_HEAP_TYPE_VIDEO_DECODE_STATISTICS    : return D3D12_QUERY_HEAP_TYPE_VIDEO_DECODE_STATISTICS;

    // NOTE: D3D12では加速構造の情報をクエリを介して取得しません(ID3D12GraphicsCommandList5::EmitRaytracingAccelerationStructurePostbuildInfoが使用されます)。そのため以下のクエリタイプは存在しません。
    // だたし、実際には動作を低オーバーヘッドを保ちながらクエリとしてエミュレーションが可能です。 これにより、Vulkanとの互換性を持つことができます。
    case buma3d::QUERY_HEAP_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE:
    case buma3d::QUERY_HEAP_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_SIZE:
    default:
        return D3D12_QUERY_HEAP_TYPE(-1);
    }
}

inline D3D12_QUERY_TYPE GetNativeQueryType(QUERY_HEAP_TYPE _type, const QUERY_DESC& _desc)
{
    switch (_type)
    {
    case buma3d::QUERY_HEAP_TYPE_OCCLUSION                                 : return (_desc.flags & QUERY_FLAG_PRECISE_OCCLUSION) ? D3D12_QUERY_TYPE_OCCLUSION : D3D12_QUERY_TYPE_BINARY_OCCLUSION;
    case buma3d::QUERY_HEAP_TYPE_TIMESTAMP                                 : return D3D12_QUERY_TYPE_TIMESTAMP;
    case buma3d::QUERY_HEAP_TYPE_PIPELINE_STATISTICS                       : return D3D12_QUERY_TYPE_PIPELINE_STATISTICS;
    case buma3d::QUERY_HEAP_TYPE_SO_STATISTICS                             : return D3D12_QUERY_TYPE(D3D12_QUERY_TYPE_SO_STATISTICS_STREAM0 + _desc.so_statistics_stream_index);
    case buma3d::QUERY_HEAP_TYPE_VIDEO_DECODE_STATISTICS                   : return D3D12_QUERY_TYPE_VIDEO_DECODE_STATISTICS;

    case buma3d::QUERY_HEAP_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE:
    case buma3d::QUERY_HEAP_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_SIZE:
    default:
        return D3D12_QUERY_TYPE(-1);
    }
}

inline D3D12_SAMPLE_POSITION* ConvertNativeSamplePosition(const buma3d::SAMPLE_POSITION& _src, D3D12_SAMPLE_POSITION* _dst)
{
    // D3D12の仕様に従い、サンプル位置の値の範囲は[-8, 7]です
    _dst->X = SCAST<INT8>((_src.x * 15.f) - 8.f);
    _dst->Y = SCAST<INT8>((_src.y * 15.f) - 8.f);
    return _dst;
}

inline D3D12_SHADING_RATE GetNativeShadingRate(const EXTENT2D& _rate)
{
    // D3D12_SHADING_RATE 列挙値にマップします: 4 2 1 -> 2 1 0
    return SCAST<D3D12_SHADING_RATE>(D3D12_MAKE_COARSE_SHADING_RATE(_rate.width >> 1, _rate.height >> 1));
}

inline D3D12_SHADING_RATE_COMBINER GetNativeShadingRateCombinerOp(SHADING_RATE_COMBINER_OP _op)
{
    switch (_op)
    {
    case buma3d::SHADING_RATE_COMBINER_OP_KEEP    : return D3D12_SHADING_RATE_COMBINER_PASSTHROUGH;
    case buma3d::SHADING_RATE_COMBINER_OP_REPLACE : return D3D12_SHADING_RATE_COMBINER_OVERRIDE;
    case buma3d::SHADING_RATE_COMBINER_OP_MIN     : return D3D12_SHADING_RATE_COMBINER_MIN;
    case buma3d::SHADING_RATE_COMBINER_OP_MAX     : return D3D12_SHADING_RATE_COMBINER_MAX;
    case buma3d::SHADING_RATE_COMBINER_OP_MUL     : return D3D12_SHADING_RATE_COMBINER_SUM;

    default:
        return D3D12_SHADING_RATE_COMBINER(-1);
    }
}


template <typename T>
inline void CalcDescriptorCounts(uint32_t _num_sizes, const T* _sizes, uint32_t* _dst_descriptor_count, uint32_t* _dst_sampler_descriptor_count)
{
    for (uint32_t i = 0; i < _num_sizes; i++)
    {
        auto&& ps = _sizes[i];
        switch (ps.type)
        {
        case buma3d::DESCRIPTOR_TYPE_CBV:
        case buma3d::DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
        case buma3d::DESCRIPTOR_TYPE_SRV_TEXTURE:
        case buma3d::DESCRIPTOR_TYPE_SRV_TYPED_BUFFER:
        case buma3d::DESCRIPTOR_TYPE_SRV_BUFFER:
        case buma3d::DESCRIPTOR_TYPE_SRV_ACCELERATION_STRUCTURE:
        case buma3d::DESCRIPTOR_TYPE_UAV_TEXTURE:
        case buma3d::DESCRIPTOR_TYPE_UAV_TYPED_BUFFER:
        case buma3d::DESCRIPTOR_TYPE_UAV_BUFFER:
            *_dst_descriptor_count += ps.num_descriptors;
            break;

        case buma3d::DESCRIPTOR_TYPE_SAMPLER:
            *_dst_sampler_descriptor_count += ps.num_descriptors;
            break;

        default:
            break;
        }
    }
}

template <typename T>
inline bool HasSameDescriptorType(uint32_t _num_sizes, const T* _sizes)
{
    util::Set<DESCRIPTOR_TYPE> types;
    for (uint32_t i = 0; i < _num_sizes; i++)
    {
        if (types.find(_sizes[i].type) != types.end())
            return true;
        types.insert(_sizes[i].type);
    }

    return false;
}


}// namespace util
}// namespace buma3d

namespace buma3d
{
namespace util
{

struct FEATURE_DATA
{
    D3D12_FEATURE_DATA_D3D12_OPTIONS                        d3d12_options;
    D3D12_FEATURE_DATA_D3D12_OPTIONS1                       d3d12_options1;
    D3D12_FEATURE_DATA_D3D12_OPTIONS2                       d3d12_options2;
    D3D12_FEATURE_DATA_D3D12_OPTIONS3                       d3d12_options3;
    D3D12_FEATURE_DATA_D3D12_OPTIONS4                       d3d12_options4;
    D3D12_FEATURE_DATA_D3D12_OPTIONS5                       d3d12_options5;
    D3D12_FEATURE_DATA_D3D12_OPTIONS6                       d3d12_options6;
    D3D12_FEATURE_DATA_D3D12_OPTIONS7                       d3d12_options7;
    D3D12_FEATURE_DATA_D3D12_OPTIONS8                       d3d12_options8;
    D3D12_FEATURE_DATA_D3D12_OPTIONS9                       d3d12_options9;
    D3D12_FEATURE_DATA_D3D12_OPTIONS10                      d3d12_options10;
    D3D12_FEATURE_DATA_D3D12_OPTIONS11                      d3d12_options11;

    D3D12_FEATURE_DATA_ARCHITECTURE                         architecture;
    D3D12_FEATURE_DATA_ARCHITECTURE1                        architecture1;
    D3D12_FEATURE_DATA_SERIALIZATION                        serialization;
    D3D12_FEATURE_DATA_PROTECTED_RESOURCE_SESSION_SUPPORT   protected_resource_session_support;
    //D3D12_FEATURE_DATA_PROTECTED_RESOURCE_SESSION_TYPE_COUNT  protected_resource_session_type_count;
    //D3D12_FEATURE_DATA_PROTECTED_RESOURCE_SESSION_TYPES       protected_resource_session_types;

    D3D12_FEATURE_DATA_FEATURE_LEVELS                       feature_levels;
    D3D12_FEATURE_DATA_GPU_VIRTUAL_ADDRESS_SUPPORT          gpu_virtual_address_support;
    D3D12_FEATURE_DATA_SHADER_MODEL                         shader_model;
    D3D12_FEATURE_DATA_ROOT_SIGNATURE                       root_signature;
    D3D12_FEATURE_DATA_SHADER_CACHE                         shader_cache;
    D3D12_FEATURE_DATA_EXISTING_HEAPS                       existing_heaps;
    D3D12_FEATURE_DATA_CROSS_NODE                           cross_node;
    //D3D12_FEATURE_DATA_COMMAND_QUEUE_PRIORITY             command_queue_priority;// 必要に応じて取得

    util::StArray<FORMAT_FEATURE_DATA, DXGI_FORMAT_B4G4R4A4_UNORM + 1> formats_data;
    //D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS   multisample_quality_levels;// 必要に応じて取得
};

}// namespace util
}// namespace buma3d

namespace buma3d
{
namespace util
{

template<typename T>
using ComPtr = Microsoft::WRL::ComPtr<T>;


BMRESULT GetBMResultFromHR(HRESULT _hr);


inline void GetHRESULTMessage(HRESULT _hr, util::String* _dst)
{
    LPSTR msg{};
    FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_ALLOCATE_BUFFER
                   , NULL, _hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), reinterpret_cast<LPSTR>(&msg), 0, NULL);
    *_dst = msg;
    auto pos = _dst->rfind("\r\n");
    if (pos != util::String::npos)
        _dst->erase(pos, _dst->size());
    LocalFree(msg);
}

inline void GetHRESULTMessage(HRESULT _hr, util::WString* _dst)
{
    LPWSTR msg{};
    FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_ALLOCATE_BUFFER
                   , NULL, _hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), reinterpret_cast<LPWSTR>(&msg), 0, NULL);
    *_dst = msg;
    auto pos = _dst->rfind(L"\r\n");
    if (pos != util::String::npos)
        _dst->erase(pos, _dst->size());
    LocalFree(msg);
}

class com_exception : public std::exception
{
public:
    com_exception(HRESULT _hr) : result(_hr), s_str{}
    {
        GetHRESULTMessage(result, &msg_str);
        sprintf_s(s_str, "Failure with HRESULT of %08X\nMessage: ", static_cast<unsigned int>(result));
        msg_str = s_str + msg_str;
    }
    ~com_exception() {}

    virtual const char* what() const override
    {
        return msg_str.c_str();
    }

private:
    HRESULT result;
    char s_str[64];
    util::String msg_str;
};

inline void ThrowIfFailed(HRESULT _hr)
{
    if (FAILED(_hr))
    {
        throw com_exception(_hr);
    }
}

inline BMRESULT CheckHRESULT(HRESULT _hr)
{
    if (FAILED(_hr))
    {
    #if _DEBUG
        util::String str;
        GetHRESULTMessage(_hr, &str);
        hlp::OutDebugStr(str);
    #endif
        return BMRESULT_FAILED;
    }

    return BMRESULT_SUCCEED;
}


inline BMRESULT CheckHRESULT(HRESULT _hr, const char* _file, uint32_t _line)
{
    if (FAILED(_hr))
    {
    #if _DEBUG
        util::String str;
        GetHRESULTMessage(_hr, &str);
        str += ", file: " + util::String(_file);
        str += ", line: " + hlp::to_string(_line) + "\n";
        hlp::OutDebugStr(str);
    #endif
        return GetBMResultFromHR(_hr);
    }

    return BMRESULT_SUCCEED;
}

inline HRESULT SetName(ID3D12Object* _obj, const char* _str)
{
    return _obj->SetName(_str ? hlp::to_wstring(_str).c_str() : nullptr);
}

inline HRESULT SetName(ID3D12Object* _obj, const wchar_t* _str)
{
    return _obj->SetName(_str);
}

inline HRESULT SetName(ID3D12Object* _obj, const util::String& _str)
{
    return _obj->SetName(_str.empty() ? nullptr : hlp::to_wstring(_str).c_str());
}

inline HRESULT SetName(ID3D12Object* _obj, const util::WString& _str)
{
    return _obj->SetName(_str.c_str());
}


}// namespace util
}// namespace buma3d

namespace buma3d
{
namespace util
{

template<typename T>
inline constexpr bool IsEnabledDebug(T* _b3d_obj)
{
    if constexpr (std::is_base_of_v<IDeviceChild, T>)
    {
        auto dev = _b3d_obj->GetDevice()->As<DeviceD3D12>();
        return dev->IsEnabledDebug();
    }
    else if constexpr (std::is_base_of_v<ISharedBase, T>)
    {
        return _b3d_obj->IsEnabledDebug();
    }
    else
    {
        B3D_UNREFERENCED(_b3d_obj);
        static_assert(false, "T is not B3D_API");
    }
}

template<typename T>
inline constexpr void AddDebugMessage(T* _b3d_obj, DEBUG_MESSAGE_SEVERITY _severity, DEBUG_MESSAGE_CATEGORY_FLAG _category, const Char8T* _str)
{
    if constexpr (!IS_ENABLE_DEBUG_OUTPUT)
    {
        B3D_UNREFERENCED(_b3d_obj, _severity, _category, _str);
        return;
    }

    if constexpr (std::is_base_of_v<IDeviceChild, T>)
    {
        auto dev = _b3d_obj->GetDevice()->As<DeviceD3D12>();
        if (dev->IsEnabledDebug())
            dev->AddMessageFromB3D(_severity, _category, _str);
    }
    else if constexpr (std::is_base_of_v<ISharedBase, T>)
    {
        if (_b3d_obj->IsEnabledDebug())
            _b3d_obj->AddMessageFromB3D(_severity, _category, _str);
    }
}

template<typename T>
inline void HRAddDebugMessageIfFailed(T* _b3d_obj, HRESULT _hr, const char* _file, int _line)
{
    if constexpr (IS_ENABLE_DEBUG_OUTPUT)
    {
        if (FAILED(_hr))
        {
            util::StringStream ss;
            util::String hrmsg; util::GetHRESULTMessage(_hr, &hrmsg);
            ss  << "HRESULT Failed: " << hrmsg << " Code: 0x" << std::hex << _hr
                << ", File: " << _file << " , Line: " << std::dec << _line;
            _b3d_obj->AddMessageFromB3D(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_MISCELLANEOUS, ss.str().c_str());
        }
    }
    else
    {
        B3D_UNREFERENCED(_b3d_obj, _hr, _file, _line);
    }
}

inline void HRTraceIfFailed(HRESULT _hr, const char* _file, int _line)
{
    if constexpr (IS_ENABLE_DEBUG_OUTPUT)
    {
        if (FAILED(_hr))
        {
            util::StringStream ss;
            util::String hrmsg; util::GetHRESULTMessage(_hr, &hrmsg);
            ss  << "HRESULT Failed: " << hrmsg << " Code: 0x" << std::hex << _hr
                << ", File: " << _file << " , Line: " << std::dec << _line;
            buma3d::hlp::OutDebugStr(ss.str().c_str());
        }
    }
    else
    {
        B3D_UNREFERENCED(_hr, _file, _line);
    }
}

template<typename T>
inline BMRESULT HRCheckResult(T* _b3d_obj, HRESULT _hr, const char* _file, int _line)
{
    if constexpr (!std::is_base_of_v<ISharedBase, T>)
    {
        B3D_UNREFERENCED(_b3d_obj);
        buma3d::util::HRTraceIfFailed(_hr, _file, _line);
    }
    else if constexpr (std::is_base_of_v<IDeviceChild, T>)
    {
        auto dev = _b3d_obj->GetDevice()->As<DeviceD3D12>();
        if (dev->IsEnabledDebug())
        {
            dev->CheckDXGIInfoQueue();
            buma3d::util::HRAddDebugMessageIfFailed(dev, _hr, _file, _line);
        }
    }
    else
    {
        if (_b3d_obj->IsEnabledDebug())
        {
            _b3d_obj->CheckDXGIInfoQueue();
            buma3d::util::HRAddDebugMessageIfFailed(_b3d_obj, _hr, _file, _line);
        }
    }
    return util::GetBMResultFromHR(_hr);
}

}// namespace util
}// namespace buma3d
