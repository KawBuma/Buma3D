#pragma once

#define VKR_TRACE_IF_FAILED(x) buma3d::util::VkRCheckResult(this, x, __FILE__, __LINE__)
#define VKR_TRACE_IF_FAILED_EX(device_or_device_child, x) buma3d::util::VkRCheckResult(device_or_device_child, x, __FILE__, __LINE__)

/*HACK: メッセージはIDで管理し、IDに対応するメッセージ文字列を予め別途ファイルから読み込みむようにする。(文字列を埋め込まないようにする。)
        更にデバッグ用の実装を別途作成すればif分岐コストも削減できる(マクロでくくっている部分を無効にした実装をDLLで切り替え...?)が、現状適切な管理方法が思い浮かばないので保留。*/

#define B3D_ADD_DEBUG_MSG(severity, category, msg) if (buma3d::util::IsEnabledDebug(this)) { buma3d::util::AddDebugMessage(this, severity, category, msg); }
#define B3D_ADD_DEBUG_MSG2(severity, category, .../*msgs*/) if (buma3d::util::IsEnabledDebug(this)) { buma3d::util::AddDebugMessage(this, severity, category, buma3d::hlp::StringConvolution(__VA_ARGS__).c_str()); }
#define B3D_ADD_DEBUG_MSG_EX(device_or_device_child, severity, category, msg) if (buma3d::util::IsEnabledDebug(device_or_device_child)) { buma3d::util::AddDebugMessage(device_or_device_child, severity, category, msg); }
#define B3D_ADD_DEBUG_MSG_EX2(device_or_device_child, severity, category, .../*msgs*/) if (buma3d::util::IsEnabledDebug(device_or_device_child)) { buma3d::util::AddDebugMessage(device_or_device_child, severity, category, buma3d::hlp::StringConvolution(__VA_ARGS__).c_str()); }

#define B3D_ADD_DEBUG_MSG_INFO_B3D(.../*msgs*/) if (buma3d::util::IsEnabledDebug(this)) { buma3d::util::AddDebugMessage(this, buma3d::DEBUG_MESSAGE_SEVERITY_OTHER, buma3d::DEBUG_MESSAGE_CATEGORY_FLAG_B3D_DETAILS, buma3d::hlp::StringConvolution( __FUNCTION__ ": ", __VA_ARGS__).c_str()); }


