#pragma once

namespace buma3d
{

// デバッグ時に使用
struct DEBUG_REPORT_USER_DATA;

class B3D_API DeviceFactoryVk : public IDeviceFactory, public util::details::NEW_DELETE_OVERRIDE
{
protected:
    B3D_APIENTRY DeviceFactoryVk();
    DeviceFactoryVk(const DeviceFactoryVk&) = delete;
    B3D_APIENTRY ~DeviceFactoryVk();

private:
    BMRESULT B3D_APIENTRY Init(const DEVICE_FACTORY_DESC& _desc);
    void B3D_APIENTRY CopyDesc(const DEVICE_FACTORY_DESC& _desc);
    void B3D_APIENTRY GetInstanceLayerNames(util::DyArray<util::SharedPtr<util::String>>* _dst_all_layer_names);
    void B3D_APIENTRY GetInstanceExtensionNames(const util::DyArray<util::SharedPtr<util::String>>& _all_layer_names, util::DyArray<util::SharedPtr<util::String>>* _dst_all_ext_names);
    bool B3D_APIENTRY CheckInstanceLayerSupport(const util::DyArray<util::SharedPtr<util::String>>& _all_layer_names, const char* _request_layer_name);
    bool B3D_APIENTRY CheckInstanceExtensionSupport(const util::DyArray<util::SharedPtr<util::String>>& _all_ext_names, const char* _request_ext_name);
    void B3D_APIENTRY SetAllocationCallbacks();
    BMRESULT B3D_APIENTRY CreateInstance();
    BMRESULT B3D_APIENTRY EnumPhysicalDevices();
    BMRESULT B3D_APIENTRY SetDebugLayer();
    BMRESULT B3D_APIENTRY CreateDebugMessageQueue();
    void B3D_APIENTRY Uninit();

public:
    static BMRESULT
        B3D_APIENTRY Create(const DEVICE_FACTORY_DESC& _desc, DeviceFactoryVk** _dst);

    void
        B3D_APIENTRY AddRef() override;

    uint32_t
        B3D_APIENTRY Release() override;

    uint32_t
        B3D_APIENTRY GetRefCount() const override;

    const char*
        B3D_APIENTRY GetName() const override;

    BMRESULT
        B3D_APIENTRY SetName(const char* _name) override;

    const DEVICE_FACTORY_DESC&
        B3D_APIENTRY GetDesc() const override;

    BMRESULT
        B3D_APIENTRY GetDebugMessageQueue(IDebugMessageQueue** _dst) override;

    BMRESULT
        B3D_APIENTRY EnumAdapters(uint32_t _adapter_index, IDeviceAdapter** _dst_adapter) override;

    BMRESULT
        B3D_APIENTRY CreateDevice(const DEVICE_DESC& _desc, IDevice** _dst) override;

    VkInstance
        B3D_APIENTRY GetVkInstance() const;

    const VkAllocationCallbacks*
        B3D_APIENTRY GetVkAllocationCallbacks() const;

    const InstancePFN&
        B3D_APIENTRY GetInstancePFN() const;

    uint32_t
        B3D_APIENTRY GetInstanceAPIVersion() const;

    const util::DyArray<util::SharedPtr<util::String>>&
        B3D_APIENTRY GetEnabledInstanceLayers() const;

    const util::DyArray<util::SharedPtr<util::String>>& 
        B3D_APIENTRY GetEnabledInstanceExtensions() const;

    bool
        B3D_APIENTRY CheckInstanceLayerSupport(const char* _request_layer_name) const;

    bool
        B3D_APIENTRY CheckInstanceExtensionSupport(const char* _request_ext_name) const;

    bool
        B3D_APIENTRY CheckInstanceLayerEnabled(const char* _request_layer_name) const;

    bool
        B3D_APIENTRY CheckInstanceExtensionEnabled(const char* _request_ext_name) const;

    bool
        B3D_APIENTRY IsEnabledDebug();

    void
        B3D_APIENTRY AddMessageFromB3D(DEBUG_MESSAGE_SEVERITY _severity, DEBUG_MESSAGE_CATEGORY_FLAG _category, const Char8T* _str);

private:
    std::atomic_uint32_t                            ref_count;
    util::UniquePtr<util::NameableObjStr>           name;
    DEVICE_FACTORY_DESC                             desc;
    util::Ptr<DebugMessageQueueVk>                  message_queue;
    util::DyArray<VkPhysicalDevice>                 physical_devices;
    util::DyArray<VkPhysicalDeviceGroupProperties>  physical_device_group_properties;
    VkInstance                                      instance;
    util::DyArray<util::SharedPtr<util::String>>    all_layers;
    util::DyArray< util::SharedPtr<util::String>>   all_extensions;
    util::DyArray<util::SharedPtr<util::String>>    enabled_layers;
    util::DyArray< util::SharedPtr<util::String>>   enabled_extensions;
    util::UniquePtr<InstancePFN>                    inspfn;
    uint32_t                                        api_version;
    util::UniquePtr<buma3d::DEBUG_REPORT_USER_DATA> user_data;

    /* https://www.khronos.org/registry/vulkan/specs/1.2-extensions/html/vkspec.html#memory-host
    Vulkanは、Vulkanの実装に代わってホストメモリの割り当てを実行する機会をアプリケーションに提供します。
    この機能を使用しない場合、実装は独自のメモリ割り当てを実行します。
    ほとんどのメモリ割り当てはクリティカルパスから外れているため、これはパフォーマンス機能を意味するものではありません。
    むしろ、これは特定の組み込みシステム、デバッグ目的（たとえば、すべてのホスト割り当ての後にガードページを置く）、またはメモリ割り当てのロギングに役立ちます。
    */
    VkAllocationCallbacks* alloc_callbacks;

    struct IVulkanDebugCallbackObject;
    class VulkanDebugCallbackObjectDebugUtils;
    class VulkanDebugCallbackObjectDebugReport;
    IVulkanDebugCallbackObject* debug_callback;

};


}// namespace buma3d
