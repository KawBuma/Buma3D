#include "Buma3DPCH.h"
#include "VulkanInstancePFN.h"

namespace buma3d
{

#define LOAD(v)                                                         \
    v = (PFN_##v)vkGetInstanceProcAddr(instance, #v);                   \
    if constexpr (IS_ENABLE_DEBUG_OUTPUT)                               \
    {                                                                   \
        if (v == nullptr)                                               \
        {                                                               \
            buma3d::hlp::OutDebugStr(#v" is not enabled/supported.\n"); \
        }                                                               \
    }

InstancePFN::InstancePFN(DeviceFactoryVk* _factory)
{
    auto instance = _factory->GetVkInstance();
    // VK_EXT_debug_report
    if (_factory->CheckInstanceExtensionSupport(VK_EXT_DEBUG_REPORT_EXTENSION_NAME))
    {
        LOAD(vkCreateDebugReportCallbackEXT);
        LOAD(vkDestroyDebugReportCallbackEXT);
        LOAD(vkDebugReportMessageEXT);
    }

    // VK_EXT_debug_utils (instance-level)
    if (_factory->CheckInstanceExtensionSupport(VK_EXT_DEBUG_UTILS_EXTENSION_NAME))
    {
        LOAD(vkCreateDebugUtilsMessengerEXT);
        LOAD(vkDestroyDebugUtilsMessengerEXT);
        LOAD(vkSubmitDebugUtilsMessageEXT);
    }

    // VK_KHR_get_physical_device_properties2
    if (_factory->CheckInstanceExtensionSupport(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME))
    {
        LOAD(vkGetPhysicalDeviceFeatures2KHR);
        LOAD(vkGetPhysicalDeviceProperties2KHR);
        LOAD(vkGetPhysicalDeviceFormatProperties2KHR);
        LOAD(vkGetPhysicalDeviceImageFormatProperties2KHR);
        LOAD(vkGetPhysicalDeviceQueueFamilyProperties2KHR);
        LOAD(vkGetPhysicalDeviceMemoryProperties2KHR);
        LOAD(vkGetPhysicalDeviceSparseImageFormatProperties2KHR);
    }

    // VK_KHR_surface
    if (_factory->CheckInstanceExtensionSupport(VK_KHR_SURFACE_EXTENSION_NAME))
    {
        LOAD(vkDestroySurfaceKHR);
        LOAD(vkGetPhysicalDeviceSurfaceSupportKHR);
        LOAD(vkGetPhysicalDeviceSurfaceCapabilitiesKHR);
        LOAD(vkGetPhysicalDeviceSurfaceFormatsKHR);
        LOAD(vkGetPhysicalDeviceSurfacePresentModesKHR);
    }

    // VK_KHR_display
    if (_factory->CheckInstanceExtensionSupport(VK_KHR_DISPLAY_EXTENSION_NAME))
    {
        LOAD(vkGetPhysicalDeviceDisplayPropertiesKHR);
        LOAD(vkGetPhysicalDeviceDisplayPlanePropertiesKHR);
        LOAD(vkGetDisplayPlaneSupportedDisplaysKHR);
        LOAD(vkGetDisplayModePropertiesKHR);
        LOAD(vkCreateDisplayModeKHR);
        LOAD(vkGetDisplayPlaneCapabilitiesKHR);
        LOAD(vkCreateDisplayPlaneSurfaceKHR);
    }

    // VK_KHR_get_display_properties2
    if (_factory->CheckInstanceExtensionSupport(VK_KHR_GET_DISPLAY_PROPERTIES_2_EXTENSION_NAME))
    {
        LOAD(vkGetPhysicalDeviceDisplayProperties2KHR);
        LOAD(vkGetPhysicalDeviceDisplayPlaneProperties2KHR);
        LOAD(vkGetDisplayModeProperties2KHR);
        LOAD(vkGetDisplayPlaneCapabilities2KHR);
    }

    // VK_NV_external_memory_capabilities
    if (_factory->CheckInstanceExtensionSupport(VK_NV_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME))
    {
        LOAD(vkGetPhysicalDeviceExternalImageFormatPropertiesNV);
    }

    // VK_KHR_get_surface_capabilities2
    if (_factory->CheckInstanceExtensionSupport(VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME))
    {
        LOAD(vkGetPhysicalDeviceSurfaceCapabilities2KHR);
        LOAD(vkGetPhysicalDeviceSurfaceFormats2KHR);
    }

    // VK_EXT_display_surface_counter
    if (_factory->CheckInstanceExtensionSupport(VK_EXT_DISPLAY_SURFACE_COUNTER_EXTENSION_NAME))
    {
        LOAD(vkGetPhysicalDeviceSurfaceCapabilities2EXT);
    }

#ifdef VK_USE_PLATFORM_WIN32_KHR

    // VK_KHR_win32_surface
    if (_factory->CheckInstanceExtensionSupport(VK_KHR_WIN32_SURFACE_EXTENSION_NAME))
    {
        LOAD(vkCreateWin32SurfaceKHR);
        LOAD(vkGetPhysicalDeviceWin32PresentationSupportKHR); 
    }

#endif // VK_USE_PLATFORM_WIN32_KHR

}

InstancePFN::~InstancePFN()
{
}


}// buma3d
