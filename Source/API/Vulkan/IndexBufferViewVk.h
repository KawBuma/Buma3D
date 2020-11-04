#pragma once

namespace buma3d
{

class B3D_API IndexBufferViewVk : public IDeviceChildVk<IIndexBufferView>, public IViewVk, public util::details::NEW_DELETE_OVERRIDE
{
public:
    // インデックスバッファバインド時のキャシュ
    struct BIND_INDEX_BUFFERS_DATA
    {
        VkBuffer     buffer;
        VkDeviceSize offset;
        VkIndexType  type;
    };

protected:
    B3D_APIENTRY IndexBufferViewVk();
    IndexBufferViewVk(const IndexBufferViewVk&) = delete;
    B3D_APIENTRY ~IndexBufferViewVk();

private:
    BMRESULT B3D_APIENTRY Init(DeviceVk* _device, IBuffer* _buffer, const INDEX_BUFFER_VIEW_DESC& _desc);
    void B3D_APIENTRY CopyDesc(const INDEX_BUFFER_VIEW_DESC& _desc);
    void B3D_APIENTRY Uninit();

public:
    static BMRESULT
        B3D_APIENTRY Create(DeviceVk* _device, IBuffer* _buffer, const INDEX_BUFFER_VIEW_DESC& _desc, IndexBufferViewVk** _dst);

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

    const VIEW_DESC&
        B3D_APIENTRY GetViewDesc() const override;

    IResource*
        B3D_APIENTRY GetResource() const override;

    const INDEX_BUFFER_VIEW_DESC&
        B3D_APIENTRY GetDesc() const override;

    const BIND_INDEX_BUFFERS_DATA&
        B3D_APIENTRY GetIndexBufferData() const;

private:
    std::atomic_uint32_t                    ref_count;
    util::UniquePtr<util::NameableObjStr>   name;
    DeviceVk*                               device;
    INDEX_BUFFER_VIEW_DESC                  desc;
    BufferVk*                               buffer;
    VkDevice                                vkdevice;
    const InstancePFN*                      inspfn;
    const DevicePFN*                        devpfn;
    BIND_INDEX_BUFFERS_DATA*                bind_index_buffers_data;

};


}// namespace buma3d
