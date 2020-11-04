#include "Buma3DPCH.h"
#include "ResourceHeapVk.h"

namespace buma3d
{

struct ResourceHeapVk::IMPORT_INFOS
{
    VkImportMemoryFdInfoKHR          import_fd           { VK_STRUCTURE_TYPE_IMPORT_MEMORY_FD_INFO_KHR };
    VkImportMemoryHostPointerInfoEXT import_host_pointer { VK_STRUCTURE_TYPE_IMPORT_MEMORY_HOST_POINTER_INFO_EXT };

#if B3D_PLATFORM_IS_USE_WINDOWS
    VkImportMemoryWin32HandleInfoKHR import_w32_khr { VK_STRUCTURE_TYPE_IMPORT_MEMORY_WIN32_HANDLE_INFO_KHR };
    VkImportMemoryWin32HandleInfoNV  import_w32_nv  { VK_STRUCTURE_TYPE_IMPORT_MEMORY_WIN32_HANDLE_INFO_NV };
#elif B3D_PLATFORM_IS_USE_ANDROID
VkImportAndroidHardwareBufferInfoANDROID import_ahd{ VK_STRUCTURE_TYPE_IMPORT_ANDROID_HARDWARE_BUFFER_INFO_ANDROID };
#endif

};

struct ResourceHeapVk::EXPORT_INFOS
{
    VkExportMemoryAllocateInfo         export_memory_ai    { VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO };
    VkExportMemoryAllocateInfoNV       export_memory_ai_nv { VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO_NV };
#if B3D_PLATFORM_IS_USE_WINDOWS
    VkExportMemoryWin32HandleInfoKHR   export_w32_khr      { VK_STRUCTURE_TYPE_EXPORT_MEMORY_WIN32_HANDLE_INFO_KHR };
    VkExportMemoryWin32HandleInfoNV    export_w32_nv       { VK_STRUCTURE_TYPE_EXPORT_MEMORY_WIN32_HANDLE_INFO_NV };
#endif
};

B3D_APIENTRY ResourceHeapVk::ResourceHeapVk()
    : ref_count     { 1 }
    , name          {}
    , device        {}
    , desc          {}
    , mapped_range  {}
    , mapped_ranges {}
    , mapped_data   {}
    , vkdevice      {}
    , inspfn        {}
    , devpfn        {}
    , device_memory {}
{

}

B3D_APIENTRY ResourceHeapVk::~ResourceHeapVk()
{
    Uninit();
}

BMRESULT
B3D_APIENTRY ResourceHeapVk::Init(DeviceVk* _device, const RESOURCE_HEAP_DESC& _desc)
{
    desc = _desc;
    (device = _device)->AddRef();
    vkdevice = device->GetVkDevice();
    inspfn = &device->GetInstancePFN();
    devpfn = &device->GetDevicePFN();

    B3D_RET_IF_FAILED(AllocateMemory(nullptr));

    return BMRESULT_SUCCEED;
}

BMRESULT 
B3D_APIENTRY ResourceHeapVk::InitForCommitted(DeviceVk* _device, const COMMITTED_RESOURCE_DESC& _desc, IResourceVk* _dedicated_resource)
{
    (device = _device)->AddRef();
    vkdevice = device->GetVkDevice();
    inspfn = &device->GetInstancePFN();
    devpfn = &device->GetDevicePFN();

    VkMemoryDedicatedRequirements dedicated_reqs{ VK_STRUCTURE_TYPE_MEMORY_DEDICATED_REQUIREMENTS };
    VkMemoryRequirements2         reqs{ VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2, &dedicated_reqs };
    
    _dedicated_resource->GetMemoryRequirements(&reqs);

    desc.size_in_bytes      = reqs.memoryRequirements.size;
    desc.alignment          = reqs.memoryRequirements.alignment;
    desc.heap_index         = _desc.heap_index;
    desc.flags              = _desc.heap_flags;
    desc.creation_node_mask = _desc.creation_node_mask;
    desc.visible_node_mask  = _desc.visible_node_mask;

    B3D_RET_IF_FAILED(AllocateMemory(_dedicated_resource));

    return BMRESULT_SUCCEED;
}

BMRESULT 
B3D_APIENTRY ResourceHeapVk::AllocateMemory(IResourceVk* _dedicated_resource)
{
    VkMemoryAllocateInfo ai{ VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
    ai.memoryTypeIndex = desc.heap_index;
    ai.allocationSize = desc.size_in_bytes;

    // pNextチェイン
    const void** last_pnext = &ai.pNext;

    /*Each pNext member of any structure(including this one) in the pNext chain must be either NULL or a pointer to a valid instance of*/

    VkMemoryAllocateFlagsInfo flags_ai{ VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO };
    B3D_RET_IF_FAILED(PrepareVkMemoryAllocateFlagsInfo(last_pnext, &flags_ai));

    // TODO: 不透明キャプチャアドレス
    VkMemoryOpaqueCaptureAddressAllocateInfo opaque_capture_address_ai{ VK_STRUCTURE_TYPE_MEMORY_OPAQUE_CAPTURE_ADDRESS_ALLOCATE_INFO };
    if (false)
    {
        opaque_capture_address_ai.opaqueCaptureAddress;
        last_pnext = util::ConnectPNextChains(last_pnext, opaque_capture_address_ai);
    }

    // TODO: メモリ優先度
    VkMemoryPriorityAllocateInfoEXT priority_ai{ VK_STRUCTURE_TYPE_MEMORY_PRIORITY_ALLOCATE_INFO_EXT };
    if (false)
    {
        priority_ai.priority;
        last_pnext = util::ConnectPNextChains(last_pnext, priority_ai);
    }

    // 専用割り当て
    VkMemoryDedicatedAllocateInfo             dedicated_ai{ VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO };
    VkDedicatedAllocationMemoryAllocateInfoNV dedicated_ai_nv{ VK_STRUCTURE_TYPE_DEDICATED_ALLOCATION_MEMORY_ALLOCATE_INFO_NV };
    if (_dedicated_resource)
    {
        B3D_RET_IF_FAILED(PrepareVkMemoryDedicatedAllocateInfo(_dedicated_resource, last_pnext, &dedicated_ai, &dedicated_ai_nv));
    }

    /* インポート、エクスポート操作 */
    if (desc.flags & RESOURCE_HEAP_FLAG_SHARED_IMPORT_FROM_HANDLE && desc.flags & RESOURCE_HEAP_FLAG_SHARED_EXPORT_TO_HANDLE)
    {
        B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_INITIALIZATION
                          , "RESOURCE_HEAP_FLAG_SHARED_IMPORT_FROM_HANDLE、RESOURCE_HEAP_FLAG_SHARED_EXPORT_TO_HANDLEフラグを同時に含めることはできません。");
        return BMRESULT_FAILED_INVALID_PARAMETER;
    }

    IMPORT_INFOS import_infos{};
    EXPORT_INFOS export_infos{};
    if (desc.flags & RESOURCE_HEAP_FLAG_SHARED_IMPORT_FROM_HANDLE)
    {
        B3D_RET_IF_FAILED(PrepareImportInfos(last_pnext, &import_infos));
    }
    else if (desc.flags & RESOURCE_HEAP_FLAG_SHARED_IMPORT_FROM_HANDLE)
    {
        B3D_RET_IF_FAILED(PrepareExportInfos(last_pnext, &export_infos));
    }


    auto vkr = vkAllocateMemory(device->GetVkDevice(), &ai, B3D_VK_ALLOC_CALLBACKS, &device_memory);
    B3D_RET_IF_FAILED(VKR_TRACE_IF_FAILED(vkr));

    return BMRESULT_SUCCEED;
}

BMRESULT 
B3D_APIENTRY ResourceHeapVk::PrepareVkMemoryAllocateFlagsInfo(const void**& _last_pnext, VkMemoryAllocateFlagsInfo* _info)
{
    // ノードマスク
    int last_node = hlp::GetLastBitIndex(desc.creation_node_mask);
    if (last_node < 0 || 
        last_node >= SCAST<int>(device->GetVkPhysicalDevices().size()))
    {
        B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_INITIALIZATION
                          , "creation_node_maskが無効です。値はデバイス内に含まれる有効なノードのビットインデックスである必要があります。");
        return BMRESULT_FAILED_INVALID_PARAMETER;
    }

