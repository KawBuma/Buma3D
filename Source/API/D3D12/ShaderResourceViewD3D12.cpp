#include "Buma3DPCH.h"
#include "ShaderResourceViewD3D12.h"

namespace buma3d
{

B3D_APIENTRY ShaderResourceViewD3D12::ShaderResourceViewD3D12()
    : ref_count             { 1 }
    , name                  {}
    , device                {}
    , desc                  {}
    , resource              {}
    , device12              {}
    , descriptor            {}
    , virtual_address       {}
    , buffer_view           {}
    , texture_view          {}
    , has_all_subresources  {}
{

}

B3D_APIENTRY ShaderResourceViewD3D12::~ShaderResourceViewD3D12()
{
    Uninit();
}

BMRESULT
B3D_APIENTRY ShaderResourceViewD3D12::Init(DeviceD3D12* _device, IResource* _resource, const SHADER_RESOURCE_VIEW_DESC& _desc)
{
    (device = _device)->AddRef();
    device12 = device->GetD3D12Device();

    (resource = _resource)->AddRef();
    CopyDesc(_desc);

    // TODO: D3D12とVulkanのディスクリプタ使用の際の、ノードマスクのための互換性を検証する。
    B3D_ADD_DEBUG_MSG_INFO_B3D("TODO: D3D12とVulkanのディスクリプタ使用の際の、ノードマスクのための互換性を検証する。");

    if (desc.view.type != VIEW_TYPE_SHADER_RESOURCE)
    {
        B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_INITIALIZATION, "SHADER_RESOURCE_VIEW_DESC::view.typeはVIEW_TYPE_SHADER_RESOURCEである必要があります。");
        return BMRESULT_FAILED_INVALID_PARAMETER;
    }

    switch (desc.view.dimension)
    {
    case VIEW_DIMENSION_BUFFER_TYPED:
    case VIEW_DIMENSION_BUFFER_STRUCTURED:
    case VIEW_DIMENSION_BUFFER_BYTEADDRESS:
        B3D_RET_IF_FAILED(InitAsBufferSRV());
        buffer_view = &desc.buffer;
        break;

    case VIEW_DIMENSION_BUFFER_ACCELERATION_STRUCTURE:
        // TODO: VIEW_DIMENSION_BUFFER_ACCELERATION_STRUCTURE
        B3D_ASSERT(false && "TODO: VIEW_DIMENSION_BUFFER_ACCELERATION_STRUCTURE");
        break;

    case VIEW_DIMENSION_TEXTURE_1D:
    case VIEW_DIMENSION_TEXTURE_1D_ARRAY:
    case VIEW_DIMENSION_TEXTURE_2D:
    case VIEW_DIMENSION_TEXTURE_2D_ARRAY:
    case VIEW_DIMENSION_TEXTURE_3D:
    case VIEW_DIMENSION_TEXTURE_CUBE:
        B3D_RET_IF_FAILED(InitAsTextureSRV());
        texture_view = &desc.texture;
        has_all_subresources = resource->As<TextureD3D12>()->GetTextureProperties().IsAllSubresources(desc.texture.subresource_range);
        break;

    default:
        return BMRESULT_FAILED_INVALID_PARAMETER;
    }

    return BMRESULT_SUCCEED;
}

BMRESULT 
B3D_APIENTRY ShaderResourceViewD3D12::InitAsBufferSRV()
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

    D3D12_SHADER_RESOURCE_VIEW_DESC srvdesc{};
    srvdesc.Format                  = util::GetNativeFormat(desc.view.format);
    srvdesc.ViewDimension           = D3D12_SRV_DIMENSION_BUFFER;
    srvdesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

    auto&& bdesc = desc.buffer;
    auto&& srv_buf = srvdesc.Buffer;
    switch (desc.view.dimension)
    {
    case VIEW_DIMENSION_BUFFER_TYPED:
    {
        // NOTE: D3D12において、TextureBuffer/tbufferは型付けされたバッファー扱いをされている模様だが、要検証: https://blog.csdn.net/P_hantom/article/details/108083086
        if (desc.view.format == RESOURCE_FORMAT_UNKNOWN)
        {
            result = BMRESULT_FAILED_INVALID_PARAMETER;
            break;
        }
        auto format_size = util::GetFormatSize(desc.view.format);
        srv_buf.FirstElement        = bdesc.first_element;
        srv_buf.NumElements         = (UINT)bdesc.num_elements;
        srv_buf.StructureByteStride = 0;
        virtual_address = buffer->GetGPUVirtualAddress() + (format_size * bdesc.first_element * bdesc.num_elements);
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
        srv_buf.Flags               = D3D12_BUFFER_SRV_FLAG_RAW;
        virtual_address = buffer->GetGPUVirtualAddress() + (util::GetFormatSize(RESOURCE_FORMAT_R32_TYPELESS) * bdesc.first_element * bdesc.num_elements);
        break;
    }

    case VIEW_DIMENSION_BUFFER_ACCELERATION_STRUCTURE:
    {
        // TODO: VIEW_DIMENSION_BUFFER_ACCELERATION_STRUCTURE
        B3D_ASSERT(false && "TODO: VIEW_DIMENSION_BUFFER_ACCELERATION_STRUCTURE");
        srvdesc.RaytracingAccelerationStructure.Location = desc.acceleration_structure.location;
        break;
    }

    default:
        result = BMRESULT_FAILED_INVALID_PARAMETER;
        break;
    }
    
    if (hlp::IsFailed(result))
        return result;

    descriptor = device->GetCPUDescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, B3D_DEFAULT_NODE_MASK).Allocate();
    device12->CreateShaderResourceView(buffer->GetD3D12Resource(), &srvdesc, descriptor.handle);

    return BMRESULT_SUCCEED;
}

