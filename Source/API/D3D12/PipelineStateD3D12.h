#pragma once

namespace buma3d
{

B3D_INTERFACE IPipelineStateD3D12 : public IDeviceChildD3D12<IPipelineState>
{
protected:
    B3D_APIENTRY ~IPipelineStateD3D12() {}

public:
    virtual ID3D12PipelineState*
        B3D_APIENTRY GetD3D12PipelineState() const = 0;

    virtual ID3D12StateObject*
        B3D_APIENTRY GetD3D12StateObject() const = 0;

    virtual void
        B3D_APIENTRY BindPipeline(ID3D12GraphicsCommandList* _list) const = 0;

    virtual bool
        B3D_APIENTRY HasDynamicState(DYNAMIC_STATE _state) const { B3D_UNREFERENCED(_state); return false; }

};


}// namespace buma3d
