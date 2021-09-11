#pragma once

namespace buma3d
{

class B3D_API StreamOutputBufferViewD3D12 : public IDeviceChildD3D12<IStreamOutputBufferView>, public IViewD3D12, public util::details::NEW_DELETE_OVERRIDE
{
protected:
    B3D_APIENTRY StreamOutputBufferViewD3D12();
    StreamOutputBufferViewD3D12(const StreamOutputBufferViewD3D12&) = delete;
    B3D_APIENTRY ~StreamOutputBufferViewD3D12();

private:
    BMRESULT B3D_APIENTRY Init(DeviceD3D12* _device, IBuffer* _buffer, IBuffer* _filled_size_counter_buffer, const STREAM_OUTPUT_BUFFER_VIEW_DESC& _desc);
    void B3D_APIENTRY CopyDesc(const STREAM_OUTPUT_BUFFER_VIEW_DESC& _desc);
    void B3D_APIENTRY Uninit();

public:
    static BMRESULT
        B3D_APIENTRY Create(DeviceD3D12* _device, IBuffer* _buffer, IBuffer* _filled_size_counter_buffer, const STREAM_OUTPUT_BUFFER_VIEW_DESC& _desc, StreamOutputBufferViewD3D12** _dst);

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

    const D3D12_GPU_VIRTUAL_ADDRESS*
        B3D_APIENTRY GetGpuVirtualAddress() const;

    bool
        B3D_APIENTRY HasAllSubresources() const override;

    const BUFFER_VIEW*
        B3D_APIENTRY GetBufferView() const override;

    const TEXTURE_VIEW*
        B3D_APIENTRY GetTextureView() const override;

    const VIEW_DESC&
        B3D_APIENTRY GetViewDesc() const override;

    IResource*
        B3D_APIENTRY GetResource() const override;

    IBuffer*
        B3D_APIENTRY GetFilledSizeCounterBuffer() const override;

    const STREAM_OUTPUT_BUFFER_VIEW_DESC&
        B3D_APIENTRY GetDesc() const override;

    const util::DyArray<D3D12_STREAM_OUTPUT_BUFFER_VIEW>&
        B3D_APIENTRY GetD3D12StreamOutputBufferViews() const;

private:
    std::atomic_uint32_t                            ref_count;
    util::UniquePtr<util::NameableObjStr>           name;
    DeviceD3D12*                                    device;
    STREAM_OUTPUT_BUFFER_VIEW_DESC                  desc;
    BufferD3D12*                                    buffer;
    BufferD3D12*                                    filled_size_counter_buffer;
    ID3D12Device*                                   device12;
    util::DyArray<D3D12_STREAM_OUTPUT_BUFFER_VIEW>  stream_output_buffer_views12;

};


}// namespace buma3d
