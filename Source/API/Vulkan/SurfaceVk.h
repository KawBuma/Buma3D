#pragma once

namespace buma3d
{

class B3D_API SurfaceVk : public ISurface, public util::details::NEW_DELETE_OVERRIDE
{
private:
    B3D_APIENTRY SurfaceVk();
    B3D_APIENTRY SurfaceVk(const SurfaceVk&) = delete;
    B3D_APIENTRY ~SurfaceVk();

private:
    BMRESULT B3D_APIENTRY Init(DeviceAdapterVk* _adapter, const  SURFACE_DESC& _desc);
    BMRESULT B3D_APIENTRY CopyDesc(const SURFACE_DESC& _desc);
    BMRESULT B3D_APIENTRY CreateVkSurface();
    BMRESULT B3D_APIENTRY CreateSurfaceData();
    void B3D_APIENTRY Uninit();

public:
    static BMRESULT
        B3D_APIENTRY Create(DeviceAdapterVk* _adapter, const SURFACE_DESC& _desc, SurfaceVk** _dst);

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

    const SURFACE_DESC&
        B3D_APIENTRY GetDesc() const override;

    SURFACE_STATE
        B3D_APIENTRY GetState() override;

    uint32_t
        B3D_APIENTRY GetSupportedSurfaceFormats(SURFACE_FORMAT* _dst) override;

    VkPhysicalDevice
        B3D_APIENTRY GetVkPhysicalDevice() const;

    VkSurfaceKHR 
        B3D_APIENTRY GetVkSurface() const;

    const VkAllocationCallbacks* 
        B3D_APIENTRY GetVkAllocationCallbacks() const;

    bool
        B3D_APIENTRY IsEnabledDebug();

    void
        B3D_APIENTRY AddMessageFromB3D(DEBUG_MESSAGE_SEVERITY _severity, DEBUG_MESSAGE_CATEGORY_FLAG _category, const Char8T* _str);

    const util::SURFACE_DATA&
        B3D_APIENTRY GetSurfaceData();

private:
    std::atomic_uint32_t                    ref_count;
    util::UniquePtr<util::NameableObjStr>   name;
    DeviceAdapterVk*                        adapter;
    SURFACE_DESC                            desc;
    VkInstance                              instance;
    VkPhysicalDevice                        physical_device;
    VkSurfaceKHR                            surface;
    const InstancePFN*                      inspfn;
    const VkAllocationCallbacks*            callbacks;
    util::UniquePtr<util::SURFACE_DATA>     surface_data;

};


}// namespace buma3d
