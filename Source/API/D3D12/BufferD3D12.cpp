#include "Buma3DPCH.h"
#include "BufferD3D12.h"

namespace buma3d
{

B3D_APIENTRY BufferD3D12::BufferD3D12()
    : ref_count           { 1 }
    , name                {}
    , device              {}
    , desc                {}
    , desc_data           {}
    , bind_node_masks     {}
    , create_type         {}
    , is_bound            {}
    , gpu_virtual_address {}
    , device12            {}
    , heap                {}
    , buffer              {}
{

}

B3D_APIENTRY BufferD3D12::~BufferD3D12()
{
    Uninit();
}

BMRESULT
B3D_APIENTRY BufferD3D12::Init(RESOURCE_CREATE_TYPE _create_type, DeviceD3D12* _device, const RESOURCE_DESC& _desc)
{
    create_type = _create_type;

    (device = _device)->AddRef();
    device12 = _device->GetD3D12Device();
    CopyDesc(_desc);

    BMRESULT bmr = BMRESULT_FAILED;
    switch (create_type)
    {
    case RESOURCE_CREATE_TYPE_PLACED:
        bmr = InitAsPlaced();
        break;
    case RESOURCE_CREATE_TYPE_RESERVED:
        bmr = InitAsReserved();
        break;
    case RESOURCE_CREATE_TYPE_COMMITTED:
        bmr = BMRESULT_SUCCEED;
        break;
    default:
        break;
    }

    return bmr;
}

BMRESULT
B3D_APIENTRY BufferD3D12::CopyDesc(const RESOURCE_DESC& _desc)
{
    desc = _desc;

    desc.buffer.usage |= BUFFER_USAGE_FLAG_COPY_SRC;
    desc.buffer.usage |= BUFFER_USAGE_FLAG_COPY_DST;
    desc.buffer.usage |= BUFFER_USAGE_FLAG_CONSTANT_BUFFER;

    if (desc.buffer.usage & BUFFER_USAGE_FLAG_SHADER_RESOURCE_BUFFER)
    {
        desc.buffer.usage |= BUFFER_USAGE_FLAG_STRUCTURED_BYTEADDRESS_TBUFFER;
        desc.buffer.usage |= BUFFER_USAGE_FLAG_TYPED_SHADER_RESOURCE_BUFFER;
    }
    if (desc.buffer.usage & BUFFER_USAGE_FLAG_UNORDERED_ACCESS_BUFFER)
    {
        desc.buffer.usage |= BUFFER_USAGE_FLAG_STRUCTURED_BYTEADDRESS_TBUFFER;
        desc.buffer.usage |= BUFFER_USAGE_FLAG_TYPED_UNORDERED_ACCESS_BUFFER;
    }

    desc.buffer.usage |= BUFFER_USAGE_FLAG_INDEX_BUFFER;
    desc.buffer.usage |= BUFFER_USAGE_FLAG_VERTEX_BUFFER;
    desc.buffer.usage |= BUFFER_USAGE_FLAG_INDIRECT_BUFFER;
    desc.buffer.usage |= BUFFER_USAGE_FLAG_STREAM_OUTPUT_BUFFER;
    desc.buffer.usage |= BUFFER_USAGE_FLAG_STREAM_OUTPUT_COUNTER_BUFFER;
    desc.buffer.usage |= BUFFER_USAGE_FLAG_CONDITIONAL_RENDERING;

    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY BufferD3D12::PrepareBindNodeMasks(uint32_t _heap_index, uint32_t _num_bind_node_masks, const NodeMask* _bind_node_masks)
{
    auto&& props = device->GetResourceHeapPropertiesForImpl()[_heap_index];
    bool is_multi_instance_heap = props.flags & RESOURCE_HEAP_PROPERTY_FLAG_MULTI_INSTANCE;
    bool is_invalod = (!is_multi_instance_heap && _num_bind_node_masks != 0);
         is_invalod |= (is_multi_instance_heap && _num_bind_node_masks != (uint32_t)device12->GetNodeCount());
    if (is_invalod)
    {
        B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_INITIALIZATION
                          , "非マルチインスタンスヒープへのバインドの際に、num_bind_node_masksは0以外であってはなりません。また、マルチインスタンスヒープへのバインドの際に、num_bind_node_masksはIDevice内のノード数と同じである必要があります。");
        return BMRESULT_FAILED_INVALID_PARAMETER;
    }

    if (is_multi_instance_heap)
    {
        // ノードマスクをキャッシュ
        bind_node_masks = B3DMakeUnique(decltype(bind_node_masks)::element_type);
        bind_node_masks->resize(_num_bind_node_masks);
        auto masks = _bind_node_masks;
        for (auto& i_node : *bind_node_masks)
        {
            if (hlp::CountBits(*masks) != 1)
            {
                B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_INITIALIZATION,
                                  "ノードiに対するbind_node_masks[i]の要素は、ヒープメモリのインスタンスを指定する単一のビットを指定する必要があります。");
                return BMRESULT_FAILED_INVALID_PARAMETER;
            }
            i_node = *masks++;
        }
    }

    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY BufferD3D12::InitAsPlaced()
{
    D3D12_RESOURCE_DESC rd{};
    util::GetNativeResourceDesc(desc, &rd);

    // このリソースが作成可能か確認。
    // WARNING: 現状ノード0(0x1)を決め打ちしているので、別ノードでのdescの有効性を確認する事が出来ていないが、恐らくノードマスクが必要な理由はdescではなく、
    // 戻り値D3D12_RESOURCE_ALLOCATION_INFO::SizeInBytesの各ノードによる値の変動を識別するためである可能性が高い。
    auto ai = device12->GetResourceAllocationInfo(B3D_DEFAULT_NODE_MASK, 1, &rd);
    if (ai.SizeInBytes == -1)// 無効なdescが指定された場合-1が返る。
    {
        if (util::IsEnabledDebug(this))
            device->CheckDXGIInfoQueue();
        return BMRESULT_FAILED;
    }

    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY BufferD3D12::InitAsReserved()
{
    D3D12_RESOURCE_DESC desc12{};
    util::GetNativeResourceDesc(desc, &desc12);

    auto hr = device12->CreateReservedResource(&desc12
                                               , util::GetFixedStateFromResourceFlags(desc.flags, desc.buffer.usage & BUFFER_USAGE_FLAG_RAY_TRACING)
                                               , nullptr
                                               , IID_PPV_ARGS(&buffer));
    B3D_RET_IF_FAILED(HR_TRACE_IF_FAILED(hr));

    // NOTE: Reserved用ヒープは作成しません。

    MarkAsBound();

    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY BufferD3D12::InitAsCommitted(DeviceD3D12* _device, const COMMITTED_RESOURCE_DESC& _desc)
{
    Init(RESOURCE_CREATE_TYPE_COMMITTED, _device, _desc.resource_desc);

    // コミットされたリソースを作成
    D3D12_HEAP_PROPERTIES hp = device->GetHeapDescs12()[_desc.heap_index].Properties;
    hp.CreationNodeMask = _desc.creation_node_mask;
    hp.VisibleNodeMask = _desc.visible_node_mask;

    D3D12_RESOURCE_DESC desc12{};
    util::GetNativeResourceDesc(_desc.resource_desc, &desc12);

    auto hr = device12->CreateCommittedResource(&hp, util::GetNativeHeapFlags(_desc.heap_flags)
                                                , &desc12
                                                , util::GetFixedStateFromHeapType(hp.Type, _desc.resource_desc.buffer.usage & BUFFER_USAGE_FLAG_RAY_TRACING)
                                                , nullptr
                                                , IID_PPV_ARGS(&buffer));
    B3D_RET_IF_FAILED(HR_TRACE_IF_FAILED(hr));

    // コミットリソース用ヒープを作成。
    B3D_RET_IF_FAILED(ResourceHeapD3D12::CreateForCommitted(_device, _desc, this, &heap));

    MarkAsBound();

    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY BufferD3D12::MarkAsBound()
{
    is_bound = true;
    gpu_virtual_address = SCAST<GpuVirtualAddress>(buffer->GetGPUVirtualAddress());
    if (util::IsEnabledDebug(this) && name)
        util::SetName(buffer, *name);
}

void
B3D_APIENTRY BufferD3D12::Uninit()
{
    desc = {};
    desc_data.reset();
    is_bound = false;
    hlp::SafeRelease(buffer);
    hlp::SafeRelease(heap);
    hlp::SafeRelease(device);
    device12 = nullptr;
    bind_node_masks.reset();
    gpu_virtual_address = 0;
    name.reset();
}

BMRESULT
B3D_APIENTRY BufferD3D12::Create(RESOURCE_CREATE_TYPE _create_type, DeviceD3D12* _device, const RESOURCE_DESC& _desc, BufferD3D12** _dst)
{
    util::Ptr<BufferD3D12> ptr;
    ptr.Attach(B3DCreateImplementationClass(BufferD3D12));
    B3D_RET_IF_FAILED(ptr->Init(_create_type, _device, _desc));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY BufferD3D12::CreateCommitted(DeviceD3D12* _device, const COMMITTED_RESOURCE_DESC& _desc, BufferD3D12** _dst)
{
    util::Ptr<BufferD3D12> ptr;
    ptr.Attach(B3DCreateImplementationClass(BufferD3D12));
    B3D_RET_IF_FAILED(ptr->InitAsCommitted(_device, _desc));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY BufferD3D12::Bind(const BIND_RESOURCE_HEAP_INFO* _info)
{
    if (create_type == RESOURCE_CREATE_TYPE_COMMITTED && !is_bound)
    {
        // CreateForComitted関数からの呼び出しなので、何もしない。
    }
    else
    {
        if (create_type != RESOURCE_CREATE_TYPE_PLACED)
        {
            B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_RESOURCE_MANIPULATION
                              , "CreatePlacedResource以外で作成されたリソースからの呼び出しは無効です。");
            return BMRESULT_FAILED_INVALID_CALL;
        }
        else if (is_bound)
        {
            B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_RESOURCE_MANIPULATION
                              , "ヒープは既にバインドされています。CreatePlacedResourceで作成されたリソースに一度バインドしたヒープは変更できず、このリソースが開放されるまでの間固有である必要があります。");
            return BMRESULT_FAILED_INVALID_CALL;
        }

        // マルチインスタンスヒープ/スワップチェイン用バインドマスクを設定。
        B3D_RET_IF_FAILED(PrepareBindNodeMasks(_info->src_heap->GetDesc().heap_index, _info->num_bind_node_masks, _info->bind_node_masks));
    }

    D3D12_RESOURCE_DESC desc12{};
    util::GetNativeResourceDesc(desc, &desc12);

    auto src_heap = _info->src_heap->As<ResourceHeapD3D12>();
    auto&& hp = device->GetHeapDescs12()[src_heap->GetDesc().heap_index].Properties;
    auto hr = device12->CreatePlacedResource(src_heap->GetD3D12Heap(), _info->src_heap_offset
                                             , &desc12
                                             , util::GetFixedStateFromHeapType(hp.Type, desc.buffer.usage & BUFFER_USAGE_FLAG_RAY_TRACING)
                                             , nullptr
                                             , IID_PPV_ARGS(&buffer));
    B3D_RET_IF_FAILED(HR_TRACE_IF_FAILED(hr));

    if (create_type == RESOURCE_CREATE_TYPE_PLACED)
        (heap = src_heap)->AddRef(); // カウントは成功後に追加

    MarkAsBound();

    return BMRESULT_SUCCEED;
}

ID3D12Resource*
B3D_APIENTRY BufferD3D12::GetD3D12Resource() const
{
    return buffer;
}

RESOURCE_CREATE_TYPE
B3D_APIENTRY BufferD3D12::GetCreateType() const
{
    return create_type;
}

BMRESULT
B3D_APIENTRY BufferD3D12::SetupBindRegions(uint32_t _num_regions, const TILED_RESOURCE_BIND_REGION* _regions, D3D12_TILED_RESOURCE_COORDINATE* _dst_start_coords, D3D12_TILE_REGION_SIZE* _dst_region_sizes) const
{
    for (uint32_t i = 0; i < _num_regions; i++)
    {
        auto&& r  = _regions[i];
        auto&& sc = _dst_start_coords[i];
        auto&& rs = _dst_region_sizes[i];

        // 有効性の検証
        //if (util::IsEnabledDebug(this))
        {
            if (r.dst_region.tile_offset.y != 0 || r.dst_region.tile_offset.z != 0)
            {
                B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_RESOURCE_MANIPULATION
                                  , "バッファの場合、TILED_REGION::tile_offset.y,zの値は0である必要があります。");
                return BMRESULT_FAILED_INVALID_PARAMETER;
            }
            if (r.dst_region.tile_size.height != 1 || r.dst_region.tile_size.depth != 1)
            {
                B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_RESOURCE_MANIPULATION
                                  , "バッファの場合、TILED_REGION::tile_size.height,depthの値は1である必要があります。");
                return BMRESULT_FAILED_INVALID_PARAMETER;
            }
            if (r.dst_region.subresource.array_slice != 0 ||
                r.dst_region.subresource.mip_slice   != 0 ||
                r.dst_region.subresource.aspect      != 0)
            {
                B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_RESOURCE_MANIPULATION
                                  , "バッファの場合、TILED_REGION::subresourceの全ての値は0である必要があります。");
                return BMRESULT_FAILED_INVALID_PARAMETER;
            }
            if (r.flags & (TILED_RESOURCE_BIND_REGION_FLAG_MIPTAIL | TILED_RESOURCE_BIND_REGION_FLAG_METADATA))
            {
                B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_RESOURCE_MANIPULATION
                                  , "バッファの場合、TILED_RESOURCE_BIND_REGION::flagsにTILED_RESOURCE_BIND_REGION_FLAG_MIPTAIL,TILED_RESOURCE_BIND_REGION_FLAG_METADATA値が含まれていない必要があります。");
                return BMRESULT_FAILED_INVALID_PARAMETER;
            }
        }

        sc.X           = r.dst_region.tile_offset.x;
        sc.Y           = 0;
        sc.Z           = 0;
        sc.Subresource = 0;

        rs.UseBox = FALSE;
        rs.NumTiles = r.dst_region.tile_size.width;
    }

