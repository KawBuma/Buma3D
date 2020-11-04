#pragma once

namespace buma3d
{

class B3D_API ComputePipelineStateD3D12 : public IPipelineStateD3D12, public util::details::NEW_DELETE_OVERRIDE
{
protected:
    B3D_APIENTRY ComputePipelineStateD3D12();
    ComputePipelineStateD3D12(const ComputePipelineStateD3D12&) = delete;
    B3D_APIENTRY ~ComputePipelineStateD3D12();

private:
    BMRESULT B3D_APIENTRY Init(DeviceD3D12* _device, const COMPUTE_PIPELINE_STATE_DESC& _desc);
    BMRESULT B3D_APIENTRY CreateComputeD3D12PipelineState();
    void B3D_APIENTRY CopyDesc(const COMPUTE_PIPELINE_STATE_DESC& _desc);
    void B3D_APIENTRY Uninit();

public:
    static BMRESULT
        B3D_APIENTRY Create(DeviceD3D12* _device, const COMPUTE_PIPELINE_STATE_DESC& _desc, ComputePipelineStateD3D12** _dst);

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

    ID3D12PipelineState*
        B3D_APIENTRY GetD3D12PipelineState() const override;

    ID3D12StateObject*
        B3D_APIENTRY GetD3D12StateObject() const override;

    void
        B3D_APIENTRY BindPipeline(ID3D12GraphicsCommandList* _list) const override;

    PIPELINE_BIND_POINT
        B3D_APIENTRY GetPipelineBindPoint() const override;

    BMRESULT
        B3D_APIENTRY GetCachedBlob(IBlob** _dst) override;

private:
    struct DESC_DATA
    {
        ~DESC_DATA()
        {
            hlp::SafeRelease(root_signature);
            hlp::SafeRelease(module);
            B3DSafeDeleteArray(entry_point_name);
        }
        RootSignatureD3D12* root_signature;
        ShaderModuleD3D12*  module;
        char*               entry_point_name;
    };

private:
    std::atomic_uint32_t                        ref_count;
    util::UniquePtr<util::NameableObjStr>       name;
    DeviceD3D12*                                device;
    COMPUTE_PIPELINE_STATE_DESC                 desc;
    DESC_DATA                                   desc_data;
    ID3D12Device*                               device12;
    ID3D12PipelineState*                        pipeline;

};


}// namespace buma3d
