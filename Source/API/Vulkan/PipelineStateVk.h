#pragma once

namespace buma3d
{

B3D_INTERFACE IPipelineStateVk : public IDeviceChildVk<IPipelineState>
{
protected:
    B3D_APIENTRY ~IPipelineStateVk() {}

public:
    virtual VkPipeline
        B3D_APIENTRY GetVkPipeline() const = 0;

    virtual VkPipelineCache
        B3D_APIENTRY GetVkPipelineCache() const = 0;

    virtual bool
        B3D_APIENTRY HasDynamicState(DYNAMIC_STATE _state) const = 0;

};


}// namespace buma3d
