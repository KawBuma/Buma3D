#pragma once

namespace buma3d
{

class B3D_API PipelineLayoutVk : public IDeviceChildVk<IPipelineLayout>, public util::details::NEW_DELETE_OVERRIDE
{
protected:
    B3D_APIENTRY PipelineLayoutVk();
    PipelineLayoutVk(const PipelineLayoutVk&) = delete;
    B3D_APIENTRY ~PipelineLayoutVk();

private:
    BMRESULT B3D_APIENTRY Init(DeviceVk* _device, const PIPELINE_LAYOUT_DESC& _desc);
    BMRESULT B3D_APIENTRY VerifyDesc(const PIPELINE_LAYOUT_DESC& _desc);
    BMRESULT B3D_APIENTRY CopyDesc(const PIPELINE_LAYOUT_DESC& _desc);
    BMRESULT B3D_APIENTRY CreateVkPipelineLayout();
    BMRESULT B3D_APIENTRY PreparePipelineLayoutCI(util::DyArray<VkDescriptorSetLayout>* _set_layouts, util::DyArray<VkPushConstantRange>* _push_constants);
    void B3D_APIENTRY Uninit();

public:
    static BMRESULT
        B3D_APIENTRY Create(DeviceVk* _device, const PIPELINE_LAYOUT_DESC& _desc, PipelineLayoutVk** _dst);

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

    const PIPELINE_LAYOUT_DESC&
        B3D_APIENTRY GetDesc() const /*override*/;

    VkPipelineLayout
        B3D_APIENTRY GetVkPipelineLayout() const;

    const VkPushConstantRange*
        B3D_APIENTRY GetVkPushConstantRanges() const;

private:
    struct DESC_DATA
    {
        ~DESC_DATA()
        {
            for (auto& i : set_layouts)
                hlp::SafeRelease(i);
        }
        util::DyArray<IDescriptorSetLayout*>    set_layouts;
        util::DyArray<PUSH_CONSTANT_PARAMETER>  push_constants;
    };

private:
    std::atomic_uint32_t                                ref_count;
    util::UniquePtr<util::NameableObjStr>               name;
    DeviceVk*                                           device;
    PIPELINE_LAYOUT_DESC                                desc;
    util::UniquePtr<DESC_DATA>                          desc_data;
    VkDevice                                            vkdevice;
    const InstancePFN*                                  inspfn;
    const DevicePFN*                                    devpfn;
    VkPipelineLayout                                    pipeline_layout;
    util::UniquePtr<util::DyArray<VkPushConstantRange>> push_constants;

};


}// namespace buma3d