namespace buma3d
{
namespace util
{

inline ROTATION_MODE GetB3DRotationMode(VkSurfaceTransformFlagBitsKHR _transform)
{
    if (_transform & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR ||
        _transform & VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_180_BIT_KHR)
        return ROTATION_MODE_IDENTITY;

    else if (_transform & VK_SURFACE_TRANSFORM_ROTATE_90_BIT_KHR ||
             _transform & VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_270_BIT_KHR)
        return ROTATION_MODE_ROTATE90;

    else if (_transform & VK_SURFACE_TRANSFORM_ROTATE_180_BIT_KHR ||
             _transform & VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_BIT_KHR)
        return ROTATION_MODE_ROTATE180;

    else if (_transform & VK_SURFACE_TRANSFORM_ROTATE_270_BIT_KHR ||
             _transform & VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_90_BIT_KHR)
        return ROTATION_MODE_ROTATE270;

    else
        return ROTATION_MODE_IDENTITY;
}

inline VkSurfaceTransformFlagBitsKHR GetNativeRotationMode(ROTATION_MODE _mode)
{
    switch (_mode)
    {
    case ROTATION_MODE_IDENTITY  : return VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    case ROTATION_MODE_ROTATE90  : return VK_SURFACE_TRANSFORM_ROTATE_90_BIT_KHR;
    case ROTATION_MODE_ROTATE180 : return VK_SURFACE_TRANSFORM_ROTATE_180_BIT_KHR;
    case ROTATION_MODE_ROTATE270 : return VK_SURFACE_TRANSFORM_ROTATE_270_BIT_KHR;
    default:
        return VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    }
}


inline VkCompositeAlphaFlagBitsKHR GetNativeAlphaMode(SWAP_CHAIN_ALPHA_MODE _mode, VkCompositeAlphaFlagsKHR _supported_alpha_mode)
{
    switch (_mode)
    {
    case SWAP_CHAIN_ALPHA_MODE_DEFAULT        : return (_supported_alpha_mode & VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR) ? VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR : VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    case SWAP_CHAIN_ALPHA_MODE_IGNORE         : return VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    case SWAP_CHAIN_ALPHA_MODE_STRAIGHT       : return VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR;
    case SWAP_CHAIN_ALPHA_MODE_PRE_MULTIPLIED : return VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR;
    default:
        return VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
    }
}

inline VkQueueFlags GetNativeCommandType(COMMAND_TYPE _type, bool _is_enable_sparse_bind = false, bool _is_enable_protected = false)
{
    VkQueueFlags result = 0;
    switch (_type)
    {
    case COMMAND_TYPE_DIRECT        : result = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT; break;
    case COMMAND_TYPE_DIRECT_ONLY   : result = VK_QUEUE_GRAPHICS_BIT;                        break;
    case COMMAND_TYPE_COMPUTE_ONLY  : result = VK_QUEUE_COMPUTE_BIT;                         break;
    case COMMAND_TYPE_COPY_ONLY     : result = VK_QUEUE_TRANSFER_BIT;                        break;
    // case COMMAND_TYPE_VIDEO_DECODE  : break; TODO: ビデオキュー
    // case COMMAND_TYPE_VIDEO_PROCESS : break;
    // case COMMAND_TYPE_VIDEO_ENCODE  : break;
    default: 
        break;
    }

    if (_is_enable_sparse_bind)
        result |= VK_QUEUE_SPARSE_BINDING_BIT;
    if (_is_enable_protected)
        result |= VK_QUEUE_PROTECTED_BIT;

    return result;
}

inline COMMAND_TYPE GetB3DCommandType(VkQueueFlags _flags, bool* _is_enable_sparse_bind = nullptr, bool* _is_enable_protected = nullptr)
{
    COMMAND_TYPE result = (COMMAND_TYPE)-1;
    
    if (_is_enable_sparse_bind)
        *_is_enable_sparse_bind = _flags & VK_QUEUE_SPARSE_BINDING_BIT;
    if (_is_enable_protected)
        *_is_enable_protected = _flags & VK_QUEUE_PROTECTED_BIT;
    // 以降の分岐の邪魔にならないよう排除
    _flags &= ~(VK_QUEUE_SPARSE_BINDING_BIT | VK_QUEUE_PROTECTED_BIT);

    // copyのみ
    if (_flags == VK_QUEUE_TRANSFER_BIT)
    {
        result = COMMAND_TYPE_COPY_ONLY;
    }
    else
    {
        // https://www.khronos.org/registry/vulkan/specs/1.2-extensions/html/vkspec.html#VkQueueFlagBits
        // NOTE: GRAPHICS、COMPUTEいずれかのフラグをもつキューにおける TRANSFERフラグの有無は実装によって異なるため、あらかじめ排除しておく。
        _flags &= ~VK_QUEUE_TRANSFER_BIT;

        // graphics | compute
        if (_flags == (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT))
            result = COMMAND_TYPE_DIRECT;

        // graphicsのみ
        else if (_flags == VK_QUEUE_GRAPHICS_BIT)
            result = COMMAND_TYPE_DIRECT_ONLY;

        // computeのみ
        else if (_flags == VK_QUEUE_COMPUTE_BIT)
            result = COMMAND_TYPE_COMPUTE_ONLY;
    }
    
    return result;
}

inline VkOffset2D GetOffset2DFromScissorRect(const SCISSOR_RECT& _rect)
{
    return VkOffset2D{ _rect.offset.x, _rect.offset.y };
}

inline VkExtent2D GetExtent2DFromScissorRect(const SCISSOR_RECT& _rect)
{
    return VkExtent2D{ _rect.extent.width, _rect.extent.height };
}

inline VkOffset2D* GetOffset2DFromScissorRect(const SCISSOR_RECT& _rect, VkOffset2D* _dst)
{
    _dst->x = _rect.offset.x;
    _dst->y = _rect.offset.y;
    return _dst;
}

inline VkExtent2D* GetExtent2DFromScissorRect(const SCISSOR_RECT& _rect, VkExtent2D* _dst)
{
    _dst->width  = _rect.extent.width;
    _dst->height = _rect.extent.height;
    return _dst;
}

inline VkRect2D GetVkRect2DFromScissorRect(const SCISSOR_RECT& _rect)
{
    return VkRect2D{ GetOffset2DFromScissorRect(_rect) , GetExtent2DFromScissorRect(_rect) };
}

template<typename T>
inline T* GetVkRect2DFromScissorRect(const SCISSOR_RECT& _rect, T* _dst_result)
{
    _dst_result->offset.x      = _rect.offset.x;
    _dst_result->offset.y      = _rect.offset.y;
    _dst_result->extent.width  = _rect.extent.width;
    _dst_result->extent.height = _rect.extent.height;
    return _dst_result;
}

inline VkBufferCreateFlags GetNativeBufferCreateFlags(RESOURCE_FLAGS _flags, BUFFER_CREATE_FLAGS _create_flags)
{
    B3D_UNREFERENCED(_create_flags);
    VkBufferCreateFlags result = 0;
    if (_flags & RESOURCE_FLAG_PROTECTED)
        result |= VK_BUFFER_CREATE_PROTECTED_BIT;

    return result;
}

inline VkBufferUsageFlags GetNativeBufferUsageFlags(BUFFER_USAGE_FLAGS _usage_flags)
{
    VkBufferUsageFlags result = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT; // GpuVirtualAddress用
    if (_usage_flags & BUFFER_USAGE_FLAG_COPY_SRC)
        result |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

    if (_usage_flags & BUFFER_USAGE_FLAG_COPY_DST)
        result |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;

    if (_usage_flags & BUFFER_USAGE_FLAG_CONSTANT_BUFFER)// uniform
        result |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

    if (_usage_flags & (BUFFER_USAGE_FLAG_UNORDERED_ACCESS_BUFFER | 
                        BUFFER_USAGE_FLAG_STRUCTURED_BYTEADDRESS_TBUFFER))
        result |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

    if (_usage_flags & (BUFFER_USAGE_FLAG_SHADER_RESOURCE_BUFFER | 
                        BUFFER_USAGE_FLAG_TYPED_SHADER_RESOURCE_BUFFER))
        result |= VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;

    if (_usage_flags & BUFFER_USAGE_FLAG_TYPED_UNORDERED_ACCESS_BUFFER)
        result |= VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;

    if (_usage_flags & BUFFER_USAGE_FLAG_INDEX_BUFFER)
        result |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

    if (_usage_flags & BUFFER_USAGE_FLAG_VERTEX_BUFFER)
        result |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

    if (_usage_flags & BUFFER_USAGE_FLAG_INDIRECT_BUFFER)
        result |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;

    if (_usage_flags & BUFFER_USAGE_FLAG_STREAM_OUTPUT_BUFFER)
        result |= VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_BUFFER_BIT_EXT;

    if (_usage_flags & BUFFER_USAGE_FLAG_STREAM_OUTPUT_COUNTER_BUFFER)
        result |= VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_COUNTER_BUFFER_BIT_EXT;

    if (_usage_flags & BUFFER_USAGE_FLAG_CONDITIONAL_RENDERING)
        result |= VK_BUFFER_USAGE_CONDITIONAL_RENDERING_BIT_EXT;

    if (_usage_flags & BUFFER_USAGE_FLAG_RAY_TRACING)
        result |= VK_BUFFER_USAGE_RAY_TRACING_BIT_KHR;

    return result;
}

inline VkImageCreateFlags GetNativeTextureCreateFlags(RESOURCE_FLAGS _flags, TEXTURE_CREATE_FLAGS _create_flags)
{
    VkImageCreateFlags result = 0;
    if (_flags & RESOURCE_FLAG_PROTECTED)
        result |= VK_IMAGE_CREATE_PROTECTED_BIT;

    if (_create_flags & TEXTURE_CREATE_FLAG_ALIAS)
        result |= VK_IMAGE_CREATE_ALIAS_BIT;

    if (_create_flags & TEXTURE_CREATE_FLAG_2D_AS_CUBE_COMPATIBLE)
        result |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

    if (_create_flags & TEXTURE_CREATE_FLAG_3D_AS_2D_ARRAY_COMPATIBLE)
        result |= VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT;

    if (_create_flags & TEXTURE_CREATE_FLAG_BLOCK_TEXEL_VIEW_COMPATIBLE)
        result |= VK_IMAGE_CREATE_BLOCK_TEXEL_VIEW_COMPATIBLE_BIT;

    if (_create_flags & TEXTURE_CREATE_FLAG_SAMPLE_LOCATIONS_COMPATIBLE_DEPTH)
        result |= VK_IMAGE_CREATE_SAMPLE_LOCATIONS_COMPATIBLE_DEPTH_BIT_EXT;

    if (_create_flags & TEXTURE_CREATE_FLAG_SUBSAMPLED)
        result |= VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT;

    //if (_create_flags & TEXTURE_CREATE_FLAG_SPLIT_INSTANCE_BIND_REGIONS)
    // result |= VK_IMAGE_CREATE_SPLIT_INSTANCE_BIND_REGIONS_BIT;

    return result;
}

inline VkImageUsageFlags GetNativeTextureUsageFlags(TEXTURE_USAGE_FLAGS _usage_flags)
{
    VkImageUsageFlags result = 0;
    if (_usage_flags & TEXTURE_USAGE_FLAG_COPY_SRC)
        result |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

    if (_usage_flags & TEXTURE_USAGE_FLAG_COPY_DST)
        result |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;

    if (_usage_flags & TEXTURE_USAGE_FLAG_SHADER_RESOURCE)
        result |= VK_IMAGE_USAGE_SAMPLED_BIT;

    if (_usage_flags & TEXTURE_USAGE_FLAG_UNORDERED_ACCESS)
        result |= VK_IMAGE_USAGE_STORAGE_BIT;

    if (_usage_flags & TEXTURE_USAGE_FLAG_COLOR_ATTACHMENT)
        result |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    if (_usage_flags & TEXTURE_USAGE_FLAG_DEPTH_STENCIL_ATTACHMENT)
        result |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

    if (_usage_flags & TEXTURE_USAGE_FLAG_TRANSIENT_ATTACHMENT)
        result |= VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;

    if (_usage_flags & TEXTURE_USAGE_FLAG_INPUT_ATTACHMENT)
        result |= VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;

    if (_usage_flags & TEXTURE_USAGE_FLAG_SHADING_RATE_IMAGE)
        result |= VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT | VK_IMAGE_USAGE_SHADING_RATE_IMAGE_BIT_NV;

    return result;
}

inline VkImageUsageFlags GetNativeSwapChainBufferFlags(SWAP_CHAIN_BUFFER_FLAGS _flags)
{
    VkImageUsageFlags result = 0;

    if (_flags & SWAP_CHAIN_BUFFER_FLAG_COPY_SRC)
        result |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

    if (_flags & SWAP_CHAIN_BUFFER_FLAG_COPY_DST)
        result |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;

    if (_flags & SWAP_CHAIN_BUFFER_FLAG_SHADER_RESOURCE)
        result |= VK_IMAGE_USAGE_SAMPLED_BIT;

    if (_flags & SWAP_CHAIN_BUFFER_FLAG_UNORDERED_ACCESS)
        result |= VK_IMAGE_USAGE_STORAGE_BIT;

    if (_flags & SWAP_CHAIN_BUFFER_FLAG_COLOR_ATTACHMENT)
        result |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    if (_flags & SWAP_CHAIN_BUFFER_FLAG_INPUT_ATTACHMENT)
        result |= VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;

    // 関数外で使用されます。
    //if (_flags & SWAP_CHAIN_BUFFER_FLAG_ALLOW_SIMULTANEOUS_ACCESS)

    return result;
}

inline VkImageType GetNativeResourceDimension(RESOURCE_DIMENSION _dimension)
{
    switch (_dimension)
    {
    case buma3d::RESOURCE_DIMENSION_TEX1D: return VK_IMAGE_TYPE_1D;
    case buma3d::RESOURCE_DIMENSION_TEX2D: return VK_IMAGE_TYPE_2D;
    case buma3d::RESOURCE_DIMENSION_TEX3D: return VK_IMAGE_TYPE_3D;

    case buma3d::RESOURCE_DIMENSION_BUFFER:
    default:
        return VkImageType(-1);
    }
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

inline VkSampleCountFlagBits GetNativeSampleCount(uint32_t _sample_count)
{
    // OPTIMIZE: GetNativeSampleCount
    switch (_sample_count)
    {
    case 1  : return VK_SAMPLE_COUNT_1_BIT;
    case 2  : return VK_SAMPLE_COUNT_2_BIT;
    case 4  : return VK_SAMPLE_COUNT_4_BIT;
    case 8  : return VK_SAMPLE_COUNT_8_BIT;
    case 16 : return VK_SAMPLE_COUNT_16_BIT;
    case 32 : return VK_SAMPLE_COUNT_32_BIT;
    case 64 : return VK_SAMPLE_COUNT_64_BIT;

    default:
        return VkSampleCountFlagBits(-1);
    }
}

inline VkImageTiling GetNativeTextureLayout(TEXTURE_LAYOUT _layout)
{
    switch (_layout)
    {
    case buma3d::TEXTURE_LAYOUT_UNKNOWN   : return VK_IMAGE_TILING_OPTIMAL;
    case buma3d::TEXTURE_LAYOUT_ROW_MAJOR : return VK_IMAGE_TILING_LINEAR;

    default:
        return VkImageTiling(-1);
    }
}

inline VkImageAspectFlags GetNativeAspectFlags(TEXTURE_ASPECT_FLAGS _flags)
{
    VkImageAspectFlags result = 0;

    if (_flags & buma3d::TEXTURE_ASPECT_FLAG_COLOR)
        result |= VK_IMAGE_ASPECT_COLOR_BIT;

    if (_flags & buma3d::TEXTURE_ASPECT_FLAG_DEPTH)
        result |= VK_IMAGE_ASPECT_DEPTH_BIT;

    if (_flags & buma3d::TEXTURE_ASPECT_FLAG_STENCIL)
        result |= VK_IMAGE_ASPECT_STENCIL_BIT;

    if (_flags & buma3d::TEXTURE_ASPECT_FLAG_METADATA)
        result |= VK_IMAGE_ASPECT_METADATA_BIT;

    if (_flags & buma3d::TEXTURE_ASPECT_FLAG_PLANE_0)
        result |= VK_IMAGE_ASPECT_PLANE_0_BIT;

    if (_flags & buma3d::TEXTURE_ASPECT_FLAG_PLANE_1)
        result |= VK_IMAGE_ASPECT_PLANE_1_BIT;

    if (_flags & buma3d::TEXTURE_ASPECT_FLAG_PLANE_2)
        result |= VK_IMAGE_ASPECT_PLANE_2_BIT;

    //if (_flags & buma3d::TEXTURE_ASPECT_FLAG_PLANE_3)
    // result |= VK_IMAGE_ASPECT_MEMORY_PLANE_3_BIT_EXT;

    return result;
}

inline TEXTURE_ASPECT_FLAGS GetB3DAspectFlags(VkImageAspectFlags _flags)
{
    TEXTURE_ASPECT_FLAGS result = TEXTURE_ASPECT_FLAG_NONE;

    if (_flags & VK_IMAGE_ASPECT_COLOR_BIT)
        result |= buma3d::TEXTURE_ASPECT_FLAG_COLOR;

    if (_flags & VK_IMAGE_ASPECT_DEPTH_BIT)
        result |= buma3d::TEXTURE_ASPECT_FLAG_DEPTH;

    if (_flags & VK_IMAGE_ASPECT_STENCIL_BIT)
        result |= buma3d::TEXTURE_ASPECT_FLAG_STENCIL;

    if (_flags & VK_IMAGE_ASPECT_METADATA_BIT)
        result |= buma3d::TEXTURE_ASPECT_FLAG_METADATA;

    if (_flags & VK_IMAGE_ASPECT_PLANE_0_BIT)
        result |= buma3d::TEXTURE_ASPECT_FLAG_PLANE_0;

    if (_flags & VK_IMAGE_ASPECT_PLANE_1_BIT)
        result |= buma3d::TEXTURE_ASPECT_FLAG_PLANE_1;

    if (_flags & VK_IMAGE_ASPECT_PLANE_2_BIT)
        result |= buma3d::TEXTURE_ASPECT_FLAG_PLANE_2;

    //if (_flags & VK_IMAGE_ASPECT_MEMORY_PLANE_0_BIT_EXT )
    //if (_flags & VK_IMAGE_ASPECT_MEMORY_PLANE_1_BIT_EXT )
    //if (_flags & VK_IMAGE_ASPECT_MEMORY_PLANE_2_BIT_EXT )
    //if (_flags & VK_IMAGE_ASPECT_MEMORY_PLANE_3_BIT_EXT )

    return result;
}

inline uint32_t GetPlaneSliceFromAspectFlags(TEXTURE_ASPECT_FLAGS _flags)
{
    if (_flags == buma3d::TEXTURE_ASPECT_FLAG_NONE)
        return 0;

    if (_flags & buma3d::TEXTURE_ASPECT_FLAG_DEPTH && _flags & buma3d::TEXTURE_ASPECT_FLAG_STENCIL)
        return 0;

    if (_flags & (buma3d::TEXTURE_ASPECT_FLAG_COLOR | buma3d::TEXTURE_ASPECT_FLAG_DEPTH | buma3d::TEXTURE_ASPECT_FLAG_PLANE_0))
        return 0;

    if (_flags & (buma3d::TEXTURE_ASPECT_FLAG_STENCIL | buma3d::TEXTURE_ASPECT_FLAG_PLANE_1))
        return 1;

    if (_flags & buma3d::TEXTURE_ASPECT_FLAG_PLANE_2)
        return 2;

    // if (_flags & buma3d::TEXTURE_ASPECT_FLAG_METADATA)
    // if (_flags & buma3d::TEXTURE_ASPECT_FLAG_PLANE_3)
    
    return (uint32_t)-1;
}

inline VkImageSubresource* ConvertNativeSubresourceOffset(const SUBRESOURCE_OFFSET& _offset, VkImageSubresource* _result)
{
    _result->aspectMask = GetNativeAspectFlags(_offset.aspect);
    _result->mipLevel   = _offset.mip_slice;
    _result->arrayLayer = _offset.array_slice;

    return _result;
}

inline VkImageSubresourceRange* ConvertNativeSubresourceRange(const SUBRESOURCE_RANGE& _range, VkImageSubresourceRange* _result)
{
    _result->aspectMask     = GetNativeAspectFlags(_range.offset.aspect);
    _result->baseMipLevel   = _range.offset.mip_slice;
    _result->levelCount     = _range.mip_levels;
    _result->baseArrayLayer = _range.offset.array_slice;
    _result->layerCount     = _range.array_size;
    return _result;
}

inline VkImageSubresourceLayers* ConvertNativeSubresourceOffsetWithArraySize(uint32_t _array_size, const SUBRESOURCE_OFFSET& _offset, VkImageSubresourceLayers* _result)
{
    _result->aspectMask     = GetNativeAspectFlags(_offset.aspect);
    _result->mipLevel       = _offset.mip_slice;
    _result->baseArrayLayer = _offset.array_slice;
    _result->layerCount     = _array_size;

    return _result;
}

inline VkSparseImageFormatFlags GetNativeTiledResourceFormatFlags(TILED_RESOURCE_FORMAT_FLAGS _flags)
{
    VkSparseImageFormatFlags result = 0;

    if (_flags & buma3d::TILED_RESOURCE_FORMAT_FLAG_SINGLE_MIPTAIL)
        result |= VK_SPARSE_IMAGE_FORMAT_SINGLE_MIPTAIL_BIT;
    
    if (_flags & buma3d::TILED_RESOURCE_FORMAT_FLAG_ALIGNED_MIP_SIZE)
        result |= VK_SPARSE_IMAGE_FORMAT_ALIGNED_MIP_SIZE_BIT;

    // VK_SPARSE_IMAGE_FORMAT_NONSTANDARD_BLOCK_SIZE_BIT

    return result;
}

inline TILED_RESOURCE_FORMAT_FLAGS GetB3DTiledResourceFormatFlags(VkSparseImageFormatFlags _flags)
{
    TILED_RESOURCE_FORMAT_FLAGS result = TILED_RESOURCE_FORMAT_FLAG_NONE;

    if (_flags & VK_SPARSE_IMAGE_FORMAT_SINGLE_MIPTAIL_BIT)
        result |= buma3d::TILED_RESOURCE_FORMAT_FLAG_SINGLE_MIPTAIL;

    if (_flags & VK_SPARSE_IMAGE_FORMAT_ALIGNED_MIP_SIZE_BIT)
        result |= buma3d::TILED_RESOURCE_FORMAT_FLAG_ALIGNED_MIP_SIZE;

    // if (_flags & VK_SPARSE_IMAGE_FORMAT_NONSTANDARD_BLOCK_SIZE_BIT)

    return result;
}

inline VkImageViewType GetNativeViewDimension(VIEW_DIMENSION _dim)
{
    switch (_dim)
    {
    case buma3d::VIEW_DIMENSION_TEXTURE_1D         : return VK_IMAGE_VIEW_TYPE_1D;
    case buma3d::VIEW_DIMENSION_TEXTURE_1D_ARRAY   : return VK_IMAGE_VIEW_TYPE_1D_ARRAY;
    case buma3d::VIEW_DIMENSION_TEXTURE_2D         : return VK_IMAGE_VIEW_TYPE_2D;
    case buma3d::VIEW_DIMENSION_TEXTURE_2D_ARRAY   : return VK_IMAGE_VIEW_TYPE_2D_ARRAY;
    case buma3d::VIEW_DIMENSION_TEXTURE_3D         : return VK_IMAGE_VIEW_TYPE_3D;
    case buma3d::VIEW_DIMENSION_TEXTURE_CUBE       : return VK_IMAGE_VIEW_TYPE_CUBE;
    case buma3d::VIEW_DIMENSION_TEXTURE_CUBE_ARRAY : return VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;

    //case buma3d::VIEW_DIMENSION_CONSTANT_BUFFER:
    //case buma3d::VIEW_DIMENSION_BUFFER_TYPED:
    //case buma3d::VIEW_DIMENSION_BUFFER_STRUCTURED:
    //case buma3d::VIEW_DIMENSION_BUFFER_COUNTER:
    //case buma3d::VIEW_DIMENSION_BUFFER_ACCELERATION_STRUCTURE:
    //case buma3d::VIEW_DIMENSION_VERTEX_BUFFER:
    //case buma3d::VIEW_DIMENSION_INDEX_BUFFER:
    default:
        return VkImageViewType(-1);
    }
}

inline VkComponentSwizzle GetNativeComponentSwizzle(COMPONENT_SWIZZLE _swizzle)
{
    switch (_swizzle)
    {
    case buma3d::COMPONENT_SWIZZLE_IDENTITY : return VK_COMPONENT_SWIZZLE_IDENTITY;
    case buma3d::COMPONENT_SWIZZLE_ZERO     : return VK_COMPONENT_SWIZZLE_ZERO;
    case buma3d::COMPONENT_SWIZZLE_ONE      : return VK_COMPONENT_SWIZZLE_ONE;
    case buma3d::COMPONENT_SWIZZLE_R        : return VK_COMPONENT_SWIZZLE_R;
    case buma3d::COMPONENT_SWIZZLE_G        : return VK_COMPONENT_SWIZZLE_G;
    case buma3d::COMPONENT_SWIZZLE_B        : return VK_COMPONENT_SWIZZLE_B;
    case buma3d::COMPONENT_SWIZZLE_A        : return VK_COMPONENT_SWIZZLE_A;

    default:
        return VkComponentSwizzle(-1);
    }
}

inline VkComponentMapping* ConvertNativeComponentMapping(const COMPONENT_MAPPING& _mapping, VkComponentMapping* _result)
{
    _result->r = GetNativeComponentSwizzle(_mapping.r);
    _result->g = GetNativeComponentSwizzle(_mapping.g);
    _result->b = GetNativeComponentSwizzle(_mapping.b);
    _result->a = GetNativeComponentSwizzle(_mapping.a);

    return _result;
}

inline bool IsIdentifyComponentMapping(const COMPONENT_MAPPING& _mapping)
{
    return (_mapping.r == buma3d::COMPONENT_SWIZZLE_R || _mapping.r == buma3d::COMPONENT_SWIZZLE_IDENTITY) &&
           (_mapping.g == buma3d::COMPONENT_SWIZZLE_G || _mapping.g == buma3d::COMPONENT_SWIZZLE_IDENTITY) &&
           (_mapping.b == buma3d::COMPONENT_SWIZZLE_B || _mapping.b == buma3d::COMPONENT_SWIZZLE_IDENTITY) &&
           (_mapping.a == buma3d::COMPONENT_SWIZZLE_A || _mapping.a == buma3d::COMPONENT_SWIZZLE_IDENTITY);
}

inline VkCompareOp GetNativeComparisonFunc(const COMPARISON_FUNC _cf)
{
    switch (_cf)
    {
    case buma3d::COMPARISON_FUNC_NEVER         : return VK_COMPARE_OP_NEVER;
    case buma3d::COMPARISON_FUNC_LESS          : return VK_COMPARE_OP_LESS;
    case buma3d::COMPARISON_FUNC_EQUAL         : return VK_COMPARE_OP_EQUAL;
    case buma3d::COMPARISON_FUNC_LESS_EQUAL    : return VK_COMPARE_OP_LESS_OR_EQUAL;
    case buma3d::COMPARISON_FUNC_GREATER       : return VK_COMPARE_OP_GREATER;
    case buma3d::COMPARISON_FUNC_NOT_EQUAL     : return VK_COMPARE_OP_NOT_EQUAL;
    case buma3d::COMPARISON_FUNC_GREATER_EQUAL : return VK_COMPARE_OP_GREATER_OR_EQUAL;
    case buma3d::COMPARISON_FUNC_ALWAYS        : return VK_COMPARE_OP_ALWAYS;
    default:
        return VkCompareOp(-1);
    }
}

inline VkSamplerAddressMode GetNativeAddressMode(const TEXTURE_ADDRESS_MODE _address_mode)
{
    switch (_address_mode)
    {
    case buma3d::TEXTURE_ADDRESS_MODE_WRAP        : return VK_SAMPLER_ADDRESS_MODE_REPEAT;
    case buma3d::TEXTURE_ADDRESS_MODE_MIRROR      : return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
    case buma3d::TEXTURE_ADDRESS_MODE_CLAMP       : return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    case buma3d::TEXTURE_ADDRESS_MODE_BORDER      : return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    case buma3d::TEXTURE_ADDRESS_MODE_MIRROR_ONCE : return VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
    default:
        return VkSamplerAddressMode(-1);
    }
}

inline VkFilter GetNativeTextureSampleMode(const TEXTURE_SAMPLE_MODE _sample_mode)
{
    switch (_sample_mode)
    {
    case buma3d::TEXTURE_SAMPLE_MODE_POINT     : return VK_FILTER_NEAREST;
    case buma3d::TEXTURE_SAMPLE_MODE_LINEAR    : return VK_FILTER_LINEAR;
    case buma3d::TEXTURE_SAMPLE_MODE_CUBIC_IMG : return VK_FILTER_CUBIC_IMG; 
    default:
        return VkFilter(-1);
    }
}

inline VkSamplerMipmapMode GetNativeTextureSampleModeMip(const TEXTURE_SAMPLE_MODE _sample_mode)
{
    switch (_sample_mode)
    {
    case buma3d::TEXTURE_SAMPLE_MODE_POINT  : return VK_SAMPLER_MIPMAP_MODE_NEAREST;
    case buma3d::TEXTURE_SAMPLE_MODE_LINEAR : return VK_SAMPLER_MIPMAP_MODE_LINEAR;
    default:
        return VkSamplerMipmapMode(-1);
    }
}

inline VkSamplerReductionMode GetNativeFilterReductionMode(const SAMPLER_FILTER_REDUCTION_MODE _reduction_mode)
{
    switch (_reduction_mode)
    {
    case buma3d::SAMPLER_FILTER_REDUCTION_MODE_STANDARD   : return VK_SAMPLER_REDUCTION_MODE_WEIGHTED_AVERAGE;
    case buma3d::SAMPLER_FILTER_REDUCTION_MODE_COMPARISON : return VK_SAMPLER_REDUCTION_MODE_WEIGHTED_AVERAGE;
    case buma3d::SAMPLER_FILTER_REDUCTION_MODE_MIN        : return VK_SAMPLER_REDUCTION_MODE_MIN;
    case buma3d::SAMPLER_FILTER_REDUCTION_MODE_MAX        : return VK_SAMPLER_REDUCTION_MODE_MAX;
    default:
        return VkSamplerReductionMode(-1);
    }
}

inline VkBorderColor GetNativeBorderColor(const BORDER_COLOR _border_color)
{
    switch (_border_color)
    {
    case buma3d::BORDER_COLOR_TRANSPARENT_BLACK_FLOAT : return VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
    case buma3d::BORDER_COLOR_TRANSPARENT_BLACK_INT   : return VK_BORDER_COLOR_INT_TRANSPARENT_BLACK;
    case buma3d::BORDER_COLOR_OPAQUE_BLACK_FLOAT      : return VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
    case buma3d::BORDER_COLOR_OPAQUE_BLACK_INT        : return VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    case buma3d::BORDER_COLOR_OPAQUE_WHITE_FLOAT      : return VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    case buma3d::BORDER_COLOR_OPAQUE_WHITE_INT        : return VK_BORDER_COLOR_INT_OPAQUE_WHITE;
    default:
        return VkBorderColor(-1);
    }
}

inline VkAttachmentDescriptionFlags GetNativeAttachmentFlags(ATTACHMENT_FLAGS _flags)
{
    VkAttachmentDescriptionFlags result = 0;

    if (_flags & buma3d::ATTACHMENT_FLAG_MAY_ALIAS)
        result |= VK_ATTACHMENT_DESCRIPTION_MAY_ALIAS_BIT;

    return result;
}

inline VkAttachmentLoadOp GetNativeLoadOp(ATTACHMENT_LOAD_OP _load_op)
{
    switch (_load_op)
    {
    case buma3d::ATTACHMENT_LOAD_OP_LOAD      : return VK_ATTACHMENT_LOAD_OP_LOAD;
    case buma3d::ATTACHMENT_LOAD_OP_CLEAR     : return VK_ATTACHMENT_LOAD_OP_CLEAR;
    case buma3d::ATTACHMENT_LOAD_OP_DONT_CARE : return VK_ATTACHMENT_LOAD_OP_DONT_CARE;

    default:
        return VkAttachmentLoadOp(-1);
    }
}

inline VkAttachmentStoreOp GetNativeStoreOp(ATTACHMENT_STORE_OP _store_op)
{

    switch (_store_op)
    {
    case buma3d::ATTACHMENT_STORE_OP_STORE     : return VK_ATTACHMENT_STORE_OP_STORE;
    case buma3d::ATTACHMENT_STORE_OP_DONT_CARE : return VK_ATTACHMENT_STORE_OP_DONT_CARE;

    default:
        return VkAttachmentStoreOp(-1);
    }
}

inline VkImageLayout ConvertResourceStateForDepthStencil(TEXTURE_ASPECT_FLAGS _aspects, RESOURCE_STATE _depth_or_depth_stencil_state)
{
    VkImageLayout result = VK_IMAGE_LAYOUT_UNDEFINED;

    bool depth = _aspects & TEXTURE_ASPECT_FLAG_DEPTH;
    bool stencil = _aspects & TEXTURE_ASPECT_FLAG_STENCIL;
    if (depth && stencil)
    {
        if (_depth_or_depth_stencil_state == buma3d::RESOURCE_STATE_DEPTH_STENCIL_ATTACHMENT_READ)
            result = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

        else if (_depth_or_depth_stencil_state == buma3d::RESOURCE_STATE_DEPTH_STENCIL_ATTACHMENT_WRITE ||
                 _depth_or_depth_stencil_state == buma3d::RESOURCE_STATE_RESOLVE_DEPTH_STENCIL_ATTACHMENT_WRITE)
            result = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    }
    else if (depth)
    {
        if (_depth_or_depth_stencil_state == buma3d::RESOURCE_STATE_DEPTH_STENCIL_ATTACHMENT_READ)
            result = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL;

        else if (_depth_or_depth_stencil_state == buma3d::RESOURCE_STATE_DEPTH_STENCIL_ATTACHMENT_WRITE ||
                 _depth_or_depth_stencil_state == buma3d::RESOURCE_STATE_RESOLVE_DEPTH_STENCIL_ATTACHMENT_WRITE)
            result = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
    }
    else if (stencil)
    {
        if (_depth_or_depth_stencil_state == buma3d::RESOURCE_STATE_DEPTH_STENCIL_ATTACHMENT_READ)
            result = VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL;

        else if (_depth_or_depth_stencil_state == buma3d::RESOURCE_STATE_DEPTH_STENCIL_ATTACHMENT_WRITE ||
                 _depth_or_depth_stencil_state == buma3d::RESOURCE_STATE_RESOLVE_DEPTH_STENCIL_ATTACHMENT_WRITE)
            result = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;
    }

    return result;
}

inline VkAccessFlags GetNativeResourceState(RESOURCE_STATE _state)
{
    switch (_state)
    {
    case buma3d::RESOURCE_STATE_UNDEFINED                               : return 0x0;
    case buma3d::RESOURCE_STATE_INDIRECT_ARGUMENT_READ                  : return VK_ACCESS_INDIRECT_COMMAND_READ_BIT;
    case buma3d::RESOURCE_STATE_CONDITIONAL_RENDERING_READ              : return VK_ACCESS_CONDITIONAL_RENDERING_READ_BIT_EXT;
    case buma3d::RESOURCE_STATE_INDEX_READ                              : return VK_ACCESS_INDEX_READ_BIT;
    case buma3d::RESOURCE_STATE_VERTEX_ATTRIBUTE_READ                   : return VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
    case buma3d::RESOURCE_STATE_CONSTANT_READ                           : return VK_ACCESS_UNIFORM_READ_BIT;
    case buma3d::RESOURCE_STATE_SHADER_READ                             : return VK_ACCESS_SHADER_READ_BIT;
    case buma3d::RESOURCE_STATE_SHADER_WRITE                            : return VK_ACCESS_SHADER_WRITE_BIT;
    case buma3d::RESOURCE_STATE_SHADER_READ_WRITE                       : return VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
    case buma3d::RESOURCE_STATE_INPUT_ATTACHMENT_READ                   : return VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
    case buma3d::RESOURCE_STATE_COLOR_ATTACHMENT_READ                   : return VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
    case buma3d::RESOURCE_STATE_COLOR_ATTACHMENT_WRITE                  : return VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    case buma3d::RESOURCE_STATE_COLOR_ATTACHMENT_READ_WRITE             : return VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    case buma3d::RESOURCE_STATE_DEPTH_STENCIL_ATTACHMENT_READ           : return VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
    case buma3d::RESOURCE_STATE_DEPTH_STENCIL_ATTACHMENT_WRITE          : return VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    case buma3d::RESOURCE_STATE_DEPTH_STENCIL_ATTACHMENT_READ_WRITE     : return VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    case buma3d::RESOURCE_STATE_RESOLVE_COLOR_ATTACHMENT_WRITE          : return VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    case buma3d::RESOURCE_STATE_RESOLVE_DEPTH_STENCIL_ATTACHMENT_WRITE  : return VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
    case buma3d::RESOURCE_STATE_COPY_SRC_READ                           : return VK_ACCESS_TRANSFER_READ_BIT;
    case buma3d::RESOURCE_STATE_COPY_DST_WRITE                          : return VK_ACCESS_TRANSFER_WRITE_BIT;
    case buma3d::RESOURCE_STATE_RESOLVE_SRC_READ                        : return VK_ACCESS_TRANSFER_READ_BIT;
    case buma3d::RESOURCE_STATE_RESOLVE_DST_WRITE                       : return VK_ACCESS_TRANSFER_WRITE_BIT;
    case buma3d::RESOURCE_STATE_HOST_READ                               : return VK_ACCESS_HOST_READ_BIT;
    case buma3d::RESOURCE_STATE_HOST_WRITE                              : return VK_ACCESS_HOST_WRITE_BIT;
    case buma3d::RESOURCE_STATE_HOST_READ_WRITE                         : return VK_ACCESS_HOST_READ_BIT | VK_ACCESS_HOST_WRITE_BIT;
    case buma3d::RESOURCE_STATE_GENERIC_MEMORY_READ                     : return VK_ACCESS_MEMORY_READ_BIT;
    case buma3d::RESOURCE_STATE_STREAM_OUTPUT_WRITE                     : return VK_ACCESS_TRANSFORM_FEEDBACK_WRITE_BIT_EXT;
    case buma3d::RESOURCE_STATE_STREAM_OUTPUT_COUNTER_READ              : return VK_ACCESS_TRANSFORM_FEEDBACK_COUNTER_READ_BIT_EXT;
    case buma3d::RESOURCE_STATE_STREAM_OUTPUT_COUNTER_WRITE             : return VK_ACCESS_TRANSFORM_FEEDBACK_COUNTER_WRITE_BIT_EXT;
    case buma3d::RESOURCE_STATE_STREAM_OUTPUT_COUNTER_READ_WRITE        : return VK_ACCESS_TRANSFORM_FEEDBACK_COUNTER_READ_BIT_EXT | VK_ACCESS_TRANSFORM_FEEDBACK_COUNTER_WRITE_BIT_EXT;
    case buma3d::RESOURCE_STATE_ACCELERATION_STRUCTURE_READ             : return VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
    case buma3d::RESOURCE_STATE_ACCELERATION_STRUCTURE_WRITE            : return VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;
    case buma3d::RESOURCE_STATE_ACCELERATION_STRUCTURE_READ_WRITE       : return VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR | VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;
    case buma3d::RESOURCE_STATE_SHADING_RATE_IMAGE_READ                 : return VK_ACCESS_SHADING_RATE_IMAGE_READ_BIT_NV;
    case buma3d::RESOURCE_STATE_PRESENT                                 : return 0x0;

    default:
        return VkAccessFlags(0);
    }
}

inline VkImageLayout GetNativeResourceStateForLayout(RESOURCE_STATE _state, TEXTURE_ASPECT_FLAGS _aspect = TEXTURE_ASPECT_FLAG_NONE)
{
    switch (_state)
    {
    case buma3d::RESOURCE_STATE_UNDEFINED                           : return VK_IMAGE_LAYOUT_UNDEFINED;
    case buma3d::RESOURCE_STATE_INDIRECT_ARGUMENT_READ              : return VK_IMAGE_LAYOUT_UNDEFINED;
    case buma3d::RESOURCE_STATE_CONDITIONAL_RENDERING_READ          : return VK_IMAGE_LAYOUT_UNDEFINED;
    case buma3d::RESOURCE_STATE_INDEX_READ                          : return VK_IMAGE_LAYOUT_UNDEFINED;
    case buma3d::RESOURCE_STATE_VERTEX_ATTRIBUTE_READ               : return VK_IMAGE_LAYOUT_UNDEFINED;
    case buma3d::RESOURCE_STATE_CONSTANT_READ                       : return VK_IMAGE_LAYOUT_UNDEFINED;
    case buma3d::RESOURCE_STATE_SHADER_READ                         : return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    case buma3d::RESOURCE_STATE_SHADER_WRITE                        : return VK_IMAGE_LAYOUT_GENERAL;
    case buma3d::RESOURCE_STATE_SHADER_READ_WRITE                   : return VK_IMAGE_LAYOUT_GENERAL;
    case buma3d::RESOURCE_STATE_INPUT_ATTACHMENT_READ               : return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    case buma3d::RESOURCE_STATE_COLOR_ATTACHMENT_READ               : return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    case buma3d::RESOURCE_STATE_COLOR_ATTACHMENT_WRITE              : return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    case buma3d::RESOURCE_STATE_COLOR_ATTACHMENT_READ_WRITE         : return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    case buma3d::RESOURCE_STATE_RESOLVE_COLOR_ATTACHMENT_WRITE      : return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    case buma3d::RESOURCE_STATE_DEPTH_STENCIL_ATTACHMENT_READ       :
    case buma3d::RESOURCE_STATE_DEPTH_STENCIL_ATTACHMENT_WRITE      :
    case buma3d::RESOURCE_STATE_DEPTH_STENCIL_ATTACHMENT_READ_WRITE :
    case buma3d::RESOURCE_STATE_RESOLVE_DEPTH_STENCIL_ATTACHMENT_WRITE: 
        return ConvertResourceStateForDepthStencil(_aspect, _state);

    case buma3d::RESOURCE_STATE_COPY_SRC_READ                       : return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    case buma3d::RESOURCE_STATE_COPY_DST_WRITE                      : return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    case buma3d::RESOURCE_STATE_RESOLVE_SRC_READ                    : return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    case buma3d::RESOURCE_STATE_RESOLVE_DST_WRITE                   : return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    case buma3d::RESOURCE_STATE_HOST_READ                           : return VK_IMAGE_LAYOUT_GENERAL;
    case buma3d::RESOURCE_STATE_HOST_WRITE                          : return VK_IMAGE_LAYOUT_GENERAL;
    case buma3d::RESOURCE_STATE_HOST_READ_WRITE                     : return VK_IMAGE_LAYOUT_GENERAL;
    case buma3d::RESOURCE_STATE_GENERIC_MEMORY_READ                 : return VK_IMAGE_LAYOUT_GENERAL;
    case buma3d::RESOURCE_STATE_STREAM_OUTPUT_WRITE                 : return VK_IMAGE_LAYOUT_UNDEFINED;
    case buma3d::RESOURCE_STATE_STREAM_OUTPUT_COUNTER_READ          : return VK_IMAGE_LAYOUT_UNDEFINED;
    case buma3d::RESOURCE_STATE_STREAM_OUTPUT_COUNTER_WRITE         : return VK_IMAGE_LAYOUT_UNDEFINED;
    case buma3d::RESOURCE_STATE_STREAM_OUTPUT_COUNTER_READ_WRITE    : return VK_IMAGE_LAYOUT_UNDEFINED;
    case buma3d::RESOURCE_STATE_ACCELERATION_STRUCTURE_READ         : return VK_IMAGE_LAYOUT_UNDEFINED;
    case buma3d::RESOURCE_STATE_ACCELERATION_STRUCTURE_WRITE        : return VK_IMAGE_LAYOUT_UNDEFINED;
    case buma3d::RESOURCE_STATE_ACCELERATION_STRUCTURE_READ_WRITE   : return VK_IMAGE_LAYOUT_UNDEFINED;
    case buma3d::RESOURCE_STATE_SHADING_RATE_IMAGE_READ             : return VK_IMAGE_LAYOUT_SHADING_RATE_OPTIMAL_NV;
    case buma3d::RESOURCE_STATE_PRESENT                             : return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    default:
        return VK_IMAGE_LAYOUT_UNDEFINED;
    }
}

inline VkAccessFlags GetNativeResourceAccessFlags(RESOURCE_ACCESS_FLAGS _flags)
{
    VkAccessFlags result = 0;

    if (_flags & buma3d::RESOURCE_ACCESS_FLAG_INDIRECT_ARGUMENT_READ           ) result |= VK_ACCESS_INDIRECT_COMMAND_READ_BIT;
    if (_flags & buma3d::RESOURCE_ACCESS_FLAG_INDEX_READ                       ) result |= VK_ACCESS_INDEX_READ_BIT;
    if (_flags & buma3d::RESOURCE_ACCESS_FLAG_VERTEX_ATTRIBUTE_READ            ) result |= VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
    if (_flags & buma3d::RESOURCE_ACCESS_FLAG_CONSTANT_READ                    ) result |= VK_ACCESS_UNIFORM_READ_BIT;
    if (_flags & buma3d::RESOURCE_ACCESS_FLAG_INPUT_ATTACHMENT_READ            ) result |= VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
    if (_flags & buma3d::RESOURCE_ACCESS_FLAG_SHADER_READ                      ) result |= VK_ACCESS_SHADER_READ_BIT;
    if (_flags & buma3d::RESOURCE_ACCESS_FLAG_SHADER_WRITE                     ) result |= VK_ACCESS_SHADER_WRITE_BIT;
    if (_flags & buma3d::RESOURCE_ACCESS_FLAG_COLOR_ATTACHMENT_READ            ) result |= VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
    if (_flags & buma3d::RESOURCE_ACCESS_FLAG_COLOR_ATTACHMENT_WRITE           ) result |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    if (_flags & buma3d::RESOURCE_ACCESS_FLAG_DEPTH_STENCIL_ATTACHMENT_READ    ) result |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
    if (_flags & buma3d::RESOURCE_ACCESS_FLAG_DEPTH_STENCIL_ATTACHMENT_WRITE   ) result |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    if (_flags & (buma3d::RESOURCE_ACCESS_FLAG_COPY_READ  | buma3d::RESOURCE_ACCESS_FLAG_RESOLVE_READ )) result |= VK_ACCESS_TRANSFER_READ_BIT;
    if (_flags & (buma3d::RESOURCE_ACCESS_FLAG_COPY_WRITE | buma3d::RESOURCE_ACCESS_FLAG_RESOLVE_WRITE)) result |= VK_ACCESS_TRANSFER_WRITE_BIT;
    if (_flags & buma3d::RESOURCE_ACCESS_FLAG_HOST_READ                        ) result |= VK_ACCESS_HOST_READ_BIT;
    if (_flags & buma3d::RESOURCE_ACCESS_FLAG_HOST_WRITE                       ) result |= VK_ACCESS_HOST_WRITE_BIT;
    if (_flags & buma3d::RESOURCE_ACCESS_FLAG_GENERIC_MEMORY_READ              ) result |= VK_ACCESS_MEMORY_READ_BIT;
    if (_flags & buma3d::RESOURCE_ACCESS_FLAG_STREAM_OUTPUT_WRITE              ) result |= VK_ACCESS_TRANSFORM_FEEDBACK_WRITE_BIT_EXT;
    if (_flags & buma3d::RESOURCE_ACCESS_FLAG_STREAM_OUTPUT_COUNTER_READ       ) result |= VK_ACCESS_TRANSFORM_FEEDBACK_COUNTER_READ_BIT_EXT;
    if (_flags & buma3d::RESOURCE_ACCESS_FLAG_STREAM_OUTPUT_COUNTER_WRITE      ) result |= VK_ACCESS_TRANSFORM_FEEDBACK_COUNTER_WRITE_BIT_EXT;
    if (_flags & buma3d::RESOURCE_ACCESS_FLAG_CONDITIONAL_RENDERING_READ       ) result |= VK_ACCESS_CONDITIONAL_RENDERING_READ_BIT_EXT;
    if (_flags & buma3d::RESOURCE_ACCESS_FLAG_ACCELERATION_STRUCTURE_READ      ) result |= VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
    if (_flags & buma3d::RESOURCE_ACCESS_FLAG_ACCELERATION_STRUCTURE_WRITE     ) result |= VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;
    if (_flags & buma3d::RESOURCE_ACCESS_FLAG_SHADING_RATE_IMAGE_READ          ) result |= VK_ACCESS_SHADING_RATE_IMAGE_READ_BIT_NV;
//  if (_flags & buma3d::RESOURCE_ACCESS_FLAG_PRESENT                          ) result |= 0;

    return result;
}

inline VkPipelineStageFlags GetNativePipelineStageFlags(PIPELINE_STAGE_FLAGS _flags)
{
    VkPipelineStageFlags result = 0;

    if (_flags & buma3d::PIPELINE_STAGE_FLAG_TOP_OF_PIPE                 ) result |= VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    if (_flags & buma3d::PIPELINE_STAGE_FLAG_DRAW_INDIRECT               ) result |= VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT;
    if (_flags & buma3d::PIPELINE_STAGE_FLAG_INPUT_ASSEMBLER             ) result |= VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
    if (_flags & buma3d::PIPELINE_STAGE_FLAG_VERTEX_SHADER               ) result |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
    if (_flags & buma3d::PIPELINE_STAGE_FLAG_HULL_SHADER                 ) result |= VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT;
    if (_flags & buma3d::PIPELINE_STAGE_FLAG_DOMAIN_SHADER               ) result |= VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT;
    if (_flags & buma3d::PIPELINE_STAGE_FLAG_GEOMETRY_SHADER             ) result |= VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT;
    if (_flags & buma3d::PIPELINE_STAGE_FLAG_PIXEL_SHADER                ) result |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    if (_flags & buma3d::PIPELINE_STAGE_FLAG_EARLY_DEPTH_STENCIL_TESTS   ) result |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    if (_flags & buma3d::PIPELINE_STAGE_FLAG_LATE_FRAGMENT_TESTS         ) result |= VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    if (_flags & buma3d::PIPELINE_STAGE_FLAG_COLOR_ATTACHMENT_OUTPUT     ) result |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    if (_flags & buma3d::PIPELINE_STAGE_FLAG_COMPUTE_SHADER              ) result |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
    if (_flags & buma3d::PIPELINE_STAGE_FLAG_COPY_RESOLVE                ) result |= VK_PIPELINE_STAGE_TRANSFER_BIT;
    if (_flags & buma3d::PIPELINE_STAGE_FLAG_BOTTOM_OF_PIPE              ) result |= VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    if (_flags & buma3d::PIPELINE_STAGE_FLAG_HOST                        ) result |= VK_PIPELINE_STAGE_HOST_BIT;
    if (_flags & buma3d::PIPELINE_STAGE_FLAG_ALL_GRAPHICS                ) result |= VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
    if (_flags & buma3d::PIPELINE_STAGE_FLAG_ALL_COMMANDS                ) result |= VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    if (_flags & buma3d::PIPELINE_STAGE_FLAG_STREAM_OUTPUT               ) result |= VK_PIPELINE_STAGE_TRANSFORM_FEEDBACK_BIT_EXT;
    if (_flags & buma3d::PIPELINE_STAGE_FLAG_CONDITIONAL_RENDERING       ) result |= VK_PIPELINE_STAGE_CONDITIONAL_RENDERING_BIT_EXT;
    if (_flags & buma3d::PIPELINE_STAGE_FLAG_SHADING_RATE_IMAGE          ) result |= VK_PIPELINE_STAGE_SHADING_RATE_IMAGE_BIT_NV;
    if (_flags & buma3d::PIPELINE_STAGE_FLAG_RAY_TRACING_SHADER          ) result |= VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR;
    if (_flags & buma3d::PIPELINE_STAGE_FLAG_ACCELERATION_STRUCTURE_BUILD) result |= VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR;
    if (_flags & buma3d::PIPELINE_STAGE_FLAG_TASK_SHADER                 ) result |= VK_PIPELINE_STAGE_TASK_SHADER_BIT_NV;
    if (_flags & buma3d::PIPELINE_STAGE_FLAG_MESH_SHADER                 ) result |= VK_PIPELINE_STAGE_MESH_SHADER_BIT_NV;

    return result;
}

inline VkShaderStageFlags MaskShaderVisibility(VkShaderStageFlags _visibility, ROOT_SIGNATURE_FLAGS _flags, RAY_TRACING_SHADER_VISIBILITY_FLAGS _rt_visibility = RAY_TRACING_SHADER_VISIBILITY_FLAG_NONE)
{
    VkShaderStageFlags result = _visibility;
    if (  _flags & buma3d::ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS    ) result &= ~VK_SHADER_STAGE_VERTEX_BIT;
    if (  _flags & buma3d::ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS     ) result &= ~VK_SHADER_STAGE_FRAGMENT_BIT;
    if (!(_flags & buma3d::ROOT_SIGNATURE_FLAG_ALLOW_HULL_SHADER_ROOT_ACCESS)    ) result &= ~VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
    if (!(_flags & buma3d::ROOT_SIGNATURE_FLAG_ALLOW_DOMAIN_SHADER_ROOT_ACCESS)  ) result &= ~VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
    if (!(_flags & buma3d::ROOT_SIGNATURE_FLAG_ALLOW_GEOMETRY_SHADER_ROOT_ACCESS)) result &= ~VK_SHADER_STAGE_GEOMETRY_BIT;
    if (!(_flags & buma3d::ROOT_SIGNATURE_FLAG_ALLOW_MESH_SHADER_ROOT_ACCESS)    ) result &= ~VK_SHADER_STAGE_TASK_BIT_NV;
    if (!(_flags & buma3d::ROOT_SIGNATURE_FLAG_ALLOW_TASK_SHADER_ROOT_ACCESS)    ) result &= ~VK_SHADER_STAGE_MESH_BIT_NV;

    //if (_flags & ROOT_SIGNATURE_FLAG_DENY_INPUT_ASSEMBLER_INPUT_LAYOUT) return ;
    //if (_flags & ROOT_SIGNATURE_FLAG_ALLOW_STREAM_OUTPUT) return ;

    if (_rt_visibility & RAY_TRACING_SHADER_VISIBILITY_FLAG_RAYGEN       ) result |= VK_SHADER_STAGE_RAYGEN_BIT_KHR;
    if (_rt_visibility & RAY_TRACING_SHADER_VISIBILITY_FLAG_ANY_HIT      ) result |= VK_SHADER_STAGE_ANY_HIT_BIT_KHR;
    if (_rt_visibility & RAY_TRACING_SHADER_VISIBILITY_FLAG_CLOSEST_HIT  ) result |= VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
    if (_rt_visibility & RAY_TRACING_SHADER_VISIBILITY_FLAG_MISS         ) result |= VK_SHADER_STAGE_MISS_BIT_KHR;
    if (_rt_visibility & RAY_TRACING_SHADER_VISIBILITY_FLAG_INTERSECTION ) result |= VK_SHADER_STAGE_INTERSECTION_BIT_KHR;
    if (_rt_visibility & RAY_TRACING_SHADER_VISIBILITY_FLAG_CALLABLE     ) result |= VK_SHADER_STAGE_CALLABLE_BIT_KHR;

    return result;
}

inline VkShaderStageFlags GetNativeShaderVisibility(SHADER_VISIBILITY _visibility)
{
    switch (_visibility)
    {
    case buma3d::SHADER_VISIBILITY_VERTEX               : return VK_SHADER_STAGE_VERTEX_BIT;
    case buma3d::SHADER_VISIBILITY_HULL                 : return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
    case buma3d::SHADER_VISIBILITY_DOMAIN               : return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
    case buma3d::SHADER_VISIBILITY_GEOMETRY             : return VK_SHADER_STAGE_GEOMETRY_BIT;
    case buma3d::SHADER_VISIBILITY_PIXEL                : return VK_SHADER_STAGE_FRAGMENT_BIT;
    case buma3d::SHADER_VISIBILITY_MESH                 : return VK_SHADER_STAGE_MESH_BIT_NV;
    case buma3d::SHADER_VISIBILITY_TASK                 : return VK_SHADER_STAGE_TASK_BIT_NV;
    case buma3d::SHADER_VISIBILITY_ALL_GRAPHICS_COMPUTE : return VK_SHADER_STAGE_ALL_GRAPHICS | VK_SHADER_STAGE_COMPUTE_BIT;
    default:
        return VkShaderStageFlags(0);
    }
}

inline VkShaderStageFlagBits GetNativeShaderStageFlagBit(SHADER_STAGE_FLAG _a_stage)
{
    switch (_a_stage)
    {
    case buma3d::SHADER_STAGE_FLAG_UNKNOWN     : return VkShaderStageFlagBits(0);
    case buma3d::SHADER_STAGE_FLAG_VERTEX      : return VK_SHADER_STAGE_VERTEX_BIT;
    case buma3d::SHADER_STAGE_FLAG_HULL        : return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
    case buma3d::SHADER_STAGE_FLAG_DOMAIN      : return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
    case buma3d::SHADER_STAGE_FLAG_GEOMETRY    : return VK_SHADER_STAGE_GEOMETRY_BIT;
    case buma3d::SHADER_STAGE_FLAG_PIXEL       : return VK_SHADER_STAGE_FRAGMENT_BIT;
    case buma3d::SHADER_STAGE_FLAG_COMPUTE     : return VK_SHADER_STAGE_COMPUTE_BIT;
    case buma3d::SHADER_STAGE_FLAG_TASK        : return VK_SHADER_STAGE_TASK_BIT_NV;
    case buma3d::SHADER_STAGE_FLAG_MESH        : return VK_SHADER_STAGE_MESH_BIT_NV;
    case buma3d::SHADER_STAGE_FLAG_RAYGEN      : return VK_SHADER_STAGE_RAYGEN_BIT_KHR;
    case buma3d::SHADER_STAGE_FLAG_ANY_HIT     : return VK_SHADER_STAGE_ANY_HIT_BIT_KHR;
    case buma3d::SHADER_STAGE_FLAG_CLOSEST_HIT : return VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
    case buma3d::SHADER_STAGE_FLAG_MISS        : return VK_SHADER_STAGE_MISS_BIT_KHR;
    case buma3d::SHADER_STAGE_FLAG_INTERSECTION: return VK_SHADER_STAGE_INTERSECTION_BIT_KHR;
    case buma3d::SHADER_STAGE_FLAG_CALLABLE    : return VK_SHADER_STAGE_CALLABLE_BIT_KHR;

    default:
        return VkShaderStageFlagBits(-1);
    }
}

inline VkShaderStageFlags GetNativeShaderStageFlags(SHADER_STAGE_FLAGS _stages)
{
    VkShaderStageFlags result = 0;

    if (_stages & buma3d::SHADER_STAGE_FLAG_VERTEX)
        result |= VK_SHADER_STAGE_VERTEX_BIT;

    if (_stages & buma3d::SHADER_STAGE_FLAG_HULL)
        result |= VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;

    if (_stages & buma3d::SHADER_STAGE_FLAG_DOMAIN)
        result |= VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;

    if (_stages & buma3d::SHADER_STAGE_FLAG_GEOMETRY)
        result |= VK_SHADER_STAGE_GEOMETRY_BIT;

    if (_stages & buma3d::SHADER_STAGE_FLAG_PIXEL)
        result |= VK_SHADER_STAGE_FRAGMENT_BIT;

    if (_stages & buma3d::SHADER_STAGE_FLAG_COMPUTE)
        result |= VK_SHADER_STAGE_COMPUTE_BIT;

    if (_stages & buma3d::SHADER_STAGE_FLAG_TASK)
        result |= VK_SHADER_STAGE_TASK_BIT_NV;

    if (_stages & buma3d::SHADER_STAGE_FLAG_MESH)
        result |= VK_SHADER_STAGE_MESH_BIT_NV;

    if (_stages & buma3d::SHADER_STAGE_FLAG_RAYGEN)
        result |= VK_SHADER_STAGE_RAYGEN_BIT_KHR;

    if (_stages & buma3d::SHADER_STAGE_FLAG_ANY_HIT)
        result |= VK_SHADER_STAGE_ANY_HIT_BIT_KHR;

    if (_stages & buma3d::SHADER_STAGE_FLAG_CLOSEST_HIT)
        result |= VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;

    if (_stages & buma3d::SHADER_STAGE_FLAG_MISS)
        result |= VK_SHADER_STAGE_MISS_BIT_KHR;

    if (_stages & buma3d::SHADER_STAGE_FLAG_INTERSECTION)
        result |= VK_SHADER_STAGE_INTERSECTION_BIT_KHR;

    if (_stages & buma3d::SHADER_STAGE_FLAG_CALLABLE)
        result |= VK_SHADER_STAGE_CALLABLE_BIT_KHR;

    return result;
}

inline VkDescriptorType GetNativeDescriptorType(DESCRIPTOR_TYPE _type)
{
    switch (_type)
    {
    case buma3d::DESCRIPTOR_TYPE_CBV                        : return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    case buma3d::DESCRIPTOR_TYPE_INPUT_ATTACHMENT           : return VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    case buma3d::DESCRIPTOR_TYPE_SRV_TEXTURE                : return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    case buma3d::DESCRIPTOR_TYPE_SRV_TYPED_BUFFER           : return VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
    case buma3d::DESCRIPTOR_TYPE_SRV_BUFFER                 : return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    case buma3d::DESCRIPTOR_TYPE_UAV_TEXTURE                : return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    case buma3d::DESCRIPTOR_TYPE_UAV_TYPED_BUFFER           : return VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
    case buma3d::DESCRIPTOR_TYPE_UAV_BUFFER                 : return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    case buma3d::DESCRIPTOR_TYPE_SRV_ACCELERATION_STRUCTURE : return VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
    case buma3d::DESCRIPTOR_TYPE_SAMPLER                    : return VK_DESCRIPTOR_TYPE_SAMPLER;
    case buma3d::DESCRIPTOR_TYPE_CBV_DYNAMIC                : return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    case buma3d::DESCRIPTOR_TYPE_SRV_BUFFER_DYNAMIC         : return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
    case buma3d::DESCRIPTOR_TYPE_UAV_BUFFER_DYNAMIC         : return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;

    // VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER  
    // VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT
    default:
        return VkDescriptorType(-1);
    }
}

inline VkDescriptorBindingFlags GetNativeDescriptorFlags(DESCRIPTOR_FLAGS _flags)
{
    VkDescriptorBindingFlags result = 0;

    if (_flags & buma3d::DESCRIPTOR_FLAG_DESCRIPTORS_UPDATE_AFTER_BIND)
        result |= VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;

    if (_flags & buma3d::DESCRIPTOR_FLAG_PARTIALLY_BOUND)
        result |= VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;

    if (_flags & buma3d::DESCRIPTOR_FLAG_DESCRIPTORS_UPDATE_UNUSED_WHILE_PENDING)
        result |= VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT;

    return result;
}

inline VkDescriptorPoolCreateFlags GetNativeDescriptorPoolFlags(DESCRIPTOR_POOL_FLAGS _flags)
{
    VkDescriptorPoolCreateFlags result = 0;

    if (_flags & buma3d::DESCRIPTOR_POOL_FLAG_FREE_DESCRIPTOR_SET)
        result |= VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

    if (_flags & buma3d::DESCRIPTOR_POOL_FLAG_UPDATE_AFTER_BIND_POOL)
        result |= VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;

    return result;
}

inline VkQueryType GetNativeQueryHeapType(QUERY_HEAP_TYPE _type)
{
    switch (_type)
    {
    case buma3d::QUERY_HEAP_TYPE_OCCLUSION                                  : return VK_QUERY_TYPE_OCCLUSION;
    case buma3d::QUERY_HEAP_TYPE_TIMESTAMP                                  : return VK_QUERY_TYPE_TIMESTAMP;
    case buma3d::QUERY_HEAP_TYPE_PIPELINE_STATISTICS                        : return VK_QUERY_TYPE_PIPELINE_STATISTICS;
    case buma3d::QUERY_HEAP_TYPE_SO_STATISTICS                              : return VK_QUERY_TYPE_TRANSFORM_FEEDBACK_STREAM_EXT;
    case buma3d::QUERY_HEAP_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE      : return VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR;
    case buma3d::QUERY_HEAP_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_SIZE  : return VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_SIZE_KHR;

    case buma3d::QUERY_HEAP_TYPE_VIDEO_DECODE_STATISTICS:
    default:
        return VkQueryType(-1);
    }
}

inline VkQueryControlFlags GetNativeQueryFlags(QUERY_FLAGS _flags)
{
    VkQueryControlFlags result = 0;

    if (_flags & QUERY_FLAG_PRECISE_OCCLUSION)
        result |= VK_QUERY_CONTROL_PRECISE_BIT;

    return result;
}

inline bool IsTextureView(VIEW_DIMENSION _dim)
{
    switch (_dim)
    {
    case buma3d::VIEW_DIMENSION_TEXTURE_1D:
    case buma3d::VIEW_DIMENSION_TEXTURE_1D_ARRAY:
    case buma3d::VIEW_DIMENSION_TEXTURE_2D:
    case buma3d::VIEW_DIMENSION_TEXTURE_2D_ARRAY:
    case buma3d::VIEW_DIMENSION_TEXTURE_3D:
    case buma3d::VIEW_DIMENSION_TEXTURE_CUBE:
    case buma3d::VIEW_DIMENSION_TEXTURE_CUBE_ARRAY:
        return true;

    default:
        return false;
    }
}

inline VkDependencyFlags GetNativeDependencyFlags(DEPENDENCY_FLAGS _flags)
{
    VkDependencyFlags result = 0;

    if (_flags & buma3d::DEPENDENCY_FLAG_BY_REGION)
        result |= VK_DEPENDENCY_BY_REGION_BIT;

    if (_flags & buma3d::DEPENDENCY_FLAG_VIEW_LOCAL)
        result |= VK_DEPENDENCY_VIEW_LOCAL_BIT;

    return result;
}

inline VkPipelineBindPoint GetNativePipelineBindPoint(PIPELINE_BIND_POINT _bind_point)
{
    switch (_bind_point)
    {
    case buma3d::PIPELINE_BIND_POINT_GRAPHICS    : return VK_PIPELINE_BIND_POINT_GRAPHICS;
    case buma3d::PIPELINE_BIND_POINT_COMPUTE     : return VK_PIPELINE_BIND_POINT_COMPUTE;
    case buma3d::PIPELINE_BIND_POINT_RAY_TRACING : return VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR;

    default:
        return VkPipelineBindPoint(-1);
    }
}

inline VkViewport* ConvertNativeViewport(const VIEWPORT& _src_viewport, VkViewport* _dst_result)
{
    _dst_result->x        = _src_viewport.x;
    _dst_result->y        = _src_viewport.y;
    _dst_result->width    = _src_viewport.width;
    _dst_result->height   = _src_viewport.height;
    _dst_result->minDepth = _src_viewport.min_depth;
    _dst_result->maxDepth = _src_viewport.max_depth;

    return _dst_result;
}


}// namespace util
}// namespace buma3d


namespace buma3d
{
namespace util
{

inline util::String GetVulkanVersionString(uint32_t _version)
{
    util::StringStream ss;
    ss << VK_VERSION_MAJOR(_version) << "."
       << VK_VERSION_MINOR(_version) << "."
       << VK_VERSION_PATCH(_version);
    return ss.str();
}

util::String GetVkResultDescription(VkResult _vkr);
util::String GetVkResultDescriptionJP(VkResult _vkr);

BMRESULT GetBMResultFromVk(VkResult _vkr);

// NOTE: 各構造のpNext変数の型は const void* だったり void* だったりする。ポインタ渡しではconst void** に強制する。
// pNextチェインを接続します。
// 最後の構造のpNextポインタのポインタが帰ります
inline const void** ConnectPNextChains(const void** _next) { return _next; }
template<typename Next = const void*, typename Head, typename ...Tail>
inline const void** ConnectPNextChains(Next* _next, Head& _head, Tail&... _tail)
{
    *_next = (void*)&_head;
    return ConnectPNextChains((const void**)&_head.pNext, _tail...);
}
template<typename Head, typename ...Types>
inline const void** ConnectPNextChains(Head& _head, Types& ..._chains)
{
    return ConnectPNextChains((const void**)&_head.pNext, _chains...);
}

template<typename T>
inline void PrepareVkSharingMode(DeviceVk* _device, VkSharingMode _mode, T* _dst_info)
{
    // 他種キュー間で共有するかどうか
    if (_mode == VK_SHARING_MODE_CONCURRENT)
    {
        _dst_info->queueFamilyIndexCount = (uint32_t)_device->GetQueueFamilyIndices().size();
        _dst_info->pQueueFamilyIndices = _device->GetQueueFamilyIndices().data();
    }
    else
    {
        _dst_info->queueFamilyIndexCount = 0;
        _dst_info->pQueueFamilyIndices = nullptr;
    }
}

// VkPhysicalDeviceProperties2::pNextにセット可能な構造体セット
struct PHYSICAL_DEVICE_PROPERTIES_CHAIN
{
    //typedef VkPhysicalDeviceMultiviewProperties           VkPhysicalDeviceMultiviewPropertiesKHR;
    //typedef VkPhysicalDeviceProperties2                   VkPhysicalDeviceProperties2KHR;
    //typedef VkPhysicalDeviceMemoryProperties2             VkPhysicalDeviceMemoryProperties2KHR;
    //typedef VkPhysicalDeviceGroupProperties               VkPhysicalDeviceGroupPropertiesKHR;
    //typedef VkPhysicalDeviceIDProperties                  VkPhysicalDeviceIDPropertiesKHR;
    //typedef VkPhysicalDevicePointClippingProperties       VkPhysicalDevicePointClippingPropertiesKHR;
    //typedef VkPhysicalDeviceMaintenance3Properties        VkPhysicalDeviceMaintenance3PropertiesKHR;
    //typedef VkPhysicalDeviceDriverProperties              VkPhysicalDeviceDriverPropertiesKHR;
    //typedef VkPhysicalDeviceFloatControlsProperties       VkPhysicalDeviceFloatControlsPropertiesKHR;
    //typedef VkPhysicalDeviceDepthStencilResolveProperties VkPhysicalDeviceDepthStencilResolvePropertiesKHR;
    //typedef VkPhysicalDeviceTimelineSemaphoreProperties   VkPhysicalDeviceTimelineSemaphorePropertiesKHR;
    //typedef VkPhysicalDeviceSamplerFilterMinmaxProperties VkPhysicalDeviceSamplerFilterMinmaxPropertiesEXT;
    //typedef VkPhysicalDeviceDescriptorIndexingProperties  VkPhysicalDeviceDescriptorIndexingPropertiesEXT;

