#pragma once

namespace buma3d
{

class B3D_API DeviceAdapterVk : public IDeviceAdapter, public util::details::NEW_DELETE_OVERRIDE
{
public:
    struct QUEUE_PROPERTIES_MAP
    {
        uint32_t     queue_family_index;
        VkQueueFlags queue_flags;
    };

protected:
    B3D_APIENTRY DeviceAdapterVk();
    DeviceAdapterVk(const DeviceAdapterVk&) = delete;
    B3D_APIENTRY ~DeviceAdapterVk();

private:
    BMRESULT B3D_APIENTRY Init(DeviceFactoryVk* _factory, VkPhysicalDevice _phys_device);
    BMRESULT B3D_APIENTRY Init(DeviceFactoryVk* _factory, const VkPhysicalDeviceGroupProperties& _pd_props);
    BMRESULT B3D_APIENTRY GetFeatures();
    BMRESULT B3D_APIENTRY GetProperties();
    void B3D_APIENTRY InitDesc();
    void B3D_APIENTRY Uninit();

public:
    static BMRESULT
        B3D_APIENTRY Create(DeviceFactoryVk* _factory, VkPhysicalDevice _phys_device, DeviceAdapterVk** _dst);

    static BMRESULT
        B3D_APIENTRY CreateForDeviceGroup(DeviceFactoryVk* _factory, const VkPhysicalDeviceGroupProperties& _pd_props, DeviceAdapterVk** _dst);

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

    const DEVICE_ADAPTER_DESC&
        B3D_APIENTRY GetDesc() const override;

    uint32_t
        B3D_APIENTRY GetCommandQueueProperties(COMMAND_QUEUE_PROPERTIES* _properties) override;

    void
        B3D_APIENTRY GetDeviceAdapterLimits(DEVICE_ADAPTER_LIMITS* _dst_limits) override;

    BMRESULT
        B3D_APIENTRY CreateSurface(const SURFACE_DESC& _desc, ISurface** _dst) override;

    BMRESULT
        B3D_APIENTRY QueryPresentationSupport(COMMAND_TYPE _queue_type, const ISurface* _surface) override;

    const util::PHYSICAL_DEVICE_DATA&
        B3D_APIENTRY GetPhysicalDeviceData() const;

    DeviceFactoryVk*
        B3D_APIENTRY GetDeviceFactoryVk();

    VkPhysicalDevice
        B3D_APIENTRY GetVkPhysicalDevice() const;

    uint32_t
        B3D_APIENTRY GetPhysicalDeviceAPIVersion() const;

    const InstancePFN&
        B3D_APIENTRY GetInstancePFN() const;

    DeviceFactoryVk*
        B3D_APIENTRY GetDeviceFactory() const;

    bool
        B3D_APIENTRY IsEnabledDebug();

    void
        B3D_APIENTRY AddMessageFromB3D(DEBUG_MESSAGE_SEVERITY _severity, DEBUG_MESSAGE_CATEGORY_FLAG _category, const char* _str);

    const VkPhysicalDeviceGroupProperties*
        B3D_APIENTRY GetVkPhysicalDeviceGroupProperties() const;

    util::MemoryProperties&
        B3D_APIENTRY GetMemoryProperties();

    const util::StArray<util::UniquePtr<QUEUE_PROPERTIES_MAP>, COMMAND_TYPE_NUM_TYPES>&
        B3D_APIENTRY QueuePropertiesMap() const;

private:
    std::atomic_uint32_t                        ref_count;
    util::UniquePtr<util::NameableObjStr>       name;
    DEVICE_ADAPTER_DESC                         desc;
    DeviceFactoryVk*                            factory;
    VkPhysicalDevice                            physical_device;
    uint32_t                                    api_version;
    util::UniquePtr<util::PHYSICAL_DEVICE_DATA> pd_data;
    const InstancePFN*                          inspfn;
    util::MemoryProperties                      mem_props;
    
    util::UniquePtr<VkPhysicalDeviceGroupProperties>                                phys_dev_group_props;
    util::StArray<util::UniquePtr<QUEUE_PROPERTIES_MAP>, COMMAND_TYPE_NUM_TYPES>    queue_properties_map;


};


}// namespace buma3d
