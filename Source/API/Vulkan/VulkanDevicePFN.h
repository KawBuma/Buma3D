#pragma once

// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/html/vkspec.html#initialization-functionpointers
// Vulkanコマンドは、プラットフォーム上の静的リンクによって必ずしも公開されるわけではありません。 

/*
vkGetInstanceProcAddr: VkInstance, VkPhysicalDevice
vkGetDeviceProcAddr: VkDevice, VkQueue, VkCommandBuffer
*/

/* https://www.khronos.org/registry/vulkan/specs/1.2-extensions/html/vkspec.html#vkGetDeviceProcAddr
複数のVulkan実装を持つシステムをサポートするために、vkGetInstanceProcAddrによって返される関数ポインターは、
異なるVkDeviceオブジェクトまたはその子オブジェクトの異なる実際の実装を呼び出すディスパッチコードを指す場合があります。
VkDeviceオブジェクトの内部ディスパッチのオーバーヘッドは、
デバイスまたはデバイスの子オブジェクトをディスパッチ可能なオブジェクトとして使用するコマンドの
デバイス固有の関数ポインターを取得することで回避できます。
*/


namespace buma3d
{

class DevicePFN
{
public:
    DevicePFN()
    {}
    DevicePFN(DeviceVk* _device);
    ~DevicePFN()
    {}

public:
    // VK_EXT_debug_marker
    PFN_vkDebugMarkerSetObjectTagEXT                    vkDebugMarkerSetObjectTagEXT{};  // VkResult (VkDevice device, const VkDebugMarkerObjectTagInfoEXT* pTagInfo);
    PFN_vkDebugMarkerSetObjectNameEXT                   vkDebugMarkerSetObjectNameEXT{}; // VkResult (VkDevice device, const VkDebugMarkerObjectNameInfoEXT* pNameInfo);
    PFN_vkCmdDebugMarkerBeginEXT                        vkCmdDebugMarkerBeginEXT{};      // void (VkCommandBuffer commandBuffer, const VkDebugMarkerMarkerInfoEXT* pMarkerInfo);
    PFN_vkCmdDebugMarkerEndEXT                          vkCmdDebugMarkerEndEXT{};        // void (VkCommandBuffer commandBuffer);
    PFN_vkCmdDebugMarkerInsertEXT                       vkCmdDebugMarkerInsertEXT{};     // void (VkCommandBuffer commandBuffer, const VkDebugMarkerMarkerInfoEXT* pMarkerInfo);

    // VK_EXT_debug_utils (device-level) 
    PFN_vkSetDebugUtilsObjectNameEXT                    vkSetDebugUtilsObjectNameEXT{};    // VkResult (VkDevice device, const VkDebugUtilsObjectNameInfoEXT* pNameInfo);
    PFN_vkSetDebugUtilsObjectTagEXT                     vkSetDebugUtilsObjectTagEXT{};     // VkResult (VkDevice device, const VkDebugUtilsObjectTagInfoEXT* pTagInfo);
    PFN_vkQueueBeginDebugUtilsLabelEXT                  vkQueueBeginDebugUtilsLabelEXT{};  // void (VkQueue queue, const VkDebugUtilsLabelEXT* pLabelInfo);
    PFN_vkQueueEndDebugUtilsLabelEXT                    vkQueueEndDebugUtilsLabelEXT{};    // void (VkQueue queue);
    PFN_vkQueueInsertDebugUtilsLabelEXT                 vkQueueInsertDebugUtilsLabelEXT{}; // void (VkQueue queue, const VkDebugUtilsLabelEXT* pLabelInfo);
    PFN_vkCmdBeginDebugUtilsLabelEXT                    vkCmdBeginDebugUtilsLabelEXT{};    // void (VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT* pLabelInfo);
    PFN_vkCmdEndDebugUtilsLabelEXT                      vkCmdEndDebugUtilsLabelEXT{};      // void (VkCommandBuffer commandBuffer);
    PFN_vkCmdInsertDebugUtilsLabelEXT                   vkCmdInsertDebugUtilsLabelEXT{};   // void (VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT* pLabelInfo);

    // VK_EXT_calibrated_timestamps
    PFN_vkGetCalibratedTimestampsEXT                    vkGetCalibratedTimestampsEXT{};                   // VkResult (VkDevice device, uint32_t timestampCount, const VkCalibratedTimestampInfoEXT* pTimestampInfos, uint64_t* pTimestamps, uint64_t* pMaxDeviation);
    PFN_vkGetPhysicalDeviceCalibrateableTimeDomainsEXT  vkGetPhysicalDeviceCalibrateableTimeDomainsEXT{}; // VkResult (VkPhysicalDevice physicalDevice, uint32_t* pTimeDomainCount, VkTimeDomainEXT* pTimeDomains);

