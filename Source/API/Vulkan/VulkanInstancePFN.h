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

拡張がinstance-levelかdevice-levelかの判別はVulkan仕様書の拡張リストの、各拡張の"Extension Type"項に示されている。https://www.khronos.org/registry/vulkan/specs/1.2-extensions/html/vkspec.html#extensions
instance-levelの拡張にもかかわらず、vkGetDeviceProcAddrによって呼び出せる拡張機能も存在する。
第1引数にVkDevice、デバイスの子オブジェクトが来ているinstance-level関数等はvkGetDeviceProcAddrでオーバーライド出来る可能性高い。

*/
namespace buma3d
{

class InstancePFN
{
public:
    InstancePFN() 
    {}
    InstancePFN(DeviceFactoryVk* _factory);
    ~InstancePFN();

public:
    // VK_EXT_debug_report
    PFN_vkCreateDebugReportCallbackEXT                      vkCreateDebugReportCallbackEXT{};                        // VkResult (VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback);
    PFN_vkDestroyDebugReportCallbackEXT                     vkDestroyDebugReportCallbackEXT{};                       // void (VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator);
    PFN_vkDebugReportMessageEXT                             vkDebugReportMessageEXT{};                               // void (VkInstance instance, VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage);

    // VK_EXT_debug_utils instance-level
    PFN_vkCreateDebugUtilsMessengerEXT                      vkCreateDebugUtilsMessengerEXT{};                        // VkResult (VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pMessenger);
    PFN_vkDestroyDebugUtilsMessengerEXT                     vkDestroyDebugUtilsMessengerEXT{};                       // void (VkInstance instance, VkDebugUtilsMessengerEXT messenger, const VkAllocationCallbacks* pAllocator);
    PFN_vkSubmitDebugUtilsMessageEXT                        vkSubmitDebugUtilsMessageEXT{};                          // void (VkInstance instance, VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageTypes, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData);

    // VK_KHR_get_physical_device_properties2
    PFN_vkGetPhysicalDeviceFeatures2KHR                     vkGetPhysicalDeviceFeatures2KHR{};                       // void (VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures2* pFeatures);
    PFN_vkGetPhysicalDeviceProperties2KHR                   vkGetPhysicalDeviceProperties2KHR{};                     // void (VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties2* pProperties);
    PFN_vkGetPhysicalDeviceFormatProperties2KHR             vkGetPhysicalDeviceFormatProperties2KHR{};               // void (VkPhysicalDevice physicalDevice, VkFormat format, VkFormatProperties2* pFormatProperties);
    PFN_vkGetPhysicalDeviceImageFormatProperties2KHR        vkGetPhysicalDeviceImageFormatProperties2KHR{};          // VkResult (VkPhysicalDevice physicalDevice, const VkPhysicalDeviceImageFormatInfo2* pImageFormatInfo, VkImageFormatProperties2* pImageFormatProperties);
    PFN_vkGetPhysicalDeviceQueueFamilyProperties2KHR        vkGetPhysicalDeviceQueueFamilyProperties2KHR{};          // void (VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount, VkQueueFamilyProperties2* pQueueFamilyProperties);
    PFN_vkGetPhysicalDeviceMemoryProperties2KHR             vkGetPhysicalDeviceMemoryProperties2KHR{};               // void (VkPhysicalDevice physicalDevice, VkPhysicalDeviceMemoryProperties2* pMemoryProperties);

    PFN_vkGetPhysicalDeviceSparseImageFormatProperties2KHR  vkGetPhysicalDeviceSparseImageFormatProperties2KHR{};    // void (VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSparseImageFormatInfo2* pFormatInfo, uint32_t* pPropertyCount, VkSparseImageFormatProperties2* pProperties);

    // VK_KHR_surface
    PFN_vkDestroySurfaceKHR                                 vkDestroySurfaceKHR{};                                   // void (VkInstance instance, VkSurfaceKHR surface, const VkAllocationCallbacks* pAllocator);
    PFN_vkGetPhysicalDeviceSurfaceSupportKHR                vkGetPhysicalDeviceSurfaceSupportKHR{};                  // VkResult (VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, VkSurfaceKHR surface, VkBool32* pSupported);
    PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR           vkGetPhysicalDeviceSurfaceCapabilitiesKHR{};             // VkResult (VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkSurfaceCapabilitiesKHR* pSurfaceCapabilities);
    PFN_vkGetPhysicalDeviceSurfaceFormatsKHR                vkGetPhysicalDeviceSurfaceFormatsKHR{};                  // VkResult (VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t* pSurfaceFormatCount, VkSurfaceFormatKHR* pSurfaceFormats);
    PFN_vkGetPhysicalDeviceSurfacePresentModesKHR           vkGetPhysicalDeviceSurfacePresentModesKHR{};             // VkResult (VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t* pPresentModeCount, VkPresentModeKHR* pPresentModes);

