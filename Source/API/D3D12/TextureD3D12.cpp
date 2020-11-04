#include "Buma3DPCH.h"
#include "TextureD3D12.h"

namespace buma3d
{

namespace /*anomymous*/
{

RESOURCE_FLAGS SwapchainFlagsToResourceFlags(const SWAP_CHAIN_DESC& _swapchain_desc)
{
    RESOURCE_FLAGS result = RESOURCE_FLAG_NONE;

    if (_swapchain_desc.flags & SWAP_CHAIN_FLAG_PROTECT_CONTENTS)
        result |= RESOURCE_FLAG_PROTECTED;

    if (_swapchain_desc.buffer.flags & SWAP_CHAIN_BUFFER_FLAG_ALLOW_SIMULTANEOUS_ACCESS)
        result |= RESOURCE_FLAG_ALLOW_SIMULTANEOUS_ACCESS;

    return result;
}

TEXTURE_USAGE_FLAGS SwapchainFlagsToUsageFlags(SWAP_CHAIN_BUFFER_FLAGS _flags)
{
    TEXTURE_USAGE_FLAGS result = TEXTURE_USAGE_FLAG_NONE;

    if (_flags & SWAP_CHAIN_BUFFER_FLAG_COPY_SRC)
        result |= TEXTURE_USAGE_FLAG_COPY_SRC;

    if (_flags & SWAP_CHAIN_BUFFER_FLAG_COPY_DST)
        result |= TEXTURE_USAGE_FLAG_COPY_DST;

    if (_flags & SWAP_CHAIN_BUFFER_FLAG_SHADER_RESOURCE)
        result |= TEXTURE_USAGE_FLAG_SHADER_RESOURCE;

    if (_flags & SWAP_CHAIN_BUFFER_FLAG_UNORDERED_ACCESS)
        result |= TEXTURE_USAGE_FLAG_UNORDERED_ACCESS;

    if (_flags & SWAP_CHAIN_BUFFER_FLAG_COLOR_ATTACHMENT)
        result |= TEXTURE_USAGE_FLAG_COLOR_ATTACHMENT;

    if (_flags & SWAP_CHAIN_BUFFER_FLAG_INPUT_ATTACHMENT)
        result |= TEXTURE_USAGE_FLAG_INPUT_ATTACHMENT;

    return result;
}

}//namespace /*anomymous*/

B3D_APIENTRY TextureD3D12::TextureD3D12()
    : ref_count           { 1 }
    , name                {}
    , device              {}
    , desc                {}
    , desc_data           {}
    , bind_node_masks     {}
    , create_type         {}
    , is_bound            {}
    , device12            {}
    , heap                {}
    , texture             {}
    , tiled_resource_data {}
{

}

B3D_APIENTRY TextureD3D12::~TextureD3D12()
{
    Uninit();
}

BMRESULT
B3D_APIENTRY TextureD3D12::Init(RESOURCE_CREATE_TYPE _create_type, DeviceD3D12* _device, const RESOURCE_DESC& _desc)
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
    case RESOURCE_CREATE_TYPE_SWAP_CHAIN:
        bmr = BMRESULT_SUCCEED;
        break;
    default:
        break;
    }

    return bmr;
}