    util::UniquePtr<VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT     > blend_operation_advanced_props_ext;
    util::UniquePtr<VkPhysicalDeviceConservativeRasterizationPropertiesEXT  > conservative_rasterization_props_ext;
    util::UniquePtr<VkPhysicalDeviceCooperativeMatrixPropertiesNV           > cooperative_matrix_props_nv;
    util::UniquePtr<VkPhysicalDeviceDeviceGeneratedCommandsPropertiesNV     > device_generated_commands_props_nv;
    util::UniquePtr<VkPhysicalDeviceDiscardRectanglePropertiesEXT           > discard_rectangle_props_ext;
    util::UniquePtr<VkPhysicalDeviceExternalMemoryHostPropertiesEXT         > external_memory_host_props_ext;
    util::UniquePtr<VkPhysicalDeviceFragmentDensityMapPropertiesEXT         > fragment_density_map_props_ext;
    util::UniquePtr<VkPhysicalDeviceInlineUniformBlockPropertiesEXT         > inline_uniform_block_props_ext;
    util::UniquePtr<VkPhysicalDeviceLineRasterizationPropertiesEXT          > line_rasterization_props_ext;
    util::UniquePtr<VkPhysicalDeviceMeshShaderPropertiesNV                  > mesh_shader_props_nv;
    util::UniquePtr<VkPhysicalDeviceMultiviewPerViewAttributesPropertiesNVX > multiview_per_view_attributes_props_nvx;
    util::UniquePtr<VkPhysicalDevicePCIBusInfoPropertiesEXT                 > pci_bus_info_props_ext;
    util::UniquePtr<VkPhysicalDevicePerformanceQueryPropertiesKHR           > performance_query_props_khr;
    util::UniquePtr<VkPhysicalDevicePushDescriptorPropertiesKHR             > push_descriptor_props_khr;
    util::UniquePtr<VkPhysicalDeviceRayTracingPropertiesKHR                 > ray_tracing_props_khr;
    util::UniquePtr<VkPhysicalDeviceRayTracingPropertiesNV                  > ray_tracing_props_nv;
    util::UniquePtr<VkPhysicalDeviceSampleLocationsPropertiesEXT            > sample_locations_props_ext;
    util::UniquePtr<VkPhysicalDeviceShaderCoreProperties2AMD                > shader_core_props2_amd;
    util::UniquePtr<VkPhysicalDeviceShaderCorePropertiesAMD                 > shader_core_props_amd;
    util::UniquePtr<VkPhysicalDeviceShaderSMBuiltinsPropertiesNV            > shader_sm_builtins_props_nv;
    util::UniquePtr<VkPhysicalDeviceShadingRateImagePropertiesNV            > shading_rate_image_props_nv;
    util::UniquePtr<VkPhysicalDeviceSubgroupSizeControlPropertiesEXT        > subgroup_size_control_props_ext;
    util::UniquePtr<VkPhysicalDeviceTexelBufferAlignmentPropertiesEXT       > texel_buffer_alignment_props_ext;
    util::UniquePtr<VkPhysicalDeviceTransformFeedbackPropertiesEXT          > transform_feedback_props_ext;
    util::UniquePtr<VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT     > vertex_attribute_divisor_props_ext;

