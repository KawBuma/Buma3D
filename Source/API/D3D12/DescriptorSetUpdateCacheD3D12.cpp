#include "Buma3DPCH.h"
#include "DescriptorSetUpdateCacheD3D12.h"

namespace buma3d
{

namespace /*anonymous*/
{

IViewD3D12* GetViewD3D12(IView* _view)
{
    switch (_view->GetViewDesc().type)
    {
    case buma3d::VIEW_TYPE_CONSTANT_BUFFER  : return _view->As<ConstantBufferViewD3D12>();
    case buma3d::VIEW_TYPE_SHADER_RESOURCE  : return _view->As<ShaderResourceViewD3D12>();
    case buma3d::VIEW_TYPE_UNORDERED_ACCESS : return _view->As<UnorderedAccessViewD3D12>();
    case buma3d::VIEW_TYPE_SAMPLER          : return _view->As<SamplerViewD3D12>();

    default:
        return nullptr;
    }
}

struct RANGE_COUNTS
{
    void Reset()
    {
        num_src_write_range           = 0;
        num_dst_write_range           = 0;
        num_write_range_with_copy_src = 0;
        num_src_copy_range            = 0;
        num_dst_copy_range            = 0;
        num_copy_range_with_copy_src  = 0;
    }
    bool     HasWriteRange() const { return num_dst_write_range | num_write_range_with_copy_src; }
    bool     HasCopyRange()  const { return num_dst_copy_range  | num_copy_range_with_copy_src; }
    uint32_t GetTotalSrcWriteRangeCount() const { return num_src_write_range + num_write_range_with_copy_src; }
    uint32_t GetTotalDstWriteRangeCount() const { return num_dst_write_range + num_write_range_with_copy_src; }
    uint32_t GetTotalSrcCopyRangeCount()  const { return num_src_copy_range + num_copy_range_with_copy_src; }
    uint32_t GetTotalDstCopyRangeCount()  const { return num_dst_copy_range + num_copy_range_with_copy_src; }
    uint32_t GetTotalSrcRangeCount()      const { return GetTotalSrcWriteRangeCount() + GetTotalSrcCopyRangeCount(); }
    uint32_t GetTotalDstRangeCount()      const { return GetTotalDstWriteRangeCount() + GetTotalDstCopyRangeCount(); }
    uint32_t num_src_write_range;
    uint32_t num_dst_write_range;
    uint32_t num_write_range_with_copy_src;
    uint32_t num_src_copy_range;
    uint32_t num_dst_copy_range;
    uint32_t num_copy_range_with_copy_src;
};
struct COPY_RANGE
{
    void ResetData(D3D12_CPU_DESCRIPTOR_HANDLE* _src_handles, UINT* _src_sizes, D3D12_CPU_DESCRIPTOR_HANDLE* _dst_handles, UINT* _dst_sizes)
    {
        src_handles.ResetData(_src_handles);
        src_sizes  .ResetData(_src_sizes);
        dst_handles.ResetData(_dst_handles);
        dst_sizes  .ResetData(_dst_sizes);
    }
    void AddRangeSrc(uint32_t _num_descriptors, D3D12_CPU_DESCRIPTOR_HANDLE _src_handle)
    {
        src_handles.Add(_src_handle);
        src_sizes.Add(_num_descriptors);
    }
    void AddRangeDst(uint32_t _num_descriptors, D3D12_CPU_DESCRIPTOR_HANDLE _dst_handle)
    {
        dst_handles.Add(_dst_handle);
        dst_sizes.Add(_num_descriptors);
    }
    util::TSUBVEC<D3D12_CPU_DESCRIPTOR_HANDLE>  src_handles;
    util::TSUBVEC<UINT>                         src_sizes;
    util::TSUBVEC<D3D12_CPU_DESCRIPTOR_HANDLE>  dst_handles;
    util::TSUBVEC<UINT>                         dst_sizes;
};

struct COPY_SRC_CACHE
{
    void Init(ID3D12Device* _device, D3D12_DESCRIPTOR_HEAP_TYPE _heap_type, uint32_t _num_ranges, uint32_t _num_descriptors, D3D12_CPU_DESCRIPTOR_HANDLE _handle, D3D12_CPU_DESCRIPTOR_HANDLE _copy_src_handle = {})
    {
        device     = _device;
        heap_type  = _heap_type;
        num_ranges = _num_ranges;
        handles.resize(num_ranges * 2); // srcとdst用に2倍します。
        sizes  .resize(num_ranges * 2);
        range.ResetData(  handles.data()             , sizes.data()
                        , handles.data() + num_ranges, sizes.data() + num_ranges);

        if (_copy_src_handle.ptr)
        {
            range.AddRangeDst(_num_descriptors, _copy_src_handle);
            range.AddRangeSrc(_num_descriptors, {}); // SetSrc()
            range.AddRangeDst(_num_descriptors, _handle);
            range.AddRangeSrc(_num_descriptors, _copy_src_handle);
        }
        else
        {
            range.AddRangeDst(_num_descriptors, _handle);
            range.AddRangeSrc(_num_descriptors, {}); // SetSrc()
        }
    }
    void SetSrc(D3D12_CPU_DESCRIPTOR_HANDLE _handle)
    {
        range.src_handles.data[0] = _handle;
    }
    void CopyDescriptorSet(D3D12_CPU_DESCRIPTOR_HANDLE _handle)
    {
        if constexpr (false)
        {
            SetSrc(_handle);
            device->CopyDescriptors(  num_ranges, range.dst_handles.data, range.dst_sizes.data
                                    , num_ranges, range.src_handles.data, range.src_sizes.data
                                    , heap_type);
        }
        else
        {
            // HACK: DescriptorSetUpdater::ApplyCopy() を参照してください。
            SetSrc(_handle);
            device->CopyDescriptors(  1, range.dst_handles.data, range.dst_sizes.data
                                    , 1, range.src_handles.data, range.src_sizes.data
                                    , heap_type);
            if (num_ranges == 2) // is_enabled_copy
            {
                device->CopyDescriptors(  1, range.dst_handles.data + 1, range.dst_sizes.data + 1
                                        , 1, range.src_handles.data + 1, range.src_sizes.data + 1
                                        , heap_type);
            }
        }
    }
    ID3D12Device*                               device;
    D3D12_DESCRIPTOR_HEAP_TYPE                  heap_type;
    uint32_t                                    num_ranges;
    util::DyArray<D3D12_CPU_DESCRIPTOR_HANDLE>  handles;
    util::DyArray<UINT>                         sizes;
    COPY_RANGE                                  range;
};

}// namespace /*anonymous*/


struct DescriptorSetUpdateCache::DATA
{
    DATA(DescriptorSetLayoutD3D12* _layout, DescriptorSetD3D12* _set, util::StArray<DescriptorPoolD3D12::POOL_ALLOCATION*, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER + 1>& _allocations)
        : device                { _layout->GetDevice()->As<DeviceD3D12>() }
        , layout                { _layout }
        , parameter_bindings    { _layout->GetRootParameters12Info().parameter_bindings.data() }
        , set                   { _set }
        , is_enabled_copy       { (_set->GetPool()->GetDesc().flags & DESCRIPTOR_POOL_FLAG_COPY_SRC) ? true : false }
        , allocations           { _allocations[0], _allocations[1] }
        , batch                 { set->GetDescriptorBatch().descriptor_batch.data() }
        , copy_caches           {}
    {
        auto Create = [&](D3D12_DESCRIPTOR_HEAP_TYPE _type)
        {
            auto&& al = allocations[_type];
            copy_caches[_type] = B3DMakeUnique(COPY_SRC_CACHE);
            copy_caches[_type]->Init(device->GetD3D12Device(), _type, is_enabled_copy ? 2 : 1
                                     , al->allocation.num_descriptors
                                     , al->allocation.handles.cpu_begin
                                     , al->copy_allocation.cpu_handle);
        };
        if (allocations[0]) Create(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        if (allocations[1]) Create(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
    }
    DeviceD3D12*                                                    device;
    DescriptorSetLayoutD3D12*                                       layout;
    const DescriptorSetLayoutD3D12::PARAMETER_BINDING*              parameter_bindings;
    DescriptorSetD3D12*                                             set;
    bool                                                            is_enabled_copy; // setがコピー元として利用可能かどうか。
    DescriptorPoolD3D12::POOL_ALLOCATION*                           allocations[D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER + 1];
    const util::UniquePtr<DescriptorSetD3D12::ISetDescriptorBatch>* batch;
    util::UniquePtr<COPY_SRC_CACHE>                                 copy_caches[2];

};

struct DescriptorSetUpdater::DATA
{
    DATA(DeviceD3D12* _device)
        : device            { _device }
        , device12          { _device->GetD3D12Device() }
        , src_handles       {}
        , src_sizes         {}
        , dst_handles       {}
        , dst_sizes         {}
        , counts            {}
    {
    }
    ~DATA()
    {
    }
    DeviceD3D12*                                device;
    ID3D12Device*                               device12;
    util::DyArray<D3D12_CPU_DESCRIPTOR_HANDLE>  src_handles;
    util::DyArray<UINT>                         src_sizes;
    util::DyArray<D3D12_CPU_DESCRIPTOR_HANDLE>  dst_handles;
    util::DyArray<UINT>                         dst_sizes;
    RANGE_COUNTS                                counts[2];
    COPY_RANGE                                  write_ranges[2];
    COPY_RANGE                                  write_ranges_for_copy_src[2];
    COPY_RANGE                                  copy_ranges[2];
    COPY_RANGE                                  copy_ranges_for_copy_src[2];
};

#pragma region DescriptorSetUpdateCache

DescriptorSetUpdateCache::DescriptorSetUpdateCache(DescriptorSetLayoutD3D12* _layout, DescriptorSetD3D12* _set, util::StArray<DescriptorPoolD3D12::POOL_ALLOCATION*, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER + 1>& _allocations)
    : data{ B3DNewArgs(DATA, _layout, _set, _allocations) }
{

}

DescriptorSetUpdateCache::~DescriptorSetUpdateCache()
{
    B3DDelete(data);
    data = nullptr;
}

void DescriptorSetUpdateCache::AddWriteDescriptorSets(DescriptorSetUpdater& _updator, const WRITE_DESCRIPTOR_SET& _write)
{
    for (uint32_t i_binding = 0; i_binding < _write.num_bindings; i_binding++)
    {
        PopulateWriteDescriptorBinding(_updator, _write.bindings[i_binding]);
    }
    for (uint32_t i_binding = 0; i_binding < _write.num_dynamic_bindings; i_binding++)
    {
        PopulateWriteDynamicDescriptorBinding(_updator, _write.dynamic_bindings[i_binding]);
    }
}

void DescriptorSetUpdateCache::AddCopyDescriptorSets(DescriptorSetUpdater& _updator, const COPY_DESCRIPTOR_SET& _copy)
{
    auto&& src_cache = _copy.src_set->As<DescriptorSetD3D12>()->GetUpdateCache();
    for (uint32_t i_binding = 0; i_binding < _copy.num_bindings; i_binding++)
    {
        auto&& cb = _copy.bindings[i_binding];
        if (data->parameter_bindings[cb.dst_binding_index].parameter->ParameterType != D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE)
        {
            // 宛先はルート定数です。
            auto&& dst_batch = SCAST<DescriptorSetD3D12::SetRootDescriptorBatch*>(this    ->data->batch[cb.dst_binding_index].get());
            auto&& src_batch = SCAST<DescriptorSetD3D12::SetRootDescriptorBatch*>(src_cache.data->batch[cb.src_binding_index].get());
            dst_batch->CopyRootDescriptor(src_batch);
        }
        else
        {
            PopulateCopyDescriptorBinding(_updator, _copy.bindings[i_binding], src_cache);
        }
    }
}

void DescriptorSetUpdateCache::CopyDescriptorSet(DescriptorSetD3D12* _src_set)
{
    if (data->allocations[0]) data->copy_caches[0]->CopyDescriptorSet(_src_set->GetUpdateCache().data->allocations[0]->copy_allocation.OffsetCPUHandle(0));
    if (data->allocations[1]) data->copy_caches[1]->CopyDescriptorSet(_src_set->GetUpdateCache().data->allocations[1]->copy_allocation.OffsetCPUHandle(0));
}

void DescriptorSetUpdateCache::PopulateWriteDescriptorBinding(DescriptorSetUpdater& _updator, const WRITE_DESCRIPTOR_BINDING& _write)
{
    auto&& b     = data->parameter_bindings[_write.dst_binding_index];
    auto&& alloc = data->allocations[b.heap_type];
    if (data->is_enabled_copy)
    {
        auto&& offset = b.descriptor_offset + _write.dst_first_array_element;
        auto&& dst_copy_src_handle = alloc->copy_allocation.OffsetCPUHandle(offset);
        auto&& dst_handle          = alloc->allocation     .OffsetCPUHandle(offset);

        // 初めにcopy_src_handleに各viewのディスクリプタをコピーします。
        auto&& write_range = _updator.data->write_ranges[b.heap_type];
        write_range.AddRangeDst(_write.num_descriptors, dst_copy_src_handle);
        for (uint32_t i = 0; i < _write.num_descriptors; i++)
            write_range.AddRangeSrc(1, GetViewD3D12(_write.src_views[i])->GetCpuDescriptorAllocation()->handle);

        // 次に、copy_src_handleをgpu可視ハンドルへコピーします: 2パス必要です。
        auto&& write_range_for_copy =_updator.data->write_ranges_for_copy_src[b.heap_type];
        write_range_for_copy.AddRangeDst(_write.num_descriptors, dst_handle);
        write_range_for_copy.AddRangeSrc(_write.num_descriptors, dst_copy_src_handle);
    }
    else
    {
        auto&& offset = b.descriptor_offset + _write.dst_first_array_element;
        auto&& write_range = _updator.data->write_ranges[b.heap_type];
        write_range.AddRangeDst(_write.num_descriptors, alloc->allocation.OffsetCPUHandle(offset));
        for (uint32_t i = 0; i < _write.num_descriptors; i++)
            write_range.AddRangeSrc(1, GetViewD3D12(_write.src_views[i])->GetCpuDescriptorAllocation()->handle);
    }
}

void DescriptorSetUpdateCache::PopulateWriteDynamicDescriptorBinding(DescriptorSetUpdater& _updator, const WRITE_DYNAMIC_DESCRIPTOR_BINDING& _write)
{
    auto dst_batch = SCAST<DescriptorSetD3D12::SetRootDescriptorBatch*>(data->batch[_write.dst_binding_index].get());
    dst_batch->WriteRootDescriptor(SCAST<D3D12_GPU_VIRTUAL_ADDRESS>((_write.src_buffer->GetGPUVirtualAddress()) + _write.src_buffer_offset), _write.size_in_bytes);
}

void DescriptorSetUpdateCache::PopulateCopyDescriptorBinding(DescriptorSetUpdater& _updator, const COPY_DESCRIPTOR_BINDING& _copy, const DescriptorSetUpdateCache& _src_cache)
{
    auto&& dst_b     = this     ->data->parameter_bindings[_copy.dst_binding_index];
    auto&& src_b     = _src_cache.data->parameter_bindings[_copy.src_binding_index];
    auto&& dst_alloc = this     ->data->allocations[dst_b.heap_type];
    auto&& src_alloc = _src_cache.data->allocations[dst_b.heap_type];
    if (data->is_enabled_copy)
    {
        auto&& dst_offset = dst_b.descriptor_offset + _copy.dst_first_array_element;
        auto&& src_offset = src_b.descriptor_offset + _copy.src_first_array_element;
        auto&& dst_copy_src_handle = dst_alloc->copy_allocation.OffsetCPUHandle(dst_offset);
        auto&& src_copy_src_handle = src_alloc->copy_allocation.OffsetCPUHandle(src_offset);
        auto&& dst_handle          = dst_alloc->allocation     .OffsetCPUHandle(dst_offset);

        auto&& copy_range = _updator.data->copy_ranges[dst_b.heap_type];
        copy_range.AddRangeDst(_copy.num_descriptors, dst_copy_src_handle);
        copy_range.AddRangeSrc(_copy.num_descriptors, src_copy_src_handle);

        auto&& copy_range_for_copy = _updator.data->copy_ranges_for_copy_src[dst_b.heap_type];
        copy_range_for_copy.AddRangeDst(_copy.num_descriptors, dst_handle);
        copy_range_for_copy.AddRangeSrc(_copy.num_descriptors, dst_copy_src_handle);
    }
    else
    {
        auto&& dst_offset = dst_b.descriptor_offset + _copy.dst_first_array_element;
        auto&& src_offset = src_b.descriptor_offset + _copy.src_first_array_element;
        auto&& dst_handle          = dst_alloc->allocation     .OffsetCPUHandle(dst_offset);
        auto&& src_copy_src_handle = src_alloc->copy_allocation.OffsetCPUHandle(src_offset);

        auto&& copy_range = _updator.data->copy_ranges[dst_b.heap_type];
        copy_range.AddRangeDst(_copy.num_descriptors, dst_handle);
        copy_range.AddRangeSrc(_copy.num_descriptors, src_copy_src_handle);
    }
}

#pragma endregion DescriptorSetUpdateCache

#pragma region DescriptorSetUpdater

DescriptorSetUpdater::DescriptorSetUpdater(DeviceD3D12* _device)
    : data{ B3DNewArgs(DATA, _device) }
{

}

DescriptorSetUpdater::~DescriptorSetUpdater()
{
    B3DDelete(data);
    data = nullptr;
}

void DescriptorSetUpdater::UpdateDescriptorSets(const UPDATE_DESCRIPTOR_SET_DESC& _update_desc)
{
    if (_update_desc.num_copy_descriptor_sets == 0 && _update_desc.num_write_descriptor_sets == 0)
        return;

    CalcDescriptorRangeCounts(_update_desc);
    ResizeRanges();
    PopulateUpdateDescriptorSets(_update_desc);
    ApplyCopy();
}

void DescriptorSetUpdater::CalcDescriptorRangeCounts(const UPDATE_DESCRIPTOR_SET_DESC& _update_desc)
{
    data->counts[0].Reset();
    data->counts[1].Reset();
    CalcWriteDescriptorRangeCounts(_update_desc);
    CalcCopyDescriptorRangeCounts(_update_desc);
}

void DescriptorSetUpdater::CalcWriteDescriptorRangeCounts(const UPDATE_DESCRIPTOR_SET_DESC& _update_desc)
{
    auto Add = [&](RANGE_COUNTS& _count, const WRITE_DESCRIPTOR_BINDING& _wb, DescriptorSetUpdateCache& _dst_cache)
    {
        _count.num_src_write_range += _wb.num_descriptors;
        _count.num_dst_write_range += 1;
        if (_dst_cache.data->is_enabled_copy)
            _count.num_write_range_with_copy_src += 1;
    };
    for (uint32_t i_set = 0; i_set < _update_desc.num_write_descriptor_sets; i_set++)
    {
        auto&& w         = _update_desc.write_descriptor_sets[i_set];
        auto&& dst_cache = w.dst_set->As<DescriptorSetD3D12>()->GetUpdateCache();
        for (uint32_t i_binding = 0; i_binding < w.num_bindings; i_binding++)
        {
            auto&& wb    = w.bindings[i_binding];
            auto&& type  = dst_cache.data->parameter_bindings[wb.dst_binding_index].heap_type;
            Add(data->counts[type], wb, dst_cache);
        }
    }
}

void DescriptorSetUpdater::CalcCopyDescriptorRangeCounts(const UPDATE_DESCRIPTOR_SET_DESC& _update_desc)
{
    auto Add = [&](RANGE_COUNTS& _count, const COPY_DESCRIPTOR_BINDING& _cb, DescriptorSetUpdateCache& _dst_cache)
    {
        _count.num_src_copy_range += 1;
        _count.num_dst_copy_range += 1;
        if (_dst_cache.data->is_enabled_copy)
            _count.num_copy_range_with_copy_src += 1;
    };
    for (uint32_t i_set = 0; i_set < _update_desc.num_copy_descriptor_sets; i_set++)
    {
        auto&& c         = _update_desc.copy_descriptor_sets[i_set];
        auto&& dst_cache = c.dst_set->As<DescriptorSetD3D12>()->GetUpdateCache();
        for (uint32_t i_binding = 0; i_binding < c.num_bindings; i_binding++)
        {
            auto&& cb = c.bindings[i_binding];
            auto&& parameter_binding = dst_cache.data->parameter_bindings[cb.dst_binding_index];
            if (parameter_binding.parameter->ParameterType != D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE)
                continue;
            Add(data->counts[parameter_binding.heap_type], cb, dst_cache);
        }
    }
}

void DescriptorSetUpdater::ResizeRanges()
{
    uint32_t total_src_size = data->counts[0].GetTotalSrcRangeCount() + data->counts[1].GetTotalSrcRangeCount();
    uint32_t total_dst_size = data->counts[0].GetTotalDstRangeCount() + data->counts[1].GetTotalDstRangeCount();
    if (total_src_size > data->src_handles.size())
    {
        data->src_handles.resize(total_src_size);
        data->src_sizes  .resize(total_src_size);
    }
    if (total_dst_size > data->dst_handles.size())
    {
        data->dst_handles.resize(total_dst_size);
        data->dst_sizes  .resize(total_dst_size);
    }

    auto src_handles = data->src_handles.data();
    auto src_sizes   = data->src_sizes  .data();
    auto dst_handles = data->dst_handles.data();
    auto dst_sizes   = data->dst_sizes  .data();
    uint32_t src_offset = 0;
    uint32_t dst_offset = 0;
    auto ResetData = [&](COPY_RANGE& _range, uint32_t _src_count, uint32_t _dst_count)
    {
        _range.ResetData(  src_handles + src_offset, src_sizes + src_offset
                         , dst_handles + dst_offset, dst_sizes + dst_offset);
        src_offset += _src_count;
        dst_offset += _dst_count;
    };
    auto ResetDataByRange = [&](COPY_RANGE& _range, COPY_RANGE& _range_for_copy_src, uint32_t _src_count, uint32_t _dst_count, uint32_t _num_range_with_copy_src)
    {
        ResetData(_range, _src_count, _dst_count);
        if (_num_range_with_copy_src != 0)
            ResetData(_range_for_copy_src, _num_range_with_copy_src, _num_range_with_copy_src);
    };
    auto ResetDataByType = [&](uint32_t _type)
    {
        auto&& c = data->counts[_type];
        ResetDataByRange(data->write_ranges[_type], data->write_ranges_for_copy_src[_type], c.num_src_write_range, c.num_dst_write_range, c.num_write_range_with_copy_src);
        ResetDataByRange(data->copy_ranges[_type] , data->copy_ranges_for_copy_src[_type] , c.num_src_copy_range , c.num_dst_copy_range , c.num_copy_range_with_copy_src);
    };
    ResetDataByType(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    ResetDataByType(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
}

void DescriptorSetUpdater::PopulateUpdateDescriptorSets(const UPDATE_DESCRIPTOR_SET_DESC& _update_desc)
{
    for (uint32_t i_set = 0; i_set < _update_desc.num_write_descriptor_sets; i_set++)
    {
        auto&& w = _update_desc.write_descriptor_sets[i_set];
        auto&& cache = w.dst_set->As<DescriptorSetD3D12>()->GetUpdateCache();
        cache.AddWriteDescriptorSets(*this, w);
    }
    for (uint32_t i_set = 0; i_set < _update_desc.num_copy_descriptor_sets; i_set++)
    {
        auto&& c = _update_desc.copy_descriptor_sets[i_set];
        auto&& cache = c.dst_set->As<DescriptorSetD3D12>()->GetUpdateCache();
        cache.AddCopyDescriptorSets(*this, c);
    }
}

void DescriptorSetUpdater::ApplyCopy()
{
    uint32_t dst_offset = 0;
    uint32_t src_offset = 0;
    if constexpr (false)
    {
        auto Copy = [&](D3D12_DESCRIPTOR_HEAP_TYPE _type)
        {
            auto&& c = data->counts[_type];
            if (c.HasWriteRange() || c.HasCopyRange())
            {
                uint32_t dst_count = c.GetTotalDstRangeCount();
                uint32_t src_count = c.GetTotalSrcRangeCount();
                data->device12->CopyDescriptors(  dst_count , data->dst_handles.data() + dst_offset, data->dst_sizes.data() + dst_offset
                                                , src_count , data->src_handles.data() + src_offset, data->src_sizes.data() + src_offset
                                                , _type);
                dst_offset += dst_count;
                src_offset += src_count;
            }
        };
        Copy(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        Copy(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
    }
    else
    {
        /*
        HACK: ID3D12Device::CopyDescriptors() 1回の呼び出しで、 pDestDescriptorRangeStarts と pSrcDescriptorRangeStarts 内のいずれかの要素の範囲にでも同じディスクリプタが含まれた場合、
              デバッグレイヤーはディスクリプタがオーバーラップしていると判断し、動作が未定義であるとエラーを発行します。 
              (例えば、ソースと宛先の各範囲サイズが全て同一であると仮定して、 dstRange[0] と srcRange[3] のようにお互いに干渉しない範囲のコピーでも、それらに同じディスクリプタが含まれていればエラーが発行されます。) 
              DescriptorSetUpdateCache::PopulateWriteDescriptorBinding() で説明しているディスクリプタの2パスコピーを1回のAPIコールで行う事を目的とし、
              コピー操作が範囲の要素順に行われる場合オーバーラップは起こらないため、このようなコピーは有効であると考えています。 
              実際に、少なくともnvidiaハードウェア(RTX2070)では正しくコピーが行われるようです。 
              それでも、以下のようにコピー操作を分割して行うことでエラーの発行を回避できます。
        */

        auto Copy = [&](D3D12_DESCRIPTOR_HEAP_TYPE _type, uint32_t _dst_count, uint32_t _src_count)
        {
            if (_dst_count && _src_count)
            {
                data->device12->CopyDescriptors(  _dst_count , data->dst_handles.data() + dst_offset, data->dst_sizes.data() + dst_offset
                                                , _src_count , data->src_handles.data() + src_offset, data->src_sizes.data() + src_offset
                                                , _type);
                dst_offset += _dst_count;
                src_offset += _src_count;
            }
        };
        auto CopyByType = [&](D3D12_DESCRIPTOR_HEAP_TYPE _type)
        {
            auto&& c = data->counts[_type];
            Copy(_type, c.num_dst_write_range             , c.num_src_write_range);
            Copy(_type, c.num_write_range_with_copy_src   , c.num_write_range_with_copy_src);
            Copy(_type, c.num_dst_copy_range              , c.num_src_copy_range);
            Copy(_type, c.num_copy_range_with_copy_src    , c.num_copy_range_with_copy_src);
        };
        CopyByType(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        CopyByType(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
    }
}


#pragma endregion DescriptorSetUpdater


}// namespace buma3d
