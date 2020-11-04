#include "Buma3DPCH.h"
#include "BufferVk.h"

namespace buma3d
{

B3D_APIENTRY BufferVk::BufferVk()
    : ref_count           { 1 }
    , name                {}
    , device              {}
    , desc                {}
    , bind_node_masks     {}
    , create_type         {}
    , is_bound            {}
    , gpu_virtual_address {}
    , vkdevice            {}
    , inspfn              {}
    , devpfn              {}
    , heap                {}
    , buffer              {}
{

}

B3D_APIENTRY BufferVk::~BufferVk()
{
    Uninit();
}

BMRESULT
B3D_APIENTRY BufferVk::Init(RESOURCE_CREATE_TYPE _create_type, DeviceVk* _device, const RESOURCE_DESC& _desc)
{
    create_type = _create_type;

    (device = _device)->AddRef();
    vkdevice = _device->GetVkDevice();
    inspfn = &_device->GetInstancePFN();
    devpfn = &_device->GetDevicePFN();
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

void
B3D_APIENTRY BufferVk::CopyDesc(const RESOURCE_DESC& _desc)
{
    desc = _desc;
}

void 
B3D_APIENTRY BufferVk::PrepareCreateInfo(const RESOURCE_DESC& _desc, VkBufferCreateInfo* _dst_ci)
{
    _dst_ci->flags       = util::GetNativeBufferCreateFlags(_desc.flags, _desc.buffer.flags);
    _dst_ci->size        = _desc.buffer.size_in_bytes;
    _dst_ci->usage       = util::GetNativeBufferUsageFlags(_desc.buffer.usage);
    _dst_ci->sharingMode = _desc.flags & RESOURCE_FLAG_ALLOW_SIMULTANEOUS_ACCESS ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE;
    util::PrepareVkSharingMode(device, _dst_ci->sharingMode, _dst_ci);
}

BMRESULT
B3D_APIENTRY BufferVk::PrepareExternalMemoryCI(const void**& _last_pnext, const RESOURCE_DESC& _desc, const VkBufferCreateInfo& _ci, VkExternalMemoryBufferCreateInfo* _external_ci)
{
    // TODO: BufferVk::PrepareExternalMemoryCI
    B3D_UNREFERENCED(_desc);
    if (false)
    {
        //VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D12_HEAP_BIT;
        //VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D12_RESOURCE_BIT;
        //VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID;
        _external_ci->handleTypes;
        _last_pnext = util::ConnectPNextChains(_last_pnext, *_external_ci);

        VkPhysicalDeviceExternalBufferInfo bi{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_BUFFER_INFO };
        bi.handleType = VkExternalMemoryHandleTypeFlagBits(_external_ci->handleTypes);// 1つのビットのみを指定する必要がある
        bi.usage = _ci.usage;
        bi.flags = _ci.flags;

        VkExternalBufferProperties prop{ VK_STRUCTURE_TYPE_EXTERNAL_BUFFER_PROPERTIES };
        vkGetPhysicalDeviceExternalBufferProperties(device->GetPrimaryVkPhysicalDevice(), &bi, &prop);
        prop.externalMemoryProperties.compatibleHandleTypes;
        prop.externalMemoryProperties.exportFromImportedHandleTypes;// "インポートしたVkPhysicalDeviceExternalBufferInfo::handleTypeのハンドル"の(が)、更にエクスポート可能なハンドルタイプ
        prop.externalMemoryProperties.externalMemoryFeatures;
    }

    return BMRESULT_SUCCEED;
}

BMRESULT 
B3D_APIENTRY BufferVk::PrepareBindNodeMasks(uint32_t _heap_index, uint32_t _num_bind_node_masks, const NodeMask* _bind_node_masks)
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
B3D_APIENTRY BufferVk::PrepareVkBindBufferMemoryDeviceGroupInfo(const void**& _last_pnext, VkBindBufferMemoryDeviceGroupInfo* _device_group_bi, util::DyArray<uint32_t>* _device_inds, IResourceHeap* _src_heap)
{
    if (_src_heap && bind_node_masks)// bind_node_masksがnullptr以外の場合、この関数の上流でマルチインスタンスヒープの処理が行われている。
    {
        _device_inds->resize(bind_node_masks->size());
        auto device_inds_data = _device_inds->data();
        for (auto& i_node : *bind_node_masks)
        {
            *device_inds_data = hlp::GetFirstBitIndex(i_node);
            device_inds_data++;
        }
    }
    else if (_src_heap) // すべてのインスタンスのデバイスインデックスを同一にする。
    {
        _device_inds->resize(device->GetVkPhysicalDevices().size(), hlp::GetFirstBitIndex(_src_heap->GetDesc().creation_node_mask));
    }
    else
    {
        B3D_ASSERT(false && __FUNCTION__": 予期しない使用法です。");
    }
    _device_group_bi->deviceIndexCount = (uint32_t)_device_inds->size();
    _device_group_bi->pDeviceIndices   = _device_inds->data();
    _last_pnext = util::ConnectPNextChains(_last_pnext, *_device_group_bi);

    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY BufferVk::InitAsPlaced()
{
    VkBufferCreateInfo ci{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    PrepareCreateInfo(desc, &ci);
    auto last_pnext = &ci.pNext;

    VkExternalMemoryBufferCreateInfo external_ci{ VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_BUFFER_CREATE_INFO };
    B3D_RET_IF_FAILED(PrepareExternalMemoryCI(last_pnext, desc, ci, &external_ci));

    auto vkr = vkCreateBuffer(vkdevice, &ci, B3D_VK_ALLOC_CALLBACKS, &buffer);
    B3D_RET_IF_FAILED(VKR_TRACE_IF_FAILED(vkr));

    
    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY BufferVk::InitAsReserved()
{
    VkBufferCreateInfo ci{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    PrepareCreateInfo(desc, &ci);
    // スパースリソースはReservedResourceとして抽象化
    // FIXME: これら全てのフラグに対応していない場合に、機能をフォールバックして扱えるようにするかどうか。
    ci.flags |= VK_BUFFER_CREATE_SPARSE_BINDING_BIT | VK_BUFFER_CREATE_SPARSE_RESIDENCY_BIT | VK_BUFFER_CREATE_SPARSE_ALIASED_BIT;
    auto last_pnext = &ci.pNext;

    VkExternalMemoryBufferCreateInfo external_ci{ VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_BUFFER_CREATE_INFO };
    B3D_RET_IF_FAILED(PrepareExternalMemoryCI(last_pnext, desc, ci, &external_ci));

    auto vkr = vkCreateBuffer(vkdevice, &ci, B3D_VK_ALLOC_CALLBACKS, &buffer);
    B3D_RET_IF_FAILED(VKR_TRACE_IF_FAILED(vkr));

    // Reservedの場合ヒープが存在していなくてもViewを作成可能なのでバインド済みとする。
    MarkAsBound();

    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY BufferVk::InitAsCommitted(DeviceVk* _device, const COMMITTED_RESOURCE_DESC& _desc)
{
    B3D_RET_IF_FAILED(Init(RESOURCE_CREATE_TYPE_COMMITTED, _device, _desc.resource_desc));

    // マルチインスタンスヒープ
    B3D_RET_IF_FAILED(PrepareBindNodeMasks(_desc.heap_index, _desc.num_bind_node_masks, _desc.bind_node_masks));

    // バッファを作成

    VkBufferCreateInfo ci{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    PrepareCreateInfo(desc, &ci);
    auto last_pnext = &ci.pNext;

    VkExternalMemoryBufferCreateInfo external_ci{ VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_BUFFER_CREATE_INFO };
    B3D_RET_IF_FAILED(PrepareExternalMemoryCI(last_pnext, desc, ci, &external_ci));

    auto vkr = vkCreateBuffer(vkdevice, &ci, B3D_VK_ALLOC_CALLBACKS, &buffer);
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

void 
B3D_APIENTRY BufferVk::MarkAsBound()
{
    is_bound = true;

    VkBufferDeviceAddressInfo info{ VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO, nullptr, buffer };
    gpu_virtual_address = SCAST<GpuVirtualAddress>(vkGetBufferDeviceAddress(vkdevice, &info));// TODO: 機能サポートの確認
}

void 
B3D_APIENTRY BufferVk::Uninit()
{
    name.reset();
    desc = {};
    bind_node_masks.reset();
    is_bound = false;
    if (buffer)
        vkDestroyBuffer(vkdevice, buffer, B3D_VK_ALLOC_CALLBACKS);
    buffer = VK_NULL_HANDLE;
    hlp::SafeRelease(heap);
    hlp::SafeRelease(device);
    vkdevice = VK_NULL_HANDLE;
    inspfn = nullptr;
    devpfn = nullptr;
    gpu_virtual_address = 0;
}

BMRESULT
B3D_APIENTRY BufferVk::Create(RESOURCE_CREATE_TYPE _create_type, DeviceVk* _device, const RESOURCE_DESC& _desc, BufferVk** _dst)
{
    util::Ptr<BufferVk> ptr;
    ptr.Attach(B3DCreateImplementationClass(BufferVk));
    B3D_RET_IF_FAILED(ptr->Init(_create_type, _device, _desc));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY BufferVk::CreateCommitted(DeviceVk* _device, const COMMITTED_RESOURCE_DESC& _desc, BufferVk** _dst)
{
    util::Ptr<BufferVk> ptr;
    ptr.Attach(B3DCreateImplementationClass(BufferVk));
    B3D_RET_IF_FAILED(ptr->InitAsCommitted(_device, _desc));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY BufferVk::Bind(const BIND_RESOURCE_HEAP_INFO* _info)
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
        else if (heap)
        {
            B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_RESOURCE_MANIPULATION
                                , "ヒープは既にバインドされています。CreatePlacedResourceで作成されたリソースに一度バインドしたヒープは変更できず、このリソースが開放されるまでの間固有である必要があります。");
            return BMRESULT_FAILED_INVALID_CALL;
        }

        // マルチインスタンスヒープ
        B3D_RET_IF_FAILED(PrepareBindNodeMasks(_info->src_heap->GetDesc().heap_index, _info->num_bind_node_masks, _info->bind_node_masks));
    }

    VkBindBufferMemoryInfo bi{ VK_STRUCTURE_TYPE_BIND_BUFFER_MEMORY_INFO };
    bi.buffer       = buffer;
    bi.memory       = _info->src_heap->As<ResourceHeapVk>()->GetVkDeviceMemory();
    bi.memoryOffset = _info->src_heap_offset;// NOTE: 引数はアライメント済みである必要があります。

    auto last_pnext = &bi.pNext;

    // デバイスマスク
    VkBindBufferMemoryDeviceGroupInfo device_group_bi{ VK_STRUCTURE_TYPE_BIND_BUFFER_MEMORY_DEVICE_GROUP_INFO };
    util::DyArray<uint32_t> device_inds;
    B3D_RET_IF_FAILED(PrepareVkBindBufferMemoryDeviceGroupInfo(last_pnext, &device_group_bi, &device_inds, _info->src_heap));

    auto vkr = vkBindBufferMemory2(vkdevice, 1, &bi);
    B3D_RET_IF_FAILED(VKR_TRACE_IF_FAILED(vkr));

    if (create_type == RESOURCE_CREATE_TYPE_PLACED)
        (heap = _info->src_heap->As<ResourceHeapVk>())->AddRef(); // CreateForComittedからの呼び出しでなければ、カウントを成功後に追加

    MarkAsBound();

    return BMRESULT_SUCCEED;
}

void 
B3D_APIENTRY BufferVk::SetDedicatedAllocationInfo(VkMemoryDedicatedAllocateInfo* _dst_info) const
{
    _dst_info->buffer = buffer;
}

void 
B3D_APIENTRY BufferVk::GetMemoryRequirements(VkMemoryRequirements2* _dst_reqs) const
{
    VkBufferMemoryRequirementsInfo2 info{ VK_STRUCTURE_TYPE_BUFFER_MEMORY_REQUIREMENTS_INFO_2, nullptr, buffer };
    vkGetBufferMemoryRequirements2(vkdevice, &info, _dst_reqs);
}

RESOURCE_CREATE_TYPE
B3D_APIENTRY BufferVk::GetCreateType() const
{
    return create_type;
}

BMRESULT
B3D_APIENTRY BufferVk::SetupBindRegions(IResourceHeap* _dst_heap, uint32_t _num_regions, const TILED_RESOURCE_BIND_REGION* _regions, VkBindSparseInfo* _dst_info) const
{
    auto dst_heap = _dst_heap->As<ResourceHeapVk>();
    auto dst_mem  = dst_heap->GetVkDeviceMemory();
    auto al       = dst_heap->GetDesc().alignment;
    // Vulkan側の構造の各要素はCommandQueueVk::BindInfoBufferが必ず所有し、VkBindSparseInfoに予めセットされています。引数をシンプルにすることを目的としてconstを外して使用します。
    auto&& bi = *_dst_info;
    auto&& bb = CCAST<VkSparseBufferMemoryBindInfo*>(bi.pBufferBinds)[bi.bufferBindCount];

    for (uint32_t i = 0; i < _num_regions; i++)
    {
        auto&& r = _regions[i];
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
                                  , "バッファの場合、TILED_RESOURCE_BIND_REGION::flagsに TILED_RESOURCE_BIND_REGION_FLAG_MIPTAIL, TILED_RESOURCE_BIND_REGION_FLAG_METADATA 値が含まれていない必要があります。");
                return BMRESULT_FAILED_INVALID_PARAMETER;
            }
        }

        auto&& bind = CCAST<VkSparseMemoryBind*>(bb.pBinds)[i];
        bind.resourceOffset = al * r.dst_region.tile_offset.x;
        bind.size           = al * r.dst_region.tile_size.width;
        bind.memory         = r.flags & TILED_RESOURCE_BIND_REGION_FLAG_BIND_TO_NULL ? VK_NULL_HANDLE : dst_mem;
        bind.memoryOffset   = al * r.heap_tile_offset;
        bind.flags          = 0;
    }

    bb.buffer = buffer;
    bb.bindCount = _num_regions;
    bi.bufferBindCount++;

    return BMRESULT_SUCCEED;
}

uint32_t 
B3D_APIENTRY BufferVk::GetTiledResourceAllocationInfo(TILED_RESOURCE_ALLOCATION_INFO* _dst_infos) const
{
    if (create_type != RESOURCE_CREATE_TYPE_RESERVED)
    {
        B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_STATE_GETTING
                          , "リソースはCreateReservedResourceから作成されている必要があります。");
        return 0;
    }

    if (_dst_infos)
    {
        VkMemoryRequirements reqs{};
        vkGetBufferMemoryRequirements(vkdevice, buffer, &reqs);
        auto&& fp                      = _dst_infos->format_properties;
        fp.aspect                      = TEXTURE_ASPECT_FLAG_NONE;
        fp.flags                       = TILED_RESOURCE_FORMAT_FLAG_NONE;
        fp.tile_shape.width_in_texels  = (uint32_t)reqs.alignment;
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
B3D_APIENTRY BufferVk::AddRef()
{
    ++ref_count;
    B3D_REFCOUNT_DEBUG(ref_count);
}

uint32_t
B3D_APIENTRY BufferVk::Release()
{
    B3D_REFCOUNT_DEBUG(ref_count - 1);
    auto count = --ref_count;
    if (count == 0)
        B3DDestroyImplementationClass(this);

    return count;
}

uint32_t
B3D_APIENTRY BufferVk::GetRefCount() const
{
    return ref_count;
}

const char*
B3D_APIENTRY BufferVk::GetName() const
{
    return name ? name->c_str() : nullptr;
}

BMRESULT
B3D_APIENTRY BufferVk::SetName(const char* _name)
{
    if (!util::IsEnabledDebug(this))
        return BMRESULT_FAILED;

    if (buffer)
        B3D_RET_IF_FAILED(device->SetVkObjectName(buffer, _name));

    if (name && !_name)
        name.reset();
    else
        name = B3DMakeUniqueArgs(util::NameableObjStr, _name);

    return BMRESULT_SUCCEED;
}

IDevice*
B3D_APIENTRY BufferVk::GetDevice() const
{
    return device;
}

const VkAllocationCallbacks*
B3D_APIENTRY BufferVk::GetVkAllocationCallbacks() const
{
    return device->GetVkAllocationCallbacks();
}

const InstancePFN&
B3D_APIENTRY BufferVk::GetIsntancePFN() const
{
    return *inspfn;
}

const DevicePFN&
B3D_APIENTRY BufferVk::GetDevicePFN() const
{
    return *devpfn;
}

const RESOURCE_DESC&
B3D_APIENTRY BufferVk::GetDesc() const
{
    return desc;
}

IResourceHeap*
B3D_APIENTRY BufferVk::GetHeap() const
{
    return heap;
}

GpuVirtualAddress
B3D_APIENTRY BufferVk::GetGPUVirtualAddress() const
{
    return gpu_virtual_address;
}

VkBuffer
B3D_APIENTRY BufferVk::GetVkBuffer() const 
{
    return buffer; 
}


}// namespace buma3d