    // vulkan11
    struct PROPS_11
    {
        util::UniquePtr<VkPhysicalDeviceIDProperties                > id_props;
        util::UniquePtr<VkPhysicalDeviceSubgroupProperties          > subgroup_props;
        util::UniquePtr<VkPhysicalDevicePointClippingProperties     > point_clipping_props;
        util::UniquePtr<VkPhysicalDeviceMultiviewProperties         > multiview_props;
        util::UniquePtr<VkPhysicalDeviceProtectedMemoryProperties   > protected_memory_props;
        util::UniquePtr<VkPhysicalDeviceMaintenance3Properties      > maintenance3_props;
    };
    util::UniquePtr<PROPS_11> props11;

    // vulkan12
    struct PROPS_12
    {
        util::UniquePtr<VkPhysicalDeviceDriverProperties               > driver_props;
        util::UniquePtr<VkPhysicalDeviceFloatControlsProperties        > float_controls_props;
        util::UniquePtr<VkPhysicalDeviceDescriptorIndexingProperties   > descriptor_indexing_props;
        util::UniquePtr<VkPhysicalDeviceDepthStencilResolveProperties  > depth_stencil_resolve_props;
        util::UniquePtr<VkPhysicalDeviceSamplerFilterMinmaxProperties  > sampler_filter_minmax_props;
        util::UniquePtr<VkPhysicalDeviceTimelineSemaphoreProperties    > timeline_semaphore_props;
    };
    util::UniquePtr<PROPS_12> props12;

