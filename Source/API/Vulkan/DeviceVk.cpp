#include "Buma3DPCH.h"
#include "DeviceVk.h"

namespace buma3d
{

namespace /*anonymous*/
{

inline float GetQueuePriority(COMMAND_QUEUE_PRIORITY _priority, uint32_t _discrete_queue_priorities)
{
    if (_priority == COMMAND_QUEUE_PRIORITY_DEFAULT)
    {
        return 0.f;
    }
    else
    {
        // _discrete_queue_priorities が2の場合、MEDIUMとHIGHでのみ区別します。MEDIUMの優先度もDEFAULTと同じ0にします。
        if (_discrete_queue_priorities == 2)
            return SCAST<float>(_priority) - 1.f;
        else
            return SCAST<float>(_priority) / SCAST<float>(COMMAND_QUEUE_PRIORITY_HIGH);
    }
}

inline void GetQueuePriorities(uint32_t _num_priorities, const COMMAND_QUEUE_PRIORITY* _src_priorities, uint32_t _discrete_queue_priorities, util::DyArray<float>* _result)
{
    _result->resize(_num_priorities);
    for (uint32_t i = 0; i < _num_priorities; i++)
        _result->data()[i] = GetQueuePriority(_src_priorities[i], _discrete_queue_priorities);
}

// VkDeviceQueueGlobalPriorityCreateInfoEXTを指定せずに作成されたキューは、デフォルトでVK_QUEUE_GLOBAL_PRIORITY_MEDIUM_EXTになります。
inline VkQueueGlobalPriorityEXT GetQueueGlobalPriority(COMMAND_QUEUE_FLAGS _flags)
{
    return (_flags & COMMAND_QUEUE_FLAG_PRIORITY_GLOBAL_REALTIME) ? VK_QUEUE_GLOBAL_PRIORITY_REALTIME_EXT : VK_QUEUE_GLOBAL_PRIORITY_MEDIUM_EXT;
}


}// namespace /*anonymous*/



#pragma region IVulkanDebugNameSetter

class DeviceVk::VulkanDebugNameSetterDebugUtils : public IVulkanDebugNameSetter
{
public:
        VulkanDebugNameSetterDebugUtils(DeviceVk* _owner)
        : owner         { _owner }
        , vkdevice      { _owner->GetVkDevice() }
        , inspfn        { &_owner->GetInstancePFN() }
        , devpfn        { &_owner->GetDevicePFN() }
        , info          { VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT }
        , label_info    { VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT }
    {
    }

    ~VulkanDebugNameSetterDebugUtils()
    {
    }

    void InsertMarker(VkQueue         _queue, const char* _marker_name, const COLOR4* _color) override { SetLabelInfos(_marker_name, _color); devpfn->vkQueueInsertDebugUtilsLabelEXT(_queue, &label_info); }
    void InsertMarker(VkCommandBuffer _cmd  , const char* _marker_name, const COLOR4* _color) override { SetLabelInfos(_marker_name, _color); devpfn->vkCmdInsertDebugUtilsLabelEXT  (_cmd  , &label_info); }
    void BeginMarker (VkQueue         _queue, const char* _marker_name, const COLOR4* _color) override { SetLabelInfos(_marker_name, _color); devpfn->vkQueueBeginDebugUtilsLabelEXT (_queue, &label_info); }
    void BeginMarker (VkCommandBuffer _cmd  , const char* _marker_name, const COLOR4* _color) override { SetLabelInfos(_marker_name, _color); devpfn->vkCmdBeginDebugUtilsLabelEXT   (_cmd  , &label_info); }
    void EndMarker   (VkQueue         _queue)                                                 override { devpfn->vkQueueEndDebugUtilsLabelEXT(_queue); }
    void EndMarker   (VkCommandBuffer _cmd)                                                   override { devpfn->vkCmdEndDebugUtilsLabelEXT(_cmd); }

    BMRESULT SetName(VkObjectType _object_type, uint64_t _object_handle, const char* _object_name) override
    {
        info.objectType     = _object_type;
        info.objectHandle   = _object_handle;
        info.pObjectName    = _object_name;
        auto vkr = devpfn->vkSetDebugUtilsObjectNameEXT(vkdevice, &info);
        return VKR_TRACE_IF_FAILED(vkr);
    }

private:
    void SetLabelInfos(const char* _marker_name, const COLOR4* _color)
    {
        if (_color)
            memcpy(label_info.color, _color, sizeof(COLOR4));
        else
            std::fill(label_info.color, label_info.color + 4, 0.f);
        label_info.pLabelName = _marker_name;
    }

private:
    DeviceVk*                       owner;
    VkDevice                        vkdevice;
    const InstancePFN*              inspfn;
    const DevicePFN*                devpfn;
    VkDebugUtilsObjectNameInfoEXT   info;
    VkDebugUtilsLabelEXT            label_info;

};

class DeviceVk::VulkanDebugNameSetterDebugReport : public IVulkanDebugNameSetter
{
public:
    VulkanDebugNameSetterDebugReport(DeviceVk* _owner)
        : owner         { _owner }
        , vkdevice      { _owner->GetVkDevice() }
        , inspfn        { &_owner->GetInstancePFN() }
        , devpfn        { &_owner->GetDevicePFN() }
        , info          { VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT }
        , marker_info   { VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT }
    {
    }

    ~VulkanDebugNameSetterDebugReport()
    {
    }

    BMRESULT SetName(VkObjectType _object_type, uint64_t _object_handle, const char* _object_name) override
    {
        info.objectType  = ConvertToVkDebugReportObjectTypeEXT(_object_type);
        info.object      = _object_handle;
        info.pObjectName = _object_name;
        auto vkr = devpfn->vkDebugMarkerSetObjectNameEXT(vkdevice, &info);
        return VKR_TRACE_IF_FAILED(vkr);
    }

    void InsertMarker(VkCommandBuffer _cmd  , const char* _marker_name, const COLOR4* _color) override { SetMarkerInfos(_marker_name, _color); devpfn->vkCmdDebugMarkerInsertEXT(_cmd, &marker_info); }
    void BeginMarker (VkCommandBuffer _cmd  , const char* _marker_name, const COLOR4* _color) override { SetMarkerInfos(_marker_name, _color); devpfn->vkCmdDebugMarkerBeginEXT (_cmd, &marker_info); }
    void EndMarker   (VkCommandBuffer _cmd)                                                   override { devpfn->vkCmdEndDebugUtilsLabelEXT(_cmd); }

