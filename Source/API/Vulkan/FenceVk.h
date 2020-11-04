#pragma once

namespace buma3d
{

class B3D_API FenceVk : public IDeviceChildVk<IFence>, public util::details::NEW_DELETE_OVERRIDE
{
protected:
    B3D_APIENTRY FenceVk();
    FenceVk(const FenceVk&) = delete;
    B3D_APIENTRY ~FenceVk();

private:
    BMRESULT B3D_APIENTRY Init(DeviceVk* _device, const FENCE_DESC& _desc);
    BMRESULT B3D_APIENTRY CreateImpl();
    BMRESULT B3D_APIENTRY CreateVkFence(DeviceVk* _device, VkFence* _dst_fence);
    BMRESULT B3D_APIENTRY SetSemaphoreFlags(const void** _last_pnext, VkExportSemaphoreCreateInfo& _export_ci);
    BMRESULT B3D_APIENTRY CreateBinaryVkSemaphore(DeviceVk* _device, VkSemaphore* _dst_semaphore);
    BMRESULT B3D_APIENTRY CreateTimelineVkSemaphore(DeviceVk* _device, VkSemaphore* _dst_semaphore);
    void B3D_APIENTRY Uninit();

public:
    static BMRESULT
        B3D_APIENTRY Create(DeviceVk* _device, const FENCE_DESC& _desc, FenceVk** _dst);

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

    const FENCE_DESC&
        B3D_APIENTRY GetDesc() const override;

    BMRESULT
        B3D_APIENTRY Reset() override;

    BMRESULT
        B3D_APIENTRY GetCompletedValue(uint64_t* _value) const override;

    BMRESULT
        B3D_APIENTRY Wait(uint64_t _value, uint32_t _timeout_millisec) override;

    BMRESULT
        B3D_APIENTRY Signal(uint64_t _value) override;

    VkSemaphore
        B3D_APIENTRY GetVkSemaphore() const;

    VkFence
        B3D_APIENTRY GetVkFence() const;

    bool
        B3D_APIENTRY IsTimelineSemaphore() const;

    BMRESULT
        B3D_APIENTRY SubmitWait(uint64_t _value);

    BMRESULT
        B3D_APIENTRY SubmitSignal(uint64_t _value);

    BMRESULT
        B3D_APIENTRY SubmitSignalToCpu();

private:
    std::atomic_uint32_t                  ref_count;
    util::UniquePtr<util::NameableObjStr> name;
    DeviceVk*                             device;
    VkDevice                              vkdevice;
    const InstancePFN*                    inspfn;
    const DevicePFN*                      devpfn;
    FENCE_DESC                            desc;

    struct IImpl;
    class BinaryGpuToCpuImpl;
    class BinaryGpuToGpuImpl;
    class TimelineImpl;
    IImpl* impl;

};


}// namespace buma3d
