#pragma once

namespace buma3d
{
namespace util
{

DXGI_COLOR_SPACE_TYPE GetNativeColorSpace(COLOR_SPACE _cs);
COLOR_SPACE GetB3DColorSpace(DXGI_COLOR_SPACE_TYPE _cs);

DXGI_FORMAT GetNativeFormat(RESOURCE_FORMAT _format);
RESOURCE_FORMAT GetB3DFormat(DXGI_FORMAT _format);

inline DXGI_FORMAT ConvertDepthStencilFormat(RESOURCE_FORMAT _format, TEXTURE_ASPECT_FLAGS _aspect)
{
    bool is_stencil_plane = _aspect & TEXTURE_ASPECT_FLAG_STENCIL;
    switch (_format)
    {
    case RESOURCE_FORMAT_D16_UNORM            : return is_stencil_plane ? DXGI_FORMAT_UNKNOWN                 : DXGI_FORMAT_R16_UNORM;
    case RESOURCE_FORMAT_D32_FLOAT            : return is_stencil_plane ? DXGI_FORMAT_UNKNOWN                 : DXGI_FORMAT_R32_FLOAT;
    case RESOURCE_FORMAT_D24_UNORM_S8_UINT    : return is_stencil_plane ? DXGI_FORMAT_X24_TYPELESS_G8_UINT    : DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
    case RESOURCE_FORMAT_D32_FLOAT_S8X24_UINT : return is_stencil_plane ? DXGI_FORMAT_X32_TYPELESS_G8X24_UINT : DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;

    default:
        return DXGI_FORMAT_UNKNOWN;
    }
}

inline DXGI_FORMAT GetNativeDepthStencilTypelessFormat(RESOURCE_FORMAT _format)
{
    switch (_format)
    {
    case RESOURCE_FORMAT_D16_UNORM            : return DXGI_FORMAT_R16_TYPELESS;
    case RESOURCE_FORMAT_D32_FLOAT            : return DXGI_FORMAT_R32_TYPELESS;
    case RESOURCE_FORMAT_D24_UNORM_S8_UINT    : return DXGI_FORMAT_R24G8_TYPELESS;
    case RESOURCE_FORMAT_D32_FLOAT_S8X24_UINT : return DXGI_FORMAT_R32G8X24_TYPELESS;

    default:
        return DXGI_FORMAT_UNKNOWN;
    }
}

struct FORMAT_FEATURE_DATA
{
    D3D12_FEATURE_DATA_FORMAT_SUPPORT support;
    D3D12_FEATURE_DATA_FORMAT_INFO    info;
};

struct FORMAT_PROPERTIES_D3D12
{
    RESOURCE_FORMAT                  b3d_format;
    const util::FORMAT_FEATURE_DATA* format_props;
};

class FormatPropertiesD3D12
{
public:
    inline static constexpr int FORMAT_OFFSET = 1 + static_cast<int>(DXGI_FORMAT_UNKNOWN);

    // WARNING: これらの値はVulkanのフォーマットの追加により変動する可能性があります。
    inline static constexpr int FORMAT_MAX = static_cast<int>(DXGI_FORMAT_B4G4R4A4_UNORM + 1);

public:
    FormatPropertiesD3D12();
    ~FormatPropertiesD3D12();

    void Init(DeviceD3D12* _device);
    const util::UnordMap<DXGI_FORMAT, util::SharedPtr<FORMAT_PROPERTIES_D3D12>>& GetFormatsProperties() const { return formats_props; }

private:
    void PrepareAndEvaluateFormatProperties(DXGI_FORMAT _format, DeviceD3D12* _device);
    void CreateProperties(util::SharedPtr<FORMAT_PROPERTIES_D3D12>& _props);
    void GetFormatProperties(DXGI_FORMAT _format, FORMAT_PROPERTIES_D3D12* _dst_format_props, DeviceD3D12* _device);
    bool HasSupportedAnyFeature(FORMAT_PROPERTIES_D3D12* _props);

private:
    util::UnordMap<DXGI_FORMAT, util::SharedPtr<FORMAT_PROPERTIES_D3D12>> formats_props;

};

struct TYPELESS_COMPATIBLE_FORMATS
{
    util::SharedPtr<util::DyArray<RESOURCE_FORMAT>> compatible_formats;  // この配列はTEXTURE_FORMAT_DESCにデフォルトが指定された場合にTextureVk::DESC_DATAでも使用します。
    util::SharedPtr<util::DyArray<DXGI_FORMAT>>     compatible_dxgiformats;
};

class FormatCompatibilityChecker
{
public:
    FormatCompatibilityChecker() {}
    ~FormatCompatibilityChecker() {}

    // 各_TYPELESSフォーマットに対する、互換性のあるフォーマットが使用中のデバイスで利用可能かどうかを調べ、compatible_formatsに追加します。
    void Init(const FormatPropertiesD3D12& _format_properties_d3d12);

    const util::UnordMap<RESOURCE_FORMAT, util::SharedPtr<TYPELESS_COMPATIBLE_FORMATS>>& GetTypelessCompatibleFormats() const { return compatible_formats; }

    // TEXTURE_FORMAT_DESC内に含まれるフォーマットが有効かどうか調べて結果を返します。
    const util::SharedPtr<TYPELESS_COMPATIBLE_FORMATS> CheckCompatibility(const TEXTURE_FORMAT_DESC& _desc) const;

private:
    void PrepareCompatibleFormatsList(RESOURCE_FORMAT _typeless_format, const std::initializer_list<RESOURCE_FORMAT>& _all_compatible_formats, const util::UnordMap<DXGI_FORMAT, util::SharedPtr<FORMAT_PROPERTIES_D3D12>>& _props);
    void ResetCompatibleFormatsListIfEmpty(util::SharedPtr<TYPELESS_COMPATIBLE_FORMATS> _compatible_formats);

private:
    util::UnordMap<RESOURCE_FORMAT, util::SharedPtr<TYPELESS_COMPATIBLE_FORMATS>> compatible_formats;

};


}// namespace util
}// namespace buma3d