    // VK_EXT_debug_reportの場合、VkQueue用のマーカー設定APIは存在しません。
    void InsertMarker(VkQueue _queue, const char* _marker_name, const COLOR4* _color) override { B3D_UNREFERENCED(_queue, _marker_name, _color); }
    void BeginMarker (VkQueue _queue, const char* _marker_name, const COLOR4* _color) override { B3D_UNREFERENCED(_queue, _marker_name, _color); }
    void EndMarker   (VkQueue _queue)                                                 override { B3D_UNREFERENCED(_queue); }


private:
    static VkDebugReportObjectTypeEXT ConvertToVkDebugReportObjectTypeEXT(VkObjectType _type)
    {
        switch (_type)
        {
        case VK_OBJECT_TYPE_UNKNOWN                         : return VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT;
        case VK_OBJECT_TYPE_INSTANCE                        : return VK_DEBUG_REPORT_OBJECT_TYPE_INSTANCE_EXT;
        case VK_OBJECT_TYPE_PHYSICAL_DEVICE                 : return VK_DEBUG_REPORT_OBJECT_TYPE_PHYSICAL_DEVICE_EXT;
        case VK_OBJECT_TYPE_DEVICE                          : return VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT;
        case VK_OBJECT_TYPE_QUEUE                           : return VK_DEBUG_REPORT_OBJECT_TYPE_QUEUE_EXT;
        case VK_OBJECT_TYPE_SEMAPHORE                       : return VK_DEBUG_REPORT_OBJECT_TYPE_SEMAPHORE_EXT;
        case VK_OBJECT_TYPE_COMMAND_BUFFER                  : return VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT;
        case VK_OBJECT_TYPE_FENCE                           : return VK_DEBUG_REPORT_OBJECT_TYPE_FENCE_EXT;
        case VK_OBJECT_TYPE_DEVICE_MEMORY                   : return VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_MEMORY_EXT;
        case VK_OBJECT_TYPE_BUFFER                          : return VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT;
        case VK_OBJECT_TYPE_IMAGE                           : return VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT;
        case VK_OBJECT_TYPE_EVENT                           : return VK_DEBUG_REPORT_OBJECT_TYPE_EVENT_EXT;
        case VK_OBJECT_TYPE_QUERY_POOL                      : return VK_DEBUG_REPORT_OBJECT_TYPE_QUERY_POOL_EXT;
        case VK_OBJECT_TYPE_BUFFER_VIEW                     : return VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_VIEW_EXT;
        case VK_OBJECT_TYPE_IMAGE_VIEW                      : return VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_VIEW_EXT;
        case VK_OBJECT_TYPE_SHADER_MODULE                   : return VK_DEBUG_REPORT_OBJECT_TYPE_SHADER_MODULE_EXT;
        case VK_OBJECT_TYPE_PIPELINE_CACHE                  : return VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_CACHE_EXT;
        case VK_OBJECT_TYPE_PIPELINE_LAYOUT                 : return VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_LAYOUT_EXT;
        case VK_OBJECT_TYPE_RENDER_PASS                     : return VK_DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT;
        case VK_OBJECT_TYPE_PIPELINE                        : return VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT;
        case VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT           : return VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT_EXT;
        case VK_OBJECT_TYPE_SAMPLER                         : return VK_DEBUG_REPORT_OBJECT_TYPE_SAMPLER_EXT;
        case VK_OBJECT_TYPE_DESCRIPTOR_POOL                 : return VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_POOL_EXT;
        case VK_OBJECT_TYPE_DESCRIPTOR_SET                  : return VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_EXT;
        case VK_OBJECT_TYPE_FRAMEBUFFER                     : return VK_DEBUG_REPORT_OBJECT_TYPE_FRAMEBUFFER_EXT;
        case VK_OBJECT_TYPE_COMMAND_POOL                    : return VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_POOL_EXT;
        case VK_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION        : return VK_DEBUG_REPORT_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION_EXT;
        case VK_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE      : return VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE_EXT;
        case VK_OBJECT_TYPE_SURFACE_KHR                     : return VK_DEBUG_REPORT_OBJECT_TYPE_SURFACE_KHR_EXT;
        case VK_OBJECT_TYPE_SWAPCHAIN_KHR                   : return VK_DEBUG_REPORT_OBJECT_TYPE_SWAPCHAIN_KHR_EXT;
        case VK_OBJECT_TYPE_DISPLAY_KHR                     : return VK_DEBUG_REPORT_OBJECT_TYPE_DISPLAY_KHR_EXT;
        case VK_OBJECT_TYPE_DISPLAY_MODE_KHR                : return VK_DEBUG_REPORT_OBJECT_TYPE_DISPLAY_MODE_KHR_EXT;
        case VK_OBJECT_TYPE_DEBUG_REPORT_CALLBACK_EXT       : return VK_DEBUG_REPORT_OBJECT_TYPE_DEBUG_REPORT_CALLBACK_EXT_EXT;
        case VK_OBJECT_TYPE_DEBUG_UTILS_MESSENGER_EXT       : return VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT;
        case VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_KHR      : return VK_DEBUG_REPORT_OBJECT_TYPE_ACCELERATION_STRUCTURE_KHR_EXT;
        case VK_OBJECT_TYPE_VALIDATION_CACHE_EXT            : return VK_DEBUG_REPORT_OBJECT_TYPE_VALIDATION_CACHE_EXT_EXT;
        case VK_OBJECT_TYPE_PERFORMANCE_CONFIGURATION_INTEL : return VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT;
        case VK_OBJECT_TYPE_DEFERRED_OPERATION_KHR          : return VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT;
        case VK_OBJECT_TYPE_INDIRECT_COMMANDS_LAYOUT_NV     : return VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT;
        case VK_OBJECT_TYPE_PRIVATE_DATA_SLOT_EXT           : return VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT;

        default:
            return VkDebugReportObjectTypeEXT(-1);
        }
    }

    void SetMarkerInfos(const char* _marker_name, const COLOR4* _color)
    {
        if (_color)
            memcpy(marker_info.color, _color, sizeof(COLOR4));
        else
            std::fill(marker_info.color, marker_info.color + 4, 0.f);
        marker_info.pMarkerName = _marker_name;
    }

private:
    DeviceVk*                       owner;
    VkDevice                        vkdevice;
    const InstancePFN*              inspfn;
    const DevicePFN*                devpfn;
    VkDebugMarkerObjectNameInfoEXT  info;
    VkDebugMarkerMarkerInfoEXT      marker_info;

};

#pragma endregion IVulkanDebugNameSetter


B3D_APIENTRY DeviceVk::DeviceVk()
    : ref_count                 { 1 }
    , name                      {}
    , desc                      {}
    , desc_data                 {}
    , adapter                   {}
    , node_mask                 {}
    , heap_props                {}
    , primary_physical_device   {}
    , physical_devices          {}
    , instance                  {}
    , device                    {}
    , alloc_callbacks           {}
    , all_layers                {}
    , all_extensions            {}
    , enabled_layers            {}
    , enabled_extensions        {}
    , device_data               {}
    , inspfn                    {}
    , devpfn                    {}
    , queue_data                {}
    , format_props              {}
    , format_comapbility        {}
    , zero_binding_layout       {}
    , debug_name_setter         {}
{

}

B3D_APIENTRY DeviceVk::~DeviceVk()
{
    Uninit();
}

BMRESULT
B3D_APIENTRY DeviceVk::Init(DeviceFactoryVk* _factory, const DEVICE_DESC& _desc)
{
    CopyDesc(_desc);

    (factory = _factory)->AddRef();

    // アダプタ、物理デバイス、をキャッシュ
    (adapter = _desc.adapter->As<DeviceAdapterVk>())->AddRef();
    if (adapter->GetDesc().node_count == 1)
    {
        primary_physical_device = adapter->GetVkPhysicalDevice();
        physical_devices.push_back(primary_physical_device);
    }
    else
    {
        auto pdgp = adapter->GetVkPhysicalDeviceGroupProperties();
        primary_physical_device = pdgp->physicalDevices[0];
        physical_devices.resize(pdgp->physicalDeviceCount);
        auto pds_data = physical_devices.data();
        for (uint32_t i = 0; i < pdgp->physicalDeviceCount; i++)
            pds_data[i] = pdgp->physicalDevices[i];
    }

    // instance、callbackをキャッシュ
    instance        = factory->GetVkInstance();
    alloc_callbacks = factory->GetVkAllocationCallbacks();
    inspfn          = &factory->GetInstancePFN();
    
    B3D_RET_IF_FAILED(CreateLogicalDevice());

    devpfn = B3DMakeUniqueArgs(DevicePFN, this);

    B3D_RET_IF_FAILED(CreateDebugNameSetter());

    // QueueVkを作成
    B3D_RET_IF_FAILED(CreateCommandQueueVk());

    // 追加のプロパティを取得
    B3D_RET_IF_FAILED(GetDeviceLevelProperties());

    MakeResourceHeapProperties();

    // フォーマットプロパティ
    format_props       = B3DMakeUnique(util::VulkanFormatProperties);
    format_comapbility = B3DMakeUnique(util::FormatCompatibilityChecker);
    format_props->Init(instance, primary_physical_device, device);
    format_comapbility->Init(*format_props);

    // 汎用のノードマスクを作成
    auto node = adapter->GetDesc().node_count;
    for (uint32_t i = 0; i < node; i++)
        hlp::SetBit(node_mask, node);

    // バインドカウントがゼロのディスクリプタセットレイアウトを作成
    B3D_RET_IF_FAILED(CreateZeroBindingDescriptorSetLayout());

    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY DeviceVk::CopyDesc(const DEVICE_DESC& _desc)
{
    desc.num_queue_create_descs = _desc.num_queue_create_descs;
    desc.flags                  = _desc.flags;
    desc.adapter                = _desc.adapter;

    // キューデスク配列をコピー
    {
        desc_data.queue_create_descs.resize(_desc.num_queue_create_descs);
        desc_data.qcdescs_priorities.resize(_desc.num_queue_create_descs);
        desc_data.qcdescs_node_masks.resize(_desc.num_queue_create_descs);
        for (uint32_t i = 0; i < _desc.num_queue_create_descs; i++)
        {
            auto&& src_qcdesc = _desc.queue_create_descs[i];
            auto&& dst_qcdesc = desc_data.queue_create_descs.data()[i];
            dst_qcdesc.type       = src_qcdesc.type;
            dst_qcdesc.flags      = src_qcdesc.flags;
            dst_qcdesc.num_queues = src_qcdesc.num_queues;
            
            // ノードマスク、プライオリティ配列をコピー
            auto&& qcdescs_priorities = desc_data.qcdescs_priorities.data()[i];
            auto&& qcdescs_node_masks = desc_data.qcdescs_node_masks.data()[i];
            qcdescs_priorities.resize(src_qcdesc.num_queues);
            qcdescs_node_masks.resize(src_qcdesc.num_queues);
            util::MemCopyArray(qcdescs_priorities.data(), src_qcdesc.priorities, src_qcdesc.num_queues);
            util::MemCopyArray(qcdescs_node_masks.data(), src_qcdesc.node_masks, src_qcdesc.num_queues);
            dst_qcdesc.priorities = qcdescs_priorities.data();
            dst_qcdesc.node_masks = qcdescs_node_masks.data();
        }
        desc.queue_create_descs = desc_data.queue_create_descs.data();
    }
}

void 
B3D_APIENTRY DeviceVk::GetDeviceLayerNames(util::DyArray<util::SharedPtr<util::String>>* _dst_all_layer_names)
{
    uint32_t num_props = 0;
    // デバイスレイヤプロパティを取得
    vkEnumerateDeviceLayerProperties(primary_physical_device, &num_props, nullptr);
    util::DyArray<VkLayerProperties> layer_props(num_props);
    vkEnumerateDeviceLayerProperties(primary_physical_device, &num_props, layer_props.data());

    for (auto& i : layer_props)
        _dst_all_layer_names->emplace_back(B3DMakeSharedArgs(util::String, i.layerName));

    if constexpr (IS_ENABLE_DEBUG_OUTPUT)
    {
        if (util::IsEnabledDebug(this))
        {
            for (auto& i : layer_props)
            {
                util::StringStream ss;
                ss << "VkLayerProperties(device level)" << std::endl;
                ss << "\tlayerName            : " << i.layerName                                    << std::endl;
                ss << "\tspecVersion          : " << util::GetVulkanVersionString(i.specVersion)    << std::endl;
                ss << "\timplementationVersion: " << i.implementationVersion                        << std::endl;
                ss << "\tdescription          : " << i.description                                  << std::endl;
                ss << std::endl;
                B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_INFO, DEBUG_MESSAGE_CATEGORY_FLAG_MISCELLANEOUS
                                  , ss.str().c_str());
            }
        }
    }
}

void 
B3D_APIENTRY DeviceVk::GetDeviceExtensionNames(const util::DyArray<util::SharedPtr<util::String>>& _all_layer_names, util::DyArray<util::SharedPtr<util::String>>* _dst_all_ext_names)
{

    for (auto& i : _all_layer_names)
    {
        uint32_t num_props = 0;
        // デバイス拡張プロパティを取得
        vkEnumerateDeviceExtensionProperties(primary_physical_device, i->c_str(), &num_props, nullptr);
        util::DyArray<VkExtensionProperties> ext_props(num_props);
        vkEnumerateDeviceExtensionProperties(primary_physical_device, i->c_str(), &num_props, ext_props.data());

        for (auto& i_ext : ext_props)
            _dst_all_ext_names->emplace_back(B3DMakeSharedArgs(util::String, i_ext.extensionName));

        if constexpr (IS_ENABLE_DEBUG_OUTPUT)
        {
            if (util::IsEnabledDebug(this))
            {
                util::StringStream ss;
                ss << "VkExtensionProperties(device level)" << std::endl;
                ss << "layerName: " << i << std::endl;
                if (ext_props.empty())
                {
                    ss << "\tNot found" << std::endl;
                }
                else
                {
                    for (auto& i_ext : ext_props)
                    {
                        ss << "\textensionName: " << i_ext.extensionName << std::endl;
                        ss << "\tspecVersion  : " << util::GetVulkanVersionString(i_ext.specVersion) << std::endl;
                    }
                }
                ss << std::endl;
                B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_INFO, DEBUG_MESSAGE_CATEGORY_FLAG_MISCELLANEOUS
                                  , ss.str().c_str());
            }
        }
    }

