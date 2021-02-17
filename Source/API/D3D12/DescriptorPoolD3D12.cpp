#include "Buma3DPCH.h"
#include "DescriptorPoolD3D12.h"
#include "DescriptorSetD3D12.h"

namespace buma3d
{

namespace /*anonymous*/
{

static constexpr const char* HEAP_TYPE_NAMES[] =
{
      " (D3D12_HEAP_TYPE_CBV_SRV_UAV (for COPY_SRC))"
    , " (D3D12_HEAP_TYPE_SAMPLER (for COPY_SRC))"
};

}// namespace /*anonymous*/

B3D_APIENTRY DescriptorPoolD3D12::DescriptorPoolD3D12()
    : ref_count         { 1 }
    , name              {}
    , device            {}
    , desc              {}
    , desc_data         {}
    , parent_heap       {}
    , pool_remains      {}
    , allocation_count  {}
    , reset_id          {}
    , device12          {}
    , heap_allocations  {}
    , dh_allocators     {}
    , copy_src_heaps    {}
{

}

B3D_APIENTRY DescriptorPoolD3D12::~DescriptorPoolD3D12()
{
    Uninit();
}

BMRESULT
B3D_APIENTRY DescriptorPoolD3D12::Init(DeviceD3D12* _device, const DESCRIPTOR_POOL_DESC& _desc)
{
    (device = _device)->AddRef();
    device12 = device->GetD3D12Device();

    if (util::HasSameDescriptorType(_desc.num_pool_sizes, _desc.pool_sizes))
    {
        B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_INITIALIZATION
                          , "DESCRIPTOR_HEAP_DESC::heap_sizesの各要素のtypeは一意である必要があります。");
        return BMRESULT_FAILED_INVALID_PARAMETER;
    }
    CopyDesc(_desc);

    uint32_t num_descs         = 0;
    uint32_t num_sampler_descs = 0;
    util::CalcDescriptorCounts(desc.num_pool_sizes, desc.pool_sizes, &num_descs, &num_sampler_descs);
    B3D_RET_IF_FAILED(AllocateFromHeap(num_descs, num_sampler_descs));

    if (desc.flags & DESCRIPTOR_POOL_FLAG_COPY_SRC)
        B3D_RET_IF_FAILED(CreateCopySrcHeaps(num_descs, num_sampler_descs));

    for (auto& i : desc_data->pool_sizes)
        pool_remains[i.type] += i.num_descriptors;
    
    return BMRESULT_SUCCEED;
}

void 
B3D_APIENTRY DescriptorPoolD3D12::CopyDesc(const DESCRIPTOR_POOL_DESC& _desc)
{
    desc = _desc;

    (parent_heap = _desc.heap->As<DescriptorHeapD3D12>())->AddRef();

    desc_data = B3DMakeUnique(DESC_DATA);
    desc_data->pool_sizes.resize(_desc.num_pool_sizes);
    desc.pool_sizes = util::MemCopyArray(desc_data->pool_sizes.data(), _desc.pool_sizes, _desc.num_pool_sizes);
}

