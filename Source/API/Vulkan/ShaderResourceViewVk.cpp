#include "Buma3DPCH.h"
#include "ShaderResourceViewVk.h"

namespace buma3d
{

struct ShaderResourceViewVk::IImpl : util::details::NEW_DELETE_OVERRIDE
{
    virtual ~IImpl() {}

    virtual BMRESULT SetName(const char* _name) = 0;

    virtual VkResult Create(VkBufferViewCreateInfo* _ci, bool _is_typed_buffer) { return VkResult(-1); }
    virtual VkResult Create(VkImageViewCreateInfo* _ci)  { return VkResult(-1); }
        
    virtual VkBufferView GetBufferView() const { return VK_NULL_HANDLE; }
    virtual VkImageView GetImageView()   const { return VK_NULL_HANDLE; }

    virtual const VkDescriptorBufferInfo* GetVkDescriptorBufferInfo() const { return nullptr; }
    virtual const VkDescriptorImageInfo*  GetVkDescriptorImageInfo()  const { return nullptr; }
    virtual VkImageLayout                 GetVkImageLayout()          const { return VK_IMAGE_LAYOUT_UNDEFINED; }

    virtual BMRESULT AddDescriptorWriteRange(DescriptorSet0Vk::UPDATE_DESCRIPTOR_RANGE_BUFFER* _dst, uint32_t _array_index) const = 0;

    virtual const VkImageSubresourceRange* GetVkImageSubresourceRange() const { return nullptr; }

};

// FIXME: VulkanのAPIはSRV、UAVが混合しているので、コード量削減のために共通の実装クラスを用意すべき。
class ShaderResourceViewVk::BufferViewImpl : public ShaderResourceViewVk::IImpl
{
public:
    BufferViewImpl(ShaderResourceViewVk& _owner)
        : owner { _owner }
        , view  {}
        , info  {}
    {
    }

    virtual ~BufferViewImpl()
    {
        B3DSafeDelete(info);

        if (view)
            vkDestroyBufferView(owner.vkdevice, view, owner.GetVkAllocationCallbacks());
        view = VK_NULL_HANDLE;
    }

    BMRESULT SetName(const char* _name) override
    {
        B3D_RET_IF_FAILED(owner.device->SetVkObjectName(view, _name));
        return BMRESULT_SUCCEED;
    }

    const VkDescriptorBufferInfo* GetVkDescriptorBufferInfo() const override
    {
        return info;
    }

    VkBufferView GetBufferView() const override
    {
        return view;
    }

    VkResult Create(VkBufferViewCreateInfo* _ci, bool _is_typed_buffer) override
    {
        if (_is_typed_buffer)
            return vkCreateBufferView(owner.vkdevice, _ci, owner.GetVkAllocationCallbacks(), &view);

        info = B3DNewArgs(VkDescriptorBufferInfo, { _ci->buffer, _ci->offset, _ci->range });
        return VK_SUCCESS;
    }

    BMRESULT AddDescriptorWriteRange(DescriptorSet0Vk::UPDATE_DESCRIPTOR_RANGE_BUFFER* _dst, uint32_t _array_index) const override
    {
        if (view)
        {
            if (!_dst->texel_buffer_views_data)
                return BMRESULT_FAILED;
            _dst->texel_buffer_views_data[_array_index] = view;
        }
        else
        {
            if (!_dst->buffer_infos_data)
                return BMRESULT_FAILED;
            _dst->buffer_infos_data[_array_index] = *info;
        }

        return BMRESULT_SUCCEED;
    }

private:
    ShaderResourceViewVk&   owner;
    VkBufferView            view;
    VkDescriptorBufferInfo* info;

};

class ShaderResourceViewVk::ImageViewImpl : public ShaderResourceViewVk::IImpl
{
public:
    ImageViewImpl(ShaderResourceViewVk& _owner)
        : owner                     { _owner }
        , view                      {}
        , image_subresource_range   {}
        , info                      { VK_NULL_HANDLE, VK_NULL_HANDLE, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL }
    {
    }

    virtual ~ImageViewImpl()
    {
        if (view)
            vkDestroyImageView(owner.vkdevice, view, owner.GetVkAllocationCallbacks());
        view = VK_NULL_HANDLE;
    }