    // vulkan1.2からこれらの構造体で一気に取得できる
    // これらの構造を使用する場合上の構造はpNextに含まれてはいけない
    util::UniquePtr<VkPhysicalDeviceVulkan11Properties> vulkan11_props;
    util::UniquePtr<VkPhysicalDeviceVulkan12Properties> vulkan12_props;

};

struct PHYSICAL_DEVICE_FEATURES_CHAIN
{
    //typedef VkPhysicalDeviceVariablePointersFeatures            VkPhysicalDeviceVariablePointerFeatures;
    //typedef VkPhysicalDeviceShaderDrawParametersFeatures        VkPhysicalDeviceShaderDrawParameterFeatures;
    //typedef VkPhysicalDeviceMultiviewFeatures                   VkPhysicalDeviceMultiviewFeaturesKHR;
    //typedef VkPhysicalDeviceFeatures2                           VkPhysicalDeviceFeatures2KHR;
    //typedef VkPhysicalDeviceShaderFloat16Int8Features           VkPhysicalDeviceShaderFloat16Int8FeaturesKHR;
    //typedef VkPhysicalDeviceShaderFloat16Int8Features           VkPhysicalDeviceFloat16Int8FeaturesKHR;
    //typedef VkPhysicalDevice16BitStorageFeatures                VkPhysicalDevice16BitStorageFeaturesKHR;
    //typedef VkPhysicalDeviceImagelessFramebufferFeatures        VkPhysicalDeviceImagelessFramebufferFeaturesKHR;
    //typedef VkPhysicalDeviceVariablePointersFeatures            VkPhysicalDeviceVariablePointerFeaturesKHR;
    //typedef VkPhysicalDeviceVariablePointersFeatures            VkPhysicalDeviceVariablePointersFeaturesKHR;
    //typedef VkPhysicalDeviceSamplerYcbcrConversionFeatures      VkPhysicalDeviceSamplerYcbcrConversionFeaturesKHR;
    //typedef VkPhysicalDeviceShaderSubgroupExtendedTypesFeatures VkPhysicalDeviceShaderSubgroupExtendedTypesFeaturesKHR;
    //typedef VkPhysicalDevice8BitStorageFeatures                 VkPhysicalDevice8BitStorageFeaturesKHR;
    //typedef VkPhysicalDeviceShaderAtomicInt64Features           VkPhysicalDeviceShaderAtomicInt64FeaturesKHR;
    //typedef VkPhysicalDeviceTimelineSemaphoreFeatures           VkPhysicalDeviceTimelineSemaphoreFeaturesKHR;
    //typedef VkPhysicalDeviceVulkanMemoryModelFeatures           VkPhysicalDeviceVulkanMemoryModelFeaturesKHR;
    //typedef VkPhysicalDeviceSeparateDepthStencilLayoutsFeatures VkPhysicalDeviceSeparateDepthStencilLayoutsFeaturesKHR;
    //typedef VkPhysicalDeviceUniformBufferStandardLayoutFeatures VkPhysicalDeviceUniformBufferStandardLayoutFeaturesKHR;
    //typedef VkPhysicalDeviceBufferDeviceAddressFeatures         VkPhysicalDeviceBufferDeviceAddressFeaturesKHR;
    //typedef VkPhysicalDeviceDescriptorIndexingFeatures          VkPhysicalDeviceDescriptorIndexingFeaturesEXT;
    //typedef VkPhysicalDeviceScalarBlockLayoutFeatures           VkPhysicalDeviceScalarBlockLayoutFeaturesEXT;
    //typedef VkPhysicalDeviceBufferDeviceAddressFeaturesEXT      VkPhysicalDeviceBufferAddressFeaturesEXT;
    //typedef VkPhysicalDeviceHostQueryResetFeatures              VkPhysicalDeviceHostQueryResetFeaturesEXT;