    // VK_KHR_performance_query
    PFN_vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR{}; // VkResult (VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, uint32_t* pCounterCount, VkPerformanceCounterKHR* pCounters, VkPerformanceCounterDescriptionKHR* pCounterDescriptions);
    PFN_vkGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR         vkGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR{};         // void (VkPhysicalDevice physicalDevice, const VkQueryPoolPerformanceCreateInfoKHR* pPerformanceQueryCreateInfo, uint32_t* pNumPasses);
    PFN_vkAcquireProfilingLockKHR                                       vkAcquireProfilingLockKHR{};                                       // VkResult (VkDevice device, const VkAcquireProfilingLockInfoKHR* pInfo);
    PFN_vkReleaseProfilingLockKHR                                       vkReleaseProfilingLockKHR{};                                       // void (VkDevice device);

    // VK_EXT_tooling_info
    PFN_vkGetPhysicalDeviceToolPropertiesEXT            vkGetPhysicalDeviceToolPropertiesEXT{}; // VkResult (VkPhysicalDevice physicalDevice, uint32_t* pToolCount, VkPhysicalDeviceToolPropertiesEXT* pToolProperties);

    // VK_EXT_sample_locations
    PFN_vkCmdSetSampleLocationsEXT                      vkCmdSetSampleLocationsEXT{};                  // void (VkCommandBuffer commandBuffer, const VkSampleLocationsInfoEXT* pSampleLocationsInfo);
    PFN_vkGetPhysicalDeviceMultisamplePropertiesEXT     vkGetPhysicalDeviceMultisamplePropertiesEXT{}; // void (VkPhysicalDevice physicalDevice, VkSampleCountFlagBits samples, VkMultisamplePropertiesEXT* pMultisampleProperties);

    // VK_NV_framebuffer_mixed_samples
    // VK_NV_coverage_reduction_mode
    PFN_vkGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV vkGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV{}; // VkResult (VkPhysicalDevice physicalDevice, uint32_t* pCombinationCount, VkFramebufferMixedSamplesCombinationNV* pCombinations);

    // VK_NV_cooperative_matrix
    PFN_vkGetPhysicalDeviceCooperativeMatrixPropertiesNV vkGetPhysicalDeviceCooperativeMatrixPropertiesNV{}; // VkResult (VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount, VkCooperativeMatrixPropertiesNV* pProperties);

    // VK_KHR_swapchain
    PFN_vkCreateSwapchainKHR                            vkCreateSwapchainKHR{};                    // VkResult (VkDevice device, const VkSwapchainCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain);
    PFN_vkDestroySwapchainKHR                           vkDestroySwapchainKHR{};                   // void (VkDevice device, VkSwapchainKHR swapchain, const VkAllocationCallbacks* pAllocator);
    PFN_vkGetSwapchainImagesKHR                         vkGetSwapchainImagesKHR{};                 // VkResult (VkDevice device, VkSwapchainKHR swapchain, uint32_t* pSwapchainImageCount, VkImage* pSwapchainImages);
    PFN_vkAcquireNextImageKHR                           vkAcquireNextImageKHR{};                   // VkResult (VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore, VkFence fence, uint32_t* pImageIndex);
    PFN_vkQueuePresentKHR                               vkQueuePresentKHR{};                       // VkResult (VkQueue queue, const VkPresentInfoKHR* pPresentInfo);
    PFN_vkGetDeviceGroupPresentCapabilitiesKHR          vkGetDeviceGroupPresentCapabilitiesKHR{};  // VkResult (VkDevice device, VkDeviceGroupPresentCapabilitiesKHR* pDeviceGroupPresentCapabilities);
    PFN_vkGetDeviceGroupSurfacePresentModesKHR          vkGetDeviceGroupSurfacePresentModesKHR{};  // VkResult (VkDevice device, VkSurfaceKHR surface, VkDeviceGroupPresentModeFlagsKHR* pModes);
    PFN_vkGetPhysicalDevicePresentRectanglesKHR         vkGetPhysicalDevicePresentRectanglesKHR{}; // VkResult (VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t* pRectCount, VkRect2D* pRects);
    PFN_vkAcquireNextImage2KHR                          vkAcquireNextImage2KHR{};                  // VkResult (VkDevice device, const VkAcquireNextImageInfoKHR* pAcquireInfo, uint32_t* pImageIndex);