    BMRESULT SetName(const char* _name) override
    {
        B3D_RET_IF_FAILED(owner.device->SetVkObjectName(view, _name));
        return BMRESULT_SUCCEED;
    }

    const VkDescriptorImageInfo* GetVkDescriptorImageInfo() const override
    {
        return &info;
    }

    VkImageView GetImageView() const override
    {
        return view;
    }

    VkResult Create(VkImageViewCreateInfo* _ci) override
    {
        util::ConvertNativeSubresourceRange(owner.desc.texture.subresource_range, &image_subresource_range);
        auto vkr = vkCreateImageView(owner.vkdevice, _ci, owner.GetVkAllocationCallbacks(), &view);
        info.imageView = view;

             if (owner.desc.flags & SHADER_RESOURCE_VIEW_FLAG_DSV_SIMULTANEOUS_DEPTH_READ)         info.imageLayout = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL;
        else if (owner.desc.flags & SHADER_RESOURCE_VIEW_FLAG_DSV_SIMULTANEOUS_STENCIL_READ)       info.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL;
        else if (owner.desc.flags & SHADER_RESOURCE_VIEW_FLAG_DSV_SIMULTANEOUS_DEPTH_STENCIL_READ) info.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

        return vkr;
    }

    VkImageLayout GetVkImageLayout() const override
    {
        return info.imageLayout;
    }

    BMRESULT AddDescriptorWriteRange(DescriptorSet0Vk::UPDATE_DESCRIPTOR_RANGE_BUFFER* _dst, uint32_t _array_index) const override
    {
        if (!_dst->image_infos_data)
            return BMRESULT_FAILED;

        _dst->image_infos_data[_array_index] = info;
        return BMRESULT_SUCCEED;
    }

    const VkImageSubresourceRange* GetVkImageSubresourceRange() const override
    {
        return &image_subresource_range;
    }

private:
    ShaderResourceViewVk&   owner;
    VkImageView             view;
    VkImageSubresourceRange image_subresource_range;
    VkDescriptorImageInfo   info;

};


B3D_APIENTRY ShaderResourceViewVk::ShaderResourceViewVk()
    : ref_count     { 1 }
    , name          {}
    , device        {}
    , desc          {}
    , resource      {}
    , vkdevice      {}
    , inspfn        {}
    , devpfn        {}
    , buffer_view   {}
    , texture_view  {}
{

}

B3D_APIENTRY ShaderResourceViewVk::~ShaderResourceViewVk()
{
    Uninit();
}

BMRESULT 
B3D_APIENTRY ShaderResourceViewVk::Init(DeviceVk* _device, IResource* _resource, const SHADER_RESOURCE_VIEW_DESC& _desc)
{
    (device = _device)->AddRef();
    vkdevice = device->GetVkDevice();
    inspfn = &device->GetInstancePFN();
    devpfn = &device->GetDevicePFN();

    (resource = _resource)->AddRef();
    CopyDesc(_desc);

    if (desc.view.type != VIEW_TYPE_SHADER_RESOURCE)
    {
        B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_INITIALIZATION
                          , "SHADER_RESOURCE_VIEW_DESC::view.typeはVIEW_TYPE_SHADER_RESOURCEである必要があります。");
        return BMRESULT_FAILED_INVALID_PARAMETER;
    }

    BMRESULT result = BMRESULT_SUCCEED;
    switch (desc.view.dimension)
    {
    case VIEW_DIMENSION_BUFFER_TYPED:
    case VIEW_DIMENSION_BUFFER_STRUCTURED:
    case VIEW_DIMENSION_BUFFER_BYTEADDRESS:
        result = InitAsBufferView();
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
        result = InitAsImageView();
        texture_view = &desc.texture;
        break;

    default:
        result = BMRESULT_FAILED_INVALID_PARAMETER;
        break;
    }

    return result;
}

void 
B3D_APIENTRY ShaderResourceViewVk::CopyDesc(const SHADER_RESOURCE_VIEW_DESC& _desc)
{
    desc = _desc;
}

