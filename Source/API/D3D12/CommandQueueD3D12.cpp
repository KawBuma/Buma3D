#include "Buma3DPCH.h"
#include "CommandQueueD3D12.h"

namespace buma3d
{

namespace /*anonymous*/
{

D3D12_TILE_RANGE_FLAGS GetNativeBindRegionFlags(TILED_RESOURCE_BIND_REGION_FLAGS _flags)
{
    D3D12_TILE_RANGE_FLAGS result = D3D12_TILE_RANGE_FLAG_NONE;
    if (_flags & TILED_RESOURCE_BIND_REGION_FLAG_BIND_TO_NULL)
        result = D3D12_TILE_RANGE_FLAG_NULL;

    return result;
}

D3D12_COMMAND_QUEUE_PRIORITY GetNativeCommandQueuePrioity(COMMAND_QUEUE_PRIORITY _prioity, COMMAND_QUEUE_FLAGS _flags)
{
    /*グローバルなリアルタイム優先度を持つコマンドキューを作成するには、アプリケーションに十分な特権が必要です。
    アプリケーションに十分な特権がない場合、またはアダプターもドライバーも必要なプリエンプション(実行中のタスクを一時的に中断する動作)を提供できない場合、グローバルリアルタイム優先度キューを作成する要求は失敗します。
    このような障害は、ハードウェアサポートの不足、または他のコマンドキューパラメータとの競合が原因である可能性があります。
    グローバルリアルタイムコマンドキューを作成する要求は、サポートできない場合でも、優先的に優先順位を下げることはありません。 
    要求はそのまま成功または失敗して、コマンドキューが他のどのキューよりも先に実行されることが保証されているかどうかをアプリケーションに示します。*/
    if (_flags & COMMAND_QUEUE_FLAG_PRIORITY_GLOBAL_REALTIME)
        return D3D12_COMMAND_QUEUE_PRIORITY_GLOBAL_REALTIME;

    switch (_prioity)
    {
    case buma3d::COMMAND_QUEUE_PRIORITY_DEFAULT : return D3D12_COMMAND_QUEUE_PRIORITY_NORMAL; 
    case buma3d::COMMAND_QUEUE_PRIORITY_MEDIUM  : return D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    case buma3d::COMMAND_QUEUE_PRIORITY_HIGH    : return D3D12_COMMAND_QUEUE_PRIORITY_HIGH; 
    default:
        return D3D12_COMMAND_QUEUE_PRIORITY(-1);
    }
}

D3D12_COMMAND_QUEUE_FLAGS GetNativeCommandQueueFlags(COMMAND_QUEUE_FLAGS _flags)
{
    B3D_UNREFERENCED(_flags);
    return D3D12_COMMAND_QUEUE_FLAG_NONE;
}

}// namespace /*anonymous*/

class CommandQueueD3D12::BindInfoBuffer
{
public:
    BindInfoBuffer()
        : mappings{}
    {}
    ~BindInfoBuffer() {}

public:
    inline BMRESULT PrepareBindInfo(uint32_t _num_bind_infos, const TILED_RESOURCE_BIND_INFO* _b3dbis, NodeMask _queue_node_mask)
    {
        if (_num_bind_infos > mappings.size())
            Resize(_num_bind_infos, _b3dbis);

        auto mappings_data = mappings.data();
        for (uint32_t i = 0;  i < _num_bind_infos; i++)
        {
            auto&& bi = _b3dbis->binds[i];
            B3D_RET_IF_FAILED(mappings_data[i]->Set(bi, _queue_node_mask));
        }

        return BMRESULT_SUCCEED;
    }
    
    inline void Resize(uint32_t _num_bind_infos, const TILED_RESOURCE_BIND_INFO* _b3dbis)
    {
        auto prev_size = mappings.size();
        if (_num_bind_infos > prev_size)
        {
            mappings.resize(_num_bind_infos);
            auto mappings_data = mappings.data();
            for (size_t i = prev_size, size = mappings.size(); i < size; i++)
                mappings_data[i] = B3DMakeUnique(TILE_MAPPING);
        }

        auto mappings_data = mappings.data();
        for (uint32_t i = 0; i < _num_bind_infos; i++)
        {
            auto&& bi = _b3dbis->binds[i];
            mappings_data[i]->Resize(bi);
        }
    }

