#include "Buma3DPCH.h"
#include "RenderTargetViewD3D12.h"

namespace buma3d
{

B3D_APIENTRY RenderTargetViewD3D12::RenderTargetViewD3D12()
    : ref_count     { 1 }
    , name          {}
    , device        {}
    , desc          {}
    , resource      {}
    , device12      {}
    , descriptor    {}
{

}

B3D_APIENTRY RenderTargetViewD3D12::~RenderTargetViewD3D12()
{
    Uninit();
}

BMRESULT
B3D_APIENTRY RenderTargetViewD3D12::Init(DeviceD3D12* _device, IResource* _resource, const RENDER_TARGET_VIEW_DESC& _desc)
{
    (device = _device)->AddRef();
    device12 = device->GetD3D12Device();

    (resource = _resource)->AddRef();
    CopyDesc(_desc);

    // TODO: D3D12とVulkanのディスクリプタ使用の際の、ノードマスクのための互換性を検証する。
    B3D_ADD_DEBUG_MSG_INFO_B3D("TODO: D3D12とVulkanのディスクリプタ使用の際の、ノードマスクのための互換性を検証する。");

    if (desc.view.type != VIEW_TYPE_RENDER_TARGET)
    {
        B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_INITIALIZATION, "RENDER_TARGET_VIEW_DESC::view.typeはVIEW_TYPE_RENDER_TARGETである必要があります。");
        return BMRESULT_FAILED_INVALID_PARAMETER;
    }

    BMRESULT result = BMRESULT_SUCCEED;
    switch (desc.view.dimension)
    {
    case VIEW_DIMENSION_TEXTURE_1D:
    case VIEW_DIMENSION_TEXTURE_1D_ARRAY:
    case VIEW_DIMENSION_TEXTURE_2D:
    case VIEW_DIMENSION_TEXTURE_2D_ARRAY:
        result = InitAsTextureRTV();
        break;

    case VIEW_DIMENSION_TEXTURE_3D:
    case VIEW_DIMENSION_TEXTURE_CUBE:
    default:
        result = BMRESULT_FAILED_INVALID_PARAMETER;
        break;
    }

    return result;
}

