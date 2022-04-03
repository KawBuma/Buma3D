#pragma once

namespace buma3d
{
namespace util
{

size_t GetFormatSize(RESOURCE_FORMAT _format);
size_t GetDepthOrStencilFormatSize(RESOURCE_FORMAT _format, bool _is_stencil);

void GetFormatBlockSize(RESOURCE_FORMAT _format, uint32_t* _dst_block_w, uint32_t* _dst_block_h);

inline UINT2 GetFormatBlockSize(RESOURCE_FORMAT _format)
{
    UINT2 result{};
    GetFormatBlockSize(_format, &result.x, &result.y);
    return result;
}

inline size_t CalcTexelsPerBlock(RESOURCE_FORMAT _format)
{
    UINT2 b{};
    GetFormatBlockSize(_format, &b.x, &b.y);
    return static_cast<size_t>(b.x * b.y);
}

bool IsTypelessFormat(RESOURCE_FORMAT _format);
RESOURCE_FORMAT GetTypelessFormat(RESOURCE_FORMAT _format);

bool IsDepthStencilFormat(RESOURCE_FORMAT _format);
bool IsDepthOnlyFormat(RESOURCE_FORMAT _format);

bool IsMultiplanarFormat(RESOURCE_FORMAT _format);
size_t GetPlaneCount(RESOURCE_FORMAT _format);

bool IsIntegerFormat(RESOURCE_FORMAT _format);
bool IsSintFormat(RESOURCE_FORMAT _format);
bool IsUintFormat(RESOURCE_FORMAT _format);
bool IsSrgbFormat(RESOURCE_FORMAT _format);

RESOURCE_FORMAT GetSrgbFormat(buma3d::RESOURCE_FORMAT _non_srgb_format);
RESOURCE_FORMAT RemoveSrgbFormat(buma3d::RESOURCE_FORMAT _format);


#pragma region implementation

struct TEXTURE_PROPERTIES
{
    const RESOURCE_DESC&    desc; // from ITexture
    bool                    is_depth_stencil_format;
    bool                    is_depth_only_format;
    bool                    is_multiplanar_format;
    TEXTURE_ASPECT_FLAGS    aspect; // このテクスチャ全体でのアスペクトです

    void Setup()
    {
        is_depth_stencil_format = util::IsDepthStencilFormat(desc.texture.format_desc.format);
        is_depth_only_format    = util::IsDepthOnlyFormat(desc.texture.format_desc.format);
        is_multiplanar_format   = util::IsMultiplanarFormat(desc.texture.format_desc.format);

        if (is_depth_stencil_format)
        {
            aspect = is_depth_only_format
                ? TEXTURE_ASPECT_FLAG_DEPTH
                : TEXTURE_ASPECT_FLAG_DEPTH | TEXTURE_ASPECT_FLAG_STENCIL;
        }
        else if (is_multiplanar_format)
        {
            auto plane_count = util::GetPlaneCount(desc.texture.format_desc.format);
            switch (plane_count)
            {
            case 1: aspect = TEXTURE_ASPECT_FLAG_PLANE_0;                                                               break;
            case 2: aspect = TEXTURE_ASPECT_FLAG_PLANE_0 | TEXTURE_ASPECT_FLAG_PLANE_1;                                 break;
            case 3: aspect = TEXTURE_ASPECT_FLAG_PLANE_0 | TEXTURE_ASPECT_FLAG_PLANE_1 | TEXTURE_ASPECT_FLAG_PLANE_2;   break;
            default:
                break;
            }
        }
        else
        {
            aspect = TEXTURE_ASPECT_FLAG_COLOR;
        }
    }

    bool IsAllSubresources(const SUBRESOURCE_RANGE& _range) const
    {
        return
            aspect == _range.offset.aspect &&
            _range.offset.mip_slice == 0 && _range.offset.array_slice == 0 && 
            (_range.mip_levels == B3D_USE_REMAINING_MIP_LEVELS  || desc.texture.mip_levels == _range.mip_levels) &&
            (_range.array_size == B3D_USE_REMAINING_ARRAY_SIZES || desc.texture.array_size == _range.array_size);
    }
};

#pragma endregion implementation


}// namespace util
}// namespace buma3d
