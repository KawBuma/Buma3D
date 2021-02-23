#include "Buma3DPCH.h"
#include "RenderTargetViewVk.h"

namespace buma3d
{

B3D_APIENTRY RenderTargetViewVk::RenderTargetViewVk()
    : ref_count                 { 1 }
    , name                      {}
    , device                    {}
    , desc                      {}
    , resource                  {}
    , vkdevice                  {}
    , inspfn                    {}
    , devpfn                    {}
    , image_view                {}
    , image_subresource_range   {}
{

}

B3D_APIENTRY RenderTargetViewVk::~RenderTargetViewVk()
{
    Uninit();
}

BMRESULT 
B3D_APIENTRY RenderTargetViewVk::Init(DeviceVk* _device, IResource* _resource, const RENDER_TARGET_VIEW_DESC& _desc)
{
    (device = _device)->AddRef();
    vkdevice = device->GetVkDevice();
    inspfn = &device->GetInstancePFN();
    devpfn = &device->GetDevicePFN();

    (resource = _resource)->AddRef();
    CopyDesc(_desc);

    if (desc.view.type != VIEW_TYPE_RENDER_TARGET)
    {
        B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_INITIALIZATION, "RENDER_TARGET_VIEW_DESC::view.typeはVIEW_TYPE_RENDER_TARGETである必要があります。");
        return BMRESULT_FAILED_INVALID_PARAMETER;
    }

    BMRESULT result = BMRESULT_SUCCEED;
    switch (desc.view.dimension)
    {
    case VIEW_DIMENSION_TEXTURE_1D:
    case VIEW_DIMENSION_TEXTURE_1D_ARRAY:
    case VIEW_DIMENSION_TEXTURE_2D:
    case VIEW_DIMENSION_TEXTURE_2D_ARRAY:
        result = InitAsImageView();
        break;

    case VIEW_DIMENSION_TEXTURE_3D:
    case VIEW_DIMENSION_TEXTURE_CUBE:
    default:
        result = BMRESULT_FAILED_INVALID_PARAMETER;
        break;
    }

    util::ConvertNativeSubresourceRange(desc.texture.subresource_range, &image_subresource_range);

    return result;
}

void 
B3D_APIENTRY RenderTargetViewVk::CopyDesc(const RENDER_TARGET_VIEW_DESC& _desc)
{
    desc = _desc;
}

