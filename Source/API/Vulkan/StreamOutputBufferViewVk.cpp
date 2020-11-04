#include "Buma3DPCH.h"
#include "StreamOutputBufferViewVk.h"

namespace buma3d
{

static constexpr VIEW_DESC DEFAULT_SOBV_DESC = { VIEW_TYPE_STREAM_OUTPUT_BUFFER, RESOURCE_FORMAT_UNKNOWN, VIEW_DIMENSION_STREAM_OUTPUT_BUFFER };

B3D_APIENTRY StreamOutputBufferViewVk::StreamOutputBufferViewVk()
    : ref_count                             { 1 }
    , name                                  {}
    , device                                {}
    , desc                                  {}
    , buffer                                {}
    , filled_size_counter_buffer            {}
    , inspfn                                {}
    , devpfn                                {}
    , buffers                               {}
    , offsets                               {}
    , sizes                                 {}
    , counter_buffers                       {}
    , counter_buffer_offsets                {}
    , bind_transform_feedback_buffers_data  {}
{

}

B3D_APIENTRY StreamOutputBufferViewVk::~StreamOutputBufferViewVk()
{
    Uninit();
}

BMRESULT
B3D_APIENTRY StreamOutputBufferViewVk::Init(DeviceVk* _device, IBuffer* _buffer, IBuffer* _filled_size_counter_buffer, const STREAM_OUTPUT_BUFFER_VIEW_DESC& _desc)
{
    (device = _device)->AddRef();
    inspfn = &device->GetInstancePFN();
    devpfn = &device->GetDevicePFN();

    (buffer = _buffer->As<BufferVk>())->AddRef();
    (filled_size_counter_buffer = _filled_size_counter_buffer->As<BufferVk>())->AddRef();
    CopyDesc(_desc);

    bind_transform_feedback_buffers_data.binding_count = desc.num_input_slots;
    buffers                  .resize(bind_transform_feedback_buffers_data.binding_count);
    offsets                  .resize(bind_transform_feedback_buffers_data.binding_count);
    sizes                    .resize(bind_transform_feedback_buffers_data.binding_count);
    counter_buffers          .resize(bind_transform_feedback_buffers_data.binding_count);
    counter_buffer_offsets   .resize(bind_transform_feedback_buffers_data.binding_count);
    bind_transform_feedback_buffers_data.buffers                = buffers               .data();
    bind_transform_feedback_buffers_data.offsets                = offsets               .data();
    bind_transform_feedback_buffers_data.sizes                  = sizes                 .data();
    bind_transform_feedback_buffers_data.counter_buffers        = counter_buffers       .data();
    bind_transform_feedback_buffers_data.counter_buffer_offsets = counter_buffer_offsets.data();

    auto buffers_data                = buffers               .data();
    auto offsets_data                = offsets               .data();
    auto sizes_data                  = sizes                 .data();
    auto counter_buffers_data        = counter_buffers       .data();
    auto counter_buffer_offsets_data = counter_buffer_offsets.data();

    auto buffer_size          = buffer->GetDesc().buffer.size_in_bytes;
    auto counter_buffer_size  = filled_size_counter_buffer->GetDesc().buffer.size_in_bytes;
    for (uint32_t i = 0; i < desc.num_input_slots; i++)
    {
        auto buffer_offset  = desc.buffer_offsets[i];
        auto size_in_bytes  = desc.sizes_in_bytes[i];
        auto counter_offset = desc.filled_size_counter_buffer_offsets[i];
        if (buffer_size         <  buffer_offset                            ||
            buffer_size         < (buffer_offset + (uint64_t)size_in_bytes) ||
            counter_buffer_size <  counter_offset                           ||
            counter_buffer_size < (counter_offset + 4ull))// カウンタバッファには少なくとも4バイトのサイズが必要です。
        {
            return BMRESULT_FAILED_RESOURCE_SIZE_EXCEEDED;
        }

        buffers_data                [i] = buffer->GetVkBuffer();
        offsets_data                [i] = buffer_offset;
        sizes_data                  [i] = size_in_bytes;
        counter_buffers_data        [i] = filled_size_counter_buffer->GetVkBuffer();
        counter_buffer_offsets_data [i] = counter_offset;
    }

    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY StreamOutputBufferViewVk::CopyDesc(const STREAM_OUTPUT_BUFFER_VIEW_DESC& _desc)
{
    desc                                    = _desc;
    desc.buffer_offsets                     = util::MemCopyArray(B3DNewArray(uint64_t, _desc.num_input_slots), _desc.buffer_offsets                     , _desc.num_input_slots);
    desc.sizes_in_bytes                     = util::MemCopyArray(B3DNewArray(uint64_t, _desc.num_input_slots), _desc.sizes_in_bytes                     , _desc.num_input_slots);
    desc.filled_size_counter_buffer_offsets = util::MemCopyArray(B3DNewArray(uint64_t, _desc.num_input_slots), _desc.filled_size_counter_buffer_offsets , _desc.num_input_slots);
}

void
B3D_APIENTRY StreamOutputBufferViewVk::Uninit()
{
    B3DSafeDeleteArray(desc.buffer_offsets);
    B3DSafeDeleteArray(desc.sizes_in_bytes);
    B3DSafeDeleteArray(desc.filled_size_counter_buffer_offsets);

    hlp::SwapClear(buffers);
    hlp::SwapClear(offsets);
    hlp::SwapClear(sizes);
    hlp::SwapClear(counter_buffers);
    hlp::SwapClear(counter_buffer_offsets);
    bind_transform_feedback_buffers_data = {};

    hlp::SafeRelease(buffer);
    hlp::SafeRelease(filled_size_counter_buffer);
    hlp::SafeRelease(device);
    inspfn = nullptr;
    devpfn = nullptr;

    name.reset();
    desc = {};
}

BMRESULT
B3D_APIENTRY StreamOutputBufferViewVk::Create(DeviceVk* _device, IBuffer* _buffer, IBuffer* _filled_size_counter_buffer, const STREAM_OUTPUT_BUFFER_VIEW_DESC& _desc, StreamOutputBufferViewVk** _dst)
{
    util::Ptr<StreamOutputBufferViewVk> ptr;
    ptr.Attach(B3DCreateImplementationClass(StreamOutputBufferViewVk));
    B3D_RET_IF_FAILED(ptr->Init(_device, _buffer, _filled_size_counter_buffer, _desc));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY StreamOutputBufferViewVk::AddRef()
{
    ++ref_count;
    B3D_REFCOUNT_DEBUG(ref_count);
}

uint32_t
B3D_APIENTRY StreamOutputBufferViewVk::Release()
{
    B3D_REFCOUNT_DEBUG(ref_count - 1);
    auto count = --ref_count;
    if (count == 0)
        B3DDestroyImplementationClass(this);

    return count;
}

uint32_t
B3D_APIENTRY StreamOutputBufferViewVk::GetRefCount() const
{
    return ref_count;
}

const char*
B3D_APIENTRY StreamOutputBufferViewVk::GetName() const
{
    return name ? name->c_str() : nullptr;
}

BMRESULT
B3D_APIENTRY StreamOutputBufferViewVk::SetName(const char* _name)
{
    if (!util::IsEnabledDebug(this))
        return BMRESULT_FAILED;

    if (name && !_name)
        name.reset();
    else
        name = B3DMakeUniqueArgs(util::NameableObjStr, _name);

    return BMRESULT_SUCCEED;
}

IDevice*
B3D_APIENTRY StreamOutputBufferViewVk::GetDevice() const
{
    return device;
}

const TEXTURE_VIEW*
B3D_APIENTRY StreamOutputBufferViewVk::GetTextureView() const
{
    return nullptr;
}

const VkAllocationCallbacks*
B3D_APIENTRY StreamOutputBufferViewVk::GetVkAllocationCallbacks() const
{
    return device->GetVkAllocationCallbacks();
}

const InstancePFN&
B3D_APIENTRY StreamOutputBufferViewVk::GetIsntancePFN() const
{
    return *inspfn;
}

const DevicePFN&
B3D_APIENTRY StreamOutputBufferViewVk::GetDevicePFN() const
{
    return *devpfn;
}

const BUFFER_VIEW*
B3D_APIENTRY StreamOutputBufferViewVk::GetBufferView() const
{
    return nullptr;
}

const VIEW_DESC&
B3D_APIENTRY StreamOutputBufferViewVk::GetViewDesc() const
{
    return DEFAULT_SOBV_DESC;
}

IResource*
B3D_APIENTRY StreamOutputBufferViewVk::GetResource() const
{
    return buffer;
}

IBuffer*
B3D_APIENTRY StreamOutputBufferViewVk::GetFilledSizeCounterBuffer() const
{
    return filled_size_counter_buffer;
}

const STREAM_OUTPUT_BUFFER_VIEW_DESC&
B3D_APIENTRY StreamOutputBufferViewVk::GetDesc() const
{
    return desc;
}

const StreamOutputBufferViewVk::BIND_TRANSFORM_FEEDBACK_BUFFERS_DATA&
B3D_APIENTRY StreamOutputBufferViewVk::GetTransformFeedbackBuffersData() const
{
    return bind_transform_feedback_buffers_data;
}


}// namespace buma3d