BMRESULT
B3D_APIENTRY DescriptorPoolD3D12::AllocateFromHeap(uint32_t _num_descs, uint32_t _num_sampler_descs)
{
    GPU_DESCRIPTOR_ALLOCATION tmp[2]{};
    B3D_RET_IF_FAILED(parent_heap->AllocateDescriptors(desc_data->pool_sizes
                                                       , _num_descs                                  , _num_sampler_descs
                                                       , &tmp[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV], &tmp[D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER]));

    auto Create = [&](auto _heap_type, auto _num)
    {
        heap_allocations[_heap_type] = B3DNewArgs(GPU_DESCRIPTOR_ALLOCATION, tmp[_heap_type]);
        dh_allocators   [_heap_type] = B3DNewArgs(GPUDescriptorAllocator
                                                  , tmp[_heap_type].increment_size, tmp[_heap_type].num_descriptors
                                                  , tmp[_heap_type].handles.cpu_begin, tmp[_heap_type].handles.gpu_begin
                                                  , desc.flags & DESCRIPTOR_POOL_FLAG_FREE_DESCRIPTOR_SET);
    };
    if (_num_descs != 0)
        Create(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, _num_descs);
    if (_num_sampler_descs != 0)
        Create(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, _num_sampler_descs);

    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY DescriptorPoolD3D12::CreateCopySrcHeaps(uint32_t _num_descs, uint32_t _num_sampler_descs)
{
    copy_src_heaps = B3DMakeUnique(B3D_T(util::StArray<COPY_SRC_HEAP, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER+1>));

    auto Create = [&](auto _heap_type, auto _num)
    {
        auto&& copy_src_heap = copy_src_heaps->at(_heap_type);

        D3D12_DESCRIPTOR_HEAP_DESC dhd{ _heap_type, _num, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, desc.heap->GetDesc().node_mask };
        auto hr = device12->CreateDescriptorHeap(&dhd, IID_PPV_ARGS(&copy_src_heap.copy_desc_heap12));
        B3D_RET_IF_FAILED(HR_TRACE_IF_FAILED(hr));

        copy_src_heap.increment_size    = device12->GetDescriptorHandleIncrementSize(_heap_type);
        copy_src_heap.cpu_base_handle   = copy_src_heap.copy_desc_heap12->GetCPUDescriptorHandleForHeapStart();

        return BMRESULT_SUCCEED;
    };

    if (_num_descs != 0)
        B3D_RET_IF_FAILED(Create(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, _num_descs));

    if (_num_sampler_descs != 0)
        B3D_RET_IF_FAILED(Create(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, _num_sampler_descs));

    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY DescriptorPoolD3D12::Uninit()
{
    if (desc.flags & DESCRIPTOR_POOL_FLAG_COPY_SRC)
    {
        for (auto& i : *copy_src_heaps)
            hlp::SafeRelease(i.copy_desc_heap12);
        copy_src_heaps.reset();
    }

    if (parent_heap)
        parent_heap->FreeDescriptors(desc_data->pool_sizes, heap_allocations[0], heap_allocations[1]);

    for (auto& i : heap_allocations)
    { B3DSafeDelete(i); }

    for (auto& i : dh_allocators)
    { B3DSafeDelete(i); }

    hlp::SafeRelease(parent_heap);

    hlp::SafeRelease(device);
    device12 = nullptr;

    desc = {};
    desc_data.reset();
    name.reset();
}

BMRESULT 
B3D_APIENTRY DescriptorPoolD3D12::Create(DeviceD3D12* _device, const DESCRIPTOR_POOL_DESC& _desc, DescriptorPoolD3D12** _dst)
{
    util::Ptr<DescriptorPoolD3D12> ptr;
    ptr.Attach(B3DCreateImplementationClass(DescriptorPoolD3D12));
    B3D_RET_IF_FAILED(ptr->Init(_device, _desc));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY DescriptorPoolD3D12::AddRef()
{
    ++ref_count;
    B3D_REFCOUNT_DEBUG(ref_count);
}

uint32_t
B3D_APIENTRY DescriptorPoolD3D12::Release()
{
    B3D_REFCOUNT_DEBUG(ref_count - 1);
    auto count = --ref_count;
    if (count == 0)
        B3DDestroyImplementationClass(this);

    return count;
}

uint32_t
B3D_APIENTRY DescriptorPoolD3D12::GetRefCount() const
{
    return ref_count;
}

const char*
B3D_APIENTRY DescriptorPoolD3D12::GetName() const
{
    return name ? name->c_str() : nullptr;
}

BMRESULT
B3D_APIENTRY DescriptorPoolD3D12::SetName(const char* _name)
{
    if (!util::IsEnabledDebug(this))
        return BMRESULT_FAILED;

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
B3D_APIENTRY DescriptorPoolD3D12::GetDevice() const
{
    return device;
}

const DESCRIPTOR_POOL_DESC&
B3D_APIENTRY DescriptorPoolD3D12::GetDesc() const
{
    return desc;
}

uint32_t
B3D_APIENTRY DescriptorPoolD3D12::GetCurrentAllocationCount()
{
    return allocation_count;
}

void
B3D_APIENTRY DescriptorPoolD3D12::ResetPoolAndInvalidateAllocatedSets()
{
    for (auto& i : dh_allocators)
    {
        if (i)
            i->ResetRanges();
    }
    allocation_count = 0;
    ++reset_id;
}

BMRESULT
B3D_APIENTRY DescriptorPoolD3D12::AllocateDescriptorSets(const DESCRIPTOR_SET_ALLOCATE_DESC& _desc, IDescriptorSet** _dst_descriptor_sets)
{
    util::DyArray<util::Ptr<DescriptorSetD3D12>> sets(_desc.num_descriptor_sets);
    auto sets_data = sets.data();
    for (uint32_t i = 0; i < _desc.num_descriptor_sets; i++)
    {
        auto l = _desc.set_layouts[i]->As<DescriptorSetLayoutD3D12>();
        B3D_RET_IF_FAILED(DescriptorSetD3D12::Create(l, this, &sets_data[i]));
    }

    for (uint32_t i = 0; i < _desc.num_descriptor_sets; i++)
    {
        _dst_descriptor_sets[i] = sets_data[i].Detach();
    }
    return BMRESULT_SUCCEED;
}

uint64_t
B3D_APIENTRY DescriptorPoolD3D12::GetResetID()
{
    return reset_id;
}

BMRESULT
B3D_APIENTRY DescriptorPoolD3D12::AllocateDescriptors(const util::DyArray<DESCRIPTOR_POOL_SIZE>& _pool_sizes, uint32_t _num_descriptors, uint32_t _num_sampler_descriptors, POOL_ALLOCATION* _dst_descriptors, POOL_ALLOCATION* _dst_sampler_descriptors)
{
    if (!IsAllocatable(_pool_sizes))
        return BMRESULT_FAILED_OUT_OF_POOL_MEMORY;

    auto Allocate = [&](D3D12_DESCRIPTOR_HEAP_TYPE _type, uint32_t _num_descriptors, POOL_ALLOCATION* _dst)
    {
        size_t offset = 0;
        _dst_descriptors->allocation = dh_allocators[_type]->Allocate(_num_descriptors, &offset);
        if (!_dst_descriptors->allocation.handles)
            return BMRESULT_FAILED_FRAGMENTED_POOL;

        if (desc.flags & DESCRIPTOR_POOL_FLAG_COPY_SRC)
        {
            auto&& copy_src_heap = copy_src_heaps->at(_type);
            _dst_descriptors->copy_allocation = { &copy_src_heap, copy_src_heap.cpu_base_handle.ptr + (copy_src_heap.increment_size * offset), _num_descriptors };
        }

        return BMRESULT_SUCCEED;
    };
    if (_num_descriptors)
        B3D_RET_IF_FAILED(Allocate(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, _num_descriptors, _dst_descriptors));
    if (_num_sampler_descriptors)
        B3D_RET_IF_FAILED(Allocate(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, _num_sampler_descriptors, _dst_sampler_descriptors));

    ++allocation_count;
    DecrementRemains(_pool_sizes);
    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY DescriptorPoolD3D12::FreeDescriptors(const util::DyArray<DESCRIPTOR_POOL_SIZE>& _pool_sizes, POOL_ALLOCATION* _dst_descriptors, POOL_ALLOCATION* _dst_sampler_descriptors)
{
    --allocation_count;
    IncrementRemains(_pool_sizes);
    if (_dst_descriptors)
        dh_allocators[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV]->Free(_dst_descriptors->allocation);
    if (_dst_sampler_descriptors)
        dh_allocators[D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER]->Free(_dst_sampler_descriptors->allocation);
}

bool
B3D_APIENTRY DescriptorPoolD3D12::IsAllocatable(const util::DyArray<DESCRIPTOR_POOL_SIZE>& _pool_sizes) const
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
B3D_APIENTRY DescriptorPoolD3D12::DecrementRemains(const util::DyArray<DESCRIPTOR_POOL_SIZE>& _pool_sizes)
{
    for (auto& i : _pool_sizes)
        pool_remains[i.type] -= i.num_descriptors;
}

void
B3D_APIENTRY DescriptorPoolD3D12::IncrementRemains(const util::DyArray<DESCRIPTOR_POOL_SIZE>& _pool_sizes)
{
    for (auto& i : _pool_sizes)
        pool_remains[i.type] += i.num_descriptors;
}


}// namespace buma3d