    /*
        _In_                                 ID3D12Resource*                        pResource,
                                             UINT                                   NumResourceRegions,
        _In_opt_                             ID3D12Heap*                            pHeaps,
        _In_reads_opt_(NumResourceRegions)   const D3D12_TILED_RESOURCE_COORDINATE* pResourceRegionStartCoordinates,
        _In_reads_opt_(NumResourceRegions)   const D3D12_TILE_REGION_SIZE*          pResourceRegionSizes,
        _In_reads_opt_(NumRanges)            const D3D12_TILE_RANGE_FLAGS*          pRangeFlags,
        _In_reads_opt_(NumRanges)            const UINT*                            pHeapRangeStartOffsets,
        _In_reads_opt_(NumRanges)            const UINT*                            pRangeTileCounts,
                                             D3D12_TILE_MAPPING_FLAGS               Flags
    */

    struct RESOURCE_REGION_DATA
    {
        inline BMRESULT Set(const TILED_RESOURCE_BIND& _b3dbi)
        {
            auto dst_resource = _b3dbi.dst_resource->DynamicCastFromThis<IResourceD3D12>();
            B3D_RET_IF_FAILED(dst_resource->SetupBindRegions(_b3dbi.num_regions, _b3dbi.regions, resource_region_start_coordinates.data(), resource_region_sizes.data()));

            return BMRESULT_SUCCEED;
        }

        inline void Resize(const TILED_RESOURCE_BIND& _b3dbi)
        {
            resource_region_start_coordinates.resize(_b3dbi.num_regions);
            resource_region_sizes.resize(_b3dbi.num_regions);

            resource_region_start_coordinates_head = resource_region_start_coordinates.data();
            resource_region_sizes_head             = resource_region_sizes.data();
        }

        const D3D12_TILED_RESOURCE_COORDINATE*         resource_region_start_coordinates_head;
        const D3D12_TILE_REGION_SIZE*                  resource_region_sizes_head;
        util::DyArray<D3D12_TILED_RESOURCE_COORDINATE> resource_region_start_coordinates; //_In_reads_opt_(NumResourceRegions)  
        util::DyArray<D3D12_TILE_REGION_SIZE>          resource_region_sizes;             //_In_reads_opt_(NumResourceRegions)  
    };

    struct HEAP_RANGE_DATA 
    {
        inline BMRESULT Set(const TILED_RESOURCE_BIND& _b3dbi)
        {
            auto range_flags_data              = range_flags.data();
            auto heap_range_start_offsets_data = heap_range_start_offsets.data();
            auto range_tile_counts_data        = range_tile_counts.data();
            for (uint32_t i = 0; i < _b3dbi.num_regions; i++)
            {
                auto&& region                    = _b3dbi.regions[i];
                range_flags_data             [i] = GetNativeBindRegionFlags(region.flags);
                heap_range_start_offsets_data[i] = region.heap_tile_offset;
                if (region.flags & TILED_RESOURCE_BIND_REGION_FLAG_MIPTAIL)
                {
                    range_tile_counts_data[i] = UINT(region.dst_miptail_region.size_in_bytes / D3D12_TILED_RESOURCE_TILE_SIZE_IN_BYTES);
                }
                else
                {
                    auto&& t = region.dst_region.tile_size;
                    range_tile_counts_data[i] = t.width * t.height * t.depth;
                }
            }

            return BMRESULT_SUCCEED;
        }

        inline void Resize(const TILED_RESOURCE_BIND& _b3dbi)
        {
            range_flags.resize(_b3dbi.num_regions);
            heap_range_start_offsets.resize(_b3dbi.num_regions);
            range_tile_counts.resize(_b3dbi.num_regions);

            range_flags_head              = range_flags.data();
            heap_range_start_offsets_head = heap_range_start_offsets.data();
            range_tile_counts_head        = range_tile_counts.data();
        }

        const D3D12_TILE_RANGE_FLAGS*           range_flags_head;              //_In_reads_opt_(NumRanges)  
        const UINT*                             heap_range_start_offsets_head; //_In_reads_opt_(NumRanges)  
        const UINT*                             range_tile_counts_head;        //_In_reads_opt_(NumRanges)  

        util::DyArray<D3D12_TILE_RANGE_FLAGS>   range_flags;                    //_In_reads_opt_(NumRanges)  
        util::DyArray<UINT>                     heap_range_start_offsets;       //_In_reads_opt_(NumRanges)  
        util::DyArray<UINT>                     range_tile_counts;              //_In_reads_opt_(NumRanges)  
    };