    util::UniquePtr<VkPhysicalDeviceASTCDecodeFeaturesEXT                      > astc_decode_features_ext;
    util::UniquePtr<VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT          > blend_operation_advanced_features_ext;
    util::UniquePtr<VkPhysicalDeviceBufferDeviceAddressFeaturesEXT             > buffer_device_address_features_ext;// vulkan1.2から: KHR拡張(と標準機能)に昇格し、非推奨
    util::UniquePtr<VkPhysicalDeviceCoherentMemoryFeaturesAMD                  > coherent_memory_features_amd;
    util::UniquePtr<VkPhysicalDeviceComputeShaderDerivativesFeaturesNV         > compute_shader_derivatives_features_nv;
    util::UniquePtr<VkPhysicalDeviceConditionalRenderingFeaturesEXT            > conditional_rendering_features_ext;
    util::UniquePtr<VkPhysicalDeviceCooperativeMatrixFeaturesNV                > cooperative_matrix_features_nv;
    util::UniquePtr<VkPhysicalDeviceCornerSampledImageFeaturesNV               > corner_sampled_image_features_nv;
    util::UniquePtr<VkPhysicalDeviceCoverageReductionModeFeaturesNV            > coverage_reduction_mode_features_nv;
    util::UniquePtr<VkPhysicalDeviceDedicatedAllocationImageAliasingFeaturesNV > dedicated_allocation_image_aliasing_features_nv;
    util::UniquePtr<VkPhysicalDeviceDepthClipEnableFeaturesEXT                 > depth_clip_enable_features_ext;
    util::UniquePtr<VkPhysicalDeviceDeviceGeneratedCommandsFeaturesNV          > device_generated_commands_features_nv;
    util::UniquePtr<VkPhysicalDeviceDiagnosticsConfigFeaturesNV                > diagnostics_config_features_nv;
    util::UniquePtr<VkPhysicalDeviceExclusiveScissorFeaturesNV                 > exclusive_scissor_features_nv;
    util::UniquePtr<VkPhysicalDeviceFragmentDensityMapFeaturesEXT              > fragment_density_map_features_ext;
    util::UniquePtr<VkPhysicalDeviceFragmentShaderBarycentricFeaturesNV        > fragment_shader_barycentric_features_nv;
    util::UniquePtr<VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT         > fragment_shader_interlock_features_ext;
    util::UniquePtr<VkPhysicalDeviceIndexTypeUint8FeaturesEXT                  > index_type_uint8_features_ext;
    util::UniquePtr<VkPhysicalDeviceInlineUniformBlockFeaturesEXT              > inline_uniform_block_features_ext;
    util::UniquePtr<VkPhysicalDeviceLineRasterizationFeaturesEXT               > line_rasterization_features_ext;
    util::UniquePtr<VkPhysicalDeviceMemoryPriorityFeaturesEXT                  > memory_priority_features_ext;
    util::UniquePtr<VkPhysicalDeviceMeshShaderFeaturesNV                       > mesh_shader_features_nv;
    util::UniquePtr<VkPhysicalDevicePerformanceQueryFeaturesKHR                > performance_query_features_khr;
    util::UniquePtr<VkPhysicalDevicePipelineCreationCacheControlFeaturesEXT    > pipeline_creation_cache_control_features_ext;
    util::UniquePtr<VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR    > pipeline_executable_properties_features_khr;
    util::UniquePtr<VkPhysicalDeviceRayTracingFeaturesKHR                      > ray_tracing_features_khr;
    util::UniquePtr<VkPhysicalDeviceRepresentativeFragmentTestFeaturesNV       > representative_fragment_test_features_nv;
    util::UniquePtr<VkPhysicalDeviceShaderClockFeaturesKHR                     > shader_clock_features_khr;
    util::UniquePtr<VkPhysicalDeviceShaderDemoteToHelperInvocationFeaturesEXT  > shader_demote_to_helper_invocation_features_ext;
    util::UniquePtr<VkPhysicalDeviceShaderImageFootprintFeaturesNV             > shader_image_footprint_features_nv;
    util::UniquePtr<VkPhysicalDeviceShaderIntegerFunctions2FeaturesINTEL       > shader_integer_functions2_features_intel;
    util::UniquePtr<VkPhysicalDeviceShaderSMBuiltinsFeaturesNV                 > shader_sm_builtins_features_nv;
    util::UniquePtr<VkPhysicalDeviceShadingRateImageFeaturesNV                 > shading_rate_image_features_nv;
    util::UniquePtr<VkPhysicalDeviceSubgroupSizeControlFeaturesEXT             > subgroup_size_control_features_ext;
    util::UniquePtr<VkPhysicalDeviceTexelBufferAlignmentFeaturesEXT            > texel_buffer_alignment_features_ext;
    util::UniquePtr<VkPhysicalDeviceTextureCompressionASTCHDRFeaturesEXT       > texture_compression_astc_hdr_features_ext;
    util::UniquePtr<VkPhysicalDeviceTransformFeedbackFeaturesEXT               > transform_feedback_features_ext;
    util::UniquePtr<VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT          > vertex_attribute_divisor_features_ext;
    util::UniquePtr<VkPhysicalDeviceYcbcrImageArraysFeaturesEXT                > ycbcr_image_arrays_features_ext;

