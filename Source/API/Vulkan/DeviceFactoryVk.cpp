#include "Buma3DPCH.h"
#include "DeviceFactoryVk.h"

namespace buma3d
{
namespace /*anonymous*/
{

VkDebugUtilsMessageSeverityFlagsEXT GetNativeDebugReportFlagsForDebugUtils(uint32_t _num_descs, const DEBUG_MESSAGE_DESC* _descs)
{
    static const VkDebugUtilsMessageSeverityFlagsEXT FLAGS[] =
    {
          VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT      // DEBUG_MESSAGE_SEVERITY_INFO
        , VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT   // DEBUG_MESSAGE_SEVERITY_WARNING
        , VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT     // DEBUG_MESSAGE_SEVERITY_ERROR
        , VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT     // DEBUG_MESSAGE_SEVERITY_CORRUPTION
        , VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT   // DEBUG_MESSAGE_SEVERITY_OTHER
    };

    VkDebugUtilsMessageSeverityFlagsEXT result{};
    if (_num_descs == 0)
    {
        for (uint32_t i = 0; i < hlp::GetStaticArraySize(FLAGS); i++)
            result |= FLAGS[i];
        return result;
    }

    for (uint32_t i = 0; i < _num_descs; i++)
        result |= FLAGS[_descs[i].severity];

    return result;
}

VkDebugReportFlagsEXT GetNativeDebugReportFlags(uint32_t _num_descs, const DEBUG_MESSAGE_DESC* _descs)
{
    static const VkDebugReportFlagsEXT FLAGS[] =
    {
          VK_DEBUG_REPORT_INFORMATION_BIT_EXT                                            // DEBUG_MESSAGE_SEVERITY_INFO
        , VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT  // DEBUG_MESSAGE_SEVERITY_WARNING
        , VK_DEBUG_REPORT_ERROR_BIT_EXT                                                  // DEBUG_MESSAGE_SEVERITY_ERROR
        , VK_DEBUG_REPORT_ERROR_BIT_EXT                                                  // DEBUG_MESSAGE_SEVERITY_CORRUPTION
        , VK_DEBUG_REPORT_DEBUG_BIT_EXT                                                  // DEBUG_MESSAGE_SEVERITY_OTHER
    };

    VkDebugReportFlagsEXT result{};

    if (_num_descs == 0)
    {
        for (uint32_t i = 0; i < hlp::GetStaticArraySize(FLAGS); i++)
            result |= FLAGS[i];
        return result;
    }

    for (uint32_t i = 0; i < _num_descs; i++)
        result |= FLAGS[_descs[i].severity];

    return result;
}

DEBUG_MESSAGE_CATEGORY_FLAG GetNativeCategoryFlags(VkDebugReportObjectTypeEXT _type)
{
    // TODO: メッセージID番号を解析して適切なカテゴリを取得
    return DEBUG_MESSAGE_CATEGORY_FLAG_ALL;
}

DEBUG_MESSAGE_CATEGORY_FLAG GetNativeCategoryFlagsForDebugUtils(VkDebugUtilsMessageTypeFlagsEXT _message_types)
{
    return DEBUG_MESSAGE_CATEGORY_FLAG_ALL;
}

DEBUG_MESSAGE_SEVERITY GetB3DMessageSeverity(VkDebugReportFlagsEXT _flag)
{
    switch (_flag)
    {
    case VK_DEBUG_REPORT_INFORMATION_BIT_EXT         : return DEBUG_MESSAGE_SEVERITY_INFO;
    case VK_DEBUG_REPORT_WARNING_BIT_EXT             : return DEBUG_MESSAGE_SEVERITY_WARNING;
    case VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT : return DEBUG_MESSAGE_SEVERITY_ERROR;
    case VK_DEBUG_REPORT_ERROR_BIT_EXT               : return DEBUG_MESSAGE_SEVERITY_ERROR;
    case VK_DEBUG_REPORT_DEBUG_BIT_EXT               : return DEBUG_MESSAGE_SEVERITY_OTHER;

    default:
        return DEBUG_MESSAGE_SEVERITY(-1);
    }
}

DEBUG_MESSAGE_SEVERITY GetB3DMessageSeverityForDebugUtils(VkDebugUtilsMessageSeverityFlagBitsEXT _message_severity)
{
    switch (_message_severity)
    {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT        : return DEBUG_MESSAGE_SEVERITY_OTHER;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT           : return DEBUG_MESSAGE_SEVERITY_INFO;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT        : return DEBUG_MESSAGE_SEVERITY_WARNING;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT          : return DEBUG_MESSAGE_SEVERITY_ERROR;

    default:
        return DEBUG_MESSAGE_SEVERITY(-1);
    }
}

}// namespace /*anonymous*/
}// namespace buma3d