    struct TILE_MAPPING_DATA
    {
        RESOURCE_REGION_DATA region_data;
        HEAP_RANGE_DATA      range_data;
    };

    struct TILE_MAPPING_REGION_AND_RANGE
    {
        ID3D12Heap*       heap;
        TILE_MAPPING_DATA data;
    };
    
    struct TILE_MAPPING
    {
        inline BMRESULT Set(const TILED_RESOURCE_BIND& _b3dbi, NodeMask _queue_node_mask)
        {
            if (_b3dbi.num_regions > region_and_ranges.size())
                Resize(_b3dbi);

            num_regions = _b3dbi.num_regions;

            resource = _b3dbi.dst_resource->DynamicCastFromThis<IResourceD3D12>()->GetD3D12Resource();
            auto region_and_ranges_data = region_and_ranges.data();
            for (uint32_t i = 0;  i < _b3dbi.num_regions;  i++)
            {
                auto&& mapping = *region_and_ranges_data[i];
                B3D_RET_IF_FAILED(mapping.data.region_data.Set(_b3dbi));
                B3D_RET_IF_FAILED(mapping.data.range_data.Set(_b3dbi));
                mapping.heap = _b3dbi.src_heaps[i]->As<ResourceHeapD3D12>()->GetD3D12Heap();
            }

            return BMRESULT_SUCCEED;
        }

        inline void Resize(const TILED_RESOURCE_BIND& _b3dbi)
        {
            uint32_t prev_size = (uint32_t)region_and_ranges.size();
            region_and_ranges.resize(_b3dbi.num_regions);
            auto region_and_ranges_data = region_and_ranges.data();
            for (uint32_t i = prev_size; i < _b3dbi.num_regions; i++)
                region_and_ranges_data[i] = B3DMakeUnique(TILE_MAPPING_REGION_AND_RANGE);

            for (uint32_t i = 0; i < _b3dbi.num_regions; i++)
            {
                auto&& mapping = *region_and_ranges_data[i];
                mapping.data.region_data.Resize(_b3dbi);
                mapping.data.range_data.Resize(_b3dbi);
            }
        }

        ID3D12Resource*                                               resource;
        uint32_t                                                      num_regions;
        util::DyArray<util::UniquePtr<TILE_MAPPING_REGION_AND_RANGE>> region_and_ranges;
    };


public:
    util::DyArray<util::UniquePtr<TILE_MAPPING>> mappings;

};

class CommandQueueD3D12::SubmitInfoBuffer
{
public:
    SubmitInfoBuffer()
        : submit_infos_head             {}
        , command_lists_in_submit_infos {}
    {}
    ~SubmitInfoBuffer() {}

    // SUBMIT_INFOを渡してSbumitInfoの一連の値をセットする。
    inline void PrepareSubmitInfo(uint32_t _num_submit_infos, const SUBMIT_INFO* _b3dsis, NodeMask _queue_node_mask)
    {
        Resize(_num_submit_infos, _b3dsis);
        auto submit_infos_data = command_lists_in_submit_infos.data();
        for (uint32_t i = 0; i < _num_submit_infos; i++)
        {
            auto&& i_si = _b3dsis[i];
            auto i_lists_data = submit_infos_data[i].data();
            for (uint32_t i_list = 0; i_list < i_si.num_command_lists_to_execute; i_list++)
                i_lists_data[i_list] = (i_si.command_lists_to_execute[i_list]->As<CommandListD3D12>()->GetD3D12GraphicsCommandList());
        }
    }
    
    inline void Resize(uint32_t _num_submit_infos, const SUBMIT_INFO* _b3dsis)
    {
        if (_num_submit_infos > command_lists_in_submit_infos.size())
        {
            command_lists_in_submit_infos.resize(_num_submit_infos);
            submit_infos_head = command_lists_in_submit_infos.data();
        }

        auto submit_infos_data = command_lists_in_submit_infos.data();
        for (uint32_t i = 0; i < _num_submit_infos; i++)
        {
            auto&& si = _b3dsis[i];
            if (si.num_command_lists_to_execute > submit_infos_data[i].size())
                submit_infos_data[i].resize(si.num_command_lists_to_execute);
        }
    }

