#pragma once

namespace buma3d
{

class B3D_API SamplerViewVk : public IDeviceChildVk<ISamplerView>, public IViewVk, public util::details::NEW_DELETE_OVERRIDE
{
protected:
    B3D_APIENTRY SamplerViewVk();
    SamplerViewVk(const SamplerViewVk&) = delete;
    B3D_APIENTRY ~SamplerViewVk();

private:
    BMRESULT B3D_APIENTRY Init(DeviceVk* _device, const SAMPLER_DESC& _desc);
    void B3D_APIENTRY CopyDesc(const SAMPLER_DESC& _desc);
    void B3D_APIENTRY Uninit();

public:
    static BMRESULT
        B3D_APIENTRY Create(DeviceVk* _device, const SAMPLER_DESC& _desc, SamplerViewVk** _dst);

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

    const BUFFER_VIEW*
        B3D_APIENTRY GetBufferView() const override;

    const TEXTURE_VIEW*
        B3D_APIENTRY GetTextureView() const override;

    const VkAllocationCallbacks*
        B3D_APIENTRY GetVkAllocationCallbacks() const override;

    const InstancePFN&
        B3D_APIENTRY GetIsntancePFN() const override;

    const DevicePFN&
        B3D_APIENTRY GetDevicePFN() const override;

    const VkDescriptorImageInfo*
        B3D_APIENTRY GetVkDescriptorImageInfo() const override;

    VkSampler
        B3D_APIENTRY GetVkSampler() const override;

    BMRESULT
        B3D_APIENTRY AddDescriptorWriteRange(void* _dst, uint32_t _array_index) const override;

    const VIEW_DESC&
        B3D_APIENTRY GetViewDesc() const override;

    IResource*
        B3D_APIENTRY GetResource() const override;

    const SAMPLER_DESC&
        B3D_APIENTRY GetDesc() const override;

private:
    std::atomic_uint32_t                    ref_count;
    util::UniquePtr<util::NameableObjStr>   name;
    DeviceVk*                               device;
    SAMPLER_DESC                            desc;

    VkDevice                                vkdevice;
    const InstancePFN*                      inspfn;
    const DevicePFN*                        devpfn;
    VkSampler                               sampler;
    VkDescriptorImageInfo                   image_info; // ディスクリプタの更新時に利用します。

};


}// namespace buma3d