// アロケーション、デバッグコールバック関数
namespace buma3d
{

// デバッグ時に使用
struct DEBUG_REPORT_USER_DATA
{
    const DEVICE_FACTORY_DESC*  desc{};
    DebugMessageQueueVk*        msg_queue;
    std::mutex                  msg_output_mutex;
    util::StringStream          ss;
};

namespace /*anonymous*/
{

VKAPI_ATTR void* 
VKAPI_CALL MAllocVk
(
      void*                   _user_data
    , size_t                  _size
    , size_t                  _alignment
    , VkSystemAllocationScope _scope
)
{
    static const char* SCOPE_NAMES[] =
    {
          "vkAlloc, System alloc scope: COMMAND"
        , "vkAlloc, System alloc scope: OBJECT"
        , "vkAlloc, System alloc scope: CACHE"
        , "vkAlloc, System alloc scope: DEVICE"
        , "vkAlloc, System alloc scope: INSTANCE"
    };
    B3D_UNREFERENCED(_user_data);
    return buma3d::MAlloc(_size, _alignment, util::details::MEMORY_TYPE::GRAPHICS_API, SCOPE_NAMES[_scope], __LINE__);
}

VKAPI_ATTR void* 
VKAPI_CALL ReallocVk
(
      void*                   _user_data
    , void*                   _original
    , size_t                  _size
    , size_t                  _alignment
    , VkSystemAllocationScope _scope
)
{
    B3D_UNREFERENCED(_user_data);
    static const char* SCOPE_NAMES[] =
    {
          "vkRealloc, System alloc scope: COMMAND"
        , "vkRealloc, System alloc scope: OBJECT"
        , "vkRealloc, System alloc scope: CACHE"
        , "vkRealloc, System alloc scope: DEVICE"
        , "vkRealloc, System alloc scope: INSTANCE"
    };
    return buma3d::Realloc(_original, _size, _alignment, util::details::MEMORY_TYPE::GRAPHICS_API, SCOPE_NAMES[_scope], __LINE__);
}

VKAPI_ATTR void 
VKAPI_CALL FreeVk(void* _user_data, void* _ptr)
{
    B3D_UNREFERENCED(_user_data);
    // pMemoryはNULLの場合があり、コールバックは安全に処理する必要があります。
    if (_ptr != nullptr)
        buma3d::Free(_ptr, util::details::MEMORY_TYPE::GRAPHICS_API, "VkFree", __LINE__);
}

VKAPI_ATTR void 
VKAPI_CALL InternalAllocationNotification
(
    void*                      _user_data
    , size_t                   _size
    , VkInternalAllocationType _allocation_type
    , VkSystemAllocationScope  _allocation_scope
)
{
    static const char* INTERNAL_TYPE_NAMES[] =
    {
        "vkInternalAllocationNotification, Internal alloc type: EXECUTABLE, "
    };
    static const char* SCOPE_NAMES[] =
    {
          "System alloc scope: COMMAND"
        , "System alloc scope: OBJECT"
        , "System alloc scope: CACHE"
        , "System alloc scope: DEVICE"
        , "System alloc scope: INSTANCE"
    };

    auto b3d_data = (buma3d::DEBUG_REPORT_USER_DATA*)(_user_data);
    std::lock_guard lock(b3d_data->msg_output_mutex);

    auto&& ss = b3d_data->ss;
    ss.str("");
    ss.clear();

    ss << INTERNAL_TYPE_NAMES[_allocation_type];
    ss << SCOPE_NAMES[_allocation_scope];
    ss << "Size: " << _size;

    b3d_data->msg_queue->AddMessageFromVkAllocationCallbacks(DEBUG_MESSAGE_SEVERITY_OTHER, DEBUG_MESSAGE_CATEGORY_FLAG_MISCELLANEOUS, ss.str().c_str());
}

VKAPI_ATTR void 
VKAPI_CALL InternalFreeNotification
(
    void*                      _user_data
    , size_t                   _size
    , VkInternalAllocationType _allocation_type
    , VkSystemAllocationScope  _allocation_scope
)
{
    static const char* INTERNAL_TYPE_NAMES[] =
    {
        "vkInternalFreeNotification, Internal alloc type: EXECUTABLE, "
    };
    static const char* SCOPE_NAMES[] =
    {
          "System alloc scope: COMMAND"
        , "System alloc scope: OBJECT"
        , "System alloc scope: CACHE"
        , "System alloc scope: DEVICE"
        , "System alloc scope: INSTANCE"
    };

    auto b3d_data = (buma3d::DEBUG_REPORT_USER_DATA*)(_user_data);
    std::lock_guard lock(b3d_data->msg_output_mutex);

    auto&& ss = b3d_data->ss;
    ss.str("");
    ss.clear();

    ss << INTERNAL_TYPE_NAMES[_allocation_type];
    ss << SCOPE_NAMES[_allocation_scope];
    ss << "Size: " << _size;

    b3d_data->msg_queue->AddMessageFromVkAllocationCallbacks(DEBUG_MESSAGE_SEVERITY_OTHER, DEBUG_MESSAGE_CATEGORY_FLAG_MISCELLANEOUS, ss.str().c_str());
}


// コールバックは、複数のスレッドから同時に呼び出すことができます（アプリケーションが複数のスレッドからVulkan呼び出しを行っている場合）。
VKAPI_ATTR VkBool32 
VKAPI_CALL DebugReportCallback
(
      VkDebugReportFlagsEXT      _flag
    , VkDebugReportObjectTypeEXT _object_type
    , uint64_t                   _object
    , size_t                     _location
    , int32_t                    _message_code
    , const char*                _layer_prefix
    , const char*                _message
    , void*                      _user_data
)
{
    static const char* OBJECT_TYPE_NAMES[] =
    {
          "Unknown"                    // = 0,
        , "VkInstance"                 // = 1,
        , "VkPhysicalDevice"           // = 2,
        , "VkDevice"                   // = 3,
        , "VkQueue"                    // = 4,
        , "VkSemaphore"                // = 5,
        , "VkCommandBuffer"            // = 6,
        , "VkFence"                    // = 7,
        , "VkDeviceMemory"             // = 8,
        , "VkBuffer"                   // = 9,
        , "VkImage"                    // = 10,
        , "VkEvent"                    // = 11,
        , "VkQueryPool"                // = 12,
        , "VkBufferView"               // = 13,
        , "VkImageView"                // = 14,
        , "VkShaderModule"             // = 15,
        , "VkPipelineCache"            // = 16,
        , "VkPipelineLayout"           // = 17,
        , "VkRenderPass"               // = 18,
        , "VkPipeline"                 // = 19,
        , "VkDescriptorSetLayout"      // = 20,
        , "VkSampler"                  // = 21,
        , "VkDescriptorPool"           // = 22,
        , "VkDescriptorSet"            // = 23,
        , "VkFramebuffer"              // = 24,
        , "VkCommandPool"              // = 25,
        , "VkSurfaceKHR"               // = 26,
        , "VkSwapchainKHR"             // = 27,
        , "VkDebugReportCallbackEXT"   // = 28,
        , "VkDisplayKHR"               // = 29,
        , "VkDisplayModeKHR"           // = 30,
        , "VkValidationCacheEXT"       // = 33,
                                       
        , "VkSamplerYcbcrConversion"   // = 1000156000,->[34]
        , "VkDescriptorUpdateTemplate" // = 1000085000,->[35]
        , "VkAccelerationStructureKHR" // = 1000165000,->[36]
    };

    auto b3d_data = (buma3d::DEBUG_REPORT_USER_DATA*)(_user_data);
    std::lock_guard lock(b3d_data->msg_output_mutex);

    auto&& ss = b3d_data->ss;
    ss.str("");
    ss.clear();
    if (_flag & VK_DEBUG_REPORT_INFORMATION_BIT_EXT)
    {
        ss << "Vulkan: INFORMATION: ";
    }
    else if (_flag & VK_DEBUG_REPORT_WARNING_BIT_EXT)
    {
        ss << "Vulkan: WARNING: ";
    }
    else if (_flag & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
    {
        ss << "Vulkan: PERFORMANCE_WARNING: ";
    }
    else if (_flag & VK_DEBUG_REPORT_ERROR_BIT_EXT)
    {
        ss << "Vulkan: ERROR: ";
    }
    else if (_flag & VK_DEBUG_REPORT_DEBUG_BIT_EXT)
    {
        ss << "Vulkan: DEBUG: ";
    }

    ss << _layer_prefix << ": ";
    ss << _message << " [ Message code: " << _message_code;

    ss << ", Obejct type: ";
    switch (_object_type)
    {
    case VK_DEBUG_REPORT_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION_EXT   : ss << OBJECT_TYPE_NAMES[34]; break;
    case VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE_EXT : ss << OBJECT_TYPE_NAMES[35]; break;
    case VK_DEBUG_REPORT_OBJECT_TYPE_ACCELERATION_STRUCTURE_NV_EXT  : ss << OBJECT_TYPE_NAMES[36]; break;
    default                                                         : ss << OBJECT_TYPE_NAMES[_object_type]; break;
    }
    ss << ", Obejct handle: "; hlp::PrintHex(ss, _object);
    ss << ", Location: " << std::dec << _location << " ]";

    b3d_data->msg_queue->AddMessageFromVkDebugReportCallbacks(GetB3DMessageSeverity(_flag), GetNativeCategoryFlags(_object_type), ss.str().c_str());

    // https://vulkan.lunarg.com/doc/view/1.0.37.0/linux/vkspec.chunked/ch32s02.html
    // このコールバックの戻り値は、検証メッセージの原因となったVulkan呼び出しが中止されるかどうかを制御します。
    // 検証メッセージを中止させるVulkan呼び出しを望まないため、VK_FALSEを返します。
    // 代わりに呼び出しを中止したい場合は、VK_TRUEを渡すと、関数はVK_ERROR_VALIDATION_FAILED_EXTを返します。
    return VK_FALSE;
}

VKAPI_ATTR VkBool32
VKAPI_CALL DebugUtilsMessengerCallback
(
      VkDebugUtilsMessageSeverityFlagBitsEXT        _message_severity
    , VkDebugUtilsMessageTypeFlagsEXT               _message_types
    , const VkDebugUtilsMessengerCallbackDataEXT*   _callback_data
    , void*                                         _user_data
)
{
    static const char* OBJECT_NAMES[] =
    {
          "Unknown"
        , "VkInstance"
        , "VkPhysicalDevice"
        , "VkDevice"
        , "VkQueue"
        , "VkSemaphore"
        , "VkCommandBuffer"
        , "VkFence"
        , "VkDeviceMemory"
        , "VkBuffer"
        , "VkImage"
        , "VkEvent"
        , "VkQueryPool"
        , "VkBufferView"
        , "VkImageView"
        , "VkShaderModule"
        , "VkPipelineCache"
        , "VkPipelineLayout"
        , "VkRenderPass"
        , "VkPipeline"
        , "VkDescriptorSetLayout"
        , "VkSampler"
        , "VkDescriptorPool"
        , "VkDescriptorSet"
        , "VkFramebuffer"
        , "VkCommandPool"

        , "VkSamplerYcbcrConversion"         // 1000156000,->[26]
        , "VkDescriptorUpdateTemplate"       // 1000085000,->[27]
        , "VkSurfaceKHR"                     // 1000000000,->[28]
        , "VkSwapchainKHR"                   // 1000001000,->[29]
        , "VkDisplayKHR"                     // 1000002000,->[30]
        , "VkDisplayModeKHR"                 // 1000002001,->[31]
        , "VkDebugReportCallbackEXT"         // 1000011000,->[32]
        , "VkDebugUtilsMessengerEXT"         // 1000128000,->[33]
        , "VkAccelerationStructureKHR"       // 1000165000,->[34]
        , "VkValidationCacheEXT"             // 1000160000,->[35]
        , "VkPerformanceConfigurationINTEL"  // 1000210000,->[36]
        , "VkDeferredOperationKHR"           // 1000268000,->[37]
        , "VkIndirectCommandsLayoutNV"       // 1000277000,->[38]
        , "VkPrivateDataSlotEXT"             // 1000295000,->[39]
    };

    auto b3d_data = (buma3d::DEBUG_REPORT_USER_DATA*)(_user_data);
    std::lock_guard lock(b3d_data->msg_output_mutex);

    auto&& ss = b3d_data->ss;
    ss.str("");
    ss.clear();

    switch (_message_severity)
    {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT : ss << "Vulkan: VERBOSE: "; break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT    : ss << "Vulkan: INFO: ";    break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT : ss << "Vulkan: WARNING: "; break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT   : ss << "Vulkan: ERROR: ";   break;
    default:
        break;
    }
    ss << (_callback_data->pMessage ? _callback_data->pMessage : "(None)");

    ss << " [ Message: Number: "; hlp::PrintHex(ss, _callback_data->messageIdNumber);
    {
        ss << ", Type: ";
        bool has_bit = false;
        if (_message_types & VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT)       { ss << "GENERAL";                                  has_bit = true; }
        if (_message_types & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT)    { ss << (has_bit ? "|VALIDATION"  : "VALIDATION");  has_bit = true; }
        if (_message_types & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT)   { ss << (has_bit ? "|PERFORMANCE" : "PERFORMANCE"); has_bit = true; }
    
        ss << ", Name: " << (_callback_data->pMessageIdName ? _callback_data->pMessageIdName : "(None)");
    }
    ss << " ] ";

    if (_callback_data->objectCount)
    {
        ss << "[ Obejct: ";
        for (uint32_t i = 0; i < _callback_data->objectCount; i++)
        {
            auto&& obj = _callback_data->pObjects[i];
            ss << "#" << i << ": ";
            ss << "Type: ";
            switch (obj.objectType)
            {
            case VK_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION        : ss << OBJECT_NAMES[26]; break;
            case VK_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE      : ss << OBJECT_NAMES[27]; break;
            case VK_OBJECT_TYPE_SURFACE_KHR                     : ss << OBJECT_NAMES[28]; break;
            case VK_OBJECT_TYPE_SWAPCHAIN_KHR                   : ss << OBJECT_NAMES[29]; break;
            case VK_OBJECT_TYPE_DISPLAY_KHR                     : ss << OBJECT_NAMES[30]; break;
            case VK_OBJECT_TYPE_DISPLAY_MODE_KHR                : ss << OBJECT_NAMES[31]; break;
            case VK_OBJECT_TYPE_DEBUG_REPORT_CALLBACK_EXT       : ss << OBJECT_NAMES[32]; break;
            case VK_OBJECT_TYPE_DEBUG_UTILS_MESSENGER_EXT       : ss << OBJECT_NAMES[33]; break;
            case VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_KHR      : ss << OBJECT_NAMES[34]; break;
            case VK_OBJECT_TYPE_VALIDATION_CACHE_EXT            : ss << OBJECT_NAMES[35]; break;
            case VK_OBJECT_TYPE_PERFORMANCE_CONFIGURATION_INTEL : ss << OBJECT_NAMES[36]; break;
            case VK_OBJECT_TYPE_DEFERRED_OPERATION_KHR          : ss << OBJECT_NAMES[37]; break;
            case VK_OBJECT_TYPE_INDIRECT_COMMANDS_LAYOUT_NV     : ss << OBJECT_NAMES[38]; break;
            case VK_OBJECT_TYPE_PRIVATE_DATA_SLOT_EXT           : ss << OBJECT_NAMES[39]; break;

            default:
                ss << OBJECT_NAMES[obj.objectType];
                break;
            }
            ss << ", Handle: "; hlp::PrintHex(ss, obj.objectHandle);
            ss << ", Name: ";
            if (obj.pObjectName)
                ss << "\"" << obj.pObjectName << "\", ";
            else
                ss << "(Unnamed)" << ", ";
        }
        ss << " ] ";
    }

    if (_callback_data->queueLabelCount)
    {
        ss << "[ Queue label name: ";
        for (uint32_t i = 0; i < _callback_data->queueLabelCount; i++)
        {
            auto&& label = _callback_data->pQueueLabels[i];
            ss << "#" << i << ": ";
            if (label.pLabelName)
                ss << "\"" << label.pLabelName << "\", ";
            else
                ss << "(Unnamed)" << ", ";
        }
        ss << " ] ";
    }

    if (_callback_data->cmdBufLabelCount)
    {
        ss << "[ Command buffer label name: " << ": ";
        for (uint32_t i = 0; i < _callback_data->cmdBufLabelCount; i++)
        {
            auto&& label = _callback_data->pCmdBufLabels[i];
            ss << "#" << i << ": ";
            if (label.pLabelName)
                ss << "\"" << label.pLabelName << "\", ";
            else
                ss << "(Unnamed)" << ", ";
        }
        ss << " ] ";
    }

    b3d_data->msg_queue->AddMessageFromVkDebugReportCallbacks(GetB3DMessageSeverityForDebugUtils(_message_severity), GetNativeCategoryFlagsForDebugUtils(_message_types), ss.str().c_str());

    return VK_FALSE;
}


}// anonymous namespace 
}// namespace buma3d

namespace buma3d
{

#pragma region IVulkanDebugCallbackObject

struct DeviceFactoryVk::IVulkanDebugCallbackObject : public util::details::NEW_DELETE_OVERRIDE
{
    IVulkanDebugCallbackObject() {}
    virtual ~IVulkanDebugCallbackObject() {}