BMRESULT 
B3D_APIENTRY RenderTargetViewVk::ValidateTextureRTV()
{
    if (resource->GetDesc().dimension == RESOURCE_DIMENSION_BUFFER)
    {
        B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_INITIALIZATION
                          , "VIEW_TYPE_TEXTURE_*の場合、リソースのdimensionはRESOURCE_DIMENSION_TEX*である必要があります。");
        return BMRESULT_FAILED_INVALID_PARAMETER;
    }
    // TODO: エラー処理(実行結果を参照しながら埋めていく)
    auto&& t = resource->As<TextureVk>()->GetDesc().texture;

    auto&& tdesc = desc.texture;
    auto&& range = tdesc.subresource_range;

    if (util::IsDepthStencilFormat(desc.view.format))
    {
        if ( (range.offset.aspect & ~(TEXTURE_ASPECT_FLAG_DEPTH | TEXTURE_ASPECT_FLAG_STENCIL)) ||
            !(range.offset.aspect &  (TEXTURE_ASPECT_FLAG_DEPTH | TEXTURE_ASPECT_FLAG_STENCIL)) ||
             (range.offset.aspect == (TEXTURE_ASPECT_FLAG_DEPTH | TEXTURE_ASPECT_FLAG_STENCIL)))
        {
            B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_INITIALIZATION
                              , "RENDER_TARGET_VIEW_DESC::...formatが深度ステンシルフォーマットの場合、"
                                "RENDER_TARGET_VIEW_DESC::...aspectはTEXTURE_ASPECT_FLAG_DEPTHまたはTEXTURE_ASPECT_FLAG_STENCILのいずれかのビットである必要があり、両方を含んではなりません。");
            return BMRESULT_FAILED_INVALID_PARAMETER;
        }
    }

    if (!util::IsIdentifyComponentMapping(tdesc.components))
    {
        B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_INITIALIZATION
                          , "RENDER_TARGET_VIEW_DESC::...componentsはIDENTIFYまたはそれぞれの変数と同じスウィズルである必要があります。");
        return BMRESULT_FAILED_INVALID_PARAMETER;
    }

    if (range.offset.mip_slice >= t.mip_levels)
    {
        B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_INITIALIZATION
                          , "RENDER_TARGET_VIEW_DESC::...mip_sliceが大きすぎます。mip_sliceはリソースのミップ数より小さい必要があります。");
        return BMRESULT_FAILED_INVALID_PARAMETER;
    }
    if (range.offset.array_slice >= t.array_size)
    {
        B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_INITIALIZATION
                          , "RENDER_TARGET_VIEW_DESC::...array_sliceが大きすぎます。array_sliceはリソースの深さ/配列要素数より小さい必要があります。");
        return BMRESULT_FAILED_INVALID_PARAMETER;
    }

    if (range.mip_levels != 1)
    {
        B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_INITIALIZATION
                          , "RENDER_TARGET_VIEW_DESC::...mip_levelsは1である必要があります。");
        return BMRESULT_FAILED_INVALID_PARAMETER;
    }

    // B3D_USE_REMAINING_ARRAY_SIZES
    {
        if (range.array_size == B3D_USE_REMAINING_ARRAY_SIZES)
            range.array_size = t.array_size - range.offset.array_slice;
    }

    if (range.offset.mip_slice   + range.mip_levels > t.mip_levels || 
        range.offset.array_slice + range.array_size > t.array_size)
    {
        B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_INITIALIZATION
                          , "ミップ、または配列の範囲指定が不正です。");
        return BMRESULT_FAILED_INVALID_PARAMETER;
    }

    switch (desc.view.dimension)
    {
    case buma3d::VIEW_DIMENSION_TEXTURE_1D:
    {
        if (range.array_size != 1)
            return BMRESULT_FAILED_INVALID_PARAMETER;
        if (t.sample_count != 1)
            return BMRESULT_FAILED_INVALID_PARAMETER;
        break;
    }

    case buma3d::VIEW_DIMENSION_TEXTURE_1D_ARRAY:
    {
        if (t.sample_count != 1)
            return BMRESULT_FAILED_INVALID_PARAMETER;
        break;
    }

    case buma3d::VIEW_DIMENSION_TEXTURE_2D:
    {
        if (range.array_size != 1)
            return BMRESULT_FAILED_INVALID_PARAMETER;
        break;
    }

    case buma3d::VIEW_DIMENSION_TEXTURE_2D_ARRAY:
    {
        break;
    }

    default:
        break;
    }

    return BMRESULT_SUCCEED;
}