    // VK_EXT_hdr_metadata
    PFN_vkSetHdrMetadataEXT                             vkSetHdrMetadataEXT{}; // void (VkDevice device, uint32_t swapchainCount, const VkSwapchainKHR* pSwapchains, const VkHdrMetadataEXT* pMetadata);

    // VK_EXT_transform_feedback
    PFN_vkCmdBindTransformFeedbackBuffersEXT            vkCmdBindTransformFeedbackBuffersEXT{}; // void (VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount, const VkBuffer* pBuffers, const VkDeviceSize* pOffsets, const VkDeviceSize* pSizes);
    PFN_vkCmdBeginTransformFeedbackEXT                  vkCmdBeginTransformFeedbackEXT{};       // void (VkCommandBuffer commandBuffer, uint32_t firstCounterBuffer, uint32_t counterBufferCount, const VkBuffer* pCounterBuffers, const VkDeviceSize* pCounterBufferOffsets);
    PFN_vkCmdEndTransformFeedbackEXT                    vkCmdEndTransformFeedbackEXT{};         // void (VkCommandBuffer commandBuffer, uint32_t firstCounterBuffer, uint32_t counterBufferCount, const VkBuffer* pCounterBuffers, const VkDeviceSize* pCounterBufferOffsets);
    PFN_vkCmdBeginQueryIndexedEXT                       vkCmdBeginQueryIndexedEXT{};            // void (VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query, VkQueryControlFlags flags, uint32_t index);
    PFN_vkCmdEndQueryIndexedEXT                         vkCmdEndQueryIndexedEXT{};              // void (VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query, uint32_t index);
    PFN_vkCmdDrawIndirectByteCountEXT                   vkCmdDrawIndirectByteCountEXT{};        // void (VkCommandBuffer commandBuffer, uint32_t instanceCount, uint32_t firstInstance, VkBuffer counterBuffer, VkDeviceSize counterBufferOffset, uint32_t counterOffset, uint32_t vertexStride);

    // VK_EXT_conditional_rendering
    PFN_vkCmdBeginConditionalRenderingEXT               vkCmdBeginConditionalRenderingEXT{};    // void (VkCommandBuffer commandBuffer, const VkConditionalRenderingBeginInfoEXT* pConditionalRenderingBegin);
    PFN_vkCmdEndConditionalRenderingEXT                 vkCmdEndConditionalRenderingEXT{};      // void (VkCommandBuffer commandBuffer);

    // VK_NV_mesh_shader
    PFN_vkCmdDrawMeshTasksNV                            vkCmdDrawMeshTasksNV{};                 // void (VkCommandBuffer commandBuffer, uint32_t taskCount, uint32_t firstTask);
    PFN_vkCmdDrawMeshTasksIndirectNV                    vkCmdDrawMeshTasksIndirectNV{};         // void (VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride);
    PFN_vkCmdDrawMeshTasksIndirectCountNV               vkCmdDrawMeshTasksIndirectCountNV{};    // void (VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride);