    virtual BMRESULT Init() = 0;

};

class DeviceFactoryVk::VulkanDebugCallbackObjectDebugUtils : public IVulkanDebugCallbackObject
{
public:
    VulkanDebugCallbackObjectDebugUtils(DeviceFactoryVk* _owner)
        : owner                 { _owner }
        , user_data             { _owner->user_data.get() }
        , debug_utils_messenger {}
    {
    }

    ~VulkanDebugCallbackObjectDebugUtils()
    {
        // (呼び出し中の場合)コールバック関数の完了を待機
        while (!user_data->msg_output_mutex.try_lock()) {}
        user_data->msg_output_mutex.unlock();

        if (debug_utils_messenger)
            owner->inspfn->vkDestroyDebugUtilsMessengerEXT(owner->instance, debug_utils_messenger, owner->GetVkAllocationCallbacks());
        debug_utils_messenger = VK_NULL_HANDLE;
    }

    BMRESULT Init() override
    {
        VkDebugUtilsMessengerCreateInfoEXT dbg_msgr_ci{ VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT };
        dbg_msgr_ci.flags = 0;
        dbg_msgr_ci.messageSeverity = GetNativeDebugReportFlagsForDebugUtils(owner->desc.debug.num_debug_messages, owner->desc.debug.debug_messages);
        dbg_msgr_ci.messageType =
            VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

        dbg_msgr_ci.pfnUserCallback = DebugUtilsMessengerCallback;
        dbg_msgr_ci.pUserData       = user_data;

        auto vkr = owner->inspfn->vkCreateDebugUtilsMessengerEXT(owner->instance, &dbg_msgr_ci, owner->GetVkAllocationCallbacks(), &debug_utils_messenger);
        if (hlp::IsFailedVk(vkr))
            return BMRESULT_FAILED;

        return BMRESULT_SUCCEED;
    }

private:
    DeviceFactoryVk*            owner;
    DEBUG_REPORT_USER_DATA*     user_data;
    VkDebugUtilsMessengerEXT    debug_utils_messenger;

};

class DeviceFactoryVk::VulkanDebugCallbackObjectDebugReport : public IVulkanDebugCallbackObject
{
public:
    VulkanDebugCallbackObjectDebugReport(DeviceFactoryVk* _owner)
        : owner                 { _owner }
        , user_data             { _owner->user_data.get() }
        , debug_report_callback {}
    {
    }