    return BMRESULT_SUCCEED;
}

uint32_t
B3D_APIENTRY BufferD3D12::GetTiledResourceAllocationInfo(TILED_RESOURCE_ALLOCATION_INFO* _dst_infos) const
{
    if (create_type != RESOURCE_CREATE_TYPE_RESERVED)
    {
        B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_STATE_GETTING
                          , "リソースはCreateReservedResourceから作成されている必要があります。");
        return 0;
    }

    if (_dst_infos)
    {
        auto&& fp = _dst_infos->format_properties;
        fp.aspect                      = TEXTURE_ASPECT_FLAG_NONE;
        fp.flags                       = TILED_RESOURCE_FORMAT_FLAG_NONE;
        fp.tile_shape.width_in_texels  = D3D12_TILED_RESOURCE_TILE_SIZE_IN_BYTES;
        fp.tile_shape.height_in_texels = 1;
        fp.tile_shape.depth_in_texels  = 1;

        auto&& miptail = _dst_infos->mip_tail;
        miptail.is_required      = false;
        miptail.first_mip_slice  = 0;
        miptail.size             = 0;
        miptail.offset           = 0;
        miptail.stride           = 0;
    }

    return 1;
}

void
B3D_APIENTRY BufferD3D12::AddRef()
{
    ++ref_count;
    B3D_REFCOUNT_DEBUG(ref_count);
}

