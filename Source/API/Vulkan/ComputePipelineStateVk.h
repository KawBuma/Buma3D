#pragma once

namespace buma3d
{

class B3D_API ComputePipelineStateVk : public IPipelineStateVk, public util::details::NEW_DELETE_OVERRIDE
{
protected:
    B3D_APIENTRY ComputePipelineStateVk();
    ComputePipelineStateVk(const ComputePipelineStateVk&) = delete;
    B3D_APIENTRY ~ComputePipelineStateVk();

private:
    BMRESULT B3D_APIENTRY Init(DeviceVk* _device, const COMPUTE_PIPELINE_STATE_DESC& _desc);
    BMRESULT B3D_APIENTRY CreateComputeVkPipeline();
    void B3D_APIENTRY CopyDesc(const COMPUTE_PIPELINE_STATE_DESC& _desc);
    void B3D_APIENTRY Uninit();

public:
    static BMRESULT
        B3D_APIENTRY Create(DeviceVk* _device, const COMPUTE_PIPELINE_STATE_DESC& _desc, ComputePipelineStateVk** _dst);

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

    const VkAllocationCallbacks*
        B3D_APIENTRY GetVkAllocationCallbacks() const override;

    const InstancePFN&
        B3D_APIENTRY GetIsntancePFN() const override;

    const DevicePFN&
        B3D_APIENTRY GetDevicePFN() const override;

    VkPipeline
        B3D_APIENTRY GetVkPipeline() const override;

    VkPipelineCache
        B3D_APIENTRY GetVkPipelineCache() const override;

    bool
        B3D_APIENTRY HasDynamicState(DYNAMIC_STATE _state) const override;

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
        RootSignatureVk* root_signature;
        ShaderModuleVk*  module;
        char*            entry_point_name;
    };

private:
    std::atomic_uint32_t                                ref_count;
    util::UniquePtr<util::NameableObjStr>               name;
    DeviceVk*                                           device;
    COMPUTE_PIPELINE_STATE_DESC                         desc;
    DESC_DATA                                           desc_data;

    VkDevice                                            vkdevice;
    const InstancePFN*                                  inspfn;
    const DevicePFN*                                    devpfn;
    VkPipeline                                          pipeline;
    VkPipelineCache                                     pipeline_cache;

};


}// namespace buma3d
