#include "Buma3DPCH.h"
#include "DescriptorSet0D3D12.h"

namespace buma3d
{

B3D_APIENTRY DescriptorSet0D3D12::DescriptorSet0D3D12()
    : ref_count                 { 1 }
    , name                      {}
    , device                    {}
    , allocation_id             {}
    , reset_id                  {}
    , device12                  {}
    , pool                      {}
    , signature                 {}
    , allocations               {}
    , descriptor_batch          {}
    , copy_src_descriptors      {}
    , update_descriptors_caches {}
{     
      
}

B3D_APIENTRY DescriptorSet0D3D12::~DescriptorSet0D3D12()
{
    Uninit();
}

BMRESULT
B3D_APIENTRY DescriptorSet0D3D12::Init(DescriptorPool0D3D12* _pool, RootSignatureD3D12* _signature)
{
    (device = _pool->GetDevice()->As<DeviceD3D12>())->AddRef();
    device12 = device->GetD3D12Device();

    (pool = _pool)->AddRef();
    (signature = _signature)->AddRef();

    allocation_id = pool->GetCurrentAllocationCount() + 1;
    reset_id      = pool->GetResetID();

    B3D_RET_IF_FAILED(AllocateDescriptors());
    CreateSetDescriptorBatch();

    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY DescriptorSet0D3D12::AllocateDescriptors()
{
    return pool->AllocateDescriptors(this);
}

void
B3D_APIENTRY DescriptorSet0D3D12::CreateSetDescriptorBatch()
{
    auto&& root_sig_desc = signature->GetDesc();
    auto&& root_parameter_counts = signature->GetRootParameterCounts();
    descriptor_batch.descriptor_batch       .resize(root_sig_desc.num_parameters);
    descriptor_batch.constants_batch        .reserve(root_parameter_counts[ROOT_PARAMETER_TYPE_PUSH_32BIT_CONSTANTS]);
    descriptor_batch.root_descriptor_batch  .reserve(root_parameter_counts[ROOT_PARAMETER_TYPE_DYNAMIC_DESCRIPTOR]);
    descriptor_batch.descriptor_table_batch .reserve(root_parameter_counts[ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE]);

    auto&& total_num_descriptors_per_tables = signature->GetTotalNumDescriptorsCountPerTables();
    uint32_t descriptor_handle_offsets[D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER + 1]{};
    for (uint32_t i_root_param_index = 0; i_root_param_index < root_sig_desc.num_parameters; i_root_param_index++)
    {
        auto&& rp    = root_sig_desc.parameters[i_root_param_index];
        auto&& batch = descriptor_batch.descriptor_batch[i_root_param_index];
        switch (rp.type)
        {
        case buma3d::ROOT_PARAMETER_TYPE_PUSH_32BIT_CONSTANTS:
            batch.reset(
                descriptor_batch.constants_batch.emplace_back(
                    B3DNewArgs(SetConstantsBatch, i_root_param_index, D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS)));
            break;
            
        case buma3d::ROOT_PARAMETER_TYPE_DYNAMIC_DESCRIPTOR:
            batch.reset(
                descriptor_batch.root_descriptor_batch.emplace_back(
                    B3DNewArgs(SetRootDescriptorBatch, i_root_param_index, util::GetNativeRootParameterTypeForDynamicDescriptor(rp.dynamic_descriptor.type))));
            break;

        case buma3d::ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE:
        {
            auto&& total_num_descriptors = total_num_descriptors_per_tables.at(i_root_param_index);
            batch.reset(
                descriptor_batch.descriptor_table_batch.emplace_back(
                    B3DNewArgs(SetDescriptorTableBatch, i_root_param_index, D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE
                               , allocations[total_num_descriptors.type].OffsetGPUHandle(descriptor_handle_offsets[total_num_descriptors.type]))));
            descriptor_handle_offsets[total_num_descriptors.type] += total_num_descriptors.total_num_descriptors;
            break;
        }

        default:
            break;
        }
    }
}

void
B3D_APIENTRY DescriptorSet0D3D12::Uninit()
{
    name.reset();

    descriptor_batch = {};
    update_descriptors_caches = {};
    if (IsValid() && (pool->GetDesc().flags & DESCRIPTOR_POOL_FLAG_FREE_DESCRIPTOR_SET))
    {
        pool->FreeDescriptors(this);
    }
    else
    {
        allocations.fill({});
    }

    allocation_id = 0;
    reset_id      = 0;

    hlp::SafeRelease(pool);
    hlp::SafeRelease(signature);
    hlp::SafeRelease(device);
    device12 = nullptr;
}

BMRESULT 
B3D_APIENTRY DescriptorSet0D3D12::Create(DescriptorPool0D3D12* _pool, RootSignatureD3D12* _signature, DescriptorSet0D3D12** _dst)
{
    util::Ptr<DescriptorSet0D3D12> ptr;
    ptr.Attach(B3DCreateImplementationClass(DescriptorSet0D3D12));
    B3D_RET_IF_FAILED(ptr->Init(_pool, _signature));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY DescriptorSet0D3D12::AddRef()
{
    ++ref_count;
    B3D_REFCOUNT_DEBUG(ref_count);
}

uint32_t
B3D_APIENTRY DescriptorSet0D3D12::Release()
{
    B3D_REFCOUNT_DEBUG(ref_count - 1);
    auto count = --ref_count;
    if (count == 0)
        B3DDestroyImplementationClass(this);

    return count;
}

uint32_t
B3D_APIENTRY DescriptorSet0D3D12::GetRefCount() const
{
    return ref_count;
}

const char*
B3D_APIENTRY DescriptorSet0D3D12::GetName() const
{
    return name ? name->c_str() : nullptr;
}

BMRESULT
B3D_APIENTRY DescriptorSet0D3D12::SetName(const char* _name)
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
B3D_APIENTRY DescriptorSet0D3D12::GetDevice() const
{
    return device;
}

IRootSignature*
B3D_APIENTRY DescriptorSet0D3D12::GetRootSignature() const
{
    return signature;
}

IDescriptorPool0*
B3D_APIENTRY DescriptorSet0D3D12::GetPool() const
{
    return pool;
}

bool
B3D_APIENTRY DescriptorSet0D3D12::IsValid() const
{
    return reset_id == pool->GetResetID();
}

BMRESULT
B3D_APIENTRY DescriptorSet0D3D12::CopyDescriptorSet(IDescriptorSet0* _src)
{
    if (signature != _src->GetRootSignature())
        return BMRESULT_FAILED_INVALID_PARAMETER;

    if (!(_src->GetPool()->GetDesc().flags & DESCRIPTOR_POOL_FLAG_COPY_SRC))
        return BMRESULT_FAILED_INVALID_PARAMETER;

    auto CopySimple = [this](auto& _dst_allocation, const DescriptorPool0D3D12::COPY_SRC_HANDLES& _src_allocation, D3D12_DESCRIPTOR_HEAP_TYPE _type)
    { if (_src_allocation.num_descriptors) device12->CopyDescriptorsSimple(_src_allocation.num_descriptors, _dst_allocation.OffsetCPUHandle(0), _src_allocation.OffsetCPUHandle(0), _type); };

    if (pool->GetDesc().flags & DESCRIPTOR_POOL_FLAG_COPY_SRC)
    {
        auto&& copy_src_allocations = _src->As<DescriptorSet0D3D12>()->copy_src_descriptors->data();
        auto&& copy_dst_allocations = copy_src_descriptors->data();
        CopySimple(copy_dst_allocations[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV] , copy_src_allocations[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV]  , D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        CopySimple(copy_dst_allocations[D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER]     , copy_src_allocations[D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER]      , D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
        CopySimple(allocations[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV]          , copy_dst_allocations[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV]  , D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        CopySimple(allocations[D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER]              , copy_dst_allocations[D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER]      , D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
    }
    else
    {
        auto&& src_allocations = _src->As<DescriptorSet0D3D12>()->copy_src_descriptors->data();
        CopySimple(allocations[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV]          , src_allocations[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV]       , D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        CopySimple(allocations[D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER]              , src_allocations[D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER]           , D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
    }

    return BMRESULT_SUCCEED;
}

const util::StArray<GPU_DESCRIPTOR_ALLOCATION, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER + 1>&
B3D_APIENTRY DescriptorSet0D3D12::GetAllocations() const
{
    return allocations;
}

uint32_t
B3D_APIENTRY DescriptorSet0D3D12::GetAllocationID() const
{
    return allocation_id;
}

uint64_t
B3D_APIENTRY DescriptorSet0D3D12::GetResetID() const
{
    return reset_id;
}

const DescriptorSet0D3D12::DESCRIPTOR_BATCH&
B3D_APIENTRY DescriptorSet0D3D12::GetDescriptorBatch() const
{
    return descriptor_batch;
}

BMRESULT
B3D_APIENTRY DescriptorSet0D3D12::WriteDescriptors(const WRITE_DESCRIPTOR_SET& _writes)
{
    auto AddCopyRanges = [&](auto& _dst_allocations)
    {
        update_descriptors_caches[0].ResetRangeCount();
        update_descriptors_caches[1].ResetRangeCount();

        auto&& dst_rsdesc = signature->GetDesc();
        auto&& total_num_descriptors_per_tables = signature->GetTotalNumDescriptorsCountPerTables();
        for (uint32_t i_dt = 0; i_dt < _writes.num_descriptor_tables; i_dt++)
        {
            auto&& write_dt              = _writes.descriptor_tables[i_dt];
            auto&& total_num_descriptors = total_num_descriptors_per_tables.at(write_dt.dst_root_parameter_index);
            auto&& dst_allocation        = _dst_allocations[total_num_descriptors.type];
            auto   abs_offsets           = total_num_descriptors.absolute_handle_offsets.data();
            auto&& cache                 = update_descriptors_caches[total_num_descriptors.type];
            for (uint32_t i_write_range = 0; i_write_range < write_dt.num_ranges; i_write_range++)
            {
                auto&& write_range = write_dt.ranges[i_write_range];
                auto dst_handle = dst_allocation.OffsetCPUHandle(SCAST<size_t>(abs_offsets[write_range.dst_range_index] + write_range.dst_first_array_element));
                cache.AddWriteRange(write_range, dst_handle);
            }
        }

        update_descriptors_caches[0].ApplyCopy(device12, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        update_descriptors_caches[1].ApplyCopy(device12, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
    };

    if (pool->GetDesc().flags & DESCRIPTOR_POOL_FLAG_COPY_SRC)
    {
        AddCopyRanges(*copy_src_descriptors);

        auto CopySimple = [this](auto& _dst_allocation, const DescriptorPool0D3D12::COPY_SRC_HANDLES& _src_allocation, D3D12_DESCRIPTOR_HEAP_TYPE _type)
        { if (_src_allocation.num_descriptors) device12->CopyDescriptorsSimple(_src_allocation.num_descriptors, _dst_allocation.OffsetCPUHandle(0), _src_allocation.OffsetCPUHandle(0), _type); };
        CopySimple(allocations[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV], copy_src_descriptors->data()[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV], D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        CopySimple(allocations[D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER]    , copy_src_descriptors->data()[D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER]    , D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
    }
    else
    {
        AddCopyRanges(allocations);
    }

    for (uint32_t i = 0; i < _writes.num_dynamic_descriptors; i++)
    {
        auto&& dd = _writes.dynamic_descriptors[i];

        auto root_descriptor = *dd.src_view->DynamicCastFromThis<IViewD3D12>()
            ->GetGpuVirtualAddress() + dd.src_view_buffer_offset;

        SCAST<SetRootDescriptorBatch*>(descriptor_batch.descriptor_batch[dd.dst_root_parameter_index].get())
            ->WriteRootDescriptor(root_descriptor);
    }

    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY DescriptorSet0D3D12::CopyDescriptors(const COPY_DESCRIPTOR_SET& _copies)
{
    auto src_set = _copies.src_set->As<DescriptorSet0D3D12>();
    if (!(src_set->GetPool()->GetDesc().flags & DESCRIPTOR_POOL_FLAG_COPY_SRC))
        return BMRESULT_FAILED_INVALID_PARAMETER;

    auto AddCopyRanges = [&](auto& _dst_allocations)
    {
        update_descriptors_caches[0].ResetRangeCount();
        update_descriptors_caches[1].ResetRangeCount();

        auto&& src_allocations                      = src_set->copy_src_descriptors->data();
        auto&& src_total_num_descriptors_per_tables = src_set->GetRootSignature()->As<RootSignatureD3D12>()->GetTotalNumDescriptorsCountPerTables();
        auto&& dst_total_num_descriptors_per_tables = signature->GetTotalNumDescriptorsCountPerTables();
        for (uint32_t i_dt = 0; i_dt < _copies.num_descriptor_tables; i_dt++)
        {
            auto&& copy_dt = _copies.descriptor_tables[i_dt];
            auto&& src_total_num_descriptors = src_total_num_descriptors_per_tables.at(copy_dt.src_root_parameter_index);
            auto&& dst_total_num_descriptors = dst_total_num_descriptors_per_tables.at(copy_dt.dst_root_parameter_index);
            auto   src_abs_offsets           = src_total_num_descriptors.absolute_handle_offsets.data();
            auto   dst_abs_offsets           = dst_total_num_descriptors.absolute_handle_offsets.data();
            auto&& src_allocation            = src_allocations[dst_total_num_descriptors.type];
            auto&& dst_allocation            = _dst_allocations[dst_total_num_descriptors.type];
            auto&& cache                     = update_descriptors_caches[dst_total_num_descriptors.type];
            for (uint32_t i_copy_range = 0; i_copy_range < copy_dt.num_ranges; i_copy_range++)
            {
                auto&& src_range = copy_dt.src_ranges[i_copy_range];
                auto&& dst_range = copy_dt.dst_ranges[i_copy_range];
                cache.AddCopyRange(  copy_dt.num_descriptors[i_copy_range]
                                   , dst_allocation.OffsetCPUHandle(dst_abs_offsets[dst_range.range_index] + dst_range.first_array_element)
                                   , src_allocation.OffsetCPUHandle(src_abs_offsets[src_range.range_index] + src_range.first_array_element));
            }
        }

        update_descriptors_caches[0].ApplyCopy(device12, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        update_descriptors_caches[1].ApplyCopy(device12, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
    };

    if (pool->GetDesc().flags & DESCRIPTOR_POOL_FLAG_COPY_SRC)
    {
        AddCopyRanges(*copy_src_descriptors);

        auto CopySimple = [this](auto& _dst_allocation, const DescriptorPool0D3D12::COPY_SRC_HANDLES& _src_allocation, D3D12_DESCRIPTOR_HEAP_TYPE _type)
        { if (_src_allocation.num_descriptors) device12->CopyDescriptorsSimple(_src_allocation.num_descriptors, _dst_allocation.OffsetCPUHandle(0), _src_allocation.OffsetCPUHandle(0), _type); };
        CopySimple(allocations[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV], copy_src_descriptors->data()[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV], D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        CopySimple(allocations[D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER]    , copy_src_descriptors->data()[D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER]    , D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
    }
    else
    {
        AddCopyRanges(allocations);
    }

    auto dst_batch = descriptor_batch.descriptor_batch.data();
    auto src_batch = src_set->GetDescriptorBatch().descriptor_batch.data();
    for (uint32_t i = 0; i < _copies.num_dynamic_descriptors; i++)
    {
        auto&& dd = _copies.dynamic_descriptors[i];
        SCAST<SetRootDescriptorBatch*>(dst_batch[dd.dst_root_parameter_index].get())
            ->CopyRootDescriptor(src_batch[dd.src_root_parameter_index].get());
    }

    return BMRESULT_SUCCEED;
}


DescriptorSet0D3D12::UpdateDescriptorsCache::UpdateDescriptorsCache()
    : src_cache_size    {}
    , dst_cache_size    {}
    , src_sizees        {}
    , dst_sizees        {}
    , copy_src          {}
    , copy_dst          {}
    , current_src_size  {}
    , current_dst_size  {}
    , src_sizees_data   {}
    , dst_sizees_data   {}
    , copy_src_data     {}
    , copy_dst_data     {}
{
}

DescriptorSet0D3D12::UpdateDescriptorsCache::~UpdateDescriptorsCache()
{
}

void DescriptorSet0D3D12::UpdateDescriptorsCache::ResetRangeCount()
{
    current_src_size = 0;
    current_dst_size = 0;
}

void DescriptorSet0D3D12::UpdateDescriptorsCache::ResizeIf(uint32_t _dst_num_descriptors, uint32_t _src_num_descriptors)
{
    if (_src_num_descriptors + current_src_size > src_cache_size)
    {
        src_sizees.resize(_src_num_descriptors + current_src_size + 16/*reservation*/);
        copy_src  .resize(_src_num_descriptors + current_src_size + 16/*reservation*/);
        src_cache_size  = (uint32_t)src_sizees.size();
        src_sizees_data = src_sizees.data();
        copy_src_data   = copy_src  .data();
    }
    if (_dst_num_descriptors + current_dst_size > dst_cache_size)
    {
        dst_sizees.resize(_dst_num_descriptors + current_dst_size + 8/*reservation*/);
        copy_dst  .resize(_dst_num_descriptors + current_dst_size + 8/*reservation*/);
        dst_cache_size  = (uint32_t)src_sizees.size();
        dst_sizees_data = dst_sizees.data();
        copy_dst_data   = copy_dst  .data();
    }
}

void DescriptorSet0D3D12::UpdateDescriptorsCache::AddWriteRange(const WRITE_DESCRIPTOR_RANGE& _write_range, D3D12_CPU_DESCRIPTOR_HANDLE _dst_handle)
{
    ResizeIf(_write_range.num_descriptors, 1);
    AddRangeDst(_write_range.num_descriptors, _dst_handle);

    for (uint32_t i = 0; i < _write_range.num_descriptors; i++)
        AddRangeSrc(1, _write_range.src_views[i]->DynamicCastFromThis<IViewD3D12>()->GetCpuDescriptorAllocation()->handle);
}

void DescriptorSet0D3D12::UpdateDescriptorsCache::AddCopyRange(uint32_t _num_descriptors, D3D12_CPU_DESCRIPTOR_HANDLE _dst_handle, D3D12_CPU_DESCRIPTOR_HANDLE _src_handle)
{
    ResizeIf(_num_descriptors, _num_descriptors);
    AddRangeDst(_num_descriptors, _dst_handle);
    AddRangeSrc(_num_descriptors, _src_handle);
}

void DescriptorSet0D3D12::UpdateDescriptorsCache::ApplyCopy(ID3D12Device* _device, D3D12_DESCRIPTOR_HEAP_TYPE _type)
{
    if (current_dst_size == 0)
        return;

    _device->CopyDescriptors(  current_dst_size, copy_dst_data, dst_sizees_data
                             , current_src_size, copy_src_data, src_sizees_data
                             , _type);
}

void DescriptorSet0D3D12::UpdateDescriptorsCache::AddRangeSrc(uint32_t _num_descriptors, D3D12_CPU_DESCRIPTOR_HANDLE _src_handle)
{
    src_sizees_data[current_src_size] = _num_descriptors;
    copy_src_data  [current_src_size] = _src_handle;
    current_src_size++;
}

void DescriptorSet0D3D12::UpdateDescriptorsCache::AddRangeDst(uint32_t _num_descriptors, D3D12_CPU_DESCRIPTOR_HANDLE _dst_handle)
{
    dst_sizees_data [current_dst_size] = _num_descriptors;
    copy_dst_data   [current_dst_size] = _dst_handle;
    current_dst_size++;
}


}// namespace buma3d