    // pLayerNameパラメータがNULLの場合、Vulkan実装または暗黙的に有効化されたレイヤーによって提供される拡張機能のみが返されます。
    uint32_t num_implicit_ext_props = 0;
    vkEnumerateDeviceExtensionProperties(primary_physical_device, nullptr, &num_implicit_ext_props, nullptr);
    util::DyArray<VkExtensionProperties> implicit_ext_props(num_implicit_ext_props);
    vkEnumerateDeviceExtensionProperties(primary_physical_device, nullptr, &num_implicit_ext_props, implicit_ext_props.data());

    // 暗黙で有効の拡張名も追加
    for (auto& it_props : implicit_ext_props)
        _dst_all_ext_names->emplace_back(B3DMakeSharedArgs(util::String, it_props.extensionName));

    if constexpr (IS_ENABLE_DEBUG_OUTPUT)
    {
        if (util::IsEnabledDebug(this))
        {
            util::StringStream ss;
            ss << "Implicit(nullptr) layer properties(device level)" << std::endl;
            for (auto& i : implicit_ext_props)
            {
                ss << "\textensionName: " << i.extensionName << std::endl;
                ss << "\tspecVersion  : " << i.specVersion << std::endl;
            }
            ss << std::endl;
            B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_INFO, DEBUG_MESSAGE_CATEGORY_FLAG_MISCELLANEOUS
                              , ss.str().c_str());
        }
    }
}

void 
B3D_APIENTRY DeviceVk::GetQueueFamilyProperties(util::DyArray<VkQueueFamilyProperties2>* _dst_qf_props, util::DyArray<VkQueueFamilyCheckpointPropertiesNV>* _dst_qf_checkpoint_props)
{
    uint32_t num_qf_props = 0;
    vkGetPhysicalDeviceQueueFamilyProperties2(primary_physical_device, &num_qf_props, nullptr);
    _dst_qf_props->resize(num_qf_props, { VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2 });
    if (_dst_qf_checkpoint_props)
    {
        _dst_qf_checkpoint_props->resize(num_qf_props, { VK_STRUCTURE_TYPE_QUEUE_FAMILY_CHECKPOINT_PROPERTIES_NV });
        queue_data.queue_family_indices.resize(num_qf_props);
        for (uint32_t i = 0; i < num_qf_props; i++)
        {
            _dst_qf_props->data()[i].pNext = &_dst_qf_checkpoint_props->data()[i];
            queue_data.queue_family_indices.data()[i] = i;// pQueueFamilyIndeciesをパラメータに持つ構造体に使用されます。
        }
    }
    vkGetPhysicalDeviceQueueFamilyProperties2(primary_physical_device, &num_qf_props, _dst_qf_props->data());
}

int 
B3D_APIENTRY DeviceVk::GetQueueFamilyIndex(const util::DyArray<VkQueueFamilyProperties2>& _qf_props, COMMAND_TYPE _type, bool* _is_enable_sparse_bind, bool* _is_enable_protected)
{
    // _qf_propsを走査して_typeに対応するフラグをもつプロパティを探す
    // GRAPHICS | SPARSE と GRAPHICS のようなケースはサポートしない
    for (int i = 0, size = (int)_qf_props.size(); i < size; i++)
    {
        if (_type == util::GetB3DCommandType(_qf_props.data()[i].queueFamilyProperties.queueFlags, _is_enable_sparse_bind, _is_enable_protected))
            return i;
    }
    return -1;
}

BMRESULT
B3D_APIENTRY DeviceVk::CreateDeviceQueueCreateInfos()
{
    // 同じtypeが入っていないかチェック
    for (uint32_t i_type = 0; i_type < uint32_t(COMMAND_TYPE_VIDEO_ENCODE + 1); i_type++)
    {
        bool is_found = false;
        for (uint32_t i_qcdesc = 0; i_qcdesc < desc.num_queue_create_descs; i_qcdesc++)
        {
            if (desc.queue_create_descs[i_qcdesc].type == (COMMAND_TYPE)i_type)
            {
                if (is_found)
                    return BMRESULT_FAILED_INVALID_PARAMETER;
                else
                    is_found = true;
            }
        }
    }
    // reserveによりcapacityを超えない限り要素追加による以前の参照の無効化は起こりません。
    queue_data.families_data.create_infos.reserve(desc.num_queue_create_descs);
    queue_data.families_data.global_priority_create_infos.reserve(desc.num_queue_create_descs);

    uint32_t discrete_queue_priorities = adapter->GetPhysicalDeviceData().properties2.properties.limits.discreteQueuePriorities;
    // 特定のキューファミリ作成情報を設定する
    auto CreateDeviceQueueCreateInfo = [discrete_queue_priorities](DEVICE_QUEUE_FAMILIES_DATA& _families_data, uint32_t _qf_index, const COMMAND_QUEUE_CREATE_DESC& _create_desc, DEVICE_QUEUE_FAMILY_DATA* _data)
    {
        // 作成数をチェック
        if (_data->props->queueFamilyProperties.queueCount < _create_desc.num_queues)
            return BMRESULT_FAILED;

        // 優先度を設定
        GetQueuePriorities(_create_desc.num_queues, _create_desc.priorities, discrete_queue_priorities, &_data->queue_priorities);

        // グローバル優占度を設定
        auto&& global_priority_ci = _families_data.global_priority_create_infos.emplace_back(VkDeviceQueueGlobalPriorityCreateInfoEXT{ VK_STRUCTURE_TYPE_DEVICE_QUEUE_GLOBAL_PRIORITY_CREATE_INFO_EXT });
        global_priority_ci.globalPriority = GetQueueGlobalPriority(_create_desc.flags);

        // 作成情報を設定
        auto&& ci = _families_data.create_infos.emplace_back(VkDeviceQueueCreateInfo{ VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO }); 
        ci.pNext            = &global_priority_ci;
        ci.flags            = 0;
        ci.queueFamilyIndex = _qf_index;
        ci.queueCount       = _create_desc.num_queues;
        ci.pQueuePriorities = _data->queue_priorities.data();

        // create infoのポインタをセット
        _data->create_info = &ci;
        _data->global_priority_create_info = &global_priority_ci;

        return BMRESULT_SUCCEED;
    };

    // 各キューファミリ作成情報を作成する
    for (uint32_t i_qcdesc = 0; i_qcdesc < desc.num_queue_create_descs; i_qcdesc++)
    {
        auto&& qcdesc = desc.queue_create_descs[i_qcdesc];
        bool is_enable_sparse_bind = false;
        bool is_enable_protect = false;
        int qf_index = GetQueueFamilyIndex(queue_data.families_data.props, qcdesc.type, &is_enable_sparse_bind, &is_enable_protect);
        if (qf_index == -1)
        {
            B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_INITIALIZATION
                              , __FUNCTION__": 無効なCOMMAND_TYPEが指定されました。");
            return BMRESULT_FAILED;
        }

        auto&& qf_data = queue_data.families.data()[qcdesc.type];
        qf_data.create_desc_index = i_qcdesc;

        // プロパティのポインタをセット
        qf_data.props = &queue_data.families_data.props.data()[qf_index];
        qf_data.checkpoint_props = &queue_data.families_data.checkpoint_props.data()[qf_index];
        
        // 特定のキューファミリ作成情報を作成する
        auto bmr = CreateDeviceQueueCreateInfo(queue_data.families_data, (uint32_t)qf_index, qcdesc, &qf_data);
        B3D_RET_IF_FAILED(bmr);
    }

    return BMRESULT_SUCCEED;
}

