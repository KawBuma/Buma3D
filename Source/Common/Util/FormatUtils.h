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
    UINT2 result;
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

bool IsIntegerFormat(RESOURCE_FORMAT _format);
bool IsSintFormat(RESOURCE_FORMAT _format);
bool IsUintFormat(RESOURCE_FORMAT _format);
bool IsSrgbFormat(RESOURCE_FORMAT _format);


}// namespace util
}// namespace buma3d