    // vulkan11
    struct FEATURES_VK11
    {
        util::UniquePtr<VkPhysicalDevice16BitStorageFeatures              > bit16_storage_features;
        util::UniquePtr<VkPhysicalDeviceMultiviewFeatures                 > multiview_features;
        util::UniquePtr<VkPhysicalDeviceVariablePointersFeatures          > variable_pointers_features;
        util::UniquePtr<VkPhysicalDeviceProtectedMemoryFeatures           > protected_memory_features;
        util::UniquePtr<VkPhysicalDeviceSamplerYcbcrConversionFeatures    > sampler_ycbcr_conversion_features;
        util::UniquePtr<VkPhysicalDeviceShaderDrawParametersFeatures      > shader_draw_parameters_features;
    };
    util::UniquePtr<FEATURES_VK11> features11;

    // vulkan12
    struct FEATURES_VK12
    {
        util::UniquePtr<VkPhysicalDevice8BitStorageFeatures                   > bit8_storage_features;
        util::UniquePtr<VkPhysicalDeviceShaderAtomicInt64Features             > shader_atomic_int64_features;
        util::UniquePtr<VkPhysicalDeviceShaderFloat16Int8Features             > shader_float16_int8_features;
        util::UniquePtr<VkPhysicalDeviceDescriptorIndexingFeatures            > descriptor_indexing_features;
        util::UniquePtr<VkPhysicalDeviceScalarBlockLayoutFeatures             > scalar_block_layout_features;
        util::UniquePtr<VkPhysicalDeviceImagelessFramebufferFeatures          > imageless_framebuffer_features;
        util::UniquePtr<VkPhysicalDeviceUniformBufferStandardLayoutFeatures   > uniform_buffer_standard_layout_features;
        util::UniquePtr<VkPhysicalDeviceShaderSubgroupExtendedTypesFeatures   > shader_subgroup_extended_types_features;
        util::UniquePtr<VkPhysicalDeviceSeparateDepthStencilLayoutsFeatures   > separate_depth_stencil_layouts_features;
        util::UniquePtr<VkPhysicalDeviceHostQueryResetFeatures                > host_query_reset_features;
        util::UniquePtr<VkPhysicalDeviceTimelineSemaphoreFeatures             > timeline_semaphore_features;
        util::UniquePtr<VkPhysicalDeviceBufferDeviceAddressFeatures           > buffer_device_address_features;
        util::UniquePtr<VkPhysicalDeviceVulkanMemoryModelFeatures             > vulkan_memory_model_features;
    };
    util::UniquePtr<FEATURES_VK12> features12;