BMRESULT 
B3D_APIENTRY ShaderResourceViewD3D12::ValidateTextureSRV()
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
                          , "SHADER_RESOURCE_VIEW_DESC::...mip_sliceが大きすぎます。mip_sliceはリソースのミップ数より小さい必要があります。");
        return BMRESULT_FAILED_INVALID_PARAMETER;
    }
    if (range.offset.array_slice >= t.array_size)
    {
        B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_INITIALIZATION
                          , "SHADER_RESOURCE_VIEW_DESC::...array_sliceが大きすぎます。array_sliceはリソースの深さ/配列要素数より小さい必要があります。");
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
B3D_APIENTRY ShaderResourceViewD3D12::InitAsTextureSRV()
{
    B3D_RET_IF_FAILED(ValidateTextureSRV());

    auto texture = resource->As<TextureD3D12>();
    auto&& res_desc = texture->GetDesc();
    auto&& t = res_desc.texture;

    auto&& tdesc = desc.texture;
    auto&& range = tdesc.subresource_range;

    D3D12_SHADER_RESOURCE_VIEW_DESC srvdesc{};

    if (util::IsDepthStencilFormat(desc.view.format))
        srvdesc.Format = util::ConvertDepthStencilFormat(desc.view.format, tdesc.subresource_range.offset.aspect);
    else
        srvdesc.Format = util::GetNativeFormat(desc.view.format);

    util::ConvertNativeComponentMapping(tdesc.components, &srvdesc.Shader4ComponentMapping);

    auto SetMipParams = [&](auto* _src_dimension) {
        _src_dimension->MostDetailedMip = range.offset.mip_slice;
        _src_dimension->MipLevels       = range.mip_levels;
    };
    auto SetArrayParams = [&](auto* _src_dimension) {
        _src_dimension->FirstArraySlice = range.offset.array_slice;
        _src_dimension->ArraySize       = range.array_size;
    };
    auto SetPlaneParams = [&](auto* _src_dimension) {
        _src_dimension->PlaneSlice = util::GetNativeAspectFlags(range.offset.aspect);
    };
    auto SetMinLODClamp = [&](auto* _src_dimension) {
        _src_dimension->ResourceMinLODClamp = 0.f;/* TODO: 互換機能を見つける。 */
    };
    auto SetCubeArrayParams = [&](auto* _src_dimension) { 
        _src_dimension->First2DArrayFace = range.offset.array_slice;
        _src_dimension->NumCubes         = range.array_size / 6;
    };

    auto As1D = [&](auto* _desc) {
        srvdesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1D;
        SetMipParams(_desc); SetMinLODClamp(_desc);
    };
    auto As1DArray = [&](auto* _desc) {
        srvdesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
        SetMipParams(_desc); SetArrayParams(_desc); SetMinLODClamp(_desc);
    };
    auto As2D = [&](auto* _desc) {
        srvdesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        SetMipParams(_desc); SetPlaneParams(_desc); SetMinLODClamp(_desc);
    };
    auto As2DArray = [&](auto* _desc) {
        srvdesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
        SetMipParams(_desc); SetArrayParams(_desc); SetPlaneParams(_desc); SetMinLODClamp(_desc);
    };
    auto As2DMS = [&](auto* _desc) {
        srvdesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
        B3D_UNREFERENCED(_desc->UnusedField_NothingToDefine);
    };
    auto As2DMSArray = [&](auto* _desc) {
        srvdesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY;
        SetArrayParams(_desc);
    };
    auto As3D = [&](auto* _desc) {
        srvdesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
        SetMipParams(_desc); SetMinLODClamp(_desc);
    };
    auto AsCube = [&](auto* _desc) {
        srvdesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
        SetMipParams(_desc); SetMinLODClamp(_desc);
    };
    auto AsCubeArray = [&](auto* _desc) { 
        srvdesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
        SetMipParams(_desc); SetCubeArrayParams(_desc); SetMinLODClamp(_desc);
    };

    switch (desc.view.dimension)
    {
    case VIEW_DIMENSION_TEXTURE_1D         :                       As1D        (&srvdesc.Texture1D);                                                break;
    case VIEW_DIMENSION_TEXTURE_1D_ARRAY   :                       As1DArray   (&srvdesc.Texture1DArray);                                           break;
    case VIEW_DIMENSION_TEXTURE_2D         : t.sample_count == 1 ? As2D        (&srvdesc.Texture2D)      : As2DMS     (&srvdesc.Texture2DMS);       break;
    case VIEW_DIMENSION_TEXTURE_2D_ARRAY   : t.sample_count == 1 ? As2DArray   (&srvdesc.Texture2DArray) : As2DMSArray(&srvdesc.Texture2DMSArray);  break;
    case VIEW_DIMENSION_TEXTURE_3D         :                       As3D        (&srvdesc.Texture3D);                                                break;
    case VIEW_DIMENSION_TEXTURE_CUBE       :                       AsCube      (&srvdesc.TextureCube);                                              break;
    case VIEW_DIMENSION_TEXTURE_CUBE_ARRAY :                       AsCubeArray (&srvdesc.TextureCubeArray);                                         break;

    default:
        B3D_ASSERT(false);
        break;
    }

    descriptor = device->GetCPUDescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, B3D_DEFAULT_NODE_MASK).Allocate();
    device12->CreateShaderResourceView(texture->GetD3D12Resource(), &srvdesc, descriptor.handle);

    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY ShaderResourceViewD3D12::CopyDesc(const SHADER_RESOURCE_VIEW_DESC& _desc)
{
    desc = _desc;
}

