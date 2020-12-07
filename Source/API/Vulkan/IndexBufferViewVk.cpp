#include "Buma3DPCH.h"
#include "IndexBufferViewVk.h"

namespace buma3d
{

namespace util
{
namespace /*anonymous*/
{

inline VkIndexType GetNativeIndexType(INDEX_TYPE _type)
{    
    switch (_type)
    {
    case buma3d::INDEX_TYPE_UINT16 : return VK_INDEX_TYPE_UINT16;
    case buma3d::INDEX_TYPE_UINT32 : return VK_INDEX_TYPE_UINT32;
    case buma3d::INDEX_TYPE_UINT8  : return VK_INDEX_TYPE_UINT8_EXT;

    default:
        return VkIndexType(-1);
    }
}

}// namespace /*anonymous*/
}// namespace util


static constexpr VIEW_DESC DEFAULT_IBV_DESC = { VIEW_TYPE_INDEX_BUFFER, RESOURCE_FORMAT_UNKNOWN, VIEW_DIMENSION_INDEX_BUFFER };

B3D_APIENTRY IndexBufferViewVk::IndexBufferViewVk()
    : ref_count                 { 1 }
    , name                      {}
    , device                    {}
    , desc                      {}
    , buffer                    {}
    , vkdevice                  {}
    , inspfn                    {}
    , devpfn                    {}
    , bind_index_buffers_data   {}
{

}

B3D_APIENTRY IndexBufferViewVk::~IndexBufferViewVk()
{
    Uninit();
}

BMRESULT
B3D_APIENTRY IndexBufferViewVk::Init(DeviceVk* _device, IBuffer* _buffer, const INDEX_BUFFER_VIEW_DESC& _desc)
{
    (device = _device)->AddRef();
    vkdevice = _device->GetVkDevice();
    inspfn = &device->GetInstancePFN();
    devpfn = &device->GetDevicePFN();

    (buffer = _buffer->As<BufferVk>())->AddRef();
    CopyDesc(_desc);

    auto buffer_size = buffer->GetDesc().buffer.size_in_bytes;
    if (buffer_size < desc.buffer_offset ||
        buffer_size < (desc.buffer_offset + (uint64_t)desc.size_in_bytes))
    {
        return BMRESULT_FAILED_RESOURCE_SIZE_EXCEEDED;
    }

    bind_index_buffers_data = B3DNew(BIND_INDEX_BUFFERS_DATA);
    bind_index_buffers_data->buffer = buffer->GetVkBuffer();
    bind_index_buffers_data->offset = desc.buffer_offset;
    bind_index_buffers_data->type   = util::GetNativeIndexType(desc.index_type);

    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY IndexBufferViewVk::CopyDesc(const INDEX_BUFFER_VIEW_DESC& _desc)
{
    desc = _desc;
}

void
B3D_APIENTRY IndexBufferViewVk::Uninit()
{
    B3DSafeDelete(bind_index_buffers_data);

    hlp::SafeRelease(buffer);
    hlp::SafeRelease(device);
    vkdevice;
    inspfn;
    devpfn;

    name.reset();
    desc = {};
}

BMRESULT
B3D_APIENTRY IndexBufferViewVk::Create(DeviceVk* _device, IBuffer* _buffer, const INDEX_BUFFER_VIEW_DESC& _desc, IndexBufferViewVk** _dst)
{
    util::Ptr<IndexBufferViewVk> ptr;
    ptr.Attach(B3DCreateImplementationClass(IndexBufferViewVk));
    B3D_RET_IF_FAILED(ptr->Init(_device, _buffer, _desc));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY IndexBufferViewVk::AddRef()
{
    ++ref_count;
    B3D_REFCOUNT_DEBUG(ref_count);
}

uint32_t
B3D_APIENTRY IndexBufferViewVk::Release()
{
    B3D_REFCOUNT_DEBUG(ref_count - 1);
    auto count = --ref_count;
    if (count == 0)
        B3DDestroyImplementationClass(this);

    return count;
}

uint32_t
B3D_APIENTRY IndexBufferViewVk::GetRefCount() const
{
    return ref_count;
}

const char*
B3D_APIENTRY IndexBufferViewVk::GetName() const
{
    return name ? name->c_str() : nullptr;
}

BMRESULT
B3D_APIENTRY IndexBufferViewVk::SetName(const char* _name)
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
B3D_APIENTRY IndexBufferViewVk::GetDevice() const
{
    return device;
}

const BUFFER_VIEW*
B3D_APIENTRY IndexBufferViewVk::GetBufferView() const
{
    return nullptr;
}

const TEXTURE_VIEW*
B3D_APIENTRY IndexBufferViewVk::GetTextureView() const
{
    return nullptr;
}

const VkAllocationCallbacks*
B3D_APIENTRY IndexBufferViewVk::GetVkAllocationCallbacks() const
{
    return device->GetVkAllocationCallbacks();
}

const InstancePFN&
B3D_APIENTRY IndexBufferViewVk::GetIsntancePFN() const
{
    return *inspfn;
}

const DevicePFN&
B3D_APIENTRY IndexBufferViewVk::GetDevicePFN() const
{
    return *devpfn;
}

const VIEW_DESC&
B3D_APIENTRY IndexBufferViewVk::GetViewDesc() const
{
    return DEFAULT_IBV_DESC;
}

IResource*
B3D_APIENTRY IndexBufferViewVk::GetResource() const
{
    return buffer;
}

const INDEX_BUFFER_VIEW_DESC&
B3D_APIENTRY IndexBufferViewVk::GetDesc() const 
{
    return desc;
}

const IndexBufferViewVk::BIND_INDEX_BUFFERS_DATA&
B3D_APIENTRY IndexBufferViewVk::GetIndexBufferData() const
{
    return *bind_index_buffers_data;
}


}// namespace buma3d
