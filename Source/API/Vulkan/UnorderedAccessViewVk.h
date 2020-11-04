#pragma once

namespace buma3d
{

class B3D_API UnorderedAccessViewVk : public IDeviceChildVk<IUnorderedAccessView>, public IViewVk, public util::details::NEW_DELETE_OVERRIDE
{
protected:
    B3D_APIENTRY UnorderedAccessViewVk();
    UnorderedAccessViewVk(const UnorderedAccessViewVk&) = delete;
    B3D_APIENTRY ~UnorderedAccessViewVk();

private:
    BMRESULT B3D_APIENTRY Init(DeviceVk* _device, IResource* _resource, IBuffer* _resource_for_counter_buffer, const UNORDERED_ACCESS_VIEW_DESC& _desc);
    void B3D_APIENTRY CopyDesc(const UNORDERED_ACCESS_VIEW_DESC& _desc);
    BMRESULT B3D_APIENTRY InitAsBufferView();
    BMRESULT B3D_APIENTRY ValidateTextureUAV();
    BMRESULT B3D_APIENTRY InitAsImageView();
    void B3D_APIENTRY Uninit();

public:
    static BMRESULT
        B3D_APIENTRY Create(DeviceVk* _device, IResource* _resource, IBuffer* _resource_for_counter_buffer, const UNORDERED_ACCESS_VIEW_DESC& _desc, UnorderedAccessViewVk** _dst);

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

    VkBufferView
        B3D_APIENTRY GetVkBufferView() const override;

    const VkDescriptorBufferInfo*
        B3D_APIENTRY GetVkDescriptorBufferInfo() const override;

    VkImageView
        B3D_APIENTRY GetVkImageView() const override;

    VkImageLayout
        B3D_APIENTRY GetVkImageLayout() const override;

    const VkImageSubresourceRange*
        B3D_APIENTRY GetVkImageSubresourceRange() const override;

    BMRESULT
        B3D_APIENTRY AddDescriptorWriteRange(void* _dst, uint32_t _array_index) const override;

    const VIEW_DESC&
        B3D_APIENTRY GetViewDesc() const override;

    IResource*
        B3D_APIENTRY GetResource() const override;

    const UNORDERED_ACCESS_VIEW_DESC&
        B3D_APIENTRY GetDesc() const override;

    IBuffer*
        B3D_APIENTRY GetCounterBuffer() const override;

private:
    std::atomic_uint32_t                    ref_count;
    util::UniquePtr<util::NameableObjStr>   name;
    DeviceVk*                               device;
    UNORDERED_ACCESS_VIEW_DESC              desc;
    IResource*                              resource;
    IBuffer*                                counter_buffer;
    VkDevice                                vkdevice;
    const InstancePFN*                      inspfn;
    const DevicePFN*                        devpfn;
    const BUFFER_VIEW*                      buffer_view;
    const TEXTURE_VIEW*                     texture_view;

    struct IImpl;
    class BufferViewImpl;
    class ImageViewImpl;
    util::UniquePtr<IImpl> impl;

};


}// namespace buma3d