BMRESULT 
B3D_APIENTRY ShaderResourceViewVk::InitAsBufferView()
{
    if (resource->GetDesc().dimension != RESOURCE_DIMENSION_BUFFER)
    {
        B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_INITIALIZATION
                          , "VIEW_TYPE_BUFFER_*の場合、リソースのdimensionはRESOURCE_DIMENSION_BUFFERである必要があります。");
        return BMRESULT_FAILED_INVALID_PARAMETER;
    }
    /*NOTE: 以下のシェーダーオブジェクトを使用する場合、VkBufferViewが必要です。
            (RW)StructuredBuffer
            (RW)ByteAddressBuffer
            (RW)Buffer
            tbuffer
    */
    /*
    StructureByteStrideの0でない場合、構造化バッファのビューが作成され、D3D12_UNORDERED_ACCESS_VIEW_DESC::FormatフィールドはDXGI_FORMAT_UNKNOWNである必要があります。
    StructureByteStrideが0の場合、バッファの型付き(typed)ビューが作成され、フォーマットを指定する必要があります。
    型付き(Typed)ビューに指定された形式は、ハードウェアでサポートされている必要があります。
    リソースには、生の非構造化データが含まれています。 UAV形式はDXGI_FORMAT_R32_TYPELESSである必要があります。
    */

    BMRESULT result = BMRESULT_SUCCEED;

    VkBufferViewCreateInfo ci{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
    ci.flags = 0/*reserved*/;
    ci.buffer = resource->As<BufferVk>()->GetVkBuffer();
    bool is_typed_buffer = false;

    auto&& bdesc = desc.buffer;
    switch (desc.view.dimension)
    {
    case VIEW_DIMENSION_BUFFER_TYPED:
    {
        if (desc.view.format == RESOURCE_FORMAT_UNKNOWN)
        {
            result = BMRESULT_FAILED_INVALID_PARAMETER;
            break;
        }
        auto format_size      = util::GetFormatSize(desc.view.format);
        auto texels_per_block = util::CalcTexelsPerBlock(desc.view.format);
        ci.offset = format_size * bdesc.first_element;
        ci.range  = format_size * bdesc.num_elements;
        ci.format = util::GetNativeFormat(desc.view.format);
        is_typed_buffer = true;
        break;
    }

    case VIEW_DIMENSION_BUFFER_STRUCTURED:
    {
        if (desc.view.format != RESOURCE_FORMAT_UNKNOWN)
        {
            result = BMRESULT_FAILED_INVALID_PARAMETER;
            break;
        }
        ci.offset = uint64_t(desc.buffer.structure_byte_stride) * bdesc.first_element;
        ci.range  = uint64_t(desc.buffer.structure_byte_stride) * bdesc.num_elements;
        break;
    }

    case VIEW_DIMENSION_BUFFER_BYTEADDRESS:
    {
        if (desc.view.format != RESOURCE_FORMAT_R32_TYPELESS)
        {
            result = BMRESULT_FAILED_INVALID_PARAMETER;
            break;
        }
        ci.offset = 32ui64 * bdesc.first_element;
        ci.range  = 32ui64 * bdesc.num_elements;
        break;
    }

    case VIEW_DIMENSION_BUFFER_ACCELERATION_STRUCTURE:
    {
        // TODO: VIEW_DIMENSION_BUFFER_ACCELERATION_STRUCTURE
        B3D_ASSERT(false && "TODO: VIEW_DIMENSION_BUFFER_ACCELERATION_STRUCTURE");
        break;
    }

    default:
        result = BMRESULT_FAILED_INVALID_PARAMETER;
        break;
    }
    
    if (hlp::IsFailed(result))
        return result;

    impl = B3DMakeUniqueArgs(BufferViewImpl, *this);
    auto vkr = impl->Create(&ci, is_typed_buffer);
    result = VKR_TRACE_IF_FAILED(vkr);

    return result;
}