BMRESULT
B3D_APIENTRY TextureD3D12::CopyDesc(const RESOURCE_DESC& _desc)
{
    desc = _desc;

    if (desc.dimension == RESOURCE_DIMENSION_TEX3D && desc.texture.array_size != 1)
    {
        B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_INITIALIZATION, "TEXTURE_DESC::array_sizeはRESOURCE_DIMENSION_TEX3Dの場合1である必要があります。");
        return BMRESULT_FAILED_INVALID_PARAMETER;
    }
    else if (desc.texture.extent.depth != 1)
    {
        B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_INITIALIZATION, "TEXTURE_DESC::extent.depthはRESOURCE_DIMENSION_TEX3D以外の場合1である必要があります。");
        return BMRESULT_FAILED_INVALID_PARAMETER;
    }

    if (desc.texture.mip_levels == 0)
    {
        B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_INITIALIZATION
                          , "TEXTURE_DESC::mip_levelsは0であってはなりません。B3D_USE_ALL_MIPS定数を使用して利用可能な全てのミップレベルを割り当てることが出来ます。");
        return BMRESULT_FAILED_INVALID_PARAMETER;
    }
    desc.texture.mip_levels = util::CalcMipLevels(desc.texture);

    desc_data = B3DMakeUnique(DESC_DATA);
    if (_desc.texture.optimized_clear_value)
    {
        desc_data->optimized_clear_value = B3DMakeUniqueArgs(CLEAR_VALUE, *_desc.texture.optimized_clear_value);
        desc.texture.optimized_clear_value = desc_data->optimized_clear_value.get();
    }

    // TEXTURE_FORMAT_DESC
    auto&& tfd = desc.texture.format_desc;
    if (util::IsTypelessFormat(tfd.format))
    {
        auto&& fc = device->GetFormatCompatibilityChecker();
        auto&& dd = desc_data;
        if (tfd.num_mutable_formats == 0)// default
        {
            dd->mutable_formats = fc.GetTypelessCompatibleFormats().at(tfd.format)->compatible_formats;
            dd->is_shared_from_typeless_compatible_formats = true;
        }
        else
        {
            auto&& f = fc.CheckCompatibility(tfd);
            if (f == nullptr)
            {
                B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_INITIALIZATION
                                  , "TEXTURE_FORMAT_DESC::mutable_formatsに、TYPELESSフォーマットと互換性が無い、または現在のデバイスでは対応していないフォーマットが含まれています。");
                return BMRESULT_FAILED_INVALID_PARAMETER;
            }

            auto&& mf = dd->mutable_formats = B3DMakeShared(util::DyArray<RESOURCE_FORMAT>);
            dd->is_shared_from_typeless_compatible_formats = false;
            
            mf->resize(tfd.num_mutable_formats);
            util::MemCopyArray(mf->data(), tfd.mutable_formats, tfd.num_mutable_formats);
        }
        tfd.mutable_formats = dd->mutable_formats->data();
    }
    else
    {
        tfd.num_mutable_formats = 0;
        tfd.mutable_formats = nullptr;
    }

    return BMRESULT_SUCCEED;
}