    const util::DyArray<ID3D12CommandList*>* submit_infos_head;
public:
    util::DyArray<util::DyArray<ID3D12CommandList*>> command_lists_in_submit_infos;

};


B3D_APIENTRY CommandQueueD3D12::CommandQueueD3D12()
    : ref_count                 { 1 }
    , name                      {}
    , device                    {}
    , desc                      {}
    , d3d12_cmd_queue           {}
    , si_buffer                 {}
    , bi_buffer                 {}
    , wait_idle_fence           {}
{

}

B3D_APIENTRY CommandQueueD3D12::~CommandQueueD3D12()
{
    Uninit();
}

BMRESULT 
B3D_APIENTRY CommandQueueD3D12::Init(DeviceD3D12* _device, uint32_t _queue_index, const COMMAND_QUEUE_CREATE_DESC& _desc)
{
    (device = _device)/*->AddRef()*/;// FIXME: 循環参照回避
    CopyDesc(_queue_index, _desc);

    B3D_RET_IF_FAILED(CreateD3D12CommandQueue());

    si_buffer = B3DNew(SubmitInfoBuffer);
    bi_buffer = B3DNew(BindInfoBuffer);

    wait_idle_fence = B3DNew(WAIT_IDLE_FENCE);
    auto hr = wait_idle_fence->Create(device->GetD3D12Device());
    B3D_RET_IF_FAILED(HR_TRACE_IF_FAILED(hr));

    return BMRESULT_SUCCEED;
}

void 
B3D_APIENTRY CommandQueueD3D12::CopyDesc(uint32_t _queue_index, const COMMAND_QUEUE_CREATE_DESC& _desc)
{
    desc.type        = _desc.type;
    desc.queue_index = _queue_index;
    desc.priority    = _desc.priorities[_queue_index];
    desc.flags       = _desc.flags;
    desc.node_mask   = _desc.node_masks[_queue_index];
}

BMRESULT
B3D_APIENTRY CommandQueueD3D12::CreateD3D12CommandQueue()
{
    D3D12_COMMAND_QUEUE_DESC qdesc{};
    qdesc.Type     = util::GetNativeCommandType(desc.type);
    qdesc.Priority = GetNativeCommandQueuePrioity(desc.priority, desc.flags);
    qdesc.Flags    = GetNativeCommandQueueFlags(desc.flags);
    qdesc.NodeMask = desc.node_mask;

    D3D12_FEATURE_DATA_COMMAND_QUEUE_PRIORITY p{ qdesc.Type, (UINT)qdesc.Priority };
    auto hr = device->GetD3D12Device()->CheckFeatureSupport(D3D12_FEATURE_COMMAND_QUEUE_PRIORITY, &p, sizeof(p));
    if (!p.PriorityForTypeIsSupported || hr != S_OK)
    {
        B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_END, DEBUG_MESSAGE_CATEGORY_FLAG_INITIALIZATION
                          , "COMMAND_QUEUE_PRIORITYに指定された優先度は現在のデバイスでサポートされていません。");
        return BMRESULT_FAILED_NOT_SUPPORTED;
    }

    hr = device->GetD3D12Device()->CreateCommandQueue(&qdesc, IID_PPV_ARGS(&d3d12_cmd_queue));
    B3D_RET_IF_FAILED(HR_TRACE_IF_FAILED(hr));

    return BMRESULT_SUCCEED;
}

void 
B3D_APIENTRY CommandQueueD3D12::Uninit()
{
    B3DSafeDelete(si_buffer);
    B3DSafeDelete(bi_buffer);
    B3DSafeDelete(wait_idle_fence);
    hlp::SafeRelease(d3d12_cmd_queue);
    //hlp::SafeRelease(device);
    desc = {};
    name.reset();
}