BMRESULT 
B3D_APIENTRY DeviceVk::CreateLogicalDevice()
{
    VkDeviceCreateInfo device_create_info{ VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
    auto last_pnext = &device_create_info.pNext;

    // デバイスグループアダプタの場合VkDeviceGroupDeviceCreateInfoを接続する。
    VkDeviceGroupDeviceCreateInfo device_group_create_info{ VK_STRUCTURE_TYPE_DEVICE_GROUP_DEVICE_CREATE_INFO };
    if (auto pdgp = adapter->GetVkPhysicalDeviceGroupProperties())
    {
        device_group_create_info.physicalDeviceCount = pdgp->physicalDeviceCount;
        device_group_create_info.pPhysicalDevices = pdgp->physicalDevices;
        last_pnext = util::ConnectPNextChains(last_pnext, device_group_create_info);
    }

    // 論理デバイスを作成
    auto&& pd_data = adapter->GetPhysicalDeviceData();
    last_pnext = util::ConnectPNextChains(last_pnext, pd_data.features2);

    // レイヤ、拡張機能を取得
    GetDeviceExtensionNames(all_layers, &all_extensions);

    // デバイスレイヤを設定
    // NOTE: デバイスレイヤは廃止されたが、互換性のために取得は可能。https://www.khronos.org/registry/vulkan/specs/1.2-extensions/html/vkspec.html#extendingvulkan-layers-devicelayerdeprecation
    /*
    // 有効化するレイヤ
    GetDeviceLayerNames(&all_layers);
    const char* LAYERS_TO_ENABLE[] =
    {
        ""
    };
    util::DyArray<const char*> enable_layer_names_ary;
    for (auto& i : LAYERS_TO_ENABLE)
    {
        if (CheckDeviceLayerSupport(i))
            enable_layer_names_ary.emplace_back(i);
        else
        {
            B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_WARNING, DEBUG_MESSAGE_CATEGORY_FLAG_INITIALIZATION
                              , hlp::StringConvolution(__FUNCTION__, ": Requested device layer \"", i, "\" NOT supported.\n").c_str());
        }
    }
    device_create_info.enabledLayerCount = (uint32_t)enable_layer_names_ary.size();
    device_create_info.ppEnabledLayerNames = enable_layer_names_ary.data();
    */

    // デバイス拡張を設定

    // 有効化する拡張
    util::DyArray<util::String> DEBUG_EXTS_TO_ENABLE =
    {
          VK_EXT_DEBUG_MARKER_EXTENSION_NAME // TODO: VK_EXT_debug_utilsへ統合され非推奨となったので、新たにVK_EXT_debug_utilsへ対応する必要がある。
    };

    util::DyArray<util::String> EXTS_TO_ENABLE =
    {
          VK_KHR_SWAPCHAIN_EXTENSION_NAME
        , VK_EXT_VERTEX_ATTRIBUTE_DIVISOR_EXTENSION_NAME
        , VK_EXT_CALIBRATED_TIMESTAMPS_EXTENSION_NAME
        , VK_KHR_PERFORMANCE_QUERY_EXTENSION_NAME
        , VK_EXT_TOOLING_INFO_EXTENSION_NAME
        , VK_EXT_SAMPLE_LOCATIONS_EXTENSION_NAME
        , VK_NV_FRAMEBUFFER_MIXED_SAMPLES_EXTENSION_NAME
        , VK_NV_COVERAGE_REDUCTION_MODE_EXTENSION_NAME
        , VK_NV_COOPERATIVE_MATRIX_EXTENSION_NAME
        , VK_EXT_HDR_METADATA_EXTENSION_NAME
        , VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME
        , VK_EXT_CONDITIONAL_RENDERING_EXTENSION_NAME
        , VK_NV_MESH_SHADER_EXTENSION_NAME
        , VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME
        , VK_KHR_RAY_TRACING_EXTENSION_NAME
        , VK_NV_EXTERNAL_MEMORY_EXTENSION_NAME

    #ifdef VK_USE_PLATFORM_WIN32_KHR
        , VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME
        , VK_KHR_EXTERNAL_SEMAPHORE_WIN32_EXTENSION_NAME
        , VK_KHR_EXTERNAL_FENCE_WIN32_EXTENSION_NAME
        , VK_NV_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME
        , VK_EXT_FULL_SCREEN_EXCLUSIVE_EXTENSION_NAME
    #endif

    };
    util::DyArray<const char*> enable_ext_names_ary;
    auto SetExts = [&](const util::DyArray<util::String>& _exts)
    {
        for (auto& i : _exts)
        {
            if (CheckDeviceExtensionSupport(i.c_str()))
                enable_ext_names_ary.emplace_back(i.c_str());
            else
            {
                B3D_ADD_DEBUG_MSG2(DEBUG_MESSAGE_SEVERITY_WARNING, DEBUG_MESSAGE_CATEGORY_FLAG_INITIALIZATION
                                   , hlp::StringConvolution(__FUNCTION__, "Requested device extension \"", i, "\" NOT supported.\n").c_str());
            }
        }
    };
    if (factory->GetDesc().debug.is_enable)
        SetExts(DEBUG_EXTS_TO_ENABLE);
    SetExts(EXTS_TO_ENABLE);
    device_create_info.enabledExtensionCount = (uint32_t)enable_ext_names_ary.size();
    device_create_info.ppEnabledExtensionNames = enable_ext_names_ary.data();

    if constexpr (IS_ENABLE_DEBUG_OUTPUT)
    {
        if (util::IsEnabledDebug(this))
        {
            util::StringStream ss;
            /*ss << "Enabled device layers" << std::endl;
            for (auto& i : enable_layer_names_ary)
                ss << "\t" << i << std::endl;*/
            ss << "Enabled device extensions" << std::endl;
            for (auto& i : enable_ext_names_ary)
                ss << "\t" << i << std::endl;
            B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_INFO, DEBUG_MESSAGE_CATEGORY_FLAG_INITIALIZATION
                              , ss.str().c_str());
        }
    }

    // 有効化されたレイヤ、拡張を格納
    {
        auto PushIfFound = [](util::DyArray<util::SharedPtr<util::String>>& _all_names, util::DyArray<util::SharedPtr<util::String>>* _enabled_names, const char* _request_name)
        {
            auto fn = [_request_name](const util::SharedPtr<util::String>& _str) { return *_str == _request_name; };
            auto&& it_found = std::find_if(_all_names.begin(), _all_names.end(), fn);
            if (it_found != _all_names.end())
                _enabled_names->push_back(*it_found);
        };

        /*enabled_layers.reserve(enable_layer_names_ary.size());
        for (auto& i : enable_layer_names_ary)
            PushIfFound(all_layers, &enabled_layers, i);*/

        enabled_extensions.reserve(enable_ext_names_ary.size());
        for (auto& i : enable_ext_names_ary)
            PushIfFound(all_extensions, &enabled_extensions, i);
    }

    // デバイスキューの作成
    // キューファミリを取得する
    //util::DyArray<VkQueueFamilyCheckpointPropertiesNV>* qf_checkpoint_props = CheckDeviceExtensionEnabled(VK_NV_DEVICE_DIAGNOSTIC_CHECKPOINTS_EXTENSION_NAME) ? &queue_data.families_data.checkpoint_props : nullptr;
    GetQueueFamilyProperties(&queue_data.families_data.props, &queue_data.families_data.checkpoint_props);

    // キューデスクを構成する
    B3D_RET_IF_FAILED(CreateDeviceQueueCreateInfos());
    device_create_info.queueCreateInfoCount = (uint32_t)queue_data.families_data.create_infos.size();
    device_create_info.pQueueCreateInfos = queue_data.families_data.create_infos.data();

    // 論理デバイスを作成
    auto vkr = vkCreateDevice(primary_physical_device, &device_create_info, B3D_VK_ALLOC_CALLBACKS, &device);
    VKR_TRACE_IF_FAILED(vkr);
    B3D_RET_IF_FAILED(util::GetBMResultFromVk(vkr));

    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY DeviceVk::CreateDebugNameSetter()
{
    if (factory->CheckInstanceExtensionEnabled(VK_EXT_DEBUG_UTILS_EXTENSION_NAME))
        debug_name_setter = B3DNewArgs(VulkanDebugNameSetterDebugUtils, this);

    else if (factory->CheckInstanceExtensionEnabled(VK_EXT_DEBUG_MARKER_EXTENSION_NAME))
        debug_name_setter = B3DNewArgs(VulkanDebugNameSetterDebugReport, this);

    return BMRESULT_SUCCEED;
}

BMRESULT 
B3D_APIENTRY DeviceVk::CreateCommandQueueVk()
{
    for (auto& it_qf : queue_data.families)
    {
        // create_info をセットしていない場合そのキュータイプは存在しない。
        if (it_qf.create_info == nullptr)
            continue;

        it_qf.queues.resize(it_qf.create_info->queueCount);
        for (uint32_t i = 0; i < it_qf.create_info->queueCount; i++)
        {
            auto&& i_queue = it_qf.queues.data()[i];
            i_queue.device_queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_INFO_2;
            i_queue.device_queue_info.queueFamilyIndex = it_qf.create_info->queueFamilyIndex;
            i_queue.device_queue_info.queueIndex = i;
            B3D_RET_IF_FAILED(CommandQueueVk::Create(this, it_qf.checkpoint_props->checkpointExecutionStageMask
                                                     , it_qf.create_info->flags, i_queue.device_queue_info, desc.queue_create_descs[it_qf.create_desc_index], &i_queue.queue));
        }
    }

    return BMRESULT_SUCCEED;
}

BMRESULT 
B3D_APIENTRY DeviceVk::GetDeviceLevelProperties()
{
    device_data = B3DMakeUnique(util::DEVICE_DATA);

    // キャリブレートタイムスタンプ
    if (devpfn->vkGetPhysicalDeviceCalibrateableTimeDomainsEXT)
    {
        // タイムドメインを取得
        // TODO: DeviceGroupの場合、各物理デバイス毎にタイムドメインを取得する必要があるか確認する。
        uint32_t count = 0;
        auto&& time_domains = *(device_data->time_domain_exts = B3DMakeUnique(decltype(device_data->time_domain_exts)::element_type));
        devpfn->vkGetPhysicalDeviceCalibrateableTimeDomainsEXT(primary_physical_device, &count, nullptr);
        time_domains.resize(count);
        auto vkr = devpfn->vkGetPhysicalDeviceCalibrateableTimeDomainsEXT(primary_physical_device, &count, time_domains.data());
        VKR_TRACE_IF_FAILED(vkr);

        // TODO: DeviceVk::GetDeviceLevelProperties: POSIXのサポート
        if (std::find(time_domains.begin(), time_domains.end(), VK_TIME_DOMAIN_CLOCK_MONOTONIC_EXT)     != time_domains.end() || 
            std::find(time_domains.begin(), time_domains.end(), VK_TIME_DOMAIN_CLOCK_MONOTONIC_RAW_EXT) != time_domains.end())
        {
            B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_OTHER, DEBUG_MESSAGE_CATEGORY_FLAG_B3D_DETAILS
                              , "TODO: DeviceVk::GetDeviceLevelProperties: POSIXのサポート");
            device_data->time_domain_exts.reset();
        }
        else
        {
            // 較正タイムスタンプ取得用構造を作成
            auto&& timestamp_infos = *(device_data->calibrated_timestamp_info_exts = B3DMakeUnique(decltype(device_data->calibrated_timestamp_info_exts)::element_type));
            timestamp_infos.resize(2/*DEVICE, QPC*/, { VK_STRUCTURE_TYPE_CALIBRATED_TIMESTAMP_INFO_EXT });

            auto&& it_found_device              = std::find(time_domains.begin(), time_domains.end(), VK_TIME_DOMAIN_DEVICE_EXT);
            auto&& it_found_performance_counter = std::find(time_domains.begin(), time_domains.end(), VK_TIME_DOMAIN_QUERY_PERFORMANCE_COUNTER_EXT);
            if (it_found_device              != time_domains.end() &&
                it_found_performance_counter != time_domains.end())
            {
                timestamp_infos[0].timeDomain = *it_found_device;
                timestamp_infos[1].timeDomain = *it_found_performance_counter;
            }
            else
            {
                // 較正タイムスタンプはサポートされない
                device_data->time_domain_exts.reset();
                device_data->calibrated_timestamp_info_exts.reset();
            }
        }
    }

    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY DeviceVk::CreateZeroBindingDescriptorSetLayout()
{
    VkDescriptorSetLayoutCreateInfo ci{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
    auto vkr = vkCreateDescriptorSetLayout(device, &ci, B3D_VK_ALLOC_CALLBACKS, &zero_binding_layout);
    B3D_RET_IF_FAILED(VKR_TRACE_IF_FAILED(vkr));

    return BMRESULT_SUCCEED;
}

void 
B3D_APIENTRY DeviceVk::MakeResourceHeapProperties()
{
    auto device_group_props = GetDeviceAdapter()->GetVkPhysicalDeviceGroupProperties();
    auto is_enable_subset_allocation = device_group_props ? device_group_props->subsetAllocation == VK_TRUE : false;
    auto&& mp = GetMemoryProperties().GetVkMemoryProperties2().memoryProperties;

    heap_props.resize(mp.memoryTypeCount);
    auto props_data = heap_props.data();
    for (uint32_t i = 0; i < mp.memoryTypeCount; i++)
    {
        auto&& t = mp.memoryTypes[i];
        auto&& h = mp.memoryHeaps[t.heapIndex];
        auto&& hp = props_data[i];
        hp.heap_index = i;

        if (is_enable_subset_allocation)
            hp.flags |= RESOURCE_HEAP_PROPERTY_FLAG_SUBSET_ALLOCATION;

        if (h.flags & VK_MEMORY_HEAP_MULTI_INSTANCE_BIT)
            hp.flags |= RESOURCE_HEAP_PROPERTY_FLAG_MULTI_INSTANCE;

        if (t.propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
            hp.flags |= RESOURCE_HEAP_PROPERTY_FLAG_DEVICE_LOCAL;
        
        if (t.propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
            hp.flags |= RESOURCE_HEAP_PROPERTY_FLAG_HOST_READABLE | RESOURCE_HEAP_PROPERTY_FLAG_HOST_WRITABLE;

        if (t.propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
            hp.flags |= RESOURCE_HEAP_PROPERTY_FLAG_HOST_COHERENT;
        
        if (t.propertyFlags & VK_MEMORY_PROPERTY_HOST_CACHED_BIT)
            hp.flags |= RESOURCE_HEAP_PROPERTY_FLAG_HOST_CACHED;
        
        if (t.propertyFlags & VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT)
            hp.flags |= RESOURCE_HEAP_PROPERTY_FLAG_LAZILY_ALLOCATED;
        
        if (t.propertyFlags & VK_MEMORY_PROPERTY_PROTECTED_BIT)
            hp.flags |= RESOURCE_HEAP_PROPERTY_FLAG_PROTECTED;
        
        //if (t.propertyFlags & VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD)
        //    hp.flags;// TODO: VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD
        //
        //if (t.propertyFlags & VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD)
        //    hp.flags;// TODO: VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD
    }
}

void
B3D_APIENTRY DeviceVk::Uninit()
{
    name.reset();

    format_props.reset();
    format_comapbility.reset();

    for (auto& i : queue_data.families)
    {
        hlp::SwapClear(i.queues);
        hlp::SwapClear(i.queue_priorities);
    }
    hlp::SwapClear(queue_data.families_data.props);
    hlp::SwapClear(queue_data.families_data.checkpoint_props);
    hlp::SwapClear(queue_data.families_data.create_infos);
    hlp::SwapClear(queue_data.families_data.global_priority_create_infos);
    
    hlp::SwapClear(heap_props);

    devpfn.reset();
    device_data.reset();
    B3DSafeDelete(debug_name_setter);

    hlp::SwapClear(all_layers);
    hlp::SwapClear(all_extensions);
    hlp::SwapClear(enabled_layers);
    hlp::SwapClear(enabled_extensions);
    
    hlp::SwapClear(desc_data.queue_create_descs);
    hlp::SwapClear(desc_data.qcdescs_node_masks);
    hlp::SwapClear(desc_data.qcdescs_priorities);

    if (zero_binding_layout)
        vkDestroyDescriptorSetLayout(device, zero_binding_layout, B3D_VK_ALLOC_CALLBACKS);
    zero_binding_layout = VK_NULL_HANDLE;

    if (device)
        vkDestroyDevice(device, B3D_VK_ALLOC_CALLBACKS);
    device = VK_NULL_HANDLE;

    hlp::SafeRelease(adapter);
    primary_physical_device = VK_NULL_HANDLE;
    hlp::SwapClear(physical_devices);

    hlp::SafeRelease(factory);
    instance = VK_NULL_HANDLE;

}

BMRESULT
B3D_APIENTRY DeviceVk::Create(DeviceFactoryVk* _factory, const DEVICE_DESC& _desc, DeviceVk** _dst)
{
    util::Ptr<DeviceVk> ptr;
    ptr.Attach(B3DCreateImplementationClass(DeviceVk));
    B3D_RET_IF_FAILED(ptr->Init(_factory, _desc));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY DeviceVk::AddRef()
{
    ++ref_count;
    B3D_REFCOUNT_DEBUG(ref_count);
}

uint32_t
B3D_APIENTRY DeviceVk::Release()
{
    B3D_REFCOUNT_DEBUG(ref_count - 1);
    auto count = --ref_count;
    if (count == 0)
        B3DDestroyImplementationClass(this);

    return count;
}

uint32_t
B3D_APIENTRY DeviceVk::GetRefCount() const
{
    return ref_count;
}

const char*
B3D_APIENTRY DeviceVk::GetName() const
{
    return name ? name->c_str() : nullptr;
}

BMRESULT
B3D_APIENTRY DeviceVk::SetName(const char* _name) 
{
    if (!util::IsEnabledDebug(this))
        return BMRESULT_FAILED;

    if (device)
        B3D_RET_IF_FAILED(SetVkObjectName(device, _name));

    if (name && !_name)
        name.reset();
    else
        name = B3DMakeUniqueArgs(util::NameableObjStr, _name);

    return BMRESULT_SUCCEED;
}

const DEVICE_DESC&
B3D_APIENTRY DeviceVk::GetDesc() const
{
    return desc;
}

NodeMask 
B3D_APIENTRY DeviceVk::GetValidNodeMask() const
{
    return node_mask;
}

uint32_t 
B3D_APIENTRY DeviceVk::GetResourceHeapProperties(RESOURCE_HEAP_PROPERTIES* _properties) const
{
    auto size = (uint32_t)heap_props.size();
    if (_properties)
        util::MemCopyArray(_properties, heap_props.data(), size);
    return size;
}

BMRESULT
B3D_APIENTRY DeviceVk::GetResourceAllocationInfo(uint32_t _num_resources, const IResource* const* _resources, RESOURCE_ALLOCATION_INFO* _dst_infos
                                                 , RESOURCE_HEAP_ALLOCATION_INFO* _dst_heap_info) const
{
    uint64_t max_alignment = 0;// _resources内の最大アライメント。
    uint32_t masked_heap_type_bits = ~0u;// _resources全てを割り当て可能なヒープタイプのビットマスク。

    // メモリ要件を取得し引数にセット
    for (uint32_t i = 0; i < _num_resources; i++)
    {
        VkMemoryRequirements2         reqs           { VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2 };
        VkMemoryDedicatedRequirements dedicated_reqs { VK_STRUCTURE_TYPE_MEMORY_DEDICATED_REQUIREMENTS };

        auto _resource = _resources[i];
        auto _resvk = _resource->DynamicCastFromThis<IResourceVk>();

        // committedリソースの際にセットする。
        if (_resvk->GetCreateType() == RESOURCE_CREATE_TYPE_COMMITTED)
            util::ConnectPNextChains(reqs, dedicated_reqs);

        // 各パラメータを取得、設定
        _resvk->GetMemoryRequirements(&reqs);

        auto&& _info = _dst_infos[i];
        _info.heap_type_bits = reqs.memoryRequirements.memoryTypeBits;
        _info.alignment      = reqs.memoryRequirements.alignment;
        _info.size_in_bytes  = reqs.memoryRequirements.size;// NOTE: Vulkan/D3D12共にsizeにはアライメント済みの値が入っている。

        if (max_alignment < reqs.memoryRequirements.alignment)
            max_alignment = reqs.memoryRequirements.alignment;

        // 全リソースを割り当て可能なヒープタイプを抽出(マスクが0になった場合をエラーとして扱うべき?)
        masked_heap_type_bits &= reqs.memoryRequirements.memoryTypeBits;

        // TODO: VkMemoryDedicatedRequirements::requiresDedicatedAllocationはそのリソースに専用割り当てが必要かどうかを示すので、今後問題が起きた際に処理を追加する。

        //B3D_RET_IF_FAILED(_resource->GetMemoryRequirements(_dst));
    }

    // 上のスコープで取得した値から各リソースのオフセットを算出
    uint64_t heap_remain = 0;// 最大アライメントのサイズがヒープに加算された後の、残りサイズを保持する一時変数。
    uint64_t heap_offset = 0;
    uint64_t total_size = 0;
    for (uint32_t i = 0; i < _num_resources; i++)
    {
        // 情報のサイズにはアライメントされたサイズを返す。
        // 合計サイズにはアライメントされた構造内の最大アライメント要求でアラインした合計サイズを返す。

        auto&& _info = _dst_infos[i];
        _info.heap_offset = hlp::AlignUp(heap_offset, _info.alignment);

        auto aligned_size = _info.size_in_bytes + (_info.heap_offset - heap_offset);// アラインされたオフセット(_info.heap_offset)と現在のオフセットとの差分

        if (heap_remain == 0 || heap_remain < aligned_size)
        {
            heap_remain += max_alignment;
            total_size += max_alignment;
        }

        heap_remain -= aligned_size;
        heap_offset += aligned_size;
    }

    _dst_heap_info->required_alignment  = max_alignment;
    _dst_heap_info->total_size_in_bytes = total_size;
    _dst_heap_info->heap_type_bits      = masked_heap_type_bits;

    return BMRESULT_SUCCEED;
}

uint32_t 
B3D_APIENTRY DeviceVk::GetTiledResourceAllocationInfo(const IResource* _reserved_resource, TILED_RESOURCE_ALLOCATION_INFO* _dst_infos) const
{
    uint32_t count = 0;
    auto&& d = _reserved_resource->GetDesc();
    if (d.dimension == RESOURCE_DIMENSION_BUFFER)
        count = _reserved_resource->As<BufferVk>()->GetTiledResourceAllocationInfo(_dst_infos);
    else
        count = _reserved_resource->As<TextureVk>()->GetTiledResourceAllocationInfo(_dst_infos);

    return count;
}


uint32_t
B3D_APIENTRY DeviceVk::GetDescriptorPoolSizesAllocationInfo(uint32_t _num_root_signatures, const IRootSignature* const* _root_signatures, const uint32_t* _num_descriptor_sets, uint32_t* _dst_max_num_register_space, DESCRIPTOR_POOL_SIZE* _dst_sizes) const
{
    // OPTIMIZE: DeviceVk::GetDescriptorPoolSizesAllocationInfo
    util::UnordMap<DESCRIPTOR_TYPE, uint32_t> ps;

    uint32_t max_num_register_space = 0;
    for (uint32_t i = 0; i < _num_root_signatures; i++)
    {
        auto&& rs = _root_signatures[i]->As<RootSignatureVk>();
        auto&& sizes = rs->GetPoolSizes();
        for (auto& [type, size] : sizes)
        {
            ps[type] += size * _num_descriptor_sets[i];
        }

        max_num_register_space = std::max(max_num_register_space, rs->GetNumRegisterSpace());
    }

    if (_dst_max_num_register_space)
        *_dst_max_num_register_space = max_num_register_space;

    uint32_t num_types = (uint32_t)ps.size();

    if (_dst_sizes)
    {
        // 必要なタイプのプールサイズのみを格納。
        size_t required_type_count = 0;
        for (auto& [type, size] : ps)
        {
            _dst_sizes[required_type_count++] = DESCRIPTOR_POOL_SIZE{ type, size };
        }
    }

    return num_types;
}

BMRESULT
B3D_APIENTRY DeviceVk::GetCommandQueue(COMMAND_TYPE _type, uint32_t _queue_index, ICommandQueue** _dst) const
{
    auto&& queues = queue_data.families[_type].queues;
    auto size = queues.size();
    if (size == 0)
        return BMRESULT_FAILED;
    if (_queue_index >= size)
        return BMRESULT_FAILED_OUT_OF_RANGE;

    (*_dst = queues.data()[_queue_index].queue)->AddRef();
    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY DeviceVk::WaitIdle()
{
    auto vkr = vkDeviceWaitIdle(device);
    B3D_RET_IF_FAILED(VKR_TRACE_IF_FAILED(vkr));

    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY DeviceVk::CreateCommandAllocator(const COMMAND_ALLOCATOR_DESC& _desc, ICommandAllocator** _dst)
{
    util::Ptr<CommandAllocatorVk> ptr;
    B3D_RET_IF_FAILED(CommandAllocatorVk::Create(this, _desc, &ptr));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY DeviceVk::AllocateCommandList(const COMMAND_LIST_DESC& _desc, ICommandList** _dst)
{
    util::Ptr<CommandListVk> ptr;
    B3D_RET_IF_FAILED(CommandListVk::Create(this, _desc, &ptr));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

BMRESULT 
B3D_APIENTRY DeviceVk::CreateFence(const FENCE_DESC& _desc, IFence** _dst)
{
    util::Ptr<FenceVk> ptr;
    B3D_RET_IF_FAILED(FenceVk::Create(this, _desc, &ptr));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY DeviceVk::CreateRootSignature(const ROOT_SIGNATURE_DESC& _desc, IRootSignature** _dst)
{
    util::Ptr<RootSignatureVk> ptr;
    B3D_RET_IF_FAILED(RootSignatureVk::Create(this, _desc, &ptr));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY DeviceVk::CreateDescriptorPool(const DESCRIPTOR_POOL_DESC& _desc, IDescriptorPool** _dst)
{
    util::Ptr<DescriptorPoolVk> ptr;
    B3D_RET_IF_FAILED(DescriptorPoolVk::Create(this, _desc, &ptr));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY DeviceVk::UpdateDescriptorSets(const UPDATE_DESCRIPTOR_SET_DESC& _update_desc)
{
    for (uint32_t i = 0; i < _update_desc.num_write_descriptor_sets; i++)
    {
        auto&& write = _update_desc.write_descriptor_sets[i];
        auto dst_set = write.dst_set->As<DescriptorSetVk>();
        B3D_RET_IF_FAILED(dst_set->AddWriteDescriptors(write));
        dst_set->UpdateDescriptors();
    }
    for (uint32_t i = 0; i < _update_desc.num_copy_descriptor_sets; i++)
    {
        auto&& copy = _update_desc.copy_descriptor_sets[i];
        auto dst_set = copy.dst_set->As<DescriptorSetVk>();
        B3D_RET_IF_FAILED(dst_set->AddCopyDescriptors(copy));
        dst_set->UpdateDescriptors();
    }

    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY DeviceVk::CreateShaderModule(const SHADER_MODULE_DESC& _desc, IShaderModule** _dst)
{
    util::Ptr<ShaderModuleVk> ptr;
    B3D_RET_IF_FAILED(ShaderModuleVk::Create(this, _desc, &ptr));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY DeviceVk::CreateGraphicsPipelineState(const GRAPHICS_PIPELINE_STATE_DESC& _desc, IPipelineState** _dst)
{
    util::Ptr<GraphicsPipelineStateVk> ptr;
    B3D_RET_IF_FAILED(GraphicsPipelineStateVk::Create(this, _desc, &ptr));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY DeviceVk::CreateComputePipelineState(const COMPUTE_PIPELINE_STATE_DESC& _desc, IPipelineState** _dst)
{
    util::Ptr<ComputePipelineStateVk> ptr;
    B3D_RET_IF_FAILED(ComputePipelineStateVk::Create(this, _desc, &ptr));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY DeviceVk::CreateRayTracingPipelineState(const RAY_TRACING_PIPELINE_STATE_DESC& _desc, IPipelineState** _dst)
{
    //util::Ptr<RayTracingPipelineStateVk> ptr;
    //B3D_RET_IF_FAILED(ComputePipelineStateVk::Create(this, _desc, &ptr));
    //
    //*_dst = ptr.Detach();
    //return BMRESULT_SUCCEED;
    return BMRESULT_FAILED_NOT_IMPLEMENTED;
}

BMRESULT 
B3D_APIENTRY DeviceVk::CreateSwapChain(const SWAP_CHAIN_DESC& _desc, ISwapChain** _dst)
{
    util::Ptr<SwapChainVk> ptr;
    B3D_RET_IF_FAILED(SwapChainVk::Create(this, _desc, &ptr));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}


BMRESULT
B3D_APIENTRY DeviceVk::CreateResourceHeap(const RESOURCE_HEAP_DESC& _desc, IResourceHeap** _dst)
{
    util::Ptr<ResourceHeapVk> ptr;
    B3D_RET_IF_FAILED(ResourceHeapVk::Create(this, _desc, &ptr));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY DeviceVk::BindResourceHeaps(uint32_t _num_bind_infos, const BIND_RESOURCE_HEAP_INFO* _bind_infos)
{
    for (uint32_t i = 0; i < _num_bind_infos; i++)
    {  
        // TODO: vkBindImage/BufferMemory2への対応
        auto&& bi = _bind_infos[i];
        B3D_RET_IF_FAILED(bi.dst_resource->DynamicCastFromThis<IResourceVk>()->Bind(&bi));
    }
    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY DeviceVk::CreatePlacedResource(const RESOURCE_DESC& _desc, IResource** _dst)
{
    switch (_desc.dimension)
    {
    case buma3d::RESOURCE_DIMENSION_BUFFER:
    {
        util::Ptr<BufferVk> ptr;
        B3D_RET_IF_FAILED(BufferVk::Create(RESOURCE_CREATE_TYPE_PLACED, this, _desc, &ptr));
        *_dst = ptr.Detach();
        break;
    }

    case buma3d::RESOURCE_DIMENSION_TEX1D:
    case buma3d::RESOURCE_DIMENSION_TEX2D:
    case buma3d::RESOURCE_DIMENSION_TEX3D:
    {
        util::Ptr<TextureVk> ptr;
        B3D_RET_IF_FAILED(TextureVk::Create(RESOURCE_CREATE_TYPE_PLACED, this, _desc, &ptr));
        *_dst = ptr.Detach();
        break;
    }

    default:
        return BMRESULT_FAILED_INVALID_PARAMETER;
    }

    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY DeviceVk::CreateReservedResource(const RESOURCE_DESC& _desc, IResource** _dst)
{
    switch (_desc.dimension)
    {
    case buma3d::RESOURCE_DIMENSION_BUFFER:
    {
        util::Ptr<BufferVk> ptr;
        B3D_RET_IF_FAILED(BufferVk::Create(RESOURCE_CREATE_TYPE_RESERVED, this, _desc, &ptr));
        *_dst = ptr.Detach();
        break;
    }

    case buma3d::RESOURCE_DIMENSION_TEX1D:
    case buma3d::RESOURCE_DIMENSION_TEX2D:
    case buma3d::RESOURCE_DIMENSION_TEX3D:
    {
        util::Ptr<TextureVk> ptr;
        B3D_RET_IF_FAILED(TextureVk::Create(RESOURCE_CREATE_TYPE_RESERVED, this, _desc, &ptr));
        *_dst = ptr.Detach();
        break;
    }

    default:
        return BMRESULT_FAILED_INVALID_PARAMETER;
    }

    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY DeviceVk::CreateCommittedResource(const COMMITTED_RESOURCE_DESC& _desc, IResource** _dst)
{
    switch (_desc.resource_desc.dimension)
    {
    case buma3d::RESOURCE_DIMENSION_BUFFER:
    {
        util::Ptr<BufferVk> ptr;
        B3D_RET_IF_FAILED(BufferVk::CreateCommitted(this, _desc, &ptr));
        *_dst = ptr.Detach();
        break;
    }

    case buma3d::RESOURCE_DIMENSION_TEX1D:
    case buma3d::RESOURCE_DIMENSION_TEX2D:
    case buma3d::RESOURCE_DIMENSION_TEX3D:
    {
        util::Ptr<TextureVk> ptr;
        B3D_RET_IF_FAILED(TextureVk::CreateCommitted(this, _desc, &ptr));
        *_dst = ptr.Detach();
        break;
    }

    default:
        return BMRESULT_FAILED_INVALID_PARAMETER;
    }

    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY DeviceVk::CreateVertexBufferView(IBuffer* _buffer, const VERTEX_BUFFER_VIEW_DESC& _desc, IVertexBufferView** _dst)
{
    util::Ptr<VertexBufferViewVk> ptr;
    B3D_RET_IF_FAILED(VertexBufferViewVk::Create(this, _buffer, _desc, &ptr));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY DeviceVk::CreateIndexBufferView(IBuffer* _buffer, const INDEX_BUFFER_VIEW_DESC& _desc, IIndexBufferView** _dst)
{
    util::Ptr<IndexBufferViewVk> ptr;
    B3D_RET_IF_FAILED(IndexBufferViewVk::Create(this, _buffer, _desc, &ptr));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY DeviceVk::CreateConstantBufferView(IBuffer* _buffer, const CONSTANT_BUFFER_VIEW_DESC& _desc, IConstantBufferView** _dst)
{
    util::Ptr<ConstantBufferViewVk> ptr;
    B3D_RET_IF_FAILED(ConstantBufferViewVk::Create(this, _buffer, _desc, &ptr));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY DeviceVk::CreateShaderResourceView(IResource* _resource, const SHADER_RESOURCE_VIEW_DESC& _desc, IShaderResourceView** _dst)
{
    util::Ptr<ShaderResourceViewVk> ptr;
    B3D_RET_IF_FAILED(ShaderResourceViewVk::Create(this, _resource, _desc, &ptr));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY DeviceVk::CreateUnorderedAccessView(IResource* _resource, IBuffer* _resource_for_counter_buffer, const UNORDERED_ACCESS_VIEW_DESC& _desc, IUnorderedAccessView** _dst)
{
    util::Ptr<UnorderedAccessViewVk> ptr;
    B3D_RET_IF_FAILED(UnorderedAccessViewVk::Create(this, _resource, _resource_for_counter_buffer, _desc, &ptr));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY DeviceVk::CreateRenderTargetView(IResource* _resource, const RENDER_TARGET_VIEW_DESC& _desc, IRenderTargetView** _dst)
{
    util::Ptr<RenderTargetViewVk> ptr;
    B3D_RET_IF_FAILED(RenderTargetViewVk::Create(this, _resource, _desc, &ptr));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY DeviceVk::CreateDepthStencilView(IResource* _resource, const DEPTH_STENCIL_VIEW_DESC& _desc, IDepthStencilView** _dst)
{
    util::Ptr<DepthStencilViewVk> ptr;
    B3D_RET_IF_FAILED(DepthStencilViewVk::Create(this, _resource, _desc, &ptr));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY DeviceVk::CreateSampler(const SAMPLER_DESC& _desc, ISamplerView** _dst)
{
    util::Ptr<SamplerViewVk> ptr;
    B3D_RET_IF_FAILED(SamplerViewVk::Create(this, _desc, &ptr));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY DeviceVk::CreateStreamOutputBufferView(IBuffer* _buffer, IBuffer* _filled_size_counter_buffer, const STREAM_OUTPUT_BUFFER_VIEW_DESC& _desc, IStreamOutputBufferView** _dst)
{
    util::Ptr<StreamOutputBufferViewVk> ptr;
    B3D_RET_IF_FAILED(StreamOutputBufferViewVk::Create(this, _buffer, _filled_size_counter_buffer, _desc, &ptr));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY DeviceVk::CreateQueryHeap(const QUERY_HEAP_DESC& _desc, IQueryHeap** _dst)
{
    util::Ptr<QueryHeapVk> ptr;
    B3D_RET_IF_FAILED(QueryHeapVk::Create(this, _desc, &ptr));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY DeviceVk::CreateRenderPass(const RENDER_PASS_DESC& _desc, IRenderPass** _dst)
{
    util::Ptr<RenderPassVk> ptr;
    B3D_RET_IF_FAILED(RenderPassVk::Create(this, _desc, &ptr));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY DeviceVk::CreateFramebuffer(const FRAMEBUFFER_DESC& _desc, IFramebuffer** _dst)
{
    util::Ptr<FramebufferVk> ptr;
    B3D_RET_IF_FAILED(FramebufferVk::Create(this, _desc, &ptr));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

VkInstance
B3D_APIENTRY DeviceVk::GetVkInstance() const
{
    return instance;
}

VkDevice
B3D_APIENTRY DeviceVk::GetVkDevice() const
{
    return device;
}

const util::DEVICE_DATA& 
B3D_APIENTRY DeviceVk::GetDeviceData() const
{
    return *device_data;
}

const InstancePFN& 
B3D_APIENTRY DeviceVk::GetInstancePFN() const
{
    return *inspfn;
}

const DevicePFN&
B3D_APIENTRY DeviceVk::GetDevicePFN() const
{
    return *devpfn;
}

const util::DyArray<util::SharedPtr<util::String>>&
B3D_APIENTRY DeviceVk::GetEnabledDeviceLayers() const
{
    return enabled_layers;
}

const util::DyArray<util::SharedPtr<util::String>>&
B3D_APIENTRY DeviceVk::GetEnabledDeviceExtensions() const
{
    return enabled_extensions;
}

const VkAllocationCallbacks*
B3D_APIENTRY DeviceVk::GetVkAllocationCallbacks() const
{
    return alloc_callbacks;
}

DeviceAdapterVk*
B3D_APIENTRY DeviceVk::GetDeviceAdapter() const
{
    return adapter;
}

VkPhysicalDevice 
B3D_APIENTRY DeviceVk::GetPrimaryVkPhysicalDevice() const
{
    return primary_physical_device;
}

const util::DyArray<VkPhysicalDevice>& 
B3D_APIENTRY DeviceVk::GetVkPhysicalDevices() const
{
    return physical_devices;
}

int 
B3D_APIENTRY DeviceVk::GetQueueFamilyIndex(COMMAND_TYPE _type)
{
    // create_infoがnullptrの場合()、指定の_typeは存在しない。
    return queue_data.families.data()[_type].create_info ? queue_data.families.data()[_type].create_info->queueFamilyIndex : -1;
}

bool
B3D_APIENTRY DeviceVk::CheckDeviceLayerSupport(const char* _request_layer_name) const
{
    auto fn = [_request_layer_name](const util::SharedPtr<util::String>& _str) { return *_str == _request_layer_name; };
    return (std::find_if(all_layers.begin(), all_layers.end(), fn) != all_layers.end());
}

bool
B3D_APIENTRY DeviceVk::CheckDeviceExtensionSupport(const char* _request_ext_name) const
{
    auto fn = [_request_ext_name](const util::SharedPtr<util::String>& _str) { return *_str == _request_ext_name; };
    return (std::find_if(all_extensions.begin(), all_extensions.end(), fn) != all_extensions.end());
}

bool
B3D_APIENTRY DeviceVk::CheckDeviceLayerEnabled(const char* _request_layer_name) const
{
    auto fn = [_request_layer_name](const util::SharedPtr<util::String>& _str) { return *_str == _request_layer_name; };
    return (std::find_if(enabled_layers.begin(), enabled_layers.end(), fn) != enabled_layers.end());
}

bool
B3D_APIENTRY DeviceVk::CheckDeviceExtensionEnabled(const char* _request_ext_name) const
{
    auto fn = [_request_ext_name](const util::SharedPtr<util::String>& _str) { return *_str == _request_ext_name; };
    return (std::find_if(enabled_extensions.begin(), enabled_extensions.end(), fn) != enabled_extensions.end());
}

const util::DyArray<uint32_t>& 
B3D_APIENTRY DeviceVk::GetQueueFamilyIndices() const
{
    return queue_data.queue_family_indices;
}

bool 
B3D_APIENTRY DeviceVk::IsEnabledDebug()
{
    return factory->IsEnabledDebug();
}

void 
B3D_APIENTRY DeviceVk::AddMessageFromB3D(DEBUG_MESSAGE_SEVERITY _severity, DEBUG_MESSAGE_CATEGORY_FLAG _category, const Char8T* _str)
{
    factory->AddMessageFromB3D(_severity, _category, _str);
}

util::MemoryProperties& 
B3D_APIENTRY DeviceVk::GetMemoryProperties() 
{
    return adapter->GetMemoryProperties();
}

const util::DyArray<RESOURCE_HEAP_PROPERTIES>& 
B3D_APIENTRY DeviceVk::GetResourceHeapPropertiesForImpl() const 
{
    return heap_props; 
}

const util::VulkanFormatProperties& 
B3D_APIENTRY DeviceVk::GetVulkanFormatProperties() const
{
    return *format_props;
}

const util::FormatCompatibilityChecker& 
B3D_APIENTRY DeviceVk::GetFormatCompatibilityChecker() const
{
    return *format_comapbility;
}

VkDescriptorSetLayout
B3D_APIENTRY DeviceVk::GetZeroBindingDescriptorSetLayout() const
{
    return zero_binding_layout;
}

DeviceVk::IVulkanDebugNameSetter*
B3D_APIENTRY DeviceVk::GetDebugNameSetter() const
{
    return debug_name_setter;
}

DeviceVk::ALLOCATION_COUNTERS&
B3D_APIENTRY DeviceVk::GetAllocationCounters()
{
    return alloc_counters;
}


DeviceVk::DEVICE_QUEUE_DATA::~DEVICE_QUEUE_DATA()
{
    //B3D_ASSERT(queue->GetRefCount() == 1);
    hlp::SafeRelease(queue);
}


}// namespace buma3d
