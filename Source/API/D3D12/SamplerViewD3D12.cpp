#include "Buma3DPCH.h"
#include "SamplerViewD3D12.h"

namespace buma3d
{

static constexpr VIEW_DESC DEFAULT_SAMPLERS_VIEW_DESC = { VIEW_TYPE_SAMPLER, RESOURCE_FORMAT_UNKNOWN, VIEW_DIMENSION_SAMPLER };

B3D_APIENTRY SamplerViewD3D12::SamplerViewD3D12()
    : ref_count                 { 1 }
    , name                      {}
    , device                    {}
    , desc                      {}
    , device12                  {}
    , static_sampler_desc       {}
    , descriptor                {}
{

}

B3D_APIENTRY SamplerViewD3D12::~SamplerViewD3D12()
{
    Uninit();
}

BMRESULT
B3D_APIENTRY SamplerViewD3D12::Init(DeviceD3D12* _device, const SAMPLER_DESC& _desc)
{
    (device = _device)->AddRef();
    device12 = device->GetD3D12Device();

    CopyDesc(_desc);

    D3D12_SAMPLER_DESC desc12{};
    desc12.Filter = D3D12_FILTER(
        D3D12_ENCODE_BASIC_FILTER(
              util::GetNativeTextureSampleMode  (desc.texture.sample.minification)                         // min
            , util::GetNativeTextureSampleMode  (desc.texture.sample.magnification)                        // mag
            , util::GetNativeTextureSampleMode  (desc.texture.sample.mip)                                  // mip
            , util::GetNativeFilterReductionMode(desc.filter.reduction_mode)                               // reduction
        ) | (desc.filter.mode == SAMPLER_FILTER_MODE_ANISOTROPHIC ? D3D12_ANISOTROPIC_FILTERING_BIT : 0x0) // aniso enabled ?
    );

    desc12.MaxAnisotropy   = desc.filter.max_anisotropy;
    desc12.ComparisonFunc  = util::GetNativeComparisonFunc(desc.filter.comparison_func);
    desc12.AddressU        = util::GetNativeAddressMode(desc.texture.address.u);
    desc12.AddressV        = util::GetNativeAddressMode(desc.texture.address.v);
    desc12.AddressW        = util::GetNativeAddressMode(desc.texture.address.w);
    desc12.MipLODBias      = desc.mip_lod.bias;
    desc12.MinLOD          = desc.mip_lod.min;
    desc12.MaxLOD          = desc.mip_lod.max;
    util::GetNativeBorderColor(desc.border_color, desc12.BorderColor);

    descriptor = device->GetCPUDescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, B3D_DEFAULT_NODE_MASK).Allocate();
    device12->CreateSampler(&desc12, descriptor.handle);
    if (util::IsEnabledDebug(this))
        device->CheckDXGIInfoQueue();

    PrepareD3D12StaticSamplerDesc(desc12);

    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY SamplerViewD3D12::PrepareD3D12StaticSamplerDesc(D3D12_SAMPLER_DESC& _desc12)
{
    static_sampler_desc.Filter           = _desc12.Filter;
    static_sampler_desc.AddressU         = _desc12.AddressU;
    static_sampler_desc.AddressV         = _desc12.AddressV;
    static_sampler_desc.AddressW         = _desc12.AddressW;
    static_sampler_desc.MipLODBias       = _desc12.MipLODBias;
    static_sampler_desc.MaxAnisotropy    = _desc12.MaxAnisotropy;
    static_sampler_desc.ComparisonFunc   = _desc12.ComparisonFunc;
    static_sampler_desc.BorderColor      = util::GetNativeStaticBorderColor(desc.border_color);
    static_sampler_desc.MinLOD           = _desc12.MinLOD;
    static_sampler_desc.MaxLOD           = _desc12.MaxLOD;
    static_sampler_desc.ShaderRegister   = 0;
    static_sampler_desc.RegisterSpace    = 0;
    static_sampler_desc.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
}

void
B3D_APIENTRY SamplerViewD3D12::CopyDesc(const SAMPLER_DESC& _desc)
{
    desc = _desc;
}

void
B3D_APIENTRY SamplerViewD3D12::Uninit()
{
    name.reset();
    desc = {};

    if (descriptor.handle)
        device->GetCPUDescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, B3D_DEFAULT_NODE_MASK).Free(descriptor);
    descriptor = {};

    hlp::SafeRelease(device);
}

BMRESULT
B3D_APIENTRY SamplerViewD3D12::Create(DeviceD3D12* _device, const SAMPLER_DESC& _desc, SamplerViewD3D12** _dst)
{
    util::Ptr<SamplerViewD3D12> ptr;
    ptr.Attach(B3DCreateImplementationClass(SamplerViewD3D12));
    B3D_RET_IF_FAILED(ptr->Init(_device, _desc));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY SamplerViewD3D12::AddRef()
{
    ++ref_count;
    B3D_REFCOUNT_DEBUG(ref_count);
}

uint32_t
B3D_APIENTRY SamplerViewD3D12::Release()
{
    B3D_REFCOUNT_DEBUG(ref_count - 1);
    auto count = --ref_count;
    if (count == 0)
        B3DDestroyImplementationClass(this);

    return count;
}

uint32_t
B3D_APIENTRY SamplerViewD3D12::GetRefCount() const
{
    return ref_count;
}

const char*
B3D_APIENTRY SamplerViewD3D12::GetName() const
{
    return name ? name->c_str() : nullptr;
}

BMRESULT
B3D_APIENTRY SamplerViewD3D12::SetName(const char* _name)
{
    if (!util::IsEnabledDebug(this))
        return BMRESULT_FAILED;
    
    if (name && !_name)
        name.reset();
    else
        name = B3DMakeUniqueArgs(util::NameableObjStr, _name);

    return BMRESULT_SUCCEED;
}

IDevice*
B3D_APIENTRY SamplerViewD3D12::GetDevice() const
{
    return device;
}

const CPU_DESCRIPTOR_ALLOCATION*
B3D_APIENTRY SamplerViewD3D12::GetCpuDescriptorAllocation() const
{
    return &descriptor;
}

const BUFFER_VIEW*
B3D_APIENTRY SamplerViewD3D12::GetBufferView() const
{
    return nullptr;
}

const TEXTURE_VIEW*
B3D_APIENTRY SamplerViewD3D12::GetTextureView() const
{
    return nullptr;
}

const D3D12_GPU_VIRTUAL_ADDRESS*
B3D_APIENTRY SamplerViewD3D12::GetGpuVirtualAddress() const
{
    return nullptr;
}

const VIEW_DESC&
B3D_APIENTRY SamplerViewD3D12::GetViewDesc() const
{
    return DEFAULT_SAMPLERS_VIEW_DESC;
}

IResource*
B3D_APIENTRY SamplerViewD3D12::GetResource() const
{
    return nullptr;
}

const SAMPLER_DESC&
B3D_APIENTRY SamplerViewD3D12::GetDesc() const 
{
    return desc;
}

const D3D12_STATIC_SAMPLER_DESC& 
B3D_APIENTRY SamplerViewD3D12::GetD3D12StaticSamplerDesc() const
{
    return static_sampler_desc;
}


}// namespace buma3d
