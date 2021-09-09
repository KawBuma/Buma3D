#include "Buma3DPCH.h"
#include "UnorderedAccessViewD3D12.h"

namespace buma3d
{

B3D_APIENTRY UnorderedAccessViewD3D12::UnorderedAccessViewD3D12()
    : ref_count         { 1 }
    , name              {}
    , device            {}
    , desc              {}
    , resource          {}
    , counter_buffer    {}
    , device12          {}
    , descriptor        {}
    , virtual_address   {}
    , buffer_view       {}
    , texture_view      {}
{

}

B3D_APIENTRY UnorderedAccessViewD3D12::~UnorderedAccessViewD3D12()
{
    Uninit();
}

BMRESULT
B3D_APIENTRY UnorderedAccessViewD3D12::Init(DeviceD3D12* _device, IResource* _resource, IBuffer* _resource_for_counter_buffer, const UNORDERED_ACCESS_VIEW_DESC& _desc)
{
    if (_resource_for_counter_buffer)
    {
        /* NOTE: 描画/ディスパッチの実行中は、カウンター リソースの状態がD3D12_RESOURCE_STATE_UNORDERED_ACCESS でなければなりません。
        また、1 つの描画/ディスパッチ呼び出しで、アプリケーションが同じ 32 ビット メモリの場所に 2 つの UAV カウンターからアクセスすることは無効です。
        このいずれかが検出されると、デバッグ レイヤーがエラーを発行します。*/

        // TODO: UnorderedAccessViewVk: カウンターバッファーへの対応
        B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_B3D_DETAILS, "TODO: UnorderedAccessViewD3D12: カウンターバッファーへの対応。");
        return BMRESULT_FAILED_NOT_IMPLEMENTED;
    }

    (device = _device)->AddRef();
    device12 = device->GetD3D12Device();
    device12 = device->GetD3D12Device();

    (resource = _resource)->AddRef();
    if (_resource_for_counter_buffer)
        (counter_buffer = _resource_for_counter_buffer)->AddRef();
    CopyDesc(_desc);

    // TODO: D3D12とVulkanのディスクリプタ使用の際の、ノードマスクのための互換性を検証する。
    B3D_ADD_DEBUG_MSG_INFO_B3D("TODO: D3D12とVulkanのディスクリプタ使用の際の、ノードマスクのための互換性を検証する。");

    if (desc.view.type != VIEW_TYPE_UNORDERED_ACCESS)
    {
        B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_INITIALIZATION, "UNORDERED_ACCESS_VIEW_DESC::view.typeはVIEW_TYPE_UNORDERED_ACCESSである必要があります。");
        return BMRESULT_FAILED_INVALID_PARAMETER;
    }

    BMRESULT result = BMRESULT_SUCCEED;
    switch (desc.view.dimension)
    {
    case VIEW_DIMENSION_BUFFER_TYPED:
    case VIEW_DIMENSION_BUFFER_STRUCTURED:
    case VIEW_DIMENSION_BUFFER_BYTEADDRESS:
        result = InitAsBufferUAV();
        buffer_view = &desc.buffer;
        break;

    case VIEW_DIMENSION_TEXTURE_1D:
    case VIEW_DIMENSION_TEXTURE_1D_ARRAY:
    case VIEW_DIMENSION_TEXTURE_2D:
    case VIEW_DIMENSION_TEXTURE_2D_ARRAY:
    case VIEW_DIMENSION_TEXTURE_3D:
        result = InitAsTextureUAV();
        texture_view = &desc.texture;
        break;

    default:
        B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_INITIALIZATION
                          , "UAVでは、view.dimensionはBUFFER_TYPED, BUFFER_STRUCTURED, BUFFER_BYTEADDRESS, TEXTURE_1D, TEXTURE_1D_ARRAY, TEXTURE_2D, TEXTURE_2D_ARRAY, またはTEXTURE_3Dである必要があります。");
        result = BMRESULT_FAILED_INVALID_PARAMETER;
        break;
    }

    return result;
}