uint32_t
B3D_APIENTRY BufferD3D12::Release()
{
    B3D_REFCOUNT_DEBUG(ref_count - 1);
    auto count = --ref_count;
    if (count == 0)
        B3DDestroyImplementationClass(this);

    return count;
}

uint32_t
B3D_APIENTRY BufferD3D12::GetRefCount() const
{
    return ref_count;
}

const char*
B3D_APIENTRY BufferD3D12::GetName() const
{
    return name ? name->c_str() : nullptr;
}

BMRESULT
B3D_APIENTRY BufferD3D12::SetName(const char* _name)
{
    if (!util::IsEnabledDebug(this))
        return BMRESULT_FAILED;

    if (buffer)
        B3D_RET_IF_FAILED(HR_TRACE_IF_FAILED(util::SetName(buffer, _name)));

    if (name && !_name)
        name.reset();
    else
        name = B3DMakeUniqueArgs(util::NameableObjStr, _name);

    return BMRESULT_SUCCEED;
}

IDevice*
B3D_APIENTRY BufferD3D12::GetDevice() const
{
    return device;
}

const RESOURCE_DESC&
B3D_APIENTRY BufferD3D12::GetDesc() const
{
    return desc;
}

IResourceHeap*
B3D_APIENTRY BufferD3D12::GetHeap() const
{
    return heap;
}

GpuVirtualAddress
B3D_APIENTRY BufferD3D12::GetGPUVirtualAddress() const
{
    return gpu_virtual_address;
}


}// namespace buma3d