BMRESULT 
B3D_APIENTRY RenderTargetViewVk::InitAsImageView()
{
    B3D_RET_IF_FAILED(ValidateTextureRTV());

    auto texture = resource->As<TextureVk>();
    VkImageViewCreateInfo ci{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
    ci.flags     = 0;//VK_IMAGE_VIEW_CREATE_FRAGMENT_DENSITY_MAP_DYNAMIC_BIT_EXT;
    ci.image     = texture->GetVkImage();
    ci.viewType  = util::GetNativeViewDimension(desc.view.dimension);
    ci.format    = util::GetNativeFormat(desc.view.format);

    auto&& tdesc = desc.texture;
    util::ConvertNativeComponentMapping(tdesc.components, &ci.components);
    ci.subresourceRange.aspectMask     = util::GetNativeAspectFlags(tdesc.subresource_range.offset.aspect);
    ci.subresourceRange.baseMipLevel   = tdesc.subresource_range.offset.mip_slice;
    ci.subresourceRange.levelCount     = tdesc.subresource_range.mip_levels;
    ci.subresourceRange.baseArrayLayer = tdesc.subresource_range.offset.array_slice;
    ci.subresourceRange.layerCount     = tdesc.subresource_range.array_size;

    auto last_pnext = &ci.pNext;

    VkImageViewUsageCreateInfo usage_ci{ VK_STRUCTURE_TYPE_IMAGE_VIEW_USAGE_CREATE_INFO };
    {
        usage_ci.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        last_pnext = util::ConnectPNextChains(last_pnext, usage_ci);
    }

    // TODO: RenderTargetViewVk: VkImageViewASTCDecodeModeEXT, VkSamplerYcbcrConversionInfo 
    VkImageViewASTCDecodeModeEXT atsc_ci{ VK_STRUCTURE_TYPE_IMAGE_VIEW_ASTC_DECODE_MODE_EXT };
    VkSamplerYcbcrConversionInfo ycbcr_ci{ VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_INFO };

    auto vkr = vkCreateImageView(vkdevice, &ci, B3D_VK_ALLOC_CALLBACKS, &image_view);
    B3D_RET_IF_FAILED(VKR_TRACE_IF_FAILED(vkr));

    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY RenderTargetViewVk::Uninit()
{
    if (image_view)
        vkDestroyImageView(vkdevice, image_view, B3D_VK_ALLOC_CALLBACKS);
    image_view = VK_NULL_HANDLE;

    image_subresource_range = {};

    hlp::SafeRelease(resource);
    hlp::SafeRelease(device);
    vkdevice = VK_NULL_HANDLE;
    inspfn = nullptr;
    devpfn = nullptr;

    desc = {};
    name.reset();
}

BMRESULT 
B3D_APIENTRY RenderTargetViewVk::Create(DeviceVk* _device, IResource* _resource, const RENDER_TARGET_VIEW_DESC& _desc, RenderTargetViewVk** _dst)
{
    util::Ptr<RenderTargetViewVk> ptr;
    ptr.Attach(B3DCreateImplementationClass(RenderTargetViewVk));
    B3D_RET_IF_FAILED(ptr->Init(_device, _resource, _desc));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY RenderTargetViewVk::AddRef()
{
    ++ref_count;
    B3D_REFCOUNT_DEBUG(ref_count);
}

uint32_t
B3D_APIENTRY RenderTargetViewVk::Release()
{
    B3D_REFCOUNT_DEBUG(ref_count - 1);
    auto count = --ref_count;
    if (count == 0)
        B3DDestroyImplementationClass(this);

    return count;
}

uint32_t
B3D_APIENTRY RenderTargetViewVk::GetRefCount() const
{
    return ref_count;
}

const char*
B3D_APIENTRY RenderTargetViewVk::GetName() const
{
    return name ? name->c_str() : nullptr;
}

BMRESULT
B3D_APIENTRY RenderTargetViewVk::SetName(const char* _name)
{
    if (!util::IsEnabledDebug(this))
        return BMRESULT_FAILED;

    if (image_view)
        B3D_RET_IF_FAILED(device->SetVkObjectName(image_view, _name));

    if (name && !_name)
        name.reset();
    else
        name = B3DMakeUniqueArgs(util::NameableObjStr, _name);

    return BMRESULT_SUCCEED;
}

IDevice*
B3D_APIENTRY RenderTargetViewVk::GetDevice() const
{
    return device;
}

const BUFFER_VIEW*
B3D_APIENTRY RenderTargetViewVk::GetBufferView() const
{
    return nullptr;
}

const TEXTURE_VIEW*
B3D_APIENTRY RenderTargetViewVk::GetTextureView() const
{
    return &desc.texture;
}

const VkAllocationCallbacks*
B3D_APIENTRY RenderTargetViewVk::GetVkAllocationCallbacks() const
{
    return device->GetVkAllocationCallbacks();
}

const InstancePFN&
B3D_APIENTRY RenderTargetViewVk::GetIsntancePFN() const
{
    return *inspfn;
}

const DevicePFN&
B3D_APIENTRY RenderTargetViewVk::GetDevicePFN() const
{
    return *devpfn;
}

VkImageView
B3D_APIENTRY RenderTargetViewVk::GetVkImageView() const
{
    return image_view;
}

VkImageLayout
B3D_APIENTRY RenderTargetViewVk::GetVkImageLayout() const
{
    return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
}

const VkImageSubresourceRange*
B3D_APIENTRY RenderTargetViewVk::GetVkImageSubresourceRange() const
{
    return &image_subresource_range;
}

BMRESULT
B3D_APIENTRY RenderTargetViewVk::AddDescriptorWriteRange(void* _dst, uint32_t _array_index) const
{
    auto&& dst = *RCAST<DescriptorSet0Vk::UPDATE_DESCRIPTOR_RANGE_BUFFER*>(_dst);
    if (!dst.image_infos_data)
        return BMRESULT_FAILED;
    auto&& info = dst.image_infos_data[_array_index];
    info.imageView   = image_view;
    info.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    info.sampler     = VK_NULL_HANDLE;

    return BMRESULT_SUCCEED;
}

const VIEW_DESC&
B3D_APIENTRY RenderTargetViewVk::GetViewDesc() const
{
    return desc.view;
}

IResource*
B3D_APIENTRY RenderTargetViewVk::GetResource() const
{
    return resource;
}

const RENDER_TARGET_VIEW_DESC&
B3D_APIENTRY RenderTargetViewVk::GetDesc() const
{
    return desc;
}


}// namespace buma3d
