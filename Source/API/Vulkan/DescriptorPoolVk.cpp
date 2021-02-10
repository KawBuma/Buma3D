#include "Buma3DPCH.h"
//#include "DescriptorPoolVk.h"
//#include "DescriptorSetVk.h"
//
//namespace buma3d
//{
//
//B3D_APIENTRY DescriptorPoolVk::DescriptorPoolVk()
//    : ref_count         { 1 }
//    , name              {}
//    , device            {}
//    , desc              {}
//    , desc_data         {}
//    , allocation_mutex  {}
//    , allocation_count  {}
//    , reset_id          {}
//    , vkdevice          {}
//    , inspfn            {}
//    , devpfn            {}
//    , descriptor_pool   {}
//{     
//      
//}
//
//B3D_APIENTRY DescriptorPoolVk::~DescriptorPoolVk()
//{
//    Uninit();
//}
//
//BMRESULT
//B3D_APIENTRY DescriptorPoolVk::Init(DeviceVk* _device, const DESCRIPTOR_POOL_DESC& _desc)
//{
//    (device = _device)->AddRef();
//    inspfn = &device->GetInstancePFN();
//    devpfn = &device->GetDevicePFN();
//    vkdevice = device->GetVkDevice();
//
//    CopyDesc(_desc);
//
//    B3D_RET_IF_FAILED(CreateVkDescriptorPool());
//
//    for (auto& i : desc_data.pool_sizes)
//        pool_remains[i.type] = i.num_descriptors;
//    
//    return BMRESULT_SUCCEED;
//}
//
//void 
//B3D_APIENTRY DescriptorPoolVk::CopyDesc(const DESCRIPTOR_POOL_DESC& _desc)
//{
//    desc = _desc;
//
//    desc_data.pool_sizes.resize(_desc.num_pool_sizes);
//    util::MemCopyArray(desc_data.pool_sizes.data(), _desc.pool_sizes, _desc.num_pool_sizes);
//    desc.pool_sizes = desc_data.pool_sizes.data();
//}
//
//void
//B3D_APIENTRY DescriptorPoolVk::GetNativePoolSizes(util::DyArray<VkDescriptorPoolSize>* _sises)
//{
//    _sises->resize(desc.num_pool_sizes);
//    auto _sizes_data = _sises->data();
//    for (uint32_t i = 0; i < desc.num_pool_sizes; i++)
//    {
//        auto&& ps = desc.pool_sizes[i];
//        _sizes_data[i].type            = util::GetNativeDescriptorType(ps.type);
//        _sizes_data[i].descriptorCount = ps.num_descriptors;
//    }
//}
//
//BMRESULT
//B3D_APIENTRY DescriptorPoolVk::CreateVkDescriptorPool()
//{
//    util::DyArray<VkDescriptorPoolSize> sizes;
//    GetNativePoolSizes(&sizes);
//
//    VkDescriptorPoolCreateInfo ci{ VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
//    ci.flags         = util::GetNativeDescriptorPoolFlags(desc.flags);
//    ci.maxSets       = desc.max_sets_allocation_count * desc.max_num_register_space;
//    ci.poolSizeCount = desc.num_pool_sizes;
//    ci.pPoolSizes    = sizes.data();
//    auto vkr = vkCreateDescriptorPool(vkdevice, &ci, B3D_VK_ALLOC_CALLBACKS, &descriptor_pool);
//    B3D_RET_IF_FAILED(VKR_TRACE_IF_FAILED(vkr));
//
//    return BMRESULT_SUCCEED;
//}
//
//void
//B3D_APIENTRY DescriptorPoolVk::Uninit()
//{
//    name.reset();
//    desc = {};
//
//    if (descriptor_pool)
//        vkDestroyDescriptorPool(vkdevice, descriptor_pool, B3D_VK_ALLOC_CALLBACKS);
//    descriptor_pool = VK_NULL_HANDLE;
//
//    hlp::SafeRelease(device);
//    vkdevice = VK_NULL_HANDLE;
//    inspfn = nullptr;
//    devpfn = nullptr;
//}
//
//BMRESULT 
//B3D_APIENTRY DescriptorPoolVk::Create(DeviceVk* _device, const DESCRIPTOR_POOL_DESC& _desc, DescriptorPoolVk** _dst)
//{
//    util::Ptr<DescriptorPoolVk> ptr;
//    ptr.Attach(B3DCreateImplementationClass(DescriptorPoolVk));
//    B3D_RET_IF_FAILED(ptr->Init(_device, _desc));
//
//    *_dst = ptr.Detach();
//    return BMRESULT_SUCCEED;
//}
//
//void
//B3D_APIENTRY DescriptorPoolVk::AddRef()
//{
//    ++ref_count;
//    B3D_REFCOUNT_DEBUG(ref_count);
//}
//
//uint32_t
//B3D_APIENTRY DescriptorPoolVk::Release()
//{
//    B3D_REFCOUNT_DEBUG(ref_count - 1);
//    auto count = --ref_count;
//    if (count == 0)
//        B3DDestroyImplementationClass(this);
//
//    return count;
//}
//
//uint32_t
//B3D_APIENTRY DescriptorPoolVk::GetRefCount() const
//{
//    return ref_count;
//}
//
//const char*
//B3D_APIENTRY DescriptorPoolVk::GetName() const
//{
//    return name ? name->c_str() : nullptr;
//}
//
//BMRESULT
//B3D_APIENTRY DescriptorPoolVk::SetName(const char* _name)
//{
//    if (!util::IsEnabledDebug(this))
//        return BMRESULT_FAILED;
//
//    if (descriptor_pool)
//        B3D_RET_IF_FAILED(device->SetVkObjectName(descriptor_pool, _name));
//
//    if (name && !_name)
//        name.reset();
//    else
//        name = B3DMakeUniqueArgs(util::NameableObjStr, _name);
//
//    return BMRESULT_SUCCEED;
//}
//
//IDevice*
//B3D_APIENTRY DescriptorPoolVk::GetDevice() const
//{
//    return device;
//}
//
//const VkAllocationCallbacks*
//B3D_APIENTRY DescriptorPoolVk::GetVkAllocationCallbacks() const
//{
//    return device->GetVkAllocationCallbacks();
//}
//
//const InstancePFN&
//B3D_APIENTRY DescriptorPoolVk::GetIsntancePFN() const
//{
//    return *inspfn;
//}
//
//const DevicePFN&
//B3D_APIENTRY DescriptorPoolVk::GetDevicePFN() const
//{
//    return *devpfn;
//}
//
//const DESCRIPTOR_POOL_DESC&
//B3D_APIENTRY DescriptorPoolVk::GetDesc() const
//{
//    return desc;
//}
//
//uint32_t
//B3D_APIENTRY DescriptorPoolVk::GetCurrentAllocationCount()
//{
//    return allocation_count;
//}
//
//void
//B3D_APIENTRY DescriptorPoolVk::ResetPoolAndInvalidateAllocatedSets()
//{
//    std::lock_guard lock(allocation_mutex);
//
//    auto vkr = vkResetDescriptorPool(vkdevice, descriptor_pool, 0/*reserved*/);
//    VKR_TRACE_IF_FAILED(vkr);
//
//    allocation_count = 0;
//    ++reset_id;
//}
//
//BMRESULT
//B3D_APIENTRY DescriptorPoolVk::AllocateDescriptorSet(IRootSignature* _root_signature, IDescriptorSet** _dst)
//{
//    std::lock_guard lock(allocation_mutex);
//
//    auto rs = _root_signature->As<RootSignatureVk>();
//
//    if (rs->GetValidSetLayouts().size() > desc.max_num_register_space)
//    {
//        B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_INITIALIZATION
//                          , "要求されたルートシグネチャ内のすべての存在するregister_spaceの数がmax_num_register_spaceの数を超えています。");
//        return BMRESULT_FAILED_OUT_OF_POOL_MEMORY;
//    }
//
//    if (allocation_count >= desc.max_sets_allocation_count)
//        return BMRESULT_FAILED_OUT_OF_POOL_MEMORY;
//
//    auto&& ps = rs->GetPoolSizes();
//    for (auto& [type, size] : ps)
//    {
//        if (size > pool_remains[type])
//            return BMRESULT_FAILED_OUT_OF_POOL_MEMORY;
//    }
//
//    util::Ptr<DescriptorSetVk> ptr;
//    B3D_RET_IF_FAILED(DescriptorSetVk::Create(this, rs, &ptr));
//
//    *_dst = ptr.Detach();
//    return BMRESULT_SUCCEED;
//}
//
//VkDescriptorPool
//B3D_APIENTRY DescriptorPoolVk::GetVkDescriptorPool() const
//{
//    return descriptor_pool;
//}
//
//uint64_t
//B3D_APIENTRY DescriptorPoolVk::GetResetID()
//{
//    return reset_id;
//}
//
//BMRESULT
//B3D_APIENTRY DescriptorPoolVk::AllocateDescriptors(DescriptorSetVk* _set)
//{
//    //std::lock_guard lock(allocation_mutex); <-AllocateDescriptorSet
//
//    auto&& ps = _set->signature->GetPoolSizes();
//    for (auto& [type, size] : ps)
//        pool_remains[type] -= size;
//
//    // ゼロバインディングも、vkAllocateDescriptorSetsの呼び出しが失敗せずセットを消費する(ディスクリプタ自体は消費しない)ので、そのようなセットは作成せず、VK_NULL_HANDLEとします。
//    _set->descriptor_sets.resize(_set->signature->GetVkDescriptorSetLayouts().size(), VK_NULL_HANDLE);
//
//    VkDescriptorSetAllocateInfo ai{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO, nullptr ,descriptor_pool };
//    for (auto& [offset, l] : _set->signature->GetValidSetLayouts())
//    {
//        ai.descriptorSetCount = l.num_layouts;
//        ai.pSetLayouts        = l.layouts;
//        auto vkr = vkAllocateDescriptorSets(vkdevice, &ai, _set->descriptor_sets.data() + offset);
//        B3D_RET_IF_FAILED(VKR_TRACE_IF_FAILED(vkr));
//    }
//
//    ++allocation_count;
//    return BMRESULT_SUCCEED;
//}
//
//void
//B3D_APIENTRY DescriptorPoolVk::FreeDescriptors(DescriptorSetVk* _set)
//{
//    auto&& ps   = _set->signature->GetPoolSizes();
//    auto&& sets = _set->descriptor_sets;
//
//    std::lock_guard lock(allocation_mutex);
//
//    for (auto& [offset, l] : _set->signature->GetValidSetLayouts())
//    {
//        auto vkr = vkFreeDescriptorSets(vkdevice, descriptor_pool, l.num_layouts, sets.data() + offset);
//        VKR_TRACE_IF_FAILED(vkr);
//        std::fill(sets.data() + offset, sets.data() + (offset + l.num_layouts), VkDescriptorSet(VK_NULL_HANDLE));
//    }
//
//    for (auto& [type, size] : ps)
//        pool_remains[type] -= size;
//
//    --allocation_count;
//}
//
//
//}// namespace buma3d