    // VK_EXT_extended_dynamic_state
    PFN_vkCmdSetCullModeEXT                             vkCmdSetCullModeEXT{};                  // void (VkCommandBuffer commandBuffer, VkCullModeFlags cullMode);
    PFN_vkCmdSetFrontFaceEXT                            vkCmdSetFrontFaceEXT{};                 // void (VkCommandBuffer commandBuffer, VkFrontFace frontFace);
    PFN_vkCmdSetPrimitiveTopologyEXT                    vkCmdSetPrimitiveTopologyEXT{};         // void (VkCommandBuffer commandBuffer, VkPrimitiveTopology primitiveTopology);
    PFN_vkCmdSetViewportWithCountEXT                    vkCmdSetViewportWithCountEXT{};         // void (VkCommandBuffer commandBuffer, uint32_t viewportCount, const VkViewport* pViewports);
    PFN_vkCmdSetScissorWithCountEXT                     vkCmdSetScissorWithCountEXT{};          // void (VkCommandBuffer commandBuffer, uint32_t scissorCount, const VkRect2D* pScissors);
    PFN_vkCmdBindVertexBuffers2EXT                      vkCmdBindVertexBuffers2EXT{};           // void (VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount, const VkBuffer* pBuffers, const VkDeviceSize* pOffsets, const VkDeviceSize* pSizes, const VkDeviceSize* pStrides);
    PFN_vkCmdSetDepthTestEnableEXT                      vkCmdSetDepthTestEnableEXT{};           // void (VkCommandBuffer commandBuffer, VkBool32 depthTestEnable);
    PFN_vkCmdSetDepthWriteEnableEXT                     vkCmdSetDepthWriteEnableEXT{};          // void (VkCommandBuffer commandBuffer, VkBool32 depthWriteEnable);
    PFN_vkCmdSetDepthCompareOpEXT                       vkCmdSetDepthCompareOpEXT{};            // void (VkCommandBuffer commandBuffer, VkCompareOp depthCompareOp);
    PFN_vkCmdSetDepthBoundsTestEnableEXT                vkCmdSetDepthBoundsTestEnableEXT{};     // void (VkCommandBuffer commandBuffer, VkBool32 depthBoundsTestEnable);
    PFN_vkCmdSetStencilTestEnableEXT                    vkCmdSetStencilTestEnableEXT{};         // void (VkCommandBuffer commandBuffer, VkBool32 stencilTestEnable);
    PFN_vkCmdSetStencilOpEXT                            vkCmdSetStencilOpEXT{};                 // void (VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, VkStencilOp failOp, VkStencilOp passOp, VkStencilOp depthFailOp, VkCompareOp compareOp);

    // VK_KHR_acceleration_structure
    PFN_vkCreateAccelerationStructureKHR                    vkCreateAccelerationStructureKHR{};                     // VkResult (VkDevice device, const VkAccelerationStructureCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkAccelerationStructureKHR* pAccelerationStructure);
    PFN_vkDestroyAccelerationStructureKHR                   vkDestroyAccelerationStructureKHR{};                    // void (VkDevice device, VkAccelerationStructureKHR accelerationStructure, const VkAllocationCallbacks* pAllocator);
    PFN_vkCmdBuildAccelerationStructuresKHR                 vkCmdBuildAccelerationStructuresKHR{};                  // void (VkCommandBuffer commandBuffer, uint32_t infoCount, const VkAccelerationStructureBuildGeometryInfoKHR* pInfos, const VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos);
    PFN_vkCmdBuildAccelerationStructuresIndirectKHR         vkCmdBuildAccelerationStructuresIndirectKHR{};          // void (VkCommandBuffer commandBuffer, uint32_t infoCount, const VkAccelerationStructureBuildGeometryInfoKHR* pInfos, const VkDeviceAddress* pIndirectDeviceAddresses, const uint32_t* pIndirectStrides, const uint32_t* const* ppMaxPrimitiveCounts);
    PFN_vkBuildAccelerationStructuresKHR                    vkBuildAccelerationStructuresKHR{};                     // VkResult (VkDevice device, VkDeferredOperationKHR deferredOperation, uint32_t infoCount, const VkAccelerationStructureBuildGeometryInfoKHR* pInfos, const VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos);
    PFN_vkCopyAccelerationStructureKHR                      vkCopyAccelerationStructureKHR{};                       // VkResult (VkDevice device, VkDeferredOperationKHR deferredOperation, const VkCopyAccelerationStructureInfoKHR* pInfo);
    PFN_vkCopyAccelerationStructureToMemoryKHR              vkCopyAccelerationStructureToMemoryKHR{};               // VkResult (VkDevice device, VkDeferredOperationKHR deferredOperation, const VkCopyAccelerationStructureToMemoryInfoKHR* pInfo);
    PFN_vkCopyMemoryToAccelerationStructureKHR              vkCopyMemoryToAccelerationStructureKHR{};               // VkResult (VkDevice device, VkDeferredOperationKHR deferredOperation, const VkCopyMemoryToAccelerationStructureInfoKHR* pInfo);
    PFN_vkWriteAccelerationStructuresPropertiesKHR          vkWriteAccelerationStructuresPropertiesKHR{};           // VkResult (VkDevice device, uint32_t accelerationStructureCount, const VkAccelerationStructureKHR* pAccelerationStructures, VkQueryType queryType, size_t dataSize, void* pData, size_t stride);
    PFN_vkCmdCopyAccelerationStructureKHR                   vkCmdCopyAccelerationStructureKHR{};                    // void (VkCommandBuffer commandBuffer, const VkCopyAccelerationStructureInfoKHR* pInfo);
    PFN_vkCmdCopyAccelerationStructureToMemoryKHR           vkCmdCopyAccelerationStructureToMemoryKHR{};            // void (VkCommandBuffer commandBuffer, const VkCopyAccelerationStructureToMemoryInfoKHR* pInfo);
    PFN_vkCmdCopyMemoryToAccelerationStructureKHR           vkCmdCopyMemoryToAccelerationStructureKHR{};            // void (VkCommandBuffer commandBuffer, const VkCopyMemoryToAccelerationStructureInfoKHR* pInfo);
    PFN_vkGetAccelerationStructureDeviceAddressKHR          vkGetAccelerationStructureDeviceAddressKHR{};           // VkDeviceAddress (VkDevice device, const VkAccelerationStructureDeviceAddressInfoKHR* pInfo);
    PFN_vkCmdWriteAccelerationStructuresPropertiesKHR       vkCmdWriteAccelerationStructuresPropertiesKHR{};        // void (VkCommandBuffer commandBuffer, uint32_t accelerationStructureCount, const VkAccelerationStructureKHR* pAccelerationStructures, VkQueryType queryType, VkQueryPool queryPool, uint32_t firstQuery);
    PFN_vkGetDeviceAccelerationStructureCompatibilityKHR    vkGetDeviceAccelerationStructureCompatibilityKHR{};     // void (VkDevice device, const VkAccelerationStructureVersionInfoKHR* pVersionInfo, VkAccelerationStructureCompatibilityKHR* pCompatibility);
    PFN_vkGetAccelerationStructureBuildSizesKHR             vkGetAccelerationStructureBuildSizesKHR{};              // void (VkDevice device, VkAccelerationStructureBuildTypeKHR buildType, const VkAccelerationStructureBuildGeometryInfoKHR* pBuildInfo, const uint32_t* pMaxPrimitiveCounts, VkAccelerationStructureBuildSizesInfoKHR* pSizeInfo);