BMRESULT 
B3D_APIENTRY ShaderResourceViewVk::ValidateTextureSRV()
{
    if (resource->GetDesc().dimension == RESOURCE_DIMENSION_BUFFER)
    {
        B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_INITIALIZATION
                          , "VIEW_TYPE_TEXTURE_*の場合、リソースのdimensionはRESOURCE_DIMENSION_TEX*である必要があります。");
        return BMRESULT_FAILED_INVALID_PARAMETER;
    }
    // TODO: エラー処理(実行結果を参照しながら埋めていく)
    auto&& t = resource->As<TextureVk>()->GetDesc().texture;

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

    if (tdesc.subresource_range.offset.aspect & ~(TEXTURE_ASPECT_FLAG_DEPTH | TEXTURE_ASPECT_FLAG_STENCIL))
    {
        if (desc.flags & (SHADER_RESOURCE_VIEW_FLAG_DSV_SIMULTANEOUS_DEPTH_READ | SHADER_RESOURCE_VIEW_FLAG_DSV_SIMULTANEOUS_STENCIL_READ))
        {
            B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_INITIALIZATION
                              , "aspectが深度、またはステンシルアスペクトではない場合、SHADER_RESOURCE_VIEW_DESC::flagsには"
                                "SHADER_RESOURCE_VIEW_FLAG_DSV_SIMULTANEOUS_DEPTH_READまたはSHADER_RESOURCE_VIEW_FLAG_DSV_SIMULTANEOUS_STENCIL_READ"
                                "が含まれていない必要があります。");
            return BMRESULT_FAILED_INVALID_PARAMETER;
        }
    }

    return BMRESULT_SUCCEED;
}

