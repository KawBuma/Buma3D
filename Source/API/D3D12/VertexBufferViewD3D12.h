#pragma once

namespace buma3d
{

class B3D_API VertexBufferViewD3D12 : public IDeviceChildD3D12<IVertexBufferView>, public IViewD3D12, public util::details::NEW_DELETE_OVERRIDE
{
protected:
    B3D_APIENTRY VertexBufferViewD3D12();
    VertexBufferViewD3D12(const VertexBufferViewD3D12&) = delete;
    B3D_APIENTRY ~VertexBufferViewD3D12();

private:
    BMRESULT B3D_APIENTRY Init(DeviceD3D12* _device, IBuffer* _buffer, const VERTEX_BUFFER_VIEW_DESC& _desc);
    void B3D_APIENTRY CopyDesc(const VERTEX_BUFFER_VIEW_DESC& _desc);
    void B3D_APIENTRY Uninit();

public:
    static BMRESULT
        B3D_APIENTRY Create(DeviceD3D12* _device, IBuffer* _buffer, const VERTEX_BUFFER_VIEW_DESC& _desc, VertexBufferViewD3D12** _dst);

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

    const VERTEX_BUFFER_VIEW_DESC&
        B3D_APIENTRY GetDesc() const override;

    const util::DyArray<D3D12_VERTEX_BUFFER_VIEW>&
        B3D_APIENTRY GetD3D12VertexBufferViews() const;

private:
    std::atomic_uint32_t                    ref_count;
    util::UniquePtr<util::NameableObjStr>   name;
    DeviceD3D12*                            device;
    VERTEX_BUFFER_VIEW_DESC                 desc;
    BufferD3D12*                            buffer;
    ID3D12Device*                           device12;
    util::DyArray<D3D12_VERTEX_BUFFER_VIEW> vertex_buffer_views12;

};


}// namespace buma3d
