#include "Buma3DPCH.h"
#include "DescriptorHeapVk.h"

namespace buma3d
{

B3D_APIENTRY DescriptorHeapVk::DescriptorHeapVk()
    : ref_count     { 1 }
    , name          {}
    , device        {}
    , desc          {}
    , desc_data     {}
    , heap_remains  {}
    , vkdevice      {}
    , inspfn        {}
    , devpfn        {}
{     
      
}

B3D_APIENTRY DescriptorHeapVk::~DescriptorHeapVk()
{
    Uninit();
}

BMRESULT
B3D_APIENTRY DescriptorHeapVk::Init(DeviceVk* _device, const DESCRIPTOR_HEAP_DESC& _desc)
{
    // HACK: 現在、DescriptorHeapVkはD3D12との互換性を保つために使用されます。
    //       各タイプのID3D12DescriptorHeapがコマンドリストに1つしか設定出来ない問題と、1つのVkDescriptorPoolからのディスクリプタセット割り当てが同期されている必要がある問題を解消します。

    (device = _device)->AddRef();
    inspfn = &device->GetInstancePFN();
    devpfn = &device->GetDevicePFN();
    vkdevice = device->GetVkDevice();

    if (util::HasSameDescriptorType(_desc.num_heap_sizes, _desc.heap_sizes))
    {
        B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_INITIALIZATION
                          , "DESCRIPTOR_HEAP_DESC::heap_sizesの各要素のtypeは一意である必要があります。");
        return BMRESULT_FAILED_INVALID_PARAMETER;
    }
    CopyDesc(_desc);

    for (auto& i : desc_data->heap_sizes)
        heap_remains[i.type] += i.num_descriptors;
    
    return BMRESULT_SUCCEED;
}

void 
B3D_APIENTRY DescriptorHeapVk::CopyDesc(const DESCRIPTOR_HEAP_DESC& _desc)
{
    desc = _desc;

    desc_data = B3DMakeUnique(DESC_DATA);
    desc_data->heap_sizes.resize(_desc.num_heap_sizes);
    desc.heap_sizes = util::MemCopyArray(desc_data->heap_sizes.data(), _desc.heap_sizes, _desc.num_heap_sizes);
}

void
B3D_APIENTRY DescriptorHeapVk::Uninit()
{
    heap_remains.fill(0);

    desc = {};
    desc_data.reset();

    hlp::SafeRelease(device);
    vkdevice = VK_NULL_HANDLE;
    inspfn = nullptr;
    devpfn = nullptr;

    name.reset();
}

BMRESULT 
B3D_APIENTRY DescriptorHeapVk::Create(DeviceVk* _device, const DESCRIPTOR_HEAP_DESC& _desc, DescriptorHeapVk** _dst)
{
    util::Ptr<DescriptorHeapVk> ptr;
    ptr.Attach(B3DCreateImplementationClass(DescriptorHeapVk));
    B3D_RET_IF_FAILED(ptr->Init(_device, _desc));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY DescriptorHeapVk::AddRef()
{
    ++ref_count;
    B3D_REFCOUNT_DEBUG(ref_count);
}

uint32_t
B3D_APIENTRY DescriptorHeapVk::Release()
{
    B3D_REFCOUNT_DEBUG(ref_count - 1);
    auto count = --ref_count;
    if (count == 0)
        B3DDestroyImplementationClass(this);

    return count;
}

uint32_t
B3D_APIENTRY DescriptorHeapVk::GetRefCount() const
{
    return ref_count;
}

const char*
B3D_APIENTRY DescriptorHeapVk::GetName() const
{
    return name ? name->c_str() : nullptr;
}

BMRESULT
B3D_APIENTRY DescriptorHeapVk::SetName(const char* _name)
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
B3D_APIENTRY DescriptorHeapVk::GetDevice() const
{
    return device;
}

const VkAllocationCallbacks*
B3D_APIENTRY DescriptorHeapVk::GetVkAllocationCallbacks() const
{
    return device->GetVkAllocationCallbacks();
}

const InstancePFN&
B3D_APIENTRY DescriptorHeapVk::GetIsntancePFN() const
{
    return *inspfn;
}

const DevicePFN&
B3D_APIENTRY DescriptorHeapVk::GetDevicePFN() const
{
    return *devpfn;
}

const DESCRIPTOR_HEAP_DESC&
B3D_APIENTRY DescriptorHeapVk::GetDesc() const
{
    return desc;
}

BMRESULT
B3D_APIENTRY DescriptorHeapVk::AllocateDescriptors(const util::DyArray<DESCRIPTOR_POOL_SIZE>& _pool_sizes)
{
    if (!IsAllocatable(_pool_sizes))
        return BMRESULT_FAILED_OUT_OF_POOL_MEMORY;

    DecrementRemains(_pool_sizes);
    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY DescriptorHeapVk::FreeDescriptors(const util::DyArray<DESCRIPTOR_POOL_SIZE>& _pool_sizes)
{
    IncrementRemains(_pool_sizes);
}

bool
B3D_APIENTRY DescriptorHeapVk::IsAllocatable(const util::DyArray<DESCRIPTOR_POOL_SIZE>& _pool_sizes) const
{
    for (auto& i : _pool_sizes)
    {
        if (i.num_descriptors > heap_remains[i.type])
            return false;
    }
    return true;
}

void
B3D_APIENTRY DescriptorHeapVk::DecrementRemains(const util::DyArray<DESCRIPTOR_POOL_SIZE>& _pool_sizes)
{
    for (auto& i : _pool_sizes)
        heap_remains[i.type] -= i.num_descriptors;
}

void
B3D_APIENTRY DescriptorHeapVk::IncrementRemains(const util::DyArray<DESCRIPTOR_POOL_SIZE>& _pool_sizes)
{
    for (auto& i : _pool_sizes)
        heap_remains[i.type] += i.num_descriptors;
}


}// namespace buma3d
