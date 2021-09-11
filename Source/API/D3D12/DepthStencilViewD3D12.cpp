#include "Buma3DPCH.h"
#include "DepthStencilViewD3D12.h"

namespace buma3d
{

B3D_APIENTRY DepthStencilViewD3D12::DepthStencilViewD3D12()
    : ref_count             { 1 }
    , name                  {}
    , device                {}
    , desc                  {}
    , resource              {}
    , device12              {}
    , descriptor            {}
    , has_all_subresources  {}
{

}

B3D_APIENTRY DepthStencilViewD3D12::~DepthStencilViewD3D12()
{
    Uninit();
}

BMRESULT
B3D_APIENTRY DepthStencilViewD3D12::Init(DeviceD3D12* _device, IResource* _resource, const DEPTH_STENCIL_VIEW_DESC& _desc)
{
    (device = _device)->AddRef();
    device12 = device->GetD3D12Device();

    (resource = _resource)->AddRef();
    CopyDesc(_desc);

    // TODO: D3D12とVulkanのディスクリプタ使用の際の、ノードマスクのための互換性を検証する。
    B3D_ADD_DEBUG_MSG_INFO_B3D("TODO: D3D12とVulkanのディスクリプタ使用の際の、ノードマスクのための互換性を検証する。");

    if (desc.view.type != VIEW_TYPE_DEPTH_STENCIL)
    {
        B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_INITIALIZATION, "DEPTH_STENCIL_VIEW_DESC::view.typeはVIEW_TYPE_DEPTH_STENCILである必要があります。");
        return BMRESULT_FAILED_INVALID_PARAMETER;
    }

    switch (desc.view.dimension)
    {
    case VIEW_DIMENSION_TEXTURE_1D:
    case VIEW_DIMENSION_TEXTURE_1D_ARRAY:
    case VIEW_DIMENSION_TEXTURE_2D:
    case VIEW_DIMENSION_TEXTURE_2D_ARRAY:
        B3D_RET_IF_FAILED(InitAsTextureDSV());
        has_all_subresources = resource->As<TextureD3D12>()->GetTextureProperties().IsAllSubresources(desc.texture.subresource_range);
        break;

    case VIEW_DIMENSION_TEXTURE_3D:
    case VIEW_DIMENSION_TEXTURE_CUBE:
    default:
        return BMRESULT_FAILED_INVALID_PARAMETER;
    }

    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY DepthStencilViewD3D12::ValidateTextureDSV()
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
        if ((range.offset.aspect  & ~(TEXTURE_ASPECT_FLAG_DEPTH | TEXTURE_ASPECT_FLAG_STENCIL)) ||
            !(range.offset.aspect & (TEXTURE_ASPECT_FLAG_DEPTH | TEXTURE_ASPECT_FLAG_STENCIL)))
        {
            B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_INITIALIZATION
                              , "DEPTH_STENCIL_VIEW_DESC::...aspectはTEXTURE_ASPECT_FLAG_DEPTHまたはTEXTURE_ASPECT_FLAG_STENCILの有効な組み合わせである必要があります。");
            return BMRESULT_FAILED_INVALID_PARAMETER;
        }
    }
    else
    {
        B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_INITIALIZATION
                          , "DEPTH_STENCIL_VIEW_DESC::...formatは深度ステンシルフォーマットである必要があります。");
        return BMRESULT_FAILED_INVALID_PARAMETER;
    }

    if (!util::IsIdentifyComponentMapping(tdesc.components))
    {
        B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_INITIALIZATION
                          , "DEPTH_STENCIL_VIEW_DESC::...componentsはIDENTIFYまたはそれぞれの変数名と同じスウィズルである必要があります。");
        return BMRESULT_FAILED_INVALID_PARAMETER;
    }

    if (range.offset.mip_slice >= t.mip_levels)
    {
        B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_INITIALIZATION
                          , "DEPTH_STENCIL_VIEW_DESC::...mip_sliceが大きすぎます。mip_sliceはリソースのミップ数より小さい必要があります。");
        return BMRESULT_FAILED_INVALID_PARAMETER;
    }
    if (range.offset.array_slice >= t.array_size)
    {
        B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_INITIALIZATION
                          , "DEPTH_STENCIL_VIEW_DESC::...array_sliceが大きすぎます。array_sliceはリソースの深さ/配列要素数より小さい必要があります。");
        return BMRESULT_FAILED_INVALID_PARAMETER;
    }

    if (range.mip_levels != 1)
    {
        B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_INITIALIZATION
                          , "DEPTH_STENCIL_VIEW_DESC::...mip_levelsは1である必要があります。");
        return BMRESULT_FAILED_INVALID_PARAMETER;
    }

    // B3D_USE_REMAINING_ARRAY_SIZES
    {
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

    default:
        break;
    }

    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY DepthStencilViewD3D12::InitAsTextureDSV()
{
    B3D_RET_IF_FAILED(ValidateTextureDSV());

    auto texture = resource->As<TextureD3D12>();
    auto&& res_desc = texture->GetDesc();
    auto&& t = res_desc.texture;

    auto&& tdesc = desc.texture;
    auto&& range = tdesc.subresource_range;

    D3D12_DEPTH_STENCIL_VIEW_DESC dsvdesc{};
    dsvdesc.Format = util::GetNativeFormat(desc.view.format);
    dsvdesc.Flags = util::GetNativeDepthStencilViewFlags(desc.flags);

    auto SetMipParams = [&](auto* _src_dimension)
    {
        _src_dimension->MipSlice = range.offset.mip_slice;
    };
    auto SetArrayParams = [&](auto* _src_dimension)
    {
        _src_dimension->FirstArraySlice = range.offset.array_slice;
        _src_dimension->ArraySize       = range.array_size;
    };

    // 既存のDXGI形式はどのビューから作成された平面かを明確にするのに十分であるため、深さおよびステンシル形式のビュー作成にPlaneSliceパラメーターは追加されません。: https://github.com/microsoft/DirectX-Specs/blob/master/d3d/PlanarDepthStencilDDISpec.md#view-creation
    //auto SetPlaneParams = [&](auto* _src_dimension)
    //{
    //    _src_dimension->PlaneSlice = util::GetNativeAspectFlags(range.offset.aspect);
    //};

    auto As1D = [&](auto* _desc)
    {
        dsvdesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE1D;
        SetMipParams(_desc);
    };
    auto As1DArray = [&](auto* _desc)
    {
        dsvdesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE1DARRAY;
        SetMipParams(_desc); SetArrayParams(_desc);
    };
    auto As2D = [&](auto* _desc)
    {
        dsvdesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
        SetMipParams(_desc);
    };
    auto As2DArray = [&](auto* _desc)
    {
        dsvdesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
        SetMipParams(_desc); SetArrayParams(_desc);
    };
    auto As2DMS = [&](auto* _desc)
    {
        dsvdesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMS;
        B3D_UNREFERENCED(_desc->UnusedField_NothingToDefine);
    };
    auto As2DMSArray = [&](auto* _desc)
    {
        dsvdesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMSARRAY;
        SetArrayParams(_desc);
    };

    switch (desc.view.dimension)
    {
    case VIEW_DIMENSION_TEXTURE_1D        : As1D      (&dsvdesc.Texture1D);                                                                    break;
    case VIEW_DIMENSION_TEXTURE_1D_ARRAY  : As1DArray (&dsvdesc.Texture1DArray);                                                               break;
    case VIEW_DIMENSION_TEXTURE_2D        : t.sample_count == 1 ? As2D     (&dsvdesc.Texture2D)      : As2DMS     (&dsvdesc.Texture2DMS);      break;
    case VIEW_DIMENSION_TEXTURE_2D_ARRAY  : t.sample_count == 1 ? As2DArray(&dsvdesc.Texture2DArray) : As2DMSArray(&dsvdesc.Texture2DMSArray); break;

    default:
        B3D_ASSERT(false);
        break;
    }

    descriptor = device->GetCPUDescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, B3D_DEFAULT_NODE_MASK).Allocate();
    device12->CreateDepthStencilView(texture->GetD3D12Resource(), &dsvdesc, descriptor.handle);

    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY DepthStencilViewD3D12::CopyDesc(const DEPTH_STENCIL_VIEW_DESC& _desc)
{
    desc = _desc;
}

