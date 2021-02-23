#include "Buma3DPCH.h"
#include "ComputePipelineStateD3D12.h"
#include <dxcapi.h>

namespace buma3d
{

B3D_APIENTRY ComputePipelineStateD3D12::ComputePipelineStateD3D12()
    : ref_count { 1 }
    , name      {}
    , device    {}
    , desc      {}
    , desc_data {}
    , device12  {}
    , pipeline  {}
{

}

B3D_APIENTRY ComputePipelineStateD3D12::~ComputePipelineStateD3D12()
{
    Uninit();
}

BMRESULT
B3D_APIENTRY ComputePipelineStateD3D12::Init0(DeviceD3D12* _device,RootSignatureD3D12* _signature, const COMPUTE_PIPELINE_STATE_DESC& _desc)
{
    (device = _device)->AddRef();
    device12 = device->GetD3D12Device();

    if (_desc.pipeline_layout)
    {
        B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_INITIALIZATION
                          , "IDevice::CreateComputePipelineState0から作成する場合、COMPUTE_PIPELINE_STATE_DESC::pipeline_layoutはnullptrである必要があります。");
        return BMRESULT_FAILED;
    }
    CopyDesc(_desc);
    (desc_data->root_signature = _signature)->AddRef();

    B3D_RET_IF_FAILED(CreateComputeD3D12PipelineState());

    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY ComputePipelineStateD3D12::Init(DeviceD3D12* _device, const COMPUTE_PIPELINE_STATE_DESC& _desc)
{
    (device = _device)->AddRef();
    device12 = device->GetD3D12Device();

    CopyDesc(_desc);

    B3D_RET_IF_FAILED(CreateComputeD3D12PipelineState());

    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY ComputePipelineStateD3D12::CreateComputeD3D12PipelineState()
{
    D3D12_COMPUTE_PIPELINE_STATE_DESC desc12{};
    desc12.CS                       = desc_data->module->GetD3D12ShaderBytecode();
    desc12.NodeMask                 = desc.node_mask;
    desc12.Flags                    = D3D12_PIPELINE_STATE_FLAG_NONE;

    desc12.pRootSignature = desc_data->root_signature
        ? desc_data->root_signature->GetD3D12RootSignature()
        : desc_data->pipeline_layout->GetD3D12RootSignature();

    // TODO: desc12.CachedPSO = {};

    auto hr = device12->CreateComputePipelineState(&desc12, IID_PPV_ARGS(&pipeline));
    B3D_RET_IF_FAILED(HR_TRACE_IF_FAILED(hr));

    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY ComputePipelineStateD3D12::CopyDesc(const COMPUTE_PIPELINE_STATE_DESC& _desc)
{
    desc = _desc;

    desc_data = B3DMakeUnique(DESC_DATA);

    if (_desc.pipeline_layout)
        (desc_data->pipeline_layout = _desc.pipeline_layout->As<PipelineLayoutD3D12>())->AddRef();

    (desc_data->module = _desc.shader_stage.module->As<ShaderModuleD3D12>())->AddRef();

    auto l = std::strlen(_desc.shader_stage.entry_point_name) + 1;
    desc_data->entry_point_name = util::MemCopyArray(B3DNewArray(char, l), _desc.shader_stage.entry_point_name, l);
    desc.shader_stage.entry_point_name = desc_data->entry_point_name;
}

void
B3D_APIENTRY ComputePipelineStateD3D12::Uninit()
{
    hlp::SafeRelease(pipeline);

    desc = {};
    desc_data.reset();

    hlp::SafeRelease(device);
    device12 = nullptr;

    name.reset();
}

BMRESULT
B3D_APIENTRY ComputePipelineStateD3D12::Create0(DeviceD3D12* _device, RootSignatureD3D12* _signature, const COMPUTE_PIPELINE_STATE_DESC& _desc, ComputePipelineStateD3D12** _dst)
{
    util::Ptr<ComputePipelineStateD3D12> ptr;
    ptr.Attach(B3DCreateImplementationClass(ComputePipelineStateD3D12));
    B3D_RET_IF_FAILED(ptr->Init0(_device, _signature, _desc));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY ComputePipelineStateD3D12::Create(DeviceD3D12* _device, const COMPUTE_PIPELINE_STATE_DESC& _desc, ComputePipelineStateD3D12** _dst)
{
    util::Ptr<ComputePipelineStateD3D12> ptr;
    ptr.Attach(B3DCreateImplementationClass(ComputePipelineStateD3D12));
    B3D_RET_IF_FAILED(ptr->Init(_device, _desc));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY ComputePipelineStateD3D12::AddRef() 
{
    ++ref_count;
    B3D_REFCOUNT_DEBUG(ref_count);
}

uint32_t
B3D_APIENTRY ComputePipelineStateD3D12::Release() 
{
    B3D_REFCOUNT_DEBUG(ref_count - 1);
    auto count = --ref_count;
    if (count == 0)
        B3DDestroyImplementationClass(this);

    return count;
}

uint32_t
B3D_APIENTRY ComputePipelineStateD3D12::GetRefCount() const 
{
    return ref_count;
}

const char*
B3D_APIENTRY ComputePipelineStateD3D12::GetName() const 
{
    return name ? name->c_str() : nullptr;
}

BMRESULT
B3D_APIENTRY ComputePipelineStateD3D12::SetName(const char* _name) 
{
    if (!util::IsEnabledDebug(this))
        return BMRESULT_FAILED;

    B3D_RET_IF_FAILED(HR_TRACE_IF_FAILED(util::SetName(pipeline, _name)));

    if (name && !_name)
        name.reset();
    else
        name = B3DMakeUniqueArgs(util::NameableObjStr, _name);

    return BMRESULT_SUCCEED;
}

IDevice*
B3D_APIENTRY ComputePipelineStateD3D12::GetDevice() const 
{
    return device;
}

ID3D12PipelineState*
B3D_APIENTRY ComputePipelineStateD3D12::GetD3D12PipelineState() const
{
    return pipeline;
}

ID3D12StateObject*
B3D_APIENTRY ComputePipelineStateD3D12::GetD3D12StateObject() const
{
    return nullptr;
}

void
B3D_APIENTRY ComputePipelineStateD3D12::BindPipeline(ID3D12GraphicsCommandList* _list) const
{
    _list->SetPipelineState(pipeline);
}

PIPELINE_BIND_POINT
B3D_APIENTRY ComputePipelineStateD3D12::GetPipelineBindPoint() const
{
    return PIPELINE_BIND_POINT_COMPUTE;
}

BMRESULT
B3D_APIENTRY ComputePipelineStateD3D12::GetCachedBlob(IBlob** _dst)
{
    return BMRESULT_FAILED_NOT_IMPLEMENTED;
}


}// namespace buma3d