void
B3D_APIENTRY ShaderResourceViewD3D12::Uninit()
{
    if (descriptor.handle)
        device->GetCPUDescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, B3D_DEFAULT_NODE_MASK).Free(descriptor);
    descriptor = {};
    virtual_address = {};

    hlp::SafeRelease(resource);
    hlp::SafeRelease(device);

    desc = {};
    name.reset();
}


BMRESULT
B3D_APIENTRY ShaderResourceViewD3D12::Create(DeviceD3D12* _device, IResource* _resource, const SHADER_RESOURCE_VIEW_DESC& _desc, ShaderResourceViewD3D12** _dst)
{
    util::Ptr<ShaderResourceViewD3D12> ptr;
    ptr.Attach(B3DCreateImplementationClass(ShaderResourceViewD3D12));
    B3D_RET_IF_FAILED(ptr->Init(_device, _resource, _desc));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY ShaderResourceViewD3D12::AddRef() 
{
    ++ref_count;
    B3D_REFCOUNT_DEBUG(ref_count);
}

uint32_t
B3D_APIENTRY ShaderResourceViewD3D12::Release() 
{
    B3D_REFCOUNT_DEBUG(ref_count - 1);
    auto count = --ref_count;
    if (count == 0)
        B3DDestroyImplementationClass(this);

    return count;
}

uint32_t
B3D_APIENTRY ShaderResourceViewD3D12::GetRefCount() const 
{
    return ref_count;
}

const char*
B3D_APIENTRY ShaderResourceViewD3D12::GetName() const 
{
    return name ? name->c_str() : nullptr;
}

BMRESULT
B3D_APIENTRY ShaderResourceViewD3D12::SetName(const char* _name) 
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
B3D_APIENTRY ShaderResourceViewD3D12::GetDevice() const 
{
    return device;
}

const CPU_DESCRIPTOR_ALLOCATION*
B3D_APIENTRY ShaderResourceViewD3D12::GetCpuDescriptorAllocation() const
{
    return &descriptor;
}

const D3D12_GPU_VIRTUAL_ADDRESS*
B3D_APIENTRY ShaderResourceViewD3D12::GetGpuVirtualAddress() const
{
    return buffer_view ? &virtual_address : nullptr;
}

bool
B3D_APIENTRY ShaderResourceViewD3D12::HasAllSubresources() const
{
    return has_all_subresources;
}

const BUFFER_VIEW*
B3D_APIENTRY ShaderResourceViewD3D12::GetBufferView() const
{
    return buffer_view;
}

const TEXTURE_VIEW*
B3D_APIENTRY ShaderResourceViewD3D12::GetTextureView() const
{
    return texture_view;
}

const VIEW_DESC&
B3D_APIENTRY ShaderResourceViewD3D12::GetViewDesc() const 
{
    return desc.view;
}

IResource*
B3D_APIENTRY ShaderResourceViewD3D12::GetResource() const 
{
    return resource;
}

const SHADER_RESOURCE_VIEW_DESC&
B3D_APIENTRY ShaderResourceViewD3D12::GetDesc() const 
{
    return desc;
}

const CPU_DESCRIPTOR_ALLOCATION& 
B3D_APIENTRY ShaderResourceViewD3D12::GetCPUDescriptorAllocation() const
{
    return descriptor;
}

}// namespace buma3d

