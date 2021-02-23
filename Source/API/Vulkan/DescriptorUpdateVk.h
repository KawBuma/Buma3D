#pragma once

namespace buma3d
{

class B3D_API DescriptorUpdateVk : public IDeviceChildVk<IDescriptorUpdate>, public util::details::NEW_DELETE_OVERRIDE
{
protected:
    B3D_APIENTRY DescriptorUpdateVk();
    DescriptorUpdateVk(const DescriptorUpdateVk&) = delete;
    B3D_APIENTRY ~DescriptorUpdateVk();

private:
    BMRESULT B3D_APIENTRY Init(DeviceVk* _device, const DESCRIPTOR_UPDATE_DESC& _desc);
    void B3D_APIENTRY Uninit();

public:
    static BMRESULT
        B3D_APIENTRY Create(DeviceVk* _device, const DESCRIPTOR_UPDATE_DESC& _desc, DescriptorUpdateVk** _dst);

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

    BMRESULT
        B3D_APIENTRY UpdateDescriptorSets(const UPDATE_DESCRIPTOR_SET_DESC& _update_desc) override;

private:
    BMRESULT B3D_APIENTRY VerifyUpdateDescriptorSets(const UPDATE_DESCRIPTOR_SET_DESC& _update_desc);

private:
    std::atomic_uint32_t                        ref_count;
    util::UniquePtr<util::NameableObjStr>       name;
    DeviceVk*                                   device;
    DESCRIPTOR_UPDATE_DESC                      desc;
    VkDevice                                    vkdevice;
    const InstancePFN*                          inspfn;
    const DevicePFN*                            devpfn;
    util::UniquePtr<DescriptorSetUpdater>       updater;

};


}// namespace buma3d
