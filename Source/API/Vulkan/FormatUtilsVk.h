#pragma once

namespace buma3d
{
namespace util
{

inline VkColorSpaceKHR GetNativeColorSpace(COLOR_SPACE _cs)
{
    switch (_cs)
    {
    case COLOR_SPACE_SRGB_NONLINEAR  : return VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    case COLOR_SPACE_BT709_LINEAR    : return VK_COLOR_SPACE_BT709_LINEAR_EXT;
    case COLOR_SPACE_BT709_NONLINEAR : return VK_COLOR_SPACE_BT709_NONLINEAR_EXT;
    case COLOR_SPACE_HDR10_ST2084_PQ : return VK_COLOR_SPACE_HDR10_ST2084_EXT;
    case COLOR_SPACE_CUSTOM          : return VK_COLOR_SPACE_PASS_THROUGH_EXT;
    default:
        return VK_COLOR_SPACE_PASS_THROUGH_EXT;
    }
}
inline COLOR_SPACE GetB3DColorSpace(VkColorSpaceKHR _cs)
{
    switch (_cs)
    {
    case VK_COLOR_SPACE_SRGB_NONLINEAR_KHR  : return COLOR_SPACE_SRGB_NONLINEAR;
    case VK_COLOR_SPACE_BT709_LINEAR_EXT    : return COLOR_SPACE_BT709_LINEAR;
    case VK_COLOR_SPACE_BT709_NONLINEAR_EXT : return COLOR_SPACE_BT709_NONLINEAR;
    case VK_COLOR_SPACE_HDR10_ST2084_EXT    : return COLOR_SPACE_HDR10_ST2084_PQ;
    case VK_COLOR_SPACE_PASS_THROUGH_EXT    : return COLOR_SPACE_CUSTOM;
    default:
        return COLOR_SPACE_CUSTOM;
    }
}

VkFormat GetNativeFormat(RESOURCE_FORMAT _format);
RESOURCE_FORMAT GetB3DFormat(VkFormat _format);


struct VULKAN_FORMAT_PROPERTIES
{
    RESOURCE_FORMAT                                                  b3d_format;
    VkFormatProperties2                                              format_props;                   // VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_2
    util::UniquePtr<VkDrmFormatModifierPropertiesListEXT>            drm_format_modifier_props_list; // VK_STRUCTURE_TYPE_DRM_FORMAT_MODIFIER_PROPERTIES_LIST_EXT
    util::UniquePtr<util::DyArray<VkDrmFormatModifierPropertiesEXT>> drm_format_modifier_props;
};

class VulkanFormatProperties
{
public:
    inline static constexpr int FORMAT_OFFSET             = 1 + static_cast<int>(VK_FORMAT_UNDEFINED);
    inline static constexpr int MULTIPLANAR_FORMAT_OFFSET = static_cast<int>(VK_FORMAT_G8B8G8R8_422_UNORM);
    inline static constexpr int PVRTC_FORMAT_OFFSET       = static_cast<int>(VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG);
    inline static constexpr int EXT_ASTC_FORMAT_OFFSET    = static_cast<int>(VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK_EXT);

    // WARNING: これらの値はVulkanのフォーマットの追加により変動する可能性があります。
    inline static constexpr int FORMAT_MAX              = static_cast<int>(VK_FORMAT_ASTC_12x12_SRGB_BLOCK);
    inline static constexpr int MULTIPLANAR_FORMAT_MAX  = static_cast<int>(VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM);
    inline static constexpr int PVRTC_FORMAT_MAX        = static_cast<int>(VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG);
    inline static constexpr int EXT_ASTC_FORMAT_MAX     = static_cast<int>(VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK_EXT);

public:
    VulkanFormatProperties();
    ~VulkanFormatProperties() {}

    void Init(VkInstance _instance, VkPhysicalDevice _physical_device, VkDevice _device);
    const util::UnordMap<VkFormat, util::SharedPtr<VULKAN_FORMAT_PROPERTIES>>& GetFormatsProperties() const { return formats_props; }

private:
    void PrepareAndEvaluateFormatProperties(VkFormat _format);
    void CreateProperties(util::SharedPtr<VULKAN_FORMAT_PROPERTIES>& _props);
    void GetVkFormatProperties2(VkFormat _format, VULKAN_FORMAT_PROPERTIES* _dst_format_props);
    bool HasSupportedAnyFeature(VULKAN_FORMAT_PROPERTIES* _props);

private:
    VkInstance       instance;
    VkPhysicalDevice physical_device;
    VkDevice         device;

    util::UnordMap<VkFormat, util::SharedPtr<VULKAN_FORMAT_PROPERTIES>> formats_props;

};

struct TYPELESS_COMPATIBLE_FORMATS
{
    util::SharedPtr<util::DyArray<RESOURCE_FORMAT>> compatible_formats;  // この配列はTEXTURE_FORMAT_DESCにデフォルトが指定された場合にTextureVk::DESC_DATAでも使用します。
    util::SharedPtr<util::DyArray<VkFormat>>        compatible_vkformats;// この配列はTEXTURE_FORMAT_DESCにデフォルトが指定された場合にVkImageFormatListCreateInfo::pViewFormatsでも使用します。
};

class FormatCompatibilityChecker
{
public:
    FormatCompatibilityChecker() {}
    ~FormatCompatibilityChecker() {}

    // 各_TYPELESSフォーマットに対する、互換性のあるフォーマットが使用中のデバイスで利用可能かどうかを調べ、compatible_formatsに追加します。
    void Init(const VulkanFormatProperties& _vulkan_format_properties);

    const util::UnordMap<RESOURCE_FORMAT, util::SharedPtr<TYPELESS_COMPATIBLE_FORMATS>>& GetTypelessCompatibleFormats() const { return compatible_formats; }

    // TEXTURE_FORMAT_DESC内に含まれるフォーマットが有効かどうか調べて結果を返します。
    const util::SharedPtr<TYPELESS_COMPATIBLE_FORMATS> CheckCompatibility(const TEXTURE_FORMAT_DESC& _desc) const;

private:
    void PrepareCompatibleFormatsList(RESOURCE_FORMAT _typeless_format, const std::initializer_list<RESOURCE_FORMAT>& _all_compatible_formats, const util::UnordMap<VkFormat, util::SharedPtr<VULKAN_FORMAT_PROPERTIES>>& _props);
    void ResetCompatibleFormatsListIfEmpty(util::SharedPtr<TYPELESS_COMPATIBLE_FORMATS> _compatible_formats);

private:
    util::UnordMap<RESOURCE_FORMAT, util::SharedPtr<TYPELESS_COMPATIBLE_FORMATS>> compatible_formats;

};


}// namespace util
}// namespace buma3d
