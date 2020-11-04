#include "Buma3DPCH.h"
#include "VertexBufferViewD3D12.h"

namespace buma3d
{

static constexpr VIEW_DESC DEFAULT_VBV_DESC = { VIEW_TYPE_VERTEX_BUFFER, RESOURCE_FORMAT_UNKNOWN, VIEW_DIMENSION_VERTEX_BUFFER };

B3D_APIENTRY VertexBufferViewD3D12::VertexBufferViewD3D12()
    : ref_count             { 1 }
    , name                  {}
    , device                {}
    , desc                  {}
    , buffer                {}
    , device12              {}
    , vertex_buffer_views12 {}
{

}

B3D_APIENTRY VertexBufferViewD3D12::~VertexBufferViewD3D12()
{
    Uninit();
}

BMRESULT
B3D_APIENTRY VertexBufferViewD3D12::Init(DeviceD3D12* _device, IBuffer* _buffer, const VERTEX_BUFFER_VIEW_DESC& _desc)
{
    (device = _device)->AddRef();
    device12 = device->GetD3D12Device();

    (buffer = _buffer->As<BufferD3D12>())->AddRef();
    CopyDesc(_desc);

    vertex_buffer_views12.resize(desc.num_input_slots);
    auto vbvs12 = vertex_buffer_views12.data();

    auto gpu_addr = buffer->GetGPUVirtualAddress();
    auto buffer_size = buffer->GetDesc().buffer.size_in_bytes;
    for (uint32_t i = 0; i < desc.num_input_slots; i++)
    {
        auto buffer_offset   = desc.buffer_offsets[i];
        auto size_in_bytes   = desc.sizes_in_bytes[i];
        auto stride_in_bytes = desc.strides_in_bytes[i];
        if (buffer_size < buffer_offset ||
            buffer_size < (buffer_offset + (uint64_t)size_in_bytes))
        {
            return BMRESULT_FAILED_RESOURCE_SIZE_EXCEEDED;
        }

        vbvs12[i].BufferLocation = gpu_addr + buffer_offset;
        vbvs12[i].SizeInBytes    = size_in_bytes;
        vbvs12[i].StrideInBytes  = stride_in_bytes;
    }

    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY VertexBufferViewD3D12::CopyDesc(const VERTEX_BUFFER_VIEW_DESC& _desc)
{
    desc = _desc;
    desc.buffer_offsets   = util::MemCopyArray(B3DNewArray(uint64_t, _desc.num_input_slots), _desc.buffer_offsets   , _desc.num_input_slots);
    desc.sizes_in_bytes   = util::MemCopyArray(B3DNewArray(uint32_t, _desc.num_input_slots), _desc.sizes_in_bytes   , _desc.num_input_slots);
    desc.strides_in_bytes = util::MemCopyArray(B3DNewArray(uint32_t, _desc.num_input_slots), _desc.strides_in_bytes , _desc.num_input_slots);
}

void
B3D_APIENTRY VertexBufferViewD3D12::Uninit()
{
    name.reset();

    B3DSafeDeleteArray(desc.buffer_offsets);
    B3DSafeDeleteArray(desc.sizes_in_bytes);
    B3DSafeDeleteArray(desc.strides_in_bytes);
    desc = {};

    hlp::SwapClear(vertex_buffer_views12);

    hlp::SafeRelease(buffer);
    hlp::SafeRelease(device);
}

BMRESULT
B3D_APIENTRY VertexBufferViewD3D12::Create(DeviceD3D12* _device, IBuffer* _buffer, const VERTEX_BUFFER_VIEW_DESC& _desc, VertexBufferViewD3D12** _dst)
{
    util::Ptr<VertexBufferViewD3D12> ptr;
    ptr.Attach(B3DCreateImplementationClass(VertexBufferViewD3D12));
    B3D_RET_IF_FAILED(ptr->Init(_device, _buffer, _desc));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY VertexBufferViewD3D12::AddRef()
{
    ++ref_count;
    B3D_REFCOUNT_DEBUG(ref_count);
}

uint32_t
B3D_APIENTRY VertexBufferViewD3D12::Release()
{
    B3D_REFCOUNT_DEBUG(ref_count - 1);
    auto count = --ref_count;
    if (count == 0)
        B3DDestroyImplementationClass(this);

    return count;
}

uint32_t
B3D_APIENTRY VertexBufferViewD3D12::GetRefCount() const
{
    return ref_count;
}

const char*
B3D_APIENTRY VertexBufferViewD3D12::GetName() const
{
    return name ? name->c_str() : nullptr;
}

BMRESULT
B3D_APIENTRY VertexBufferViewD3D12::SetName(const char* _name)
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
B3D_APIENTRY VertexBufferViewD3D12::GetDevice() const
{
    return device;
}

const CPU_DESCRIPTOR_ALLOCATION*
B3D_APIENTRY VertexBufferViewD3D12::GetCpuDescriptorAllocation() const
{
    return nullptr;
}

const TEXTURE_VIEW*
B3D_APIENTRY VertexBufferViewD3D12::GetTextureView() const
{
    return nullptr;
}

const BUFFER_VIEW*
B3D_APIENTRY VertexBufferViewD3D12::GetBufferView() const
{
    return nullptr;
}

const D3D12_GPU_VIRTUAL_ADDRESS*
B3D_APIENTRY VertexBufferViewD3D12::GetGpuVirtualAddress() const
{
    return nullptr;
}

const VIEW_DESC&
B3D_APIENTRY VertexBufferViewD3D12::GetViewDesc() const
{
    return DEFAULT_VBV_DESC;
}

IResource*
B3D_APIENTRY VertexBufferViewD3D12::GetResource() const
{
    return buffer;
}

const VERTEX_BUFFER_VIEW_DESC&
B3D_APIENTRY VertexBufferViewD3D12::GetDesc() const
{
    return desc;
}

const util::DyArray<D3D12_VERTEX_BUFFER_VIEW>&
B3D_APIENTRY VertexBufferViewD3D12::GetD3D12VertexBufferViews() const
{
    return vertex_buffer_views12;
}


}// namespace buma3d
