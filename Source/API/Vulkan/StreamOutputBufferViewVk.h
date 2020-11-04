#pragma once

namespace buma3d
{

class B3D_API StreamOutputBufferViewVk : public IDeviceChildVk<IStreamOutputBufferView>, public IViewVk, public util::details::NEW_DELETE_OVERRIDE
{
public:
    struct BIND_TRANSFORM_FEEDBACK_BUFFERS_DATA
    {
        uint32_t            binding_count;
        const VkBuffer*     buffers;
        const VkDeviceSize* offsets;
        const VkDeviceSize* sizes;
        const VkBuffer*     counter_buffers;
        const VkDeviceSize* counter_buffer_offsets;
    };

protected:
    B3D_APIENTRY StreamOutputBufferViewVk();
    StreamOutputBufferViewVk(const StreamOutputBufferViewVk&) = delete;
    B3D_APIENTRY ~StreamOutputBufferViewVk();

private:
    BMRESULT B3D_APIENTRY Init(DeviceVk* _device, IBuffer* _buffer, IBuffer* _filled_size_counter_buffer, const STREAM_OUTPUT_BUFFER_VIEW_DESC& _desc);
    void B3D_APIENTRY CopyDesc(const STREAM_OUTPUT_BUFFER_VIEW_DESC& _desc);
    void B3D_APIENTRY Uninit();

public:
    static BMRESULT
        B3D_APIENTRY Create(DeviceVk* _device, IBuffer* _buffer, IBuffer* _filled_size_counter_buffer, const STREAM_OUTPUT_BUFFER_VIEW_DESC& _desc, StreamOutputBufferViewVk** _dst);

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

    IBuffer*
        B3D_APIENTRY GetFilledSizeCounterBuffer() const override;

    const STREAM_OUTPUT_BUFFER_VIEW_DESC&
        B3D_APIENTRY GetDesc() const override;

    const BIND_TRANSFORM_FEEDBACK_BUFFERS_DATA&
        B3D_APIENTRY GetTransformFeedbackBuffersData() const;

private:
    std::atomic_uint32_t                    ref_count;
    util::UniquePtr<util::NameableObjStr>   name;
    DeviceVk*                               device;
    STREAM_OUTPUT_BUFFER_VIEW_DESC          desc;
    BufferVk*                               buffer;
    BufferVk*                               filled_size_counter_buffer;
    const InstancePFN*                      inspfn;
    const DevicePFN*                        devpfn;

    util::DyArray<VkBuffer>                 buffers;
    util::DyArray<VkDeviceSize>             offsets;
    util::DyArray<VkDeviceSize>             sizes;
    util::DyArray<VkBuffer>                 counter_buffers;
    util::DyArray<VkDeviceSize>             counter_buffer_offsets;
    BIND_TRANSFORM_FEEDBACK_BUFFERS_DATA    bind_transform_feedback_buffers_data;

};


}// namespace buma3d
