#pragma once

namespace buma3d
{

class B3D_API VertexBufferViewVk : public IDeviceChildVk<IVertexBufferView>, public IViewVk, public util::details::NEW_DELETE_OVERRIDE
{
public:
    // 頂点バッファバインド時のキャシュ
    struct BIND_VERTEX_BUFFERS_DATA
    {
        uint32_t            binding_count;
        const VkBuffer*     buffers;
        const VkDeviceSize* offsets;
        const VkDeviceSize* sizes;
        const VkDeviceSize* strides;
    };

protected:
    B3D_APIENTRY VertexBufferViewVk();
    VertexBufferViewVk(const VertexBufferViewVk&) = delete;
    B3D_APIENTRY ~VertexBufferViewVk();

private:
    BMRESULT B3D_APIENTRY Init(DeviceVk* _device, IBuffer* _buffer, const VERTEX_BUFFER_VIEW_DESC& _desc);
    void B3D_APIENTRY CopyDesc(const VERTEX_BUFFER_VIEW_DESC& _desc);
    void B3D_APIENTRY Uninit();

public:
    static BMRESULT
        B3D_APIENTRY Create(DeviceVk* _device, IBuffer* _buffer, const VERTEX_BUFFER_VIEW_DESC& _desc, VertexBufferViewVk** _dst);

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

    const VERTEX_BUFFER_VIEW_DESC&
        B3D_APIENTRY GetDesc() const override;

    const BIND_VERTEX_BUFFERS_DATA&
        B3D_APIENTRY GetVertexBuffersData() const;

private:
    std::atomic_uint32_t                    ref_count;
    util::UniquePtr<util::NameableObjStr>   name;
    DeviceVk*                               device;
    VERTEX_BUFFER_VIEW_DESC                 desc;
    BufferVk*                               buffer;
    const InstancePFN*                      inspfn;
    const DevicePFN*                        devpfn;

    util::DyArray<VkBuffer>                 buffers;
    util::DyArray<VkDeviceSize>             offsets;
    util::DyArray<VkDeviceSize>             sizes;
    util::DyArray<VkDeviceSize>             strides;
    BIND_VERTEX_BUFFERS_DATA                bind_vertex_buffers_data;

};


}// namespace buma3d
