#include "Buma3DPCH.h"
#include "VulkanDevicePFN.h"

namespace buma3d
{

#define CHECK_EXTENSION(x) _device->CheckDeviceExtensionEnabled(x) || fac->CheckInstanceExtensionEnabled(x)

#define LOAD_DEVICE(v)                                                                                                                                     \
    v = (PFN_##v)vkGetDeviceProcAddr(vkd, #v);                                                                                                             \
    if (v == nullptr)                                                                                                                                      \
    {                                                                                                                                                      \
        buma3d::util::AddDebugMessage(this, DEBUG_MESSAGE_SEVERITY_WARNING, DEBUG_MESSAGE_CATEGORY_FLAG_INITIALIZATION, #v" is not enabled/supported. (Device level)\n"); \
    }

#define LOAD_INSTANCE(v)                                                                                                                                   \
    v = (PFN_##v)vkGetInstanceProcAddr(vki, #v);                                                                                                           \
    if (v == nullptr)                                                                                                                                      \
    {                                                                                                                                                      \
        buma3d::util::AddDebugMessage(this, DEBUG_MESSAGE_SEVERITY_WARNING, DEBUG_MESSAGE_CATEGORY_FLAG_INITIALIZATION, #v" is not enabled/supported. (Instance level)\n"); \
    }

#define LOAD(v)                                                                                                                                            \
    v = LOAD_DEVICE(v);                                                                                                                                    \
    if (v == nullptr)                                                                                                                                      \
        v = LOAD_INSTANCE(v);                                                                                                                              \
    if (v == nullptr)                                                                                                                                      \
        buma3d::util::AddDebugMessage(this, DEBUG_MESSAGE_SEVERITY_WARNING, DEBUG_MESSAGE_CATEGORY_FLAG_INITIALIZATION, #v" is not enabled/supported.\n"); \

DevicePFN::DevicePFN(DeviceVk* _device)
{
    auto vkd = _device->GetVkDevice();
    auto fac = _device->GetDeviceAdapter()->GetDeviceFactory();
    auto vki = fac->GetVkInstance();
    // VK_EXT_debug_marker
    if (CHECK_EXTENSION(VK_EXT_DEBUG_MARKER_EXTENSION_NAME))
    {
        LOAD(vkDebugMarkerSetObjectTagEXT);
        LOAD(vkDebugMarkerSetObjectNameEXT);
        LOAD(vkCmdDebugMarkerBeginEXT);
        LOAD(vkCmdDebugMarkerEndEXT);
        LOAD(vkCmdDebugMarkerInsertEXT);
    }

    // VK_EXT_debug_utils device-level 
    if (CHECK_EXTENSION(VK_EXT_DEBUG_UTILS_EXTENSION_NAME))
    {
        LOAD(vkSetDebugUtilsObjectNameEXT);
        LOAD(vkSetDebugUtilsObjectTagEXT);
        LOAD(vkQueueBeginDebugUtilsLabelEXT);
        LOAD(vkQueueEndDebugUtilsLabelEXT);
        LOAD(vkQueueInsertDebugUtilsLabelEXT);
        LOAD(vkCmdBeginDebugUtilsLabelEXT);
        LOAD(vkCmdEndDebugUtilsLabelEXT);
        LOAD(vkCmdInsertDebugUtilsLabelEXT);
    }

    // VK_EXT_calibrated_timestamps
    if (CHECK_EXTENSION(VK_EXT_CALIBRATED_TIMESTAMPS_EXTENSION_NAME))
    {
        LOAD(vkGetCalibratedTimestampsEXT);
        LOAD(vkGetPhysicalDeviceCalibrateableTimeDomainsEXT);
    }

    // VK_KHR_performance_query
    if (CHECK_EXTENSION(VK_KHR_PERFORMANCE_QUERY_EXTENSION_NAME))
    {
        LOAD(vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR);
        LOAD(vkGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR);
        LOAD(vkAcquireProfilingLockKHR);
        LOAD(vkReleaseProfilingLockKHR);
    }

    // VK_EXT_tooling_info
    if (CHECK_EXTENSION(VK_EXT_TOOLING_INFO_EXTENSION_NAME))
    {
        LOAD(vkGetPhysicalDeviceToolPropertiesEXT);
    }

    // VK_EXT_sample_locations
    if (CHECK_EXTENSION(VK_EXT_SAMPLE_LOCATIONS_EXTENSION_NAME))
    {
        LOAD(vkCmdSetSampleLocationsEXT);
        LOAD(vkGetPhysicalDeviceMultisamplePropertiesEXT);
    }

    // VK_NV_coverage_reduction_mode
    if (CHECK_EXTENSION(VK_NV_COVERAGE_REDUCTION_MODE_EXTENSION_NAME))
    {
        LOAD(vkGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV);
    }

    // VK_NV_cooperative_matrix
    if (CHECK_EXTENSION(VK_NV_COOPERATIVE_MATRIX_EXTENSION_NAME))
    {
        LOAD(vkGetPhysicalDeviceCooperativeMatrixPropertiesNV);
    }

    // VK_KHR_swapchain
    if (CHECK_EXTENSION(VK_KHR_SWAPCHAIN_EXTENSION_NAME))
    {
        LOAD(vkCreateSwapchainKHR);
        LOAD(vkDestroySwapchainKHR);
        LOAD(vkGetSwapchainImagesKHR);
        LOAD(vkAcquireNextImageKHR);
        LOAD(vkQueuePresentKHR);
        LOAD(vkGetDeviceGroupPresentCapabilitiesKHR);
        LOAD(vkGetDeviceGroupSurfacePresentModesKHR);
        LOAD(vkGetPhysicalDevicePresentRectanglesKHR);
        LOAD(vkAcquireNextImage2KHR);
    }

    // VK_EXT_hdr_metadata
    if (CHECK_EXTENSION(VK_EXT_HDR_METADATA_EXTENSION_NAME))
    {
        LOAD(vkSetHdrMetadataEXT);
    }


    // VK_EXT_transform_feedback
    if (CHECK_EXTENSION(VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME))
    {
        LOAD(vkCmdBindTransformFeedbackBuffersEXT);
        LOAD(vkCmdBeginTransformFeedbackEXT);
        LOAD(vkCmdEndTransformFeedbackEXT);
        LOAD(vkCmdBeginQueryIndexedEXT);
        LOAD(vkCmdEndQueryIndexedEXT);
        LOAD(vkCmdDrawIndirectByteCountEXT);
    }

    // VK_EXT_conditional_rendering
    if (CHECK_EXTENSION(VK_EXT_CONDITIONAL_RENDERING_EXTENSION_NAME))
    {
        LOAD(vkCmdBeginConditionalRenderingEXT);
        LOAD(vkCmdEndConditionalRenderingEXT);
    }

    // VK_NV_mesh_shader
    if (CHECK_EXTENSION(VK_NV_MESH_SHADER_EXTENSION_NAME))
    {
        LOAD(vkCmdDrawMeshTasksNV);
        LOAD(vkCmdDrawMeshTasksIndirectNV);
        LOAD(vkCmdDrawMeshTasksIndirectCountNV);
    }

    // VK_EXT_extended_dynamic_state
    if (CHECK_EXTENSION(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME))
    {
        LOAD(vkCmdSetCullModeEXT);
        LOAD(vkCmdSetFrontFaceEXT);
        LOAD(vkCmdSetPrimitiveTopologyEXT);
        LOAD(vkCmdSetViewportWithCountEXT);
        LOAD(vkCmdSetScissorWithCountEXT);
        LOAD(vkCmdBindVertexBuffers2EXT);
        LOAD(vkCmdSetDepthTestEnableEXT);
        LOAD(vkCmdSetDepthWriteEnableEXT);
        LOAD(vkCmdSetDepthCompareOpEXT);
        LOAD(vkCmdSetDepthBoundsTestEnableEXT);
        LOAD(vkCmdSetStencilTestEnableEXT);
        LOAD(vkCmdSetStencilOpEXT);
    }

        // VK_KHR_acceleration_structure
    if (CHECK_EXTENSION(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME))
    {
        LOAD(vkCreateAccelerationStructureKHR);
        LOAD(vkDestroyAccelerationStructureKHR);
        LOAD(vkCmdBuildAccelerationStructuresKHR);
        LOAD(vkCmdBuildAccelerationStructuresIndirectKHR);
        LOAD(vkBuildAccelerationStructuresKHR);
        LOAD(vkCopyAccelerationStructureKHR);
        LOAD(vkCopyAccelerationStructureToMemoryKHR);
        LOAD(vkCopyMemoryToAccelerationStructureKHR);
        LOAD(vkWriteAccelerationStructuresPropertiesKHR);
        LOAD(vkCmdCopyAccelerationStructureKHR);
        LOAD(vkCmdCopyAccelerationStructureToMemoryKHR);
        LOAD(vkCmdCopyMemoryToAccelerationStructureKHR);
        LOAD(vkGetAccelerationStructureDeviceAddressKHR);
        LOAD(vkCmdWriteAccelerationStructuresPropertiesKHR);
        LOAD(vkGetDeviceAccelerationStructureCompatibilityKHR);
        LOAD(vkGetAccelerationStructureBuildSizesKHR);
    }

    // VK_KHR_ray_tracing_pipeline
    if (CHECK_EXTENSION(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME))
    {
        LOAD(vkCmdTraceRaysKHR);
        LOAD(vkCreateRayTracingPipelinesKHR);
        LOAD(vkGetRayTracingCaptureReplayShaderGroupHandlesKHR);
        LOAD(vkCmdTraceRaysIndirectKHR);
        LOAD(vkGetRayTracingShaderGroupStackSizeKHR);
        LOAD(vkCmdSetRayTracingPipelineStackSizeKHR);
    }

    // VK_KHR_deferred_host_operations
    if (CHECK_EXTENSION(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME))
    {
        LOAD(vkCreateDeferredOperationKHR);
        LOAD(vkDestroyDeferredOperationKHR);
        LOAD(vkGetDeferredOperationMaxConcurrencyKHR);
        LOAD(vkGetDeferredOperationResultKHR);
        LOAD(vkDeferredOperationJoinKHR);
    }

    // VK_KHR_fragment_shading_rate
    if (CHECK_EXTENSION(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME))
    {
        LOAD(vkGetPhysicalDeviceFragmentShadingRatesKHR);
        LOAD(vkCmdSetFragmentShadingRateKHR);
    }

#ifdef VK_USE_PLATFORM_WIN32_KHR

    // VK_KHR_external_memory_win32
    if (CHECK_EXTENSION(VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME))
    {
        LOAD(vkGetMemoryWin32HandleKHR);
        LOAD(vkGetMemoryWin32HandlePropertiesKHR);
    }

    // VK_KHR_external_semaphore_win32
    if (CHECK_EXTENSION(VK_KHR_EXTERNAL_SEMAPHORE_WIN32_EXTENSION_NAME))
    {
        LOAD(vkImportSemaphoreWin32HandleKHR);
        LOAD(vkGetSemaphoreWin32HandleKHR);
    }

    // VK_KHR_external_fence_win32
    if (CHECK_EXTENSION(VK_KHR_EXTERNAL_FENCE_WIN32_EXTENSION_NAME))
    {
        LOAD(vkImportFenceWin32HandleKHR);
        LOAD(vkGetFenceWin32HandleKHR);
    }

    // VK_NV_external_memory_win32
    if (CHECK_EXTENSION(VK_NV_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME))
    {
        LOAD(vkGetMemoryWin32HandleNV);
    }

    // VK_EXT_full_screen_exclusive
    if (CHECK_EXTENSION(VK_EXT_FULL_SCREEN_EXCLUSIVE_EXTENSION_NAME))
    {
        LOAD(vkGetPhysicalDeviceSurfacePresentModes2EXT);
        LOAD(vkAcquireFullScreenExclusiveModeEXT);
        LOAD(vkReleaseFullScreenExclusiveModeEXT);
        LOAD(vkGetDeviceGroupSurfacePresentModes2EXT);
    }

    // VK_EXT_hdr_metadata
    if (CHECK_EXTENSION(VK_EXT_HDR_METADATA_EXTENSION_NAME))
    {
        LOAD(vkSetHdrMetadataEXT)
    }

#endif // VK_USE_PLATFORM_WIN32_KHR

}

}// namespace buma3d