    auto&& props = device->GetResourceHeapPropertiesForImpl()[desc.heap_index];
    if (!(props.flags & RESOURCE_HEAP_PROPERTY_FLAG_MULTI_INSTANCE) && hlp::CountBits(desc.creation_node_mask) != 1)
    {
        B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_INITIALIZATION
                          , "creation_node_maskが無効です。指定されたヒープインデックスのプロパティにRESOURCE_HEAP_PROPERTY_FLAG_MULTI_INSTANCEがありません。この場合、creation_node_mask値はデバイス内の有効なノードインデックスを示すの単一のビットである必要があります。");
        return BMRESULT_FAILED_INVALID_PARAMETER;
    }

    /* FIXME: メモリの割り当て時にVK_MEMORY_ALLOCATE_DEVICE_MASK_BITが指定されており、deviceMaskに｢有効なデバイスマスク｣が指定されていれば、
              ヒープのプロパティにVK_MEMORY_HEAP_MULTI_INSTANCE_BITが設定されていなかったとしても、
              メモリに複数のメモリインスタンスを作成出来かもしれない。(Vulkanの仕様ではこの場合の動作は定義されておらず、｢有効なデバイスマスク｣が何なのか(論理デバイス内の単一物理デバイスビットか、それとも複数のビットか)も未定義。)
              なのでMULTI_INSTANCEのフラグの名前から挙動を推測できるよう、このフラグが指定されていない場合は現状creation_node_maskに複数のビットを設定できないようにしている。
              仮に上記の未定義動作が有効であるならば、MULTI_INSTANCE_BITの存在意義がよく分からなくなってしまうが、一応こちらの仕様を変更してVulkanに合わせる。そもそもマルチGPU環境が無いので動作確認が出来ない状態にある。*/
    _info->flags |= VK_MEMORY_ALLOCATE_DEVICE_MASK_BIT;
    _info->deviceMask |= desc.creation_node_mask;

    // デバイスアドレスの対応を確認
    // TODO: 機能サポートを抽象化してこの無駄な分岐を除去する。
    auto&& f = device->GetDeviceAdapter()->GetPhysicalDeviceData().features_chain;
    if (f.features12)
    {
        if (f.features12->buffer_device_address_features->bufferDeviceAddress)
            _info->flags |= VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
    }
    else if (f.vulkan12_features)
    {
        if (f.vulkan12_features->bufferDeviceAddress)
            _info->flags |= VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
    }

    _last_pnext = util::ConnectPNextChains(_last_pnext, *_info);

    return BMRESULT_SUCCEED;
}

