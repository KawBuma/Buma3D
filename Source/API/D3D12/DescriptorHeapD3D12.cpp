#include "Buma3DPCH.h"
#include "DescriptorHeapD3D12.h"
#include "DescriptorSetD3D12.h"

namespace buma3d
{

namespace /*anonymous*/
{

static constexpr const char* HEAP_TYPE_NAMES[] =
{
      " (D3D12_HEAP_TYPE_CBV_SRV_UAV)"
    , " (D3D12_HEAP_TYPE_SAMPLER)"
};

}// namespace /*anonymous*/

B3D_APIENTRY DescriptorHeapD3D12::DescriptorHeapD3D12()
    : ref_count         { 1 }
    , name              {}
    , device            {}
    , desc              {}
    , desc_data         {}
    , heap_remains      {}
    , device12          {}
    , desc_heaps12      {}
    , dh_allocators     {}
    , num_heaps         {}
    , heap_start_offset {}
{
      
}

B3D_APIENTRY DescriptorHeapD3D12::~DescriptorHeapD3D12()
{
    Uninit();
}

BMRESULT
B3D_APIENTRY DescriptorHeapD3D12::Init(DeviceD3D12* _device, const DESCRIPTOR_HEAP_DESC& _desc)
{
    (device = _device)->AddRef();
    device12 = device->GetD3D12Device();

    if (util::HasSameDescriptorType(_desc.num_heap_sizes, _desc.heap_sizes))
    {
        B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_INITIALIZATION
                          , "DESCRIPTOR_HEAP_DESC::heap_sizesの各要素のtypeは一意である必要があります。");
        return BMRESULT_FAILED_INVALID_PARAMETER;
    }
    CopyDesc(_desc);

    uint32_t num_descs         = 0;
    uint32_t num_sampler_descs = 0;
    util::CalcDescriptorCounts(desc.num_heap_sizes, desc.heap_sizes, &num_descs, &num_sampler_descs);
    B3D_RET_IF_FAILED(CreateDescriptorHeaps(num_descs, num_sampler_descs));

    for (auto& i : desc_data->heap_sizes)
        heap_remains[i.type] += i.num_descriptors;

    if (num_descs != 0)
    {
        num_heaps++;
        heap_start_offset = 0;
    }
    if (num_sampler_descs != 0)
    {
        num_heaps++;
        heap_start_offset = num_heaps == 1 ? 1 : 0;
    }

    return BMRESULT_SUCCEED;
}

void 
B3D_APIENTRY DescriptorHeapD3D12::CopyDesc(const DESCRIPTOR_HEAP_DESC& _desc)
{
    desc = _desc;

    desc_data = B3DMakeUnique(DESC_DATA);
    desc_data->heap_sizes.resize(_desc.num_heap_sizes);
    desc.heap_sizes = util::MemCopyArray(desc_data->heap_sizes.data(), _desc.heap_sizes, _desc.num_heap_sizes);
}