BMRESULT
B3D_APIENTRY RenderTargetViewD3D12::ValidateTextureRTV()
{
    if (resource->GetDesc().dimension == RESOURCE_DIMENSION_BUFFER)
    {
        B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_INITIALIZATION
                          , "VIEW_TYPE_TEXTURE_*の場合、リソースのdimensionはRESOURCE_DIMENSION_TEX*である必要があります。");
        return BMRESULT_FAILED_INVALID_PARAMETER;
    }
    // TODO: エラー処理(実行結果を参照しながら埋めていく)
    auto&& t = resource->As<TextureD3D12>()->GetDesc().texture;

    // FIXME: エラー処理は、B3D側で抽象化している両APIの使用法のみにすべき?(つまり、両API共通の検証をB3Dで常に行うべきかどうか)

    auto&& tdesc = desc.texture;
    auto&& range = tdesc.subresource_range;
    
    if (util::IsDepthStencilFormat(desc.view.format))
    {  
        if ( (range.offset.aspect & ~(TEXTURE_ASPECT_FLAG_DEPTH | TEXTURE_ASPECT_FLAG_STENCIL)) ||
            !(range.offset.aspect &  (TEXTURE_ASPECT_FLAG_DEPTH | TEXTURE_ASPECT_FLAG_STENCIL)) ||
             (range.offset.aspect == (TEXTURE_ASPECT_FLAG_DEPTH | TEXTURE_ASPECT_FLAG_STENCIL)))
        {
            B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_INITIALIZATION
                              , "RENDER_TARGET_VIEW_DESC::...formatが深度ステンシルフォーマットの場合、"
                                "RENDER_TARGET_VIEW_DESC::...aspectはTEXTURE_ASPECT_FLAG_DEPTHまたはTEXTURE_ASPECT_FLAG_STENCILのいずれかのビットである必要があり、両方を含んではなりません。");
            return BMRESULT_FAILED_INVALID_PARAMETER;
        }
    }

    if (!util::IsIdentifyComponentMapping(tdesc.components))
    {
        B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_INITIALIZATION
                          , "RENDER_TARGET_VIEW_DESC::...componentsはIDENTIFYまたはそれぞれの変数と同じスウィズルである必要があります。");
        return BMRESULT_FAILED_INVALID_PARAMETER;
    }

    if (range.offset.mip_slice >= t.mip_levels)
    {
        B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_INITIALIZATION
                          , "RENDER_TARGET_VIEW_DESC::...mip_sliceが大きすぎます。mip_sliceはリソースのミップ数より小さい必要があります。");
        return BMRESULT_FAILED_INVALID_PARAMETER;
    }
    if (range.offset.array_slice >= t.array_size)
    {
        B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_INITIALIZATION
                          , "RENDER_TARGET_VIEW_DESC::...array_sliceが大きすぎます。array_sliceはリソースの深さ/配列要素数より小さい必要があります。");
        return BMRESULT_FAILED_INVALID_PARAMETER;
    }

    // B3D_USE_REMAINING_MIP_LEVELS, B3D_USE_REMAINING_ARRAY_SIZES
    {
        if (range.mip_levels == B3D_USE_REMAINING_MIP_LEVELS)
            range.mip_levels = t.mip_levels - range.offset.mip_slice;

        if (range.array_size == B3D_USE_REMAINING_ARRAY_SIZES)
            range.array_size = t.array_size - range.offset.array_slice;
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
        if (t.sample_count != 1)
            return BMRESULT_FAILED_INVALID_PARAMETER;
        break;
    }

    case buma3d::VIEW_DIMENSION_TEXTURE_1D_ARRAY:
    {
        if (t.sample_count != 1)
            return BMRESULT_FAILED_INVALID_PARAMETER;
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
        if (t.sample_count != 1)
            return BMRESULT_FAILED_INVALID_PARAMETER;
        break;
    }

    case buma3d::VIEW_DIMENSION_TEXTURE_CUBE:
    {
        if (t.sample_count != 1)
            return BMRESULT_FAILED_INVALID_PARAMETER;
        if (range.array_size != 6)
        {
            B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_INITIALIZATION
                              , "dimensionがVIEW_DIMENSION_TEXTURE_CUBEの場合、array_sizeの値は6である必要があります。");
            return BMRESULT_FAILED_INVALID_PARAMETER;
        }
        break;
    }

    case buma3d::VIEW_DIMENSION_TEXTURE_CUBE_ARRAY:
    {
        if (t.sample_count != 1)
            return BMRESULT_FAILED_INVALID_PARAMETER;
        if (range.array_size % 6 != 0)
        {
            B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_INITIALIZATION
                              , "dimensionがVIEW_DIMENSION_TEXTURE_CUBE_ARRAYの場合、array_sizeの値は6の倍数である必要があります。");
            return BMRESULT_FAILED_INVALID_PARAMETER;
        }
        break;
    }
    default:
        break;
    }

    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY RenderTargetViewD3D12::InitAsTextureRTV()
{
    B3D_RET_IF_FAILED(ValidateTextureRTV());

    auto texture = resource->As<TextureD3D12>();
    auto&& res_desc = texture->GetDesc();
    auto&& t = res_desc.texture;

    auto&& tdesc = desc.texture;
    auto&& range = tdesc.subresource_range;

    D3D12_RENDER_TARGET_VIEW_DESC rtvdesc{};

    if (util::IsDepthStencilFormat(desc.view.format))
        rtvdesc.Format = util::ConvertDepthStencilFormat(desc.view.format, tdesc.subresource_range.offset.aspect);
    else
        rtvdesc.Format = util::GetNativeFormat(desc.view.format);

    auto SetMipParams = [&](auto* _src_dimension)
    {
        _src_dimension->MipSlice = range.offset.mip_slice;
    };
    auto SetArrayParams = [&](auto* _src_dimension)
    {
        _src_dimension->FirstArraySlice = range.offset.array_slice;
        _src_dimension->ArraySize       = range.array_size;
    };
    auto Set3DParams = [&](auto* _src_dimension)
    {  
        _src_dimension->FirstWSlice = range.offset.array_slice;
        _src_dimension->WSize       = range.array_size;
    };
    auto SetPlaneParams = [&](auto* _src_dimension)
    {
        _src_dimension->PlaneSlice = util::GetNativeAspectFlags(range.offset.aspect);
    };

    auto As1D = [&](auto* _desc)
    {
        rtvdesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1D;
        SetMipParams(_desc);
    };
    auto As1DArray = [&](auto* _desc)
    {
        rtvdesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1DARRAY;
        SetMipParams(_desc); SetArrayParams(_desc);
    };
    auto As2D = [&](auto* _desc)
    {
        rtvdesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
        SetMipParams(_desc); SetPlaneParams(_desc);
    };
    auto As2DArray = [&](auto* _desc)
    {
        rtvdesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
        SetMipParams(_desc); SetArrayParams(_desc); SetPlaneParams(_desc);
    };
    auto As2DMS = [&](auto* _desc)
    {
        rtvdesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMS;
        B3D_UNREFERENCED(_desc->UnusedField_NothingToDefine);
    };
    auto As2DMSArray = [&](auto* _desc)
    {
        rtvdesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMSARRAY;
        SetArrayParams(_desc);
    };
    auto As3D = [&](auto* _desc)
    {
        rtvdesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE3D;
        SetMipParams(_desc);
        Set3DParams(_desc);
    };

    switch (desc.view.dimension)
    {
    case VIEW_DIMENSION_TEXTURE_1D       : As1D     (&rtvdesc.Texture1D);                                                 break;
    case VIEW_DIMENSION_TEXTURE_1D_ARRAY : As1DArray(&rtvdesc.Texture1DArray);                                            break;
    case VIEW_DIMENSION_TEXTURE_2D       : t.sample_count == 1 ? As2D(&rtvdesc.Texture2D) : As2DMS(&rtvdesc.Texture2DMS); break;

    case VIEW_DIMENSION_TEXTURE_2D_ARRAY: 
        if (res_desc.dimension == RESOURCE_DIMENSION_TEX3D)
            As3D(&rtvdesc.Texture3D);
        else
            t.sample_count == 1 ? As2DArray(&rtvdesc.Texture2DArray) : As2DMSArray(&rtvdesc.Texture2DMSArray);
        break;

    default:
        B3D_ASSERT(false);
        break;
    }

    descriptor = device->GetCPUDescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, B3D_DEFAULT_NODE_MASK).Allocate();
    device12->CreateRenderTargetView(texture->GetD3D12Resource(), &rtvdesc, descriptor.handle);

    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY RenderTargetViewD3D12::CopyDesc(const RENDER_TARGET_VIEW_DESC& _desc)
{
    desc = _desc;
}