BMRESULT 
B3D_APIENTRY ResourceHeapVk::PrepareVkMemoryDedicatedAllocateInfo(IResourceVk* _dedicated_resource, const void**& _last_pnext, VkMemoryDedicatedAllocateInfo* _info, VkDedicatedAllocationMemoryAllocateInfoNV* _infonv)
{
    _dedicated_resource->SetDedicatedAllocationInfo(_info);
    _last_pnext = util::ConnectPNextChains(_last_pnext, *_info);

    B3D_UNREFERENCED(_infonv);
    //else if (false)// Vkulkan 1.1から正式なインターフェースとしてVkMemoryDedicatedAllocateInfoが追加
    //{
    // _last_pnext = util::ConnectPNextChains(_last_pnext, *_infonv);
    //}

    return BMRESULT_SUCCEED;
}

BMRESULT 
B3D_APIENTRY ResourceHeapVk::PrepareImportInfos(const void**& _last_pnext, IMPORT_INFOS* _infos)
{
    auto&& import_fd           = _infos->import_fd;
    auto&& import_host_pointer = _infos->import_host_pointer;
#if B3D_PLATFORM_IS_USE_WINDOWS
    auto&& import_w32_khr      = _infos->import_w32_khr;
    auto&& import_w32_nv       = _infos->import_w32_nv;
#elif B3D_PLATFORM_IS_USE_ANDROID
    auto&& import_ahd  { VK_STRUCTURE_TYPE_IMPORT_ANDROID_HARDWARE_BUFFER_INFO_ANDROID };
#endif

    if (false)
    {
        // TODO: 
        import_fd.handleType;
        import_fd.fd;
        _last_pnext = util::ConnectPNextChains(_last_pnext, import_fd);
    }
    else if (false)
    {
        // TODO: 
        import_host_pointer.pHostPointer;
        import_host_pointer.handleType;
        _last_pnext = util::ConnectPNextChains(_last_pnext, import_host_pointer);
    }
#if B3D_PLATFORM_IS_USE_WINDOWS
    // pNextチェーンにVkImportMemoryWin32HandleInfoKHR構造が含まれている場合は、VkImportMemoryWin32HandleInfoNV構造が含まれていてはなりません。
    else if (false)
    {
        // TODO:
        import_w32_khr.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D12_HEAP_BIT;
        import_w32_khr.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D12_RESOURCE_BIT;
        import_w32_khr.handle     = NULL;
        import_w32_khr.name       = nullptr;
        _last_pnext = util::ConnectPNextChains(_last_pnext, import_w32_khr);
    }
    else if (false)
    {
        // TODO:
        _last_pnext = util::ConnectPNextChains(_last_pnext, import_w32_nv);
    }
#elif B3D_PLATFORM_IS_USE_ANDROID
    if (false)
    {
        // TODO:
        _last_pnext = util::ConnectPNextChains(_last_pnext, import_ahd);
    }
#endif

    return BMRESULT_SUCCEED;
}

