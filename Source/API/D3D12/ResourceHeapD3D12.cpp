#include "Buma3DPCH.h"
#include "ResourceHeapD3D12.h"

namespace buma3d
{

// データを読み取り、または書き込みを行わない場合に通知 https://docs.microsoft.com/en-us/windows/win32/api/d3d12/nf-d3d12-id3d12resource-map
static constexpr D3D12_RANGE NON_READ_OR_WRITTEN = { 0,0 };

B3D_APIENTRY ResourceHeapD3D12::ResourceHeapD3D12()
    : ref_count        { 1 }
    , name             {}
    , device           {}
    , desc             {}
    , device12         {}
    , heap             {}
    , mapped_range     {}
    , mapped_range12   {}
    , mapped_data      {}
    , resource_for_map {}
    , mapping_usage    {}
{

}

B3D_APIENTRY ResourceHeapD3D12::~ResourceHeapD3D12()
{
    Uninit();
}

BMRESULT
B3D_APIENTRY ResourceHeapD3D12::Init(DeviceD3D12* _device, const RESOURCE_HEAP_DESC& _desc)
{
    (device = _device)->AddRef();
    device12 = device->GetD3D12Device();

    CopyDesc(_desc);

    B3D_RET_IF_FAILED(CreateD3D12Heap());

    ConfigMappingUsage();
    if (mapping_usage != NONE)
        B3D_RET_IF_FAILED(CreateD3D12ResourceForMap());

    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY ResourceHeapD3D12::CopyDesc(const buma3d::RESOURCE_HEAP_DESC& _desc)
{
    desc = _desc;
    if (desc.alignment == 0)
        desc.alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
}

BMRESULT 
B3D_APIENTRY ResourceHeapD3D12::InitForCommitted(DeviceD3D12* _device, const COMMITTED_RESOURCE_DESC& _desc, IResource* _resource)
{
    (device = _device)->AddRef();
    device12 = device->GetD3D12Device();

    auto flags = device->GetHeapDescs12()[_desc.heap_index].Flags;
    auto res_desc12 = _resource->DynamicCastFromThis<IResourceD3D12>()->GetD3D12Resource()->GetDesc();
    auto ai = device12->GetResourceAllocationInfo(_desc.visible_node_mask, 1, &res_desc12);
    if (ai.SizeInBytes == -1)
    {
        if (util::IsEnabledDebug(this))
            device->CheckDXGIInfoQueue();
        return BMRESULT_FAILED;
    }

    desc.size_in_bytes      = ai.SizeInBytes;
    desc.alignment          = ai.Alignment;
    desc.heap_index         = _desc.heap_index;
    desc.flags              = _desc.heap_flags;
    desc.creation_node_mask = _desc.creation_node_mask;
    desc.visible_node_mask  = _desc.visible_node_mask;

    if (flags & D3D12_HEAP_FLAG_DENY_BUFFERS)
    {
        // TODO: D3D12_HEAP_FLAGがバッファを拒否するタイプの場合の実装。
        resource_for_map = _resource->DynamicCastFromThis<IResourceD3D12>()->GetD3D12Resource();
        resource_for_map->AddRef();
    }
    else
    {
        resource_for_map = _resource->DynamicCastFromThis<IResourceD3D12>()->GetD3D12Resource();
        resource_for_map->AddRef();// ID3D12Resourceのみ参照カウントを増やす。(IResourceとの循環参照回避のため)
    }

    ConfigMappingUsage();

    return BMRESULT_SUCCEED;
}

BMRESULT 
B3D_APIENTRY ResourceHeapD3D12::CreateD3D12Heap()
{
    auto&& heap_desc12 = device->GetHeapDescs12();
    if (desc.heap_index >= heap_desc12.size())
        return BMRESULT_FAILED_INVALID_PARAMETER;

    // リソースセッションのクエリ
    if (desc.flags & RESOURCE_HEAP_FLAG_PROTECTED)
    {
        if (device->GetDeviceAdapter()->GetFeatureData().protected_resource_session_support.Support == D3D12_PROTECTED_RESOURCE_SESSION_SUPPORT_FLAG_NONE)
            return BMRESULT_FAILED_NOT_SUPPORTED_FEATURE;

        // TODO: 保護されたリソースの作成
        return BMRESULT_FAILED_NOT_IMPLEMENTED;
    }

    D3D12_HEAP_DESC desc12 = heap_desc12.data()[desc.heap_index];
    desc12.SizeInBytes                 = desc.size_in_bytes;
    desc12.Alignment                   = desc.alignment;
    desc12.Properties.VisibleNodeMask  = desc.visible_node_mask;
    desc12.Properties.CreationNodeMask = desc.creation_node_mask;
    desc12.Flags                       |= util::GetNativeHeapFlags(desc.flags);

    auto hr = device->GetD3D12Device()->CreateHeap(&desc12, IID_PPV_ARGS(&heap));
    B3D_RET_IF_FAILED(HR_TRACE_IF_FAILED(hr));

    return BMRESULT_SUCCEED;
}

void 
B3D_APIENTRY ResourceHeapD3D12::ConfigMappingUsage()
{
    auto&& heap_desc12 = device->GetHeapDescs12()[desc.heap_index];

    if (heap_desc12.Flags & D3D12_HEAP_FLAG_DENY_BUFFERS)
        mapping_usage |= MAPPING::TEXTURE;

    switch (heap_desc12.Properties.Type)
    {
    case D3D12_HEAP_TYPE_DEFAULT  : mapping_usage |= MAPPING::NONE; break;
    case D3D12_HEAP_TYPE_UPLOAD   : mapping_usage |= MAPPING::WO; break;
    case D3D12_HEAP_TYPE_READBACK : mapping_usage |= MAPPING::RO; break;

    case D3D12_HEAP_TYPE_CUSTOM:
    {
        switch (heap_desc12.Properties.CPUPageProperty)
        {
        case D3D12_CPU_PAGE_PROPERTY_NOT_AVAILABLE : mapping_usage |= MAPPING::NONE; break;
        case D3D12_CPU_PAGE_PROPERTY_WRITE_COMBINE : mapping_usage |= MAPPING::WO; break;
        case D3D12_CPU_PAGE_PROPERTY_WRITE_BACK    : mapping_usage |= MAPPING::RW; break;

        default:
            B3D_ASSERT(false && __FUNCTION__);
        }
    }
    break;

    default:
        B3D_ASSERT(false && __FUNCTION__);
    }
}

BMRESULT 
B3D_APIENTRY ResourceHeapD3D12::CreateD3D12ResourceForMap()
{
    D3D12_RESOURCE_DESC buffer_desc
    {
          D3D12_RESOURCE_DIMENSION_BUFFER            // Dimension;
        , D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT // Alignment;
        , desc.size_in_bytes                         // Width;
        , 1                                          // Height;
        , 1                                          // DepthOrArraySize;
        , 1                                          // MipLevels;
        , DXGI_FORMAT_UNKNOWN                        // Format;
        , 1                                          // SampleDesc.Count;
        , 0                                          // SampleDesc.Quality;
        , D3D12_TEXTURE_LAYOUT_ROW_MAJOR             // Layout;
        , D3D12_RESOURCE_FLAG_NONE                   // Flags;
    };

    // TODO: HEAP_TIER2未満か、HEAP_FLAG_ALLOW_ONLY_BUFFERSでない場合の対応
    auto hr = device12->CreatePlacedResource(heap, 0, &buffer_desc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&resource_for_map));
    B3D_RET_IF_FAILED(HR_TRACE_IF_FAILED(hr));

    if (util::IsEnabledDebug(this))
        hr = util::SetName(resource_for_map, "ResourceHeapD3D12::resource_for_map");
    B3D_RET_IF_FAILED(HR_TRACE_IF_FAILED(hr));

    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY ResourceHeapD3D12::Uninit()
{
    name.reset();
    desc = {};
    
    if (mapped_data)
        Unmap();
    mapped_range = {};
    mapped_range12 = {};
    hlp::SafeRelease(resource_for_map);
    
    hlp::SafeRelease(heap);
    hlp::SafeRelease(device);
    device12 = nullptr;
}

BMRESULT
B3D_APIENTRY ResourceHeapD3D12::Create(DeviceD3D12* _device, const RESOURCE_HEAP_DESC& _desc, ResourceHeapD3D12** _dst)
{
    util::Ptr<ResourceHeapD3D12> ptr;
    ptr.Attach(B3DCreateImplementationClass(ResourceHeapD3D12));
    B3D_RET_IF_FAILED(ptr->Init(_device, _desc));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

BMRESULT 
B3D_APIENTRY ResourceHeapD3D12::CreateForCommitted(DeviceD3D12* _device, const COMMITTED_RESOURCE_DESC& _desc, IResource* _resource, ResourceHeapD3D12** _dst)
{
    util::Ptr<ResourceHeapD3D12> ptr;
    ptr.Attach(B3DCreateImplementationClass(ResourceHeapD3D12));
    B3D_RET_IF_FAILED(ptr->InitForCommitted(_device, _desc, _resource));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY ResourceHeapD3D12::AddRef()
{
    ++ref_count;
    B3D_REFCOUNT_DEBUG(ref_count);
}

uint32_t
B3D_APIENTRY ResourceHeapD3D12::Release()
{
    B3D_REFCOUNT_DEBUG(ref_count - 1);
    auto count = --ref_count;
    if (count == 0)
        B3DDestroyImplementationClass(this);

    return count;
}

uint32_t
B3D_APIENTRY ResourceHeapD3D12::GetRefCount() const
{
    return ref_count;
}

const char*
B3D_APIENTRY ResourceHeapD3D12::GetName() const
{
    return name ? name->c_str() : nullptr;
}

BMRESULT
B3D_APIENTRY ResourceHeapD3D12::SetName(const char* _name)
{
    if (!util::IsEnabledDebug(this))
        return BMRESULT_FAILED;
    
    if (heap)
        B3D_RET_IF_FAILED(HR_TRACE_IF_FAILED(util::SetName(heap, _name)));

    if (resource_for_map)
        B3D_RET_IF_FAILED(HR_TRACE_IF_FAILED(util::SetName(resource_for_map
                                                           , _name
                                                           ? hlp::StringConvolution("ResourceHeapD3D12::resource_for_map ", _name).c_str()
                                                           : nullptr)));

    if (name && !_name)
        name.reset();
    else
        name = B3DMakeUniqueArgs(util::NameableObjStr, _name);

    return BMRESULT_SUCCEED;
}

IDevice*
B3D_APIENTRY ResourceHeapD3D12::GetDevice() const
{
    return device;
}

const RESOURCE_HEAP_DESC&
B3D_APIENTRY ResourceHeapD3D12::GetDesc() const
{
    return desc;
}

BMRESULT
B3D_APIENTRY ResourceHeapD3D12::Map(const MAPPED_RANGE* _range_to_map)
{
    if (mapping_usage & MAPPING::NONE || mapped_data)
        return BMRESULT_FAILED_INVALID_CALL;

    const D3D12_RANGE* range;
    if (_range_to_map)
    {
        mapped_range = { _range_to_map->offset, _range_to_map->size };
        mapped_range12 = { mapped_range.offset, mapped_range.offset + mapped_range.size };
        range = &mapped_range12;
    }
    else
    {
        mapped_range = { 0, desc.size_in_bytes };
        mapped_range12 = { mapped_range.offset, mapped_range.size };
        range = nullptr;
    }

    if (mapping_usage & MAPPING::WO)
        range = &NON_READ_OR_WRITTEN;

    // TODO: D3D12_HEAP_FLAGがバッファを拒否するタイプの場合の実装。

    void** data = mapping_usage & MAPPING::TEXTURE ? nullptr : &mapped_data;
    auto hr = resource_for_map->Map(0, range, data);
    B3D_RET_IF_FAILED(HR_TRACE_IF_FAILED(hr));

    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY ResourceHeapD3D12::GetMappedData(MAPPED_RANGE* _mapped_range, void** _dst_mapped_data) const
{
    if (mapping_usage & MAPPING::NONE || !mapped_data)
        return BMRESULT_FAILED_INVALID_CALL;

    *_mapped_range = mapped_range;
    *_dst_mapped_data = (byte*)mapped_data + mapped_range.offset;

    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY ResourceHeapD3D12::FlushMappedRanges(uint32_t _num_ranges, const MAPPED_RANGE* _ranges)
{
    if (mapping_usage & (MAPPING::NONE | MAPPING::RO) || !mapped_data)
        return BMRESULT_FAILED_INVALID_CALL;

    if (_ranges == nullptr)
    {
        resource_for_map->Unmap(0, nullptr);
    }
    else
    {
        D3D12_RANGE range{ UINT64_MAX , 0 };
        for (uint32_t i = 0; i < _num_ranges; i++)
        {
            auto&& r = _ranges[i];
            range.Begin = std::min(range.Begin , r.offset);
            range.End   = std::max(range.End   , r.offset + r.size);
        }
        resource_for_map->Unmap(0, &range);
    }

    void* data = nullptr;
    auto hr = resource_for_map->Map(0, &mapped_range12, mapping_usage & MAPPING::TEXTURE ? nullptr : &data);
    B3D_ASSERT(data == mapped_data && __FUNCTION__);
    HR_TRACE_IF_FAILED(hr);

    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY ResourceHeapD3D12::InvalidateMappedRanges(uint32_t _num_ranges, const MAPPED_RANGE* _ranges)
{
    if (mapping_usage & (MAPPING::NONE | MAPPING::WO) || !mapped_data)
        return BMRESULT_FAILED_INVALID_CALL;

    resource_for_map->Unmap(0, &NON_READ_OR_WRITTEN);

    B3D_UNREFERENCED(_num_ranges, _ranges);

    void* data = nullptr;
    auto hr = resource_for_map->Map(0, &mapped_range12, mapping_usage & MAPPING::TEXTURE ? nullptr : &data);
    B3D_ASSERT(data == mapped_data);// 取得するデータのアドレスは同一か
    HR_TRACE_IF_FAILED(hr);

    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY ResourceHeapD3D12::Unmap(const MAPPED_RANGE* _used_range)
{
    if (mapping_usage & MAPPING::NONE || !mapped_data)
        return BMRESULT_FAILED_INVALID_CALL;

    const D3D12_RANGE* range;
    if (mapping_usage & MAPPING::RO)
    {
        range = &NON_READ_OR_WRITTEN;
    }
    else
    {
        if (_used_range)
        {
            mapped_range12 = { _used_range->offset, _used_range->size };
            range = &mapped_range12;
        }
        else
        {
            range = nullptr;
        }
    }

    resource_for_map->Unmap(0, range);

    mapped_data = nullptr;
    mapped_range12 = {};
    mapped_range = {};

    return BMRESULT_SUCCEED;
}

ID3D12Heap* 
B3D_APIENTRY ResourceHeapD3D12::GetD3D12Heap() const
{
    return heap;
}


}// namespace buma3d