void
B3D_APIENTRY DepthStencilViewD3D12::Uninit()
{
    if (descriptor.handle)
        device->GetCPUDescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, B3D_DEFAULT_NODE_MASK).Free(descriptor);
    descriptor = {};

    hlp::SafeRelease(resource);
    hlp::SafeRelease(device);

    name.reset();
    desc = {};
}


BMRESULT
B3D_APIENTRY DepthStencilViewD3D12::Create(DeviceD3D12* _device, IResource* _resource, const DEPTH_STENCIL_VIEW_DESC& _desc, DepthStencilViewD3D12** _dst)
{
    util::Ptr<DepthStencilViewD3D12> ptr;
    ptr.Attach(B3DCreateImplementationClass(DepthStencilViewD3D12));
    B3D_RET_IF_FAILED(ptr->Init(_device, _resource, _desc));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY DepthStencilViewD3D12::AddRef()
{
    ++ref_count;
    B3D_REFCOUNT_DEBUG(ref_count);
}

uint32_t
B3D_APIENTRY DepthStencilViewD3D12::Release()
{
    B3D_REFCOUNT_DEBUG(ref_count - 1);
    auto count = --ref_count;
    if (count == 0)
        B3DDestroyImplementationClass(this);

    return count;
}

uint32_t
B3D_APIENTRY DepthStencilViewD3D12::GetRefCount() const
{
    return ref_count;
}

const char*
B3D_APIENTRY DepthStencilViewD3D12::GetName() const
{
    return name ? name->c_str() : nullptr;
}

BMRESULT
B3D_APIENTRY DepthStencilViewD3D12::SetName(const char* _name)
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
B3D_APIENTRY DepthStencilViewD3D12::GetDevice() const
{
    return device;
}

const CPU_DESCRIPTOR_ALLOCATION*
B3D_APIENTRY DepthStencilViewD3D12::GetCpuDescriptorAllocation() const
{
    return &descriptor;
}

const D3D12_GPU_VIRTUAL_ADDRESS*
B3D_APIENTRY DepthStencilViewD3D12::GetGpuVirtualAddress() const
{
    return nullptr;
}

bool
B3D_APIENTRY DepthStencilViewD3D12::HasAllSubresources() const
{
    return has_all_subresources;
}

const BUFFER_VIEW*
B3D_APIENTRY DepthStencilViewD3D12::GetBufferView() const
{
    return nullptr;
}

const TEXTURE_VIEW*
B3D_APIENTRY DepthStencilViewD3D12::GetTextureView() const
{
    return &desc.texture;
}

const VIEW_DESC&
B3D_APIENTRY DepthStencilViewD3D12::GetViewDesc() const
{
    return desc.view;
}

IResource*
B3D_APIENTRY DepthStencilViewD3D12::GetResource() const
{
    return resource;
}

const DEPTH_STENCIL_VIEW_DESC&
B3D_APIENTRY DepthStencilViewD3D12::GetDesc() const
{
    return desc;
}


}// namespace buma3d