    // VK_KHR_ray_tracing_pipeline
    PFN_vkCmdTraceRaysKHR                                   vkCmdTraceRaysKHR{};                                    // void (VkCommandBuffer commandBuffer, const VkStridedDeviceAddressRegionKHR* pRaygenShaderBindingTable, const VkStridedDeviceAddressRegionKHR* pMissShaderBindingTable, const VkStridedDeviceAddressRegionKHR* pHitShaderBindingTable, const VkStridedDeviceAddressRegionKHR* pCallableShaderBindingTable, uint32_t width, uint32_t height, uint32_t depth);
    PFN_vkCreateRayTracingPipelinesKHR                      vkCreateRayTracingPipelinesKHR{};                       // VkResult (VkDevice device, VkDeferredOperationKHR deferredOperation, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkRayTracingPipelineCreateInfoKHR* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines);
    PFN_vkGetRayTracingCaptureReplayShaderGroupHandlesKHR   vkGetRayTracingCaptureReplayShaderGroupHandlesKHR{};    // VkResult (VkDevice device, VkPipeline pipeline, uint32_t firstGroup, uint32_t groupCount, size_t dataSize, void* pData);
    PFN_vkCmdTraceRaysIndirectKHR                           vkCmdTraceRaysIndirectKHR{};                            // void (VkCommandBuffer commandBuffer, const VkStridedDeviceAddressRegionKHR* pRaygenShaderBindingTable, const VkStridedDeviceAddressRegionKHR* pMissShaderBindingTable, const VkStridedDeviceAddressRegionKHR* pHitShaderBindingTable, const VkStridedDeviceAddressRegionKHR* pCallableShaderBindingTable, VkDeviceAddress indirectDeviceAddress);
    PFN_vkGetRayTracingShaderGroupStackSizeKHR              vkGetRayTracingShaderGroupStackSizeKHR{};               // VkDeviceSize (VkDevice device, VkPipeline pipeline, uint32_t group, VkShaderGroupShaderKHR groupShader);
    PFN_vkCmdSetRayTracingPipelineStackSizeKHR              vkCmdSetRayTracingPipelineStackSizeKHR{};               // void (VkCommandBuffer commandBuffer, uint32_t pipelineStackSize);

