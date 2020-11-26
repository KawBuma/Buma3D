#include "Buma3DPCH.h"
#include "TextureVk.h"

namespace buma3d
{

namespace /*anonymous*/
{

VkImageCreateFlags SwapchainFlagsToImageFlags(VkSwapchainCreateFlagsKHR _swapchain_flags)
{
    VkImageCreateFlags result = 0;

    // NOTE: SPLIT_INSTANCE_BIND_REGIONSへの対応は現状しない。
    //if (_swapchain_flags & VK_SWAPCHAIN_CREATE_SPLIT_INSTANCE_BIND_REGIONS_BIT_KHR)

    if (_swapchain_flags & VK_SWAPCHAIN_CREATE_PROTECTED_BIT_KHR)
        result |= VK_IMAGE_CREATE_PROTECTED_BIT;

    if (_swapchain_flags & VK_SWAPCHAIN_CREATE_MUTABLE_FORMAT_BIT_KHR)
        result |= VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT | VK_IMAGE_CREATE_EXTENDED_USAGE_BIT;

    return result;
}

RESOURCE_FLAGS SwapchainFlagsToResourceFlags(const VkSwapchainCreateInfoKHR& _swapchain_ci)
{
    RESOURCE_FLAGS result = RESOURCE_FLAG_NONE;

    if (_swapchain_ci.flags & VK_SWAPCHAIN_CREATE_PROTECTED_BIT_KHR)
        result |= RESOURCE_FLAG_PROTECTED;

    //if (_swapchain_ci.flags & VK_SWAPCHAIN_CREATE_MUTABLE_FORMAT_BIT_KHR)

    if (_swapchain_ci.imageSharingMode == VK_SHARING_MODE_CONCURRENT)
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

    //if (_flags & SWAP_CHAIN_BUFFER_FLAG_ALLOW_SIMULTANEOUS_ACCESS)

    return result;
}

enum BLOCK_TEXEL_SIZE
{
      BTS_8
    , BTS_16
    , BTS_32
    , BTS_64
    , BTS_128
    , BTS_CNT
};

/**
 * @brief Standard Sparse Image Block Shapes 
 *        https://www.khronos.org/registry/vulkan/specs/1.2-extensions/html/chap33.html#sparsememory-sparseblockshapesmsaa
*/
struct STANDARD_BLOCK_SHAPES
{
    VkExtent3D shape_2d    [BTS_CNT];   // Block Shape (2D)
    VkExtent3D shape_3d    [BTS_CNT];   // Block Shape (3D)
    VkExtent3D shape_2d_x2 [BTS_CNT];   // Block Shape (sample 2X)
    VkExtent3D shape_2d_x4 [BTS_CNT];   // Block Shape (sample 4X)
    VkExtent3D shape_2d_x8 [BTS_CNT];   // Block Shape (sample 8X)
    VkExtent3D shape_2d_x16[BTS_CNT];   // Block Shape (sample 16X)

