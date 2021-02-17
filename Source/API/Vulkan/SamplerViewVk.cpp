#include "Buma3DPCH.h"
#include "SamplerViewVk.h"

namespace buma3d
{

static constexpr VIEW_DESC DEFAULT_SAMPLERS_VIEW_DESC = { VIEW_TYPE_SAMPLER, RESOURCE_FORMAT_UNKNOWN, VIEW_DIMENSION_SAMPLER };

B3D_APIENTRY SamplerViewVk::SamplerViewVk()
    : ref_count     { 1 }
    , name          {}
    , device        {}
    , desc          {}
    , vkdevice      {}
    , inspfn        {}
    , devpfn        {}
    , sampler       {}
    , image_info    { VK_NULL_HANDLE, VK_NULL_HANDLE, VK_IMAGE_LAYOUT_UNDEFINED }
{

}

B3D_APIENTRY SamplerViewVk::~SamplerViewVk()
{
    Uninit();
}

BMRESULT
B3D_APIENTRY SamplerViewVk::Init(DeviceVk* _device, const SAMPLER_DESC& _desc)
{
    (device = _device)->AddRef();
    vkdevice = _device->GetVkDevice();
    inspfn = &device->GetInstancePFN();
    devpfn = &device->GetDevicePFN();

    if (device->GetAllocationCounters().samplers >= device->GetDeviceAdapter()->GetPhysicalDeviceData().properties2.properties.limits.maxSamplerAllocationCount)
        return BMRESULT_FAILED_TOO_MANY_OBJECTS;

    CopyDesc(_desc);

    VkSamplerCreateInfo ci{ VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
    /*VK_SAMPLER_CREATE_SUBSAMPLED_BIT_EXT
    * VK_SAMPLER_CREATE_SUBSAMPLED_COARSE_RECONSTRUCTION_BIT_EXT*/
    ci.flags                   = 0;
    ci.magFilter               = util::GetNativeTextureSampleMode   (desc.texture.sample.magnification);
    ci.minFilter               = util::GetNativeTextureSampleMode   (desc.texture.sample.minification);
    ci.mipmapMode              = util::GetNativeTextureSampleModeMip(desc.texture.sample.mip);
    ci.addressModeU            = util::GetNativeAddressMode         (desc.texture.address.u);
    ci.addressModeV            = util::GetNativeAddressMode         (desc.texture.address.v);
    ci.addressModeW            = util::GetNativeAddressMode         (desc.texture.address.w);
    ci.anisotropyEnable        = desc.filter.mode == SAMPLER_FILTER_MODE_ANISOTROPHIC;
    ci.compareEnable           = desc.filter.reduction_mode == SAMPLER_FILTER_REDUCTION_MODE_COMPARISON;
    ci.maxAnisotropy           = SCAST<float>(desc.filter.max_anisotropy);
    ci.compareOp               = util::GetNativeComparisonFunc(desc.filter.comparison_func);
    ci.mipLodBias              = desc.mip_lod.bias;
    ci.minLod                  = desc.mip_lod.min;
    ci.maxLod                  = desc.mip_lod.max;
    ci.borderColor             = util::GetNativeBorderColor(desc.border_color);
    ci.unnormalizedCoordinates = VK_FALSE;// NOTE: D3D12との互換無し。

    auto last_pnext = &ci.pNext;

    VkSamplerCustomBorderColorCreateInfoEXT border_color_ci{ VK_STRUCTURE_TYPE_SAMPLER_CUSTOM_BORDER_COLOR_CREATE_INFO_EXT };
    if (false)
    {
        border_color_ci.format;
        border_color_ci.customBorderColor;
        last_pnext = util::ConnectPNextChains(last_pnext, border_color_ci);
    }

    VkSamplerReductionModeCreateInfo reduction_mode_ci{ VK_STRUCTURE_TYPE_SAMPLER_REDUCTION_MODE_CREATE_INFO };
    reduction_mode_ci.reductionMode = util::GetNativeFilterReductionMode(desc.filter.reduction_mode);
    last_pnext = util::ConnectPNextChains(last_pnext, border_color_ci);

    // TODO: SamplerViewVk: VkSamplerYcbcrConversionInfo 
    VkSamplerYcbcrConversionInfo ycbcr_conv { VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_INFO };

    auto vkr = vkCreateSampler(vkdevice, &ci, B3D_VK_ALLOC_CALLBACKS, &sampler);
    B3D_RET_IF_FAILED(VKR_TRACE_IF_FAILED(vkr));

    image_info.sampler = sampler;
    ++device->GetAllocationCounters().samplers;

    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY SamplerViewVk::CopyDesc(const SAMPLER_DESC& _desc)
{
    desc = _desc;
}

void
B3D_APIENTRY SamplerViewVk::Uninit()
{
    if (sampler)
    {
        vkDestroySampler(vkdevice, sampler, B3D_VK_ALLOC_CALLBACKS);
        sampler = VK_NULL_HANDLE;
        --(device->GetAllocationCounters().samplers);
    }

    hlp::SafeRelease(device);
    vkdevice = VK_NULL_HANDLE;
    inspfn   = nullptr;
    devpfn   = nullptr;

    desc = {};
    name.reset();
}

BMRESULT
B3D_APIENTRY SamplerViewVk::Create(DeviceVk* _device, const SAMPLER_DESC& _desc, SamplerViewVk** _dst)
{
    util::Ptr<SamplerViewVk> ptr;
    ptr.Attach(B3DCreateImplementationClass(SamplerViewVk));
    B3D_RET_IF_FAILED(ptr->Init(_device, _desc));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY SamplerViewVk::AddRef()
{
    ++ref_count;
    B3D_REFCOUNT_DEBUG(ref_count);
}

uint32_t
B3D_APIENTRY SamplerViewVk::Release()
{
    B3D_REFCOUNT_DEBUG(ref_count - 1);
    auto count = --ref_count;
    if (count == 0)
        B3DDestroyImplementationClass(this);

    return count;
}

uint32_t
B3D_APIENTRY SamplerViewVk::GetRefCount() const
{
    return ref_count;
}

const char*
B3D_APIENTRY SamplerViewVk::GetName() const
{
    return name ? name->c_str() : nullptr;
}

BMRESULT
B3D_APIENTRY SamplerViewVk::SetName(const char* _name)
{
    if (!util::IsEnabledDebug(this))
        return BMRESULT_FAILED;

    if (sampler)
        B3D_RET_IF_FAILED(device->SetVkObjectName(sampler, _name));

    if (name && !_name)
        name.reset();
    else
        name = B3DMakeUniqueArgs(util::NameableObjStr, _name);

    return BMRESULT_SUCCEED;
}

IDevice*
B3D_APIENTRY SamplerViewVk::GetDevice() const
{
    return device;
}

const BUFFER_VIEW*
B3D_APIENTRY SamplerViewVk::GetBufferView() const
{
    return nullptr;
}

const TEXTURE_VIEW*
B3D_APIENTRY SamplerViewVk::GetTextureView() const
{
    return nullptr;
}

const VkAllocationCallbacks*
B3D_APIENTRY SamplerViewVk::GetVkAllocationCallbacks() const
{
    return device->GetVkAllocationCallbacks();
}

const InstancePFN&
B3D_APIENTRY SamplerViewVk::GetIsntancePFN() const
{
    return *inspfn;
}

const DevicePFN&
B3D_APIENTRY SamplerViewVk::GetDevicePFN() const
{
    return *devpfn;
}

const VkDescriptorImageInfo*
B3D_APIENTRY SamplerViewVk::GetVkDescriptorImageInfo() const
{
    return &image_info;
}

VkSampler
B3D_APIENTRY SamplerViewVk::GetVkSampler() const
{
    return sampler;
}

BMRESULT
B3D_APIENTRY SamplerViewVk::AddDescriptorWriteRange(void* _dst, uint32_t _array_index) const
{
    auto&& dst = *RCAST<DescriptorSet0Vk::UPDATE_DESCRIPTOR_RANGE_BUFFER*>(_dst);
    if (!dst.image_infos_data)
        return BMRESULT_FAILED;

    dst.image_infos_data[_array_index] = image_info;
    return BMRESULT_SUCCEED;
}

const VIEW_DESC&
B3D_APIENTRY SamplerViewVk::GetViewDesc() const
{
    return DEFAULT_SAMPLERS_VIEW_DESC;
}

IResource*
B3D_APIENTRY SamplerViewVk::GetResource() const
{
    return nullptr;
}

const SAMPLER_DESC&
B3D_APIENTRY SamplerViewVk::GetDesc() const 
{
    return desc;
}


}// namespace buma3d
