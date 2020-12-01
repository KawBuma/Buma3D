#include "Buma3DPCH.h"
#include "ConstantBufferViewD3D12.h"

namespace buma3d
{

static constexpr VIEW_DESC DEFAULT_CBV_DESC = { VIEW_TYPE_CONSTANT_BUFFER, RESOURCE_FORMAT_UNKNOWN, VIEW_DIMENSION_CONSTANT_BUFFER };

B3D_APIENTRY ConstantBufferViewD3D12::ConstantBufferViewD3D12()
    : ref_count         { 1 }
    , name              {}
    , device            {}
    , desc              {}
    , buffer            {}
    , device12          {}
    , descriptor        {}
    , virtual_address   {}
{

}

B3D_APIENTRY ConstantBufferViewD3D12::~ConstantBufferViewD3D12()
{
    Uninit();
}

BMRESULT
B3D_APIENTRY ConstantBufferViewD3D12::Init(DeviceD3D12* _device, IBuffer* _buffer, const CONSTANT_BUFFER_VIEW_DESC& _desc)
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

    // D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT
    // TODO: データアライメントを抽象化し、アプリケーションに提供する。
    B3D_ADD_DEBUG_MSG_INFO_B3D("TODO: データアライメントを抽象化し、アプリケーションに提供する。(VkPhysicalDeviceLimitsの様な)");

    D3D12_CONSTANT_BUFFER_VIEW_DESC cbvdesc{};
    cbvdesc.BufferLocation = buffer->GetGPUVirtualAddress() + desc.buffer_offset;
    cbvdesc.SizeInBytes    = (UINT)desc.size_in_bytes;

    if (!hlp::IsAligned(cbvdesc.BufferLocation, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT))
        return BMRESULT_FAILED_INVALID_PARAMETER;

    descriptor = device->GetCPUDescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 0x1).Allocate();
    device12->CreateConstantBufferView(&cbvdesc, descriptor.handle);

    virtual_address = cbvdesc.BufferLocation;
    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY ConstantBufferViewD3D12::CopyDesc(const CONSTANT_BUFFER_VIEW_DESC& _desc)
{
    desc = _desc;
}

void
B3D_APIENTRY ConstantBufferViewD3D12::Uninit()
{
    if (descriptor.handle)
        device->GetCPUDescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, B3D_DEFAULT_NODE_MASK).Free(descriptor);
    descriptor = {};
    virtual_address = {};
    hlp::SafeRelease(buffer);
    hlp::SafeRelease(device);

    desc = {};
    name.reset();
}

BMRESULT
B3D_APIENTRY ConstantBufferViewD3D12::Create(DeviceD3D12* _device, IBuffer* _buffer, const CONSTANT_BUFFER_VIEW_DESC& _desc, ConstantBufferViewD3D12** _dst)
{
    util::Ptr<ConstantBufferViewD3D12> ptr;
    ptr.Attach(B3DCreateImplementationClass(ConstantBufferViewD3D12));
    B3D_RET_IF_FAILED(ptr->Init(_device, _buffer, _desc));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY ConstantBufferViewD3D12::AddRef() 
{
    ++ref_count;
    B3D_REFCOUNT_DEBUG(ref_count);
}

uint32_t
B3D_APIENTRY ConstantBufferViewD3D12::Release() 
{
    B3D_REFCOUNT_DEBUG(ref_count - 1);
    auto count = --ref_count;
    if (count == 0)
        B3DDestroyImplementationClass(this);

    return count;
}

uint32_t
B3D_APIENTRY ConstantBufferViewD3D12::GetRefCount() const 
{
    return ref_count;
}

const char*
B3D_APIENTRY ConstantBufferViewD3D12::GetName() const 
{
    return name ? name->c_str() : nullptr;
}

BMRESULT
B3D_APIENTRY ConstantBufferViewD3D12::SetName(const char* _name) 
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
B3D_APIENTRY ConstantBufferViewD3D12::GetDevice() const 
{
    return device;
}

const CPU_DESCRIPTOR_ALLOCATION*
B3D_APIENTRY ConstantBufferViewD3D12::GetCpuDescriptorAllocation() const
{
    return &descriptor;
}

const BUFFER_VIEW*
B3D_APIENTRY ConstantBufferViewD3D12::GetBufferView() const
{
    return nullptr;
}

const TEXTURE_VIEW*
B3D_APIENTRY ConstantBufferViewD3D12::GetTextureView() const
{
    return nullptr;
}

const D3D12_GPU_VIRTUAL_ADDRESS*
B3D_APIENTRY ConstantBufferViewD3D12::GetGpuVirtualAddress() const
{
    return &virtual_address;
}

const VIEW_DESC&
B3D_APIENTRY ConstantBufferViewD3D12::GetViewDesc() const 
{
    return DEFAULT_CBV_DESC;
}

IResource*
B3D_APIENTRY ConstantBufferViewD3D12::GetResource() const 
{
    return buffer;
}

const CONSTANT_BUFFER_VIEW_DESC&
B3D_APIENTRY ConstantBufferViewD3D12::GetDesc() const 
{
    return desc;
}


}// namespace buma3d