BMRESULT
B3D_APIENTRY UnorderedAccessViewD3D12::InitAsBufferUAV()
{
    if (resource->GetDesc().dimension != RESOURCE_DIMENSION_BUFFER)
    {
        B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_INITIALIZATION
                          , "VIEW_TYPE_BUFFER_*の場合、リソースのdimensionはRESOURCE_DIMENSION_BUFFERである必要があります。");
        return BMRESULT_FAILED_INVALID_PARAMETER;
    }
    auto buffer = resource->As<BufferD3D12>();

    /*
    StructureByteStrideの0でない場合、構造化バッファのビューが作成され、D3D12_UNORDERED_ACCESS_VIEW_DESC::FormatフィールドはDXGI_FORMAT_UNKNOWNである必要があります。
    StructureByteStrideが0の場合、バッファの型付き(typed)ビューが作成され、フォーマットを指定する必要があります。
    型付き(Typed)ビューに指定された形式は、ハードウェアでサポートされている必要があります。
    リソースには、生の非構造化データが含まれています。 UAV形式はDXGI_FORMAT_R32_TYPELESSである必要があります。
    */

    // TODO: 圧縮フォーマットのバッファの対応を確認。
    B3D_ADD_DEBUG_MSG_INFO_B3D("TODO: 圧縮フォーマットのバッファの対応を確認。");

    BMRESULT result = BMRESULT_SUCCEED;

    D3D12_UNORDERED_ACCESS_VIEW_DESC uavdesc{};
    uavdesc.Format = util::GetNativeFormat(desc.view.format);
    uavdesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;

    auto&& bdesc = desc.buffer;
    auto&& srv_buf = uavdesc.Buffer;
    switch (desc.view.dimension)
    {
    case VIEW_DIMENSION_BUFFER_TYPED:
    {
        if (desc.view.format == RESOURCE_FORMAT_UNKNOWN)
        {
            result = BMRESULT_FAILED_INVALID_PARAMETER;
            break;
        }
        auto format_size = util::GetFormatSize(desc.view.format);
        srv_buf.FirstElement        = bdesc.first_element;
        srv_buf.NumElements         = (UINT)bdesc.num_elements;
        srv_buf.StructureByteStride = 0;
        virtual_address             = buffer->GetGPUVirtualAddress() + (format_size * bdesc.first_element * bdesc.num_elements);
        break;
    }

    case VIEW_DIMENSION_BUFFER_STRUCTURED:
    {
        if (desc.view.format != RESOURCE_FORMAT_UNKNOWN)
        {
            result = BMRESULT_FAILED_INVALID_PARAMETER;
            break;
        }
        srv_buf.FirstElement        = bdesc.first_element;
        srv_buf.NumElements         = (UINT)bdesc.num_elements;
        srv_buf.StructureByteStride = bdesc.structure_byte_stride;
        virtual_address = buffer->GetGPUVirtualAddress() + (bdesc.structure_byte_stride * bdesc.first_element * bdesc.num_elements);
        break;
    }

    case VIEW_DIMENSION_BUFFER_BYTEADDRESS:
    {
        if (desc.view.format != RESOURCE_FORMAT_R32_TYPELESS)
        {
            result = BMRESULT_FAILED_INVALID_PARAMETER;
            break;
        }
        srv_buf.FirstElement        = bdesc.first_element;
        srv_buf.NumElements         = (UINT)bdesc.num_elements;
        srv_buf.StructureByteStride = 0;
        srv_buf.Flags               = D3D12_BUFFER_UAV_FLAG_RAW;
        virtual_address = buffer->GetGPUVirtualAddress() + (util::GetFormatSize(RESOURCE_FORMAT_R32_TYPELESS) * bdesc.first_element * bdesc.num_elements);
        break;
    }

    default:
        result = BMRESULT_FAILED_INVALID_PARAMETER;
        break;
    }

    if (hlp::IsFailed(result))
        return result;

    descriptor = device->GetCPUDescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, B3D_DEFAULT_NODE_MASK).Allocate();
    device12->CreateUnorderedAccessView(buffer->GetD3D12Resource()
                                        , counter_buffer ? counter_buffer->As<BufferD3D12>()->GetD3D12Resource() : nullptr
                                        , &uavdesc
                                        , descriptor.handle);

    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY UnorderedAccessViewD3D12::ValidateTextureUAV()
{
    if (resource->GetDesc().dimension == RESOURCE_DIMENSION_BUFFER)
    {
        B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_INITIALIZATION
                          , "VIEW_TYPE_TEXTURE_*の場合、リソースのdimensionはRESOURCE_DIMENSION_TEX*である必要があります。");
        return BMRESULT_FAILED_INVALID_PARAMETER;
    }

    // TODO: エラー処理(実行結果を参照しながら埋めていく)
    auto&& t = resource->As<TextureD3D12>()->GetDesc().texture;

    auto&& tdesc = desc.texture;
    auto&& range = tdesc.subresource_range;

    if (range.offset.mip_slice >= t.mip_levels)
    {
        B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_INITIALIZATION
                          , "UNORDERED_ACCESS_VIEW_DESC::...mip_sliceが大きすぎます。mip_sliceはリソースのミップ数より小さい必要があります。");
        return BMRESULT_FAILED_INVALID_PARAMETER;
    }
    if (range.offset.array_slice >= t.array_size)
    {
        B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_INITIALIZATION
                          , "UNORDERED_ACCESS_VIEW_DESC::...array_sliceが大きすぎます。array_sliceはリソースの深さ/配列要素数より小さい必要があります。");
        return BMRESULT_FAILED_INVALID_PARAMETER;
    }

    // B3D_USE_REMAINING_ARRAY_SIZES
    {
        if (range.array_size == B3D_USE_REMAINING_ARRAY_SIZES)
            range.array_size = t.array_size - range.offset.array_slice;
    }

    if (t.sample_count != 1)
    {
        B3D_ADD_DEBUG_MSG2(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_INITIALIZATION
                           , "テクスチャのsample_countが", t.sample_count, "です。 UAVでは、マルチサンプルテクスチャを使用できません。");
        return BMRESULT_FAILED_INVALID_PARAMETER;
    }
    if (range.mip_levels != 1)
    {
        B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_INITIALIZATION
                          , "UAVでは、mip_levelsは1である必要があります。");
        return BMRESULT_FAILED_INVALID_PARAMETER;
    }
    if (range.offset.mip_slice   + range.mip_levels > t.mip_levels || 
        range.offset.array_slice + range.array_size > t.array_size)
    {
        B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_INITIALIZATION
                          , "ミップ、または配列の範囲指定が不正です。");
        return BMRESULT_FAILED_INVALID_PARAMETER;
    }

    switch (desc.view.dimension)
    {
    case buma3d::VIEW_DIMENSION_TEXTURE_1D:
    {
        if (range.array_size != 1)
            return BMRESULT_FAILED_INVALID_PARAMETER;
        break;
    }

    case buma3d::VIEW_DIMENSION_TEXTURE_1D_ARRAY:
    {
        break;
    }

    case buma3d::VIEW_DIMENSION_TEXTURE_2D:
    {
        if (range.array_size != 1)
            return BMRESULT_FAILED_INVALID_PARAMETER;
        break;
    }

    case buma3d::VIEW_DIMENSION_TEXTURE_2D_ARRAY:
    {
        break;
    }

    case buma3d::VIEW_DIMENSION_TEXTURE_3D:
    {
        break;
    }

    default:
        break;
    }

    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY UnorderedAccessViewD3D12::InitAsTextureUAV()
{
    B3D_RET_IF_FAILED(ValidateTextureUAV());

    auto texture = resource->As<TextureD3D12>();
    auto&& res_desc = texture->GetDesc();
    auto&& t = res_desc.texture;

    auto&& tdesc = desc.texture;
    auto&& range = tdesc.subresource_range;

    D3D12_UNORDERED_ACCESS_VIEW_DESC uavdesc{};

    if (util::IsDepthStencilFormat(desc.view.format))
        uavdesc.Format = util::ConvertDepthStencilFormat(desc.view.format, tdesc.subresource_range.offset.aspect);
    else
        uavdesc.Format = util::GetNativeFormat(desc.view.format);

    auto SetMipParams = [&](auto* _src_dimension) {
        _src_dimension->MipSlice = range.offset.mip_slice;
    };
    auto SetArrayParams = [&](auto* _src_dimension) {
        _src_dimension->FirstArraySlice = range.offset.array_slice;
        _src_dimension->ArraySize       = range.array_size;
    };
    auto SetPlaneParams = [&](auto* _src_dimension) {
        _src_dimension->PlaneSlice = util::GetNativeAspectFlags(range.offset.aspect);
    };
    auto Set3DParams = [&](auto* _src_dimension) {
        _src_dimension->FirstWSlice = range.offset.array_slice;
        _src_dimension->WSize       = range.array_size;
    };

    auto As1D = [&](auto* _desc) {
        uavdesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1D;
        SetMipParams(_desc);
    };
    auto As1DArray = [&](auto* _desc) {
        uavdesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1DARRAY;
        SetMipParams(_desc); SetArrayParams(_desc);
    };
    auto As2D = [&](auto* _desc) {
        uavdesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
        SetMipParams(_desc); SetPlaneParams(_desc);
    };
    auto As2DArray = [&](auto* _desc) {
        uavdesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
        SetMipParams(_desc); SetArrayParams(_desc); SetPlaneParams(_desc);
    };
    auto As3D = [&](auto* _desc) {
        uavdesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
        SetMipParams(_desc);
        Set3DParams(_desc);
    };

    switch (desc.view.dimension)
    {
    case VIEW_DIMENSION_TEXTURE_1D        : As1D      (&uavdesc.Texture1D);      break;
    case VIEW_DIMENSION_TEXTURE_1D_ARRAY  : As1DArray (&uavdesc.Texture1DArray); break;
    case VIEW_DIMENSION_TEXTURE_2D        : As2D      (&uavdesc.Texture2D);      break;
    case VIEW_DIMENSION_TEXTURE_2D_ARRAY  : As2DArray (&uavdesc.Texture2DArray); break;
    case VIEW_DIMENSION_TEXTURE_3D        : As3D      (&uavdesc.Texture3D);      break;

    default:
        B3D_ASSERT(false);
        break;
    }

    descriptor = device->GetCPUDescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, B3D_DEFAULT_NODE_MASK).Allocate();
    device12->CreateUnorderedAccessView(texture->GetD3D12Resource()
                                        , counter_buffer ? counter_buffer->As<BufferD3D12>()->GetD3D12Resource() : nullptr
                                        , &uavdesc
                                        , descriptor.handle);

    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY UnorderedAccessViewD3D12::CopyDesc(const UNORDERED_ACCESS_VIEW_DESC& _desc)
{
    desc = _desc;
}