    const VkExtent3D* Get(RESOURCE_DIMENSION _dimension, BLOCK_TEXEL_SIZE _texel_size, uint32_t _sample_count)
    {
        switch (_dimension)
        {
        case buma3d::RESOURCE_DIMENSION_TEX2D: return ByTexelSize(_texel_size, BySampleCount(_sample_count));
        case buma3d::RESOURCE_DIMENSION_TEX3D: return ByTexelSize(_texel_size, shape_3d);

        default:
            return nullptr;
        }
    }

private:
    const VkExtent3D* BySampleCount(uint32_t _sample_count) const
    {
        switch (_sample_count)
        {
        case 1  : return shape_2d;
        case 2  : return shape_2d_x2;
        case 4  : return shape_2d_x4;
        case 8  : return shape_2d_x8;
        case 16 : return shape_2d_x16;

        default:
            return nullptr;
        }
    }
    const VkExtent3D* ByTexelSize(size_t _size, const VkExtent3D* _shapes) const
    {
        if (!_shapes)
            return nullptr;

        switch (_size)
        {
        case 8   : return &_shapes[BTS_8   ];
        case 16  : return &_shapes[BTS_16  ];
        case 32  : return &_shapes[BTS_32  ];
        case 64  : return &_shapes[BTS_64  ];
        case 128 : return &_shapes[BTS_128 ];

        default:
            return nullptr;
        }
    }

};

static constexpr STANDARD_BLOCK_SHAPES standard_block_shapes =
{
    // Block Shape (2D)
    { { 256, 256,  1 }   // BTS_8
    , { 256, 128,  1 }   // BTS_16
    , { 128, 128,  1 }   // BTS_32
    , { 128,  64,  1 }   // BTS_64
    , {  64,  64,  1 } } // BTS_128
    
    // Block Shape (3D)
    , { { 64, 32, 32 }
      , { 32, 32, 32 }
      , { 32, 32, 16 }
      , { 32, 16, 16 }
      , { 16, 16, 16 } }

    // Block Shape (2X)
    , { { 128, 256, 1 }
      , { 128, 128, 1 }
      , {  64, 128, 1 }
      , {  64,  64, 1 }
      , {  32,  64, 1 } }
    
    // Block Shape (4X)
    , { { 128, 128, 1 }
      , { 128,  64, 1 }
      , {  64,  64, 1 }
      , {  64,  32, 1 }
      , {  32,  32, 1 } }
    
    // Block Shape (8X)
    , { { 64, 128, 1 }
      , { 64,  64, 1 }
      , { 32,  64, 1 }
      , { 32,  32, 1 }
      , { 16,  32, 1 } }

    // Block Shape (16X)
    , { { 64, 64, 1 }
      , { 64, 32, 1 }
      , { 32, 32, 1 }
      , { 32, 16, 1 }
      , { 16, 16, 1 } }
};


}// namespace /*anonymous*/


B3D_APIENTRY TextureVk::TextureVk()
    : ref_count       { 1 }
    , name            {}
    , device          {}
    , desc            {}
    , desc_data       {}
    , bind_node_masks {}
    , create_type     {}
    , is_bound        {}
    , heap            {}
    , vkdevice        {}
    , inspfn          {}
    , devpfn          {}
    , image           {}
    , sparse_data     {}
    , native_usage    {}
{

}

B3D_APIENTRY TextureVk::~TextureVk()
{
    Uninit();
}

BMRESULT
B3D_APIENTRY TextureVk::Init(RESOURCE_CREATE_TYPE _create_type, DeviceVk* _device, const RESOURCE_DESC& _desc)
{
    create_type = _create_type;

    (device = _device)->AddRef();
    vkdevice = _device->GetVkDevice();
    inspfn = &_device->GetInstancePFN();
    devpfn = &_device->GetDevicePFN();
    B3D_RET_IF_FAILED(CopyDesc(_desc));

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
B3D_APIENTRY TextureVk::CopyDesc(const RESOURCE_DESC& _desc)
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
    desc.texture.mip_levels = (uint16_t)util::CalcMipLevels(desc.texture);

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
B3D_APIENTRY TextureVk::PrepareCreateInfo(const RESOURCE_DESC& _desc, VkImageCreateInfo* _dst_ci)
{
    auto&& t = _desc.texture;
    _dst_ci->flags         = util::GetNativeTextureCreateFlags(_desc.flags, t.flags);
    _dst_ci->imageType     = util::GetNativeResourceDimension (_desc.dimension);
    _dst_ci->format        = util::GetNativeFormat            (_desc.texture.format_desc.format);
    _dst_ci->extent.width  = t.extent.width;
    _dst_ci->extent.height = t.extent.height;
    _dst_ci->extent.depth  = t.extent.depth;
    _dst_ci->arrayLayers   = t.array_size;
    _dst_ci->mipLevels     = util::CalcMipLevels             (_desc.texture);
    _dst_ci->samples       = util::GetNativeSampleCount      (t.sample_count);
    _dst_ci->tiling        = util::GetNativeTextureLayout    (t.layout);
    _dst_ci->usage         = util::GetNativeTextureUsageFlags(t.usage);
    _dst_ci->initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;// FIXME: リニア画像のみVK_IMAGE_LAYOUT_PREINITIALIZEDが指定可能だが、一旦決め打ち。
    _dst_ci->sharingMode   = _desc.flags & RESOURCE_FLAG_ALLOW_SIMULTANEOUS_ACCESS ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE;
    util::PrepareVkSharingMode(device, _dst_ci->sharingMode, _dst_ci);

    native_usage = _dst_ci->usage;

    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY TextureVk::PrepareStencilUsageCI(const void**& _last_pnext, VkImageStencilUsageCreateInfo* _stencil_usage_ci)
{
    // TODO: TextureVk::PrepareStencilUsageCI
    // FIXME: ステンシル固有のusageを指定するが、現状保留
    B3D_UNREFERENCED(_last_pnext, _stencil_usage_ci);
    //if (false)
    //{
    // _stencil_usage_ci->stencilUsage = vk_image_usage;
    // _last_pnext = util::ConnectPNextChains(_last_pnext, *_stencil_usage_ci);
    //}

    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY TextureVk::PrepareFormatListCI(const void**& _last_pnext, const VkImageCreateInfo& _ci, VkImageFormatListCreateInfo* _format_list_ci, util::SharedPtr<util::DyArray<VkFormat>>* _dst_formats)
{
    /*TODO: 40.1.1. Compatible formats of planes of multi-planar formats https://www.khronos.org/registry/vulkan/specs/1.2-extensions/html/vkspec.html#formats-compatible-planes
            に載っている、マルチプラナーフォーマットに対する各プレーンスライス毎のフォーマットも、この構造体のpViewFormatsにリストしなければならないのかが、仕様の記述だけでは曖昧なので実際に検証する。
            その場合FormatCompatibilityCheckerにも色々追加しなければならない。あとIsMultiPlanarFormat()とか。*/

    if (_ci.flags & VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT)
    {
        auto&& tfd = desc.texture.format_desc;
        if (desc_data->is_shared_from_typeless_compatible_formats)
        {
            // 事前に作成された全ての互換フォーマットが格納された配列を使用
            *_dst_formats = device->GetFormatCompatibilityChecker().GetTypelessCompatibleFormats().at(tfd.format)->compatible_vkformats;
        }
        else
        {
            *_dst_formats = B3DMakeShared(util::DyArray<VkFormat>);

            (*_dst_formats)->reserve(desc_data->mutable_formats->size());
            for (auto& i : *desc_data->mutable_formats)
                (*_dst_formats)->emplace_back(util::GetNativeFormat(i));
        }

        // この構造には、この画像のビューを作成するときに使用できるすべての形式のリストが含まれています。
        _format_list_ci->viewFormatCount = (uint32_t)(*_dst_formats)->size();
        _format_list_ci->pViewFormats = (*_dst_formats)->data();
        _last_pnext = util::ConnectPNextChains(_last_pnext, *_format_list_ci);
    }

    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY TextureVk::PrepareDrmFormatModifierExplicitCI(const void**& _last_pnext, VkImageDrmFormatModifierExplicitCreateInfoEXT* _drm_format_modifier_explicit_ci)
{
    // TODO: TextureVk::PrepareDrmFormatModifierExplicitCI
    if (false)
    {
        auto&& _drm = _drm_format_modifier_explicit_ci;
        _drm->drmFormatModifier;
        _drm->drmFormatModifierPlaneCount;
        _drm->pPlaneLayouts;
        _last_pnext = util::ConnectPNextChains(_last_pnext, *_drm);
    }

    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY TextureVk::PrepareDrmFormatModifierListCI(const void**& _last_pnext, VkImageDrmFormatModifierListCreateInfoEXT* _drm_format_modifier_list_ci)
{
    // TODO: TextureVk::PrepareDrmFormatModifierListCI
    if (false)
    {
        auto&& _drm = _drm_format_modifier_list_ci;
        _drm->drmFormatModifierCount;
        _drm->pDrmFormatModifiers;
        _last_pnext = util::ConnectPNextChains(_last_pnext, *_drm);
    }

    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY TextureVk::PrepareExternalMemoryCI(const void**& _last_pnext, const RESOURCE_DESC& _desc, const VkImageCreateInfo& _ci, VkExternalMemoryImageCreateInfo* _external_ci, VkExternalMemoryImageCreateInfoNV* _external_ci_nv)
{
    // TODO: TextureVk::PrepareExternalMemoryCI
    if (false)
    {
        _external_ci->handleTypes;
        _last_pnext = util::ConnectPNextChains(_last_pnext, *_external_ci);
    }
    else if (false)
    {
        _external_ci_nv->handleTypes;
        _last_pnext = util::ConnectPNextChains(_last_pnext, *_external_ci_nv);
    }

    return BMRESULT_SUCCEED;
}

BMRESULT 
B3D_APIENTRY TextureVk::PrepareBindNodeMasks(uint32_t _heap_index, uint32_t _num_bind_node_masks, const NodeMask* _bind_node_masks)
{
    auto&& props = device->GetResourceHeapPropertiesForImpl()[_heap_index];
    bool is_multi_instance_heap = props.flags & RESOURCE_HEAP_PROPERTY_FLAG_MULTI_INSTANCE;
    bool is_invalod = (!is_multi_instance_heap && _num_bind_node_masks != 0);
         is_invalod |= (is_multi_instance_heap && _num_bind_node_masks != (uint32_t)device->GetVkPhysicalDevices().size());
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
B3D_APIENTRY TextureVk::PrepareVkBindImageMemoryDeviceGroupInfo(const void**& _last_pnext, VkBindImageMemoryDeviceGroupInfo* _device_group_bi, util::DyArray<uint32_t>* _device_inds, IResourceHeap* _src_heap, uint32_t _swapchain_device_index /*= -1*/)
{
    if (_swapchain_device_index != (uint32_t)-1)// スワップチェインVkImage[i]のインスタンスを指定
    {
        _device_inds->resize(device->GetVkPhysicalDevices().size(), _swapchain_device_index);
    }
    else if (_src_heap && bind_node_masks)// bind_node_masksがnullptr以外の場合、この関数の上流でマルチインスタンスヒープの処理が行われている。
    {
        /* NOTE: 各リソースには、各GPUのインスタンスがあります。(8/55 ページ) https://www.khronos.org/assets/uploads/developers/library/2017-gdc/GDC_Vulkan-on-Desktop_Feb17.pdf
                 メンタルモデル：リソースには、すべてのGPUにわたって単一の仮想アドレスがあり、各GPUのローカルまたは「ピア」メモリインスタンスにバインドできます。
                 詳細な仕様についてはこちらを参照してください: https://www.khronos.org/registry/vulkan/specs/1.2-extensions/html/vkspec.html#VkBindImageMemoryDeviceGroupInfo */
        auto size                 = bind_node_masks->size();
        auto bind_node_masks_data = bind_node_masks->data();

        _device_inds->resize(size);
        auto device_inds_data = _device_inds->data();
        for (size_t i = 0; i < size; i++)
            device_inds_data[i] = hlp::GetFirstBitIndex(bind_node_masks_data[i]);
    }
    else if (_src_heap)// すべてのインスタンスが同じメモリインスタンスをバインドする。
    {
        _device_inds->resize(device->GetVkPhysicalDevices().size(), hlp::GetFirstBitIndex(_src_heap->GetDesc().creation_node_mask));
    }
    else
    {
        B3D_ASSERT(false && __FUNCTION__": 予期せぬ使用法です。");
    }
    _device_group_bi->deviceIndexCount = (uint32_t)_device_inds->size();
    _device_group_bi->pDeviceIndices   = _device_inds->data();

    /*NOTE: pNext チェーンに VkBindImageMemoryDeviceGroupInfo 構造体が含まれている場合、
    pSplitInstanceBindRegionsのすべての要素は、imageの寸法内に含まれる有効な矩形でなければなりません。
    iamgeの同じインスタンスiに対応する pSplitInstanceBindRegions のすべての要素の領域の和は、画像全体をカバーしていなければならない。<-重要
    
    pNext チェーンに VkBindImageMemoryDeviceGroupInfo 構造体が含まれており、そしてsplitInstanceBindRegionCountが0でない場合は、
    VK_IMAGE_CREATE_SPLIT_INSTANCE_BIND_REGIONS_BITがセットされた状態で画像が作成されている必要があります。
    
    splitInstanceBindRegionCountがゼロより大きい場合、pSplitInstanceBindRegionsは 論理デバイス内の物理デバイス数^2 の矩形配列です。
    //_recs->resize(size * size);
    //_device_group_bi->splitInstanceBindRegionCount = _rects.size();
    //_device_group_bi->pSplitInstanceBindRegions = _rects.data();*/

    _last_pnext = util::ConnectPNextChains(_last_pnext, *_device_group_bi);

    return BMRESULT_SUCCEED;
}

void 
B3D_APIENTRY TextureVk::CreateSparseResourceData()
{
    sparse_data = B3DMakeUnique(SPARSE_RESOURCE_DATA);
    VkMemoryRequirements2 reqs{ VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2 };
    GetMemoryRequirements(&reqs);
    sparse_data->block_size = reqs.memoryRequirements.alignment;

    VkImageSparseMemoryRequirementsInfo2 mr_info{ VK_STRUCTURE_TYPE_IMAGE_SPARSE_MEMORY_REQUIREMENTS_INFO_2, nullptr, image };
    uint32_t mem_reqs_count = 0;
    vkGetImageSparseMemoryRequirements2(vkdevice, &mr_info, &mem_reqs_count, nullptr);

    if (mem_reqs_count)
    {
        sparse_data->memory_requirements.resize(mem_reqs_count, { VK_STRUCTURE_TYPE_SPARSE_IMAGE_MEMORY_REQUIREMENTS_2 });
        vkGetImageSparseMemoryRequirements2(vkdevice, &mr_info, &mem_reqs_count, sparse_data->memory_requirements.data());
    }
}

BMRESULT
B3D_APIENTRY TextureVk::InitAsPlaced()
{
    VkImageCreateInfo ci{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
    B3D_RET_IF_FAILED(PrepareCreateInfo(desc, &ci));
    auto last_pnext = &ci.pNext;
    
    // TODO: VkImageStencilUsageCreateInfo 
    VkImageStencilUsageCreateInfo stencil_usage_ci{ VK_STRUCTURE_TYPE_IMAGE_STENCIL_USAGE_CREATE_INFO };
    B3D_RET_IF_FAILED(PrepareStencilUsageCI(last_pnext, &stencil_usage_ci));

    VkImageFormatListCreateInfo format_list_ci{ VK_STRUCTURE_TYPE_IMAGE_FORMAT_LIST_CREATE_INFO };
    util::SharedPtr<util::DyArray<VkFormat>> view_formats;
    B3D_RET_IF_FAILED(PrepareFormatListCI(last_pnext, ci, &format_list_ci, &view_formats));

    // TODO: DrmFormatModifier
    VkImageDrmFormatModifierExplicitCreateInfoEXT drm_format_modifier_explicit_ci{ VK_STRUCTURE_TYPE_IMAGE_DRM_FORMAT_MODIFIER_EXPLICIT_CREATE_INFO_EXT };
    B3D_RET_IF_FAILED(PrepareDrmFormatModifierExplicitCI(last_pnext, &drm_format_modifier_explicit_ci));
    
    VkImageDrmFormatModifierListCreateInfoEXT drm_format_modifier_list_ci{ VK_STRUCTURE_TYPE_IMAGE_DRM_FORMAT_MODIFIER_LIST_CREATE_INFO_EXT };
    B3D_RET_IF_FAILED(PrepareDrmFormatModifierListCI(last_pnext, &drm_format_modifier_list_ci));

    // TODO: VkExternalMemoryImageCreateInfo
    VkExternalMemoryImageCreateInfo   external_ci    { VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO };
    VkExternalMemoryImageCreateInfoNV external_ci_nv { VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO_NV };
    B3D_RET_IF_FAILED(PrepareExternalMemoryCI(last_pnext, desc, ci, &external_ci, &external_ci_nv));

#if B3D_PLATFORM_IS_USED_ANDROID
    // TDOO: VkExternalFormatANDROID 
    VkExternalFormatANDROID external_format_adr{ VK_STRUCTURE_TYPE_EXTERNAL_FORMAT_ANDROID };
    static_assert(false, "TODO: VkExternalFormatANDROID");
#endif // B3D_PLATFORM_IS_USED_ANDROID

    auto vkr = vkCreateImage(vkdevice, &ci, B3D_VK_ALLOC_CALLBACKS, &image);
    B3D_RET_IF_FAILED(VKR_TRACE_IF_FAILED(vkr));

    return BMRESULT_SUCCEED; 
}

BMRESULT
B3D_APIENTRY TextureVk::InitAsReserved()
{
    VkImageCreateInfo ci{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
    B3D_RET_IF_FAILED(PrepareCreateInfo(desc, &ci));
    // スパースリソースはReservedResourceとして抽象化
    // FIXME: これら全てのフラグに対応していない場合に、機能をフォールバックして扱えるようにするかどうか。
    ci.flags |= VK_BUFFER_CREATE_SPARSE_BINDING_BIT | VK_BUFFER_CREATE_SPARSE_RESIDENCY_BIT | VK_BUFFER_CREATE_SPARSE_ALIASED_BIT;
    auto last_pnext = &ci.pNext;
    
    // TODO: VkImageStencilUsageCreateInfo 
    VkImageStencilUsageCreateInfo stencil_usage_ci{ VK_STRUCTURE_TYPE_IMAGE_STENCIL_USAGE_CREATE_INFO };
    B3D_RET_IF_FAILED(PrepareStencilUsageCI(last_pnext, &stencil_usage_ci));

    VkImageFormatListCreateInfo format_list_ci{ VK_STRUCTURE_TYPE_IMAGE_FORMAT_LIST_CREATE_INFO };
    util::SharedPtr<util::DyArray<VkFormat>> view_formats;
    B3D_RET_IF_FAILED(PrepareFormatListCI(last_pnext, ci, &format_list_ci, &view_formats));

    // TODO: DrmFormatModifier
    VkImageDrmFormatModifierExplicitCreateInfoEXT drm_format_modifier_explicit_ci{ VK_STRUCTURE_TYPE_IMAGE_DRM_FORMAT_MODIFIER_EXPLICIT_CREATE_INFO_EXT };
    B3D_RET_IF_FAILED(PrepareDrmFormatModifierExplicitCI(last_pnext, &drm_format_modifier_explicit_ci));
    
    VkImageDrmFormatModifierListCreateInfoEXT drm_format_modifier_list_ci{ VK_STRUCTURE_TYPE_IMAGE_DRM_FORMAT_MODIFIER_LIST_CREATE_INFO_EXT };
    B3D_RET_IF_FAILED(PrepareDrmFormatModifierListCI(last_pnext, &drm_format_modifier_list_ci));

    // TODO: VkExternalMemoryImageCreateInfo
    VkExternalMemoryImageCreateInfo   external_ci    { VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO };
    VkExternalMemoryImageCreateInfoNV external_ci_nv { VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO_NV };
    B3D_RET_IF_FAILED(PrepareExternalMemoryCI(last_pnext, desc, ci, &external_ci, &external_ci_nv));

#if B3D_PLATFORM_IS_USED_ANDROID
    // TDOO: VkExternalFormatANDROID 
    VkExternalFormatANDROID external_format_adr{ VK_STRUCTURE_TYPE_EXTERNAL_FORMAT_ANDROID };
    static_assert(false, "TODO: VkExternalFormatANDROID");
#endif // B3D_PLATFORM_IS_USED_ANDROID

    auto vkr = vkCreateImage(vkdevice, &ci, B3D_VK_ALLOC_CALLBACKS, &image);
    B3D_RET_IF_FAILED(VKR_TRACE_IF_FAILED(vkr));
    
    // スパースバインド時に使用する情報を取得。
    CreateSparseResourceData();

    // Reservedの場合、VK_BUFFER_CREATE_SPARSE_RESIDENCY_BITを使用することでヒープが存在していなくてもViewを作成可能となるのでバインド済みとする。
    MarkAsBound();

    // NOTE: Reserved用ヒープは作成しません。

    return BMRESULT_SUCCEED;
}

BMRESULT 
B3D_APIENTRY TextureVk::InitAsCommitted(DeviceVk* _device, const COMMITTED_RESOURCE_DESC& _desc)
{
    B3D_RET_IF_FAILED(Init(RESOURCE_CREATE_TYPE_COMMITTED, _device, _desc.resource_desc));

    // マルチインスタンスヒープ
    B3D_RET_IF_FAILED(PrepareBindNodeMasks(_desc.heap_index, _desc.num_bind_node_masks, _desc.bind_node_masks));

    // 画像を作成

    VkImageCreateInfo ci{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
    PrepareCreateInfo(desc, &ci);
    auto last_pnext = &ci.pNext;

    // 専用割り当て
    VkDedicatedAllocationImageCreateInfoNV dedicated_ci{ VK_STRUCTURE_TYPE_DEDICATED_ALLOCATION_IMAGE_CREATE_INFO_NV };
    dedicated_ci.dedicatedAllocation = VK_TRUE;
    last_pnext = util::ConnectPNextChains(last_pnext, dedicated_ci);

    // TODO: VkImageStencilUsageCreateInfo 
    VkImageStencilUsageCreateInfo stencil_usage_ci{ VK_STRUCTURE_TYPE_IMAGE_STENCIL_USAGE_CREATE_INFO };
    B3D_RET_IF_FAILED(PrepareStencilUsageCI(last_pnext, &stencil_usage_ci));

    VkImageFormatListCreateInfo format_list_ci{ VK_STRUCTURE_TYPE_IMAGE_FORMAT_LIST_CREATE_INFO };
    util::SharedPtr<util::DyArray<VkFormat>> view_formats;
    B3D_RET_IF_FAILED(PrepareFormatListCI(last_pnext, ci, &format_list_ci, &view_formats));

    // TODO: DrmFormatModifier
    VkImageDrmFormatModifierExplicitCreateInfoEXT drm_format_modifier_explicit_ci{ VK_STRUCTURE_TYPE_IMAGE_DRM_FORMAT_MODIFIER_EXPLICIT_CREATE_INFO_EXT };
    B3D_RET_IF_FAILED(PrepareDrmFormatModifierExplicitCI(last_pnext, &drm_format_modifier_explicit_ci));
    
    VkImageDrmFormatModifierListCreateInfoEXT drm_format_modifier_list_ci{ VK_STRUCTURE_TYPE_IMAGE_DRM_FORMAT_MODIFIER_LIST_CREATE_INFO_EXT };
    B3D_RET_IF_FAILED(PrepareDrmFormatModifierListCI(last_pnext, &drm_format_modifier_list_ci));

    // TODO: VkExternalMemoryImageCreateInfo
    VkExternalMemoryImageCreateInfo   external_ci    { VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO };
    VkExternalMemoryImageCreateInfoNV external_ci_nv { VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO_NV };
    B3D_RET_IF_FAILED(PrepareExternalMemoryCI(last_pnext, desc, ci, &external_ci, &external_ci_nv));

#if B3D_PLATFORM_IS_USED_ANDROID
    // TDOO: VkExternalFormatANDROID 
    VkExternalFormatANDROID external_format_adr{ VK_STRUCTURE_TYPE_EXTERNAL_FORMAT_ANDROID };
    static_assert(false, "TODO: VkExternalFormatANDROID");
#endif // B3D_PLATFORM_IS_USED_ANDROID

    auto vkr = vkCreateImage(vkdevice, &ci, B3D_VK_ALLOC_CALLBACKS, &image);
    B3D_RET_IF_FAILED(VKR_TRACE_IF_FAILED(vkr));

    // コミットリソース用の専用ヒープを作成。
    B3D_RET_IF_FAILED(ResourceHeapVk::CreateForCommitted(_device, _desc, this, &heap));

    // 専用ヒープをバインド
    BIND_RESOURCE_HEAP_INFO heap_bi =
    {
          heap                      // src_heap;
        , 0                         // src_heap_offset;
        , _desc.num_bind_node_masks // num_bind_node_masks;
        , _desc.bind_node_masks     // bind_node_masks;
        , this                      // dst_resource;
    };
    B3D_RET_IF_FAILED(Bind(&heap_bi));

    return BMRESULT_SUCCEED;
}

BMRESULT 
B3D_APIENTRY TextureVk::InitForSwapChain(SwapChainVk* _swapchain, const VkSwapchainCreateInfoKHR& _swapchain_ci, uint32_t _image_index, VkImage _swapchain_image)
{
    auto&& scd = _swapchain->GetDesc();
    RESOURCE_DESC desc_for_swap_chain{};
    desc_for_swap_chain.dimension                       = RESOURCE_DIMENSION_TEX2D;
    desc_for_swap_chain.flags                           = SwapchainFlagsToResourceFlags(_swapchain_ci);
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
    B3D_RET_IF_FAILED(Init(RESOURCE_CREATE_TYPE_SWAP_CHAIN, _swapchain->GetDevice()->As<DeviceVk>(), desc_for_swap_chain));

    // VkGetSwapchainImagesから取得されるデフォルトのVkImageを使用する。
    if (_swapchain_image)
    {
        image = _swapchain_image;
        return BMRESULT_SUCCEED;
    }

    // 複数のキューを用いたスワップチェイン用画像を作成
    create_type = RESOURCE_CREATE_TYPE_SWAP_CHAIN_MULTI_NODES;

    VkImageCreateInfo ci{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
    // VkImageSwapchainCreateInfoKHR::swapchainがVK_NULL_HANDLEでない場合、VkImageCreateInfoのフィールドは、スワップチェーンの暗黙の画像作成パラメーターと一致する必要があります。
    ci.flags                 = SwapchainFlagsToImageFlags(_swapchain_ci.flags);
    ci.format                = _swapchain_ci.imageFormat;
    ci.extent.width          = _swapchain_ci.imageExtent.width;
    ci.extent.height         = _swapchain_ci.imageExtent.height;
    ci.extent.depth          = 1;
    ci.arrayLayers           = _swapchain_ci.imageArrayLayers;
    ci.usage                 = _swapchain_ci.imageUsage;
    ci.sharingMode           = _swapchain_ci.imageSharingMode;
    ci.queueFamilyIndexCount = _swapchain_ci.queueFamilyIndexCount;
    ci.pQueueFamilyIndices   = _swapchain_ci.pQueueFamilyIndices;
    ci.imageType             = VK_IMAGE_TYPE_2D;
    ci.mipLevels             = 1;
    ci.samples               = VK_SAMPLE_COUNT_1_BIT;
    ci.tiling                = VK_IMAGE_TILING_OPTIMAL;
    ci.initialLayout         = VK_IMAGE_LAYOUT_UNDEFINED;

    native_usage = ci.usage;

    auto last_pnext = &ci.pNext;

    // スワップチェイン用イメージ
    VkImageSwapchainCreateInfoKHR swapchain_ci{ VK_STRUCTURE_TYPE_IMAGE_SWAPCHAIN_CREATE_INFO_KHR };
    swapchain_ci.swapchain = _swapchain->GetVkSwapchain();
    util::ConnectPNextChains(last_pnext, swapchain_ci);

    // TODO: 専用割り当て のスワップチェイン用イメージ作成時の有効性を確認。
    VkDedicatedAllocationImageCreateInfoNV dedicated_ci{ VK_STRUCTURE_TYPE_DEDICATED_ALLOCATION_IMAGE_CREATE_INFO_NV };
    dedicated_ci.dedicatedAllocation = VK_TRUE;
    last_pnext = util::ConnectPNextChains(last_pnext, dedicated_ci);

    VkImageFormatListCreateInfo format_list_ci{ VK_STRUCTURE_TYPE_IMAGE_FORMAT_LIST_CREATE_INFO };
    util::SharedPtr<util::DyArray<VkFormat>> view_formats;
    B3D_RET_IF_FAILED(PrepareFormatListCI(last_pnext, ci, &format_list_ci, &view_formats));

    // TODO: DrmFormatModifier
    VkImageDrmFormatModifierExplicitCreateInfoEXT drm_format_modifier_explicit_ci{ VK_STRUCTURE_TYPE_IMAGE_DRM_FORMAT_MODIFIER_EXPLICIT_CREATE_INFO_EXT };
    B3D_RET_IF_FAILED(PrepareDrmFormatModifierExplicitCI(last_pnext, &drm_format_modifier_explicit_ci));
    
    VkImageDrmFormatModifierListCreateInfoEXT drm_format_modifier_list_ci{ VK_STRUCTURE_TYPE_IMAGE_DRM_FORMAT_MODIFIER_LIST_CREATE_INFO_EXT };
    B3D_RET_IF_FAILED(PrepareDrmFormatModifierListCI(last_pnext, &drm_format_modifier_list_ci));

    auto vkr = vkCreateImage(vkdevice, &ci, B3D_VK_ALLOC_CALLBACKS, &image);
    B3D_RET_IF_FAILED(VKR_TRACE_IF_FAILED(vkr));

    // スワップチェインのメモリをバインド
    B3D_RET_IF_FAILED(BindForSwapChain(_swapchain, _image_index));

    // NOTE: スワップチェイン用ヒープは作成しません。

    return BMRESULT_SUCCEED;
}

void 
B3D_APIENTRY TextureVk::MarkAsBound()
{
    is_bound = true;
}

void 
B3D_APIENTRY TextureVk::Uninit()
{
    name.reset();
    desc = {};
    desc_data.reset();
    bind_node_masks.reset();
    is_bound = false;

    if (image && create_type != RESOURCE_CREATE_TYPE_SWAP_CHAIN)// スワップチェインから取得したデフォルトのVkImageの破棄はvkDestroySwapchainKHRで行われます。
        vkDestroyImage(vkdevice, image, B3D_VK_ALLOC_CALLBACKS);
    image = VK_NULL_HANDLE;

    hlp::SafeRelease(heap);
    hlp::SafeRelease(device);
    vkdevice = VK_NULL_HANDLE;
    inspfn = nullptr;
    devpfn = nullptr;

    sparse_data.reset();
}

BMRESULT
B3D_APIENTRY TextureVk::Create(RESOURCE_CREATE_TYPE _create_type, DeviceVk* _device, const RESOURCE_DESC& _desc, TextureVk** _dst)
{
    util::Ptr<TextureVk> ptr;
    ptr.Attach(B3DCreateImplementationClass(TextureVk));
    B3D_RET_IF_FAILED(ptr->Init(_create_type, _device, _desc));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY TextureVk::CreateCommitted(DeviceVk* _device, const COMMITTED_RESOURCE_DESC& _desc, TextureVk** _dst)
{
    util::Ptr<TextureVk> ptr;
    ptr.Attach(B3DCreateImplementationClass(TextureVk));
    B3D_RET_IF_FAILED(ptr->InitAsCommitted(_device, _desc));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

BMRESULT 
B3D_APIENTRY TextureVk::CreateForSwapChain(SwapChainVk* _swapchain, const VkSwapchainCreateInfoKHR& _swapchain_ci, uint32_t _image_index, VkImage _swapchain_image, TextureVk** _dst)
{
    util::Ptr<TextureVk> ptr;
    ptr.Attach(B3DCreateImplementationClass(TextureVk));
    B3D_RET_IF_FAILED(ptr->InitForSwapChain(_swapchain, _swapchain_ci, _image_index, _swapchain_image));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

BMRESULT 
B3D_APIENTRY TextureVk::Bind(const BIND_RESOURCE_HEAP_INFO* _info)
{
    if (create_type == RESOURCE_CREATE_TYPE_COMMITTED && !is_bound)
    {
        // CreateCommitted関数からの呼び出しなので、何もしない。
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

    VkBindImageMemoryInfo bi{ VK_STRUCTURE_TYPE_BIND_BUFFER_MEMORY_INFO };
    bi.image        = image;
    bi.memory       = _info->src_heap->As<ResourceHeapVk>()->GetVkDeviceMemory();
    bi.memoryOffset = _info->src_heap_offset;// NOTE: 引数はアライメント済みである必要があります。

    auto last_pnext = &bi.pNext;

    // pNextチェーンにVkBindImagePlaneMemoryInfo構造が含まれている場合、imageはVK_IMAGE_CREATE_DISJOINT_BITビットセットで作成されている必要があります。
    // NOTE: disjointは現状見送り
    //VkBindImagePlaneMemoryInfo plane_bi{ VK_STRUCTURE_TYPE_BIND_IMAGE_PLANE_MEMORY_INFO };
    //if (false)
    //{
    // plane_bi.planeAspect;
    // last_pnext = util::ConnectPNextChains(last_pnext, plane_bi);
    //}

    // デバイスマスク
    VkBindImageMemoryDeviceGroupInfo device_group_bi{ VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_DEVICE_GROUP_INFO };
    util::DyArray<uint32_t> device_inds;
    //util::DyArray<VkRect2D> rects;
    B3D_RET_IF_FAILED(PrepareVkBindImageMemoryDeviceGroupInfo(last_pnext, &device_group_bi, &device_inds, _info->src_heap));

    auto vkr = vkBindImageMemory2(vkdevice, 1, &bi);
    B3D_RET_IF_FAILED(VKR_TRACE_IF_FAILED(vkr));

    if (create_type == RESOURCE_CREATE_TYPE_PLACED)
        (heap = _info->src_heap->As<ResourceHeapVk>())->AddRef(); // カウントは成功後に追加

    MarkAsBound();

    return BMRESULT_SUCCEED;
}

BMRESULT 
B3D_APIENTRY TextureVk::BindForSwapChain(SwapChainVk* _swapchain, uint32_t _image_index)
{
    VkBindImageMemoryInfo bi{ VK_STRUCTURE_TYPE_BIND_BUFFER_MEMORY_INFO };
    bi.image        = image;
    bi.memory       = VK_NULL_HANDLE;// pNextチェーンにVkBindImageMemorySwapchainInfoKHR構造が含まれている場合、memoryはVK_NULL_HANDLEである必要があります。
    bi.memoryOffset = 0;

    auto last_pnext = &bi.pNext;

    // デバイスマスク
    // メモリはswapchainにバインドでき、VkBindImageMemoryDeviceGroupInfoのpDeviceIndicesまたはpSplitInstanceBindRegionsメンバーを使用できます。
    VkBindImageMemoryDeviceGroupInfo device_group_bi{ VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_DEVICE_GROUP_INFO };
    util::DyArray<uint32_t> device_inds;
    B3D_RET_IF_FAILED(PrepareVkBindImageMemoryDeviceGroupInfo(last_pnext, &device_group_bi, &device_inds, /*_src_heap = */nullptr, _swapchain->GetQueueNodeIndices()[_image_index]));

    // スワップチェイン
    VkBindImageMemorySwapchainInfoKHR swapchain_bi{ VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_SWAPCHAIN_INFO_KHR };
    swapchain_bi.imageIndex = _image_index;
    swapchain_bi.swapchain = _swapchain->GetVkSwapchain();
    last_pnext = util::ConnectPNextChains(last_pnext, swapchain_bi);

    auto vkr = vkBindImageMemory2(vkdevice, 1, &bi);
    B3D_RET_IF_FAILED(VKR_TRACE_IF_FAILED(vkr));

    MarkAsBound();

    return BMRESULT_SUCCEED;
}

void 
B3D_APIENTRY TextureVk::SetDedicatedAllocationInfo(VkMemoryDedicatedAllocateInfo* _dst_info) const
{
    _dst_info->image = image;
}

void 
B3D_APIENTRY TextureVk::GetMemoryRequirements(VkMemoryRequirements2* _dst_reqs) const
{
    VkImageMemoryRequirementsInfo2 info{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_REQUIREMENTS_INFO_2, nullptr, image };

    // FIXME: disjointマルチプラナーフォーマットの際にセットする。が、現在は実装していない。(d3d12との互換性が無い)
    //VkImagePlaneMemoryRequirementsInfo plane_info{ VK_STRUCTURE_TYPE_IMAGE_PLANE_MEMORY_REQUIREMENTS_INFO };
    //auto last_pnext = &info.pNext;
    //if (false)
    //{
    // //plane_info.planeAspect = VK_IMAGE_ASPECT_MEMORY_PLANE_{0,1,2,3}_BIT_EXT;
    // last_pnext = util::ConnectPNextChains(last_pnext, plane_info);
    //}

    vkGetImageMemoryRequirements2(vkdevice, &info, _dst_reqs);
}

RESOURCE_CREATE_TYPE
B3D_APIENTRY TextureVk::GetCreateType() const
{
    return create_type;
}

BMRESULT 
B3D_APIENTRY TextureVk::SetupBindRegions(IResourceHeap* _dst_heap, uint32_t _num_regions, const TILED_RESOURCE_BIND_REGION* _regions, VkBindSparseInfo* _dst_info) const
{
    auto dst_heap  = _dst_heap->As<ResourceHeapVk>();
    auto dst_mem   = dst_heap->GetVkDeviceMemory();
    // Vulkan側の構造の各要素はCommandQueueVk::BindInfoBufferが必ず所有し、VkBindSparseInfoに予めセットされています。引数をシンプルにすることを目的としてconstを外して使用します。
    auto&& bi = *_dst_info;

    auto&& memory_requirements_data = sparse_data->memory_requirements.data();
    auto block_size = sparse_data->block_size;
    auto BindImage = [&](const TILED_RESOURCE_BIND_REGION& _region) 
    {
        // 有効性の検証
        //if (util::IsEnabledDebug(this))
        {

        }

        auto&& ib = CCAST<VkSparseImageMemoryBindInfo*>(bi.pImageBinds)[bi.imageBindCount];
        {
            auto aspect_index = util::GetPlaneSliceFromAspectFlags(_region.dst_region.subresource.aspect);
            auto&& granularity = memory_requirements_data[aspect_index].memoryRequirements.formatProperties.imageGranularity;

            auto&& bind = CCAST<VkSparseImageMemoryBind*>(ib.pBinds)[ib.bindCount];
            util::ConvertNativeSubresourceOffset(_region.dst_region.subresource, &bind.subresource);
            bind.offset.x      = _region.dst_region.tile_offset.x    * granularity.width ;
            bind.offset.y      = _region.dst_region.tile_offset.y    * granularity.height;
            bind.offset.z      = _region.dst_region.tile_offset.x    * granularity.depth ;
            bind.extent.width  = _region.dst_region.tile_size.width  * granularity.width ;
            bind.extent.height = _region.dst_region.tile_size.height * granularity.height;
            bind.extent.depth  = _region.dst_region.tile_size.depth  * granularity.depth ;
            bind.memory        = _region.flags & TILED_RESOURCE_BIND_REGION_FLAG_BIND_TO_NULL ? VK_NULL_HANDLE : dst_mem;
            bind.memoryOffset  = _region.heap_tile_offset * block_size;
            bind.flags         = 0;

            ib.bindCount++;
        }

        return BMRESULT_SUCCEED;
    };
    auto BindImageOpaque = [&](const TILED_RESOURCE_BIND_REGION& _region)
    {
        auto&& iob = CCAST<VkSparseImageOpaqueMemoryBindInfo*>(bi.pImageOpaqueBinds)[bi.imageOpaqueBindCount];
        {
            auto aspect_index = util::GetPlaneSliceFromAspectFlags(_region.dst_miptail_region.subresource.aspect);
            auto&& reqs = memory_requirements_data[aspect_index].memoryRequirements;
            // 有効性の検証
            //if (util::IsEnabledDebug(this))
            {
                if (_region.flags & TILED_RESOURCE_BIND_REGION_FLAG_MIPTAIL && _region.dst_region.subresource.mip_slice != reqs.imageMipTailFirstLod)
                {
                    B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_RESOURCE_MANIPULATION
                                      , "TILE_REGION::subresource.mip_sliceがTILED_RESOURCE_ALLOCATION_INFO::first_mip_tail_sliceと一致しません。"
                                        "TILED_RESOURCE_BIND_REGION::flagsにTILED_RESOURCE_BIND_REGION_FLAG_MIPTAILが含まれる場合、mip_sliceは指定されたリソースのTILED_RESOURCE_ALLOCATION_INFO::first_mip_tail_sliceと一致する必要があります。");
                    return BMRESULT_FAILED_INVALID_PARAMETER;
                }
            }

            auto&& bind = CCAST<VkSparseMemoryBind*>(iob.pBinds)[iob.bindCount];
            bind.resourceOffset = _region.dst_miptail_region.offset_in_bytes;
            bind.size           = _region.dst_miptail_region.size_in_bytes;
            bind.memory         = _region.flags & TILED_RESOURCE_BIND_REGION_FLAG_BIND_TO_NULL ? VK_NULL_HANDLE : dst_mem;
            bind.memoryOffset   = _region.heap_tile_offset * block_size;

            /*NOTE: Vulkan実装は、同じ画像に対して標準のスパース画像ブロック形状とカスタムスパース画像ブロック形状の両方をサポートしてはなりません。
                    サポートされている場合は、標準のスパース画像ブロック形状を(優先して)使用する必要があります。
                    VK_SPARSE_MEMORY_BIND_METADATA_BITが存在する場合、resourceOffsetは、メタデータアスペクトに対して返されるスパースリソースプロパティのimageMipTailOffsetから明示的に派生(取得)している必要があります。
                    imageMipTailOffsetに返される値を操作することにより、resourceOffsetは、デバイスの仮想アドレスオフセットに直接関連付ける必要はありません。
                    代わりに、実装が正しいデバイス仮想アドレスを導出するのを最も簡単にする値にすることができます。*/
            bind.flags = _region.flags & TILED_RESOURCE_BIND_REGION_FLAG_METADATA ? VK_SPARSE_MEMORY_BIND_METADATA_BIT : 0;

            iob.bindCount++;
        }

        return BMRESULT_SUCCEED;
    };

    bool has_bound_image = false;
    bool has_bound_image_opaque = false;
    for (uint32_t i = 0; i < _num_regions; i++)
    {
        auto&& r = _regions[i];

        if (r.flags & (TILED_RESOURCE_BIND_REGION_FLAG_METADATA | TILED_RESOURCE_BIND_REGION_FLAG_MIPTAIL))
        {
            B3D_RET_IF_FAILED(BindImageOpaque(r));
            has_bound_image_opaque = true;
        }
        else
        {
            B3D_RET_IF_FAILED(BindImage(r));
            has_bound_image = true;
        }
    }

    if (has_bound_image)
    {
        CCAST<VkSparseImageMemoryBindInfo*>(bi.pImageBinds)[bi.imageBindCount].image = image;
        bi.imageBindCount++;
    }
    if (has_bound_image_opaque)
    {
        CCAST<VkSparseImageOpaqueMemoryBindInfo*>(bi.pImageOpaqueBinds)[bi.imageOpaqueBindCount].image = image;
        bi.imageOpaqueBindCount++;
    }

    return BMRESULT_SUCCEED;
}

uint32_t 
B3D_APIENTRY TextureVk::GetTiledResourceAllocationInfo(TILED_RESOURCE_ALLOCATION_INFO* _dst_infos) const
{
    if (create_type != RESOURCE_CREATE_TYPE_RESERVED)
    {
        B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_STATE_GETTING
                          , "リソースはCreateReservedResourceから作成されている必要があります。");
        return 0;
    }

    uint32_t num_reqs = (uint32_t)sparse_data->memory_requirements.size();
    auto reqs_data = sparse_data->memory_requirements.data();
    if (_dst_infos)
    {
        for (uint32_t i = 0; i < num_reqs; i++)
        {
            auto&& req = reqs_data[i].memoryRequirements;

            auto&& fpvk = req.formatProperties;
            auto&& fp = _dst_infos[i].format_properties;
            fp.aspect                      = util::GetB3DAspectFlags(fpvk.aspectMask);
            fp.flags                       = util::GetB3DTiledResourceFormatFlags(fpvk.flags);
            fp.tile_shape.width_in_texels  = fpvk.imageGranularity.width;
            fp.tile_shape.height_in_texels = fpvk.imageGranularity.height;
            fp.tile_shape.depth_in_texels  = fpvk.imageGranularity.depth;

            auto&& miptail = _dst_infos[i].mip_tail;
            if (req.imageMipTailSize != 0)
            {
                miptail.is_required     = true;
                miptail.first_mip_slice = req.imageMipTailFirstLod;
                miptail.size            = req.imageMipTailSize;
                miptail.offset          = req.imageMipTailOffset;
                miptail.stride          = req.imageMipTailStride;
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
        
    }

    return num_reqs;
}

void
B3D_APIENTRY TextureVk::AddRef()
{
    ++ref_count;
    B3D_REFCOUNT_DEBUG(ref_count);
}

uint32_t
B3D_APIENTRY TextureVk::Release()
{
    B3D_REFCOUNT_DEBUG(ref_count - 1);
    auto count = --ref_count;
    if (count == 0)
        B3DDestroyImplementationClass(this);

    return count;
}

uint32_t
B3D_APIENTRY TextureVk::GetRefCount() const
{
    return ref_count;
}

const char*
B3D_APIENTRY TextureVk::GetName() const
{
    return name ? name->c_str() : nullptr;
}

BMRESULT
B3D_APIENTRY TextureVk::SetName(const char* _name)
{
    if (!util::IsEnabledDebug(this))
        return BMRESULT_FAILED;

    if (image)
        B3D_RET_IF_FAILED(device->SetVkObjectName(image, _name));

    if (name && !_name)
        name.reset();
    else
        name = B3DMakeUniqueArgs(util::NameableObjStr, _name);

    return BMRESULT_SUCCEED;
}

IDevice* 
B3D_APIENTRY TextureVk::GetDevice() const
{
    return device;
}

const VkAllocationCallbacks*
B3D_APIENTRY TextureVk::GetVkAllocationCallbacks() const
{
    return device->GetVkAllocationCallbacks();
}

const InstancePFN&
B3D_APIENTRY TextureVk::GetIsntancePFN() const
{
    return *inspfn;
}

const DevicePFN&
B3D_APIENTRY TextureVk::GetDevicePFN() const
{
    return *devpfn;
}

const RESOURCE_DESC&
B3D_APIENTRY TextureVk::GetDesc() const
{
    return desc;
}

IResourceHeap* 
B3D_APIENTRY TextureVk::GetHeap() const
{
    return heap;
}

VkImage 
B3D_APIENTRY TextureVk::GetVkImage() const 
{
    return image; 
}

VkImageUsageFlags
B3D_APIENTRY TextureVk::GetVkImageUsageFlags() const
{
    return native_usage;
}


}// namespace buma3d