BMRESULT
B3D_APIENTRY DescriptorHeapD3D12::CreateDescriptorHeaps(uint32_t _num_descs, uint32_t _num_sampler_descs)
{
    auto Create = [&](auto _heap_type, auto _num)
    {
        D3D12_DESCRIPTOR_HEAP_DESC heap_desc{};
        heap_desc.Type              = _heap_type;
        heap_desc.NumDescriptors    = _num;
        heap_desc.Flags             = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        heap_desc.NodeMask          = desc.node_mask;

        auto&& heap = desc_heaps12[_heap_type];
        auto hr = device12->CreateDescriptorHeap(&heap_desc, IID_PPV_ARGS(&heap));
        B3D_RET_IF_FAILED(HR_TRACE_IF_FAILED(hr));

        dh_allocators[_heap_type] = B3DNewArgs(GPUDescriptorAllocator
                                               , device12->GetDescriptorHandleIncrementSize(_heap_type), _num
                                               , heap->GetCPUDescriptorHandleForHeapStart()
                                               , heap->GetGPUDescriptorHandleForHeapStart()
                                               , /*enabled free descriptor*/ true);
        return BMRESULT_SUCCEED;
    };

    if (_num_descs != 0)
        B3D_RET_IF_FAILED(Create(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, _num_descs));

    if (_num_sampler_descs != 0)
        B3D_RET_IF_FAILED(Create(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, _num_sampler_descs));

    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY DescriptorHeapD3D12::Uninit()
{
    for (auto& i : desc_heaps12)
        hlp::SafeRelease(i);

    for (auto& i : dh_allocators)
    { B3DSafeDelete(i); }

    heap_remains.fill(0);

    desc = {};
    desc_data.reset();

    hlp::SafeRelease(device);
    device12 = nullptr;

    name.reset();
}

BMRESULT 
B3D_APIENTRY DescriptorHeapD3D12::Create(DeviceD3D12* _device, const DESCRIPTOR_HEAP_DESC& _desc, DescriptorHeapD3D12** _dst)
{
    util::Ptr<DescriptorHeapD3D12> ptr;
    ptr.Attach(B3DCreateImplementationClass(DescriptorHeapD3D12));
    B3D_RET_IF_FAILED(ptr->Init(_device, _desc));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY DescriptorHeapD3D12::AddRef()
{
    ++ref_count;
    B3D_REFCOUNT_DEBUG(ref_count);
}

uint32_t
B3D_APIENTRY DescriptorHeapD3D12::Release()
{
    B3D_REFCOUNT_DEBUG(ref_count - 1);
    auto count = --ref_count;
    if (count == 0)
        B3DDestroyImplementationClass(this);

    return count;
}

uint32_t
B3D_APIENTRY DescriptorHeapD3D12::GetRefCount() const
{
    return ref_count;
}

const char*
B3D_APIENTRY DescriptorHeapD3D12::GetName() const
{
    return name ? name->c_str() : nullptr;
}

BMRESULT
B3D_APIENTRY DescriptorHeapD3D12::SetName(const char* _name)
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

    if (name && !_name)
        name.reset();
    else
        name = B3DMakeUniqueArgs(util::NameableObjStr, _name);

    return BMRESULT_SUCCEED;
}

IDevice*
B3D_APIENTRY DescriptorHeapD3D12::GetDevice() const
{
    return device;
}

const DESCRIPTOR_HEAP_DESC&
B3D_APIENTRY DescriptorHeapD3D12::GetDesc() const
{
    return desc;
}

BMRESULT
B3D_APIENTRY DescriptorHeapD3D12::AllocateDescriptors(const util::DyArray<DESCRIPTOR_POOL_SIZE>& _pool_sizes, uint32_t _num_descriptors, uint32_t _num_sampler_descriptors, GPU_DESCRIPTOR_ALLOCATION* _dst_descriptors, GPU_DESCRIPTOR_ALLOCATION* _dst_sampler_descriptors)
{
    if (!IsAllocatable(_pool_sizes))
        return BMRESULT_FAILED_OUT_OF_POOL_MEMORY;

    if (_num_descriptors)
    {
        *_dst_descriptors = dh_allocators[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV]->Allocate(_num_descriptors);
        if (!_dst_descriptors->handles)
            return BMRESULT_FAILED_FRAGMENTED_POOL;
    }
    if (_num_sampler_descriptors)
    {
        *_dst_sampler_descriptors = dh_allocators[D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER]->Allocate(_num_sampler_descriptors);
        if (!_dst_sampler_descriptors->handles)
            return BMRESULT_FAILED_FRAGMENTED_POOL;
    }

    DecrementRemains(_pool_sizes);
    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY DescriptorHeapD3D12::FreeDescriptors(const util::DyArray<DESCRIPTOR_POOL_SIZE>& _pool_sizes, GPU_DESCRIPTOR_ALLOCATION* _dst_descriptors, GPU_DESCRIPTOR_ALLOCATION* _dst_sampler_descriptors)
{
    IncrementRemains(_pool_sizes);
    if (_dst_descriptors)
        dh_allocators[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV]->Free(*_dst_descriptors);
    if (_dst_sampler_descriptors)
        dh_allocators[D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER]->Free(*_dst_sampler_descriptors);
}

const util::StArray<ID3D12DescriptorHeap*, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER + 1>&
B3D_APIENTRY DescriptorHeapD3D12::GetD3D12DescriptorHeaps() const
{
    return desc_heaps12;
}

const util::StArray<ID3D12DescriptorHeap*, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER + 1>&
B3D_APIENTRY DescriptorHeapD3D12::GetD3D12DescriptorHeaps(uint32_t* _dst_num_heaps, uint32_t* _dst_heap_start_offset) const
{
    *_dst_num_heaps         = num_heaps;
    *_dst_heap_start_offset = heap_start_offset;
    return desc_heaps12;
}

bool
B3D_APIENTRY DescriptorHeapD3D12::IsAllocatable(const util::DyArray<DESCRIPTOR_POOL_SIZE>& _pool_sizes) const
{
    for (auto& i : _pool_sizes)
    {
        if (i.num_descriptors > heap_remains[i.type])
            return false;
    }
    return true;
}

void
B3D_APIENTRY DescriptorHeapD3D12::DecrementRemains(const util::DyArray<DESCRIPTOR_POOL_SIZE>& _pool_sizes)
{
    for (auto& i : _pool_sizes)
        heap_remains[i.type] -= i.num_descriptors;
}

void
B3D_APIENTRY DescriptorHeapD3D12::IncrementRemains(const util::DyArray<DESCRIPTOR_POOL_SIZE>& _pool_sizes)
{
    for (auto& i : _pool_sizes)
        heap_remains[i.type] += i.num_descriptors;
}


}// namespace buma3d