void
B3D_APIENTRY RenderTargetViewD3D12::Uninit()
{
    if (descriptor.handle)
        device->GetCPUDescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, B3D_DEFAULT_NODE_MASK).Free(descriptor);
    descriptor = {};

    hlp::SafeRelease(resource);
    hlp::SafeRelease(device);

    name.reset();
    desc = {};
}


BMRESULT
B3D_APIENTRY RenderTargetViewD3D12::Create(DeviceD3D12* _device, IResource* _resource, const RENDER_TARGET_VIEW_DESC& _desc, RenderTargetViewD3D12** _dst)
{
    util::Ptr<RenderTargetViewD3D12> ptr;
    ptr.Attach(B3DCreateImplementationClass(RenderTargetViewD3D12));
    B3D_RET_IF_FAILED(ptr->Init(_device, _resource, _desc));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY RenderTargetViewD3D12::AddRef()
{
    ++ref_count;
    B3D_REFCOUNT_DEBUG(ref_count);
}

uint32_t
B3D_APIENTRY RenderTargetViewD3D12::Release()
{
    B3D_REFCOUNT_DEBUG(ref_count - 1);
    auto count = --ref_count;
    if (count == 0)
        B3DDestroyImplementationClass(this);

    return count;
}

uint32_t
B3D_APIENTRY RenderTargetViewD3D12::GetRefCount() const
{
    return ref_count;
}

const char*
B3D_APIENTRY RenderTargetViewD3D12::GetName() const
{
    return name ? name->c_str() : nullptr;
}

BMRESULT
B3D_APIENTRY RenderTargetViewD3D12::SetName(const char* _name)
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
B3D_APIENTRY RenderTargetViewD3D12::GetDevice() const
{
    return device;
}

const CPU_DESCRIPTOR_ALLOCATION*
B3D_APIENTRY RenderTargetViewD3D12::GetCpuDescriptorAllocation() const
{
    return &descriptor;
}

const BUFFER_VIEW*
B3D_APIENTRY RenderTargetViewD3D12::GetBufferView() const
{
    return nullptr;
}

const TEXTURE_VIEW*
B3D_APIENTRY RenderTargetViewD3D12::GetTextureView() const
{
    return &desc.texture;
}

const D3D12_GPU_VIRTUAL_ADDRESS*
B3D_APIENTRY RenderTargetViewD3D12::GetGpuVirtualAddress() const
{
    return nullptr;
}

const VIEW_DESC&
B3D_APIENTRY RenderTargetViewD3D12::GetViewDesc() const
{
    return desc.view;
}

IResource*
B3D_APIENTRY RenderTargetViewD3D12::GetResource() const
{
    return resource;
}

const RENDER_TARGET_VIEW_DESC&
B3D_APIENTRY RenderTargetViewD3D12::GetDesc() const
{
    return desc;
}


}// namespace buma3d

