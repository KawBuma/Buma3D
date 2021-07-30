#include "Buma3DPCH.h"
#include "DescriptorPoolVk.h"
#include "DescriptorSetVk.h"

namespace buma3d
{

B3D_APIENTRY DescriptorPoolVk::DescriptorPoolVk()
    : ref_count         { 1 }
    , name              {}
    , device            {}
    , desc              {}
    , desc_data         {}
    , parent_heap       {}
    , allocation_count  {}
    , reset_id          {}
    , vkdevice          {}
    , inspfn            {}
    , devpfn            {}
    , descriptor_pool   {}
{

}

B3D_APIENTRY DescriptorPoolVk::~DescriptorPoolVk()
{
    Uninit();
}

BMRESULT
B3D_APIENTRY DescriptorPoolVk::Init(DeviceVk* _device, const DESCRIPTOR_POOL_DESC& _desc)
{
    (device = _device)->AddRef();
    inspfn = &device->GetInstancePFN();
    devpfn = &device->GetDevicePFN();
    vkdevice = device->GetVkDevice();

    if (util::HasSameDescriptorType(_desc.num_pool_sizes, _desc.pool_sizes))
    {
        B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_INITIALIZATION
                          , "DESCRIPTOR_HEAP_DESC::heap_sizesの各要素のtypeは一意である必要があります。");
        return BMRESULT_FAILED_INVALID_PARAMETER;
    }
    CopyDesc(_desc);

    B3D_RET_IF_FAILED(parent_heap->AllocateDescriptors(desc_data->pool_sizes));

    B3D_RET_IF_FAILED(CreateVkDescriptorPool());

