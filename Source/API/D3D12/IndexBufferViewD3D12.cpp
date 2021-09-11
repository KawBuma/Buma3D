#include "Buma3DPCH.h"
#include "IndexBufferViewD3D12.h"

namespace buma3d
{

static constexpr VIEW_DESC DEFAULT_IBV_DESC = { VIEW_TYPE_INDEX_BUFFER, RESOURCE_FORMAT_UNKNOWN, VIEW_DIMENSION_INDEX_BUFFER };

B3D_APIENTRY IndexBufferViewD3D12::IndexBufferViewD3D12()
    : ref_count         { 1 }
    , name              {}
    , device            {}
    , desc              {}
    , buffer            {}
    , device12          {}
    , index_buffer_view {}
{

}

B3D_APIENTRY IndexBufferViewD3D12::~IndexBufferViewD3D12()
{
    Uninit();
}

BMRESULT
B3D_APIENTRY IndexBufferViewD3D12::Init(DeviceD3D12* _device, IBuffer* _buffer, const INDEX_BUFFER_VIEW_DESC& _desc)
{
    (device = _device)->AddRef();
    device12 = device->GetD3D12Device();

    (buffer = _buffer->As<BufferD3D12>())->AddRef();
    CopyDesc(_desc);

    auto buffer_size = buffer->GetDesc().buffer.size_in_bytes;
    if (buffer_size < desc.buffer_offset ||
        buffer_size < (desc.buffer_offset + (uint64_t)desc.size_in_bytes))
    {
        return BMRESULT_FAILED_RESOURCE_SIZE_EXCEEDED;
    }

    index_buffer_view.BufferLocation = buffer->GetGPUVirtualAddress() + desc.buffer_offset;
    index_buffer_view.SizeInBytes    = desc.size_in_bytes;
    index_buffer_view.Format         = util::GetNativeIndexType(desc.index_type);

    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY IndexBufferViewD3D12::CopyDesc(const INDEX_BUFFER_VIEW_DESC& _desc)
{
    desc = _desc;
}

void
B3D_APIENTRY IndexBufferViewD3D12::Uninit()
{
    name.reset();
    desc = {};

    index_buffer_view = {};

    hlp::SafeRelease(buffer);
    hlp::SafeRelease(device);
}

BMRESULT
B3D_APIENTRY IndexBufferViewD3D12::Create(DeviceD3D12* _device, IBuffer* _buffer, const INDEX_BUFFER_VIEW_DESC& _desc, IndexBufferViewD3D12** _dst)
{
    util::Ptr<IndexBufferViewD3D12> ptr;
    ptr.Attach(B3DCreateImplementationClass(IndexBufferViewD3D12));
    B3D_RET_IF_FAILED(ptr->Init(_device, _buffer, _desc));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY IndexBufferViewD3D12::AddRef()
{
    ++ref_count;
    B3D_REFCOUNT_DEBUG(ref_count);
}

uint32_t
B3D_APIENTRY IndexBufferViewD3D12::Release()
{
    B3D_REFCOUNT_DEBUG(ref_count - 1);
    auto count = --ref_count;
    if (count == 0)
        B3DDestroyImplementationClass(this);

    return count;
}

uint32_t
B3D_APIENTRY IndexBufferViewD3D12::GetRefCount() const
{
    return ref_count;
}

const char*
B3D_APIENTRY IndexBufferViewD3D12::GetName() const
{
    return name ? name->c_str() : nullptr;
}

BMRESULT
B3D_APIENTRY IndexBufferViewD3D12::SetName(const char* _name)
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
B3D_APIENTRY IndexBufferViewD3D12::GetDevice() const
{
    return device;
}

const CPU_DESCRIPTOR_ALLOCATION*
B3D_APIENTRY IndexBufferViewD3D12::GetCpuDescriptorAllocation() const
{
    return nullptr;
}

const D3D12_GPU_VIRTUAL_ADDRESS*
B3D_APIENTRY IndexBufferViewD3D12::GetGpuVirtualAddress() const
{
    return nullptr;
}

bool
B3D_APIENTRY IndexBufferViewD3D12::HasAllSubresources() const
{
    return false;
}

const BUFFER_VIEW*
B3D_APIENTRY IndexBufferViewD3D12::GetBufferView() const
{
    return nullptr;
}

const TEXTURE_VIEW*
B3D_APIENTRY IndexBufferViewD3D12::GetTextureView() const
{
    return nullptr;
}

const VIEW_DESC&
B3D_APIENTRY IndexBufferViewD3D12::GetViewDesc() const
{
    return DEFAULT_IBV_DESC;
}

IResource*
B3D_APIENTRY IndexBufferViewD3D12::GetResource() const
{
    return buffer;
}

const INDEX_BUFFER_VIEW_DESC&
B3D_APIENTRY IndexBufferViewD3D12::GetDesc() const
{
    return desc;
}

const D3D12_INDEX_BUFFER_VIEW&
B3D_APIENTRY IndexBufferViewD3D12::GetD3D12IndexBufferView() const
{
    return index_buffer_view;
}


}// namespace buma3d