    ~VulkanDebugCallbackObjectDebugReport()
    {
        // (呼び出し中の場合)コールバック関数の完了を待機
        while (!user_data->msg_output_mutex.try_lock()) {}
        user_data->msg_output_mutex.unlock();

        if (debug_report_callback)
            owner->inspfn->vkDestroyDebugReportCallbackEXT(owner->instance, debug_report_callback, owner->GetVkAllocationCallbacks());
        debug_report_callback = VK_NULL_HANDLE;
    }

    BMRESULT Init() override
    {
        // デバッグレポート用デバッグコールバック関数を設定
        VkDebugReportCallbackCreateInfoEXT dbg_rep_ci{ VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT };
        dbg_rep_ci.flags       = GetNativeDebugReportFlags(owner->desc.debug.num_debug_messages, owner->desc.debug.debug_messages);
        dbg_rep_ci.pfnCallback = DebugReportCallback;
        dbg_rep_ci.pUserData   = user_data;

        auto vkr = owner->inspfn->vkCreateDebugReportCallbackEXT(owner->instance, &dbg_rep_ci, owner->GetVkAllocationCallbacks(), &debug_report_callback);
        if (hlp::IsFailedVk(vkr))
            return BMRESULT_FAILED;

        return BMRESULT_SUCCEED;
    }

private:
    DeviceFactoryVk*            owner;
    DEBUG_REPORT_USER_DATA*     user_data;
    VkDebugReportCallbackEXT    debug_report_callback;

};

#pragma endregion IVulkanDebugCallbackObject

B3D_APIENTRY DeviceFactoryVk::DeviceFactoryVk()
    : ref_count             { 1 }
    , name                  {}
    , desc                  {}
    , message_queue         {}
    , physical_devices      {}
    , instance              {}
    , all_layers            {}
    , all_extensions        {}
    , enabled_layers        {}
    , enabled_extensions    {}
    , inspfn                {}
    , api_version           {}    
    , user_data             {}
    , alloc_callbacks       {}
    , debug_callback        {}
{

}

B3D_APIENTRY DeviceFactoryVk::~DeviceFactoryVk()
{
    Uninit();
}

BMRESULT
B3D_APIENTRY DeviceFactoryVk::Init(const DEVICE_FACTORY_DESC& _desc)
{
    CopyDesc(_desc);

    if (desc.debug.is_enabled && buma3d::IsEnableAllocatorDebug())
        SetAllocationCallbacks();

    if (desc.debug.is_enabled)
        B3D_RET_IF_FAILED(CreateDebugMessageQueue());

    B3D_RET_IF_FAILED(CreateInstance());

    inspfn = B3DMakeUniqueArgs(InstancePFN, this);

    if (desc.debug.is_enabled) 
        B3D_RET_IF_FAILED(SetDebugLayer());
    
    B3D_RET_IF_FAILED(EnumPhysicalDevices());

    return BMRESULT_SUCCEED;
}

void 
B3D_APIENTRY DeviceFactoryVk::CopyDesc(const DEVICE_FACTORY_DESC& _desc)
{
    desc = _desc;
    if (_desc.debug.num_debug_messages != 0)
        desc.debug.debug_messages = util::MemCopyArray(B3DNewArray(DEBUG_MESSAGE_DESC, _desc.debug.num_debug_messages)
                                                       , _desc.debug.debug_messages, _desc.debug.num_debug_messages);
}

void 
B3D_APIENTRY DeviceFactoryVk::GetInstanceLayerNames(util::DyArray<util::SharedPtr<util::String>>* _dst_all_layer_names) 
{
    uint32_t count = 0;
    // デバイスレイヤプロパティを取得
    vkEnumerateInstanceLayerProperties(&count, nullptr);
    util::DyArray<VkLayerProperties> layer_props((size_t)count);
    vkEnumerateInstanceLayerProperties(&count, layer_props.data());

    for (auto& i : layer_props)
    {
        _dst_all_layer_names->emplace_back(B3DMakeSharedArgs(util::String, i.layerName));
    }

    if constexpr (IS_ENABLE_DEBUG_OUTPUT)
    {
        if (util::IsEnabledDebug(this))
        {
            util::StringStream ss;
            for (auto& i : layer_props)
            {
                ss << "VkLayerProperties(instance level)" << std::endl;
                ss << "\tlayerName            : " << i.layerName << std::endl;
                ss << "\tspecVersion          : " << util::GetVulkanVersionString(i.specVersion) << std::endl;
                ss << "\timplementationVersion: " << i.implementationVersion << std::endl;
                ss << "\tdescription          : " << i.description << std::endl;
            }
            util::AddDebugMessage(this, DEBUG_MESSAGE_SEVERITY_INFO, DEBUG_MESSAGE_CATEGORY_FLAG_MISCELLANEOUS, ss.str().c_str());
        }
    }
}

void 
B3D_APIENTRY DeviceFactoryVk::GetInstanceExtensionNames(const util::DyArray<util::SharedPtr<util::String>>& _all_layer_names, util::DyArray<util::SharedPtr<util::String>>* _dst_all_ext_names) 
{
    for (auto& i : _all_layer_names)
    {
        uint32_t count = 0;
        // デバイス拡張プロパティを取得
        vkEnumerateInstanceExtensionProperties(i->c_str(), &count, nullptr);
        util::DyArray<VkExtensionProperties> ext_props(count);
        vkEnumerateInstanceExtensionProperties(i->c_str(), &count, ext_props.data());

        for (auto& i_ext : ext_props)
        {
            _dst_all_ext_names->emplace_back(B3DMakeSharedArgs(util::String, i_ext.extensionName));
        }

        if constexpr (IS_ENABLE_DEBUG_OUTPUT)
        {
            if (util::IsEnabledDebug(this))
            {
                util::StringStream ss;
                ss << "VkExtensionProperties(instance level)" << std::endl;
                ss << "layerName: " << *i << std::endl;
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
                util::AddDebugMessage(this, DEBUG_MESSAGE_SEVERITY_INFO, DEBUG_MESSAGE_CATEGORY_FLAG_MISCELLANEOUS, ss.str().c_str());
            }
        }
    }

    // pLayerNameパラメータがNULLの場合、Vulkan実装または暗黙的に有効化されたレイヤーによって提供される拡張機能のみが返されます。
    uint32_t num_implicit_ext_props = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &num_implicit_ext_props, nullptr);
    util::DyArray<VkExtensionProperties> implicit_ext_props(num_implicit_ext_props);
    vkEnumerateInstanceExtensionProperties(nullptr, &num_implicit_ext_props, implicit_ext_props.data());