BMRESULT 
B3D_APIENTRY ResourceHeapVk::PrepareExportInfos(const void**& _last_pnext, EXPORT_INFOS* _infos)
{
    auto&& export_memory_ai    = _infos->export_memory_ai;
    auto&& export_memory_ai_nv = _infos->export_memory_ai_nv;
#if B3D_PLATFORM_IS_USE_WINDOWS
    auto&& export_w32_khr      = _infos->export_w32_khr;
    auto&& export_w32_nv       = _infos->export_w32_nv;
#endif

    if (false)
    {
        // TODO: 
        export_memory_ai.handleTypes;
        _last_pnext = util::ConnectPNextChains(_last_pnext, export_memory_ai);
    }
    else if (false)
    {
        // TODO: 
        export_memory_ai_nv.handleTypes;
        _last_pnext = util::ConnectPNextChains(_last_pnext, export_memory_ai_nv);
    }
#if B3D_PLATFORM_IS_USE_WINDOWS
    // pNextチェーンにVkExportMemoryAllocateInfo構造が含まれている場合は、VkExportMemoryAllocateInfoNVまたはVkExportMemoryWin32HandleInfoNV構造を含まないようにする必要があります。
    else if (false)
    {
        // TODO:
        _last_pnext = util::ConnectPNextChains(_last_pnext, export_w32_khr);
    }
    else if (false)
    {
        // TODO:
        _last_pnext = util::ConnectPNextChains(_last_pnext, export_w32_nv);
    }
#endif

    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY ResourceHeapVk::Uninit()
{
    name.reset();
    desc = {};

    if (device_memory)
        vkFreeMemory(device->GetVkDevice(), device_memory, B3D_VK_ALLOC_CALLBACKS);
    device_memory = VK_NULL_HANDLE;

    inspfn = nullptr;
    devpfn = nullptr;

    hlp::SafeRelease(device);
}

BMRESULT
B3D_APIENTRY ResourceHeapVk::Create(DeviceVk* _device, const RESOURCE_HEAP_DESC& _desc, ResourceHeapVk** _dst)
{
    util::Ptr<ResourceHeapVk> ptr;
    ptr.Attach(B3DCreateImplementationClass(ResourceHeapVk));
    B3D_RET_IF_FAILED(ptr->Init(_device, _desc));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

BMRESULT 
B3D_APIENTRY ResourceHeapVk::CreateForCommitted(DeviceVk* _device, const COMMITTED_RESOURCE_DESC& _desc, IResourceVk* _dedicated_resource, ResourceHeapVk** _dst)
{
    util::Ptr<ResourceHeapVk> ptr;
    ptr.Attach(B3DCreateImplementationClass(ResourceHeapVk));
    B3D_RET_IF_FAILED(ptr->InitForCommitted(_device, _desc, _dedicated_resource));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY ResourceHeapVk::AddRef()
{
    ++ref_count;
    B3D_REFCOUNT_DEBUG(ref_count);
}

uint32_t
B3D_APIENTRY ResourceHeapVk::Release()
{
    B3D_REFCOUNT_DEBUG(ref_count - 1);
    auto count = --ref_count;
    if (count == 0)
        B3DDestroyImplementationClass(this);

    return count;
}

uint32_t
B3D_APIENTRY ResourceHeapVk::GetRefCount() const
{
    return ref_count;
}

const char*
B3D_APIENTRY ResourceHeapVk::GetName() const
{
    return name ? name->c_str() : nullptr;
}

BMRESULT
B3D_APIENTRY ResourceHeapVk::SetName(const char* _name)
{
    if (!util::IsEnabledDebug(this))
        return BMRESULT_FAILED;

    if (device_memory)
        B3D_RET_IF_FAILED(device->SetVkObjectName(device_memory, _name));

    if (name && !_name)
        name.reset();
    else
        name = B3DMakeUniqueArgs(util::NameableObjStr, _name);

    return BMRESULT_SUCCEED;
}

IDevice*
B3D_APIENTRY ResourceHeapVk::GetDevice() const
{
    return device;
}

const VkAllocationCallbacks*
B3D_APIENTRY ResourceHeapVk::GetVkAllocationCallbacks() const
{
    return device->GetVkAllocationCallbacks();
}

const InstancePFN&
B3D_APIENTRY ResourceHeapVk::GetIsntancePFN() const
{
    return *inspfn;
}

const DevicePFN&
B3D_APIENTRY ResourceHeapVk::GetDevicePFN() const
{
    return *devpfn;
}

const RESOURCE_HEAP_DESC&
B3D_APIENTRY ResourceHeapVk::GetDesc() const
{
    return desc;
}

BMRESULT
B3D_APIENTRY ResourceHeapVk::Map(const MAPPED_RANGE* _range_to_map)
{
    if (mapped_data)
        return BMRESULT_FAILED_INVALID_CALL;

    // WARNING: マルチインスタンスメモリではMap不可

    /* 範囲の先頭をVkPhysicalDeviceLimits::nonCoherentAtomSizeの最も近い倍数に切り捨てる必要があります。
       範囲の終了をVkPhysicalDeviceLimits::nonCoherentAtomSizeの最も近い倍数に切り上げます。*/
    // NOTE: Mapに限ったことではないがVulkan APIの有効性チェックはベンダーによって異なり、厳格に行われない可能性があるので、実際に遭遇した場合は対策を考える必要がある。
    if (_range_to_map)
    {
        auto alignment = device->GetDeviceAdapter()->GetPhysicalDeviceData().properties2.properties.limits.nonCoherentAtomSize;
        VkDeviceSize offset = hlp::AlignDown(_range_to_map->offset, alignment);
        VkDeviceSize size = std::clamp(hlp::AlignUp(_range_to_map->size, alignment), 0ull, desc.size_in_bytes);
        
        auto vkr = vkMapMemory(vkdevice, device_memory, offset, size, /*reserved*/0, &mapped_data);
        B3D_RET_IF_FAILED(VKR_TRACE_IF_FAILED(vkr));

        mapped_range = { offset ,size };// 成功後に設定
    }
    else
    {
        auto vkr = vkMapMemory(vkdevice, device_memory, 0, VK_WHOLE_SIZE, /*reserved*/0, &mapped_data);
        B3D_RET_IF_FAILED(VKR_TRACE_IF_FAILED(vkr));

        mapped_range = { 0, desc.size_in_bytes };// 成功後に設定
    }

    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY ResourceHeapVk::GetMappedData(MAPPED_RANGE* _mapped_range, void** _dst_mapped_data) const
{
    if (!mapped_data)
        return BMRESULT_FAILED_INVALID_CALL;

    *_mapped_range = mapped_range;
    *_dst_mapped_data = mapped_data;

    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY ResourceHeapVk::FlushMappedRanges(uint32_t _num_ranges, const MAPPED_RANGE* _ranges)
{
    if (!mapped_data)
        return BMRESULT_FAILED_INVALID_CALL;

    if (mapped_ranges.size() < _num_ranges)
        mapped_ranges.resize(_num_ranges, { VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE, nullptr, device_memory });

    auto alignment = device->GetDeviceAdapter()->GetPhysicalDeviceData().properties2.properties.limits.nonCoherentAtomSize;
    auto mapped_ranges_data = mapped_ranges.data();
    if (_num_ranges == 1 && _ranges == nullptr)
    {
        mapped_ranges_data[0].offset = 0;
        mapped_ranges_data[0].size = VK_WHOLE_SIZE;
    }
    else
    {
        for (uint32_t i = 0; i < _num_ranges; i++)
        {
            mapped_ranges_data[i].offset = hlp::AlignDown(_ranges[i].offset, alignment);
            mapped_ranges_data[i].size = std::clamp(hlp::AlignUp(_ranges[i].size, alignment), 0ull, desc.size_in_bytes);
        }
    }

    auto vkr = vkFlushMappedMemoryRanges(vkdevice, _num_ranges, mapped_ranges_data);
    B3D_RET_IF_FAILED(VKR_TRACE_IF_FAILED(vkr));

    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY ResourceHeapVk::InvalidateMappedRanges(uint32_t _num_ranges, const MAPPED_RANGE* _ranges)
{
    if (!mapped_data)
        return BMRESULT_FAILED_INVALID_CALL;

    if (mapped_ranges.size() < _num_ranges)
        mapped_ranges.resize(_num_ranges, { VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE, nullptr, device_memory });

    auto alignment = device->GetDeviceAdapter()->GetPhysicalDeviceData().properties2.properties.limits.nonCoherentAtomSize;
    auto mapped_ranges_data = mapped_ranges.data();
    if (_num_ranges == 1 && _ranges == nullptr)
    {
        mapped_ranges_data[0].offset = 0;
        mapped_ranges_data[0].size = VK_WHOLE_SIZE;
    }
    else
    {
        for (uint32_t i = 0; i < _num_ranges; i++)
        {
            mapped_ranges_data[i].offset = hlp::AlignDown(_ranges[i].offset, alignment);
            mapped_ranges_data[i].size = std::clamp(hlp::AlignUp(_ranges[i].size, alignment), 0ull, desc.size_in_bytes);
        }
    }

    auto vkr = vkInvalidateMappedMemoryRanges(vkdevice, _num_ranges, mapped_ranges_data);
    B3D_RET_IF_FAILED(VKR_TRACE_IF_FAILED(vkr));

    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY ResourceHeapVk::Unmap(const MAPPED_RANGE* _used_range)
{
    if (!mapped_data)
        return BMRESULT_FAILED_INVALID_CALL;

    B3D_UNREFERENCED(_used_range);
    vkUnmapMemory(vkdevice, device_memory);
    mapped_data = nullptr;

    return BMRESULT_SUCCEED;
}

VkDeviceMemory
B3D_APIENTRY ResourceHeapVk::ResourceHeapVk::GetVkDeviceMemory() const
{
    return device_memory;
}


}// namespace buma3d