    // VK_KHR_display
    PFN_vkGetPhysicalDeviceDisplayPropertiesKHR             vkGetPhysicalDeviceDisplayPropertiesKHR{};               // VkResult (VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount, VkDisplayPropertiesKHR* pProperties);
    PFN_vkGetPhysicalDeviceDisplayPlanePropertiesKHR        vkGetPhysicalDeviceDisplayPlanePropertiesKHR{};          // VkResult (VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount, VkDisplayPlanePropertiesKHR* pProperties);
    PFN_vkGetDisplayPlaneSupportedDisplaysKHR               vkGetDisplayPlaneSupportedDisplaysKHR{};                 // VkResult (VkPhysicalDevice physicalDevice, uint32_t planeIndex, uint32_t* pDisplayCount, VkDisplayKHR* pDisplays);
    PFN_vkGetDisplayModePropertiesKHR                       vkGetDisplayModePropertiesKHR{};                         // VkResult (VkPhysicalDevice physicalDevice, VkDisplayKHR display, uint32_t* pPropertyCount, VkDisplayModePropertiesKHR* pProperties);
    PFN_vkCreateDisplayModeKHR                              vkCreateDisplayModeKHR{};                                // VkResult (VkPhysicalDevice physicalDevice, VkDisplayKHR display, const VkDisplayModeCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDisplayModeKHR* pMode);
    PFN_vkGetDisplayPlaneCapabilitiesKHR                    vkGetDisplayPlaneCapabilitiesKHR{};                      // VkResult (VkPhysicalDevice physicalDevice, VkDisplayModeKHR mode, uint32_t planeIndex, VkDisplayPlaneCapabilitiesKHR* pCapabilities);
    PFN_vkCreateDisplayPlaneSurfaceKHR                      vkCreateDisplayPlaneSurfaceKHR{};                        // VkResult (VkInstance instance, const VkDisplaySurfaceCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface);

    // VK_KHR_get_display_properties2
    PFN_vkGetPhysicalDeviceDisplayProperties2KHR            vkGetPhysicalDeviceDisplayProperties2KHR{};              // VkResult (VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount, VkDisplayProperties2KHR* pProperties);
    PFN_vkGetPhysicalDeviceDisplayPlaneProperties2KHR       vkGetPhysicalDeviceDisplayPlaneProperties2KHR{};         // VkResult (VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount, VkDisplayPlaneProperties2KHR* pProperties);
    PFN_vkGetDisplayModeProperties2KHR                      vkGetDisplayModeProperties2KHR{};                        // VkResult (VkPhysicalDevice physicalDevice, VkDisplayKHR display, uint32_t* pPropertyCount, VkDisplayModeProperties2KHR* pProperties);
    PFN_vkGetDisplayPlaneCapabilities2KHR                   vkGetDisplayPlaneCapabilities2KHR{};                     // VkResult (VkPhysicalDevice physicalDevice, const VkDisplayPlaneInfo2KHR* pDisplayPlaneInfo, VkDisplayPlaneCapabilities2KHR* pCapabilities);

    // VK_NV_external_memory_capabilities
    PFN_vkGetPhysicalDeviceExternalImageFormatPropertiesNV  vkGetPhysicalDeviceExternalImageFormatPropertiesNV{};    // VkResult (VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type, VkImageTiling tiling, VkImageUsageFlags usage, VkImageCreateFlags flags, VkExternalMemoryHandleTypeFlagsNV externalHandleType, VkExternalImageFormatPropertiesNV* pExternalImageFormatProperties);

    // VK_KHR_get_surface_capabilities2
    PFN_vkGetPhysicalDeviceSurfaceCapabilities2KHR          vkGetPhysicalDeviceSurfaceCapabilities2KHR{};            // VkResult (VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo, VkSurfaceCapabilities2KHR* pSurfaceCapabilities);
    PFN_vkGetPhysicalDeviceSurfaceFormats2KHR               vkGetPhysicalDeviceSurfaceFormats2KHR{};                 // VkResult (VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo, uint32_t* pSurfaceFormatCount, VkSurfaceFormat2KHR* pSurfaceFormats);

    // VK_EXT_display_surface_counter
    PFN_vkGetPhysicalDeviceSurfaceCapabilities2EXT          vkGetPhysicalDeviceSurfaceCapabilities2EXT{};            // VkResult (VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkSurfaceCapabilities2EXT* pSurfaceCapabilities);

#ifdef VK_USE_PLATFORM_WIN32_KHR

                                                                                                                     // VK_KHR_win32_surface
    PFN_vkCreateWin32SurfaceKHR                             vkCreateWin32SurfaceKHR{};                               // VkResult (VkInstance instance, const VkWin32SurfaceCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface);
    PFN_vkGetPhysicalDeviceWin32PresentationSupportKHR      vkGetPhysicalDeviceWin32PresentationSupportKHR{};        // VkBool32 (VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex);

#endif // VK_USE_PLATFORM_WIN32_KHR

public:

};


}// namespace buma3d