BMRESULT 
B3D_APIENTRY TextureD3D12::PrepareBindNodeMasks(uint32_t _heap_index, uint32_t _num_bind_node_masks, const NodeMask* _bind_node_masks)
{
    auto&& props = device->GetResourceHeapPropertiesForImpl()[_heap_index];
    bool is_multi_instance_heap = props.flags & RESOURCE_HEAP_PROPERTY_FLAG_MULTI_INSTANCE;
    bool is_invalod = (!is_multi_instance_heap && _num_bind_node_masks != 0);
         is_invalod |= (is_multi_instance_heap && _num_bind_node_masks != (uint32_t)device->GetD3D12Device()->GetNodeCount());
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

void 
B3D_APIENTRY TextureD3D12::CreateTiledResourceData()
{
    tiled_resource_data = B3DMakeUnique(TELED_RESOURCE_DATA);
    tiled_resource_data->Get(device12, texture);
}

BMRESULT
B3D_APIENTRY TextureD3D12::InitAsPlaced()
{
    D3D12_RESOURCE_DESC rd{};
    util::GetNativeResourceDesc(desc, &rd);

    // このリソースが作成可能か確認。(BufferD3D12.cppを参照)
    auto ai = device12->GetResourceAllocationInfo(0x1, 1, &rd);
    if (ai.SizeInBytes == -1)
    {
        if (util::IsEnabledDebug(this))
            device->CheckDXGIInfoQueue();
        return BMRESULT_FAILED;
    }

    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY TextureD3D12::InitAsReserved()
{
    D3D12_RESOURCE_DESC desc12{};
    util::GetNativeResourceDesc(desc, &desc12);

    D3D12_CLEAR_VALUE cv12{};
    D3D12_CLEAR_VALUE* cv12_ptr = nullptr;
    if (desc.texture.optimized_clear_value)
        cv12_ptr = util::GetNativeClearValue(desc.texture.format_desc.format, *desc.texture.optimized_clear_value, &cv12);

    auto hr = device12->CreateReservedResource(&desc12
                                               , util::GetFixedStateFromResourceFlags(desc.flags, false/*バッファのみで利用*/)
                                               , cv12_ptr
                                               , IID_PPV_ARGS(&texture));
    B3D_RET_IF_FAILED(HR_TRACE_IF_FAILED(hr));

    // NOTE: Reserved用ヒープは作成しません。

    MarkAsBound();

    return BMRESULT_SUCCEED;
}

BMRESULT 
B3D_APIENTRY TextureD3D12::InitAsCommitted(DeviceD3D12* _device, const COMMITTED_RESOURCE_DESC& _desc)
{
    Init(RESOURCE_CREATE_TYPE_COMMITTED, _device, _desc.resource_desc);

    // コミットされたリソースを作成
    D3D12_HEAP_PROPERTIES hp = device->GetHeapDescs12()[_desc.heap_index].Properties;
    hp.CreationNodeMask = _desc.creation_node_mask;
    hp.VisibleNodeMask = _desc.visible_node_mask;

    D3D12_RESOURCE_DESC desc12{};
    util::GetNativeResourceDesc(_desc.resource_desc, &desc12);

    D3D12_CLEAR_VALUE cv12{};
    D3D12_CLEAR_VALUE* cv12_ptr = nullptr;
    if (desc.texture.optimized_clear_value)
        cv12_ptr = util::GetNativeClearValue(desc.texture.format_desc.format, *desc.texture.optimized_clear_value, &cv12);

    auto hr = device12->CreateCommittedResource(&hp, util::GetNativeHeapFlags(_desc.heap_flags)
                                                , &desc12
                                                , util::GetFixedStateFromResourceFlags(desc.flags, false/*バッファのみで利用*/)
                                                , cv12_ptr
                                                , IID_PPV_ARGS(&texture));
    B3D_RET_IF_FAILED(HR_TRACE_IF_FAILED(hr));
    
    // コミットリソース用ヒープを作成。
    B3D_RET_IF_FAILED(ResourceHeapD3D12::CreateForCommitted(_device, _desc, this, &heap));
    
    MarkAsBound();

    return BMRESULT_SUCCEED;
}

BMRESULT 
B3D_APIENTRY TextureD3D12::InitForSwapChain(SwapChainD3D12* _swapchain, util::ComPtr<ID3D12Resource> _buffer)
{
    auto&& scd = _swapchain->GetDesc();
    RESOURCE_DESC desc_for_swap_chain{};
    desc_for_swap_chain.dimension                       = RESOURCE_DIMENSION_TEX2D;
    desc_for_swap_chain.flags                           = SwapchainFlagsToResourceFlags(scd);
    desc_for_swap_chain.texture.extent.width            = scd.buffer.width;
    desc_for_swap_chain.texture.extent.height           = scd.buffer.height;
    desc_for_swap_chain.texture.extent.depth            = 1;
    desc_for_swap_chain.texture.array_size              = 1;
    desc_for_swap_chain.texture.mip_levels              = 1;
    desc_for_swap_chain.texture.sample_count            = 1;
    desc_for_swap_chain.texture.format_desc             = scd.buffer.format_desc;
    desc_for_swap_chain.texture.layout                  = TEXTURE_LAYOUT_UNKNOWN;
    desc_for_swap_chain.texture.optimized_clear_value   = nullptr;
    desc_for_swap_chain.texture.flags                   = TEXTURE_CREATE_FLAG_NONE;
    desc_for_swap_chain.texture.usage                   = SwapchainFlagsToUsageFlags(scd.buffer.flags);
    B3D_RET_IF_FAILED(Init(RESOURCE_CREATE_TYPE_SWAP_CHAIN, _swapchain->GetDevice()->As<DeviceD3D12>(), desc_for_swap_chain));

    (texture = _buffer.Get())->AddRef();

    if (scd.num_present_queues > 1)
    {
        create_type = RESOURCE_CREATE_TYPE_SWAP_CHAIN_MULTI_NODES;
    }

    MarkAsBound();

    // NOTE: スワップチェイン用ヒープは作成しません。

    return BMRESULT_SUCCEED;
}

void 
B3D_APIENTRY TextureD3D12::MarkAsBound()
{
    is_bound = true;
    if (util::IsEnabledDebug(this) && name)
        util::SetName(texture, *name);
}

void
B3D_APIENTRY TextureD3D12::Uninit()
{
    name.reset();
    desc = {};
    desc_data.reset();
    is_bound = false;
    hlp::SafeRelease(texture);
    hlp::SafeRelease(heap);
    hlp::SafeRelease(device);
    device12 = nullptr;
    bind_node_masks.reset();
    tiled_resource_data.reset();
}

BMRESULT
B3D_APIENTRY TextureD3D12::Create(RESOURCE_CREATE_TYPE _create_type, DeviceD3D12* _device, const RESOURCE_DESC& _desc, TextureD3D12** _dst)
{
    util::Ptr<TextureD3D12> ptr;
    ptr.Attach(B3DCreateImplementationClass(TextureD3D12));
    B3D_RET_IF_FAILED(ptr->Init(_create_type, _device, _desc));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

BMRESULT 
B3D_APIENTRY TextureD3D12::CreateCommitted(DeviceD3D12* _device, const COMMITTED_RESOURCE_DESC& _desc, TextureD3D12** _dst)
{
    util::Ptr<TextureD3D12> ptr;
    ptr.Attach(B3DCreateImplementationClass(TextureD3D12));
    B3D_RET_IF_FAILED(ptr->InitAsCommitted(_device, _desc));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY TextureD3D12::CreateForSwapChain(SwapChainD3D12* _swapchain, util::ComPtr<ID3D12Resource> _buffer, TextureD3D12** _dst)
{
    util::Ptr<TextureD3D12> ptr;
    ptr.Attach(B3DCreateImplementationClass(TextureD3D12));
    B3D_RET_IF_FAILED(ptr->InitForSwapChain(_swapchain, _buffer));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY TextureD3D12::Bind(const BIND_RESOURCE_HEAP_INFO* _info)
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

    D3D12_CLEAR_VALUE cv12{};
    D3D12_CLEAR_VALUE* cv12_ptr = nullptr;
    if (desc.texture.optimized_clear_value)
        cv12_ptr = util::GetNativeClearValue(desc.texture.format_desc.format, *desc.texture.optimized_clear_value, &cv12);

    auto src_heap = _info->src_heap->As<ResourceHeapD3D12>();
    auto hr = device12->CreatePlacedResource(src_heap->GetD3D12Heap(), _info->src_heap_offset
                                             , &desc12
                                             , util::GetFixedStateFromResourceFlags(desc.flags, false/*バッファのみで利用*/)
                                             , cv12_ptr
                                             , IID_PPV_ARGS(&texture));
    B3D_RET_IF_FAILED(HR_TRACE_IF_FAILED(hr));

    if (create_type == RESOURCE_CREATE_TYPE_PLACED)
        (heap = src_heap)->AddRef(); // カウントは成功後に追加

    MarkAsBound();

    return BMRESULT_SUCCEED;
}

ID3D12Resource*
B3D_APIENTRY TextureD3D12::GetD3D12Resource() const
{
    return texture;
}

RESOURCE_CREATE_TYPE
B3D_APIENTRY TextureD3D12::GetCreateType() const
{
    return create_type;
}

BMRESULT 
B3D_APIENTRY TextureD3D12::SetupBindRegions(uint32_t _num_regions, const TILED_RESOURCE_BIND_REGION* _regions, D3D12_TILED_RESOURCE_COORDINATE* _dst_start_coords, D3D12_TILE_REGION_SIZE* _dst_region_sizes) const
{
    for (uint32_t i = 0; i < _num_regions; i++)
    {
        auto&& r  = _regions[i];
        auto&& sc = _dst_start_coords[i];
        auto&& rs = _dst_region_sizes[i];

        if (r.flags & TILED_RESOURCE_BIND_REGION_FLAG_MIPTAIL)
        {
            auto&& miptail = r.dst_miptail_region;
            //sc.X         = miptail.subresource.array_slice * tiled_resource_data->packed_mip_info.StartTileIndexInOverallResource;
            sc.X           = UINT(miptail.offset_in_bytes / D3D12_TILED_RESOURCE_TILE_SIZE_IN_BYTES);
            sc.Y           = 0;
            sc.Z           = 0;
            sc.Subresource = util::ConvertNativeSubresourceOffset(desc.texture.mip_levels, desc.texture.array_size, miptail.subresource);

            rs.UseBox = FALSE;
            rs.NumTiles = UINT(miptail.size_in_bytes / D3D12_TILED_RESOURCE_TILE_SIZE_IN_BYTES);
        }
        else
        {
            auto&& region = r.dst_region;
            sc.X           = region.tile_offset.x;
            sc.Y           = region.tile_offset.y;
            sc.Z           = region.tile_offset.z;
            sc.Subresource = util::ConvertNativeSubresourceOffset(desc.texture.mip_levels, desc.texture.array_size, region.subresource);

            rs.UseBox   = TRUE;
            rs.Width    = region.tile_size.width;
            rs.Height   = region.tile_size.height;
            rs.Depth    = region.tile_size.depth;
            rs.NumTiles = region.tile_size.width * region.tile_size.height * region.tile_size.depth;
        }
    }

    return BMRESULT_SUCCEED;
}

uint32_t 
B3D_APIENTRY TextureD3D12::GetTiledResourceAllocationInfo(TILED_RESOURCE_ALLOCATION_INFO* _dst_infos) const
{
    if (create_type != RESOURCE_CREATE_TYPE_RESERVED)
    {
        B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_STATE_GETTING
                          , "リソースはCreateReservedResourceから作成されている必要があります。");
        return 0;
    }

    if (_dst_infos)
    {
        auto&& shape = tiled_resource_data->tile_shape;
        auto&& fp = _dst_infos->format_properties;
        fp.aspect                      = util::IsDepthOnlyFormat(desc.texture.format_desc.format) ? TEXTURE_ASPECT_FLAG_DEPTH : TEXTURE_ASPECT_FLAG_COLOR;
        fp.flags                       = TILED_RESOURCE_FORMAT_FLAG_NONE;
        fp.tile_shape.width_in_texels  = shape.WidthInTexels;
        fp.tile_shape.height_in_texels = shape.HeightInTexels;
        fp.tile_shape.depth_in_texels  = shape.DepthInTexels;

        auto&& packed_mip = tiled_resource_data->packed_mip_info;
        auto&& miptail = _dst_infos->mip_tail;
        if (packed_mip.NumPackedMips != 0)
        {
            miptail.is_required     = true;
            miptail.first_mip_slice = packed_mip.NumStandardMips;
            miptail.size            = packed_mip.NumTilesForPackedMips           * D3D12_TILED_RESOURCE_TILE_SIZE_IN_BYTES;
            miptail.offset          = packed_mip.StartTileIndexInOverallResource * D3D12_TILED_RESOURCE_TILE_SIZE_IN_BYTES;
            miptail.stride          = packed_mip.StartTileIndexInOverallResource * D3D12_TILED_RESOURCE_TILE_SIZE_IN_BYTES;
        }
        else
        {
            miptail.is_required     = false;
            miptail.first_mip_slice = 0;
            miptail.size            = 0;
            miptail.offset          = 0;
            miptail.stride          = 0;
        }
    }

    return 1;
}

void
B3D_APIENTRY TextureD3D12::AddRef()
{
    ++ref_count;
    B3D_REFCOUNT_DEBUG(ref_count);
}

uint32_t
B3D_APIENTRY TextureD3D12::Release()
{
    B3D_REFCOUNT_DEBUG(ref_count - 1);
    auto count = --ref_count;
    if (count == 0)
        B3DDestroyImplementationClass(this);

    return count;
}

uint32_t
B3D_APIENTRY TextureD3D12::GetRefCount() const
{
    return ref_count;
}

const char*
B3D_APIENTRY TextureD3D12::GetName() const
{
    return name ? name->c_str() : nullptr;
}

BMRESULT
B3D_APIENTRY TextureD3D12::SetName(const char* _name)
{
    if (!util::IsEnabledDebug(this))
        return BMRESULT_FAILED;

    if (texture)
        B3D_RET_IF_FAILED(HR_TRACE_IF_FAILED(util::SetName(texture, _name)));

    if (name && !_name)
        name.reset();
    else
        name = B3DMakeUniqueArgs(util::NameableObjStr, _name);

    return BMRESULT_SUCCEED;
}

IDevice*
B3D_APIENTRY TextureD3D12::GetDevice() const
{
    return device;
}

const RESOURCE_DESC&
B3D_APIENTRY TextureD3D12::GetDesc() const
{
    return desc;
}

IResourceHeap*
B3D_APIENTRY TextureD3D12::GetHeap() const
{
    return heap;
}

void TextureD3D12::TELED_RESOURCE_DATA::Get(ID3D12Device* _device, ID3D12Resource* _resource)
{
    num_tiles = 0;
    packed_mip_info = {};
    tile_shape = {};

    auto t = _resource->GetDesc();
    num_subresource_tilings = UINT(t.MipLevels) * UINT(t.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE3D ? 1 : t.DepthOrArraySize);
    subresource_tilings.resize(num_subresource_tilings);
    _device->GetResourceTiling(_resource, &num_tiles, &packed_mip_info, &tile_shape, &num_subresource_tilings, 0, subresource_tilings.data());
}


}// namespace buma3d
