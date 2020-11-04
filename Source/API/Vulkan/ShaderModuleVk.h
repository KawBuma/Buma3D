#pragma once

namespace buma3d
{

class B3D_API ShaderModuleVk : public IDeviceChildVk<IShaderModule>, public util::details::NEW_DELETE_OVERRIDE
{
protected:
    B3D_APIENTRY ShaderModuleVk();
    ShaderModuleVk(const ShaderModuleVk&) = delete;
    B3D_APIENTRY ~ShaderModuleVk();

private:
    BMRESULT B3D_APIENTRY Init(DeviceVk* _device, const SHADER_MODULE_DESC& _desc);
    void B3D_APIENTRY CopyDesc(const SHADER_MODULE_DESC& _desc);
    void B3D_APIENTRY Uninit();

public:
    static BMRESULT
        B3D_APIENTRY Create(DeviceVk* _device, const SHADER_MODULE_DESC& _desc, ShaderModuleVk** _dst);

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

    IDevice*
        B3D_APIENTRY GetDevice() const override;

    const VkAllocationCallbacks*
        B3D_APIENTRY GetVkAllocationCallbacks() const override;

    const InstancePFN&
        B3D_APIENTRY GetIsntancePFN() const override;

    const DevicePFN&
        B3D_APIENTRY GetDevicePFN() const override;

    VkShaderModule
        B3D_APIENTRY GetVkShaderModule() const;

private:
    struct DESC_DATA
    {
        std::vector<uint8_t> shader_bytecode;
    };

private:
    std::atomic_uint32_t                    ref_count;
    util::UniquePtr<util::NameableObjStr>   name;
    DeviceVk*                               device;
    SHADER_MODULE_DESC                      desc;
    DESC_DATA                               desc_data;

    VkDevice                                vkdevice;
    const InstancePFN*                      inspfn;
    const DevicePFN*                        devpfn;
    VkShaderModule                          shader_module;

};


}// namespace buma3d
