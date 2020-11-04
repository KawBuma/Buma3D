#pragma once

namespace buma3d
{

class B3D_API SamplerViewD3D12 : public IDeviceChildD3D12<ISamplerView>, public IViewD3D12, public util::details::NEW_DELETE_OVERRIDE
{
protected:
    B3D_APIENTRY SamplerViewD3D12();
    SamplerViewD3D12(const SamplerViewD3D12&) = delete;
    B3D_APIENTRY ~SamplerViewD3D12();

private:
    BMRESULT B3D_APIENTRY Init(DeviceD3D12* _device, const SAMPLER_DESC& _desc);
    void B3D_APIENTRY PrepareD3D12StaticSamplerDesc(D3D12_SAMPLER_DESC& _desc12);
    void B3D_APIENTRY CopyDesc(const SAMPLER_DESC& _desc);
    void B3D_APIENTRY Uninit();

public:
    static BMRESULT
        B3D_APIENTRY Create(DeviceD3D12* _device, const SAMPLER_DESC& _desc, SamplerViewD3D12** _dst);

    void
        B3D_APIENTRY AddRef() override;

    uint32_t
        B3D_APIENTRY Release() override;

    uint32_t
        B3D_APIENTRY GetRefCount() const override;

    const char*
        B3D_APIENTRY GetName() const override;

    BMRESULT
        B3D_APIENTRY SetName(const char* _name) override;

    IDevice*
        B3D_APIENTRY GetDevice() const override;

    const CPU_DESCRIPTOR_ALLOCATION*
        B3D_APIENTRY GetCpuDescriptorAllocation() const override;

    const BUFFER_VIEW*
        B3D_APIENTRY GetBufferView() const override;

    const TEXTURE_VIEW*
        B3D_APIENTRY GetTextureView() const override;

    const D3D12_GPU_VIRTUAL_ADDRESS*
        B3D_APIENTRY GetGpuVirtualAddress() const;

    const VIEW_DESC&
        B3D_APIENTRY GetViewDesc() const override;

    IResource*
        B3D_APIENTRY GetResource() const override;

    const SAMPLER_DESC&
        B3D_APIENTRY GetDesc() const override;

    const D3D12_STATIC_SAMPLER_DESC&
        B3D_APIENTRY GetD3D12StaticSamplerDesc() const;

private:
    std::atomic_uint32_t                   ref_count;
    util::UniquePtr<util::NameableObjStr>  name;
    DeviceD3D12*                           device;
    SAMPLER_DESC                           desc;
    ID3D12Device*                          device12;
    D3D12_STATIC_SAMPLER_DESC              static_sampler_desc;    
    CPU_DESCRIPTOR_ALLOCATION              descriptor;

};


}// namespace buma3d
