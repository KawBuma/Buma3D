#include "Buma3DPCH.h"
#include "VertexBufferViewVk.h"

namespace buma3d
{

static constexpr VIEW_DESC DEFAULT_VBV_DESC = { VIEW_TYPE_VERTEX_BUFFER, RESOURCE_FORMAT_UNKNOWN, VIEW_DIMENSION_VERTEX_BUFFER };

B3D_APIENTRY VertexBufferViewVk::VertexBufferViewVk()
    : ref_count                 { 1 }
    , name                      {}
    , device                    {}
    , desc                      {}
    , buffer                    {}
    , inspfn                    {}
    , devpfn                    {}
    , bind_vertex_buffers_data  {}
    , buffers                   {}
    , offsets                   {}
{

}

B3D_APIENTRY VertexBufferViewVk::~VertexBufferViewVk()
{
    Uninit();
}

BMRESULT
B3D_APIENTRY VertexBufferViewVk::Init(DeviceVk* _device, IBuffer* _buffer, const VERTEX_BUFFER_VIEW_DESC& _desc)
{
    (device = _device)->AddRef();
    inspfn = &device->GetInstancePFN();
    devpfn = &device->GetDevicePFN();

    (buffer = _buffer->As<BufferVk>())->AddRef();
    CopyDesc(_desc);

    bind_vertex_buffers_data.binding_count = desc.num_input_slots;
    buffers .resize(bind_vertex_buffers_data.binding_count);
    offsets .resize(bind_vertex_buffers_data.binding_count);
    bind_vertex_buffers_data.buffers = buffers.data();
    bind_vertex_buffers_data.offsets = offsets.data();
    auto buffers_data = buffers.data();
    auto offsets_data = offsets.data();

    auto buffer_size = buffer->GetDesc().buffer.size_in_bytes;
    for (uint32_t i = 0; i < desc.num_input_slots; i++)
    {
        auto buffer_offset   = desc.buffer_offsets[i];
        auto size_in_bytes   = desc.sizes_in_bytes[i];
        if (buffer_size < buffer_offset ||
            buffer_size < (buffer_offset + (uint64_t)size_in_bytes))
        {
            return BMRESULT_FAILED_RESOURCE_SIZE_EXCEEDED;
        }

        buffers_data[i] = buffer->GetVkBuffer();
        offsets_data[i] = SCAST<VkDeviceSize>(buffer_offset);
    }

    // VK_EXT_extended_dynamic_state拡張により、コマンドバッファへの頂点バインド時にストライドを指定することが可能です。
    if (devpfn->vkCmdBindVertexBuffers2EXT)
    {
        sizes   .resize(bind_vertex_buffers_data.binding_count);
        strides .resize(bind_vertex_buffers_data.binding_count);
        bind_vertex_buffers_data.sizes   = sizes.data();
        bind_vertex_buffers_data.strides = strides.data();
        auto sizes_data   = sizes.data();
        auto strides_data = strides.data();
        for (uint32_t i = 0; i < desc.num_input_slots; i++)
        {
            sizes_data[i] = SCAST<VkDeviceSize>(desc.sizes_in_bytes[i]);
            strides_data[i] = SCAST<VkDeviceSize>(desc.strides_in_bytes[i]);
        }
    }

    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY VertexBufferViewVk::CopyDesc(const VERTEX_BUFFER_VIEW_DESC& _desc)
{
    desc = _desc;
    desc.buffer_offsets   = util::MemCopyArray(B3DNewArray(uint64_t, _desc.num_input_slots), _desc.buffer_offsets   , _desc.num_input_slots);
    desc.sizes_in_bytes   = util::MemCopyArray(B3DNewArray(uint32_t, _desc.num_input_slots), _desc.sizes_in_bytes   , _desc.num_input_slots);
    desc.strides_in_bytes = util::MemCopyArray(B3DNewArray(uint32_t, _desc.num_input_slots), _desc.strides_in_bytes , _desc.num_input_slots);
}

void
B3D_APIENTRY VertexBufferViewVk::Uninit()
{
    B3DSafeDeleteArray(desc.buffer_offsets);
    B3DSafeDeleteArray(desc.sizes_in_bytes);
    B3DSafeDeleteArray(desc.strides_in_bytes);

    bind_vertex_buffers_data = {};
    hlp::SwapClear(buffers);
    hlp::SwapClear(offsets);
    hlp::SwapClear(sizes);
    hlp::SwapClear(strides);

    hlp::SafeRelease(buffer);
    hlp::SafeRelease(device);
    devpfn = nullptr;
    inspfn = nullptr;

    name.reset();
    desc = {};
}

BMRESULT
B3D_APIENTRY VertexBufferViewVk::Create(DeviceVk* _device, IBuffer* _buffer, const VERTEX_BUFFER_VIEW_DESC& _desc, VertexBufferViewVk** _dst)
{
    util::Ptr<VertexBufferViewVk> ptr;
    ptr.Attach(B3DCreateImplementationClass(VertexBufferViewVk));
    B3D_RET_IF_FAILED(ptr->Init(_device, _buffer, _desc));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY VertexBufferViewVk::AddRef()
{
    ++ref_count;
    B3D_REFCOUNT_DEBUG(ref_count);
}

uint32_t
B3D_APIENTRY VertexBufferViewVk::Release()
{
    B3D_REFCOUNT_DEBUG(ref_count - 1);
    auto count = --ref_count;
    if (count == 0)
        B3DDestroyImplementationClass(this);

    return count;
}

uint32_t
B3D_APIENTRY VertexBufferViewVk::GetRefCount() const
{
    return ref_count;
}

const char*
B3D_APIENTRY VertexBufferViewVk::GetName() const
{
    return name ? name->c_str() : nullptr;
}

BMRESULT
B3D_APIENTRY VertexBufferViewVk::SetName(const char* _name)
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
B3D_APIENTRY VertexBufferViewVk::GetDevice() const
{
    return device;
}

const BUFFER_VIEW*
B3D_APIENTRY VertexBufferViewVk::GetBufferView() const
{
    return nullptr;
}

const TEXTURE_VIEW*
B3D_APIENTRY VertexBufferViewVk::GetTextureView() const
{
    return nullptr;
}

const VkAllocationCallbacks*
B3D_APIENTRY VertexBufferViewVk::GetVkAllocationCallbacks() const
{
    return device->GetVkAllocationCallbacks();
}

const InstancePFN&
B3D_APIENTRY VertexBufferViewVk::GetIsntancePFN() const
{
    return *inspfn;
}

const DevicePFN&
B3D_APIENTRY VertexBufferViewVk::GetDevicePFN() const
{
    return *devpfn;
}

const VIEW_DESC&
B3D_APIENTRY VertexBufferViewVk::GetViewDesc() const
{
    return DEFAULT_VBV_DESC;
}

IResource*
B3D_APIENTRY VertexBufferViewVk::GetResource() const
{
    return buffer;
}

const VERTEX_BUFFER_VIEW_DESC&
B3D_APIENTRY VertexBufferViewVk::GetDesc() const 
{
    return desc;
}

const VertexBufferViewVk::BIND_VERTEX_BUFFERS_DATA&
B3D_APIENTRY VertexBufferViewVk::GetVertexBuffersData() const
{
    return bind_vertex_buffers_data;
}


}// namespace buma3d