BMRESULT 
B3D_APIENTRY ShaderResourceViewVk::InitAsImageView()
{
    B3D_RET_IF_FAILED(ValidateTextureSRV());

    auto texture = resource->As<TextureVk>();
    /*NOTE: 以下のシェーダーオブジェクトを使用する場合、VkImageViewが必要です。
            (RW)Texture1D
            (RW)Texture1DArray
            (RW)Texture2D
            (RW)Texture2DArray
            (RW)Texture3D
            Texture2DMS
            Texture2DMSArray
            TextureCube
            TextureCubeArray
    */
    VkImageViewCreateInfo ci{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
    ci.flags    = 0;//VK_IMAGE_VIEW_CREATE_FRAGMENT_DENSITY_MAP_DYNAMIC_BIT_EXT;
    ci.image    = texture->GetVkImage();
    ci.viewType = util::GetNativeViewDimension(desc.view.dimension);
    ci.format   = util::GetNativeFormat(desc.view.format);

    auto&& tdesc = desc.texture;
    util::ConvertNativeComponentMapping(tdesc.components, &ci.components);
    ci.subresourceRange.aspectMask     = util::GetNativeAspectFlags(tdesc.subresource_range.offset.aspect);
    ci.subresourceRange.baseMipLevel   = tdesc.subresource_range.offset.mip_slice;
    ci.subresourceRange.levelCount     = tdesc.subresource_range.mip_levels;
    ci.subresourceRange.baseArrayLayer = tdesc.subresource_range.offset.array_slice;
    ci.subresourceRange.layerCount     = tdesc.subresource_range.array_size;

    auto last_pnext = &ci.pNext;

    VkImageViewUsageCreateInfo usage_ci{ VK_STRUCTURE_TYPE_IMAGE_VIEW_USAGE_CREATE_INFO };
    {
        // TODO: SRV Usage有効性の確認。
        //usage_ci.usage = texture->GetVkImageUsageFlags();
        usage_ci.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;

        if (desc.flags & SHADER_RESOURCE_VIEW_FLAG_DENY_INPUT_ATTACHMENT)
            usage_ci.usage &= ~VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;

        last_pnext = util::ConnectPNextChains(last_pnext, usage_ci);
    }

    BMRESULT result = BMRESULT_SUCCEED;

    // TODO: ShaderResourceViewVk: VkImageViewASTCDecodeModeEXT, VkSamplerYcbcrConversionInfo 
    VkImageViewASTCDecodeModeEXT atsc_ci{ VK_STRUCTURE_TYPE_IMAGE_VIEW_ASTC_DECODE_MODE_EXT };
    VkSamplerYcbcrConversionInfo ycbcr_ci{ VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_INFO };

    impl = B3DMakeUniqueArgs(ImageViewImpl, *this);
    auto vkr = impl->Create(&ci);
    result = VKR_TRACE_IF_FAILED(vkr);

    return result;
}

void
B3D_APIENTRY ShaderResourceViewVk::Uninit()
{
    impl.reset();
    hlp::SafeRelease(resource);

    hlp::SafeRelease(device);
    vkdevice = VK_NULL_HANDLE;
    inspfn = nullptr;
    devpfn = nullptr;

    name.reset();
    desc = {};
}

BMRESULT 
B3D_APIENTRY ShaderResourceViewVk::Create(DeviceVk* _device, IResource* _resource, const SHADER_RESOURCE_VIEW_DESC& _desc, ShaderResourceViewVk** _dst)
{
    util::Ptr<ShaderResourceViewVk> ptr;
    ptr.Attach(B3DCreateImplementationClass(ShaderResourceViewVk));
    B3D_RET_IF_FAILED(ptr->Init(_device, _resource, _desc));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY ShaderResourceViewVk::AddRef()
{
    ++ref_count;
    B3D_REFCOUNT_DEBUG(ref_count);
}

uint32_t
B3D_APIENTRY ShaderResourceViewVk::Release()
{
    B3D_REFCOUNT_DEBUG(ref_count - 1);
    auto count = --ref_count;
    if (count == 0)
        B3DDestroyImplementationClass(this);

    return count;
}

uint32_t
B3D_APIENTRY ShaderResourceViewVk::GetRefCount() const
{
    return ref_count;
}

const char*
B3D_APIENTRY ShaderResourceViewVk::GetName() const
{
    return name ? name->c_str() : nullptr;
}

BMRESULT
B3D_APIENTRY ShaderResourceViewVk::SetName(const char* _name)
{
    if (!util::IsEnabledDebug(this))
        return BMRESULT_FAILED;

    if (impl)
    {
        B3D_RET_IF_FAILED(impl->SetName(_name));
    }

    if (name && !_name)
        name.reset();
    else
        name = B3DMakeUniqueArgs(util::NameableObjStr, _name);

    return BMRESULT_SUCCEED;
}

IDevice*
B3D_APIENTRY ShaderResourceViewVk::GetDevice() const
{
    return device;
}

const BUFFER_VIEW*
B3D_APIENTRY ShaderResourceViewVk::GetBufferView() const
{
    return buffer_view;
}

const TEXTURE_VIEW*
B3D_APIENTRY ShaderResourceViewVk::GetTextureView() const
{
    return texture_view;
}

const VkAllocationCallbacks*
B3D_APIENTRY ShaderResourceViewVk::GetVkAllocationCallbacks() const
{
    return device->GetVkAllocationCallbacks();
}

const InstancePFN&
B3D_APIENTRY ShaderResourceViewVk::GetIsntancePFN() const
{
    return *inspfn;
}

const DevicePFN&
B3D_APIENTRY ShaderResourceViewVk::GetDevicePFN() const
{
    return *devpfn;
}

VkBufferView
B3D_APIENTRY ShaderResourceViewVk::GetVkBufferView() const
{
    return impl->GetBufferView();
}

const VkDescriptorBufferInfo*
B3D_APIENTRY ShaderResourceViewVk::GetVkDescriptorBufferInfo() const
{
    return impl->GetVkDescriptorBufferInfo();
}

const VkDescriptorImageInfo*
B3D_APIENTRY ShaderResourceViewVk::GetVkDescriptorImageInfo() const
{
    return impl->GetVkDescriptorImageInfo();
}

VkImageView
B3D_APIENTRY ShaderResourceViewVk::GetVkImageView() const
{
    return impl->GetImageView();
}

VkImageLayout
B3D_APIENTRY ShaderResourceViewVk::GetVkImageLayout() const
{
    return impl->GetVkImageLayout();
}

const VkImageSubresourceRange*
B3D_APIENTRY ShaderResourceViewVk::GetVkImageSubresourceRange() const
{
    return impl->GetVkImageSubresourceRange();
}

BMRESULT
B3D_APIENTRY ShaderResourceViewVk::AddDescriptorWriteRange(void* _dst, uint32_t _array_index) const
{
    return impl->AddDescriptorWriteRange(RCAST<DescriptorSet0Vk::UPDATE_DESCRIPTOR_RANGE_BUFFER*>(_dst), _array_index);
}

const VIEW_DESC&
B3D_APIENTRY ShaderResourceViewVk::GetViewDesc() const
{
    return desc.view;
}

IResource*
B3D_APIENTRY ShaderResourceViewVk::GetResource() const
{
    return resource;
}

const SHADER_RESOURCE_VIEW_DESC&
B3D_APIENTRY ShaderResourceViewVk::GetDesc() const
{
    return desc;
}


}// namespace buma3d