void
B3D_APIENTRY UnorderedAccessViewD3D12::Uninit()
{
    if (descriptor.handle)
        device->GetCPUDescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, B3D_DEFAULT_NODE_MASK).Free(descriptor);
    descriptor = {};
    virtual_address = {};

    hlp::SafeRelease(resource);
    hlp::SafeRelease(counter_buffer);
    hlp::SafeRelease(device);

    name.reset();
    desc = {};
}


BMRESULT
B3D_APIENTRY UnorderedAccessViewD3D12::Create(DeviceD3D12* _device, IResource* _resource, IBuffer* _resource_for_counter_buffer, const UNORDERED_ACCESS_VIEW_DESC& _desc, UnorderedAccessViewD3D12** _dst)
{
    util::Ptr<UnorderedAccessViewD3D12> ptr;
    ptr.Attach(B3DCreateImplementationClass(UnorderedAccessViewD3D12));
    B3D_RET_IF_FAILED(ptr->Init(_device, _resource, _resource_for_counter_buffer, _desc));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY UnorderedAccessViewD3D12::AddRef()
{
    ++ref_count;
    B3D_REFCOUNT_DEBUG(ref_count);
}

uint32_t
B3D_APIENTRY UnorderedAccessViewD3D12::Release()
{
    B3D_REFCOUNT_DEBUG(ref_count - 1);
    auto count = --ref_count;
    if (count == 0)
        B3DDestroyImplementationClass(this);

    return count;
}

uint32_t
B3D_APIENTRY UnorderedAccessViewD3D12::GetRefCount() const
{
    return ref_count;
}

const char*
B3D_APIENTRY UnorderedAccessViewD3D12::GetName() const
{
    return name ? name->c_str() : nullptr;
}

BMRESULT
B3D_APIENTRY UnorderedAccessViewD3D12::SetName(const char* _name)
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
B3D_APIENTRY UnorderedAccessViewD3D12::GetDevice() const
{
    return device;
}

const CPU_DESCRIPTOR_ALLOCATION*
B3D_APIENTRY UnorderedAccessViewD3D12::GetCpuDescriptorAllocation() const
{
    return &descriptor;
}

const BUFFER_VIEW*
B3D_APIENTRY UnorderedAccessViewD3D12::GetBufferView() const
{
    return buffer_view;
}

const TEXTURE_VIEW*
B3D_APIENTRY UnorderedAccessViewD3D12::GetTextureView() const
{
    return texture_view;
}

const D3D12_GPU_VIRTUAL_ADDRESS*
B3D_APIENTRY UnorderedAccessViewD3D12::GetGpuVirtualAddress() const
{
    return buffer_view ? &virtual_address : nullptr;
}

const VIEW_DESC&
B3D_APIENTRY UnorderedAccessViewD3D12::GetViewDesc() const
{
    return desc.view;
}

IResource*
B3D_APIENTRY UnorderedAccessViewD3D12::GetResource() const
{
    return resource;
}

const UNORDERED_ACCESS_VIEW_DESC&
B3D_APIENTRY UnorderedAccessViewD3D12::GetDesc() const
{
    return desc;
}

IBuffer*
B3D_APIENTRY UnorderedAccessViewD3D12::GetCounterBuffer() const
{
    return counter_buffer;
}

const CPU_DESCRIPTOR_ALLOCATION&
B3D_APIENTRY UnorderedAccessViewD3D12::GetCPUDescriptorAllocation() const
{
    return descriptor;
}


}// namespace buma3d