    // 暗黙で有効の拡張名も追加
    for (auto& it_props : implicit_ext_props)
        _dst_all_ext_names->emplace_back(B3DMakeSharedArgs(util::String, it_props.extensionName));

    if constexpr (IS_ENABLE_DEBUG_OUTPUT)
    {
        if (util::IsEnabledDebug(this)) 
        {
            util::StringStream ss;
            ss << "Implicit(nullptr) layer properties(instance level)" << std::endl;
            for (auto& i : implicit_ext_props)
            {
                ss << "\textensionName: " << i.extensionName << std::endl;
                ss << "\tspecVersion  : " << i.specVersion << std::endl;
            }
            ss << std::endl;
            util::AddDebugMessage(this, DEBUG_MESSAGE_SEVERITY_INFO, DEBUG_MESSAGE_CATEGORY_FLAG_MISCELLANEOUS
                                  , ss.str().c_str());
        }
    }
}

bool 
B3D_APIENTRY DeviceFactoryVk::CheckInstanceLayerSupport(const util::DyArray<util::SharedPtr<util::String>>& _all_layer_names, const char* _request_layer_name)
{
    auto fn = [_request_layer_name](const util::SharedPtr<util::String>& _str) { return *_str == _request_layer_name; };
    if (std::find_if(_all_layer_names.begin(), _all_layer_names.end(), fn) != _all_layer_names.end())
        return true;

    B3D_ADD_DEBUG_MSG2(DEBUG_MESSAGE_SEVERITY_WARNING, DEBUG_MESSAGE_CATEGORY_FLAG_INITIALIZATION
                       , __FUNCTION__, ": Requested instance layer \"", _request_layer_name, "\" NOT supported.\n");

    return false;
}

bool 
B3D_APIENTRY DeviceFactoryVk::CheckInstanceExtensionSupport(const util::DyArray<util::SharedPtr<util::String>>& _all_ext_names, const char* _request_ext_name)
{
    auto fn = [_request_ext_name](const util::SharedPtr<util::String>& _str) { return *_str == _request_ext_name; };
    if (std::find_if(_all_ext_names.begin(), _all_ext_names.end(), fn) != _all_ext_names.end())
        return true;

    B3D_ADD_DEBUG_MSG2(DEBUG_MESSAGE_SEVERITY_WARNING, DEBUG_MESSAGE_CATEGORY_FLAG_INITIALIZATION
                       , __FUNCTION__, ": Requested instance extension \"", _request_ext_name, "\" NOT supported.\n");

    return false;
}

void 
B3D_APIENTRY DeviceFactoryVk::SetAllocationCallbacks()
{
    // vulkanのメモリ割り当て時デバッグコールバック関数を設定
    alloc_callbacks                        = B3DNew(VkAllocationCallbacks);
    alloc_callbacks->pfnAllocation         = MAllocVk;
    alloc_callbacks->pfnFree               = FreeVk;
    alloc_callbacks->pfnReallocation       = ReallocVk;
    alloc_callbacks->pfnInternalAllocation = InternalAllocationNotification;
    alloc_callbacks->pfnInternalFree       = InternalFreeNotification;
    alloc_callbacks->pUserData             = user_data.get();
}

BMRESULT 
B3D_APIENTRY DeviceFactoryVk::CreateInstance()
{
    // Vulkanバージョンの取得
    VkResult vkr;
    vkr = vkEnumerateInstanceVersion(&api_version);
    if constexpr (IS_ENABLE_DEBUG_OUTPUT)
    {
        if (util::IsEnabledDebug(this))
        {
            util::StringStream ss;
            ss << "Vulkan API version: ";
            ss << util::GetVulkanVersionString(api_version) << std::endl;
            util::AddDebugMessage(this, DEBUG_MESSAGE_SEVERITY_INFO, DEBUG_MESSAGE_CATEGORY_FLAG_MISCELLANEOUS
                                  , ss.str().c_str());
        }
    }

    // インスタンスを作成
    VkInstanceCreateInfo create_info{ VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
    create_info.flags = 0/*reserved*/;

    // アプリケーション
    VkApplicationInfo appinfo{ VK_STRUCTURE_TYPE_APPLICATION_INFO };
    appinfo.pApplicationName     = "Buma3D";
    appinfo.applicationVersion   = VK_MAKE_VERSION(0, 0, 1);
    appinfo.pEngineName          = "Buma3D";
    appinfo.engineVersion        = VK_MAKE_VERSION(0, 0, 1);
    appinfo.apiVersion           = VK_API_VERSION_1_2;// アプリケーションがサポートする最も高いVulkanAPIバージョン
    create_info.pApplicationInfo = &appinfo;

    // レイヤーの設定
    // NOTE: 実装(vulkan側)は、動作の違いのために一緒に有効にできない拡張機能のペア、またはアドバタイズ(提供)されたバージョンに対して有効にできない拡張機能のペアをアドバタイズしてはなりません。
    // レイヤ、拡張機能を取得
    GetInstanceLayerNames(&all_layers);
    GetInstanceExtensionNames(all_layers, &all_extensions);

    // インスタンスレイヤを設定

    // 有効化するレイヤ
    const char* DEBUG_LAYERS_TO_ENABLE[] =
    {

        /* https://vulkan.lunarg.com/doc/view/1.1.101.0/windows/api_dump_layer.html
        VK_LAYER_LUNARG_api_dumpユーティリティレイヤーは、API呼び出し、パラメーター、および値を、識別された出力ストリームに出力します。
        環境変数またはvk_layer_settings.txtファイルを使用して調整できるいくつかの設定があります。*/
        //, "VK_LAYER_LUNARG_api_dump"

        /*https://vulkan.lunarg.com/doc/view/1.1.106.0/windows/khronos_validation_layer.html
        VK_LAYER_KHRONOS_validationレイヤーは、次の検証領域をサポートしています。
        Thread safety validation, Stateless parameter validation, Object lifetime validation, Core validation checks,
        GPU-Assisted validation , Best practices validation  , Handle wrapping functionality*/
        "VK_LAYER_KHRONOS_validation"

        /*https://vulkan.lunarg.com/doc/view/1.2.131.2/linux/monitor_layer.html
        VK_LAYER_LUNARG_monitorユーティリティレイヤーは、アプリケーションのタイトルバーにリアルタイムフレーム/秒の値を出力します。*/
        //, "VK_LAYER_LUNARG_monitor"

        /*https://vulkan.lunarg.com/doc/view/1.0.13.0/windows/layers.html
        この省略形のレイヤー定義を指定すると、検証レイヤーの標準セットが最適な順序で読み込まれます。*/
        //, "VK_LAYER_LUNARG_standard_validation"

        /*https://vulkan.lunarg.com/doc/view/1.2.131.2/linux/trace_tools.html
        サーバーモードでトレースする場合、ローカルまたはリモートクライアントはVktraceレイヤーを有効にする必要があります。*/
        //, "VK_LAYER_LUNARG_vktrace"
    };
    //const char* LAYERS_TO_ENABLE[] =
    //{
    // /*VK_LAYER_NV_optimusレイヤーにより、NVIDIA GPUが最初に列挙されるようにGPUがソートされます。*/
    // //, "VK_LAYER_NV_optimus"
    //
    // /*https://vulkan.lunarg.com/doc/view/1.0.68.0/windows/device_simulation_layer.html
    // このレイヤーを使用すると、クエリによって報告される機能の一部を削除することで、より能力の高いデバイスが能力の低いデバイスをシミュレートできます。
    // システムの基盤となる実際のデバイスにまだ存在していないエミュレートされた機能をレイヤーが追加することはできません。
    // DevSimでは、能力の低いデバイスが能力の高いデバイスをエミュレートすることはできません。*/
    // //, "VK_LAYER_LUNARG_device_simulation"
    //
    // /*https://vulkan.lunarg.com/doc/view/1.2.131.2/linux/screenshot_layer.html
    // VK_LAYER_LUNARG_screenshotレイヤーは、フレームを画像ファイルに記録します。*/
    // //, "VK_LAYER_LUNARG_screenshot"
    //};

    // 有効化するレイヤーのセット
    util::DyArray<const char*> enable_layer_names_ary;
    if (desc.debug.is_enabled)
    {
        for (auto& i : DEBUG_LAYERS_TO_ENABLE)
        {
            if (CheckInstanceLayerSupport(all_layers, i))
                enable_layer_names_ary.emplace_back(i);
        }
    }
    create_info.enabledLayerCount = (uint32_t)enable_layer_names_ary.size();
    create_info.ppEnabledLayerNames = enable_layer_names_ary.data();

    // 有効化するインスタンス拡張
    const char* DEBUG_INST_EXT_TO_ENABLE[] =
    {
        // VK_LAYER_KHRONOS_validation
          VK_EXT_DEBUG_REPORT_EXTENSION_NAME
        , VK_EXT_DEBUG_UTILS_EXTENSION_NAME
        , VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME
    };
    const char* INST_EXT_TO_ENABLE[] =
    {
          VK_KHR_DEVICE_GROUP_CREATION_EXTENSION_NAME
        , VK_KHR_EXTERNAL_FENCE_CAPABILITIES_EXTENSION_NAME
        , VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME
        , VK_KHR_EXTERNAL_SEMAPHORE_CAPABILITIES_EXTENSION_NAME
        , VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME
        , VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME
        , VK_KHR_SURFACE_EXTENSION_NAME
        , VK_KHR_SURFACE_PROTECTED_CAPABILITIES_EXTENSION_NAME
        , VK_EXT_SWAPCHAIN_COLOR_SPACE_EXTENSION_NAME
        , VK_NV_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME
    #ifdef VK_USE_PLATFORM_WIN32_KHR
        , VK_KHR_WIN32_SURFACE_EXTENSION_NAME
    #endif
    };
    util::DyArray<const char*> enable_ext_names_ary;
    // 有効化するレイヤーのインスタンス拡張のセット
    if (desc.debug.is_enabled)
    {
        for (auto& i : DEBUG_INST_EXT_TO_ENABLE)
        {
            if (CheckInstanceExtensionSupport(all_extensions, i))
                enable_ext_names_ary.emplace_back(i);
        }
    }
    for (auto& i : INST_EXT_TO_ENABLE)
    {
        if (CheckInstanceExtensionSupport(all_extensions, i))
            enable_ext_names_ary.emplace_back(i);
    }
    create_info.enabledExtensionCount   = (uint32_t)enable_ext_names_ary.size();
    create_info.ppEnabledExtensionNames = enable_ext_names_ary.data();

    if constexpr (IS_ENABLE_DEBUG_OUTPUT)
    {
        if (util::IsEnabledDebug(this))
        {
            util::StringStream ss;
            ss << "Enabled instance layers" << std::endl;
            for (auto& i : enable_layer_names_ary)
                ss << "\t" << i << std::endl;
            ss << "Enabled instance extensions" << std::endl;
            for (auto& i : enable_ext_names_ary)
                ss << "\t" << i << std::endl;
            util::AddDebugMessage(this, DEBUG_MESSAGE_SEVERITY_INFO, DEBUG_MESSAGE_CATEGORY_FLAG_MISCELLANEOUS 
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

        enabled_layers.reserve(enable_layer_names_ary.size());
        for (auto& i : enable_layer_names_ary)
            PushIfFound(all_layers, &enabled_layers, i);

        enabled_extensions.reserve(enable_ext_names_ary.size());
        for (auto& i : enable_ext_names_ary)
            PushIfFound(all_extensions, &enabled_extensions, i);
    }

    // pNextチェイン

    // 検証機能の設定
    VkValidationFeaturesEXT validation_features{};
    validation_features.sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT;
    validation_features.pNext = nullptr;

    // 有効にする検証機能
    util::DyArray<VkValidationFeatureEnableEXT> vfenables;
    if (desc.debug.is_enabled)
    {
        vfenables.emplace_back(VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT);

        // gpu検証
        if (desc.debug.gpu_based_validation.is_enabled)
        {
            vfenables.emplace_back(VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT);
            vfenables.emplace_back(VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_RESERVE_BINDING_SLOT_EXT);
        }
    }
    validation_features.enabledValidationFeatureCount = (uint32_t)vfenables.size();
    validation_features.pEnabledValidationFeatures = vfenables.data();

    // 無効にする検証機能
    util::DyArray<VkValidationFeatureDisableEXT> vfdisables;
    //vfdisables.emplace_back(VK_VALIDATION_FEATURE_DISABLE_ALL_EXT);
    validation_features.disabledValidationFeatureCount = (uint32_t)vfdisables.size();
    validation_features.pDisabledValidationFeatures = vfdisables.data();

    // 無効にする検証チェック
    VkValidationCheckEXT check_disable[] = { VK_VALIDATION_CHECK_ALL_EXT };
    VkValidationFlagsEXT validation_flags{ VK_STRUCTURE_TYPE_VALIDATION_FLAGS_EXT };
    validation_flags.disabledValidationCheckCount = 1;
    validation_flags.pDisabledValidationChecks    = check_disable;

    /* (Vulkan側の)実装のコンポーネント(ローダー、有効なレイヤー、ドライバー)は、そのコンポーネントがサポートする
    拡張機能によって定義されていないsType値を持つチェーン内の構造を処理せずにスキップする必要があります。*/

    // pNextを接続
    create_info.pNext = &validation_features;
    if (desc.debug.is_enabled == false)
        validation_features.pNext = &validation_flags;

    vkr = vkCreateInstance(&create_info, B3D_VK_ALLOC_CALLBACKS, &instance);
    VKR_TRACE_IF_FAILED(vkr);

    if (hlp::IsFailedVk(vkr))
        return BMRESULT_FAILED;

    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY DeviceFactoryVk::EnumPhysicalDevices()
{
    VkResult vkr;
    // 物理デバイスを列挙
    uint32_t num_physical_devices = 0;
    vkr = vkEnumeratePhysicalDevices(instance, &num_physical_devices, nullptr);
    if (num_physical_devices == 0)
    {
        B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_INITIALIZATION, "使用可能なデバイスはありません。");
        return BMRESULT_FAILED;
    }

    physical_devices.resize((size_t)num_physical_devices);
    vkr = vkEnumeratePhysicalDevices(instance, &num_physical_devices, physical_devices.data());
    B3D_RET_IF_FAILED(VKR_TRACE_IF_FAILED(vkr));

    // デバイスグループを列挙
    uint32_t num_dg_props = 0;
    vkr = vkEnumeratePhysicalDeviceGroups(instance, &num_dg_props, nullptr);
    if (num_dg_props)
    {
        physical_device_group_properties.resize(num_dg_props, { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GROUP_PROPERTIES });
        vkr = vkEnumeratePhysicalDeviceGroups(instance, &num_dg_props, physical_device_group_properties.data());
        B3D_RET_IF_FAILED(VKR_TRACE_IF_FAILED(vkr));

        // 単一GPUのみで構成されているデバイスグループは使用しない。
        if (physical_device_group_properties.size())
        {
            auto&& remove_it = std::remove_if(physical_device_group_properties.begin(), physical_device_group_properties.end()
                                              , [](const VkPhysicalDeviceGroupProperties& _prop) { return _prop.physicalDeviceCount <= 1; });

            if (remove_it != physical_device_group_properties.end())
                physical_device_group_properties.erase(remove_it, physical_device_group_properties.end());
        }
    }

    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY DeviceFactoryVk::SetDebugLayer()
{
    // ユーザーデータを作成。
    user_data = B3DMakeUnique(DEBUG_REPORT_USER_DATA);
    user_data->desc      = &desc;
    user_data->msg_queue = message_queue.Get();

    if (CheckInstanceExtensionEnabled(VK_EXT_DEBUG_UTILS_EXTENSION_NAME))
        debug_callback = B3DNewArgs(VulkanDebugCallbackObjectDebugUtils, this);

    else if (CheckInstanceExtensionEnabled(VK_EXT_DEBUG_REPORT_EXTENSION_NAME))
        debug_callback = B3DNewArgs(VulkanDebugCallbackObjectDebugReport, this);

    else
    {
        B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_INITIALIZATION, __FUNCTION__": デバッグレイヤーはサポートされていません。");
        return BMRESULT_FAILED_NOT_SUPPORTED;
    }

    return debug_callback->Init();
}

BMRESULT
B3D_APIENTRY DeviceFactoryVk::CreateDebugMessageQueue()
{
    B3D_RET_IF_FAILED(DebugMessageQueueVk::Create(this, &message_queue));
    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY DeviceFactoryVk::Uninit()
{
    api_version = 0;

    B3DSafeDelete(debug_callback);

    hlp::SwapClear(physical_devices);
    hlp::SwapClear(enabled_layers);
    hlp::SwapClear(enabled_extensions);

    // (呼び出し中の場合)コールバック関数の完了を待機
    if (user_data)
    {
        while (user_data->msg_output_mutex.try_lock()) {}
        user_data->msg_output_mutex.unlock();
        user_data.reset();
    }

    inspfn.reset();

    if (instance)
        vkDestroyInstance(instance, B3D_VK_ALLOC_CALLBACKS);
    instance = VK_NULL_HANDLE;

    B3DSafeDelete(alloc_callbacks);
    B3DSafeDeleteArray(desc.debug.debug_messages);
    message_queue.Reset();
    name.reset();
}

BMRESULT 
B3D_APIENTRY DeviceFactoryVk::Create(const DEVICE_FACTORY_DESC& _desc, DeviceFactoryVk** _dst)
{
    util::Ptr<DeviceFactoryVk> ptr;
    ptr.Attach(B3DCreateImplementationClass(DeviceFactoryVk));
    B3D_RET_IF_FAILED(ptr->Init(_desc));
    
    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY DeviceFactoryVk::AddRef() 
{
    ++ref_count;
    B3D_REFCOUNT_DEBUG(ref_count);
}

uint32_t
B3D_APIENTRY DeviceFactoryVk::Release() 
{
    B3D_REFCOUNT_DEBUG(ref_count - 1);
    auto count = --ref_count;
    if (count == 0)
        B3DDestroyImplementationClass(this);

    return count;
}

uint32_t
B3D_APIENTRY DeviceFactoryVk::GetRefCount() const 
{
    return ref_count;
}

const char*
B3D_APIENTRY DeviceFactoryVk::GetName() const
{
    return name ? name->c_str() : nullptr;
}

BMRESULT 
B3D_APIENTRY DeviceFactoryVk::SetName(const char* _name)
{
    if (!util::IsEnabledDebug(this))
        return BMRESULT_FAILED;

    if (name && !_name)
        name.reset();
    else
        name = B3DMakeUniqueArgs(util::NameableObjStr, _name);

    return BMRESULT_SUCCEED;
}

const DEVICE_FACTORY_DESC& 
B3D_APIENTRY DeviceFactoryVk::GetDesc() const
{
    return desc;
}

BMRESULT 
B3D_APIENTRY DeviceFactoryVk::GetDebugMessageQueue(IDebugMessageQueue** _dst)
{
    if (!desc.debug.is_enabled || !message_queue)
        return BMRESULT_FAILED;

    (*_dst = message_queue.Get())->AddRef();
    return BMRESULT_SUCCEED;
}

BMRESULT 
B3D_APIENTRY DeviceFactoryVk::EnumAdapters(uint32_t _adapter_index, IDeviceAdapter** _dst_adapter)
{
    util::Ptr<DeviceAdapterVk> ptr;
    // デバイスグループアダプタは、非デバイスグループアダプタ(physical_devices)のインデックスの直後に配置します。
    if (_adapter_index >= physical_devices.size())
    {
        // 物理デバイスグループを列挙
        auto device_group_adapter_index = _adapter_index - physical_devices.size();
        if (device_group_adapter_index < physical_device_group_properties.size())
        {
            B3D_RET_IF_FAILED(DeviceAdapterVk::CreateForDeviceGroup(this, physical_device_group_properties.data()[device_group_adapter_index], &ptr));
        }
        else
        {
            return BMRESULT_FAILED_OUT_OF_RANGE;
        }
    }
    else
    {
        // 通常の物理デバイスを列挙
        B3D_RET_IF_FAILED(DeviceAdapterVk::Create(this, physical_devices.data()[_adapter_index], &ptr));
    }

    *_dst_adapter = ptr.Detach();
    return BMRESULT_SUCCEED;
}

BMRESULT 
B3D_APIENTRY DeviceFactoryVk::CreateDevice(const DEVICE_DESC& _desc, IDevice** _dst)
{
    util::Ptr<DeviceVk> ptr;
    B3D_RET_IF_FAILED(DeviceVk::Create(this, _desc, &ptr));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

VkInstance
B3D_APIENTRY DeviceFactoryVk::GetVkInstance() const
{
    return instance;
}

const VkAllocationCallbacks* 
B3D_APIENTRY DeviceFactoryVk::GetVkAllocationCallbacks() const
{
    return alloc_callbacks;
}

const InstancePFN& 
B3D_APIENTRY DeviceFactoryVk::GetInstancePFN() const
{
    return *inspfn;
}

uint32_t 
B3D_APIENTRY DeviceFactoryVk::GetInstanceAPIVersion() const
{
    return api_version;
}

const util::DyArray<util::SharedPtr<util::String>>& 
B3D_APIENTRY DeviceFactoryVk::GetEnabledInstanceLayers() const 
{
    return enabled_layers;
}

const util::DyArray<util::SharedPtr<util::String>>&
B3D_APIENTRY DeviceFactoryVk::GetEnabledInstanceExtensions() const 
{
    return enabled_extensions;
}

bool
B3D_APIENTRY DeviceFactoryVk::CheckInstanceLayerSupport(const char* _request_layer_name) const
{
    auto fn = [_request_layer_name](const util::SharedPtr<util::String>& _str) { return *_str == _request_layer_name; };
    return std::find_if(all_layers.begin(), all_layers.end(), fn) != all_layers.end();
}

bool
B3D_APIENTRY DeviceFactoryVk::CheckInstanceExtensionSupport(const char* _request_ext_name) const
{
    auto fn = [_request_ext_name](const util::SharedPtr<util::String>& _str) { return *_str == _request_ext_name; };
    return std::find_if(all_extensions.begin(), all_extensions.end(), fn) != all_extensions.end();
}

bool
B3D_APIENTRY DeviceFactoryVk::CheckInstanceLayerEnabled(const char* _request_layer_name) const
{
    auto fn = [_request_layer_name](const util::SharedPtr<util::String>& _str) { return *_str == _request_layer_name; };
    return std::find_if(enabled_layers.begin(), enabled_layers.end(), fn) != enabled_layers.end();
}

bool
B3D_APIENTRY DeviceFactoryVk::CheckInstanceExtensionEnabled(const char* _request_ext_name) const
{
    auto fn = [_request_ext_name](const util::SharedPtr<util::String>& _str) { return *_str == _request_ext_name; };
    return std::find_if(enabled_extensions.begin(), enabled_extensions.end(), fn) != enabled_extensions.end();
}

bool 
B3D_APIENTRY DeviceFactoryVk::IsEnabledDebug()
{
    return desc.debug.is_enabled;
}

void 
B3D_APIENTRY DeviceFactoryVk::AddMessageFromB3D(DEBUG_MESSAGE_SEVERITY _severity, DEBUG_MESSAGE_CATEGORY_FLAG _category, const Char8T* _str)
{
    message_queue->AddMessageFromB3D(_severity, _category, _str);
}


}// namespace buma3d