    // vulkan1.2から上のの構造体で一気に取得できる
    // これらの構造を使用する場合上の構造はpNextに含まれてはいけない
    util::UniquePtr<VkPhysicalDeviceVulkan11Features> vulkan11_features;
    util::UniquePtr<VkPhysicalDeviceVulkan12Features> vulkan12_features;

};

struct PHYSICAL_DEVICE_DATA
{
    VkPhysicalDeviceFeatures2               features2;
    util::PHYSICAL_DEVICE_FEATURES_CHAIN    features_chain;

    VkPhysicalDeviceProperties2             properties2;
    util::PHYSICAL_DEVICE_PROPERTIES_CHAIN  properties_chain;
};

struct DEVICE_DATA
{
    util::UniquePtr<util::DyArray<VkTimeDomainEXT>>               time_domain_exts;
    util::UniquePtr<util::DyArray<VkCalibratedTimestampInfoEXT>>  calibrated_timestamp_info_exts;

    util::UniquePtr<util::DyArray<VkPhysicalDeviceToolPropertiesEXT>> tool_properties_exts;
};

struct SURFACE_DATA
{
    VkPhysicalDeviceSurfaceInfo2KHR             surface_info2_khr{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SURFACE_INFO_2_KHR };

#ifdef VK_USE_PLATFORM_WIN32_KHR
    VkSurfaceFullScreenExclusiveInfoEXT         full_screen_exclusive_info_ext       { VK_STRUCTURE_TYPE_SURFACE_FULL_SCREEN_EXCLUSIVE_INFO_EXT };
    VkSurfaceFullScreenExclusiveWin32InfoEXT    full_screen_exclusive_win32_info_ext { VK_STRUCTURE_TYPE_SURFACE_FULL_SCREEN_EXCLUSIVE_WIN32_INFO_EXT };
#endif
    util::DyArray<VkSurfaceFormat2KHR>          formats2_khr;// VK_STRUCTURE_TYPE_SURFACE_FORMAT_2_KHR
    //util::DyArray<VkPresentModeKHR>           present_mode_khr;// CHANGED: プラットフォームによってはクエリにVkDeviceが必要な場合もあるため、SwapChainVkで逐次取得する。

    VkSurfaceCapabilities2KHR                   capa2_khr                           { VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_2_KHR };
    VkSurfaceProtectedCapabilitiesKHR           protected_capa_khr                  { VK_STRUCTURE_TYPE_SURFACE_PROTECTED_CAPABILITIES_KHR };
    VkSurfaceCapabilitiesFullScreenExclusiveEXT capa_full_screen_exclusive_ext      { VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_FULL_SCREEN_EXCLUSIVE_EXT };
    VkSharedPresentSurfaceCapabilitiesKHR       shared_present_surface_capa_khr     { VK_STRUCTURE_TYPE_SHARED_PRESENT_SURFACE_CAPABILITIES_KHR };
    VkDisplayNativeHdrSurfaceCapabilitiesAMD    display_native_hdr_surface_capa_amd { VK_STRUCTURE_TYPE_DISPLAY_NATIVE_HDR_SURFACE_CAPABILITIES_AMD };
};

struct SWAP_CHAIN_DATA
{
    VkDeviceGroupPresentCapabilitiesKHR device_group_present_capa_khr{ VK_STRUCTURE_TYPE_DEVICE_GROUP_PRESENT_CAPABILITIES_KHR };
    VkDeviceGroupPresentModeFlagsKHR    device_group_present_mode_flags_khr{};
    util::DyArray<VkRect2D>             local_multi_device_rects;
};

class MemoryProperties
{
    friend class DeviceAdapterVk;
    friend class DeviceVk;
    void Init(VkPhysicalDevice _pd);
public:
    MemoryProperties() {}
    ~MemoryProperties() {}

    void UpdateBudgetProperties(VkPhysicalDevice _pd);

    /**
     * @brief 要求されたメモリプロパティに対応するVkMemoryTypeへのインデックスを返します。
     * @param _required_memory_type_bits vkGet Image/BUffer MemoryRequirements によって返される、指定のイメージまたはバッファに必要なメモリタイプのビットインデックス(場合によって複数指定されている)。
     * @param _required_props アプリケーションによって要求されるメモリプロパティのフラグ。
     * @return _required_memory_type_bitsに設定されているいずれかのビットのインデックス(小さいインデックスが優先して返ります。)。 _required_propsに対応するメモリタイプが存在しない場合、-1が返ります。
    */
    int32_t FindProperties(uint32_t _required_memory_type_bits, VkMemoryPropertyFlags _required_memory_props) const;

    const VkPhysicalDeviceMemoryProperties2&         GetVkMemoryProperties2()         const { return mem_props; }
    const VkPhysicalDeviceMemoryBudgetPropertiesEXT& GetVkMemoryBudgetPropertiesEXT() const { return budget_props; }

private:
    VkPhysicalDeviceMemoryProperties2   mem_props;
    VkPhysicalDeviceMemoryBudgetPropertiesEXT budget_props;

};


}// namespace util
}// namespace buma3d

namespace buma3d
{
namespace hlp
{

inline bool IsSucceedVk(VkResult _vkr)
{
    return _vkr >= VK_SUCCESS;
}

inline bool IsFailedVk(VkResult _vkr)
{
    return _vkr <= VK_ERROR_OUT_OF_HOST_MEMORY;
}

inline bool IsSucceedVk(VkResult _vkr, VkResult& _dst)
{
    _dst = _vkr;
    return _vkr >= VK_SUCCESS;
}

inline bool IsFailedVk(VkResult _vkr, VkResult& _dst)
{
    _dst = _vkr;
    return _vkr <= VK_ERROR_OUT_OF_HOST_MEMORY;
}

inline bool IsSucceedVk(VkResult _vkr, VkResult& _dst, util::String& _dst_message)
{
    _dst = _vkr;
    _dst_message = util::GetVkResultDescriptionJP(_vkr);
    return _vkr >= VK_SUCCESS;
}

inline bool IsFailedVk(VkResult _vkr, VkResult& _dst, util::String& _dst_message)
{
    _dst = _vkr;
    _dst_message = util::GetVkResultDescriptionJP(_vkr);
    return _vkr <= VK_ERROR_OUT_OF_HOST_MEMORY;
}


}// namespace hlp
}// namespace buma3d


namespace buma3d
{
namespace util
{

template<typename T>
inline bool IsEnabledDebug(T* _b3d_obj)
{
    if constexpr (std::is_base_of_v<IDeviceChild, T>)
    {
        auto dev = _b3d_obj->GetDevice()->As<DeviceVk>();
        return dev->IsEnabledDebug();
    }
    else if constexpr (std::is_base_of_v<ISharedBase, T>)
    {
        return _b3d_obj->IsEnabledDebug();
    }
    else
    {
        B3D_UNREFERENCED(_b3d_obj);
        static_assert(false, "T is not B3DAPI");
    }
}

template<typename T>
inline void AddDebugMessage(T* _b3d_obj, DEBUG_MESSAGE_SEVERITY _severity, DEBUG_MESSAGE_CATEGORY_FLAG _category, const Char8T* _str)
{
    if constexpr (!IS_ENABLE_DEBUG_OUTPUT)
    {
        B3D_UNREFERENCED(_b3d_obj, _severity, _category, _str);
        return;
    }

    if constexpr (std::is_base_of_v<IDeviceChild, T>)
    {
        auto dev = _b3d_obj->GetDevice()->As<DeviceVk>();
        if (dev->IsEnabledDebug())
        {
            dev->AddMessageFromB3D(_severity, _category, _str);
        }
    }
    else if constexpr (std::is_base_of_v<ISharedBase, T>)
    {
        if (_b3d_obj->IsEnabledDebug())
        {
            _b3d_obj->AddMessageFromB3D(_severity, _category, _str);
        }
    }
}

template<typename T>
inline void VkRAddDebugMessageIfFailed(T* _b3d_obj, VkResult _vkr, const char* _file, int _line)
{
    if constexpr (IS_ENABLE_DEBUG_OUTPUT)
    {
        if (hlp::IsFailedVk(_vkr))
        {
            util::StringStream ss;
            ss << "VkResult: Failed: " << buma3d::util::GetVkResultDescriptionJP(_vkr) << " , File: " << _file << ", Line: " << _line;
            _b3d_obj->AddMessageFromB3D(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_MISCELLANEOUS, ss.str().c_str());
        }
    }
    else
    {
        B3D_UNREFERENCED(_b3d_obj, _vkr, _file, _line);
    }
}

inline void VkRTraceIfFailed(VkResult _vkr, const char* _file, int _line)
{
    if constexpr (IS_ENABLE_DEBUG_OUTPUT)
    {
        if (_vkr > VK_SUCCESS)
        {
            util::StringStream ss;
            ss << "VkResult: " << buma3d::util::GetVkResultDescriptionJP(_vkr) << " , File: " << _file << ", Line: " << _line;
            buma3d::hlp::OutDebugStr(ss.str().c_str());
        }
        else if (_vkr <= VK_ERROR_OUT_OF_HOST_MEMORY)
        {
            util::StringStream ss;
            ss << "VkResult Failed: " << buma3d::util::GetVkResultDescriptionJP(_vkr) << " , File: " << _file << ", Line: " << _line;
            buma3d::hlp::OutDebugStr(ss.str().c_str());
        }
    }
    else
    {
        B3D_UNREFERENCED(_vkr, _file, _line);
    }
}

template<typename T>
inline BMRESULT VkRCheckResult(T* _b3d_obj, VkResult _vkr, const char* _file, int _line)
{
    if constexpr (!std::is_base_of_v<ISharedBase, T>)
    {
        B3D_UNREFERENCED(_b3d_obj);
        buma3d::util::VkRTraceIfFailed(_vkr, _file, _line);
    }
    else if constexpr (std::is_base_of_v<IDeviceChild, T>)
    {
        auto dev = _b3d_obj->GetDevice()->As<DeviceVk>();
        if (dev->IsEnabledDebug())
        {
            buma3d::util::VkRAddDebugMessageIfFailed(dev, _vkr, _file, _line);
        }
    }
    else
    {
        if (_b3d_obj->IsEnabledDebug())
        {
            buma3d::util::VkRAddDebugMessageIfFailed(_b3d_obj, _vkr, _file, _line);
        }
    }
    return util::GetBMResultFromVk(_vkr);
}


}// namespace util
}// namespace buma3d