BMRESULT
B3D_APIENTRY CommandQueueD3D12::Create(DeviceD3D12* _device, uint32_t _queue_index, const COMMAND_QUEUE_CREATE_DESC& _desc, CommandQueueD3D12** _dst)
{
    util::Ptr<CommandQueueD3D12> ptr;
    ptr.Attach(B3DCreateImplementationClass(CommandQueueD3D12));
    B3D_RET_IF_FAILED(ptr->Init(_device, _queue_index, _desc));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY CommandQueueD3D12::AddRef()
{
    ++ref_count;
    B3D_REFCOUNT_DEBUG(ref_count);
}

uint32_t
B3D_APIENTRY CommandQueueD3D12::Release()
{
    B3D_REFCOUNT_DEBUG(ref_count - 1);
    auto count = --ref_count;
    if (count == 0)
        B3DDestroyImplementationClass(this);

    return count;
}

uint32_t
B3D_APIENTRY CommandQueueD3D12::GetRefCount() const
{
    return ref_count;
}

const char*
B3D_APIENTRY CommandQueueD3D12::GetName() const
{
    return name ? name->c_str() : nullptr;
}

BMRESULT
B3D_APIENTRY CommandQueueD3D12::SetName(const char* _name)
{
    if (!util::IsEnabledDebug(this))
        return BMRESULT_FAILED;
    B3D_RET_IF_FAILED(HR_TRACE_IF_FAILED(util::SetName(d3d12_cmd_queue, _name)));

    if (wait_idle_fence)
    {
        B3D_RET_IF_FAILED(HR_TRACE_IF_FAILED(util::SetName(wait_idle_fence->fence
                                                           , _name
                                                           ? hlp::StringConvolution("CommandQueueD3D12::wait_idle_fence (", _name, ")").c_str()
                                                           : nullptr)));
    }

    if (name && !_name)
        name.reset();
    else
        name = B3DMakeUniqueArgs(util::NameableObjStr, _name);

    return BMRESULT_SUCCEED;
}

IDevice*
B3D_APIENTRY CommandQueueD3D12::GetDevice() const
{
    return device;
}

const COMMAND_QUEUE_DESC&
B3D_APIENTRY CommandQueueD3D12::GetDesc() const
{
    return desc;
}

BMRESULT
B3D_APIENTRY CommandQueueD3D12::WaitIdle()
{
    auto hr = wait_idle_fence->Signal(d3d12_cmd_queue);
    B3D_RET_IF_FAILED(HR_TRACE_IF_FAILED(hr));

    hr = wait_idle_fence->Wait();
    B3D_RET_IF_FAILED(HR_TRACE_IF_FAILED(hr));

    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY CommandQueueD3D12::SubmitTileBindings(const SUBMIT_TILE_BINDINGS_DESC& _desc)
{
    B3D_RET_IF_FAILED(bi_buffer->PrepareBindInfo(_desc.num_bind_infos, _desc.bind_infos, desc.node_mask));

    auto mappings_data = bi_buffer->mappings.data();
    for (uint32_t i_bi = 0; i_bi < _desc.num_bind_infos; i_bi++)
    {
        auto&& bi = _desc.bind_infos[i_bi];

        // GPUフェンス待機操作を送信
        B3D_RET_IF_FAILED(SubmitWait(bi.wait_fence));

        // タイルマッピング
        {
            auto&& mapping = *mappings_data[i_bi];
            auto&& region_and_ranges_data = mapping.region_and_ranges.data();
            for (uint32_t i_bind = 0; i_bind < bi.num_binds; i_bind++)
            {
                auto&& r = *region_and_ranges_data[i_bind];
                d3d12_cmd_queue->UpdateTileMappings(  mapping.resource
                                                    , mapping.num_regions
                                                    , r.data.region_data.resource_region_start_coordinates_head
                                                    , r.data.region_data.resource_region_sizes_head
                                                    , r.heap
                                                    , mapping.num_regions
                                                    , r.data.range_data.range_flags_head
                                                    , r.data.range_data.heap_range_start_offsets_head
                                                    , r.data.range_data.range_tile_counts_head
                                                    , D3D12_TILE_MAPPING_FLAG_NONE);
            }
        }

        // GPUシグナル操作を送信
        B3D_RET_IF_FAILED(SubmitSignal(bi.signal_fence));
    }

    if (_desc.signal_fence_to_cpu)
        B3D_RET_IF_FAILED(_desc.signal_fence_to_cpu->As<FenceD3D12>()->SubmitSignalToCpu(d3d12_cmd_queue));

    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY CommandQueueD3D12::Submit(const SUBMIT_DESC& _desc)
{
    si_buffer->PrepareSubmitInfo(_desc.num_submit_infos, _desc.submit_infos, desc.node_mask);

    for (uint32_t i_si = 0; i_si < _desc.num_submit_infos; i_si++)
    {
        auto&& si = _desc.submit_infos[i_si];

        // GPUフェンス待機操作を送信
        B3D_RET_IF_FAILED(SubmitWait(si.wait_fence));

        // コマンドリスト実行操作を送信
        auto&& cmd_lists = si_buffer->submit_infos_head[i_si];
        d3d12_cmd_queue->ExecuteCommandLists(si.num_command_lists_to_execute, cmd_lists.data());

        // GPUシグナル操作を送信
        B3D_RET_IF_FAILED(SubmitSignal(si.signal_fence));
    }

    if (_desc.signal_fence_to_cpu)
        B3D_RET_IF_FAILED(_desc.signal_fence_to_cpu->As<FenceD3D12>()->SubmitSignalToCpu(d3d12_cmd_queue));

    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY CommandQueueD3D12::SubmitSignal(const SUBMIT_SIGNAL_DESC& _desc) 
{
    B3D_RET_IF_FAILED(SubmitSignal(_desc.signal_fence));

    if (_desc.signal_fence_to_cpu)
        B3D_RET_IF_FAILED(_desc.signal_fence_to_cpu->As<FenceD3D12>()->SubmitSignalToCpu(d3d12_cmd_queue));

    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY CommandQueueD3D12::SubmitWait(const SUBMIT_WAIT_DESC& _desc)
{
    B3D_RET_IF_FAILED(SubmitWait(_desc.wait_fence));

    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY CommandQueueD3D12::InsertMarker(const char* _marker_name, const COLOR4* _color)
{
    auto col = _color
        ? PIX_COLOR(SCAST<BYTE>(_color->r * 255.f), SCAST<BYTE>(_color->g * 255.f), SCAST<BYTE>(_color->b * 255.f))
        : PIX_COLOR_DEFAULT;
    PIXSetMarker(d3d12_cmd_queue, SCAST<UINT64>(col), _marker_name);

    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY CommandQueueD3D12::BeginMarker(const char* _marker_name, const COLOR4* _color)
{
    auto col = _color
        ? PIX_COLOR(SCAST<BYTE>(_color->r * 255.f), SCAST<BYTE>(_color->g * 255.f), SCAST<BYTE>(_color->b * 255.f))
        : PIX_COLOR_DEFAULT;
    PIXBeginEvent(d3d12_cmd_queue, SCAST<UINT64>(col), _marker_name);

    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY CommandQueueD3D12::EndMarker() 
{
    PIXEndEvent(d3d12_cmd_queue);
    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY CommandQueueD3D12::GetTimestampFrequency(uint64_t* _dst_frequency) const
{
    uint64_t frequency = 0;
    auto hr = d3d12_cmd_queue->GetTimestampFrequency(&frequency);
    HR_TRACE_IF_FAILED(hr);
    B3D_RET_IF_FAILED(util::GetBMResultFromHR(hr));

    *_dst_frequency = frequency;
    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY CommandQueueD3D12::GetClockCalibration(uint64_t* _dst_gpu_timestamp, uint64_t* _dst_cpu_timestamp) const
{
    uint64_t gpu_timestamp = 0;
    uint64_t cpu_timestamp = 0;
    auto hr = d3d12_cmd_queue->GetClockCalibration(&gpu_timestamp, &cpu_timestamp);
    HR_TRACE_IF_FAILED(hr);
    B3D_RET_IF_FAILED(util::GetBMResultFromHR(hr));

    *_dst_gpu_timestamp = gpu_timestamp;
    *_dst_cpu_timestamp = cpu_timestamp;
    return BMRESULT_SUCCEED;
}

ID3D12CommandQueue* 
B3D_APIENTRY CommandQueueD3D12::GetD3D12CommandQueue() 
{
    return d3d12_cmd_queue; 
}


BMRESULT
B3D_APIENTRY CommandQueueD3D12::SubmitSignal(const FENCE_SUBMISSION& _signal)
{
    for (uint32_t i = 0; i < _signal.num_fences; i++)
    {
        auto fence12 = _signal.fences[i]->As<FenceD3D12>();
        B3D_RET_IF_FAILED(fence12->SubmitSignal(d3d12_cmd_queue, _signal.fence_values + i));
    }
    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY CommandQueueD3D12::SubmitWait(const FENCE_SUBMISSION& _wait)
{
    for (uint32_t i = 0; i < _wait.num_fences; i++)
    {
        auto fence12 = _wait.fences[i]->As<FenceD3D12>();
        B3D_RET_IF_FAILED(fence12->SubmitWait(d3d12_cmd_queue, _wait.fence_values + i));
    }
    return BMRESULT_SUCCEED;
}


}// namespace buma3d
