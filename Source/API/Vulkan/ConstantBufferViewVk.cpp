#include "Buma3DPCH.h"
#include "ConstantBufferViewVk.h"

namespace buma3d
{

static constexpr VIEW_DESC DEFAULT_CBV_DESC = { VIEW_TYPE_CONSTANT_BUFFER, RESOURCE_FORMAT_UNKNOWN, VIEW_DIMENSION_CONSTANT_BUFFER };

B3D_APIENTRY ConstantBufferViewVk::ConstantBufferViewVk()
    : ref_count              { 1 }
    , name                   {}
    , device                 {}
    , desc                   {}
    , buffer                 {}
    , vkdevice               {}
    , inspfn                 {}
    , devpfn                 {}
    , descriptor_buffer_info {}
{

}

B3D_APIENTRY ConstantBufferViewVk::~ConstantBufferViewVk()
{
    Uninit();
}

BMRESULT
B3D_APIENTRY ConstantBufferViewVk::Init(DeviceVk* _device, IBuffer* _buffer, const CONSTANT_BUFFER_VIEW_DESC& _desc)
{
    (device = _device)->AddRef();
    inspfn = &device->GetInstancePFN();
    devpfn = &device->GetDevicePFN();

    (buffer = _buffer->As<BufferVk>())->AddRef();
    CopyDesc(_desc);

    auto buffer_size = buffer->GetDesc().buffer.size_in_bytes;
    if (buffer_size <  desc.buffer_offset ||
        buffer_size < (desc.buffer_offset + (uint64_t)desc.size_in_bytes))
    {
        return BMRESULT_FAILED_RESOURCE_SIZE_EXCEEDED;
    }

    descriptor_buffer_info.buffer = buffer->GetVkBuffer();
    descriptor_buffer_info.offset = desc.buffer_offset;
    descriptor_buffer_info.range  = desc.size_in_bytes;

    if (!hlp::IsAligned(descriptor_buffer_info.offset, device->GetDeviceAdapter()->GetPhysicalDeviceData().properties2.properties.limits.minUniformBufferOffsetAlignment))
        return BMRESULT_FAILED_INVALID_PARAMETER;

    if (descriptor_buffer_info.range >= device->GetDeviceAdapter()->GetPhysicalDeviceData().properties2.properties.limits.maxUniformBufferRange)
        return BMRESULT_FAILED_INVALID_PARAMETER;

    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY ConstantBufferViewVk::CopyDesc(const CONSTANT_BUFFER_VIEW_DESC& _desc)
{
    desc = _desc;
}

void
B3D_APIENTRY ConstantBufferViewVk::Uninit()
{
    name.reset();
    desc = {};

    descriptor_buffer_info = {};
    
    hlp::SafeRelease(buffer);
    hlp::SafeRelease(device);
}

BMRESULT
B3D_APIENTRY ConstantBufferViewVk::Create(DeviceVk* _device, IBuffer* _buffer, const CONSTANT_BUFFER_VIEW_DESC& _desc, ConstantBufferViewVk** _dst)
{
    util::Ptr<ConstantBufferViewVk> ptr;
    ptr.Attach(B3DCreateImplementationClass(ConstantBufferViewVk));
    B3D_RET_IF_FAILED(ptr->Init(_device, _buffer, _desc));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY ConstantBufferViewVk::AddRef() 
{
    ++ref_count;
    B3D_REFCOUNT_DEBUG(ref_count);
}

uint32_t
B3D_APIENTRY ConstantBufferViewVk::Release() 
{
    B3D_REFCOUNT_DEBUG(ref_count - 1);
    auto count = --ref_count;
    if (count == 0)
        B3DDestroyImplementationClass(this);

    return count;
}

uint32_t
B3D_APIENTRY ConstantBufferViewVk::GetRefCount() const 
{
    return ref_count;
}

const char*
B3D_APIENTRY ConstantBufferViewVk::GetName() const 
{
    return name ? name->c_str() : nullptr;
}

BMRESULT
B3D_APIENTRY ConstantBufferViewVk::SetName(const char* _name) 
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
B3D_APIENTRY ConstantBufferViewVk::GetDevice() const 
{
    return device;
}

const BUFFER_VIEW*
B3D_APIENTRY ConstantBufferViewVk::GetBufferView() const
{
    return nullptr;
}

const TEXTURE_VIEW*
B3D_APIENTRY ConstantBufferViewVk::GetTextureView() const
{
    return nullptr;
}

const VkAllocationCallbacks*
B3D_APIENTRY ConstantBufferViewVk::GetVkAllocationCallbacks() const
{
    return device->GetVkAllocationCallbacks();
}

const InstancePFN&
B3D_APIENTRY ConstantBufferViewVk::GetIsntancePFN() const
{
    return *inspfn;
}

const DevicePFN&
B3D_APIENTRY ConstantBufferViewVk::GetDevicePFN() const
{
    return *devpfn;
}

const VkDescriptorBufferInfo*
B3D_APIENTRY ConstantBufferViewVk::GetVkDescriptorBufferInfo() const
{
    return &descriptor_buffer_info;
}

BMRESULT
B3D_APIENTRY ConstantBufferViewVk::AddDescriptorWriteRange(void* _dst, uint32_t _array_index) const
{
    auto&& dst = *RCAST<DescriptorSetVk::UPDATE_DESCRIPTOR_RANGE_BUFFER*>(_dst);
    if (!dst.buffer_infos_data)
        return BMRESULT_FAILED;
    dst.buffer_infos_data[_array_index] = descriptor_buffer_info;

    return BMRESULT_SUCCEED;
}

const VIEW_DESC&
B3D_APIENTRY ConstantBufferViewVk::GetViewDesc() const 
{
    return DEFAULT_CBV_DESC;
}

IResource*
B3D_APIENTRY ConstantBufferViewVk::GetResource() const 
{
    return buffer;
}

const CONSTANT_BUFFER_VIEW_DESC&
B3D_APIENTRY ConstantBufferViewVk::GetDesc() const 
{
    return desc;
}


}// namespace buma3d
