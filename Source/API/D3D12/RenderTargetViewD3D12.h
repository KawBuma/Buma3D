#pragma once

namespace buma3d
{

class B3D_API RenderTargetViewD3D12 : public IDeviceChildD3D12<IRenderTargetView>, public IViewD3D12, public util::details::NEW_DELETE_OVERRIDE
{
protected:
    B3D_APIENTRY RenderTargetViewD3D12();
    RenderTargetViewD3D12(const RenderTargetViewD3D12&) = delete;
    B3D_APIENTRY ~RenderTargetViewD3D12();

private:
    BMRESULT B3D_APIENTRY Init(DeviceD3D12* _device, IResource* _resource, const RENDER_TARGET_VIEW_DESC& _desc);
    BMRESULT B3D_APIENTRY ValidateTextureRTV();
    BMRESULT B3D_APIENTRY InitAsTextureRTV();
    void B3D_APIENTRY CopyDesc(const RENDER_TARGET_VIEW_DESC& _desc);
    void B3D_APIENTRY Uninit();

public:
    static BMRESULT
        B3D_APIENTRY Create(DeviceD3D12* _device, IResource* _resource, const RENDER_TARGET_VIEW_DESC& _desc, RenderTargetViewD3D12** _dst);

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

    const RENDER_TARGET_VIEW_DESC&
        B3D_APIENTRY GetDesc() const override;

private:
    std::atomic_uint32_t                    ref_count;
    util::UniquePtr<util::NameableObjStr>   name;
    DeviceD3D12*                            device;
    RENDER_TARGET_VIEW_DESC                 desc;
    IResource*                              resource;
    ID3D12Device*                           device12;
    CPU_DESCRIPTOR_ALLOCATION               descriptor;

};


}// namespace buma3d