    for (auto& i : desc_data->pool_sizes)
        pool_remains[i.type] = i.num_descriptors;

    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY DescriptorPoolVk::CopyDesc(const DESCRIPTOR_POOL_DESC& _desc)
{
    desc = _desc;

    (parent_heap = _desc.heap->As<DescriptorHeapVk>())->AddRef();

    desc_data = B3DMakeUnique(DESC_DATA);
    desc_data->pool_sizes.resize(_desc.num_pool_sizes);
    desc.pool_sizes = util::MemCopyArray(desc_data->pool_sizes.data(), _desc.pool_sizes, _desc.num_pool_sizes);
}

void
B3D_APIENTRY DescriptorPoolVk::GetNativePoolSizes(util::DyArray<VkDescriptorPoolSize>* _sises)
{
    _sises->resize(desc.num_pool_sizes);
    auto _sizes_data = _sises->data();
    for (uint32_t i = 0; i < desc.num_pool_sizes; i++)
    {
        auto&& ps = desc.pool_sizes[i];
        _sizes_data[i].type            = util::GetNativeDescriptorType(ps.type);
        _sizes_data[i].descriptorCount = ps.num_descriptors;
    }
}

BMRESULT
B3D_APIENTRY DescriptorPoolVk::CreateVkDescriptorPool()
{
    util::DyArray<VkDescriptorPoolSize> sizes;
    GetNativePoolSizes(&sizes);

    VkDescriptorPoolCreateInfo ci{ VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
    ci.flags         = util::GetNativeDescriptorPoolFlags(desc.flags);
    ci.maxSets       = desc.max_sets_allocation_count;
    ci.poolSizeCount = desc.num_pool_sizes;
    ci.pPoolSizes    = sizes.data();
    auto vkr = vkCreateDescriptorPool(vkdevice, &ci, B3D_VK_ALLOC_CALLBACKS, &descriptor_pool);
    B3D_RET_IF_FAILED(VKR_TRACE_IF_FAILED(vkr));

    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY DescriptorPoolVk::Uninit()
{
    if (parent_heap)
        parent_heap->FreeDescriptors(desc_data->pool_sizes);
    hlp::SafeRelease(parent_heap);

    if (descriptor_pool)
        vkDestroyDescriptorPool(vkdevice, descriptor_pool, B3D_VK_ALLOC_CALLBACKS);
    descriptor_pool = VK_NULL_HANDLE;

    desc = {};
    desc_data.reset();

    hlp::SafeRelease(device);
    vkdevice = VK_NULL_HANDLE;
    inspfn = nullptr;
    devpfn = nullptr;

    name.reset();
}

BMRESULT
B3D_APIENTRY DescriptorPoolVk::Create(DeviceVk* _device, const DESCRIPTOR_POOL_DESC& _desc, DescriptorPoolVk** _dst)
{
    util::Ptr<DescriptorPoolVk> ptr;
    ptr.Attach(B3DCreateImplementationClass(DescriptorPoolVk));
    B3D_RET_IF_FAILED(ptr->Init(_device, _desc));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY DescriptorPoolVk::AddRef()
{
    ++ref_count;
    B3D_REFCOUNT_DEBUG(ref_count);
}

uint32_t
B3D_APIENTRY DescriptorPoolVk::Release()
{
    B3D_REFCOUNT_DEBUG(ref_count - 1);
    auto count = --ref_count;
    if (count == 0)
        B3DDestroyImplementationClass(this);

    return count;
}

uint32_t
B3D_APIENTRY DescriptorPoolVk::GetRefCount() const
{
    return ref_count;
}

const char*
B3D_APIENTRY DescriptorPoolVk::GetName() const
{
    return name ? name->c_str() : nullptr;
}

BMRESULT
B3D_APIENTRY DescriptorPoolVk::SetName(const char* _name)
{
    if (!util::IsEnabledDebug(this))
        return BMRESULT_FAILED;

    if (descriptor_pool)
        B3D_RET_IF_FAILED(device->SetVkObjectName(descriptor_pool, _name));

    if (name && !_name)
        name.reset();
    else
        name = B3DMakeUniqueArgs(util::NameableObjStr, _name);

    return BMRESULT_SUCCEED;
}

IDevice*
B3D_APIENTRY DescriptorPoolVk::GetDevice() const
{
    return device;
}

const VkAllocationCallbacks*
B3D_APIENTRY DescriptorPoolVk::GetVkAllocationCallbacks() const
{
    return device->GetVkAllocationCallbacks();
}

const InstancePFN&
B3D_APIENTRY DescriptorPoolVk::GetIsntancePFN() const
{
    return *inspfn;
}

const DevicePFN&
B3D_APIENTRY DescriptorPoolVk::GetDevicePFN() const
{
    return *devpfn;
}

const DESCRIPTOR_POOL_DESC&
B3D_APIENTRY DescriptorPoolVk::GetDesc() const
{
    return desc;
}

uint32_t
B3D_APIENTRY DescriptorPoolVk::GetCurrentAllocationCount()
{
    return allocation_count;
}

void
B3D_APIENTRY DescriptorPoolVk::ResetPoolAndInvalidateAllocatedSets()
{
    auto vkr = vkResetDescriptorPool(vkdevice, descriptor_pool, 0/*reserved*/);
    VKR_TRACE_IF_FAILED(vkr);

    allocation_count = 0;
    ++reset_id;

    for (auto& i : desc_data->pool_sizes)
        pool_remains[i.type] = i.num_descriptors;
}

BMRESULT
B3D_APIENTRY DescriptorPoolVk::AllocateDescriptorSets(const DESCRIPTOR_SET_ALLOCATE_DESC& _desc, IDescriptorSet** _dst_descriptor_sets)
{
    util::DyArray<VkDescriptorSet> setsvk(_desc.num_descriptor_sets);
    auto setsvk_data = setsvk.data();
    B3D_RET_IF_FAILED(AllocateVkDescriptorSets(_desc, setsvk_data));

    util::DyArray<util::Ptr<DescriptorSetVk>> sets(_desc.num_descriptor_sets);
    auto sets_data = sets.data();
    for (uint32_t i = 0; i < _desc.num_descriptor_sets; i++)
    {
        auto l = _desc.set_layouts[i]->As<DescriptorSetLayoutVk>();
        B3D_RET_IF_FAILED(DescriptorSetVk::Create(l, this, setsvk_data[i], &sets_data[i]));
    }

    for (uint32_t i = 0; i < _desc.num_descriptor_sets; i++)
    {
        _dst_descriptor_sets[i] = sets_data[i].Detach();
    }
    return BMRESULT_SUCCEED;
}

VkDescriptorPool
B3D_APIENTRY DescriptorPoolVk::GetVkDescriptorPool() const
{
    return descriptor_pool;
}

uint64_t
B3D_APIENTRY DescriptorPoolVk::GetResetID()
{
    return reset_id;
}

BMRESULT
B3D_APIENTRY DescriptorPoolVk::AllocateDescriptors(const util::DyArray<DESCRIPTOR_POOL_SIZE>& _pool_sizes)
{
    if (!IsAllocatable(_pool_sizes))
        return BMRESULT_FAILED_OUT_OF_POOL_MEMORY;

    ++allocation_count;
    DecrementRemains(_pool_sizes);
    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY DescriptorPoolVk::FreeDescriptors(const util::DyArray<DESCRIPTOR_POOL_SIZE>& _pool_sizes)
{
    --allocation_count;
    IncrementRemains(_pool_sizes);
}

bool
B3D_APIENTRY DescriptorPoolVk::IsAllocatable(const util::DyArray<DESCRIPTOR_POOL_SIZE>& _pool_sizes) const
{
    if (allocation_count > desc.max_sets_allocation_count)
        return false;

    for (auto& i : _pool_sizes)
    {
        if (i.num_descriptors > pool_remains[i.type])
            return false;
    }
    return true;
}

void
B3D_APIENTRY DescriptorPoolVk::DecrementRemains(const util::DyArray<DESCRIPTOR_POOL_SIZE>& _pool_sizes)
{
    for (auto& i : _pool_sizes)
        pool_remains[i.type] -= i.num_descriptors;
}

void
B3D_APIENTRY DescriptorPoolVk::IncrementRemains(const util::DyArray<DESCRIPTOR_POOL_SIZE>& _pool_sizes)
{
    for (auto& i : _pool_sizes)
        pool_remains[i.type] += i.num_descriptors;
}

BMRESULT
B3D_APIENTRY DescriptorPoolVk::AllocateVkDescriptorSets(const buma3d::DESCRIPTOR_SET_ALLOCATE_DESC& _desc, VkDescriptorSet* _setsvk_data)
{
    util::DyArray<VkDescriptorSetLayout> set_layouts(_desc.num_descriptor_sets);
    auto set_layouts_data = set_layouts.data();
    for (uint32_t i = 0; i < _desc.num_descriptor_sets; i++)
        set_layouts_data[i] = _desc.set_layouts[i]->As<DescriptorSetLayoutVk>()->GetVkDescriptorSetLayout();

    // vkAllocateDescriptorSetsは要素内のいずれかの割当に失敗した場合、他のすべての要素もVK_NULL_HANDLEを返します。
    VkDescriptorSetAllocateInfo ai{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
    ai.descriptorPool     = descriptor_pool;
    ai.descriptorSetCount = _desc.num_descriptor_sets;
    ai.pSetLayouts        = set_layouts_data;
    auto vkr = vkAllocateDescriptorSets(vkdevice, &ai, _setsvk_data);
    B3D_RET_IF_FAILED(VKR_TRACE_IF_FAILED(vkr));

    return BMRESULT_SUCCEED;
}


}// namespace buma3d
