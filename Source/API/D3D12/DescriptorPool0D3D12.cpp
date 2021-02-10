#include "Buma3DPCH.h"
#include "DescriptorPool0D3D12.h"
#include "DescriptorSet0D3D12.h"

namespace buma3d
{

namespace /*anonymous*/
{

static constexpr const char* HEAP_TYPE_NAMES[] =
{
      " (D3D12_HEAP_TYPE_CBV_SRV_UAV)"
    , " (D3D12_HEAP_TYPE_SAMPLER)"
    , " (D3D12_HEAP_TYPE_CBV_SRV_UAV (for COPY_SRC))"
    , " (D3D12_HEAP_TYPE_SAMPLER (for COPY_SRC))"
};

}// namespace /*anonymous*/

B3D_APIENTRY DescriptorPool0D3D12::DescriptorPool0D3D12()
    : ref_count         { 1 }
    , name              {}
    , device            {}
    , desc              {}
    , desc_data         {}
    , pool_remains      {}
    , allocation_mutex  {}
    , allocation_count  {}
    , reset_id          {}
    , device12          {}
    , dh_allocators     {}
    , desc_heaps12      {}
    , copy_src_heaps    {}
{     
      
}

B3D_APIENTRY DescriptorPool0D3D12::~DescriptorPool0D3D12()
{
    Uninit();
}

BMRESULT
B3D_APIENTRY DescriptorPool0D3D12::Init(DeviceD3D12* _device, const DESCRIPTOR_POOL_DESC0& _desc)
{
    (device = _device)->AddRef();
    device12 = device->GetD3D12Device();

    CopyDesc(_desc);

    uint32_t num_descs         = GetCbvSrvUavCountsInPoolSizes();
    uint32_t num_sampler_descs = GetSamplerCountsInPoolSizes();
    B3D_RET_IF_FAILED(CreateDescriptorHeaps(num_descs, num_sampler_descs));

    for (auto& i : desc_data.pool_sizes)
        pool_remains[i.type] = i.num_descriptors;
    
    return BMRESULT_SUCCEED;
}

void 
B3D_APIENTRY DescriptorPool0D3D12::CopyDesc(const DESCRIPTOR_POOL_DESC0& _desc)
{
    desc = _desc;

    desc_data.pool_sizes.resize(_desc.num_pool_sizes);
    util::MemCopyArray(desc_data.pool_sizes.data(), _desc.pool_sizes, _desc.num_pool_sizes);
    desc.pool_sizes = desc_data.pool_sizes.data();
}

uint32_t
B3D_APIENTRY DescriptorPool0D3D12::GetCbvSrvUavCountsInPoolSizes()
{
    uint32_t result = 0;
    for (uint32_t i = 0; i < desc.num_pool_sizes; i++)
    {
        auto&& ps = desc.pool_sizes[i];
        switch (ps.type)
        {
        case buma3d::DESCRIPTOR_TYPE_CBV:
        case buma3d::DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
        case buma3d::DESCRIPTOR_TYPE_SRV_TEXTURE:
        case buma3d::DESCRIPTOR_TYPE_SRV_TYPED_BUFFER:
        case buma3d::DESCRIPTOR_TYPE_SRV_BUFFER:
        case buma3d::DESCRIPTOR_TYPE_SRV_ACCELERATION_STRUCTURE:
        case buma3d::DESCRIPTOR_TYPE_UAV_TEXTURE:
        case buma3d::DESCRIPTOR_TYPE_UAV_TYPED_BUFFER:
        case buma3d::DESCRIPTOR_TYPE_UAV_BUFFER:
            result += ps.num_descriptors;
            break;

        default:
            break;
        }
    }

    return result;
}

uint32_t
B3D_APIENTRY DescriptorPool0D3D12::GetSamplerCountsInPoolSizes()
{
    uint32_t result = 0;
    for (uint32_t i = 0; i < desc.num_pool_sizes; i++)
    {
        auto&& ps = desc.pool_sizes[i];
        switch (ps.type)
        {
        case buma3d::DESCRIPTOR_TYPE_SAMPLER:
            result += ps.num_descriptors;
            break;

        default:
            break;
        }
    }

    return result;
}

BMRESULT
B3D_APIENTRY DescriptorPool0D3D12::CreateDescriptorHeaps(uint32_t _num_descs, uint32_t _num_sampler_descs)
{
    auto Create = [&](auto _heap_type, auto _num)
    {
        auto hr = (dh_allocators[_heap_type] = B3DNewArgs(GPUDescriptorAllocator, device12, _heap_type, _num, desc.node_mask))->Init();
        B3D_RET_IF_FAILED(HR_TRACE_IF_FAILED(hr));
        desc_heaps12.emplace_back(dh_allocators[_heap_type]->GetD3D12DescriptorHeap())->AddRef();

        if (desc.flags & DESCRIPTOR_POOL_FLAG_COPY_SRC)
        {
            auto&& copy_src_heap = copy_src_heaps->emplace_back();

            D3D12_DESCRIPTOR_HEAP_DESC dhd{ _heap_type, _num, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, B3D_DEFAULT_NODE_MASK };
            auto hr = device12->CreateDescriptorHeap(&dhd, IID_PPV_ARGS(&copy_src_heap.copy_desc_heap12));
            B3D_RET_IF_FAILED(HR_TRACE_IF_FAILED(hr));

            copy_src_heap.increment_size    = device12->GetDescriptorHandleIncrementSize(_heap_type);
            copy_src_heap.cpu_base_handle   = copy_src_heap.copy_desc_heap12->GetCPUDescriptorHandleForHeapStart();
        }

        return BMRESULT_SUCCEED;
    };

    if (desc.flags & DESCRIPTOR_POOL_FLAG_COPY_SRC)
        copy_src_heaps = B3DMakeUnique(util::DyArray<COPY_SRC_HEAP>);

    if (_num_descs != 0)
        B3D_RET_IF_FAILED(Create(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, _num_descs));

    if (_num_sampler_descs != 0)
        B3D_RET_IF_FAILED(Create(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, _num_sampler_descs));

    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY DescriptorPool0D3D12::Uninit()
{
    if (desc.flags & DESCRIPTOR_POOL_FLAG_COPY_SRC)
    {
        for (auto& i : *copy_src_heaps)
            hlp::SafeRelease(i.copy_desc_heap12);
        copy_src_heaps.reset();
    }
    
    for (auto& i : desc_heaps12)
        hlp::SafeRelease(i);

    for (auto& i : dh_allocators)
    { B3DSafeDelete(i); }

    hlp::SafeRelease(device);
    device12 = nullptr;

    desc = {};
    name.reset();
}

BMRESULT 
B3D_APIENTRY DescriptorPool0D3D12::Create(DeviceD3D12* _device, const DESCRIPTOR_POOL_DESC0& _desc, DescriptorPool0D3D12** _dst)
{
    util::Ptr<DescriptorPool0D3D12> ptr;
    ptr.Attach(B3DCreateImplementationClass(DescriptorPool0D3D12));
    B3D_RET_IF_FAILED(ptr->Init(_device, _desc));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY DescriptorPool0D3D12::AddRef()
{
    ++ref_count;
    B3D_REFCOUNT_DEBUG(ref_count);
}

uint32_t
B3D_APIENTRY DescriptorPool0D3D12::Release()
{
    B3D_REFCOUNT_DEBUG(ref_count - 1);
    auto count = --ref_count;
    if (count == 0)
        B3DDestroyImplementationClass(this);

    return count;
}

uint32_t
B3D_APIENTRY DescriptorPool0D3D12::GetRefCount() const
{
    return ref_count;
}

const char*
B3D_APIENTRY DescriptorPool0D3D12::GetName() const
{
    return name ? name->c_str() : nullptr;
}

BMRESULT
B3D_APIENTRY DescriptorPool0D3D12::SetName(const char* _name)
{
    if (!util::IsEnabledDebug(this))
        return BMRESULT_FAILED;

    uint32_t count = 0;
    for (auto& i : desc_heaps12)
    {
        if (i)
        {
            auto hr = util::SetName(i, _name ? hlp::StringConvolution(_name, HEAP_TYPE_NAMES[count]).c_str() : nullptr);
            B3D_RET_IF_FAILED(HR_TRACE_IF_FAILED(hr));
        }
        count++;
    }

    if (desc.flags & DESCRIPTOR_POOL_FLAG_COPY_SRC)
    {
        for (auto& i : *copy_src_heaps)
        {
            auto hr = util::SetName(i.copy_desc_heap12, _name ? hlp::StringConvolution(_name, HEAP_TYPE_NAMES[i.copy_desc_heap12->GetDesc().Type + 2]).c_str() : nullptr);
            B3D_RET_IF_FAILED(HR_TRACE_IF_FAILED(hr));
        }
    }

    if (name && !_name)
        name.reset();
    else
        name = B3DMakeUniqueArgs(util::NameableObjStr, _name);

    return BMRESULT_SUCCEED;
}

IDevice*
B3D_APIENTRY DescriptorPool0D3D12::GetDevice() const
{
    return device;
}

const DESCRIPTOR_POOL_DESC0&
B3D_APIENTRY DescriptorPool0D3D12::GetDesc() const
{
    return desc;
}

uint32_t
B3D_APIENTRY DescriptorPool0D3D12::GetCurrentAllocationCount()
{
    return allocation_count;
}

void
B3D_APIENTRY DescriptorPool0D3D12::ResetPoolAndInvalidateAllocatedSets()
{
    std::lock_guard lock(allocation_mutex);

    for (auto& i : dh_allocators)
    {
        if (i)
            i->ResetRanges();
    }
    allocation_count = 0;
    ++reset_id;
}

BMRESULT
B3D_APIENTRY DescriptorPool0D3D12::AllocateDescriptorSet(IRootSignature* _root_signature, IDescriptorSet0** _dst)
{
    std::lock_guard lock(allocation_mutex);

    auto rs = _root_signature->As<RootSignatureD3D12>();

    if (rs->GetNumRegisterSpace() > desc.max_num_register_space)
    {
        B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_INITIALIZATION
                          , "要求されたルートシグネチャ内のすべての存在するregister_spaceの数がmax_num_register_spaceの数を超えています。");
        return BMRESULT_FAILED_OUT_OF_POOL_MEMORY;
    }

    if (allocation_count >= desc.max_sets_allocation_count)
        return BMRESULT_FAILED_OUT_OF_POOL_MEMORY;

    auto&& ps = rs->GetPoolSizes();
    for (auto& [type, size] : ps)
    {
        if (size > pool_remains[type])
            return BMRESULT_FAILED_OUT_OF_POOL_MEMORY;
    }

    util::Ptr<DescriptorSet0D3D12> ptr;
    B3D_RET_IF_FAILED(DescriptorSet0D3D12::Create(this, rs, &ptr));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

util::StArray<GPUDescriptorAllocator*, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER + 1>&
B3D_APIENTRY DescriptorPool0D3D12::GetDescHeapAllocators()
{
    return dh_allocators;
}

const util::DyArray<ID3D12DescriptorHeap*>&
B3D_APIENTRY DescriptorPool0D3D12::GetD3D12DescriptorHeaps() const
{
    return desc_heaps12;
}

uint64_t
B3D_APIENTRY DescriptorPool0D3D12::GetResetID()
{
    return reset_id;
}

BMRESULT
B3D_APIENTRY DescriptorPool0D3D12::AllocateDescriptors(DescriptorSet0D3D12* _set)
{
    // OPTIMIZE: DescriptorSet0D3D12::AllocateDescriptors()、ないしディスクリプタ系全般をリファクタ
    auto&& ps = _set->signature->GetPoolSizes();
    auto&& ps12 = _set->signature->GetPoolSizes12();

    /*NOTE: 単純なメモリ不足(pool->pool_remainsの合計ディスクリプタサイズの不足)による割り当て失敗の確認はDescriptorPool0D3D12::AllocateDescriptorSetによって事前に行われるので、
            dh_allocators[_type]->Allocate(_size)が失敗する場合、断片化を意味します。*/

    auto Allocate = [this, _set](auto _type, auto _size)
    {
        auto&& allocator = dh_allocators[_type];
        auto&& allocation = _set->allocations[_type];
        allocation = allocator->Allocate(_size);
        if (!allocation.handles)
            return BMRESULT_FAILED_FRAGMENTED_POOL;

        if (desc.flags & DESCRIPTOR_POOL_FLAG_COPY_SRC)
        {
            auto&& copy_src_heap = copy_src_heaps->data()[_type];
            _set->copy_src_descriptors->data()[_type] = { &copy_src_heap, copy_src_heap.cpu_base_handle.ptr + (copy_src_heap.increment_size * allocator->CalcBeginOffset(allocation)), _size };
        }

        return BMRESULT_SUCCEED;
    };

    //std::lock_guard lock(allocation_mutex); <-AllocateDescriptorSet

    if (desc.flags & DESCRIPTOR_POOL_FLAG_COPY_SRC)
        _set->copy_src_descriptors = B3DMakeUnique(B3D_T(util::StArray<DescriptorPool0D3D12::COPY_SRC_HANDLES, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER + 1>));

    if (ps12.num_descs)
    {
        B3D_RET_IF_FAILED(Allocate(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, ps12.num_descs));
        for (auto& [type, size] : ps)
        {
            if (type != DESCRIPTOR_TYPE_SAMPLER)
                pool_remains[type] -= size;
        }
    }
    if (ps12.num_sampler_descs)
    {
        B3D_RET_IF_FAILED(Allocate(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, ps12.num_sampler_descs));
        pool_remains[DESCRIPTOR_TYPE_SAMPLER] -= ps12.num_sampler_descs;
    }

    ++allocation_count;
    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY DescriptorPool0D3D12::FreeDescriptors(DescriptorSet0D3D12* _set)
{
    auto&& ps          = _set->signature->GetPoolSizes();
    auto&& allocations = _set->allocations;
    auto FreeIfExist = [this, &allocations](auto _type)
    {
        auto&& al = allocations[_type];
        if (!al.handles)
            return false;

        dh_allocators[_type]->Free(al); al = {};
        return true;
    };

    std::lock_guard lock(allocation_mutex);

    if (FreeIfExist(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV))
    {
        for (auto& [type, size] : ps)
        {
            if (type != DESCRIPTOR_TYPE_SAMPLER)
                pool_remains[type] += size;
        }
    }
    if (FreeIfExist(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER))
    {
        pool_remains[DESCRIPTOR_TYPE_SAMPLER] += ps.at(DESCRIPTOR_TYPE_SAMPLER);
    }

    --allocation_count;
}


}// namespace buma3d