    // VK_KHR_deferred_host_operations
    PFN_vkCreateDeferredOperationKHR                        vkCreateDeferredOperationKHR{};                         // VkResult (VkDevice device, const VkAllocationCallbacks* pAllocator, VkDeferredOperationKHR* pDeferredOperation);
    PFN_vkDestroyDeferredOperationKHR                       vkDestroyDeferredOperationKHR{};                        // void (VkDevice device, VkDeferredOperationKHR operation, const VkAllocationCallbacks* pAllocator);
    PFN_vkGetDeferredOperationMaxConcurrencyKHR             vkGetDeferredOperationMaxConcurrencyKHR{};              // uint32_t (VkDevice device, VkDeferredOperationKHR operation);
    PFN_vkGetDeferredOperationResultKHR                     vkGetDeferredOperationResultKHR{};                      // VkResult (VkDevice device, VkDeferredOperationKHR operation);
    PFN_vkDeferredOperationJoinKHR                          vkDeferredOperationJoinKHR{};                           // VkResult (VkDevice device, VkDeferredOperationKHR operation);

    // VK_KHR_fragment_shading_rate
    PFN_vkGetPhysicalDeviceFragmentShadingRatesKHR          vkGetPhysicalDeviceFragmentShadingRatesKHR{};           // VkResult (VkPhysicalDevice physicalDevice, uint32_t* pFragmentShadingRateCount, VkPhysicalDeviceFragmentShadingRateKHR* pFragmentShadingRates);
    PFN_vkCmdSetFragmentShadingRateKHR                      vkCmdSetFragmentShadingRateKHR{};                       // void (VkCommandBuffer commandBuffer, const VkExtent2D* pFragmentSize, const VkFragmentShadingRateCombinerOpKHR combinerOps[2]);

#ifdef VK_USE_PLATFORM_WIN32_KHR

    // VK_KHR_external_memory_win32
    PFN_vkGetMemoryWin32HandleKHR                   vkGetMemoryWin32HandleKHR{};                  // VkResult (VkDevice device, const VkMemoryGetWin32HandleInfoKHR* pGetWin32HandleInfo, HANDLE* pHandle);
    PFN_vkGetMemoryWin32HandlePropertiesKHR         vkGetMemoryWin32HandlePropertiesKHR{};        // VkResult (VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType, HANDLE handle, VkMemoryWin32HandlePropertiesKHR* pMemoryWin32HandleProperties);

    // VK_KHR_external_semaphore_win32
    PFN_vkImportSemaphoreWin32HandleKHR             vkImportSemaphoreWin32HandleKHR{};            // VkResult (VkDevice device, const VkImportSemaphoreWin32HandleInfoKHR* pImportSemaphoreWin32HandleInfo);
    PFN_vkGetSemaphoreWin32HandleKHR                vkGetSemaphoreWin32HandleKHR{};               // VkResult (VkDevice device, const VkSemaphoreGetWin32HandleInfoKHR* pGetWin32HandleInfo, HANDLE* pHandle);

    // VK_KHR_external_fence_win32
    PFN_vkImportFenceWin32HandleKHR                 vkImportFenceWin32HandleKHR{};                // VkResult (VkDevice device, const VkImportFenceWin32HandleInfoKHR* pImportFenceWin32HandleInfo);
    PFN_vkGetFenceWin32HandleKHR                    vkGetFenceWin32HandleKHR{};                   // VkResult (VkDevice device, const VkFenceGetWin32HandleInfoKHR* pGetWin32HandleInfo, HANDLE* pHandle);

    // VK_NV_external_memory_win32
    PFN_vkGetMemoryWin32HandleNV                    vkGetMemoryWin32HandleNV{};                   // VkResult (VkDevice device, VkDeviceMemory memory, VkExternalMemoryHandleTypeFlagsNV handleType, HANDLE* pHandle);

    // VK_EXT_full_screen_exclusive
    PFN_vkGetPhysicalDeviceSurfacePresentModes2EXT  vkGetPhysicalDeviceSurfacePresentModes2EXT{}; // VkResult (VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo, uint32_t* pPresentModeCount, VkPresentModeKHR* pPresentModes);
    PFN_vkAcquireFullScreenExclusiveModeEXT         vkAcquireFullScreenExclusiveModeEXT{};        // VkResult (VkDevice device, VkSwapchainKHR swapchain);
    PFN_vkReleaseFullScreenExclusiveModeEXT         vkReleaseFullScreenExclusiveModeEXT{};        // VkResult (VkDevice device, VkSwapchainKHR swapchain);
    PFN_vkGetDeviceGroupSurfacePresentModes2EXT     vkGetDeviceGroupSurfacePresentModes2EXT{};    // VkResult (VkDevice device, const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo, VkDeviceGroupPresentModeFlagsKHR* pModes);

#endif // VK_USE_PLATFORM_WIN32_KHR

public:

};


}// namespace buma3d
