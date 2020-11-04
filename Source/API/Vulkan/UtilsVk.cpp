#include "Buma3DPCH.h"
#include "UtilsVk.h"

namespace buma3d
{
namespace util
{
namespace /*anonymous*/
{

enum class B3D_VkResult
{
      VK_SUCCESS                                            = 0
    , VK_NOT_READY                                          = 1
    , VK_TIMEOUT                                            = 2
    , VK_EVENT_SET                                          = 3
    , VK_EVENT_RESET                                        = 4
    , VK_INCOMPLETE                                         = 5
    , VK_ERROR_OUT_OF_HOST_MEMORY                           = -1
    , VK_ERROR_OUT_OF_DEVICE_MEMORY                         = -2
    , VK_ERROR_INITIALIZATION_FAILED                        = -3
    , VK_ERROR_DEVICE_LOST                                  = -4
    , VK_ERROR_MEMORY_MAP_FAILED                            = -5
    , VK_ERROR_LAYER_NOT_PRESENT                            = -6
    , VK_ERROR_EXTENSION_NOT_PRESENT                        = -7
    , VK_ERROR_FEATURE_NOT_PRESENT                          = -8
    , VK_ERROR_INCOMPATIBLE_DRIVER                          = -9
    , VK_ERROR_TOO_MANY_OBJECTS                             = -10
    , VK_ERROR_FORMAT_NOT_SUPPORTED                         = -11
    , VK_ERROR_FRAGMENTED_POOL                              = -12
    , VK_ERROR_UNKNOWN                                      = -13
    , VK_ERROR_OUT_OF_POOL_MEMORY                           = -1000069000
    , VK_ERROR_INVALID_EXTERNAL_HANDLE                      = -1000072003
    , VK_ERROR_FRAGMENTATION                                = -1000161000
    , VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS               = -1000257000
    , VK_ERROR_SURFACE_LOST_KHR                             = -1000000000
    , VK_ERROR_NATIVE_WINDOW_IN_USE_KHR                     = -1000000001
    , VK_SUBOPTIMAL_KHR                                     = 1000001003
    , VK_ERROR_OUT_OF_DATE_KHR                              = -1000001004
    , VK_ERROR_INCOMPATIBLE_DISPLAY_KHR                     = -1000003001
    , VK_ERROR_VALIDATION_FAILED_EXT                        = -1000011001
    , VK_ERROR_INVALID_SHADER_NV                            = -1000012000
    , VK_ERROR_INCOMPATIBLE_VERSION_KHR                     = -1000150000
    , VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT = -1000158000
    , VK_ERROR_NOT_PERMITTED_EXT                            = -1000174001
    , VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT          = -1000255000
    , VK_THREAD_IDLE_KHR                                    = 1000268000
    , VK_THREAD_DONE_KHR                                    = 1000268001
    , VK_OPERATION_DEFERRED_KHR                             = 1000268002
    , VK_OPERATION_NOT_DEFERRED_KHR                         = 1000268003
    , VK_PIPELINE_COMPILE_REQUIRED_EXT                      = 1000297000
    , VK_ERROR_OUT_OF_POOL_MEMORY_KHR                       = VK_ERROR_OUT_OF_POOL_MEMORY
    , VK_ERROR_INVALID_EXTERNAL_HANDLE_KHR                  = VK_ERROR_INVALID_EXTERNAL_HANDLE
    , VK_ERROR_FRAGMENTATION_EXT                            = VK_ERROR_FRAGMENTATION
    , VK_ERROR_INVALID_DEVICE_ADDRESS_EXT                   = VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS
    , VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS_KHR           = VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS
    , VK_ERROR_PIPELINE_COMPILE_REQUIRED_EXT                = VK_PIPELINE_COMPILE_REQUIRED_EXT
    , VK_RESULT_MAX_ENUM                                    = 0x7FFFFFFF
};

}

util::String GetVkResultDescription(VkResult _vkr)
{
    util::String result;
    switch ((B3D_VkResult)_vkr)
    {
    #pragma region Success codes
    case B3D_VkResult::VK_SUCCESS:
        result += "VK_SUCCESS: ";
        result += "Command successfully completed";
        break;
    case B3D_VkResult::VK_NOT_READY:
        result += "VK_NOT_READY: ";
        result += "A fence or query has not yet completed";
        break;
    case B3D_VkResult::VK_TIMEOUT:
        result += "VK_TIMEOUT: ";
        result += "A wait operation has not completed in the specified time";
        break;
    case B3D_VkResult::VK_EVENT_SET:
        result += "VK_EVENT_SET: ";
        result += "An event is signaled";
        break;
    case B3D_VkResult::VK_EVENT_RESET:
        result += "VK_EVENT_RESET: ";
        result += "An event is unsignaled";
        break;
    case B3D_VkResult::VK_INCOMPLETE:
        result += "VK_INCOMPLETE: ";
        result += "A return array was too small for the result";
        break;
    case B3D_VkResult::VK_SUBOPTIMAL_KHR:
        result += "VK_SUBOPTIMAL_KHR: ";
        result += "A swapchain no longer matches the surface properties exactly, but can still be used to present to the surface successfully.";
        break;
    case B3D_VkResult::VK_THREAD_IDLE_KHR:
        result += "VK_THREAD_IDLE_KHR: ";
        result += "A deferred operation is not complete but there is currently no work for this thread to do at the time of this call.";
        break;
    case B3D_VkResult::VK_THREAD_DONE_KHR:
        result += "VK_THREAD_DONE_KHR: ";
        result += "A deferred operation is not complete but there is no work remaining to assign to additional threads.";
        break;
    case B3D_VkResult::VK_OPERATION_DEFERRED_KHR:
        result += "VK_OPERATION_DEFERRED_KHR: ";
        result += "A deferred operation was requested and at least some of the work was deferred.";
        break;
    case B3D_VkResult::VK_OPERATION_NOT_DEFERRED_KHR:
        result += "VK_OPERATION_NOT_DEFERRED_KHR: ";
        result += "A deferred operation was requested and no operations were deferred.";
        break;
    case B3D_VkResult::VK_PIPELINE_COMPILE_REQUIRED_EXT:
        result += "VK_PIPELINE_COMPILE_REQUIRED_EXT: ";
        result += "A requested pipeline creation would have required compilation, but the application requested compilation to not be performed.";
        break;
    #pragma endregion Success codes
    #pragma region Error codes
    case B3D_VkResult::VK_ERROR_OUT_OF_HOST_MEMORY:
        result += "VK_ERROR_OUT_OF_HOST_MEMORY: ";
        result += "A host memory allocation has failed.";
        break;
    case B3D_VkResult::VK_ERROR_OUT_OF_DEVICE_MEMORY:
        result += "VK_ERROR_OUT_OF_DEVICE_MEMORY: ";
        result += "A device memory allocation has failed.";
        break;
    case B3D_VkResult::VK_ERROR_INITIALIZATION_FAILED:
        result += "VK_ERROR_INITIALIZATION_FAILED: ";
        result += "Initialization of an object could not be completed for implementation-specific reasons.";
        break;
    case B3D_VkResult::VK_ERROR_DEVICE_LOST:
        result += "VK_ERROR_DEVICE_LOST: ";
        result += "The logical or physical device has been lost. See Lost Device";
        break;
    case B3D_VkResult::VK_ERROR_MEMORY_MAP_FAILED:
        result += "VK_ERROR_MEMORY_MAP_FAILED: ";
        result += "Mapping of a memory object has failed.";
        break;
    case B3D_VkResult::VK_ERROR_LAYER_NOT_PRESENT:
        result += "VK_ERROR_LAYER_NOT_PRESENT: ";
        result += "A requested layer is not present or could not be loaded.";
        break;
    case B3D_VkResult::VK_ERROR_EXTENSION_NOT_PRESENT:
        result += "VK_ERROR_EXTENSION_NOT_PRESENT: ";
        result += "A requested extension is not supported.";
        break;
    case B3D_VkResult::VK_ERROR_FEATURE_NOT_PRESENT:
        result += "VK_ERROR_FEATURE_NOT_PRESENT: ";
        result += "A requested feature is not supported.";
        break;
    case B3D_VkResult::VK_ERROR_INCOMPATIBLE_DRIVER:
        result += "VK_ERROR_INCOMPATIBLE_DRIVER: ";
        result += "The requested version of Vulkan is not supported by the driver or is otherwise incompatible for implementation-specific reasons.";
        break;
    case B3D_VkResult::VK_ERROR_TOO_MANY_OBJECTS:
        result += "VK_ERROR_TOO_MANY_OBJECTS: ";
        result += "Too many objects of the type have already been created.";
        break;
    case B3D_VkResult::VK_ERROR_FORMAT_NOT_SUPPORTED:
        result += "VK_ERROR_FORMAT_NOT_SUPPORTED: ";
        result += "A requested format is not supported on this device.";
        break;
    case B3D_VkResult::VK_ERROR_FRAGMENTED_POOL:
        result += "VK_ERROR_FRAGMENTED_POOL: ";
        result += "A pool allocation has failed due to fragmentation of the pool’s memory. This must only be returned if no attempt to allocate host or device memory was made to accommodate the new allocation. This should be returned in preference to VK_ERROR_OUT_OF_POOL_MEMORY, but only if the implementation is certain that the pool allocation failure was due to fragmentation.";
        break;
    case B3D_VkResult::VK_ERROR_SURFACE_LOST_KHR:
        result += "VK_ERROR_SURFACE_LOST_KHR: ";
        result += "A surface is no longer available.";
        break;
    case B3D_VkResult::VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
        result += "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR: ";
        result += "The requested window is already in use by Vulkan or another API in a manner which prevents it from being used again.";
        break;
    case B3D_VkResult::VK_ERROR_OUT_OF_DATE_KHR:
        result += "VK_ERROR_OUT_OF_DATE_KHR: ";
        result += "A surface has changed in such a way that it is no longer compatible with the swapchain, and further presentation requests using the swapchain will fail. Applications must query the new surface properties and recreate their swapchain if they wish to continue presenting to the surface.";
        break;
    case B3D_VkResult::VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
        result += "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR: ";
        result += "The display used by a swapchain does not use the same presentable image layout, or is incompatible in a way that prevents sharing an image.";
        break;
    case B3D_VkResult::VK_ERROR_INVALID_SHADER_NV:
        result += "VK_ERROR_INVALID_SHADER_NV: ";
        result += "One or more shaders failed to compile or link. More details are reported back to the application via VK_EXT_debug_report if enabled.";
        break;
    case B3D_VkResult::VK_ERROR_OUT_OF_POOL_MEMORY:
        result += "VK_ERROR_OUT_OF_POOL_MEMORY: ";
        result += "A pool memory allocation has failed. This must only be returned if no attempt to allocate host or device memory was made to accommodate the new allocation. If the failure was definitely due to fragmentation of the pool, VK_ERROR_FRAGMENTED_POOL should be returned instead.";
        break;
    case B3D_VkResult::VK_ERROR_INVALID_EXTERNAL_HANDLE:
        result += "VK_ERROR_INVALID_EXTERNAL_HANDLE: ";
        result += "An external handle is not a valid handle of the specified type.";
        break;
    case B3D_VkResult::VK_ERROR_FRAGMENTATION:
        result += "VK_ERROR_FRAGMENTATION: ";
        result += "A descriptor pool creation has failed due to fragmentation.";
        break;
//  case B3D_VkResult::VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS : 
    case B3D_VkResult::VK_ERROR_INVALID_DEVICE_ADDRESS_EXT:
        result += "VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS or VK_ERROR_INVALID_DEVICE_ADDRESS_EXT: ";
        result += "A buffer creation failed because the requested address is not available. A buffer creation or memory allocation failed because the requested address is not available. A shader group handle assignment failed because the requested shader group handle information is no longer valid.";
        break;
    case B3D_VkResult::VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT:
        result += "VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT: ";
        result += "An operation on a swapchain created with VK_FULL_SCREEN_EXCLUSIVE_APPLICATION_CONTROLLED_EXT failed as it did not have exlusive full-screen access. This may occur due to implementation-dependent reasons, outside of the application’s control.";
        break;
    case B3D_VkResult::VK_ERROR_UNKNOWN:
        result += "VK_ERROR_UNKNOWN: ";
        result += "An unknown error has occurred; either the application has provided invalid input, or an implementation failure has occurred.";
        break;
    case B3D_VkResult::VK_ERROR_VALIDATION_FAILED_EXT:
        result += "VK_ERROR_VALIDATION_FAILED_EXT: ";
        result += "The primary expected use of VK_ERROR_VALIDATION_FAILED_EXT is for validation layer testing. It is not expected that an application would see this error code during normal use of the validation layers.";
        break;
    case B3D_VkResult::VK_ERROR_INCOMPATIBLE_VERSION_KHR:
        result += "VK_ERROR_INCOMPATIBLE_VERSION_KHR: ";
        result += "vkGetDeviceAccelerationStructureCompatibilityKHR: VK_ERROR_INCOMPATIBLE_VERSION_KHR is returned if an acceleration structure serialized with version as the version information is not compatible with device.";
        break;
    case B3D_VkResult::VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT:
        result += "VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT: ";
        result += "When creating an image with VkImageDrmFormatModifierExplicitCreateInfoEXT, it is the application’s responsibility to satisfy all valid usage requirements. However, the implementation must validate that the provided pPlaneLayouts, when combined with the provided drmFormatModifier and other creation parameters in VkImageCreateInfo and its pNext chain, produce a valid image. (This validation is necessarily implementation-dependent and outside the scope of Vulkan, and therefore not described by valid usage requirements). If this validation fails, then vkCreateImage returns VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT.";
        break;
    case B3D_VkResult::VK_ERROR_NOT_PERMITTED_EXT:
        result += "VK_ERROR_NOT_PERMITTED_EXT: ";
        result += "VkQueueGlobalPriorityEXT: The global priority level of a queue takes precedence over the per - process queue priority(VkDeviceQueueCreateInfo::pQueuePriorities). Abuse of this feature may result in starving the rest of the system of implementation resources. Therefore, the driver implementation may deny requests to acquire a priority above the default priority(VK_QUEUE_GLOBAL_PRIORITY_MEDIUM_EXT) if the caller does not have sufficient privileges.In this scenario VK_ERROR_NOT_PERMITTED_EXT is returned.";
        break;
    #pragma endregion Error codes
    default:
        result += "Unknown error code: "; 
        result += "Unknown error code"; 
        break;
    }
    return result;
}

util::String GetVkResultDescriptionJP(VkResult _vkr)
{
    util::String result;
    switch ((B3D_VkResult)_vkr)
    {
    #pragma region Success codes
    case B3D_VkResult::VK_SUCCESS:
        result += "VK_SUCCESS: ";
        result += "コマンドが正常に完了しました。";
        break;
    case B3D_VkResult::VK_NOT_READY:
        result += "VK_NOT_READY: ";
        result += "フェンスまたはクエリはまだ完了していません。";
        break;
    case B3D_VkResult::VK_TIMEOUT:
        result += "VK_TIMEOUT: ";
        result += "指定された時間内に待機操作が完了しませんでした。";
        break;
    case B3D_VkResult::VK_EVENT_SET:
        result += "VK_EVENT_SET: ";
        result += "イベントが通知されました。";
        break;
    case B3D_VkResult::VK_EVENT_RESET:
        result += "VK_EVENT_RESET: ";
        result += "イベントが通知されませんでした。";
        break;
    case B3D_VkResult::VK_INCOMPLETE:
        result += "VK_INCOMPLETE: ";
        result += "戻り配列が結果に対して小さすぎました。";
        break;
    case B3D_VkResult::VK_SUBOPTIMAL_KHR:
        result += "VK_SUBOPTIMAL_KHR: ";
        result += "スワップチェーンはサーフェスのプロパティと正確に一致しなくなりましたが、引き続きサーフェスに正しくプレゼントするために使用できます。";
        break;
    case B3D_VkResult::VK_THREAD_IDLE_KHR:
        result += "VK_THREAD_IDLE_KHR: ";
        result += "遅延操作は完了していませんが、現在、この呼び出し時にこのスレッドが行う作業はありません。";
        break;
    case B3D_VkResult::VK_THREAD_DONE_KHR:
        result += "VK_THREAD_DONE_KHR: ";
        result += "遅延操作は完了していませんが、追加のスレッドに割り当てる作業が残っていません。";
        break;
    case B3D_VkResult::VK_OPERATION_DEFERRED_KHR:
        result += "VK_OPERATION_DEFERRED_KHR: ";
        result += "遅延操作が要求され、作業の少なくとも一部が遅延されました。";
        break;
    case B3D_VkResult::VK_OPERATION_NOT_DEFERRED_KHR:
        result += "VK_OPERATION_NOT_DEFERRED_KHR: ";
        result += "遅延操作が要求され、操作は遅延されませんでした。";
        break;
    case B3D_VkResult::VK_PIPELINE_COMPILE_REQUIRED_EXT:
        result += "VK_PIPELINE_COMPILE_REQUIRED_EXT: ";
        result += "要求されたパイプラインの作成にはコンパイルが必要でしたが、アプリケーションはコンパイルを実行しないように要求しました。";
        break;
    #pragma endregion Success codes
    #pragma region Error codes
    case B3D_VkResult::VK_ERROR_OUT_OF_HOST_MEMORY:
        result += "VK_ERROR_OUT_OF_HOST_MEMORY: ";
        result += "ホストのメモリ割り当てが失敗しました。";
        break;
    case B3D_VkResult::VK_ERROR_OUT_OF_DEVICE_MEMORY:
        result += "VK_ERROR_OUT_OF_DEVICE_MEMORY: ";
        result += "デバイスのメモリ割り当てに失敗しました。";
        break;
    case B3D_VkResult::VK_ERROR_INITIALIZATION_FAILED:
        result += "VK_ERROR_INITIALIZATION_FAILED: ";
        result += "実装固有の理由により、オブジェクトの初期化を完了できませんでした。";
        break;
    case B3D_VkResult::VK_ERROR_DEVICE_LOST:
        result += "VK_ERROR_DEVICE_LOST: ";
        result += "論理デバイスまたは物理デバイスが失われました。 \"Lost Device\"を参照してください。";
        break;
    case B3D_VkResult::VK_ERROR_MEMORY_MAP_FAILED:
        result += "VK_ERROR_MEMORY_MAP_FAILED: ";
        result += "メモリオブジェクトのマッピングに失敗しました。";
        break;
    case B3D_VkResult::VK_ERROR_LAYER_NOT_PRESENT:
        result += "VK_ERROR_LAYER_NOT_PRESENT: ";
        result += "リクエストされたレイヤーが存在しないか、読み込むことができませんでした。";
        break;
    case B3D_VkResult::VK_ERROR_EXTENSION_NOT_PRESENT:
        result += "VK_ERROR_EXTENSION_NOT_PRESENT: ";
        result += "リクエストされた拡張はサポートされていません。";
        break;
    case B3D_VkResult::VK_ERROR_FEATURE_NOT_PRESENT:
        result += "VK_ERROR_FEATURE_NOT_PRESENT: ";
        result += "要求された機能はサポートされていません。";
        break;
    case B3D_VkResult::VK_ERROR_INCOMPATIBLE_DRIVER:
        result += "VK_ERROR_INCOMPATIBLE_DRIVER: ";
        result += "要求されたバージョンのVulkanはドライバーによってサポートされていないか、実装固有の理由により互換性がありません。";
        break;
    case B3D_VkResult::VK_ERROR_TOO_MANY_OBJECTS:
        result += "VK_ERROR_TOO_MANY_OBJECTS: ";
        result += "同じタイプのオブジェクトが多すぎます。";
        break;
    case B3D_VkResult::VK_ERROR_FORMAT_NOT_SUPPORTED:
        result += "VK_ERROR_FORMAT_NOT_SUPPORTED: ";
        result += "リクエストされたフォーマットはこのデバイスではサポートされていません。";
        break;
    case B3D_VkResult::VK_ERROR_FRAGMENTED_POOL:
        result += "VK_ERROR_FRAGMENTED_POOL: ";
        result += "プールのメモリの断片化のため、プールの割り当てに失敗しました。これは、新しい割り当てに対応するためにホストまたはデバイスのメモリを割り当てようとしない場合にのみ返されます。これは、VK_ERROR_OUT_OF_POOL_MEMORYよりも優先して返される必要がありますが、プールの割り当ての失敗の原因がフラグメント化であることが確実な場合のみです。";
        break;
    case B3D_VkResult::VK_ERROR_SURFACE_LOST_KHR:
        result += "VK_ERROR_SURFACE_LOST_KHR: ";
        result += "サーフェスはもう利用できません。";
        break;
    case B3D_VkResult::VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
        result += "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR: ";
        result += "要求されたウィンドウは、Vulkanまたは別のAPIによって、再び使用できないようにすでに使用されています。";
        break;
    case B3D_VkResult::VK_ERROR_OUT_OF_DATE_KHR:
        result += "VK_ERROR_OUT_OF_DATE_KHR: ";
        result += "サーフェスが変更されたため、スワップチェーンとの互換性がなくなり、スワップチェーンを使用したその後のプレゼンテーション要求は失敗します。 アプリケーションは、新しいサーフェスプロパティを照会し、サーフェスへの表示を継続したい場合は、スワップチェーンを再作成する必要があります。";
        break;
    case B3D_VkResult::VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
        result += "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR: ";
        result += "スワップチェーンで使用されるディスプレイは、同じ表示可能な画像レイアウトを使用していないか、画像の共有を妨げる方法で互換性がありません。";
        break;
    case B3D_VkResult::VK_ERROR_INVALID_SHADER_NV:
        result += "VK_ERROR_INVALID_SHADER_NV: ";
        result += "1つ以上のシェーダーがコンパイルまたはリンクに失敗しました。 VK_EXT_debug_reportが有効になっている場合、詳細はデバッグレポートを介してアプリケーションに報告されます。";
        break;
    case B3D_VkResult::VK_ERROR_OUT_OF_POOL_MEMORY:
        result += "VK_ERROR_OUT_OF_POOL_MEMORY: ";
        result += "プールのメモリ割り当てに失敗しました。これは、新しい割り当てに対応するためにホストまたはデバイスのメモリを割り当てようとしない場合にのみ返されます。失敗が明らかにプールの断片化によるものである場合は、代わりにVK_ERROR_FRAGMENTED_POOLを返す必要があります。";
        break;
    case B3D_VkResult::VK_ERROR_INVALID_EXTERNAL_HANDLE:
        result += "VK_ERROR_INVALID_EXTERNAL_HANDLE: ";
        result += "外部ハンドルは、指定されたタイプの有効なハンドルではありません。";
        break;
    case B3D_VkResult::VK_ERROR_FRAGMENTATION:
        result += "VK_ERROR_FRAGMENTATION: ";
        result += "断片化のため、ディスクリプタプールの作成に失敗しました。";
        break;
//  case B3D_VkResult::VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS: 
    case B3D_VkResult::VK_ERROR_INVALID_DEVICE_ADDRESS_EXT:
        result += "VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS または VK_ERROR_INVALID_DEVICE_ADDRESS_EXT: ";
        result += "要求されたアドレスが使用できないため、バッファの作成またはメモリの割り当てに失敗しました。要求されたシェーダーグループハンドル情報が有効ではなくなったため、シェーダーグループハンドルの割り当てに失敗しました。";
        break;
    case B3D_VkResult::VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT:
        result += "VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT: ";
        result += "VK_FULL_SCREEN_EXCLUSIVE_APPLICATION_CONTROLLED_EXTで作成されたスワップチェーンでの操作は、排他的なフルスクリーンアクセスがなかったため失敗しました。これは、アプリケーションの制御外で、実装に依存する理由により発生する可能性があります。";
        break;
    case B3D_VkResult::VK_ERROR_UNKNOWN:
        result += "VK_ERROR_UNKNOWN: ";
        result += "不明なエラーが発生しました。アプリケーションが無効な入力を提供したか、実装エラーが発生しました。";
        break;
    case B3D_VkResult::VK_ERROR_VALIDATION_FAILED_EXT:
        result += "VK_ERROR_VALIDATION_FAILED_EXT: ";
        result += "VK_ERROR_VALIDATION_FAILED_EXTの予想される主な用途は、検証レイヤーのテストです。検証レイヤーの通常の使用中に、アプリケーションがこのエラーコードを表示することは想定されていません。";
        break;
    case B3D_VkResult::VK_ERROR_INCOMPATIBLE_VERSION_KHR:
        result += "VK_ERROR_INCOMPATIBLE_VERSION_KHR: ";
        result += "vkGetDeviceAccelerationStructureCompatibilityKHR: VK_ERROR_INCOMPATIBLE_VERSION_KHRはdeviceと互換性がないversionで加速構造がシリアライズされた場合に返されます。";
        break;
    case B3D_VkResult::VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT:
        result += "VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT: ";
        result += "VkImageDrmFormatModifierExplicitCreateInfoEXTを使用して画像を作成する場合、すべての有効な使用要件を満たすのはアプリケーションの責任です。ただし、実装は、提供されたpPlaneLayoutsが、提供されたdrmFormatModifierと、VkImageCreateInfoおよびそのpNextチェーン内の他の作成パラメーターと組み合わされたときに、有効な画像を生成することを検証する必要があります。 （この検証は実装依存であり、Vulkanの範囲外であるため、Valid Usageの要件では説明されていません）。 この検証が失敗した場合、vkCreateImageはVK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXTを返します。";
        break;
    case B3D_VkResult::VK_ERROR_NOT_PERMITTED_EXT:
        result += "VK_ERROR_NOT_PERMITTED_EXT: ";
        result += "VkQueueGlobalPriorityEXT： キューのグローバル優先度レベルは、プロセスごとのキュー優先度（VkDeviceQueueCreateInfo::pQueuePriorities）よりも優先されます。 この機能を悪用すると、システムの残りの実装リソースが不足する可能性があります。 したがって、呼び出し元に十分な特権がない場合、ドライバーの実装は、デフォルトの優先度（VK_QUEUE_GLOBAL_PRIORITY_MEDIUM_EXT）を超える優先度を取得する要求を拒否することがあります。 このシナリオでは、VK_ERROR_NOT_PERMITTED_EXTが返されます。";    
        break;
    #pragma endregion Error codes
    default: 
        result += "未知のエラーコード: ";
        result += "未知のエラーコード"; 
        break;
    }
    return result;
}

BMRESULT GetBMResultFromVk(VkResult _vkr)
{
    switch (_vkr)
    {
    case VK_SUCCESS:
    case VK_EVENT_SET:
    case VK_EVENT_RESET:
    case VK_INCOMPLETE:
    case VK_SUBOPTIMAL_KHR:
        return BMRESULT_SUCCEED;

    case VK_NOT_READY:
        return BMRESULT_SUCCEED_NOT_READY;

    case VK_TIMEOUT:
        return BMRESULT_SUCCEED_TIMEOUT;

    case VK_ERROR_OUT_OF_HOST_MEMORY:
        return BMRESULT_FAILED_OUT_OF_SYSTEM_MEMORY;

    case VK_ERROR_OUT_OF_DEVICE_MEMORY:
        return BMRESULT_FAILED_OUT_OF_DEVICE_MEMORY;

    case VK_ERROR_INITIALIZATION_FAILED:
    case VK_ERROR_MEMORY_MAP_FAILED:
    case VK_ERROR_TOO_MANY_OBJECTS:
    case VK_ERROR_FORMAT_NOT_SUPPORTED:
    case VK_ERROR_FRAGMENTED_POOL:
    case VK_ERROR_UNKNOWN:
    case VK_ERROR_OUT_OF_POOL_MEMORY:
    case VK_ERROR_INVALID_EXTERNAL_HANDLE:
    case VK_ERROR_FRAGMENTATION:
    case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS:
    case VK_ERROR_SURFACE_LOST_KHR:
    case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
    case VK_ERROR_OUT_OF_DATE_KHR:
    case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
    case VK_ERROR_VALIDATION_FAILED_EXT:
    case VK_ERROR_INVALID_SHADER_NV:
    case VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT:
    case VK_ERROR_NOT_PERMITTED_EXT:
    case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT:
        return BMRESULT_FAILED;

    case VK_ERROR_LAYER_NOT_PRESENT:
    case VK_ERROR_EXTENSION_NOT_PRESENT:
    case VK_ERROR_FEATURE_NOT_PRESENT:
        return BMRESULT_FAILED_NOT_SUPPORTED_FEATURE;

    case VK_ERROR_INCOMPATIBLE_DRIVER:
        return BMRESULT_FAILED_NOT_SUPPORTED;

    case VK_ERROR_DEVICE_LOST:
        return BMRESULT_FAILED_DEVICE_REMOVED;

    default:
        return BMRESULT_FAILED;
    }
}

void MemoryProperties::Init(VkPhysicalDevice _pd)
{
    mem_props = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2 };
    budget_props = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_BUDGET_PROPERTIES_EXT };
    ConnectPNextChains(mem_props, budget_props);

    vkGetPhysicalDeviceMemoryProperties2(_pd, &mem_props);
}

void MemoryProperties::UpdateBudgetProperties(VkPhysicalDevice _pd)
{
    vkGetPhysicalDeviceMemoryProperties2(_pd, &mem_props);
}

int32_t MemoryProperties::FindProperties(uint32_t _required_memory_type_bits, VkMemoryPropertyFlags _required_memory_props) const
{
    auto mem_count = mem_props.memoryProperties.memoryTypeCount;
    for (uint32_t i = 0; i < mem_count; ++i)
    {
        // 要求されたメモリタイプのインデックスではない場合continue
        if (!(_required_memory_type_bits & (1 << i)))
            continue;

        // インデックスiのメモリタイプのプロパティが要求されたプロパティと完全に一致する場合true
        if ((_required_memory_props & mem_props.memoryProperties.memoryTypes[i].propertyFlags) == _required_memory_props)
            return SCAST<int32_t>(i);
    }

    // 対応するメモリタイプが見つからない
    return -1;
}


}// namespace util
}// namespace buma3d
